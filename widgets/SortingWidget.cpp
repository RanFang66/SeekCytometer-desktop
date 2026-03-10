#include "SortingWidget.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include "WorkSheetWidget.h"
#include "GatesModel.h"
#include "CytometerController.h"
#include "EventDataManager.h"

SortingWidget::SortingWidget(const QString &tilte, QWidget *parent)
    : QDockWidget{tilte, parent}, updateTimer(new QTimer(this))
{
    initSortingWidget();
}

void SortingWidget::initSortingWidget()
{
    QWidget *mainWidget = new QWidget(this);
    QGroupBox *groupDrive = new QGroupBox(tr("Sort Drive"), this);
    QGroupBox *groupSetup = new QGroupBox(tr("Sort Setup"), this);
    QGroupBox *groupStatus = new QGroupBox(tr("Sort Status"), this);


    btnRunSorting = new QPushButton(tr("Start Sort"));
    btnConfirmSetting = new QPushButton(tr("Comfirm Setting"));
    btnPauseSorting = new QPushButton(tr("Pause"));
    editDriveWidth = new QLineEdit("100", this);
    // editDriveStrength = new QLineEdit("5000", this);
    editCoolingTime = new QLineEdit("200", this);
    editDriveDealy = new QLineEdit("100", this);
    editSortDist = new QLineEdit("0", this);
    comboDriveMode = new QComboBox(this);
    comboDriveMode->addItem(tr("Level Trig"), 0);
    comboDriveMode->addItem(tr("Edge Trig"), 1);
    // cBoxContinousMode = new QCheckBox(tr("Continous Mode"), this);
    comboSortMode = new QComboBox(this);
    comboSortMode->addItem(tr("Continous Mode"));
    comboSortMode->addItem(tr("Target Events"));
    editTargetEvents = new QLineEdit("10000", this);
    editTargetEvents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    comboPopulation = new QComboBox(this);
    comboPopulation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    comboPopulation->addItem(tr("All"),0);


    QGridLayout *driveLayout = new QGridLayout(groupDrive);
    driveLayout->addWidget(new QLabel(tr("Drive Mode"), this), 0, 0);
    driveLayout->addWidget(comboDriveMode, 0, 1);

    driveLayout->addWidget(new QLabel(tr("Drive Width(us)"), this), 0, 2);
    driveLayout->addWidget(editDriveWidth, 0, 3);
    // driveLayout->addWidget(new QLabel(tr("Drive Strength"), this), 0, 2);
    // driveLayout->addWidget(editDriveStrength, 0, 3);
    driveLayout->addWidget(new QLabel(tr("Cooling Time(us)"), this), 1, 0);
    driveLayout->addWidget(editCoolingTime, 1, 1);
    driveLayout->addWidget(new QLabel(tr("Drive Delay(us)"), this), 1, 2);
    driveLayout->addWidget(editDriveDealy, 1, 3);
    driveLayout->addWidget(new QLabel(tr("Sort Distance(um")), 2, 0);
    driveLayout->addWidget(editSortDist, 2, 1);

    driveLayout->addWidget(btnConfirmSetting, 2, 2, 1, 2);

    groupDrive->setLayout(driveLayout);

    QGridLayout *setupLayout = new QGridLayout(groupSetup);
    setupLayout->addWidget(btnRunSorting, 1, 0, 1, 3);
    setupLayout->addWidget(btnPauseSorting, 1, 3, 1, 2);
    setupLayout->addWidget(new QLabel(tr("Target Population"), this), 0, 0);
    setupLayout->addWidget(comboPopulation, 0, 1);
    // setupLayout->addWidget(cBoxContinousMode, 1, 2);
    setupLayout->addWidget(comboSortMode, 0, 2);
    setupLayout->addWidget(new QLabel(tr("Target Events"), this), 0, 3);
    setupLayout->addWidget(editTargetEvents, 0, 4);
    groupSetup->setLayout(setupLayout);


    QGridLayout *statusLayout = new QGridLayout(groupStatus);
    lblSortNum = new QLabel("0", this);
    lblDiscardNum = new QLabel("0", this);
    lblSortRate = new QLabel("0 / s", this);
    lblProcessRate = new QLabel("0 / s", this);
    lblEventsNum = new QLabel("0", this);
    lblSortRatio = new QLabel("0.00%", this);
    lblDiscardRatio = new QLabel("0.00%", this);
    lblSortTime = new QLabel("0 s", this);
    lblCellSpeed = new QLabel("0 m/s", this);
    // progressSort = new QProgressBar(this);



    statusLayout->addWidget(new QLabel(tr("Sort Time"), this), 0, 0);
    statusLayout->addWidget(lblSortTime, 0, 1);
    statusLayout->addWidget(new QLabel(tr("Event Number"), this), 0, 2);
    statusLayout->addWidget(lblEventsNum, 0, 3);



    statusLayout->addWidget(new QLabel(tr("Sort Number"), this), 1, 0);
    statusLayout->addWidget(lblSortNum, 1, 1);
    statusLayout->addWidget(new QLabel(tr("Discard Number"), this), 1, 2);
    statusLayout->addWidget(lblDiscardNum, 1, 3);

    statusLayout->addWidget(new QLabel(tr("Events Rate"), this), 2, 0);
    statusLayout->addWidget(lblProcessRate, 2, 1);
    statusLayout->addWidget(new QLabel(tr("Sort Rate"), this), 2, 2);
    statusLayout->addWidget(lblSortRate, 2, 3);


    statusLayout->addWidget(new QLabel(tr("Sort Ratio"), this), 3, 0);
    statusLayout->addWidget(lblSortRatio, 3, 1);
    statusLayout->addWidget(new QLabel(tr("Discard Ratio"), this), 3, 2);
    statusLayout->addWidget(lblDiscardRatio, 3, 3);

    statusLayout->addWidget(new QLabel(tr("Cell Speed"), this), 4, 0);
    statusLayout->addWidget(lblCellSpeed, 4, 1);

    // statusLayout->addWidget(new QLabel(tr("Sort Efficiency"), this), 3, 0);
    // statusLayout->addWidget(lblSortEfficiency, 3, 1);
    // statusLayout->addWidget(progressSort, 2, 2, 1, 2);
    groupStatus->setLayout(statusLayout);




    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(groupDrive, 1);
    mainLayout->addWidget(groupSetup, 1);
    mainLayout->addWidget(groupStatus, 2);
    mainWidget->setLayout(mainLayout);

    setWidget(mainWidget);


    updateTimer->setInterval(1000);
    updateTimer->stop();
    connect(updateTimer, &QTimer::timeout, this, &SortingWidget::updateDisplay);

    connect(btnRunSorting, &QPushButton::clicked, this, &SortingWidget::startSorting);
    // connect(comboDriveMode, &QComboBox::currentIndexChanged, this, &SortingWidget::changeDriveParameters);
    // connect(editCoolingTime, &QLineEdit::editingFinished, this, &SortingWidget::changeDriveParameters);
    // connect(editDriveWidth, &QLineEdit::editingFinished, this, &SortingWidget::changeDriveParameters);
    // connect(editDriveDealy, &QLineEdit::editingFinished, this, &SortingWidget::changeDriveParameters);

    connect(btnConfirmSetting, &QPushButton::clicked, this, &SortingWidget::changeDriveParameters);

    connect(comboPopulation, &QComboBox::currentIndexChanged, this, &SortingWidget::changeGate);

    connect(GatesModel::instance(), &GatesModel::dataChanged, this, &SortingWidget::updatePopulation);
}

