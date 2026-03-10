#include "CameraWidget.h"
#include "CameraController.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QMetaObject>
#include <QDebug>

CameraWidget::CameraWidget(const QString &title, QWidget *parent)
    : QDockWidget(title, parent),
      m_cameraThread(new QThread(this)),
      m_controller(new CameraController()),
      m_isCapturing(false)
{
    // 将控制器移到独立线程
    m_controller->moveToThread(m_cameraThread);

    // 线程启动时初始化相机
    connect(m_cameraThread, &QThread::started,
            m_controller, &CameraController::initialize);

    // 初始化UI
    initDockWidget();

    // 连接信号槽
    connectSignalsSlots();

    // 启动线程
    m_cameraThread->start();
}

CameraWidget::~CameraWidget()
{
    qDebug() << "CameraWidget::~CameraWidget()";

    // 停止采集并关闭相机（阻塞调用，确保完成）
    if (m_controller) {
        QMetaObject::invokeMethod(m_controller,
                                  &CameraController::shutdown,
                                  Qt::BlockingQueuedConnection);
    }

    // 退出线程
    m_cameraThread->quit();
    m_cameraThread->wait();

    // 清理对象
    if (m_controller) {
        m_controller->deleteLater();
    }
}

void CameraWidget::initDockWidget()
{
    QWidget *mainWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainWidget);

    // === 左侧：图像显示区 ===
    m_imageLabel = new QLabel();
    m_imageLabel->setMinimumSize(640, 480);
    m_imageLabel->setScaledContents(true);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet("QLabel { background-color: #2b2b2b; color: #888; border: 1px solid #555; }");
    m_imageLabel->setText(tr("Camera Initializing..."));
    mainLayout->addWidget(m_imageLabel, 3);  // 伸展因子3

    // === 右侧：控制面板 ===
    QVBoxLayout *controlLayout = new QVBoxLayout();

    // 1. 状态信息组
    QGroupBox *statusGroup = new QGroupBox(tr("Camera Status"));
    QFormLayout *statusLayout = new QFormLayout();
    lblCameraStatus = new QLabel(tr("Initializing..."));
    lblResolution = new QLabel("-");
    lblCameraName = new QLabel("JHUMS(SN)");
    statusLayout->addRow(tr("Name(SN):"), lblCameraName);
    statusLayout->addRow(tr("Status:"), lblCameraStatus);
    statusLayout->addRow(tr("Resolution:"), lblResolution);
    statusGroup->setLayout(statusLayout);
    controlLayout->addWidget(statusGroup);

    // 2. 采集控制组
    QGroupBox *captureGroup = new QGroupBox(tr("Capture Control"));
    QVBoxLayout *captureLayout = new QVBoxLayout();
    btnStartStop = new QPushButton(tr("Start"));
    btnStartStop->setEnabled(false);  // 初始化前禁用
    btnSaveImage = new QPushButton(tr("Save Image"));
    btnSaveImage->setEnabled(false);
    captureLayout->addWidget(btnStartStop);
    captureLayout->addWidget(btnSaveImage);
    captureGroup->setLayout(captureLayout);
    controlLayout->addWidget(captureGroup);

    // 3. 色彩平衡组
    QGroupBox *wbGroup = new QGroupBox(tr("White Balance"));
    QFormLayout *wbLayout = new QFormLayout();

    // Auto White Balance
    cBoxAWB = new QCheckBox("AWB");
    btnOnePushWB = new QPushButton(tr("One Push WB"));

    cBoxAWB->setEnabled(false);
    btnOnePushWB->setEnabled(false);
    wbLayout->addRow(cBoxAWB, btnOnePushWB);


    // 红色增益
    QHBoxLayout *redLayout = new QHBoxLayout();
    sliderRedGain = new QSlider(Qt::Horizontal);
    sliderRedGain->setRange(0, 300);  // 0.0-3.0 * 100
    sliderRedGain->setValue(100);     // 默认1.0
    sliderRedGain->setEnabled(false);
    spinRedGain = new QDoubleSpinBox();
    spinRedGain->setRange(0.0, 3.0);
    spinRedGain->setValue(1.0);
    spinRedGain->setSingleStep(0.1);
    spinRedGain->setEnabled(false);
    redLayout->addWidget(sliderRedGain, 3);
    redLayout->addWidget(spinRedGain, 1);
    wbLayout->addRow(tr("Red:"), redLayout);

    // 绿色增益
    QHBoxLayout *greenLayout = new QHBoxLayout();
    sliderGreenGain = new QSlider(Qt::Horizontal);
    sliderGreenGain->setRange(0, 300);
    sliderGreenGain->setValue(100);
    sliderGreenGain->setEnabled(false);
    spinGreenGain = new QDoubleSpinBox();
    spinGreenGain->setRange(0.0, 3.0);
    spinGreenGain->setValue(1.0);
    spinGreenGain->setSingleStep(0.1);
    spinGreenGain->setEnabled(false);
    greenLayout->addWidget(sliderGreenGain, 3);
    greenLayout->addWidget(spinGreenGain, 1);
    wbLayout->addRow(tr("Green:"), greenLayout);

    // 蓝色增益
    QHBoxLayout *blueLayout = new QHBoxLayout();
    sliderBlueGain = new QSlider(Qt::Horizontal);
    sliderBlueGain->setRange(0, 300);
    sliderBlueGain->setValue(100);
    sliderBlueGain->setEnabled(false);
    spinBlueGain = new QDoubleSpinBox();
    spinBlueGain->setRange(0.0, 3.0);
    spinBlueGain->setValue(1.0);
    spinBlueGain->setSingleStep(0.1);
    spinBlueGain->setEnabled(false);
    blueLayout->addWidget(sliderBlueGain, 3);
    blueLayout->addWidget(spinBlueGain, 1);
    wbLayout->addRow(tr("Blue:"), blueLayout);

    wbGroup->setLayout(wbLayout);
    controlLayout->addWidget(wbGroup);

    // 4. Exposure
    QGroupBox *exposureGroup = new QGroupBox(tr("Exposure && Gain"));
    QFormLayout *exposureLayout = new QFormLayout();

    cBoxAutoExposure = new QCheckBox("Auto Exposure");
    cBoxAutoGain = new QCheckBox("Auto Gain");

    cBoxAutoExposure->setEnabled(false);
    cBoxAutoGain->setEnabled(false);
    // Exposure
    QHBoxLayout *expLayout = new QHBoxLayout();
    sliderExposure = new QSlider(Qt::Horizontal);
    sliderExposure->setRange(1, 4000);
    sliderExposure->setValue(100);
    sliderExposure->setEnabled(false);
    spinExposure = new QSpinBox();
    spinExposure->setRange(1, 4000);
    spinExposure->setValue(100);
    spinExposure->setEnabled(false);
    expLayout->addWidget(sliderExposure, 3);
    expLayout->addWidget(spinExposure, 1);
    exposureLayout->addRow(cBoxAutoExposure, cBoxAutoGain);
    exposureLayout->addRow(tr("Exposure:"), expLayout);



    // Gain
    QHBoxLayout *gainLayout = new QHBoxLayout();
    sliderGain = new QSlider(Qt::Horizontal);
    sliderGain->setRange(1, 255);
    sliderGain->setValue(50);
    sliderGain->setEnabled(false);
    spinGain = new QSpinBox();
    spinGain->setRange(1, 255);
    spinGain->setValue(50);
    spinGain->setEnabled(false);
    gainLayout->addWidget(sliderGain, 3);
    gainLayout->addWidget(spinGain, 1);

    exposureLayout->addRow(tr("Gain:"), gainLayout);

    // AETarget
    QHBoxLayout *aetargetLayout = new QHBoxLayout();
    sliderAETarget = new QSlider(Qt::Horizontal);
    sliderAETarget->setRange(1, 255);
    sliderAETarget->setValue(100);
    sliderAETarget->setEnabled(false);
    spinAETarget = new QSpinBox();
    spinAETarget->setRange(1, 255);
    spinAETarget->setValue(100);
    spinAETarget->setEnabled(false);
    aetargetLayout->addWidget(sliderAETarget, 3);
    aetargetLayout->addWidget(spinAETarget, 1);
    exposureLayout->addRow(tr("AE Target:"), aetargetLayout);


    exposureGroup->setLayout(exposureLayout);
    controlLayout->addWidget(exposureGroup);

    // 5. Image Enhancement
    QGroupBox *imageGroup = new QGroupBox(tr("Image Enhancement"));
    QFormLayout *imageLayout = new QFormLayout();

    // Gamma
    QHBoxLayout *gammaLayout = new QHBoxLayout();
    sliderGamma = new QSlider(Qt::Horizontal);
    sliderGamma->setRange(10, 300);  // 0.1-3.0 * 100
    sliderGamma->setValue(100);       // 默认1.0
    sliderGamma->setEnabled(false);
    spinGamma = new QDoubleSpinBox();
    spinGamma->setRange(0.1, 3.0);
    spinGamma->setValue(1.0);
    spinGamma->setSingleStep(0.1);
    spinGamma->setEnabled(false);
    gammaLayout->addWidget(sliderGamma, 3);
    gammaLayout->addWidget(spinGamma, 1);
    imageLayout->addRow(tr("Gamma:"), gammaLayout);

    // Contrast
    QHBoxLayout *contrastLayout = new QHBoxLayout();
    sliderContrast = new QSlider(Qt::Horizontal);
    sliderContrast->setRange(0, 200);  // 0.0-2.0 * 100
    sliderContrast->setValue(100);     // 默认1.0
    sliderContrast->setEnabled(false);
    spinContrast = new QDoubleSpinBox();
    spinContrast->setRange(0.0, 2.0);
    spinContrast->setValue(1.0);
    spinContrast->setSingleStep(0.1);
    spinContrast->setEnabled(false);
    contrastLayout->addWidget(sliderContrast, 3);
    contrastLayout->addWidget(spinContrast, 1);
    imageLayout->addRow(tr("Contrast:"), contrastLayout);

    // Saturation
    QHBoxLayout *saturationLayout = new QHBoxLayout();
    sliderSaturation = new QSlider(Qt::Horizontal);
    sliderSaturation->setRange(0, 200);     //0.0-2.0*100
    sliderSaturation->setValue(100);
    sliderSaturation->setEnabled(false);
    spinSaturation = new QDoubleSpinBox();
    spinSaturation->setRange(0.0, 2.0);
    spinSaturation->setValue(1.0);
    spinSaturation->setSingleStep(0.1);
    spinSaturation->setEnabled(false);
    saturationLayout->addWidget(sliderSaturation, 3);
    saturationLayout->addWidget(spinSaturation, 1);
    imageLayout->addRow(tr("Saturation:"), saturationLayout);


    // Black Level
    QHBoxLayout *blackLevelLayout = new QHBoxLayout();
    sliderBlackLevel = new QSlider(Qt::Horizontal);
    sliderBlackLevel->setRange(0, 255);             // 0--255
    sliderBlackLevel->setValue(100);
    sliderBlackLevel->setEnabled(false);
    spinBlackLevel = new QSpinBox();
    spinBlackLevel->setRange(0, 255);
    spinBlackLevel->setSingleStep(1);
    spinBlackLevel->setValue(100);
    spinBlackLevel->setEnabled(false);
    blackLevelLayout->addWidget(sliderBlackLevel, 3);
    blackLevelLayout->addWidget(spinBlackLevel, 1);
    imageLayout->addRow(tr("Black Level:"), blackLevelLayout);




    imageGroup->setLayout(imageLayout);
    controlLayout->addWidget(imageGroup);

    controlLayout->addStretch();  // 底部伸展

    // 组装主布局
    mainLayout->addLayout(controlLayout, 1);  // 伸展因子1

    mainWidget->setLayout(mainLayout);
    setWidget(mainWidget);
}

