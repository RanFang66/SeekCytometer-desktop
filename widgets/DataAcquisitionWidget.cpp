#include "DataAcquisitionWidget.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include "CytometerController.h"

DataAcquisitionWidget::DataAcquisitionWidget(const QString &tilte, QWidget *parent)
: QDockWidget{tilte, parent}
{
    initDockWidget();
}

void DataAcquisitionWidget::startAcquisition()
{
    if (btnAcquisition->text() == tr("Start Acquisition")) {
        btnAcquisition->setText(tr("Stop Acquisition"));
        CytometerController::instance()->startAcquisition();
    } else {
        btnAcquisition->setText(tr("Start Acquisition"));
        CytometerController::instance()->stopAcquisition();
    }

    //TODO: Implement this function
}

void DataAcquisitionWidget::startRecordData()
{

}

void DataAcquisitionWidget::restartAcquisition()
{

}


void DataAcquisitionWidget::initDockWidget()
{
    QWidget *mainWidget = new QWidget(this);
    QGroupBox *groupActivity = new QGroupBox(tr("Current Activity"), this);
    QGroupBox *groupBasicControls = new QGroupBox(tr("Basic Controls"), this);
    QGroupBox *groupSetup = new QGroupBox(tr("Acquisition Setup"), this);
    QGroupBox *groupStatus = new QGroupBox(tr("Acquisition Status"), this);


    QGridLayout *activityLayout = new QGridLayout(groupActivity);
    progressTube = new QProgressBar(this);
    lblThresholdRate = new QLabel("0 evt/s", this);
    lblStopGateEvents = new QLabel("0 evt", this);
    lblElapsedTime = new QLabel("00:00:00");

    progressTube->setFixedHeight(20);
    progressTube->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    progressTube->setRange(0, 100);
    progressTube->setValue(10);
    activityLayout->addWidget(new QLabel(tr("Active Tube"), this), 0, 0);
    activityLayout->addWidget(new QLabel(tr("Threshold Rate"), this), 0, 1);
    activityLayout->addWidget(new QLabel(tr("Stopping Gate Events"), this), 0, 2);
    activityLayout->addWidget(new QLabel(tr("Elased Time"), this), 0, 3);
    activityLayout->addWidget(progressTube, 1, 0);
    activityLayout->addWidget(lblThresholdRate, 1, 1);
    activityLayout->addWidget(lblStopGateEvents, 1, 2);
    activityLayout->addWidget(lblElapsedTime, 1, 3);

    groupActivity->setLayout(activityLayout);


    QHBoxLayout *basicControlsLayout = new QHBoxLayout(groupBasicControls);
    btnAcquisition = new QPushButton(tr("Start Acquisition"), this);
    btnRecord = new QPushButton(tr("Record"), this);
    btnRestart = new QPushButton(tr("Restart"), this);
    basicControlsLayout->addWidget(btnAcquisition);
    basicControlsLayout->addWidget(btnRecord);
    basicControlsLayout->addWidget(btnRestart);
    groupBasicControls->setLayout(basicControlsLayout);

    QGridLayout *setupLayout = new QGridLayout(groupSetup);
    cBoxStoppingGate = new QComboBox(this);
    cBoxRecordEvents = new QComboBox(this);
    spinBoxStoppingTime = new QSpinBox(this);
    cBoxStorageGate = new QComboBox(this);
    cBoxDisplayEvents = new QComboBox(this);
    cBoxFlowRate = new QComboBox(this);


    cBoxStoppingGate->addItem(tr("All Events"));
    cBoxStoppingGate->setCurrentIndex(0);
    QStringList eventsNum;
    eventsNum  << "10000 evt" << "20000 evt" << "30000 evt" << "40000 evt" << "50000 evt";
    cBoxRecordEvents->addItems(eventsNum);
    cBoxRecordEvents->setCurrentIndex(0);
    cBoxStorageGate->addItem(tr("All Events"));
    cBoxStorageGate->setCurrentIndex(0);
    QStringList eventsToDisplay;
    eventsToDisplay << "1000 evt" << "2000 evt" << "5000 evt" << "10000 evt";
    cBoxDisplayEvents->addItems(eventsToDisplay);
    cBoxDisplayEvents->setCurrentIndex(0);
    spinBoxStoppingTime->setRange(0, 600);
    spinBoxStoppingTime->setValue(0);
    QStringList flowRate;
    flowRate << "Low" << "Medium" << "High";
    cBoxFlowRate->addItems(flowRate);
    cBoxFlowRate->setCurrentIndex(1);

    setupLayout->addWidget(new QLabel(tr("Stopping Gate:"), this), 0, 0);
    setupLayout->addWidget(cBoxStoppingGate, 0, 1);
    setupLayout->addWidget(new QLabel(tr("Events To Record:"), this), 0, 2);
    setupLayout->addWidget(cBoxRecordEvents, 0, 3);
    setupLayout->addWidget(new QLabel(tr("Stopping Time"), this), 0, 4);
    setupLayout->addWidget(spinBoxStoppingTime, 0, 5);
    setupLayout->addWidget(new QLabel(tr("Storage Gate"), this), 1, 0);
    setupLayout->addWidget(cBoxStorageGate, 1, 1);
    setupLayout->addWidget(new QLabel(tr("Events To Display"), this), 1, 2);
    setupLayout->addWidget(cBoxDisplayEvents, 1, 3);
    setupLayout->addWidget(new QLabel(tr("Flow Rate"), this), 1, 4);
    setupLayout->addWidget(cBoxFlowRate, 1, 5);
    groupSetup->setLayout(setupLayout);

    QGridLayout *statusLayout = new QGridLayout(groupStatus);
    lblProcessEvenets = new QLabel("0 evt", this);
    lblThresholdCount = new QLabel("0 evt", this);
    statusLayout->addWidget(new QLabel(tr("Processed Events:"), this), 0, 0);
    statusLayout->addWidget(lblProcessEvenets, 0, 1);
    statusLayout->addWidget(new QLabel(tr("Threshold Count:"), this), 0, 2);
    statusLayout->addWidget(lblThresholdCount, 0, 3);
    groupStatus->setLayout(statusLayout);

    groupActivity->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    groupBasicControls->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    groupSetup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    groupStatus->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(groupActivity);
    mainLayout->addWidget(groupBasicControls);
    mainLayout->addWidget(groupSetup);
    mainLayout->addWidget(groupStatus);
    mainLayout->addStretch();
    mainWidget->setLayout(mainLayout);

    setWidget(mainWidget);

    connect(btnAcquisition, &QPushButton::clicked, this, &DataAcquisitionWidget::startAcquisition);
    connect(btnRecord, &QPushButton::clicked, this, &DataAcquisitionWidget::startRecordData);
    connect(btnRestart, &QPushButton::clicked, this, &DataAcquisitionWidget::restartAcquisition);
}
