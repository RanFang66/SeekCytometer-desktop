#ifndef WORKSHEETWIDGET_H
#define WORKSHEETWIDGET_H

#include <QDockWidget>
#include <QAction>
#include <QTabWidget>
#include "WorkSheetView.h"
#include <QTimer>

#include "GatesModel.h"
#include <QTableView>

class WorkSheetWidget : public QDockWidget
{
    Q_OBJECT
public:
    static WorkSheetWidget *instance()
    {
        static WorkSheetWidget instance("WorkSheets");
        return &instance;
    }


    void setActive(bool active);
    void setActive(bool active, int interval);
    bool isActive() const;

    void resetPlots();

public slots:
    void addWorkSheetView(int worksheetId);
    void onFinishedDrawingGate(GateItem *gateItem);

private slots:
    void onCurrentTabChanged(int index);

    void addNewPlot(QAction *action);

    void onUpdateTimerTimeout();

private:
    explicit WorkSheetWidget(const QString &title, QWidget *parent = nullptr);
    WorkSheetWidget &operator=(const WorkSheetWidget &) = delete;
    WorkSheetWidget(const WorkSheetWidget &) = delete;


    void initDockWidget();
    void addPlot(PlotType type);

    // General actions
    QAction *actionPrint;
    QAction *actionSavePDF;
    QAction *actionSelect;

    // Actions for plots
    QAction *actionNewHistogram;
    QAction *actionNewScatter;
    QAction *actionNewContour;

    QTabWidget *tabWidget;
    WorkSheetView *currentWorkSheetView;
    WorkSheetScene *currentWorkSheetScene;
    bool        m_active;
    QTimer      *m_updateTimer;
    int         m_updateInterval;

    QList<int>  m_activedWorksheetId;


    QTableView *tableView;
    GatesModel *m_model;

};

#endif // WORKSHEETWIDGET_H
