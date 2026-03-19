#ifndef GATEITEM_H
#define GATEITEM_H

#include <QGraphicsObject>
#include "Gate.h"

#include "PlotBase.h"



class GateItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit GateItem(GateType type, const QPointF &m_startPos, PlotBase *parent = nullptr);
    explicit GateItem(const Gate &gate, PlotBase *parent = nullptr);

    const Gate  &gate() const { return m_gate; }
    PlotBase    *parentPlot() const { return m_parent; }
    QString getGateName() const {return m_gate.name(); }
    int     getGateId() const {return m_gate.id();}
    void    setGateName(const QString &name) { m_gate.setName(name); }
    void    setGateId(int id) { m_gate.setId(id);}
    void    setGateColor(const QColor &color) { m_gate.setColor(color); }
    QColor  getGateColor() const { return m_gate.color(); }

    virtual void        updateGatePreview(const QPointF &point) = 0;
    virtual void        finishDrawing(const QPointF &point) = 0;
    virtual GateType    gateType() const = 0;
    virtual void        updateGateData() = 0;

signals:
    void gateDeleteRequested(GateItem *item);

protected:
    Gate        m_gate;
    QPointF     m_startPos;
    QPointF     m_previewPos;
    PlotBase    *m_parent;
    bool        m_drawingFinished;

    // void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
};


#endif // GATEITEM_H
