#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_browserButton_clicked();
    void onNewLog(QtMsgType type, const QString &msg);
    void on_refreshPortButton_clicked();
    void updateSerialPorts();
    void onFinished();
    void on_transPushButton_clicked();

    void on_stopPushButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