void CameraWidget::connectSignalsSlots()
{
    // === 相机控制器 -> UI ===
    connect(m_controller, &CameraController::cameraInitialized,
            this, &CameraWidget::onCameraInitialized);
    connect(m_controller, &CameraController::cameraInitFailed,
            this, &CameraWidget::onCameraInitFailed);
    connect(m_controller, &CameraController::newImageReady,
            this, &CameraWidget::onNewImageReady,
            Qt::QueuedConnection);

    // === UI -> 相机控制器 ===
    // 采集控制
    connect(btnStartStop, &QPushButton::clicked,
            this, &CameraWidget::onStartStopClicked);
    connect(btnSaveImage, &QPushButton::clicked,
            this, &CameraWidget::onSaveImageClicked);

    // 增益和曝光
    connect(cBoxAutoExposure, &QCheckBox::checkStateChanged, this, &CameraWidget::onAutoExposureChanged);
    connect(cBoxAutoGain, &QCheckBox::checkStateChanged, this, &CameraWidget::onAutoGainChanged);


    connect(sliderGain, &QSlider::valueChanged,
            this, &CameraWidget::onGainChanged);
    connect(spinGain, QOverload<int>::of(&QSpinBox::valueChanged),
            sliderGain, &QSlider::setValue);

    connect(sliderExposure, &QSlider::valueChanged,
            this, &CameraWidget::onExposureChanged);
    connect(spinExposure, QOverload<int>::of(&QSpinBox::valueChanged),
            sliderExposure, &QSlider::setValue);

    connect(sliderAETarget, &QSlider::valueChanged,
            this, &CameraWidget::onAETargetChanged);
    connect(spinAETarget, QOverload<int>::of(&QSpinBox::valueChanged),
            sliderAETarget, &QSlider::setValue);

    // 色彩平衡

    connect(cBoxAWB, &QCheckBox::checkStateChanged, this, &CameraWidget::onAWBChanged);

    connect(sliderRedGain, &QSlider::valueChanged,
            this, &CameraWidget::onRedGainChanged);
    connect(spinRedGain, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) { sliderRedGain->setValue(static_cast<int>(value * 100)); });

    connect(sliderGreenGain, &QSlider::valueChanged,
            this, &CameraWidget::onGreenGainChanged);
    connect(spinGreenGain, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) { sliderGreenGain->setValue(static_cast<int>(value * 100)); });

    connect(sliderBlueGain, &QSlider::valueChanged,
            this, &CameraWidget::onBlueGainChanged);
    connect(spinBlueGain, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) { sliderBlueGain->setValue(static_cast<int>(value * 100)); });

    // Gamma和对比度
    connect(sliderGamma, &QSlider::valueChanged,
            [this](int value) {
                double gamma = value / 100.0;
                spinGamma->setValue(gamma);
                onGammaChanged(gamma);
            });
    connect(spinGamma, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) { sliderGamma->setValue(static_cast<int>(value * 100)); });

    connect(sliderContrast, &QSlider::valueChanged,
            [this](int value) {
                double contrast = value / 100.0;
                spinContrast->setValue(contrast);
                onContrastChanged(contrast);
            });
    connect(spinContrast, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) { sliderContrast->setValue(static_cast<int>(value * 100)); });

    connect(sliderSaturation, &QSlider::valueChanged,
            [this](int value) {
                double sat = value / 100.0;
                spinSaturation->setValue(sat);
                onContrastChanged(sat);
            });
    connect(spinSaturation, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) { sliderSaturation->setValue(static_cast<int>(value * 100)); });


    connect(sliderBlackLevel, &QSlider::valueChanged, this, &CameraWidget::onBlackLevelChanged);

    connect(spinBlackLevel, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int value) { sliderBlackLevel->setValue(value); });
}

