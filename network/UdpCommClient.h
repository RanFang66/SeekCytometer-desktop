#ifndef UDPCOMMCLIENT_H
#define UDPCOMMCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QByteArray>
#include "UdpCommFrame.h"
#include "Gate.h"
#include <QTimer>
#include "EventData.h"

using SampleData = QVector<QVector<int>>;




/**
 * @brief UdpCommClient class.
 * Encapsulates QUdpSocket send/receive logic and uses UdpCommFrame to parse frames.
 * Designed to run in a separate thread if desired.
 */
class UdpCommClient : public QObject
{
    Q_OBJECT
public:
    explicit UdpCommClient(QObject *parent = nullptr);

    void     startUdpClient();


public slots:
    /**
     * @brief Initializes and binds the UDP socket in its own thread.
     * @param localBindAddress The local IP to bind to.
     * @param localBindPort The local port to bind to.
     * @param remoteAddress The remote target IP to send to.
     * @param remotePort The remote target port to send to.
     */
    void doInitialize();

    /**
     * @brief Sends a frame according to the custom frame protocol.
     * @param commandType The 16-bit command type.
     * @param data The payload data.
     *
     * Each time you call sendFrame(), the internal sequence counter is incremented by 1.
     */
    bool sendFrame(CommCmdType commandType, const QByteArray &data);
    bool sendHandshake();
    bool sendWaveformRequest(bool enabled, int enabledChannels, int interval);
    bool sendAcquireStart();
    bool sendAcquireStop();
    bool sendSortingStart();
    bool sendSortingStop();
    bool sendDetectorSettings(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                              const QList<int> &roles = QList<int>());
    bool sendDriveParameters(int type, int delay, int width, int coolingTime, int coe);
    bool sendGateData(const Gate &gate);

    bool sendSpeedMeasureSetting(int preId, int postId, int preThresh, int dist, int maxTimeSpan);

    bool sendDisableDetector(int id);

private slots:
    void onHandshakeTimerTimeout();

signals:
    /**
     * @brief Emitted when a valid frame is parsed from the received data.
     * @param sequence The frame's sequence number.
     * @param commandType The 16-bit command type.
     * @param dataField The payload data from the frame.
     * @param sender The IP address of the sender.
     * @param senderPort The UDP port of the sender.
     */
    void frameReceived(quint16 sequence,
                       CommCmdType commandType,
                       const QByteArray &dataField,
                       const QHostAddress &sender,
                       quint16 senderPort);

    void sampleDataReady(QVector<SampleData> data);
    void eventDataReady(QVector<EventData> data, int enableSortNum, int sortedNum, double timeSpan);
    void handshakeReceived(const QHostAddress &sender, quint16 senderPort);
    void waveformDataReceived(const QVector<int> &data);

    void udpCommEstablished();
    void udpCommLost();



private slots:
    /**
     * @brief Slot triggered when there is pending data to read from the socket.
     */
    void onReadyRead();


private:
    QUdpSocket      *m_udpSocket;           ///< The UDP socket instance
    QHostAddress    m_remoteAddress;        ///< The remote address to send data
    quint16         m_remotePort;           ///< The remote port to send data
    QHostAddress    m_localAddress;         ///< The local address to bind to
    quint16         m_localPort;            ///< The local port
    QByteArray      m_receiveBuffer;        ///< A buffer to accumulate incoming data

    quint16         m_sequenceCounter;      ///< Sequence counter that increments for each frame sent
    quint16         m_sequenceValLast;      ///< Sequence value in last timer interrupt
    int             m_timerInterval;        ///< Timer interval
    int             m_commLostCounter;      ///< Counter to judge communication lost with SoC
    bool            m_connected;            ///< Communication state with SoC


    quint16         m_sequenceReceived;     ///< Sequence value received from SoC
    quint16         m_sequenceReceivedLast; ///< Sequence value received from SoC in last time
    QTimer          *m_handshakeTimer;      ///< Timer for handshake frame


    void parseHandshakeFrame(const QByteArray &data);
    void parseWaveformFrame(const QByteArray &data);
    void parseSampleData(const QByteArray &data);
    void parseEventData(const QByteArray &data);
};

#endif // UDPCOMMCLIENT_H
