#include "DataManager.h"
#include "HistogramPlot.h"
#include "ScatterPlot.h"
#include <QFile>
#include <QDateTime>
#include <QDir>

DataManager::DataManager(QObject *parent)
    : QObject{parent}
{}

void DataManager::initDataManager(const QVector<DetectorSettings> &settings)
{
    m_enabledChannels.clear();
    m_measurementTypesPerChannel.clear();
    for (const DetectorSettings &setting : settings) {
        m_enabledChannels.append(setting.detectorId());
        QVector<MeasurementType> types;
        types.append(MeasurementType::Height);
        types.append(MeasurementType::Width);
        types.append(MeasurementType::Area);
        // if (setting.isEnabledHeight()) {
        //     types.append(MeasurementType::Height);
        // }
        // if (setting.isEnabledWidth()) {
        //     types.append(MeasurementType::Width);
        // }
        // if (setting.isEenabledArea()) {
        //     types.append(MeasurementType::Area);
        // }
        m_measurementTypesPerChannel.append(types);
    }
    m_sampleData.init(getEmptySampleRecord());

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
        for (int ch : m_enabledChannels) {
            int index = m_enabledChannels.indexOf(ch);
            for (MeasurementType type : m_measurementTypesPerChannel.at(index)) {
                textStream << QString("Channel-%1(%2)").arg(ch).arg(MeasurementTypeHelper::measurementTypeToString(type));
                textStream << ",";
            }
        }
        textStream << "\n";
        dataFile.close();
    }

}

SampleData DataManager::getEmptySampleRecord()
{
    SampleData data;
    data.resize(m_enabledChannels.size());
    for (int i = 0; i < data.size(); ++i) {
        data[i].resize(m_measurementTypesPerChannel.at(i).size());
    }
    return data;
}

int DataManager::getSampleRecordByteSize() const
{
    int bytes;
    for (int i = 0; i < m_enabledChannels.size(); ++i) {
        for (int j = 0; j < m_measurementTypesPerChannel.at(i).size(); ++j) {
            bytes += sizeof(int);
        }
    }
    return bytes;
}



void DataManager::addSample(const SampleData &data)
{
    m_sampleData.write(data);
}

void DataManager::addSamples(const QVector<SampleData> &data)
{
    m_sampleData.writeMultiple(data);
    saveDataToCsvFile(data);
}

const QVector<SampleData> &DataManager::getSampleData()
{
    return m_sampleData.readMultiple(m_sampleData.avaiable());
}

void DataManager::processData(const QVector<PlotBase *> &plots)
{
    if (m_sampleData.isEmpty()) return;
    QVector<SampleData> data = m_sampleData.readMultiple(m_sampleData.avaiable());

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

void DataManager::processHistogramData(PlotBase *plot, const QVector<SampleData> &data)
{
    HistogramPlot *histogramPlot = static_cast<HistogramPlot*>(plot);
    if (!histogramPlot) return;

    int channelX = histogramPlot->axisXDetectorId();
    MeasurementType xType = histogramPlot->xMeasurementType();

    int channelXIndex = m_enabledChannels.indexOf(channelX);
    if (channelXIndex < 0) return;
    int measurementXIndex = m_measurementTypesPerChannel.at(channelXIndex).indexOf(xType);
    if (measurementXIndex < 0) return;

    QVector<int> histogram(data.size(), 0);
    for (int i = 0; i < data.size(); ++i) {
        histogram[i] = data.at(i).at(channelXIndex).at(measurementXIndex);
    }
    histogramPlot->updateData(histogram);
}

void DataManager::processScatterData(PlotBase *plot, const QVector<SampleData> &data)
{
    ScatterPlot *scatterPlot = static_cast<ScatterPlot*>(plot);
    if (!scatterPlot) return;

    int channelX = scatterPlot->axisXDetectorId();
    MeasurementType xType = scatterPlot->xMeasurementType();
    int channelY = scatterPlot->axisYDetectorId();
    MeasurementType yType = scatterPlot->yMeasurementType();

    int channelXIndex = m_enabledChannels.indexOf(channelX);
    int channelYIndex = m_enabledChannels.indexOf(channelY);
    if (channelXIndex < 0 || channelYIndex < 0) return;

    int measurementXIndex = m_measurementTypesPerChannel.at(channelXIndex).indexOf(xType);
    int measurementYIndex = m_measurementTypesPerChannel.at(channelYIndex).indexOf(yType);
    if (measurementXIndex < 0 || measurementYIndex < 0) return;

    QVector<QPoint> scatterData(data.size());
    for (int i = 0; i < data.size(); ++i) {
        scatterData[i] = QPoint(data.at(i).at(channelXIndex).at(measurementXIndex),
                                 data.at(i).at(channelYIndex).at(measurementYIndex));
    }
    scatterPlot->updateData(scatterData);
}

void DataManager::processContourData(PlotBase *plot, const QVector<SampleData> &data)
{
    // To be implemented
}

void DataManager::saveDataToCsvFile(const QVector<SampleData> &updateData)
{
    QFile dataFile(m_dataSavePath);
    if (dataFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODeviceBase::Append)) {
        QTextStream dataStream(&dataFile);
        for (int i = 0; i < updateData.size(); i++) {
            SampleData data = updateData.at(i);
            for (int ch : m_enabledChannels) {
                int index = m_enabledChannels.indexOf(ch);
                for (MeasurementType type : m_measurementTypesPerChannel.at(index)) {
                    dataStream << data.at(index).at(static_cast<int>(type));
                    dataStream << ",";
                }
            }

            dataStream << "\n";
        }
        dataFile.close();
    } else {
        qDebug() << "Save Failed! Error: data file open failed!";
    }
}
