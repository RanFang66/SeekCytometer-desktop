#include "MainWindow.h"
#include <QMessageBox>

#include "ExperimentsBrowser.h"
#include "CytometerSettingsWidget.h"
#include "DataAcquisitionWidget.h"
#include "WorkSheetWidget.h"
#include "User.h"
#include "CytometerController.h"
#include "SortingWidget.h"
#include "WaveformWidget.h"

#include "SampleChipWidget.h"
#include "OpticsControlWidget.h"
#include "MicroFluidicWidget.h"
#include "CameraWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowIcon(QIcon(":/seekgene.ico"));
    initStatusBar();
    initMenuBar();
    setupToolBar();
    initDockWidgets();

    setWindowTitle(QString("SeekCytometer - %1").arg(User::loginUser().name()));
    setWindowState(Qt::WindowMaximized);

    CytometerController::instance()->start();
    connect(CytometerController::instance(), &CytometerController::connected, this, [this](){
        statusBar->updateConnectInfo(StatusIndicator::STATUS_RUNNING, tr("Connected to Server"));
        QMessageBox::information(this, tr("Connected"), tr("Connected with Cytometer"));
    });

    connect(CytometerController::instance(), &CytometerController::disconnected, this, [this](){
        statusBar->updateConnectInfo(StatusIndicator::STATUS_IDLE, tr("Disconnected"));
        QMessageBox::warning(this, tr("Disconnected"), tr("Disconnected with Cytometer"));
    });

    connect(CytometerController::instance(), &CytometerController::errorOccurred, this, [this](){
        statusBar->updateConnectInfo(StatusIndicator::STATUS_FAULT, tr("Fault"));
        QMessageBox::critical(this, tr("Fault!"), tr("There is critical fault"));
    });


    // QTimer::singleShot(1000, this, [this]() {
    //     CytometerController::instance()->connect();
    // });

}

MainWindow::~MainWindow() {}


void MainWindow::initStatusBar()
{
    statusBar = new CustomStatusBar;
    setStatusBar(statusBar);
}

void MainWindow::initMenuBar()
{
    menuBarManager = new MenuBarManager(this);
    setMenuBar(menuBarManager->getMenuBar());
}

void MainWindow::setupToolBar()
{
    toolBar = addToolBar("Main Toolbar");

    toolBar->addAction(menuBarManager->getSaveAction());
    toolBar->addSeparator();
    toolBar->addAction(menuBarManager->getShowBrowserAction());
    toolBar->addAction(menuBarManager->getShowCytometerAction());
    toolBar->addAction(menuBarManager->getShowWorkSheetAction());
    toolBar->addAction(menuBarManager->getShowAcquisitionAction());
    toolBar->addAction(menuBarManager->getShowSortingAction());
    toolBar->addAction(menuBarManager->getShowCameraAction());
    toolBar->addAction(menuBarManager->getShowWaveformAction());
    toolBar->addSeparator();
    toolBar->addAction(menuBarManager->getResetLayoutAction());
}

