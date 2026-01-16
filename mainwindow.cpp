#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Logger.h"
#include <QStandardPaths>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include "midiPlayer.h"
#include <QSerialPortInfo>
QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
midiPlayer* i_midiPlayer; //top criminal design
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    updateSerialPorts();
    connect(&Logger::instance(), &Logger::newLog, this, &MainWindow::onNewLog);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_browserButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("選擇檔案："), homePath, tr("Midi檔案 (*.mid);;所有檔案 (*.*)"));
    if (!fileName.isEmpty()) {
        ui->filePathLineEdit->setText(fileName);
    }
}
void MainWindow::onNewLog(QtMsgType type, const QString &msg)
{
    QString color;
    switch (type) {
    case QtDebugMsg:    color = "gray"; break;
    case QtInfoMsg:     color = "#0E4C92"; break;
    case QtWarningMsg:  color = "#FFA500"; break;
    case QtCriticalMsg: color = "red"; break;
    case QtFatalMsg:    color = "darkred"; break;
    }

    QString html = QString("<span style='color:%1'>%2</span>").arg(color).arg(msg);

    ui->debugBrowser->append(html);
}
void MainWindow::onFinished() {
    i_midiPlayer->disconnectArduino();
    qInfo() << "播放完畢！";
    ui->stopPushButton->hide();
    ui->transPushButton->show();
}
void MainWindow::updateSerialPorts() {
    ui->portComboBox->clear();
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        ui->portComboBox->addItem(info.portName()+(info.description().isEmpty() ? "" : " ("+info.description()+")"), info.systemLocation());
    }
}
void MainWindow::on_refreshPortButton_clicked()
{
    updateSerialPorts();
}





void MainWindow::on_transPushButton_clicked()
{
    i_midiPlayer = new midiPlayer(this);
    connect(i_midiPlayer, &midiPlayer::errorOccurred, [&](QString msg){
        qCritical() << "發生錯誤：" << msg;
        QMessageBox::information(nullptr, "播放midi時發生錯誤！", msg);
        i_midiPlayer->disconnectArduino();

    });
    connect(i_midiPlayer, &midiPlayer::finished, this, &MainWindow::onFinished);
    ui->stopPushButton->hide();
    if (!i_midiPlayer->connectArduino(ui->portComboBox->currentData().toString())) {
        qCritical() << "無法連接至Arduino";
        return;
    } else if (!i_midiPlayer->loadMid(ui->filePathLineEdit->text())) {
        qCritical() << "Midi讀取異常";
    } else {
        ui->stopPushButton->show();
        ui->transPushButton->hide();
        i_midiPlayer->start();
    }
}



void MainWindow::on_stopPushButton_clicked()
{
    if (!i_midiPlayer) {
        qCritical() << "現在無Midi播放器";
        return;
    } else {
        i_midiPlayer->stop();
        i_midiPlayer->disconnectArduino();
        qInfo() << "現在應該已停止Midi播放器";
        ui->stopPushButton->hide();
        ui->transPushButton->show();
    }
}