void SortingWidget::resetSortingStatus()
{
    m_sortTime = 0;
    lblSortTime->setText("0 s");
    lblEventsNum->setText("0");
    lblSortNum->setText("0");
    lblDiscardNum->setText("0");
    lblProcessRate->setText("0 / s");
    lblSortRate->setText("0 /s");
    lblSortRatio->setText("0.00%");
    lblDiscardRatio->setText("0.00%");
    lblCellSpeed->setText("0 m/s");
}

int SortingWidget::calculateCoe(int measureDist, int sortDist)
{
    int coe = sortDist * 16384 / measureDist;
    return coe;
}

SortingWidget::~SortingWidget()
{
    deleteLater();
}

void SortingWidget::updatePopulation()
{
    comboPopulation->clear();
    QList<Gate> gateList = GatesModel::instance()->getGateList();
    for (int row = 0; row < gateList.size(); row++) {
        comboPopulation->addItem(gateList[row].name(), row);
    }
    comboPopulation->addItem("All", -1);
}

const Gate SortingWidget::getCurrentPopulation() const
{
    int row = comboPopulation->currentData().toInt();

    if (row == -1) {
        return Gate();
    }
    return GatesModel::instance()->getGate(row);
}

void SortingWidget::changeDriveParameters()
{
    m_driveType = comboDriveMode->currentData().toInt();
    m_driveDelay = editDriveDealy->text().toInt();
    m_driveWidth = editDriveWidth->text().toInt();
    m_coolingTime = editCoolingTime->text().toInt();
    m_sortDist = editSortDist->text().toInt();

    int coe;
    if (m_measureDist == 0 || m_sortDist == 0) {
        coe = 0;
    } else {
        coe = calculateCoe(m_measureDist, m_sortDist);
    }
    emit driveParametersChanged(m_driveType, m_driveDelay, m_driveWidth, m_coolingTime, coe);
}

