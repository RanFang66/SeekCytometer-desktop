#include "MainWindow.h"
#include <QMessageBox>
#include <QTimer>

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

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (m_firstShow) {
        m_firstShow = false;
        // 延迟到事件循环，确保窗口已完成布局并拥有最终尺寸
        QTimer::singleShot(0, this, &MainWindow::applyDefaultDockSizes);
    }
}

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

    // 创建并保存所有 dock widget 引用
    m_browserDock = ExperimentsBrowser::instance();
    m_cytometerDock = new CytometerSettingsWidget("Cytometer", this);
    m_worksheetDock = WorkSheetWidget::instance();
    m_waveformDock = WaveformWidget::instance();
    m_acquisitionDock = new DataAcquisitionWidget("Acquisition Control", this);
    m_sortingDock = SortingWidget::instance();
    m_cameraDock = new CameraWidget("Camera Control", this);

    connect(ExperimentsBrowser::instance(), &ExperimentsBrowser::worksheetSelected, WorkSheetWidget::instance(), &WorkSheetWidget::addWorkSheetView);
    connect(ExperimentsBrowser::instance(), &ExperimentsBrowser::settingsSelected,
            qobject_cast<CytometerSettingsWidget*>(m_cytometerDock), &CytometerSettingsWidget::onCytometerSettingsChanged);

    // 记录 tabified 分组，用于隐藏后恢复位置
    tabGroups.append(QList<QDockWidget*>{m_worksheetDock, m_waveformDock});
    tabGroups.append(QList<QDockWidget*>{m_acquisitionDock, m_sortingDock, m_cameraDock});

    // Connect View menu actions to toggle dock widget visibility
    auto bindDockToggle = [this](QAction *action, QDockWidget *dock) {
        QObject::connect(action, &QAction::triggered, this, [this, dock](bool checked) {
            toggleDockWidget(dock, checked);
        });
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

    bindDockToggle(menuBarManager->getShowBrowserAction(), m_browserDock);
    bindDockToggle(menuBarManager->getShowCytometerAction(), m_cytometerDock);
    bindDockToggle(menuBarManager->getShowWorkSheetAction(), m_worksheetDock);
    bindDockToggle(menuBarManager->getShowAcquisitionAction(), m_acquisitionDock);
    bindDockToggle(menuBarManager->getShowSortingAction(), m_sortingDock);
    bindDockToggle(menuBarManager->getShowCameraAction(), m_cameraDock);
    bindDockToggle(menuBarManager->getShowWaveformAction(), m_waveformDock);

    // Reset Layout 按钮连接
    connect(menuBarManager->getResetLayoutAction(), &QAction::triggered, this, &MainWindow::resetToDefaultLayout);

    applyDefaultLayout();
}

void MainWindow::applyDefaultLayout()
{
    // 第一列：Browser
    addDockWidget(Qt::LeftDockWidgetArea, m_browserDock);

    // 第二列：Cytometer（上）+ Acquisition/Sorting/Camera（下，tabified）
    splitDockWidget(m_browserDock, m_cytometerDock, Qt::Horizontal);


    // 第三列：WorkSheet / Waveform（tabified）
    splitDockWidget(m_cytometerDock, m_worksheetDock, Qt::Horizontal);


    splitDockWidget(m_cytometerDock, m_acquisitionDock, Qt::Vertical);
    tabifyDockWidget(m_acquisitionDock, m_sortingDock);
    tabifyDockWidget(m_sortingDock, m_cameraDock);
    m_sortingDock->raise();


    tabifyDockWidget(m_worksheetDock, m_waveformDock);
    m_worksheetDock->raise();


}

void MainWindow::resetToDefaultLayout()
{
    // 显示所有 dock widget
    QList<QDockWidget*> allDocks = {m_browserDock, m_cytometerDock, m_worksheetDock,
                                     m_waveformDock, m_acquisitionDock, m_sortingDock, m_cameraDock};
    for (QDockWidget *dock : allDocks) {
        dock->setVisible(true);
        dock->setFloating(false);
    }

    // 同步 View 菜单的 action 状态
    menuBarManager->getShowBrowserAction()->setChecked(true);
    menuBarManager->getShowCytometerAction()->setChecked(true);
    menuBarManager->getShowWorkSheetAction()->setChecked(true);
    menuBarManager->getShowAcquisitionAction()->setChecked(true);
    menuBarManager->getShowSortingAction()->setChecked(true);
    menuBarManager->getShowCameraAction()->setChecked(true);
    menuBarManager->getShowWaveformAction()->setChecked(true);

    // 重新应用默认布局
    applyDefaultLayout();

    // 强制恢复最大化，防止 dock 重排后窗口尺寸超出屏幕
    setWindowState(Qt::WindowMaximized);

    // 延迟应用比例，等窗口几何更新完成
    QTimer::singleShot(0, this, &MainWindow::applyDefaultDockSizes);
}

void MainWindow::applyDefaultDockSizes()
{
    // 三列宽度比 1:2:3
    QList<QDockWidget*> horizontalDocks = {m_browserDock, m_cytometerDock, m_worksheetDock};
    QList<int> sizes = {1, 2, 3};
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





