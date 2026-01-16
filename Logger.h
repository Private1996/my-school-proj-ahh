#ifndef LOGGER_H
#define LOGGER_H
#include <qobject.h>
class Logger : public QObject
{
Q_OBJECT
public:
static Logger& instance() {
    static Logger _instance;
    return _instance;
}
void sendLog(QtMsgType type, const QString &msg) {
    emit newLog(type,msg);
}
signals:
void newLog(QtMsgType type, const QString &msg);
private:
    Logger() {}
};
void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
#endif // LOGGER_H
