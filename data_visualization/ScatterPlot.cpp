#include "ScatterPlot.h"
#include <QMarginsF>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneMouseEvent>

#include <QTimer>

ScatterPlot::ScatterPlot(const Plot &plot, QGraphicsItem *parent)
    : PlotBase(plot, parent), m_data(DEFAULT_DATA_LENGTH)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | ItemClipsToShape);

    m_xAxis->setRange(1000, 10000);
    m_yAxis->setRange(1000, 10000);

    m_xAxis->setTicks(5);
    m_yAxis->setTicks(5);

    m_xAxis->setAxisName(plot.axisXName());
    m_yAxis->setAxisName(plot.axisYName());
}


void ScatterPlot::updateData(const QVector<QPoint> &data)
{
    if (data.isEmpty()) return;
    m_data.writeMultiple(data);
    m_xMin = m_xAxis->minValue();
    m_xMax = m_xAxis->maxValue();
    m_yMin = m_yAxis->minValue();
    m_yMax = m_yAxis->maxValue();
    QRectF plotRange(QPointF(m_xMin, m_yMax), QPointF(m_xMax, m_yMin));

    QPoint topRight, bottomLeft;
    m_data.getMinMax(bottomLeft, topRight);
    bool needUpdate = false;
    if (!plotRange.contains(topRight)) {
        m_xMax = topRight.x();
        m_yMax = topRight.y();
        needUpdate = true;
    }
    if (!plotRange.contains(bottomLeft)) {
        m_xMin = bottomLeft.x();
        m_yMin = bottomLeft.y();
        needUpdate = true;
    }


    if (needUpdate) {
        m_xAxis->setRange(m_xMin * 0.9, m_xMax * 1.1);
        m_yAxis->setRange(m_yMin * 0.9, m_yMax * 1.1);
    }

    update();
}


void ScatterPlot::paintPlot(QPainter *painter)
{
    if (!painter) return;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::blue);

    const bool xLog = (m_xAxis->scaleType() == CustomAxis::Logarithmic);
    const bool yLog = (m_yAxis->scaleType() == CustomAxis::Logarithmic);

    for (const QPoint &point : m_data.readAll()) {

        if ((xLog && point.x() <= 0) ||
            (yLog && point.y() <= 0))
            continue;


        QPointF pDraw = mapPointToPlotArea(point);
        if (!m_plotArea.contains(pDraw)) {
            continue;
        }
        painter->drawPoint(mapPointToPlotArea(point));
    }

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

void ScatterPlot::resetPlot()
{
    m_data.clear();
}

void ScatterPlot::autoAdjustAxisRange()
{
    QPoint topRight, bottomLeft;
    m_data.getMinMax(bottomLeft, topRight);
    m_xAxis->setRange(bottomLeft.x(), topRight.x());
    m_yAxis->setRange(bottomLeft.y(), topRight.y());
}

void ScatterPlot::changeAxisType(CustomAxis::ScaleType type)
{
    m_xAxis->setScaleType(type);
    m_yAxis->setScaleType(type);
    update();
}

void ScatterPlot::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (!m_axisUnlocked || !m_plotArea.contains(event->pos())) {
        event->ignore();
        return;
    }

    QPointF pos = event->pos();


    const double zoomFactor = (event->delta() > 0) ? 0.9 : 1.1;
    QPointF dataPos = mapPlotAreaToPoint(pos);


    zoomAxis(m_xAxis, dataPos.x(), zoomFactor);
    zoomAxis(m_yAxis, dataPos.y(), zoomFactor);

    update();
    event->accept();
}

void ScatterPlot::zoomAxis(CustomAxis *axis, double center, double factor)
{
    double min = axis->minValue();
    double max = axis->maxValue();

    double newMin = center - (center - min) * factor;
    double newMax = center + (max - center) * factor;

    if (newMax - newMin < 1)
        return;

    axis->setRange(newMin, newMax);
}

void ScatterPlot::mousePressEvent(QGraphicsSceneMouseEvent *event)
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

void ScatterPlot::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
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

void ScatterPlot::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragMode != DragRubberBand || event->button() != Qt::LeftButton) {
        QGraphicsObject::mouseReleaseEvent(event);
        m_dragMode = NoDrag;
        return;
    }

    m_dragMode = NoDrag;

    QRectF rubberRect(m_rubberStartPos, m_rubberEndPos);
    rubberRect = rubberRect.normalized() & m_plotArea;

    if (rubberRect.width() < 5 || rubberRect.height() < 5) {
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

