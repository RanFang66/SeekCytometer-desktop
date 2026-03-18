#include "WorkSheetWidget.h"
#include <QToolBar>
#include <QActionGroup>
#include <QVBoxLayout>
#include "WorkSheetsDAO.h"
#include "AddNewPlotDialog.h"
#include "PlotsDAO.h"
// #include "DataManager.h"
// #include "GatesDAO.h"
#include <QInputDialog>
#include "EventDataManager.h"
#include "GatesModel.h"
#include <QSplitter>

WorkSheetWidget::WorkSheetWidget(const QString &title, QWidget *parent)
    : QDockWidget{title, parent},
    m_updateTimer(new QTimer(this)),
    m_active(false),
    m_updateInterval(1000),
    tableView(new QTableView(this)),
    m_model(GatesModel::instance())
{
    initDockWidget();
}

void WorkSheetWidget::setActive(bool active)
{
    setActive(active, m_updateInterval);
}

void WorkSheetWidget::setActive(bool active, int interval)
{
    m_active = active;
    m_updateInterval = interval;
    if (m_active) {
        m_updateTimer->start(m_updateInterval);
    } else {
        m_updateTimer->stop();
    }
}

bool WorkSheetWidget::isActive() const
{
    return m_active;
}

void WorkSheetWidget::resetPlots()
{
    currentWorkSheetScene->resetPlots();
}




void WorkSheetWidget::addWorkSheetView(int worksheetId)
{
    if (m_activedWorksheetId.contains(worksheetId))
        return;



    // SortingWidget::instance()->updatePopulation(worksheetId);
    m_activedWorksheetId.append(worksheetId);
    WorkSheet workSheet = WorkSheetsDAO().fetchWorkSheet(worksheetId);
    WorkSheetView *workSheetView = new WorkSheetView(workSheet);
    tabWidget->addTab(workSheetView, workSheet.name());
    currentWorkSheetView = workSheetView;
    currentWorkSheetScene = workSheetView->scene();
    connect(currentWorkSheetScene, &WorkSheetScene::finishedDrawingGate, this, &WorkSheetWidget::onFinishedDrawingGate);
    m_model->resetGateModel(worksheetId);

    // for (const Gate& gate : GatesDAO().fetchGates(worksheetId)) {
    //     GatesDAO().deleteGate(gate.id());
    // }

    QList<Plot> plotList = PlotsDAO().fetchPlots(worksheetId);
    // QList<Gate> gateList = GatesDAO().fetchGates(worksheetId);


    for (const Plot &plot : plotList) {
        PlotBase *plotBase = workSheetView->scene()->addNewPlot(plot.plotType(), plot);
        if (plotBase) {
            for (const Gate &gate : m_model->getGateList()) {
                if (gate.xAxisSettingId() == plot.axisXId() && gate.yAxisSettingId() == plot.axisYId()
                    && gate.xMeasurementType() == plot.xMeasurementType() && gate.yMeasurementType() == plot.yMeasurementType()) {
                    plotBase->updateAxisRanges(gate);
                    workSheetView->scene()->addNewGate(gate.gateType(), gate, plotBase);
                }
            }
        }
    }
}

void WorkSheetWidget::onFinishedDrawingGate(GateItem *gateItem)
{
    if (!currentWorkSheetScene) {
        return;
    }
    bool ok;
    bool addGateOk = false;
    QString gateName = QInputDialog::getText(this, tr("Add New Gate"), tr("Please input gate name and confirm to add this new gate"), QLineEdit::Normal, gateItem->gate().name(), &ok);
    if (ok && !gateName.isEmpty()) {
        gateItem->setGateName(gateName);
        // int gateId = GatesDAO().insertGate(gateItem->gate());
        // if (gateId > 0) {
        //     gateItem->setGateId(gateId);
        //     addGateOk = true;
        // }

        int gateId = m_model->addGate(gateItem->gate());
        if (gateId > 0) {
            gateItem->setGateId(gateId);
            addGateOk = true;
        }
    }

    currentWorkSheetScene->finishDrawingGate(addGateOk);
}

