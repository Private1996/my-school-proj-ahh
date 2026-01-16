#ifndef MIDIPLAYER_H
#define MIDIPLAYER_H
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QSerialPort>
#include <drumstick/qsmf.h>
#include <vector>
struct ScheduledEvent {
    long timestampMs; // mean when(after music starts to play) to send command
    uint8_t statusChannel; //status and channel
    uint8_t data1;
    uint8_t data2;
    bool isTempoEvent;
    int newTempoMicros;
    bool operator<(const ScheduledEvent& other) const {
        return timestampMs < other.timestampMs; //sort operator: make sure event sorted by time
    }
};
class midiPlayer : public QObject {
    Q_OBJECT
public:
    explicit midiPlayer(QObject *parent = nullptr);
    ~midiPlayer();
    bool loadMid(const QString &filename);
    void start();
    void stop();
    bool connectArduino(const QString &portName);
    void disconnectArduino();
signals:
    void finished();
    void errorOccurred(QString msg);
private slots:
    void onSmfNoteOn(int chan, int pitch, int vol); //meant to be used in loadMid()
    void onSmfNoteOff(int chan, int pitch, int vol);
    void onSmfTempo(int tempo);
private:
    void sendMidi(int status, int d1, int d2); //send to arduino
    void clearPlaylist(); //it explained itself clearly
    QSerialPort *m_serial;
    drumstick::File::QSmf *m_smfReader; //meant to be used in loadMid()
    std::vector<ScheduledEvent> m_playlist;
    int m_ppq; //pulses per quarter notes / ticks per quarter notes
    bool m_stopFlag;
};
#endif // MIDIPLAYER_H
