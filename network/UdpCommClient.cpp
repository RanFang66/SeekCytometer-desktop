#include "UdpCommClient.h"
#include "DataManager.h"
#include "DetectorSettingsModel.h"
#include "EventDataManager.h"

UdpCommClient::UdpCommClient(QObject *parent)
    : QObject{parent}, m_udpSocket{new QUdpSocket(this)}, m_remotePort(0),
    m_sequenceCounter(0), m_sequenceValLast(0), m_sequenceReceived(0), m_sequenceReceivedLast(0),
    m_timerInterval(2000), m_commLostCounter(0), m_connected(false)
{
    qRegisterMetaType<EventData>("EventData");
    qRegisterMetaType<QList<EventData>>("QList<EventData>");
    qRegisterMetaType<QList<EventData>*>("QList<EventData>*");
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &UdpCommClient::onReadyRead);

    m_handshakeTimer = new QTimer();
    m_handshakeTimer->setInterval(2000);
    m_handshakeTimer->stop();

    connect(m_handshakeTimer, &QTimer::timeout, this, &UdpCommClient::onHandshakeTimerTimeout);

    m_remoteAddress = QHostAddress("192.168.8.10");
    m_remotePort = 5001;
    m_localAddress = QHostAddress("192.168.8.35");
    m_localPort = 8080;

}

void UdpCommClient::startUdpClient()
{
#if ENABLE_DEBUG
    QTimer::singleShot(2000, this, [this]() {
        m_connected = true;
        emit udpCommEstablished();
    });
#else
    m_handshakeTimer->start();
#endif
}


void UdpCommClient::doInitialize()
{
    bool bindOk = m_udpSocket->bind(m_localAddress, m_localPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!bindOk) {
        qWarning() << "[UdpCommClient] Failed to bind on"
                   << m_localAddress.toString() << ":" << m_localPort
                   << m_udpSocket->errorString();
    } else {
        qDebug() << "[UdpCommClient] Successfully bound on"
                 << m_localAddress.toString() << ":" << m_localPort;
    }
}


bool UdpCommClient::sendFrame(CommCmdType commandType, const QByteArray &data)
{
    QByteArray frame = UdpCommFrame::packFrame(++m_sequenceCounter, commandType, data);
    qint64 bytesSent = m_udpSocket->writeDatagram(frame, m_remoteAddress, m_remotePort);
    if (bytesSent != frame.size()) {
        qWarning() << "[UdpCommClient] Failed to send frame!"
                   << m_udpSocket->errorString();
        return false;
    }
    return true;
}

bool UdpCommClient::sendHandshake()
{
    return sendFrame(CommCmdType::CMD_HAND_SHAKE, QByteArray());
}

bool UdpCommClient::sendWaveformRequest(bool enabled, int enabledChannels, int interval)
{
    QByteArray data;

    // quint8 enableCh = 0;
    // for (int val : enabledChannels) {
    //     enableCh |=  (0x01 << (val - 1));
    // }

    data.append(static_cast<char>(enabled));
    data.append(static_cast<char>(enabledChannels & 0x00ff));
    data.append(static_cast<char>(interval & 0x00ff));

    return sendFrame(CommCmdType::CMD_WAVEFORM_DATA, data);
}

bool UdpCommClient::sendAcquireStart()
{
    return sendFrame(CommCmdType::CMD_ACQUIRE_START, QByteArray());
}

bool UdpCommClient::sendAcquireStop()
{
    return sendFrame(CommCmdType::CMD_ACQUIRE_STOP, QByteArray());
}

bool UdpCommClient::sendSortingStart()
{
    return sendFrame(CommCmdType::CMD_SORTING_START, QByteArray());
}

bool UdpCommClient::sendSortingStop()
{
    return sendFrame(CommCmdType::CMD_SORTING_STOP, QByteArray());
}

bool UdpCommClient::sendDetectorSettings(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                         const QList<int> &roles)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    for (const DetectorSettings &setting : DetectorSettingsModel::instance()->detectorSettings()) {
        stream << (char)setting.detectorId();
        stream << (char)setting.isEnabledHeight();
        stream << (char)setting.isEnabledWidth();
        stream << (char)setting.isEenabledArea();
        stream << (char)setting.isEnabledThreshold();
        stream << setting.thresholdValue();
    }
    if (!sendFrame(CommCmdType::CMD_DETECTOR_SETTINGS, data)) {
        return false;
    } else {
        qDebug() << "[UdpCommClient] Sent detector settings to remote.";
        return true;
    }
}

