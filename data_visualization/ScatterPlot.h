#ifndef SCATTERPLOT_H
#define SCATTERPLOT_H

#include <QGraphicsObject>
#include <QPainter>
#include "PlotBase.h"
#include <QFont>
#include <QFontMetrics>
#include "ChartBuffer.h"

#include <QPoint>

struct PointMinMax {
    QPoint min(const QPoint& a, const QPoint& b) const {
        QPoint p;
        p.setX(a.x() > b.x() ? b.x() : a.x());
        p.setY(a.y() > b.y() ? b.y() : a.y());
        return p;
    }

    QPoint max(const QPoint& a, const QPoint& b) const {
        QPoint p;
        p.setX(a.x() < b.x() ? b.x() : a.x());
        p.setY(a.y() < b.y() ? b.y() : a.y());
        return p;
    }

    bool equal(const QPoint &a, const QPoint &b) const {
        return (a.x() == b.x()) || (a.y() == b.y());
    }
};

class ScatterPlot : public PlotBase
{
    Q_OBJECT
public:
    ScatterPlot(const Plot &plot, QGraphicsItem *parent = nullptr);

public slots:
    void updateData(const QVector<QPoint> &data);

protected:
    void            paintPlot(QPainter *painter) override;
    void            resetPlot() override;
    void autoAdjustAxisRange() override;
    void changeAxisType(CustomAxis::ScaleType type) override;

private:
    ChartBuffer<QPoint, PointMinMax>     m_data;
    static constexpr int    DEFAULT_DATA_LENGTH = 60000;
};


#endif // SCATTERPLOT_H