void CameraWidget::onStartStopClicked()
{
    if (!m_isCapturing) {
        // 开始采集
        QMetaObject::invokeMethod(m_controller,
                                  &CameraController::startCapture,
                                  Qt::QueuedConnection);
        m_isCapturing = true;
        btnStartStop->setText(tr("Stop"));
        btnSaveImage->setEnabled(true);
    } else {
        // 停止采集
        QMetaObject::invokeMethod(m_controller,
                                  &CameraController::stopCapture,
                                  Qt::QueuedConnection);
        m_isCapturing = false;
        btnStartStop->setText(tr("Start"));
    }
}

void CameraWidget::onSaveImageClicked()
{
    if (m_currentImage.isNull()) {
        QMessageBox::warning(this, tr("Warning"), tr("No image to save"));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                     tr("Save Image"),
                                                     QString(),
                                                     tr("Images (*.png *.jpg *.bmp)"));

    if (!fileName.isEmpty()) {
        if (m_currentImage.save(fileName)) {
            QMessageBox::information(this, tr("Success"), tr("Image saved successfully"));
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to save image"));
        }
    }
}

void CameraWidget::onGainChanged(int value)
{
    spinGain->setValue(value);
    QMetaObject::invokeMethod(m_controller,
                              [this, value]() {
                                  m_controller->setGain(value);
                              },
                              Qt::QueuedConnection);
}

