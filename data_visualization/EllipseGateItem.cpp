#include "EllipseGateItem.h"
#include <QPainter>

EllipseGateItem::EllipseGateItem(const QPointF &startPosInPlot, PlotBase *parent)
    : GateItem(GateType::EllipseGate, startPosInPlot, parent)
{
    m_ellipse = QRectF(0, 0, 0, 0);
}

EllipseGateItem::EllipseGateItem(const Gate &gate, PlotBase *parent)
    : GateItem{gate, parent}
{
    QPointF p1 = m_parent->mapPointToPlotArea(gate.points().at(0));
    QPointF p2 = m_parent->mapPointToPlotArea(gate.points().at(1));

    m_ellipse = QRectF(p1, p2);
}

void EllipseGateItem::updateGatePreview(const QPointF &point)
{
    prepareGeometryChange();
    m_previewPos = mapFromScene(point);
    m_ellipse.setWidth(m_previewPos.x());
    m_ellipse.setHeight(m_previewPos.y());
    update();
}

void EllipseGateItem::finishDrawing(const QPointF &point)
{
    updateGateData();
    prepareGeometryChange();
    setPos(0, 0);
    m_drawingFinished = true;
    update();
}

void EllipseGateItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->save();
    if (m_drawingFinished) {
        QPointF p1 = m_parent->mapPointToPlotArea(m_gate.points().at(0));
        QPointF p2 = m_parent->mapPointToPlotArea(m_gate.points().at(1));
        m_ellipse = QRectF(p1, p2).normalized();

        painter->setPen(QPen(m_gate.color(), 2));
        painter->drawEllipse(m_ellipse);
    } else {
        painter->setPen(QPen(m_gate.color(), 2, Qt::DashDotLine));
        painter->drawEllipse(m_ellipse);
    }
    painter->drawText(m_ellipse, Qt::AlignLeft|Qt::AlignTop, m_gate.name());

    painter->restore();
}

QRectF EllipseGateItem::boundingRect() const
{
    if (m_drawingFinished) {
        return m_parent->plotArea();
    }
    return m_ellipse;
}

void EllipseGateItem::updateGateData()
{
    QPointF p1 = mapToParent(m_ellipse.topLeft());
    QPointF p2 = mapToParent(m_ellipse.bottomRight());
    m_gate.setPoints({m_parent->mapPlotAreaToPoint(p1), m_parent->mapPlotAreaToPoint(p2)});
}

QPainterPath EllipseGateItem::shape() const
{
    QPainterPath path;
    if (m_drawingFinished) {
        QPointF p1 = m_parent->mapPointToPlotArea(m_gate.points().at(0));
        QPointF p2 = m_parent->mapPointToPlotArea(m_gate.points().at(1));
        path.addEllipse(QRectF(p1, p2).normalized());
    } else {
        path.addEllipse(m_ellipse);
    }
    return path;
}