bool UdpCommClient::sendGateData(const Gate &gate)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    int detectorX = gate.xAxisDetectorId();
    int detectorY = gate.yAxisDetectorId();
    stream << (char)gate.gateType();
    stream << (char)(detectorX & 0x00ff);
    stream << (char)(detectorY & 0x00ff);
    stream << (char)gate.xMeasurementType();
    stream << (char)gate.yMeasurementType();
    for (const QPoint &point : gate.points()) {
        stream << point.x();
        stream << point.y();
    }
    if (!sendFrame(CommCmdType::CMD_GATE_SETTINGS, data)) {
        return false;
    } else {
        qDebug() << "[UdpCommClient] Sent gate settings to remote.";
        return true;
    }
}

bool UdpCommClient::sendSpeedMeasureSetting(int preId, int postId, int preThresh, int dist, int maxTimeSpan)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << (char)(preId & 0x00FF);
    stream << (char)(postId & 0x00FF);
    stream << preThresh;
    stream << dist;
    stream << maxTimeSpan;
    if (!sendFrame(CommCmdType::CMD_SPEED_MEASURE_SETTINGS, data)) {
        return false;
    } else {
        qDebug() << "[UdpCommClient] Sent speed measure settings to remote.";
        return true;
    }
}

bool UdpCommClient::sendDriveParameters(int type, int delay, int width, int coolingTime, int coe)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << (char)type;
    stream << delay;
    stream << width;
    stream << coolingTime;
    stream << coe;
    if (!sendFrame(CommCmdType::CMD_DRIVE_SETTINGS, data)) {
        return false;
    } else {
        qDebug() << "[UdpCommClient] Sent drive parameters to remote.";
        return true;
    }
}

bool UdpCommClient::sendDisableDetector(int id)
{
    QByteArray data;
    data.append(static_cast<char>(id));
    if (!sendFrame(CommCmdType::CMD_DISABLE_DETECTOR, data)) {
        return false;
    } else {
        qDebug() << QString("[UdpCommClient] Disable detector %1 to remote.").arg(id);
        return true;
    }
}

void UdpCommClient::onHandshakeTimerTimeout()
{
    /*
     * If there is no frame send to SoC between this timer interval, then send handshake frame
     */
    if (m_sequenceValLast == m_sequenceCounter) {
        sendHandshake();
    }
    m_sequenceValLast = m_sequenceCounter;

    if (m_connected) {
        if (m_sequenceReceivedLast == m_sequenceReceived) {
            m_commLostCounter += m_timerInterval;
            if (m_commLostCounter > 10000) {
                emit udpCommLost();
                m_connected = false;
                m_commLostCounter = 0;
            }
        } else {
            m_commLostCounter = 0;
        }
    } else {
        if (m_sequenceReceivedLast != m_sequenceReceived) {
            emit udpCommEstablished();
            m_connected = true;
        }
    }

    m_sequenceReceivedLast = m_sequenceReceived;
}



void UdpCommClient::onReadyRead()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort = 0;
        m_udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        m_receiveBuffer.append(datagram);

        // Try to parse frames in a loop, because there may be multiple frames or partial frames
        while (true) {
            QByteArray oneFrame;
            if (!UdpCommFrame::tryParseFrame(m_receiveBuffer, oneFrame)) {
                break;
            }
            // We got a complete, valid frame
            quint16 sequence = UdpCommFrame::getSequence(oneFrame);
            m_sequenceReceived = sequence;

            CommCmdType cmdType = UdpCommFrame::getCommandType(oneFrame);
            QByteArray dataField = UdpCommFrame::getDataField(oneFrame);

            // Emit signal to notify listeners
            emit frameReceived(sequence, cmdType, dataField, sender, senderPort);
            switch (cmdType) {
                case CommCmdType::CMD_HAND_SHAKE:
                    parseHandshakeFrame(dataField);
                    break;
                case CommCmdType::CMD_PULSE_DATA:
                    // parseSampleData(dataField);
                    parseEventData(dataField);
                    break;
                case CommCmdType::CMD_WAVEFORM_DATA:
                    parseWaveformFrame(dataField);
                    break;
                default:
                    break;
            }
        }
    }
}

void UdpCommClient::parseHandshakeFrame(const QByteArray &data)
{
    QDataStream stream(data);
    quint8 state;
    stream >> state;
    //qDebug() << "[UdpCommClient] Handshake frame received " << "Current Cytometer state:" << state;
}



