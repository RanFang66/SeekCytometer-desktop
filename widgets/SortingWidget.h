#ifndef SORTINGWIDGET_H
#define SORTINGWIDGET_H

#include <QDockWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QProgressBar>
#include "Gate.h"
#include <QTimer>


class SortingWidget : public QDockWidget
{
    Q_OBJECT
public:
    static SortingWidget *instance()
    {
        static SortingWidget instance("Sort Control");
        return &instance;
    }


    ~SortingWidget();

    const Gate getCurrentPopulation() const;
    void setMeasureDist(int dist);

public slots:
    void updatePopulation();



signals:
    void driveParametersChanged(int type, int delay, int width, int coolingTime, int coe);
    void gateChanged(const Gate &gate);

private slots:
    void changeDriveParameters();
    void changeGate();
    void startSorting();
    void updateDisplay();


private:
    explicit SortingWidget(const QString &tilte = QString("Sorting"), QWidget *parent = nullptr);
    SortingWidget &operator=(const SortingWidget &) = delete;
    SortingWidget(const SortingWidget &) = delete;


    void initSortingWidget();
    void resetSortingStatus();
    int  calculateCoe(int measureDist, int sortDist);


    int m_driveType;
    int m_driveDelay;
    int m_driveWidth;
    int m_coolingTime;
    int m_sortDist;
    int m_measureDist;

    Gate m_currGate;

    QTimer *updateTimer;

    int  m_sortTime;

    QPushButton *btnRunSorting;
    QPushButton *btnPauseSorting;
    QPushButton *btnConfirmSetting;
    QLineEdit   *editDriveWidth;
    QLineEdit   *editDriveStrength;
    QLineEdit   *editCoolingTime;
    QLineEdit   *editDriveDealy;
    QLineEdit   *editSortDist;
    QComboBox   *comboDriveMode;
    QComboBox   *comboPopulation;
    QComboBox   *comboSortMode;
    QCheckBox   *cBoxContinousMode;
    QLineEdit   *editTargetEvents;


    QLabel          *lblEventsNum;
    QLabel          *lblSortTime;
    QLabel          *lblSortNum;
    QLabel          *lblDiscardNum;
    QLabel          *lblProcessRate;
    QLabel          *lblSortRate;
    QLabel          *lblSortRatio;
    QLabel          *lblCellSpeed;
    QLabel          *lblDiscardRatio;


    // QProgressBar    *progressSort;
};

inline void SortingWidget::setMeasureDist(int dist)
{
    m_measureDist = dist;
}

#endif // SORTINGWIDGET_H