void SortingWidget::changeGate()
{
    int id = comboPopulation->currentData().toInt();

    if (id == -1) {
        m_currGate = Gate();
    } else {
        m_currGate = GatesModel::instance()->getGate(id);
    }
    emit gateChanged(m_currGate);
}

void SortingWidget::startSorting()
{
    if (btnRunSorting->text() == tr("Start Sort")) {
        btnRunSorting->setText(tr("Stop Sort"));
        CytometerController::instance()->startSorting();
        WorkSheetWidget::instance()->resetPlots();
        resetSortingStatus();
        updateTimer->start();
    } else {
        btnRunSorting->setText(tr("Start Sort"));
        CytometerController::instance()->stopSorting();
        updateTimer->stop();
    }
}

void SortingWidget::updateDisplay()
{
    m_sortTime++;
    EventDataManager &dataManager = EventDataManager::instance();
    int eventsNum = dataManager.processedEventNum();
    int sortNum = dataManager.sortedEventNum();
    // int enableSortNum = dataManager.enableSortedEventNum();
    int discardNum = dataManager.discardedEventNum();
    double eventsRate = static_cast<double>(eventsNum) / m_sortTime;
    double sortRate = static_cast<double>(sortNum) / m_sortTime;

    double sortRatio = (eventsNum > 0) ? (static_cast<double>(sortNum) / eventsNum * 100.0) : 0;
    double discardRatio = (eventsNum > 0) ? (static_cast<double>(discardNum) / eventsNum * 100.0) : 0;
    double speed = dataManager.speedMeasured();

    lblSortTime->setText(QString("%1 s").arg(m_sortTime));
    lblEventsNum->setText(QString::number(eventsNum));
    lblSortNum->setText(QString::number(sortNum));
    lblDiscardNum->setText(QString::number(discardNum));
    lblProcessRate->setText(QString::asprintf("%.1f / s", eventsRate));
    lblSortRate->setText(QString::asprintf("%.1f / s", sortRate));
    lblSortRatio->setText(QString::asprintf("%.2f %%", sortRatio));
    lblDiscardRatio->setText(QString::asprintf("%.2f %%", discardRatio));
    lblCellSpeed->setText(QString::asprintf("%.2f m/s", speed));
}



