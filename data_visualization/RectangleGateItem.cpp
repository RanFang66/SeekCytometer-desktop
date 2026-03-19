#include "RectangleGateItem.h"
#include <QPainter>

RectangleGateItem::RectangleGateItem(const QPointF &startPos, PlotBase *parent)
    : GateItem{GateType::RectangleGate, startPos, parent}
{
    // Position at parent origin so all coordinates are in parent-local space
    setPos(0, 0);
    // m_startPos is set by base class to startPos (parent-local coords)
    m_previewPos = startPos;
}

RectangleGateItem::RectangleGateItem(const Gate &gate, PlotBase *parent)
    : GateItem{gate, parent}
{
    setPos(0, 0);
}

void RectangleGateItem::updateGatePreview(const QPointF &scenePoint)
{
    prepareGeometryChange();
    // Convert scene coordinate to parent-local (since pos is 0,0, this equals parent's local coords)
    m_previewPos = mapFromScene(scenePoint);
    update();
}

void RectangleGateItem::finishDrawing(const QPointF &scenePoint)
{
    m_previewPos = mapFromScene(scenePoint);

    // Convert both corner pixel positions to data values
    QPointF dataP1 = m_parent->mapPlotAreaToPoint(m_startPos);
    QPointF dataP2 = m_parent->mapPlotAreaToPoint(m_previewPos);

    // Normalize: point 0 = (min_x, min_y), point 1 = (max_x, max_y)
    QList<QPointF> points;
    points.append(QPointF(qMin(dataP1.x(), dataP2.x()), qMin(dataP1.y(), dataP2.y())));
    points.append(QPointF(qMax(dataP1.x(), dataP2.x()), qMax(dataP1.y(), dataP2.y())));
    m_gate.setPoints(points);

    m_drawingFinished = true;
    prepareGeometryChange();
    update();
}


QRectF RectangleGateItem::boundingRect() const
{
    // Always cover the full plot area so repaints are correct
    return m_parent->plotArea();
}

void RectangleGateItem::updateGateData()
{
    // Data values are set in finishDrawing; nothing additional needed
}

void RectangleGateItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->save();
    QRectF plotArea = m_parent->plotArea();

    if (m_drawingFinished) {
        // Map data coordinates back to pixel positions (re-computed each paint for axis changes)
        QPointF p1 = m_parent->mapPointToPlotArea(m_gate.points().at(0));
        QPointF p2 = m_parent->mapPointToPlotArea(m_gate.points().at(1));
        QRectF rect = QRectF(p1, p2).normalized();

        // Clamp to plot area
        rect = rect.intersected(plotArea);

        painter->setPen(QPen(m_gate.color(), 2));
        painter->drawRect(rect);
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignTop, m_gate.name());
    } else {
        // Preview: draw rectangle between start and current mouse position
        QRectF rect = QRectF(m_startPos, m_previewPos).normalized();

        // Clamp to plot area
        rect = rect.intersected(plotArea);

        painter->setPen(QPen(m_gate.color(), 2, Qt::DashDotLine));
        painter->drawRect(rect);
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignTop, m_gate.name());
    }

    painter->restore();
}
