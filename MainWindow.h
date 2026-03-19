#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QDockWidget>
#include "CustomStatusBar.h"
#include "MenuBarManager.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

private slots:
    void resetToDefaultLayout();

private:
    CustomStatusBar         *statusBar;
    MenuBarManager          *menuBarManager;
    QToolBar                *toolBar;

    // 记录 tabified dock 分组，用于隐藏后恢复
    QList<QList<QDockWidget*>> tabGroups;

    // 保存所有 dock widget 引用，用于 Reset Layout
    QDockWidget *m_browserDock;
    QDockWidget *m_cytometerDock;
    QDockWidget *m_worksheetDock;
    QDockWidget *m_waveformDock;
    QDockWidget *m_acquisitionDock;
    QDockWidget *m_sortingDock;
    QDockWidget *m_cameraDock;

    void showEvent(QShowEvent *event) override;

    void initStatusBar();
    void initMenuBar();
    void setupToolBar();
    void initDockWidgets();
    void applyDefaultLayout();
    void applyDefaultDockSizes();
    void toggleDockWidget(QDockWidget *dock, bool visible);

    bool m_firstShow = true;

};
#endif // MAINWINDOW_H
