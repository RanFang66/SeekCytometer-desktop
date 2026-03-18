#include "TestDataGenerator.h"
#include <QRandomGenerator>
#include "EventDataManager.h"
#include <QDateTime>

#include <random>
#include <algorithm>

// 生成一个高斯分布整数 + 噪声，并裁剪在指定范围 [minValue, maxValue]
int generateGaussianWithNoise(double mean,
                              double stddev,
                              int noiseMin,
                              int noiseMax,
                              int minValue,
                              int maxValue)
{
    // ------------------------
    // 1. 正态分布随机数生成器
    // ------------------------
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::normal_distribution<double> dist(mean, stddev);

    // 生成高斯数
    double gaussianValue = dist(rng);

    // ------------------------
    // 2. 均匀噪声
    // ------------------------
    std::uniform_int_distribution<int> noiseDist(noiseMin, noiseMax - 1);
    int noise = noiseDist(rng);

    // ------------------------
    // 3. 合成 + 取整
    // ------------------------
    int result = static_cast<int>(gaussianValue + noise);

    // ------------------------
    // 4. 裁剪到范围 [minValue, maxValue]
    // ------------------------
    result = std::clamp(result, minValue, maxValue);

    return result;
}




TestDataGenerator::TestDataGenerator(QObject *parent)
    : QObject{parent}, m_generateTimer(new QTimer(this)), m_dataCount(100), m_interval(500), m_dataMin(0), m_dataMax(32768)
{
    m_eventId = 0;
    m_generateTimer->setSingleShot(false);
    m_generateTimer->stop();
    // connect(m_generateTimer, &QTimer::timeout, this, &TestDataGenerator::generateTestData);
    connect(m_generateTimer, &QTimer::timeout, this, &TestDataGenerator::generateEventData);
}

void TestDataGenerator::configTestGenerator(int dataCount, int interval, int dataMin, int dataMax)
{
    QMutexLocker locker(&m_mutex);
    m_dataCount = dataCount;
    m_interval = interval;
    m_dataMin = dataMin;
    m_dataMax = dataMax;
    m_currentTime = 0;
}


void TestDataGenerator::startGenerateData()
{
    m_eventId = 0;
    m_generateTimer->start(m_interval);
}

void TestDataGenerator::generateTestData()
{
    QVector<SampleData> generatedData;
    for (int i = 0; i < m_dataCount; i++) {
        SampleData data = DataManager::instance().getEmptySampleRecord();
        for (int chIndex = 0; chIndex < data.size(); ++chIndex) {
            for (int mIndex = 0; mIndex < data.at(chIndex).size(); ++mIndex) {
                data[chIndex][mIndex] = QRandomGenerator::global()->bounded(m_dataMin, m_dataMax);
            }
        }
        generatedData.append(data);
    }
    emit testDataGenerated(generatedData);
}


void TestDataGenerator::generateEventData()
{
    QVector<EventData> eventData;
    int enableSortNum = 0;
    int sortedNum = 0;
    int distTime = 0;
    int timeBuff = 0;
    int validCh = 0;
    for (int ch : EventDataManager::instance().enabledChannels()) {
        validCh |= (1 << ch);
    }
    for (int count = 0; count < m_dataCount; count++) {
        m_eventId++;
        EventData event(EventDataManager::instance().enabledChannels());
        event.setEventId(m_eventId);
        int val = QRandomGenerator::global()->bounded(0, 100);
        if (val > 30) {
            event.setEnableSort(true);
            enableSortNum++;
            if (val > 45) {
                event.setSorted(true);
                sortedNum++;
            }
        }
        int timeSpan = QRandomGenerator::global()->bounded(100, 300);
        int distTime =  QRandomGenerator::global()->bounded(30, 90);
        m_currentTime += timeSpan;
        timeBuff += distTime;
        event.setPostTimeUs(m_currentTime);
        event.setDiffTimeUs(distTime);
        event.setValidSpeedMeasure(true);
        event.setValidChPulse(validCh);

        double mean = (m_dataMax  + m_dataMin)  / 2;
        double stddev = (m_dataMax - m_dataMin) / 6;
        int noiseLimit = mean / 10;
        for (int ch : EventDataManager::instance().enabledChannels()) {
            for (MeasurementType type : MeasurementTypeHelper::measurementTypeList()) {
                event.setData(ch, type, generateGaussianWithNoise(mean, stddev, -noiseLimit, noiseLimit, m_dataMin, m_dataMax));
            }
        }
        eventData.append(event);
    }

    emit eventDataGenerated(eventData, enableSortNum, sortedNum, (double)timeBuff / m_dataCount);
}

void TestDataGenerator::stopGenerateData()
{
    m_generateTimer->stop();
}

void TestDataGenerator::resetGenerator()
{
    QMutexLocker locker(&m_mutex);
    m_dataCount = 100;
    m_interval = 500;
    m_dataMin = 0;
    m_dataMax = 32768;
    m_currentTime = 0;
}


