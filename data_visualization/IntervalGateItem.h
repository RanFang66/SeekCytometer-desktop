#ifndef INTERVALGATEITEM_H
#define INTERVALGATEITEM_H

#include "GateItem.h"

class IntervalGateItem : public GateItem
{
    Q_OBJECT
public:
    IntervalGateItem(const QPointF &startPointInPlot, PlotBase *parent = nullptr);
    IntervalGateItem(const Gate &gate, PlotBase *parent = nullptr);

    void updateGatePreview(const QPointF &point) override;
    void finishDrawing(const QPointF &point) override;
    QRectF boundingRect() const override;
    GateType gateType() const override { return GateType::IntervalGate; }
    void updateGateData() override;
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif // INTERVALGATEITEM_H
