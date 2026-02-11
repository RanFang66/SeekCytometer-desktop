#include "HistogramPlot.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>

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
}



static double mapValueToRatio(double val, double minVal, double maxVal, CustomAxis::ScaleType type = CustomAxis::ScaleType::Linear)
{
    if (type == CustomAxis::Linear) {
        return (val - minVal) / (maxVal - minVal);
    } else {
        double logMin = std::log10(minVal);
        double logMax = std::log10(maxVal);
        return (std::log10(val) - logMin) / (logMax - logMin);
    }
}

void HistogramPlot::updateData(const QVector<int> &data)
{
    if (data.isEmpty()) return;
    m_data.writeMultiple(data);

    m_data.getMinMax(m_xMinVal, m_xMaxVal);
    const bool isLog = m_xAxis->isLog();
    m_bins.updateBins(m_xMinVal, m_xMaxVal, m_data.readAll(), isLog);
    m_xAxis->setRange(m_bins.binStart(), m_bins.binEnd());
    m_yAxis->setRange(0.0, m_bins.maxBinVal() * 1.1);
    update();
}





void HistogramPlot::paintPlot(QPainter *painter)
{
    if (!painter) return;

    painter->save();
    painter->setPen(Qt::blue);


    for (int i = 0; i < m_plotArea.width(); i++) {
        int xVal = mapXAxisToValue(i + m_plotArea.left());

        int binVal = m_bins.getBinValue(xVal);
        if (binVal == 0) {
            continue;
        }

        qreal yTop = m_plotArea.bottom() - ((binVal - m_yAxis->minValue()) / m_yAxis->range()) * m_plotArea.height();

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
    update();
}

void HistogramPlot::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (!m_axisUnlocked) {
        event->ignore();
        return;
    }

    QPointF pos = event->pos();

    const double zoomFactor = (event->delta() > 0) ? 0.9 : 1.1;
    QPointF dataPos = mapPlotAreaToPoint(pos);

    if (m_axisXArea.contains(pos)) {
        zoomAxis(m_xAxis, dataPos.x(), zoomFactor);
    } else if (m_axisYArea.contains(pos)) {
        zoomAxis(m_yAxis, dataPos.y(), zoomFactor);
    } else {
        QGraphicsObject::wheelEvent(event);
        return;
    }

    update();
    event->accept();
}

void HistogramPlot::zoomAxis(CustomAxis *axis, double center, double factor)
{
    double min = axis->minValue();
    double max = axis->maxValue();

    double newMin = center - (center - min) * factor;
    double newMax = center + (max - center) * factor;

    if (newMax - newMin < 1e-6)
        return;

    axis->setRange(newMin, newMax);
}

void HistogramPlot::mousePressEvent(QGraphicsSceneMouseEvent *event)
{ 
    if (event->button() != Qt::LeftButton) {
        QGraphicsObject::mousePressEvent(event);
        return;
    }
    if (!m_axisUnlocked) {
        if (!m_plotArea.contains(event->pos())) {
            QGraphicsObject::mousePressEvent(event);
            return;
        }
        m_cursorValue = mapPlotAreaToPoint(event->pos());
        m_showCursorValue = true;
        update();

        QTimer::singleShot(4000, this, [this]() {
            m_showCursorValue = false;
            update();
        });

        event->accept();
        return;
    }

    QPointF pos = event->pos();
    m_rubberStartPos = pos;
    m_rubberEndPos   = pos;
    if (m_plotArea.contains(pos)) {
        m_dragMode = DragRubberBand;
    } else if (m_axisXArea.contains(pos)) {
        m_dragMode = DragX;
    } else if (m_axisYArea.contains(pos)) {
        m_dragMode = DragY;
    } else {
        m_dragMode = NoDrag;
        QGraphicsObject::mousePressEvent(event);
        return;
    }

    event->accept();
}

void HistogramPlot::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_axisUnlocked || m_dragMode == NoDrag) {
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }

    QPointF delta = event->pos() - m_rubberStartPos;
    double dValue = 0;
    switch (m_dragMode) {
    case DragX:
        dValue = delta.x() * m_xAxis->range() / m_plotArea.width();
        m_xAxis->setRange(
            m_xAxis->minValue() - dValue,
            m_xAxis->maxValue() - dValue
        );
        m_rubberStartPos = event->pos();
        break;
    case DragY:
        dValue = -delta.y() * m_yAxis->range() / m_plotArea.height();
        m_yAxis->setRange(
            m_yAxis->minValue() - dValue,
            m_yAxis->maxValue() - dValue
        );
        m_rubberStartPos = event->pos();
        break;
    case DragRubberBand:
        m_rubberEndPos = event->pos();
        break;
    default:
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }

    update();
    event->accept();
}

void HistogramPlot::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragMode != DragRubberBand || event->button() != Qt::LeftButton) {
        QGraphicsObject::mouseReleaseEvent(event);
        m_dragMode = NoDrag;
        return;
    }

    m_dragMode = NoDrag;

    QRectF rubberRect(m_rubberStartPos, m_rubberEndPos);
    rubberRect = rubberRect.normalized() & m_plotArea;

    if (rubberRect.width() < 10 || rubberRect.height() < 10) {
        update();
        return;
    }

    QPointF topRight = mapPlotAreaToPoint(rubberRect.topRight());
    QPointF bottomLeft = mapPlotAreaToPoint(rubberRect.bottomLeft());


    if (m_axisUnlocked) {
        m_xAxis->setRange(bottomLeft.x(), topRight.x());
        m_yAxis->setRange(bottomLeft.y(), topRight.y());
    }

    update();
    event->accept();
}

