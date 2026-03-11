#include "QuadrantGateItem.h"
#include <QPainter>

QuadrantGateItem::QuadrantGateItem(const QPointF &origin, PlotBase *parent)
    : GateItem(GateType::QuadrantGate, origin, parent), m_origin(mapFromParent(origin))
{
    setPos(parent->plotArea().topLeft());
    m_boundingRect = QRectF(0, 0, parent->plotArea().width(), parent->plotArea().height());
}

QuadrantGateItem::QuadrantGateItem(const Gate &gate, PlotBase *parent)
    : GateItem{gate, parent}
{
    m_boundingRect = m_parent->plotArea();
    QPointF p = m_parent->mapPointToPlotArea(gate.points().at(0));
    m_origin = p;
}

void QuadrantGateItem::updateGatePreview(const QPointF &origin)
{
    m_origin = mapFromScene(origin);
    prepareGeometryChange();
    update();
}

void QuadrantGateItem::finishDrawing(const QPointF &origin)
{
    updateGateData();
    prepareGeometryChange();
    setPos(0, 0);
    m_boundingRect = m_parent->plotArea();
    m_drawingFinished = true;
    update();
}

void QuadrantGateItem::updateGateData()
{
    QPointF p1 = mapToParent(m_origin);
    m_gate.setPoints({m_parent->mapPlotAreaToPoint(p1)});
}


QRectF QuadrantGateItem::boundingRect() const
{
    if (m_drawingFinished) {
        return m_parent->plotArea();
    }
    return m_boundingRect;
}

void QuadrantGateItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    QRectF plotArea = m_parent->plotArea();

    if (m_drawingFinished) {
        QPointF origin = m_parent->mapPointToPlotArea(m_gate.points().at(0));
        m_origin = origin;

        painter->setPen(QPen(Qt::red, 2));
        painter->drawLine(plotArea.left(), origin.y(), plotArea.right(), origin.y());
        painter->drawLine(origin.x(), plotArea.top(), origin.x(), plotArea.bottom());
        painter->drawEllipse(origin, 3, 3);
    } else {
        QPointF origin = mapToParent(m_origin);

        painter->setPen(QPen(Qt::blue, 2, Qt::DashDotLine));
        painter->drawLine(plotArea.left(), origin.y(), plotArea.right(), origin.y());
        painter->drawLine(origin.x(), plotArea.top(), origin.x(), plotArea.bottom());
        painter->drawEllipse(origin, 3, 3);
    }

    painter->drawText(plotArea, Qt::AlignLeft|Qt::AlignTop, m_gate.name());

    painter->restore();
}



