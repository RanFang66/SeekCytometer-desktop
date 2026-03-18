#include "ScatterPlot.h"
#include <QMarginsF>
#include "AddGateButtonItem.h"

ScatterPlot::ScatterPlot(const Plot &plot, QGraphicsItem *parent)
    : PlotBase(plot, parent), m_data(DEFAULT_DATA_LENGTH)
{
    m_xAxis->setRange(0, 10000);
    m_yAxis->setRange(0, 10000);

    m_xAxis->setTicks(5);
    m_yAxis->setTicks(5);

    m_xAxis->setAxisName(plot.axisXName());
    m_yAxis->setAxisName(plot.axisYName());

    // Gate buttons: scatter plot supports rectangle, polygon, ellipse, quadrant gates
    qreal btnX = m_boundingRect.left() + 10;
    qreal btnY = m_boundingRect.top() + 5;
    qreal btnSpacing = 24;

    auto *rectBtn = new AddGateButtonItem(GateType::RectangleGate, this);
    rectBtn->setPos(btnX, btnY);

    auto *polyBtn = new AddGateButtonItem(GateType::PolygonGate, this);
    polyBtn->setPos(btnX + btnSpacing, btnY);

    auto *ellipseBtn = new AddGateButtonItem(GateType::EllipseGate, this);
    ellipseBtn->setPos(btnX + btnSpacing * 2, btnY);

    auto *quadBtn = new AddGateButtonItem(GateType::QuadrantGate, this);
    quadBtn->setPos(btnX + btnSpacing * 3, btnY);
}


void ScatterPlot::updateData(const QVector<QPoint> &data)
{
    if (data.isEmpty()) return;
    m_data.writeMultiple(data);

    if (!m_axisUnlocked) {
        QPoint bottomLeft, topRight;
        if (m_data.getMinMax(bottomLeft, topRight)) {
            qreal dataXMin = bottomLeft.x();
            qreal dataXMax = topRight.x();
            qreal dataYMin = bottomLeft.y();
            qreal dataYMax = topRight.y();

            qreal xPadding = (dataXMax - dataXMin) * 0.05;
            qreal yPadding = (dataYMax - dataYMin) * 0.05;
            m_xAxis->setRange(dataXMin - xPadding, dataXMax + xPadding);
            m_yAxis->setRange(dataYMin - yPadding, dataYMax + yPadding);
        }
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

