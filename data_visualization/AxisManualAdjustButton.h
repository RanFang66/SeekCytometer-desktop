#ifndef AXISMANUALADJUSTBUTTON_H
#define AXISMANUALADJUSTBUTTON_H

#include <QGraphicsObject>
#include <QObject>
#include "PlotBase.h"

class AxisManualAdjustButton : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit AxisManualAdjustButton(PlotBase *plot);

    QRectF boundingRect() const override;

    void paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *w) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    PlotBase *m_plot;
    void showAxisRangeDialog();
};

#endif // AXISMANUALADJUSTBUTTON_H
