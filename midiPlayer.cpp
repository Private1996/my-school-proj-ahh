#include <drumstick/qsmf.h>
#include <drumstick.h>
#include <QSerialPort>
#include <QDebug>
#include <QMessageBox>
#include <algorithm>
#include "midiPlayer.h"
namespace drum = drumstick::File;
midiPlayer::midiPlayer(QObject *parent) : QObject(parent), m_ppq(480), m_stopFlag(false)
{
    m_serial = new QSerialPort(this);
}
midiPlayer::~midiPlayer() {
    if (m_serial->isOpen()) {
        m_serial->close(); //make sure port is closed
    }
}
bool midiPlayer::connectArduino(const QString &portName) {
    if (m_serial->isOpen()) {
        m_serial->close();
        QThread::msleep(100); //wait till things released
    };

    m_serial->setPortName(portName);
    m_serial->setBaudRate(115200); //fixed rate idk why
    m_serial->setDataBits(QSerialPort::Data8); //or char
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serial->open(QIODevice::ReadWrite)) {
        qInfo() << "Midi播放器: Arduino 連接成功於" << portName;
        return true;
    }
    emit errorOccurred("無法開啟 Serial Port: " + portName + " （因" + m_serial->errorString() + ")");
    return false;
}
void midiPlayer::disconnectArduino()
{
    if (m_serial->isOpen()) m_serial->close();
    qInfo() << "Midi播放器: 與Arduino中斷連接";
}
void midiPlayer::clearPlaylist() {
    m_playlist.clear();
}


//where something great start ahh
bool midiPlayer::loadMid(const QString &filename) {
    m_smfReader = new drum::QSmf(this);
    //create smfReader to load midi

    int trackCount = m_smfReader->getTracks();

    m_ppq = m_smfReader->getDivision();
    bool ok = true;

    ok &= (connect(m_smfReader, &drum::QSmf::signalSMFNoteOn, this, &midiPlayer::onSmfNoteOn) != nullptr);
    ok &= (connect(m_smfReader, &drum::QSmf::signalSMFNoteOff, this, &midiPlayer::onSmfNoteOff) != nullptr);
    ok &= (connect(m_smfReader, &drum::QSmf::signalSMFTempo, this, &midiPlayer::onSmfTempo) != nullptr);
    try {
        m_smfReader->readFromFile(filename);
    } catch (std::exception &e) {
        emit errorOccurred("Midi播放器: 無法讀取檔案"+QString(e.what()));
        m_smfReader->deleteLater();
        m_smfReader = nullptr;
        return false;
    }

    qDebug() << "Midi播放器: 載入" << filename << " 軌道數: " << trackCount << "PPQ: " << m_ppq;
    //now useless
    //bye
    m_smfReader->deleteLater();
    m_smfReader = nullptr;
    if (m_playlist.empty()) {
        qWarning() << "Midi播放器: 未讀取任何音符，Midi檔可能為空";
        return false;
    }
    std::sort(m_playlist.begin(), m_playlist.end());
    qInfo() << "Midi播放器: 解析完畢，一共有" << m_playlist.size() << "個事件，PPQ: " << m_ppq;
    return true;
}



//manage signal slots here
void midiPlayer::onSmfNoteOn(int chan, int pitch, int vol) {
    long tick = m_smfReader->getCurrentTime(); //get absolute time
    ScheduledEvent se;
    se.timestampMs = tick;
    se.statusChannel = 0x90 | (chan & 0x0F);
    se.data1 = pitch;
    se.data2 = vol;
    se.isTempoEvent = false;
    m_playlist.push_back(se);
}
void midiPlayer::onSmfNoteOff(int chan, int pitch, int vol) {
    long tick = m_smfReader->getCurrentTime(); //get absolute time
    ScheduledEvent se;
    se.timestampMs = tick;
    se.statusChannel = 0x80 | (chan & 0x0F);
    se.data1 = pitch;
    se.data2 = vol;
    se.isTempoEvent = false;
    m_playlist.push_back(se);
}
void midiPlayer::onSmfTempo(int tempo)
{
    long tick = m_smfReader->getCurrentTime();

    ScheduledEvent se;
    se.timestampMs = tick;
    se.isTempoEvent = true;
    se.newTempoMicros = tempo;

    m_playlist.push_back(se);
}







//start/stop midi playing
void midiPlayer::start() {
    if (m_playlist.empty()) {
        emit errorOccurred("事件清單為空");
        return;
    }
    m_stopFlag = false;
    QElapsedTimer timer;
    timer.start();

    double currentTempoMicros = 500000.0; // 120 BPM
    double currentTimeMs = 0.0;
    long lastTick = 0;
    qInfo() << "Midi播放器: 開始播放...";
    for (const auto &event : m_playlist) {
        if (m_stopFlag) return;

        long deltaTicks = event.timestampMs - lastTick; //calculate delta tick like everyone do
        if (deltaTicks > 0) {
            double deltaMs = (deltaTicks * currentTempoMicros) / (1000.0 * m_ppq);
            currentTimeMs += deltaMs;


            while (timer.elapsed() < currentTimeMs) {
                QCoreApplication::processEvents(); //waste time while nothing to do
                if (m_stopFlag) return;
            }
        }
        if (event.isTempoEvent) {
            currentTempoMicros = event.newTempoMicros;
            qDebug() << "現在速度：" << currentTempoMicros;
        } else {
            sendMidi(event.statusChannel, event.data1, event.data2);
        }
        lastTick = event.timestampMs;
    }
    qInfo() << "Midi播放器: 播放結束";
    emit finished();
}
void midiPlayer::stop()
{
    m_stopFlag = true;
}


void midiPlayer::sendMidi(int status, int d1, int d2)
{
    if (m_serial->isOpen()) {
        QByteArray data;
        data.append(static_cast<char>(status));
        data.append(static_cast<char>(d1));
        data.append(static_cast<char>(d2));
        m_serial->write(data);
    }
}