void CameraWidget::onExposureChanged(int value)
{
    spinExposure->setValue(value);
    QMetaObject::invokeMethod(m_controller,
                              [this, value]() {
                                  m_controller->setExposure(value);
                              },
                              Qt::QueuedConnection);
}

void CameraWidget::onAutoExposureChanged(bool checked)
{
    spinExposure->setEnabled(!checked);
    sliderExposure->setEnabled(!checked);
    QMetaObject::invokeMethod(m_controller,
                              [this, checked]() {
                                m_controller->setAutoExposure(checked);
                            },
                              Qt::QueuedConnection);
}

void CameraWidget::onAutoGainChanged(bool checked)
{
    spinGain->setEnabled(!checked);
    sliderGain->setEnabled(!checked);
    QMetaObject::invokeMethod(m_controller,
                              [this, checked]() {
                                  m_controller->setAutoGain(checked);
                              },
                              Qt::QueuedConnection);
}

void CameraWidget::onAETargetChanged(int value)
{
    spinAETarget->setValue(value);
    QMetaObject::invokeMethod(m_controller,
                              [this, value]() {
                                  m_controller->setAETarget(value);
                              },
                              Qt::QueuedConnection);
}

void CameraWidget::onAWBChanged(bool checked)
{
    spinRedGain->setEnabled(!checked);
    spinBlueGain->setEnabled(!checked);
    spinGreenGain->setEnabled(!checked);
    sliderRedGain->setEnabled(!checked);
    sliderGreenGain->setEnabled(!checked);
    sliderBlueGain->setEnabled(!checked);
    QMetaObject::invokeMethod(m_controller,
                              [this, checked]() {
                                  m_controller->setAutoWhiteBalance(checked);
                              },
                              Qt::QueuedConnection);
}

