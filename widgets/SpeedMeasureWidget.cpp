#include "SpeedMeasureWidget.h"
#include <QGridLayout>
#include "DetectorSettingsModel.h"
#include "DetectorsDAO.h"
#include "SortingWidget.h"
#include "EventDataManager.h"
SpeedMeasureWidget::SpeedMeasureWidget(QWidget *parent)
    : QWidget{parent}
{
    initWidget();

}

SpeedMeasureWidget::~SpeedMeasureWidget()
{
    deleteLater();
}

void SpeedMeasureWidget::onCytometerSettingChanged()
{
    comboPostCh->clear();
    for (const DetectorSettings &settings : DetectorSettingsModel::instance()->detectorSettings()) {
        comboPostCh->addItem(QString("%1(Channel%2)").arg(settings.parameterName()).arg(settings.detectorId()), settings.detectorId());
    }

    comboPreCh->clear();
    for (const Detector &detector : DetectorsDAO().fetchDetectorsUnset(DetectorSettingsModel::instance()->getSettingId())) {
        comboPreCh->addItem(QString("Channel %1").arg(detector.id()), detector.id());
    }
}

void SpeedMeasureWidget::initWidget()
{
    // QLabel *lblPreCh = new QLabel(tr("Previous Channel Select"), this);
    // QLabel *lblPostCh = new QLabel(tr("Second Channel Select"), this);
    // QLabel *lblDist = new QLabel(tr("Two measurement point distance(μm): "), this);



    comboPreCh = new QComboBox(this);
    comboPostCh = new QComboBox(this);
    spinDist = new QSpinBox(this);
    editPreChThresh = new QLineEdit("300", this);
    btnConfirm = new QPushButton(tr("Confirm Setting"), this);

    spinMaxTimeDiff = new QSpinBox(this);


    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(new QLabel(tr("Previous Channel Select"), this), 0, 0);
    layout->addWidget(comboPreCh, 0, 1);
    layout->addWidget(new QLabel(tr("Previous Channel Threshold"), this), 1, 0);
    layout->addWidget(editPreChThresh, 1, 1);
    layout->addWidget(new QLabel(tr("Second Channel Select"), this), 2, 0);
    layout->addWidget(comboPostCh, 2, 1);
    layout->addWidget(new QLabel(tr("Max Time Span(us)"), this), 3, 0);
    layout->addWidget(spinMaxTimeDiff, 3, 1);

    layout->addWidget(new QLabel(tr("Two measurement point distance(μm): "), this), 4, 0);
    layout->addWidget(spinDist, 4, 1);
    layout->addWidget(btnConfirm, 5, 0, 1, 2);
    spinDist->setRange(0, 2000);
    spinDist->setValue(200);
    spinMaxTimeDiff->setRange(1, 2000);
    spinMaxTimeDiff->setValue(60);

    setLayout(layout);

    connect(btnConfirm, &QPushButton::clicked, this, [this](){
        m_preChId = comboPreCh->currentData().toInt();
        m_preChThresh = editPreChThresh->text().toInt();
        m_postChId = comboPostCh->currentData().toInt();
        m_dist = spinDist->value();
        m_maxTimeSpan = spinMaxTimeDiff->value();
        SortingWidget::instance()->setMeasureDist(m_dist);
        EventDataManager::instance().setSpeedMeasureDist(m_dist);
        emit speedMeasureSettingChangged(m_preChId, m_postChId, m_preChThresh, m_dist, m_maxTimeSpan);
    });


    connect(DetectorSettingsModel::instance(), &DetectorSettingsModel::dataChanged, this, &SpeedMeasureWidget::onCytometerSettingChanged);
}
