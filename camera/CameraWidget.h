#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QDockWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QThread>
#include <QCheckBox>

class CameraController;

class CameraWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit CameraWidget(const QString &title, QWidget *parent = nullptr);
    ~CameraWidget();

private:
    void initDockWidget();       // 初始化UI布局
    void connectSignalsSlots();  // 连接信号槽

private slots:
    void onStartStopClicked();
    void onSaveImageClicked();
    void onGainChanged(int value);
    void onExposureChanged(int value);

    void onAutoExposureChanged(bool checked);
    void onAutoGainChanged(bool checked);
    void onAETargetChanged(int value);


    void onAWBChanged(bool checked);
    void onRedGainChanged(int value);
    void onGreenGainChanged(int value);
    void onBlueGainChanged(int value);


    void onGammaChanged(double value);
    void onContrastChanged(double value);
    void onSaturationChanged(double value);
    void onBlackLevelChanged(int value);

    void onCameraInitialized(int width, int height);
    void onCameraInitFailed(QString error);
    void onNewImageReady(const QImage &image);

private:
    // 图像显示
    QLabel *m_imageLabel;

    // 控制按钮
    QPushButton *btnStartStop;
    QPushButton *btnSaveImage;

    // White Balance
    QCheckBox *cBoxAWB;
    QPushButton *btnOnePushWB;
    QSlider *sliderRedGain;
    QDoubleSpinBox *spinRedGain;
    QSlider *sliderGreenGain;
    QDoubleSpinBox *spinGreenGain;
    QSlider *sliderBlueGain;
    QDoubleSpinBox *spinBlueGain;

    // Exposure
    QCheckBox *cBoxAutoGain;
    QCheckBox *cBoxAutoExposure;
    QSlider *sliderExposure;
    QSpinBox *spinExposure;
    QSlider *sliderGain;
    QSpinBox *spinGain;
    QSlider *sliderAETarget;
    QSpinBox *spinAETarget;

    // Image Enhancement
    QSlider *sliderGamma;
    QDoubleSpinBox *spinGamma;
    QSlider *sliderContrast;
    QDoubleSpinBox *spinContrast;
    QSlider *sliderSaturation;
    QDoubleSpinBox *spinSaturation;
    QSlider *sliderBlackLevel;
    QSpinBox *spinBlackLevel;


    // 状态显示
    QLabel *lblCameraStatus;
    QLabel *lblResolution;
    QLabel *lblCameraName;

    // 线程模型
    QThread *m_cameraThread;
    CameraController *m_controller;

    // 状态
    bool m_isCapturing;
    QImage m_currentImage;  // 保存当前图像用于保存功能
};

#endif // CAMERAWIDGET_H
