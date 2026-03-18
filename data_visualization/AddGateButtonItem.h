#ifndef ADDGATEBUTTONITEM_H
#define ADDGATEBUTTONITEM_H

#include <QGraphicsObject>
#include "Gate.h"

class PlotBase;
class WorkSheetScene;

class AddGateButtonItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit AddGateButtonItem(GateType gateType, PlotBase *plot);

    QRectF boundingRect() const override;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *w) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    GateType  m_gateType;
    PlotBase *m_plot;

    QString label() const;
    void    paintGateIcon(QPainter *p, const QRectF &rect) const;
    bool    isActive() const;
};

#endif // ADDGATEBUTTONITEM_H
