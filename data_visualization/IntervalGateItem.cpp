#include "IntervalGateItem.h"
#include <QPainter>

IntervalGateItem::IntervalGateItem(const QPointF &startPosInPlot, PlotBase *parent)
    : GateItem(GateType::IntervalGate, startPosInPlot, parent)
{
    // Position at parent origin so all coordinates are in parent-local space
    setPos(0, 0);
    // m_startPos is set by base class to startPosInPlot (parent-local coords)
    m_previewPos = startPosInPlot;
}

IntervalGateItem::IntervalGateItem(const Gate &gate, PlotBase *parent)
    : GateItem{gate, parent}
{
    setPos(0, 0);
}

void IntervalGateItem::updateGatePreview(const QPointF &scenePoint)
{
    prepareGeometryChange();
    // Convert scene coordinate to parent-local (since pos is 0,0, mapFromScene == parent's mapFromScene)
    m_previewPos = mapFromScene(scenePoint);
    update();
}

void IntervalGateItem::finishDrawing(const QPointF &scenePoint)
{
    m_previewPos = mapFromScene(scenePoint);

    // Convert the two x-pixel positions to data values
    qreal x1 = m_parent->mapXAxisToValue(m_startPos.x());
    qreal x2 = m_parent->mapXAxisToValue(m_previewPos.x());

    // For interval gate, only x range matters; store y as 0
    QList<QPointF> points;
    points.append(QPointF(qMin(x1, x2), 0));
    points.append(QPointF(qMax(x1, x2), 0));
    m_gate.setPoints(points);

    m_drawingFinished = true;
    prepareGeometryChange();
    update();
}

void IntervalGateItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->save();
    QRectF plotArea = m_parent->plotArea();
    qreal top = plotArea.top();
    qreal bottom = plotArea.bottom();
    qreal midY = (top + bottom) / 2.0;

    if (m_drawingFinished) {
        // Map data x-values back to pixel positions (re-computed each paint for axis changes)
        qreal left = m_parent->mapValueToXAixs(m_gate.points().at(0).x());
        qreal right = m_parent->mapValueToXAixs(m_gate.points().at(1).x());
        if (left > right) qSwap(left, right);

        // Clamp to plot area
        left = qMax(left, plotArea.left());
        right = qMin(right, plotArea.right());

        painter->setPen(QPen(m_gate.color(), 2));
        painter->drawLine(QPointF(left, top), QPointF(left, bottom));
        painter->drawLine(QPointF(right, top), QPointF(right, bottom));
        painter->drawLine(QPointF(left, midY), QPointF(right, midY));
        painter->drawText(QRectF(left, top, right - left, bottom - top),
                          Qt::AlignLeft | Qt::AlignTop, m_gate.name());
    } else {
        // Preview: draw between start and current mouse position
        qreal left = qMin(m_startPos.x(), m_previewPos.x());
        qreal right = qMax(m_startPos.x(), m_previewPos.x());

        // Clamp to plot area
        left = qMax(left, plotArea.left());
        right = qMin(right, plotArea.right());

        painter->setPen(QPen(m_gate.color(), 2, Qt::DashDotLine));
        painter->drawLine(QPointF(left, top), QPointF(left, bottom));
        painter->drawLine(QPointF(right, top), QPointF(right, bottom));
        painter->drawLine(QPointF(left, midY), QPointF(right, midY));
        painter->drawText(QRectF(left, top, right - left, bottom - top),
                          Qt::AlignLeft | Qt::AlignTop, m_gate.name());
    }

    painter->restore();
}

QRectF IntervalGateItem::boundingRect() const
{
    // Always cover the full plot area so repaints are correct
    return m_parent->plotArea();
}

void IntervalGateItem::updateGateData()
{
    // Data values are set in finishDrawing; nothing additional needed
}
