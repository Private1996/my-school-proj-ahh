#include "mainwindow.h"
#include "Logger.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <qdatetime.h>
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString txt;
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    switch (type) {
    case QtDebugMsg:
        txt = QString("[%1] [Debug] %2").arg(timeStr).arg(msg);
        break;
    case QtInfoMsg:
        txt = QString("[%1] [Info] %2").arg(timeStr).arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("[%1] [Warning] %2").arg(timeStr).arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("[%1] [Critical] %2").arg(timeStr).arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("[%1] [Fatal] %2").arg(timeStr).arg(msg);
        break;
    }

    Logger::instance().sendLog(type, txt);
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "evilArduinoClient_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    qInstallMessageHandler(myMessageOutput);

    MainWindow w;    
    w.show();
    return a.exec();
}