void WorkSheetWidget::initDockWidget()
{
    QWidget *mainWidget = new QWidget(this);

    tableView->setModel(m_model);
    // tableView->setEditTriggers(QAbstractItemView::DoubleClicked);

    QToolBar *toolBar = new QToolBar("ToolBar", mainWidget);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    actionPrint = new QAction("Print", this);
    actionSavePDF = new QAction("Save PDF", this);
    actionSelect = new QAction("Select", this);

    toolBar->addAction(actionPrint);
    toolBar->addAction(actionSavePDF);
    toolBar->addAction(actionSelect);
    toolBar->addSeparator();

    QActionGroup *plotGroup = new QActionGroup(this);
    actionNewHistogram = new QAction("New Histogram", this);
    actionNewScatter = new QAction("New Scatter", this);
    actionNewContour = new QAction("New Contour", this);
    actionNewHistogram->setData(QVariant::fromValue(PlotType::HISTOGRAM_PLOT));
    actionNewScatter->setData(QVariant::fromValue(PlotType::SCATTER_PLOT));
    actionNewContour->setData(QVariant::fromValue(PlotType::CONTOUR_PLOT));
    plotGroup->addAction(actionNewHistogram);
    plotGroup->addAction(actionNewScatter);
    plotGroup->addAction(actionNewContour);
    toolBar->addAction(actionNewHistogram);
    toolBar->addAction(actionNewScatter);
    toolBar->addAction(actionNewContour);
    toolBar->addSeparator();


    // Gate buttons are now on each plot (AddGateButtonItem), no longer on the toolbar


    tabWidget = new QTabWidget(mainWidget);

    tabWidget->setTabsClosable(true);
    tabWidget->setMinimumHeight(300);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, [this](int index){
        QWidget *tab = tabWidget->widget(index);
        WorkSheetView *workSheetView = qobject_cast<WorkSheetView*>(tab);
        if (workSheetView) {
            m_activedWorksheetId.removeOne(workSheetView->worksheetId());
        }

        tab->deleteLater();
        tabWidget->removeTab(index);
    });


    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(tabWidget);
    splitter->addWidget(tableView);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    splitter->setChildrenCollapsible(false); // 防止部件被完全折叠

    QVBoxLayout *layout = new QVBoxLayout(mainWidget);
    layout->addWidget(toolBar);
    layout->addWidget(splitter);


    mainWidget->setLayout(layout);
    setWidget(mainWidget);
    connect(tabWidget, &QTabWidget::currentChanged, this, &WorkSheetWidget::onCurrentTabChanged);
    connect(plotGroup, &QActionGroup::triggered, this, &WorkSheetWidget::addNewPlot);
    connect(m_updateTimer, &QTimer::timeout, this, &WorkSheetWidget::onUpdateTimerTimeout);

}

void WorkSheetWidget::addPlot(PlotType plotType)
{
    AddNewPlotDialog dialog(plotType, currentWorkSheetView->worksheetId(), this);
    if (dialog.exec() == QDialog::Accepted) {
        Plot plot = dialog.getPlot();
        int plotId = PlotsDAO().insertPlot(plot);
        if (plotId > 0) {
            currentWorkSheetScene->addNewPlot(plotType, plot);
        }
    }
}

void WorkSheetWidget::onCurrentTabChanged(int index)
{
    currentWorkSheetView = qobject_cast<WorkSheetView*>(tabWidget->widget(index));
    if (currentWorkSheetView) {
        currentWorkSheetScene = currentWorkSheetView->scene();
        m_model->resetGateModel(currentWorkSheetView->worksheetId());
    }
}

void WorkSheetWidget::addNewPlot(QAction *action)
{
    if (!currentWorkSheetScene) {
        return;
    }
    PlotType plotType = action->data().value<PlotType>();
    addPlot(plotType);
}


void WorkSheetWidget::onUpdateTimerTimeout()
{
    // DataManager::instance().processData(currentWorkSheetScene->plots());
    EventDataManager::instance().processData(currentWorkSheetScene->plots());
}