void CameraWidget::onRedGainChanged(int value)
{
    double gain = value / 100.0;
    spinRedGain->setValue(gain);

    double r = gain;
    double g = spinGreenGain->value();
    double b = spinBlueGain->value();

    QMetaObject::invokeMethod(m_controller,
                              [this, r, g, b]() {
                                  m_controller->setWhiteBalance(r, g, b);
                              },
                              Qt::QueuedConnection);
}

void CameraWidget::onGreenGainChanged(int value)
{
    double gain = value / 100.0;
    spinGreenGain->setValue(gain);

    double r = spinRedGain->value();
    double g = gain;
    double b = spinBlueGain->value();

    QMetaObject::invokeMethod(m_controller,
                              [this, r, g, b]() {
                                  m_controller->setWhiteBalance(r, g, b);
                              },
                              Qt::QueuedConnection);
}

void CameraWidget::onBlueGainChanged(int value)
{
    double gain = value / 100.0;
    spinBlueGain->setValue(gain);

    double r = spinRedGain->value();
    double g = spinGreenGain->value();
    double b = gain;

    QMetaObject::invokeMethod(m_controller,
                              [this, r, g, b]() {
                                  m_controller->setWhiteBalance(r, g, b);
                              },
                              Qt::QueuedConnection);
}

