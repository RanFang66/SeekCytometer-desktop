#include "PolygonGateItem.h"
#include <QPainter>

PolygonGateItem::PolygonGateItem(const QPointF &startPoint, PlotBase *parent)
    : GateItem(GateType::PolygonGate, startPoint, parent)
{
    m_polygon << QPointF(0, 0);
}

PolygonGateItem::PolygonGateItem(const Gate &gate, PlotBase *parent)
    : GateItem{gate, parent}
{
    // QPointF p1 = m_parent->mapPointToPlotArea(gate.points().at(0));
    // QPointF p2 = m_parent->mapPointToPlotArea(gate.points().at(1));

    // m_boundingRect = QRectF(p1, p2);
    for (const QPointF &p : gate.points()) {
        QPointF point = m_parent->mapPointToPlotArea(p);
        m_polygon << point;
    }
}

void PolygonGateItem::updatePolygon(const QPointF &point)
{
    prepareGeometryChange();
    m_previewPos = mapFromScene(point);
    m_polygon << mapFromScene(point);
    update();
}

void PolygonGateItem::updateGatePreview(const QPointF &point)
{
    prepareGeometryChange();
    m_previewPos = mapFromScene(point);
    update();
}

void PolygonGateItem::finishDrawing(const QPointF &point)
{
    m_polygon << mapFromScene(point);
    updateGateData();
    prepareGeometryChange();
    setPos(0, 0);
    m_drawingFinished = true;
    update();
}

void PolygonGateItem::updateGateData()
{
    QList<QPointF> points;
    for (const QPointF &point : m_polygon) {
        QPointF p = mapToParent(point);
        points.append(m_parent->mapPlotAreaToPoint(p));
    }
    m_gate.setPoints(points);
}



QRectF PolygonGateItem::boundingRect() const
{
    if (m_drawingFinished) {
        return m_parent->plotArea();
    }

    QPolygonF polygonTmp = m_polygon;
    polygonTmp << m_previewPos;
    QRectF rect = polygonTmp.boundingRect();
    qreal margin = 1;
    rect.adjust(-margin, -margin, margin, margin);
    return rect;
}

void PolygonGateItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    if (m_drawingFinished) {
        m_polygon.clear();
        for (const QPointF &p : m_gate.points()) {
            m_polygon << m_parent->mapPointToPlotArea(p);
        }

        painter->setPen(QPen(Qt::red, 2));
        painter->drawPolygon(m_polygon);
        painter->drawText(m_polygon.boundingRect(), Qt::AlignLeft|Qt::AlignTop, m_gate.name());
    } else {
        painter->setPen(QPen(Qt::blue, 2, Qt::DashDotLine));
        painter->drawPolyline(m_polygon);
        painter->drawLine(m_polygon.last(), m_previewPos);

        QPolygonF tmp = m_polygon;
        tmp << m_previewPos;
        painter->drawText(tmp.boundingRect(), Qt::AlignLeft|Qt::AlignTop, m_gate.name());
    }

    painter->restore();
}