void MainWindow::initDockWidgets()
{
    QWidget *p = takeCentralWidget();
    if (p) {
        delete p;
    }
    setDockNestingEnabled(true);
    setDockOptions(AllowTabbedDocks | AllowNestedDocks);
    setStyleSheet(
        "QDockWidget {"
        "   border: 3px solid #0a0a0a;"
        "}"
        "QDockWidget::title {"
        "   background: #e0e0e0;"
        "   padding: 3px;"
        "}"
        );

    ExperimentsBrowser *experimentsBrowser = ExperimentsBrowser::instance();
    addDockWidget(Qt::LeftDockWidgetArea, experimentsBrowser);


    CytometerSettingsWidget *cytometerSettingsWidget = new CytometerSettingsWidget("Cytometer", this);
    splitDockWidget(experimentsBrowser, cytometerSettingsWidget, Qt::Horizontal);

    // WorkSheetWidget *workSheetWidget = new WorkSheetWidget("WorkSheet", this);
    splitDockWidget(cytometerSettingsWidget, WorkSheetWidget::instance(), Qt::Horizontal);
    tabifyDockWidget(WorkSheetWidget::instance(), WaveformWidget::instance());
    WorkSheetWidget::instance()->raise();

    DataAcquisitionWidget *acquisitionWidget = new DataAcquisitionWidget("Acquisition Control", this);
    splitDockWidget(cytometerSettingsWidget, acquisitionWidget, Qt::Vertical);

    tabifyDockWidget(acquisitionWidget, SortingWidget::instance());

    // 添加相机控制widget
    CameraWidget *cameraWidget = new CameraWidget("Camera Control", this);
    tabifyDockWidget(SortingWidget::instance(), cameraWidget);

    // SampleChipWidget *sampleChipWidget = new SampleChipWidget("Chip Control", this);
    // tabifyDockWidget(SortingWidget::instance(), sampleChipWidget);

    // OpticsControlWidget *opticsControlWidget = new OpticsControlWidget("Optics Control", this);
    // tabifyDockWidget(sampleChipWidget, opticsControlWidget);

    // MicroFluidicWidget *microFluidicWidget = new MicroFluidicWidget("MicroFluidic Control", this);
    // tabifyDockWidget(opticsControlWidget, microFluidicWidget);

    SortingWidget::instance()->raise();

    connect(experimentsBrowser, &ExperimentsBrowser::worksheetSelected, WorkSheetWidget::instance(), &WorkSheetWidget::addWorkSheetView);
    connect(experimentsBrowser, &ExperimentsBrowser::settingsSelected, cytometerSettingsWidget, &CytometerSettingsWidget::onCytometerSettingsChanged);

    // 记录 tabified 分组，用于隐藏后恢复位置
    tabGroups.append({WorkSheetWidget::instance(), WaveformWidget::instance()});
    tabGroups.append({acquisitionWidget, SortingWidget::instance(), cameraWidget});

    // Connect View menu actions to toggle dock widget visibility
    auto bindDockToggle = [this](QAction *action, QDockWidget *dock) {
        // Use triggered (not toggled) so programmatic setChecked won't cause feedback loop
        QObject::connect(action, &QAction::triggered, this, [this, dock](bool checked) {
            toggleDockWidget(dock, checked);
        });
        // Sync action state, but skip uncheck when dock is just on a background tab
        QObject::connect(dock, &QDockWidget::visibilityChanged, this, [this, action, dock](bool visible) {
            QSignalBlocker blocker(action);
            if (!visible) {
                for (const auto &group : tabGroups) {
                    if (group.contains(dock)) {
                        for (QDockWidget *peer : group) {
                            if (peer != dock && peer->isVisible()) {
                                return; // tab switch, not a real hide
                            }
                        }
                        break;
                    }
                }
            }
            action->setChecked(visible);
        });
    };

    bindDockToggle(menuBarManager->getShowBrowserAction(), experimentsBrowser);
    bindDockToggle(menuBarManager->getShowCytometerAction(), cytometerSettingsWidget);
    bindDockToggle(menuBarManager->getShowWorkSheetAction(), WorkSheetWidget::instance());
    bindDockToggle(menuBarManager->getShowAcquisitionAction(), acquisitionWidget);
    bindDockToggle(menuBarManager->getShowSortingAction(), SortingWidget::instance());
    bindDockToggle(menuBarManager->getShowCameraAction(), cameraWidget);
    bindDockToggle(menuBarManager->getShowWaveformAction(), WaveformWidget::instance());

    QList<QDockWidget*> horizontalDocks = {
        experimentsBrowser,
        cytometerSettingsWidget,
        WorkSheetWidget::instance()
    };
    QList<int> sizes = {1, 3, 3};  // 比例值（实际像素会根据窗口大小计算）
    resizeDocks(horizontalDocks, sizes, Qt::Horizontal);
}

void MainWindow::toggleDockWidget(QDockWidget *dock, bool visible)
{
    if (!visible) {
        dock->setVisible(false);
        return;
    }

    // 查找该 dock 是否属于某个 tabified 分组
    for (const auto &group : tabGroups) {
        if (!group.contains(dock))
            continue;

        // 找到同组中一个可见的 dock 作为锚点，重新 tabify
        for (QDockWidget *peer : group) {
            if (peer != dock && peer->isVisible()) {
                dock->setVisible(true);
                tabifyDockWidget(peer, dock);
                dock->raise();
                return;
            }
        }
        break;
    }

    // 不属于任何 tab 分组，或同组全部隐藏，直接显示
    dock->setVisible(true);
}





