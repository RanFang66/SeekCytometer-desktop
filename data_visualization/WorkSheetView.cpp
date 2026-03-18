#include "WorkSheetView.h"
#include "HistogramPlot.h"
#include "ScatterPlot.h"
#include <QRandomGenerator>
WorkSheetView::WorkSheetView(const WorkSheet &workSheet, QWidget *parent)
    : QGraphicsView{parent}, m_workSheet{workSheet}
{
    initWorkSheetView();
}


void WorkSheetView::initWorkSheetView()
{
    m_scene = new WorkSheetScene(this);
    m_scene->setBackgroundBrush(Qt::white);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setMouseTracking(true);
}
