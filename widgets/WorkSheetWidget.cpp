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
#include <QColorDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include "EventDataManager.h"
#include "GatesModel.h"
#include "GateStatistics.h"
#include "HistogramPlot.h"
#include "ScatterPlot.h"
#include <QSplitter>
#include <cmath>

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

        // Pick a default color based on current gate count, then let user choose
        QColor defaultColor = Gate::defaultGateColor(m_model->rowCount());
        QColor chosenColor = QColorDialog::getColor(defaultColor, this, tr("Select Gate Color"));
        if (chosenColor.isValid()) {
            gateItem->setGateColor(chosenColor);
        } else {
            gateItem->setGateColor(defaultColor);
        }

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
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
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
    tabWidget->setMinimumHeight(200);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, [this](int index){
        QWidget *tab = tabWidget->widget(index);
        WorkSheetView *workSheetView = qobject_cast<WorkSheetView*>(tab);
        if (workSheetView) {
            m_activedWorksheetId.removeOne(workSheetView->worksheetId());
        }

        tab->deleteLater();
        tabWidget->removeTab(index);
    });


    // Buttons below the table
    btnUpdateStats = new QPushButton(tr("Update Statistics"), this);
    btnDeleteGate = new QPushButton(tr("Delete Gate"), this);
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnUpdateStats);
    btnLayout->addWidget(btnDeleteGate);

    QWidget *tableContainer = new QWidget(this);
    QVBoxLayout *tableLayout = new QVBoxLayout(tableContainer);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->addWidget(tableView);
    tableLayout->addLayout(btnLayout);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(tabWidget);
    splitter->addWidget(tableContainer);
    splitter->setStretchFactor(0, 4);
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
    connect(btnUpdateStats, &QPushButton::clicked, this, &WorkSheetWidget::onUpdateStatisticsClicked);
    connect(btnDeleteGate, &QPushButton::clicked, this, &WorkSheetWidget::onDeleteGateClicked);

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
    updateGateStatistics();
}

void WorkSheetWidget::updateGateStatistics()
{
    if (!currentWorkSheetScene) return;

    const QList<GateItem*> &gateItems = currentWorkSheetScene->gates();
    if (gateItems.isEmpty()) return;

    for (GateItem *gateItem : gateItems) {
        const Gate &gate = gateItem->gate();
        PlotBase *plot = gateItem->parentPlot();
        if (!plot) continue;

        GateStatistics stats;

        if (gate.gateType() == GateType::IntervalGate) {
            // 1D gate on histogram
            stats.is1D = true;
            HistogramPlot *histPlot = dynamic_cast<HistogramPlot*>(plot);
            if (!histPlot) continue;

            const QList<QPoint> &pts = gate.points();
            if (pts.size() < 2) continue;
            int gateMin = qMin(pts[0].x(), pts[1].x());
            int gateMax = qMax(pts[0].x(), pts[1].x());

            QVector<int> allData = histPlot->readAllData();
            double sumX = 0.0;
            int count = 0;

            for (int val : allData) {
                if (val >= gateMin && val <= gateMax) {
                    sumX += val;
                    count++;
                }
            }

            stats.count = count;
            if (count > 0) {
                stats.meanX = sumX / count;

                double sumSqDiffX = 0.0;
                for (int val : allData) {
                    if (val >= gateMin && val <= gateMax) {
                        double diff = val - stats.meanX;
                        sumSqDiffX += diff * diff;
                    }
                }
                stats.stdDevX = std::sqrt(sumSqDiffX / count);
                stats.cvX = (stats.meanX != 0.0) ? (stats.stdDevX / std::abs(stats.meanX)) * 100.0 : 0.0;
            }
        } else {
            // 2D gate (Rectangle, etc.)
            stats.is1D = false;
            ScatterPlot *scatPlot = dynamic_cast<ScatterPlot*>(plot);
            if (!scatPlot) continue;

            const QList<QPoint> &pts = gate.points();
            if (pts.size() < 2) continue;

            int gateMinX, gateMaxX, gateMinY, gateMaxY;
            gate.getGateRange(gateMinX, gateMaxX, gateMinY, gateMaxY);

            QVector<QPoint> allData = scatPlot->readAllData();
            double sumX = 0.0, sumY = 0.0;
            int count = 0;

            for (const QPoint &pt : allData) {
                if (pt.x() >= gateMinX && pt.x() <= gateMaxX &&
                    pt.y() >= gateMinY && pt.y() <= gateMaxY) {
                    sumX += pt.x();
                    sumY += pt.y();
                    count++;
                }
            }

            stats.count = count;
            if (count > 0) {
                stats.meanX = sumX / count;
                stats.meanY = sumY / count;

                double sumSqDiffX = 0.0, sumSqDiffY = 0.0;
                for (const QPoint &pt : allData) {
                    if (pt.x() >= gateMinX && pt.x() <= gateMaxX &&
                        pt.y() >= gateMinY && pt.y() <= gateMaxY) {
                        double diffX = pt.x() - stats.meanX;
                        double diffY = pt.y() - stats.meanY;
                        sumSqDiffX += diffX * diffX;
                        sumSqDiffY += diffY * diffY;
                    }
                }
                stats.stdDevX = std::sqrt(sumSqDiffX / count);
                stats.stdDevY = std::sqrt(sumSqDiffY / count);
                stats.cvX = (stats.meanX != 0.0) ? (stats.stdDevX / std::abs(stats.meanX)) * 100.0 : 0.0;
                stats.cvY = (stats.meanY != 0.0) ? (stats.stdDevY / std::abs(stats.meanY)) * 100.0 : 0.0;
            }
        }

        m_model->updateGateStatistics(gate.id(), stats);
    }
}

void WorkSheetWidget::onUpdateStatisticsClicked()
{
    updateGateStatistics();
}

void WorkSheetWidget::onDeleteGateClicked()
{
    QModelIndex currentIndex = tableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, tr("Delete Gate"), tr("Please select a gate row first."));
        return;
    }

    int row = currentIndex.row();
    const Gate gate = m_model->getGate(row);

    QMessageBox::StandardButton ret = QMessageBox::question(
        this, tr("Delete Gate"),
        tr("Are you sure you want to delete gate \"%1\"?").arg(gate.name()),
        QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes) {
        return;
    }

    // Remove from scene
    if (currentWorkSheetScene) {
        currentWorkSheetScene->removeGateItem(gate.id());
    }

    // Remove from model (and database)
    m_model->removeGate(row);
}
