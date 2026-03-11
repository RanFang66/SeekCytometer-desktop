#include "RectangleGateItem.h"
#include <QPainter>

RectangleGateItem::RectangleGateItem(const QPointF &startPos, PlotBase *parent)
    : GateItem{GateType::RectangleGate, startPos, parent}
{
    m_rectangle = QRectF(0, 0, 0, 0);
}

RectangleGateItem::RectangleGateItem(const Gate &gate, PlotBase *parent)
    : GateItem{gate, parent}
{
    QPointF p1 = m_parent->mapPointToPlotArea(gate.points().at(0));
    QPointF p2 = m_parent->mapPointToPlotArea(gate.points().at(1));

    m_rectangle = QRectF(p1, p2).normalized();
    setFlags(ItemIsSelectable);
}

void RectangleGateItem::updateGatePreview(const QPointF &point)
{
    prepareGeometryChange();
    m_previewPos = mapFromScene(point);
    m_rectangle.setWidth(m_previewPos.x());
    m_rectangle.setHeight(m_previewPos.y());
    update();
}

void RectangleGateItem::finishDrawing(const QPointF &point)
{
    updateGateData();
    prepareGeometryChange();
    setPos(0, 0);
    m_drawingFinished = true;
    update();
}


QRectF RectangleGateItem::boundingRect() const
{
    if (m_drawingFinished) {
        return m_parent->plotArea();
    }
    return m_rectangle;
}

void RectangleGateItem::updateGateData()
{
    QList<QPointF> points;
    QPointF p1 = mapToParent(m_rectangle.topLeft());
    QPointF p2 = mapToParent(m_rectangle.bottomRight());
    points.append(m_parent->mapPlotAreaToPoint(p1));
    points.append(m_parent->mapPlotAreaToPoint(p2));
    m_gate.setPoints(points);
}

void RectangleGateItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->save();

    if (m_drawingFinished) {
        QPointF p1 = m_parent->mapPointToPlotArea(m_gate.points().at(0));
        QPointF p2 = m_parent->mapPointToPlotArea(m_gate.points().at(1));
        m_rectangle = QRectF(p1, p2).normalized();

        painter->setPen(QPen(Qt::red, 2));
        painter->drawRect(m_rectangle);
    } else {
        painter->setPen(QPen(Qt::blue, 2, Qt::DashDotLine));
        painter->drawRect(m_rectangle);
    }

    painter->drawText(m_rectangle, Qt::AlignLeft|Qt::AlignTop, m_gate.name());

    painter->restore();
}
