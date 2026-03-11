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

private:
    CustomStatusBar         *statusBar;
    MenuBarManager          *menuBarManager;
    QToolBar                *toolBar;

    // 记录 tabified dock 分组，用于隐藏后恢复
    QList<QList<QDockWidget*>> tabGroups;

    void initStatusBar();
    void initMenuBar();
    void setupToolBar();
    void initDockWidgets();
    void toggleDockWidget(QDockWidget *dock, bool visible);

};
#endif // MAINWINDOW_H
