#include "IntervalGateItem.h"
#include <QPainter>

IntervalGateItem::IntervalGateItem(const QPointF &startPosInPlot, PlotBase *parent)
    : GateItem(GateType::IntervalGate, startPosInPlot, parent)
{
    setPos(QPointF(startPosInPlot.x(), parent->plotArea().top()));
    m_boundingRect = QRectF(0, 0, 0, parent->plotArea().height());
}

IntervalGateItem::IntervalGateItem(const Gate &gate, PlotBase *parent)
    : GateItem{gate, parent}
{
    QPointF p1 = m_parent->mapPointToPlotArea(gate.points().at(0));
    QPointF p2 = m_parent->mapPointToPlotArea(gate.points().at(1));

    m_boundingRect = QRectF(p1, p2);
}

void IntervalGateItem::updateGatePreview(const QPointF &point)
{
    prepareGeometryChange();
    m_previewPos = mapFromScene(point);
    m_boundingRect.setWidth(m_previewPos.x());
    update();
}

void IntervalGateItem::finishDrawing(const QPointF &point)
{
    updateGateData();
    prepareGeometryChange();
    setPos(0, 0);
    m_drawingFinished = true;
    update();
}


void IntervalGateItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->save();
    if (m_drawingFinished) {
        QPointF p1 = m_parent->mapPointToPlotArea(m_gate.points().at(0));
        QPointF p2 = m_parent->mapPointToPlotArea(m_gate.points().at(1));
        m_boundingRect = QRectF(p1, p2).normalized();

        painter->setPen(QPen(Qt::red, 2));
        painter->drawLine(m_boundingRect.left(), m_boundingRect.center().y(), m_boundingRect.right(), m_boundingRect.center().y());
        painter->drawLine(m_boundingRect.topLeft(), m_boundingRect.bottomLeft());
        painter->drawLine(m_boundingRect.topRight(), m_boundingRect.bottomRight());
    } else {
        painter->setPen(QPen(Qt::blue, 2, Qt::DashDotLine));
        painter->drawLine(m_boundingRect.left(), m_boundingRect.center().y(), m_boundingRect.right(), m_boundingRect.center().y());
        painter->drawLine(m_boundingRect.topLeft(), m_boundingRect.bottomLeft());
        painter->drawLine(m_boundingRect.topRight(), m_boundingRect.bottomRight());
    }
    painter->drawText(m_boundingRect, Qt::AlignLeft|Qt::AlignTop, m_gate.name());

    painter->restore();
}

QRectF IntervalGateItem::boundingRect() const
{
    if (m_drawingFinished) {
        return m_parent->plotArea();
    }
    return m_boundingRect;
}

void IntervalGateItem::updateGateData()
{
    QPointF p1 = mapToParent(m_boundingRect.topLeft());
    QPointF p2 = mapToParent(m_boundingRect.bottomRight());
    m_gate.setPoints({m_parent->mapPlotAreaToPoint(p1), m_parent->mapPlotAreaToPoint(p2)});
}
