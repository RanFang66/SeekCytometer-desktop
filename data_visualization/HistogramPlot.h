#ifndef HISTOGRAMPLOT_H
#define HISTOGRAMPLOT_H

#include <QGraphicsObject>
#include <QFont>
#include <QFontMetrics>
#include "ChartBuffer.h"
#include "PlotBase.h"


class HistoBins
{
public:
    explicit HistoBins(int num = 388) {
        m_binNum = qMax(388, num);
        m_maxValue = 0;
        m_isLog = false;
        bins.resize(m_binNum);
        bins.fill(0);
    }

    int getBinValue(double xVal) {
        double mappedVal = xVal;
        if (m_isLog) {
            if (xVal <= 0) return 0;
            mappedVal = std::log10(xVal);
        }

        if (mappedVal < m_binStart || mappedVal > m_binEnd) {
            return 0;
        }

        int index = (mappedVal - m_binStart) / m_binStep;
        if (index >= 0 && index < m_binNum) {
            return bins[index];
        } else {
            return 0;
        }
    }

    int binNum() const {
        return m_binNum;
    }

    qreal binStep() const {
        return m_binStep;
    }

    qreal binStart() const {
        return m_binStart;
    }

    qreal binEnd() const {
        return m_binEnd;
    }

    // Return real-space boundaries (for setting axis range)
    qreal realBinStart() const {
        return m_isLog ? std::pow(10.0, m_binStart) : m_binStart;
    }

    qreal realBinEnd() const {
        return m_isLog ? std::pow(10.0, m_binEnd) : m_binEnd;
    }

    int maxBinVal() const {
        return m_maxValue;
    }

    bool isLog() const {
        return m_isLog;
    }

    void updateBins(int min, int max, const QList<int> &data, bool isLog = false) {
        if (min > max) return;
        m_isLog = isLog;

        int range = max - min;
        if (range < m_binNum)
            range = m_binNum;

        qreal realStart = (min + max) / 2.0 - range / 2.0;
        qreal realEnd = realStart + range;

        if (isLog) {
            if (realStart <= 0) realStart = 1;
            if (realEnd <= realStart) realEnd = realStart * 10;
            m_binStart = std::log10(realStart);
            m_binEnd = std::log10(realEnd);
        } else {
            m_binStart = realStart;
            m_binEnd = realEnd;
        }
        m_binStep = (m_binEnd - m_binStart) / m_binNum;

        bins.fill(0);
        int index = 0;
        m_maxValue = 0;
        for (const int &val : data) {
            if (isLog) {
                if (val <= 0) {
                    continue;
                }
                index = (std::log10(val) - m_binStart) / m_binStep;
            } else {
                index = (val - m_binStart) / m_binStep;
            }
            if (index < 0) index = 0;
            if (index >= m_binNum) {
                index = m_binNum - 1;
            }
            int &cnt = bins[index];
            cnt++;
            m_maxValue = qMax(m_maxValue, cnt);
        }
    }


private:
    QList<int> bins;
    qreal   m_binStart;   // in log10 space when m_isLog
    qreal   m_binEnd;     // in log10 space when m_isLog
    qreal   m_binStep;    // in log10 space when m_isLog
    int     m_binNum;
    int     m_maxValue;
    bool    m_isLog;
};


class HistogramPlot : public PlotBase
{
    Q_OBJECT
public:
    explicit HistogramPlot(const Plot &plot, QGraphicsItem *parent = nullptr);

public slots:
    void updateData(const QVector<int> &data);

protected:
    void            paintPlot(QPainter *painter) override;
    void            resetPlot() override;

    void autoAdjustAxisRange() override;
    void changeAxisType(CustomAxis::ScaleType type) override;

private:
    static constexpr int DEFAULT_DATA_LENGTH = 60000;

    HistoBins   m_bins;


    ChartBuffer<int>        m_data;
    // QList<int>              m_bins;
    int                     m_maxValue;
    int                     m_xMinVal;
    int                     m_xMaxVal;
};

#endif // HISTOGRAMPLOT_H
