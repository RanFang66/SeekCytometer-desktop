#include "EventDataManager.h"
#include "HistogramPlot.h"
#include "ScatterPlot.h"
#include <QFile>
#include <QDir>


EventDataManager::EventDataManager(QObject *parent)
    : QObject{parent}
{
    qRegisterMetaType<EventData>("EventData");
    qRegisterMetaType<QList<EventData>>("QList<EventData>");
    qRegisterMetaType<QList<EventData>*>("QList<EventData>*");
}

void EventDataManager::processHistogramData(PlotBase *plot, const QVector<EventData> &data)
{
    HistogramPlot *histogramPlot = static_cast<HistogramPlot*>(plot);
    if (!histogramPlot) return;

    int channelX = histogramPlot->axisXDetectorId();
    MeasurementType xType = histogramPlot->xMeasurementType();


    QVector<int> histogram;
    for (const EventData &aData : data) {
        if (aData.isValidChPulse(channelX)) {
            histogram.append(aData.getData(channelX, xType));
        }
    }
    histogramPlot->updateData(histogram);
}

void EventDataManager::processScatterData(PlotBase *plot, const QVector<EventData> &data)
{
    ScatterPlot *scatterPlot = static_cast<ScatterPlot*>(plot);
    if (!scatterPlot) return;

    int channelX = scatterPlot->axisXDetectorId();
    MeasurementType xType = scatterPlot->xMeasurementType();
    int channelY = scatterPlot->axisYDetectorId();
    MeasurementType yType = scatterPlot->yMeasurementType();


    QVector<QPoint> scatterData(data.size());
    for (int i = 0; i < data.size(); ++i) {
        scatterData[i] = QPoint(data.at(i).getData(channelX, xType),
                                 data.at(i).getData(channelY, yType));
    }
    scatterPlot->updateData(scatterData);
}

void EventDataManager::processContourData(PlotBase *plot, const QVector<EventData> &data)
{
    // To be implemented

}



void EventDataManager::saveEventToCsvFile(const QVector<EventData> &updateData)
{
    QFile dataFile(m_dataSavePath);
    if (!dataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        qDebug() << "Save Failed! Error: data file open failed!";
        return;
    }

    QTextStream dataStream(&dataFile);
    for (const EventData &data : updateData) {
        dataStream << data.getEventId() << ",";
        dataStream << data.isValidSpeedMeasure() << ',';
        dataStream << data.getPostTimeUs() << ",";
        dataStream << data.getDiffTimeUs() << ",";
        dataStream << (data.isEnabledSort() ? "true" : "false") << ",";
        dataStream << (data.isRealSorted() ? "true" : "false") << ",";
        dataStream << QString::number(data.validChPulse())<<",";
        for (int ch : data.getEnabledChannels()) {
            for (MeasurementType type : MeasurementTypeHelper::measurementTypeList()) {
                dataStream << data.getData(ch, type) << ",";
            }
        }

        dataStream << "\n";
    }
}

void EventDataManager::initEventDataManager(const QVector<DetectorSettings> &settings)
{
    m_processedEvent = 0;
    m_enableSortEvent = 0;
    m_sortedEvent = 0;
    m_discardEvent = 0;

    m_enabledChannels.clear();
    for (const DetectorSettings &setting : settings) {
        m_enabledChannels.append(setting.detectorId());
    }
    m_eventData.init(EventData(m_enabledChannels));


    m_dataSavePath = QString("./SeekCytometerData/pulse_data_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
    QDir saveDir(m_dataSavePath);
    if (!saveDir.exists()) {
        if (saveDir.mkpath(".")) {
            qDebug() << "Created directory " << m_dataSavePath << " ok";
        } else {
            qDebug() << "Created directory " << m_dataSavePath << " failed!";
            m_dataSavePath = ".";
        }
    }
    m_dataSavePath = saveDir.absoluteFilePath("pulsedata.csv");
    QFile dataFile(m_dataSavePath);
    if (dataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODeviceBase::Append)) {
        QTextStream textStream(&dataFile);
        textStream << "Event ID";
        textStream << ",";
        textStream << "Valid Speed";
        textStream << ",";
        textStream << "Start Time(s)";
        textStream << ",";
        textStream << QString("Speed (us for %1um)").arg(m_speedMeasureDist);
        textStream << ",";
        textStream << "Sort Triggered";
        textStream << ",";
        textStream << "Sorted";
        textStream << ",";
        textStream << "Pulse Ch";
        textStream << ",";
        for (int ch : m_enabledChannels) {
            for (MeasurementType type : MeasurementTypeHelper::measurementTypeList()) {
                textStream << QString("Channel-%1(%2)").arg(ch).arg(MeasurementTypeHelper::measurementTypeToString(type));
                textStream << ",";
            }
        }
        textStream << "\n";
        dataFile.close();
    }
}

void EventDataManager::addEvent(const EventData &data)
{
    m_processedEvent++;
    if (data.isEnabledSort()) m_enableSortEvent++;
    if (data.isRealSorted()) m_sortedEvent++;
    if (data.isEnabledSort() && !data.isRealSorted()) m_discardEvent++;

    m_eventData.write(data);
}

void EventDataManager::addEvents(const QVector<EventData> data, int enableSortNum, int sortedNum, double timeSpan)
{
    m_processedEvent += data.size();
    m_enableSortEvent += enableSortNum;
    m_sortedEvent += sortedNum;
    m_discardEvent += (enableSortNum - sortedNum);
    m_speedMeasureTimeSpan = timeSpan;
    m_speedMeasured = m_speedMeasureDist / m_speedMeasureTimeSpan;
    m_eventData.writeMultiple(data);
    saveEventToCsvFile(data);
}

const QVector<EventData> &EventDataManager::getEventData()
{
    return m_eventData.readMultiple(m_eventData.avaiable());
}

void EventDataManager::processData(const QVector<PlotBase *> &plots)
{
    if (m_eventData.isEmpty()) return;
    QVector<EventData> data = m_eventData.readMultiple(m_eventData.avaiable());

    for (PlotBase *plot : plots) {
        PlotType plotType = plot->plotType();
        switch (plotType) {
        case PlotType::HISTOGRAM_PLOT:
            processHistogramData(plot, data);
            break;
        case PlotType::SCATTER_PLOT:
            processScatterData(plot, data);
            break;
        case PlotType::CONTOUR_PLOT:
            processContourData(plot, data);
            break;
        default:
            break;
        }
    }
}



