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

    // acquisitionWidget->raise();

    connect(experimentsBrowser, &ExperimentsBrowser::worksheetSelected, WorkSheetWidget::instance(), &WorkSheetWidget::addWorkSheetView);
    connect(experimentsBrowser, &ExperimentsBrowser::settingsSelected, cytometerSettingsWidget, &CytometerSettingsWidget::onCytometerSettingsChanged);


    QList<QDockWidget*> horizontalDocks = {
        experimentsBrowser,
        cytometerSettingsWidget,
        WorkSheetWidget::instance()
    };
    QList<int> sizes = {1, 3, 3};  // 比例值（实际像素会根据窗口大小计算）
    resizeDocks(horizontalDocks, sizes, Qt::Horizontal);
}





