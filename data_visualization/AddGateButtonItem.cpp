#include "AddGateButtonItem.h"
#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include "PlotBase.h"
#include "WorkSheetScene.h"

AddGateButtonItem::AddGateButtonItem(GateType gateType, PlotBase *plot)
    : QGraphicsObject(plot), m_gateType(gateType), m_plot(plot)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setCursor(Qt::PointingHandCursor);
    setToolTip(label());
}

QRectF AddGateButtonItem::boundingRect() const
{
    return QRectF(0, 0, 20, 20);
}

QString AddGateButtonItem::label() const
{
    switch (m_gateType) {
    case GateType::IntervalGate:  return "Interval Gate";
    case GateType::RectangleGate: return "Rectangle Gate";
    case GateType::PolygonGate:   return "Polygon Gate";
    case GateType::EllipseGate:   return "Ellipse Gate";
    case GateType::QuadrantGate:  return "Quadrant Gate";
    default:                      return "";
    }
}

bool AddGateButtonItem::isActive() const
{
    WorkSheetScene *wsScene = qobject_cast<WorkSheetScene*>(scene());
    if (!wsScene)
        return false;
    return wsScene->isDrawingGate()
           && wsScene->drawingGateType() == m_gateType
           && wsScene->activePlot() == m_plot;
}

void AddGateButtonItem::paintGateIcon(QPainter *p, const QRectF &rect) const
{
    const qreal m = 4; // margin inside icon
    QRectF inner = rect.adjusted(m, m, -m, -m);

    switch (m_gateType) {
    case GateType::IntervalGate: {
        // |-|  two vertical lines + horizontal connector
        qreal midY = inner.center().y();
        p->drawLine(QPointF(inner.left(), inner.top()), QPointF(inner.left(), inner.bottom()));
        p->drawLine(QPointF(inner.right(), inner.top()), QPointF(inner.right(), inner.bottom()));
        p->drawLine(QPointF(inner.left(), midY), QPointF(inner.right(), midY));
        break;
    }
    case GateType::RectangleGate:
        p->drawRect(inner);
        break;
    case GateType::PolygonGate: {
        // triangle
        QPolygonF tri;
        tri << QPointF(inner.center().x(), inner.top())
            << inner.bottomLeft()
            << inner.bottomRight();
        p->drawPolygon(tri);
        break;
    }
    case GateType::EllipseGate:
        p->drawEllipse(inner);
        break;
    case GateType::QuadrantGate: {
        // cross +
        p->drawLine(QPointF(inner.center().x(), inner.top()),
                     QPointF(inner.center().x(), inner.bottom()));
        p->drawLine(QPointF(inner.left(), inner.center().y()),
                     QPointF(inner.right(), inner.center().y()));
        break;
    }
    default:
        break;
    }
}

void AddGateButtonItem::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *w)
{
    Q_UNUSED(option);
    Q_UNUSED(w);

    p->setRenderHint(QPainter::Antialiasing);

    QRectF rect = boundingRect();

    // Background
    if (isActive()) {
        p->setBrush(QColor(100, 150, 255, 180));
    } else {
        p->setBrush(QColor(220, 220, 220, 180));
    }
    p->setPen(QPen(Qt::gray, 1));
    p->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), 3, 3);

    // Icon
    p->setPen(QPen(Qt::black, 1.5));
    p->setBrush(Qt::NoBrush);
    paintGateIcon(p, rect);
}

void AddGateButtonItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    WorkSheetScene *wsScene = qobject_cast<WorkSheetScene*>(scene());
    if (wsScene) {
        if (isActive()) {
            // Toggle off: cancel drawing
            wsScene->cancelDrawingGate();
        } else if (!wsScene->isDrawingGate()) {
            wsScene->startDrawingGateOnPlot(m_gateType, m_plot);
        }
    }
    update();
    event->accept();
}