/*
 * Event Frame: {Head Magic(0x55AA55AA) | Event Id with Sort State | Pre Time | Post Time
 * | (Peak | Width | Area) * (Enable Channel Num) | Tail Magic(0xAA55AA55)}
*/
void UdpCommClient::parseEventData(const QByteArray &data)
{
    int channelSize = EventDataManager::instance().enabledChannels().size();
    int eventSize = channelSize * 3 + 5;
    int eventByteSize = eventSize * 4;
    int eventNum = data.size() / eventByteSize;

    if (eventNum < 1) {
        qWarning() << QString("Received Event Data Frame is too short, expected atleast %d bytes, but %d byte actually").arg(eventByteSize).arg(data.size());
        return;
    }


    QVector<EventData> eventDataBuffer;
    int enableSortNum = 0;
    int sortedNum = 0;
    int timeSpanBuff = 0;
    int validEventNum = 0;

    for (int i = 0; i < eventNum; i++) {
        EventData eventData(EventDataManager::instance().enabledChannels(), data.mid(i * eventByteSize, eventByteSize));
        if (!eventData.isValidEvent()) {
            continue;
        }
        validEventNum++;
        if (eventData.isEnabledSort()) {
            enableSortNum++;
        }
        if (eventData.isRealSorted()) {
            sortedNum++;
        }

        timeSpanBuff += eventData.getDiffTimeUs();
        eventDataBuffer.append(eventData);
    }




    if (eventNum != validEventNum) {
        qDebug() << "Received " << eventNum << " Events Data" << validEventNum << "Valid" << data.first(4) << data.last(4);
    }

    // QDataStream stream(data);

    // while (!stream.atEnd()) {
    //     EventData eventData(EventDataManager::instance().enabledChannels(), stream);
    //     eventDataBuffer.append(eventData);
    // }

    if (validEventNum == 0) {
        return;
    }
    double timeSpan = (double)timeSpanBuff / validEventNum;
    emit eventDataReady(eventDataBuffer, enableSortNum, sortedNum, timeSpan);
}

void UdpCommClient::parseSampleData(const QByteArray &data)
{


    QVector<SampleData> sampleData;
    QDataStream stream(data);

    int eventCount = 0;

    quint32 header;
    quint32 eventId;
    quint32 eventFlag;
    quint32 magicWord;
    while (!stream.atEnd()) {
        stream >> header;
        eventId  = header & 0x00FFFFFF;
        eventFlag = (header >> 24) & 0x00FF;

        SampleData oneSample = DataManager::instance().getEmptySampleRecord();
        for (int j = 0; j < oneSample.size(); ++j) {
            for (int k = 0; k < oneSample.at(j).size(); ++k) {
                stream >> oneSample[j][k];
            }
        }

        stream >> magicWord;
        sampleData.append(oneSample);
    }
    emit sampleDataReady(sampleData);

    // QVector<SampleData> sampleData;
    // QDataStream stream(data);
    // quint16 numSamples = 0;
    // if (stream.atEnd()) {
    //     qWarning() << "[UdpCommClient] No sample data in the frame!";
    //     return;
    // }

    // stream >> numSamples;
    // // Check data length is correct
    // int expectedDataLength = numSamples * DataManager::instance().getSampleRecordByteSize();
    // if (data.size() != expectedDataLength + sizeof(quint16)) {
    //     qWarning() << "[UdpCommClient] Incorrect sample data length!";
    //     return;
    // }

    // sampleData.reserve(numSamples);
    // for (int i = 0; i < numSamples; ++i) {
    //     SampleData oneSample = DataManager::instance().getEmptySampleRecord();
    //     for (int j = 0; j < oneSample.size(); ++j) {
    //         for (int k = 0; k < oneSample.at(j).size(); ++k) {
    //             stream >> oneSample[j][k];
    //         }
    //     }
    //     sampleData.push_back(oneSample);
    // }
    // emit sampleDataReady(sampleData);
}


void UdpCommClient::parseWaveformFrame(const QByteArray &data)
{
    // QVector<QVector<int>> waveform;
    // QDataStream stream(data);
    // int numWaveforms = 0;
    // quint16 enabledChannels = 0;
    // stream >> numWaveforms;
    // stream >> enabledChannels;
    // QList<int> enabledChannelsList;
    // for (int i = 0; i < 8; ++i) {
    //     if (enabledChannels & (1 << i)) {
    //         enabledChannelsList.push_back(i);
    //     }
    // }
    // waveform.resize(numWaveforms);
    // for (int i = 0; i < numWaveforms; i++) {
    //     waveform[i].resize(enabledChannelsList.size());
    // }
    // int expectedDataLength = numWaveforms * enabledChannelsList.size() * sizeof(int);
    // if (data.size() != expectedDataLength + sizeof(int) + sizeof(quint16)) {
    //     qWarning() << "[UdpCommClient] Incorrect waveform data length!";
    //     return;
    // }

    // for (int i = 0; i < numWaveforms; i++) {
    //     for (int j = 0; j < enabledChannelsList.size(); j++) {
    //         stream >> waveform[i][j];
    //     }
    // }
    QVector<int> waveform;
    QDataStream stream(data);
    while (!stream.atEnd()) {
        int value;
        stream >> value;
        waveform.append(value);
    }

    emit waveformDataReceived(waveform);
}
