#include "HistogramPlot.h"

#include <QPainter>
#include "AddGateButtonItem.h"

HistogramPlot::HistogramPlot(const Plot &plot, QGraphicsItem *parent)
    : PlotBase(plot, parent), m_data(DEFAULT_DATA_LENGTH)
{
    m_xAxis->setRange(0, 10000);
    m_yAxis->setRange(0, 100);

    m_xAxis->setTicks(5);
    m_yAxis->setTicks(5);

    m_xAxis->setAxisName(plot.axisXName());
    m_yAxis->setAxisName("Count");

    m_bins = HistoBins(m_plotArea.width());

    // Gate button: histogram only supports interval gate
    auto *intervalBtn = new AddGateButtonItem(GateType::IntervalGate, this);
    intervalBtn->setPos(m_boundingRect.left() + 10, m_boundingRect.top() + 5);
}



void HistogramPlot::updateData(const QVector<int> &data)
{
    if (data.isEmpty()) return;
    m_data.writeMultiple(data);

    m_data.getMinMax(m_xMinVal, m_xMaxVal);
    const bool isLog = m_xAxis->isLog();
    m_bins.updateBins(m_xMinVal, m_xMaxVal, m_data.readAll(), isLog);

    if (!m_axisUnlocked) {
        m_xAxis->setRange(m_bins.realBinStart(), m_bins.realBinEnd());
        m_yAxis->setRange(0.0, m_bins.maxBinVal() * 1.1);
    }
    update();
}





void HistogramPlot::paintPlot(QPainter *painter)
{
    if (!painter) return;

    painter->save();
    painter->setPen(Qt::blue);


    for (int i = 0; i < m_plotArea.width(); i++) {
        double xVal = mapXAxisToValue(i + m_plotArea.left());

        int binVal = m_bins.getBinValue(xVal);
        if (binVal == 0) {
            continue;
        }

        qreal yTop = mapValueToYAixs(binVal);

        if (yTop >= m_plotArea.bottom()) {
            continue;
        } else if (yTop < m_plotArea.top()) {
            yTop = m_plotArea.top();
        }

        qreal x = m_plotArea.left() + i;
        painter->drawLine(
            QPointF(x, m_plotArea.bottom()),
            QPointF(x, yTop)
            );

    }


    // ---------- 框选 ----------
    if (m_dragMode == DragRubberBand) {
        QRectF rect(m_rubberStartPos, m_rubberEndPos);
        rect = rect.normalized();

        QPen pen(Qt::DashLine);
        pen.setColor(Qt::darkGray);
        painter->setPen(pen);
        painter->setBrush(QColor(100, 100, 255, 40));

        painter->drawRect(rect);
    }

    painter->restore();
}

void HistogramPlot::resetPlot()
{
    m_data.clear();
}

void HistogramPlot::autoAdjustAxisRange()
{
    m_data.getMinMax(m_xMinVal, m_xMaxVal);
    m_xAxis->setRange(m_xMinVal, m_xMaxVal);
    m_yAxis->setRange(0, m_bins.maxBinVal() * 1.1);
    update();
}

void HistogramPlot::changeAxisType(CustomAxis::ScaleType type)
{
    m_xAxis->setScaleType(type);
    if (!m_data.isEmpty()) {
        const bool isLog = m_xAxis->isLog();
        m_bins.updateBins(m_xMinVal, m_xMaxVal, m_data.readAll(), isLog);
        m_xAxis->setRange(m_bins.realBinStart(), m_bins.realBinEnd());
        m_yAxis->setRange(0.0, m_bins.maxBinVal() * 1.1);
    }
    update();
}