void CameraWidget::onGammaChanged(double value)
{
    QMetaObject::invokeMethod(m_controller,
                              [this, value]() {
                                  m_controller->setGamma(value);
                              },
                              Qt::QueuedConnection);
}

void CameraWidget::onContrastChanged(double value)
{
    QMetaObject::invokeMethod(m_controller,
                              [this, value]() {
                                  m_controller->setContrast(value);
                              },
                              Qt::QueuedConnection);
}

void CameraWidget::onSaturationChanged(double value)
{
    QMetaObject::invokeMethod(m_controller,
                              [this, value]() {
                                  m_controller->setSaturation(value);
                              },
                              Qt::QueuedConnection);
}

void CameraWidget::onBlackLevelChanged(int value)
{
    spinBlackLevel->setValue(value);
    QMetaObject::invokeMethod(m_controller,
                              [this, value]() {
                                  m_controller->setBlackLevel(value);
                              },
                              Qt::QueuedConnection);
}

void CameraWidget::onCameraInitialized(int width, int height)
{
    lblCameraStatus->setText(tr("Ready"));
    lblCameraName->setText(QString("%1(%2)").arg(m_controller->modelName(), m_controller->name()));
    lblResolution->setText(QString("%1 x %2").arg(width).arg(height));
    m_imageLabel->setText(tr("Click 'Start' to begin capture"));

    // 启用所有控件
    btnStartStop->setEnabled(true);
    cBoxAutoExposure->setEnabled(true);
    cBoxAutoGain->setEnabled(true);



    sliderGain->setEnabled(true);
    spinGain->setEnabled(true);
    sliderExposure->setEnabled(true);
    spinExposure->setEnabled(true);
    sliderAETarget->setEnabled(true);
    spinAETarget->setEnabled(true);


    cBoxAWB->setEnabled(true);
    sliderRedGain->setEnabled(true);
    spinRedGain->setEnabled(true);
    sliderGreenGain->setEnabled(true);
    spinGreenGain->setEnabled(true);
    sliderBlueGain->setEnabled(true);
    spinBlueGain->setEnabled(true);
    sliderGamma->setEnabled(true);
    spinGamma->setEnabled(true);
    sliderContrast->setEnabled(true);
    spinContrast->setEnabled(true);
    sliderSaturation->setEnabled(true);
    spinSaturation->setEnabled(true);
    sliderBlackLevel->setEnabled(true);
    spinBlackLevel->setEnabled(true);

    qDebug() << "Camera ready:" << width << "x" << height;
}

void CameraWidget::onCameraInitFailed(QString error)
{
    lblCameraStatus->setText(tr("Failed"));
    m_imageLabel->setText(tr("Camera initialization failed:\n%1").arg(error));

    QMessageBox::warning(this, tr("Camera Error"), error);

    qWarning() << "Camera initialization failed:" << error;
}

void CameraWidget::onNewImageReady(const QImage &image)
{
    if (!image.isNull()) {
        m_currentImage = image;
        m_imageLabel->setPixmap(QPixmap::fromImage(image));
    }
}
