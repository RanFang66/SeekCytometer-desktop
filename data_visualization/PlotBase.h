#ifndef PLOTBASE_H
#define PLOTBASE_H

#include <QGraphicsObject>
#include "CustomAxis.h"
#include "Plot.h"
#include "Gate.h"
// #include "AxisLockButtonItem.h"

class PlotBase : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit PlotBase(const Plot &plot, QGraphicsItem *parent = nullptr);

    void setBoundingRect(const QRectF &rect);
    void setTitle(const QString &title);
    void setTitleFont(const QFont &font);
    void setAxisXName(const QString &name);
    void setAxisYName(const QString &name);

    virtual void resetPlot() = 0;

    int worksheetId() const { return m_plot.workSheetId();}
    int plotId() const {return m_plot.id();}
    QString title() const {return m_title;}
    QString axisXName() const {return m_xAxis->axisName();}
    QString axisYName() const {return m_yAxis->axisName();}
    int axisXSettingId() const {return m_plot.axisXId();}
    int axisYSettingId() const {return m_plot.axisYId();}
    PlotType plotType() const {return m_plot.plotType();}
    int axisXDetectorId() const {return m_plot.axisXDetectorId();}
    int axisYDetectorId() const {return m_plot.axisYDetectorId();}
    MeasurementType xMeasurementType() const {return m_plot.xMeasurementType();}
    MeasurementType yMeasurementType() const {return m_plot.yMeasurementType();}
    QRectF  plotArea() const {return m_plotArea;}

    qreal   mapValueToXAixs(qreal value) const;
    qreal   mapValueToYAixs(qreal value) const;
    qreal   mapXAxisToValue(qreal value) const;
    qreal   mapYAxisToValue(qreal value) const;

    QPointF mapPointToPlotArea(const QPointF &point) const;
    QPointF mapPointToPlotArea(qreal x, qreal y) const;
    QPointF mapPlotAreaToPoint(const QPointF &point) const;

    QRectF  mapRectToPlotArea(const QRectF &rect) const;
    QRectF  mapRectToAxis(const QRectF &rect) const;

    bool    isInPlotArea(const QPointF &point) const;
    QPointF limitPointInPlot(const QPointF &point) const;
    QPointF limitScenePointInPlot(const QPointF &point) const;

    virtual void autoAdjustAxisRange() = 0;

    void updateAxisRange(int xMin, int xMax, int yMin, int yMax);

    void updateAxisRanges(const Gate &gate);

    CustomAxis::ScaleType AxisType() const {
        return m_xAxis->scaleType();
    }

    CustomAxis* xAxis() const { return m_xAxis; }
    CustomAxis* yAxis() const { return m_yAxis; }

    void setAxisUnlocked(bool unlocked);

    bool isAxisUnlocked() const;

    virtual void changeAxisType(CustomAxis::ScaleType type) = 0;

    void saveToImage();

signals:
    void deleteRequested(PlotBase *plot);

protected:

    enum AxisDragMode {
        NoDrag,
        DragX,
        DragY,
        DragRubberBand,
    };

    // QPointF m_lastDragPos;
    AxisDragMode m_dragMode = NoDrag;

    QPointF m_rubberStartPos;
    QPointF m_rubberEndPos;
    void updateLayout();
    void paintTitle(QPainter *painter);;
    void paintAxis(QPainter *painter);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    virtual void paintPlot(QPainter *painter) = 0;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void drawCursorValue(QPainter *painter);

    // void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    // void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    // void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    Plot        m_plot;
    CustomAxis *m_xAxis;
    CustomAxis *m_yAxis;
    QString m_title;
    QRectF m_boundingRect;
    QRectF m_plotArea;
    QRectF m_axisXArea;
    QRectF m_axisYArea;
    QRectF m_titleArea;

    QFont m_titleFont;

    bool m_axisUnlocked;
    bool    m_showCursorValue;
    QPointF m_cursorValue;

    static constexpr qreal PLOT_MARGIN = 10.0;
    static constexpr qreal TITLE_MARGIN = 10.0;
};




#endif // PLOTBASE_H
