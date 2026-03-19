#include "WorkSheetScene.h"
#include <QGraphicsSceneMouseEvent>
#include "GateItemFactory.h"
#include "HistogramPlot.h"
#include "ScatterPlot.h"
#include "PlotsDAO.h"
#include "GatesDAO.h"
#include <QMessageBox>
#include "WorkSheetWidget.h"


WorkSheetScene::WorkSheetScene(QObject *parent)
    : QGraphicsScene{parent}, m_drawState{DrawingState::DrawingIdle}, m_gateType(GateType::UnknownGate), m_activePlot{nullptr}, m_gateItem{nullptr}
{}

void WorkSheetScene::startDrawingGateOnPlot(GateType gateType, PlotBase *plot)
{
    if (m_drawState != DrawingState::DrawingIdle || !plot)
        return;

    m_drawState = DrawingState::DrawingStarted;
    m_gateType = gateType;
    m_activePlot = plot;
    m_activePlot->setFlag(QGraphicsItem::ItemIsMovable, false);
}

void WorkSheetScene::cancelDrawingGate()
{
    if (m_drawState == DrawingState::DrawingInProgress && m_gateItem) {
        // Remove the preview gate item
        delete m_gateItem;
        m_gateItem = nullptr;
    }
    if (m_activePlot) {
        m_activePlot->setFlag(QGraphicsItem::ItemIsMovable, true);
    }
    m_gateItem = nullptr;
    m_activePlot = nullptr;
    m_startPosInPlot = QPointF();
    m_gateType = GateType::UnknownGate;
    m_drawState = DrawingState::DrawingIdle;
    update();
}

void WorkSheetScene::finishDrawingGate(bool ok)
{
    if (!ok && m_gateItem) {
        delete m_gateItem;
        QMessageBox::warning(WorkSheetWidget::instance(), tr("Insert Gate Failed!"), tr("Insert Gate Failed"));
    } else {
        m_gateItems.append(m_gateItem);
        QMessageBox::information(WorkSheetWidget::instance(), tr("Insert Gate Ok!"), tr("Insert Gate Ok"));
    }
    m_gateItem = nullptr;
    m_activePlot = nullptr;
    m_startPosInPlot = QPointF();
    m_gateType = GateType::UnknownGate;
    m_drawState = DrawingState::DrawingIdle;
}

PlotBase *WorkSheetScene::addNewPlot(PlotType plotType, const Plot &plot)
{
    qDebug() << "Add New Plot, type: " << Plot::plotTypeToString(plotType)
    << "X Axis: " << plot.axisXDetectorId() << MeasurementTypeHelper::measurementTypeToString(plot.xMeasurementType())
    << "Y Axis: " << plot.axisYDetectorId() << MeasurementTypeHelper::measurementTypeToString(plot.yMeasurementType());
    HistogramPlot *histogramPlot = nullptr;
    ScatterPlot *scatterPlot = nullptr;
    PlotBase *plotBase = nullptr;
    switch (plotType) {
    case PlotType::HISTOGRAM_PLOT:
        histogramPlot = new HistogramPlot(plot);
        if (histogramPlot) {
            plotBase = (histogramPlot);
            addItem(histogramPlot);
            m_plots.append(static_cast<PlotBase*>(histogramPlot));
        }
        break;
    case PlotType::SCATTER_PLOT:
        scatterPlot = new ScatterPlot(plot);
        if (scatterPlot) {
            plotBase = scatterPlot;
            addItem(scatterPlot);
            m_plots.append(static_cast<PlotBase*>(scatterPlot));
        }
        break;
    case PlotType::CONTOUR_PLOT:
        break;
    default:
        break;
    }
    m_plots.last()->setPos(((m_plots.size()-1) % 3) * Plot::defaultPlotSize, ((m_plots.size()-1) / 3) * Plot::defaultPlotSize);
    update();
    return plotBase;
}

void WorkSheetScene::addNewGate(GateType gateType, const Gate &gate, PlotBase *parent)
{
    GateItem *gateItem = GateItemFactory::createGateItem(gateType, gate, parent);
    qDebug() << "Add New Gate, type: " << Gate::gateTypeToString(gateType)
             << gate.pointsString();

    m_gateItems.append(gateItem);
    update();
}

void WorkSheetScene::resetPlots()
{
    for (PlotBase *plot : m_plots) {
        plot->resetPlot();
    }
}


void WorkSheetScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    switch (m_drawState) {
    case DrawingState::DrawingStarted: {
        // First click after button: active plot is already known
        QPointF localPos = m_activePlot->mapFromScene(event->scenePos());
        if (m_activePlot->isInPlotArea(localPos)) {
            m_drawState = DrawingState::DrawingInProgress;
            m_startPosInPlot = m_activePlot->limitPointInPlot(localPos);
            m_gateItem = GateItemFactory::createGateItem(m_gateType, m_startPosInPlot, m_activePlot);
            m_gateItem->setGateName(QString("P%1").arg(m_gateItems.size()+1));
            event->accept();
            return;
        }
        // Click outside the active plot's area - cancel drawing and fall through
        cancelDrawingGate();
        break;
    }
    case DrawingState::DrawingInProgress: {
        // Second click: finish drawing (or add vertex for polygon)
        if (m_gateType == GateType::PolygonGate) {
            PolygonGateItem *polygonGateItem = qgraphicsitem_cast<PolygonGateItem*>(m_gateItem);
            if (polygonGateItem) {
                QPointF pos = m_activePlot->limitScenePointInPlot(event->scenePos());
                polygonGateItem->updatePolygon(pos);
            }
        } else {
            // Finish drawing for interval/rectangle/ellipse/quadrant gates
            QPointF pos = m_activePlot->limitScenePointInPlot(event->scenePos());
            m_gateItem->finishDrawing(pos);
            m_activePlot->setFlag(QGraphicsItem::ItemIsMovable, true);
            m_drawState = DrawingState::DrawingFinished;
            emit finishedDrawingGate(m_gateItem);
        }
        event->accept();
        return;
    }
    case DrawingState::DrawingFinished:
    default:
        break;
    }
    QGraphicsScene::mousePressEvent(event);
}

void WorkSheetScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_drawState == DrawingState::DrawingInProgress && m_activePlot && m_gateItem) {
        QPointF pos = m_activePlot->limitScenePointInPlot(event->scenePos());
        m_gateItem->updateGatePreview(pos);
        event->accept();
        return;
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void WorkSheetScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseReleaseEvent(event);
}

void WorkSheetScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_drawState == DrawingState::DrawingInProgress && m_gateType == GateType::PolygonGate) {
        PolygonGateItem *polygonGateItem = qgraphicsitem_cast<PolygonGateItem*>(m_gateItem);
        if (polygonGateItem) {
            QPointF pos = m_activePlot->limitScenePointInPlot(event->scenePos());
            polygonGateItem->finishDrawing(pos);
            m_activePlot->setFlag(QGraphicsItem::ItemIsMovable, true);
            m_drawState = DrawingState::DrawingFinished;
            emit finishedDrawingGate(m_gateItem);
        }
    }
    QGraphicsScene::mouseDoubleClickEvent(event);
}

void WorkSheetScene::onDeletePlot(PlotBase *plot)
{
    QMessageBox::StandardButton ret = QMessageBox::question(WorkSheetWidget::instance(), tr("Delete Plot"), tr("Confirm to delelte this Plot?"));

    if (ret == QMessageBox::Yes) {
        removeItem(plot);
        m_plots.removeOne(plot);
        PlotsDAO().deletePlot(plot->plotId());
        plot->deleteLater();
    }
}

void WorkSheetScene::onDeleteGate(GateItem *gate)
{
    QMessageBox::StandardButton ret = QMessageBox::question(WorkSheetWidget::instance(), tr("Delete Gate"), tr("Confirm to delelte this Gate?"));
    if (ret == QMessageBox::Yes) {
        removeItem(gate);
        m_gateItems.removeOne(gate);
        GatesModel::instance()->removeGate(gate->gate());
        gate->deleteLater();
    }
}


void WorkSheetScene::removeGateItem(int gateId)
{
    for (int i = 0; i < m_gateItems.size(); ++i) {
        if (m_gateItems[i]->getGateId() == gateId) {
            GateItem *item = m_gateItems.takeAt(i);
            removeItem(item);
            item->deleteLater();
            break;
        }
    }
}

bool WorkSheetScene::segmentsIntersect(const QPointF &p1, const QPointF &p2, const QPointF &q1, const QPointF &q2)
{
    auto orientation = [](const QPointF &a, const QPointF &b, const QPointF &c) {
        double cross = (b.x() - a.x()) * (c.y() - a.y()) - (b.y() - a.y()) * (c.x() - a.x());
        if (qFuzzyCompare(cross, 0.0)) return 0;
        return (cross>0) ? 1 : -1;
    };

    int o1 = orientation(p1, p2, q1);
    int o2 = orientation(p1, p2, q2);
    int o3 = orientation(q1, q2, p1);
    int o4 = orientation(q1, q2, p2);
    if (o1 != o2 && o3 != o4) return true;
    return false;
}

