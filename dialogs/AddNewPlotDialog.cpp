#include "AddNewPlotDialog.h"
#include <QFormLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "DetectorSettingsModel.h"
#include "DetectorSettingsDAO.h"

AddNewPlotDialog::AddNewPlotDialog(PlotType plotType, int worksheetId, QWidget *parent)
:QDialog{parent}, m_plotType{plotType}, m_worksheetId{worksheetId}
{
    initDialog();
}

QString AddNewPlotDialog::xAxisName() const
{
    return m_xAxisName;
}

QString AddNewPlotDialog::yAxisName() const
{
    return m_yAxisName;
}

int AddNewPlotDialog::xAxisId() const
{
    return m_xAxisId;
}

int AddNewPlotDialog::yAxisId() const
{
    return m_yAxisId;
}

PlotType AddNewPlotDialog::plotType() const
{
    return m_plotType;
}

QString AddNewPlotDialog::plotName() const
{
    return plotNameEdit->text();
}

MeasurementType AddNewPlotDialog::xMeasurementType() const
{
    return m_xMeasurementType;
}

MeasurementType AddNewPlotDialog::yMeasurementType() const
{
    return m_yMeasurementType;
}

Plot AddNewPlotDialog::getPlot() const
{
    Plot plot;
    plot.setWorkSheetId(m_worksheetId);
    plot.setAxisXId(xAxisId());
    plot.setAxisYId(yAxisId());
    plot.setPlotType(plotType());
    plot.setName(plotName());
    plot.setXMeasurementType(xMeasurementType());
    plot.setYMeasurementType(yMeasurementType());
    return plot;
}

void AddNewPlotDialog::initDialog()
{
    xAxisCombo = new QComboBox(this);
    yAxisCombo = new QComboBox(this);
    plotNameEdit = new QLineEdit(this);

    switch (m_plotType) {
        case PlotType::HISTOGRAM_PLOT:
            setWindowTitle(tr("Add New Histogram"));
            break;
        case PlotType::SCATTER_PLOT:
            setWindowTitle(tr("Add New Scatter Plot"));
            break;
        case PlotType::CONTOUR_PLOT:
            setWindowTitle(tr("Add New Contour Plot"));
            break;
        default:
            break;
    }

    QVBoxLayout *layout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnOk = new QPushButton(tr("Ok"), this);
    QPushButton *btnCancel = new QPushButton(tr("Cancel"), this);
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    xAxisCombo->clear();
    yAxisCombo->clear();
    for (const DetectorSettings &settings : DetectorSettingsModel::instance()->detectorSettings()) {
        for (const MeasurementType &type : settings.enabledMeasurementTypes()) {
            xAxisCombo->addItem(MeasurementTypeHelper::parameterMeasurementType(settings.parameterName(), type), settings.id());
            yAxisCombo->addItem(MeasurementTypeHelper::parameterMeasurementType(settings.parameterName(), type), settings.id());
        }
    }
    xAxisCombo->setCurrentIndex(0);
    yAxisCombo->setCurrentIndex(1);

    updateDefaultPlotName();

    formLayout->addRow(tr("X Axis Parameter: "), xAxisCombo);
    if (m_plotType == PlotType::SCATTER_PLOT || m_plotType == PlotType::CONTOUR_PLOT) {
        formLayout->addRow(tr("Y Axis Parameter: "), yAxisCombo);
    } else {
        yAxisCombo->hide();
    }
    formLayout->addRow(tr("Plot Name: "), plotNameEdit);
    layout->addLayout(formLayout);
    layout->addLayout(btnLayout);
    setLayout(layout);

    connect(xAxisCombo, &QComboBox::currentIndexChanged, this, &AddNewPlotDialog::updateDefaultPlotName);
    connect(yAxisCombo, &QComboBox::currentIndexChanged, this, &AddNewPlotDialog::updateDefaultPlotName);
    connect(btnOk, &QPushButton::clicked, this, [this](){
        accept();
    });
    connect(btnCancel, &QPushButton::clicked, this, [this](){
        reject();
    });
}

void AddNewPlotDialog::updateDefaultPlotName()
{
    m_xAxisId = xAxisCombo->currentData().toInt();
    m_xAxisName = xAxisCombo->currentText();
    if (m_xAxisName.last(1) == "H") {
        m_xMeasurementType = MeasurementType::Height;
    } else if (m_xAxisName.last(1) == "W") {
        m_xMeasurementType = MeasurementType::Width;
    } else if (m_xAxisName.last(1) == "A") {
        m_xMeasurementType = MeasurementType::Area;
    } else {
        m_xMeasurementType = MeasurementType::Unknown;
    }

    if (m_plotType != PlotType::HISTOGRAM_PLOT) {
        m_yAxisId = yAxisCombo->currentData().toInt();
        m_yAxisName = yAxisCombo->currentText();
        if (m_yAxisName.last(1) == "H") {
            m_yMeasurementType = MeasurementType::Height;
        } else if (m_yAxisName.last(1) == "W") {
            m_yMeasurementType = MeasurementType::Width;
        } else if (m_yAxisName.last(1) == "A") {
            m_yMeasurementType = MeasurementType::Area;
        } else {
            m_yMeasurementType = MeasurementType::Unknown;
        }
        m_plotName = QString("%1 %2-%3").arg(Plot::plotTypeToString(m_plotType), m_xAxisName, m_yAxisName);
    } else {
        m_yAxisId = 0;
        m_yAxisName = "Count";
        m_yMeasurementType = MeasurementType::Unknown;
        m_plotName = QString("Histogram %1").arg(m_xAxisName);
    }
    plotNameEdit->setText(m_plotName);
}
