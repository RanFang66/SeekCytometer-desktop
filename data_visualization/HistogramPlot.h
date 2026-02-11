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
        bins.resize(m_binNum);
        bins.fill(0);
    }

    int getBinValue(double xVal) {
        if (xVal < m_binStart || xVal > m_binEnd) {
            return 0;
        }

        int index = 0;
        index = (xVal - m_binStart) / m_binStep;
        if (index >0 && index < m_binNum) {
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

    int maxBinVal() const {
        return m_maxValue;
    }



    void updateBins(int min, int max, const QList<int> &data, bool isLog = false) {
        if (min > max) return;
        int range = max - min;
        if (range < m_binNum)
            range = m_binNum;

        m_binStart = (min + max) / 2 - range / 2;
        m_binEnd = m_binStart + range;


        if (isLog) {
            m_binStart = std::log10(m_binStart);
            m_binEnd = std::log10(m_binEnd);
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
            if (index >= m_binNum) {
                index = m_binNum-1;
            }
            int &cnt = bins[index];
            cnt++;
            m_maxValue = qMax(m_maxValue, cnt);
        }
    }


private:
    QList<int> bins;
    qreal   m_binStart;
    qreal   m_binEnd;
    qreal   m_binStep;
    int     m_binNum;
    int     m_maxValue;
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

    void wheelEvent(QGraphicsSceneWheelEvent *event) override;
    void zoomAxis(CustomAxis *axis, double center, double factor);

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    static constexpr int DEFAULT_DATA_LENGTH = 60000;

    AxisDragMode m_dragMode = NoDrag;

    HistoBins   m_bins;


    ChartBuffer<int>        m_data;
    // QList<int>              m_bins;
    int                     m_maxValue;
    int                     m_xMinVal;
    int                     m_xMaxVal;
};

#endif // HISTOGRAMPLOT_H
