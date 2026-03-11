#ifndef MENUBARMANAGER_H
#define MENUBARMANAGER_H

#include <QMenuBar>
#include <QMainWindow>
#include <QAction>

class MenuBarManager : public QObject {
    Q_OBJECT

public:
    explicit MenuBarManager(QMainWindow *parent);
    QMenuBar *getMenuBar() const;

    // 获取 QAction（用于工具栏复用）
    QAction *getOpenAction() const { return openAction; }
    QAction *getSaveAction() const { return saveAction; }
    QAction *getRunExperimentAction() const { return runExpAction; }

    QAction *getShowBrowserAction() const { return showBrowser; }
    QAction *getShowCytometerAction() const { return showCytometer; }
    QAction *getShowWorkSheetAction() const { return showWorkSheet; }
    QAction *getShowAcquisitionAction() const { return showAcquisition; }
    QAction *getShowSortingAction() const { return showSorting; }
    QAction *getShowCameraAction() const { return showCamera; }
    QAction *getShowWaveformAction() const { return showWaveform; }
    QAction *getResetLayoutAction() const { return resetLayout; }

private:
    QMenuBar *menuBar;
    QMainWindow *mainWindow;

    // 复用的 QAction
    QAction *openAction;
    QAction *saveAction;
    QAction *exitAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *runExpAction;
    QAction *settingsAction;

    QAction *showBrowser;
    QAction *showCytometer;
    QAction *showWorkSheet;
    QAction *showAcquisition;
    QAction *showSorting;
    QAction *showCamera;
    QAction *showWaveform;
    QAction *resetLayout;

    QAction *userManage;


    void setupMenuBar();
    void createFileMenu();
    void createEditMenu();
    void createViewMenu();
    void createExperimentMenu();
    void createCytometerMenu();
    void createWorkSheetMenu();
    void createHelpMenu();
    void createDataMenu();

private slots:
    void openFile();
    void saveFile();
    void runExperiment();
    void openUserManageDialog();
};

#endif // MENUBARMANAGER_H
