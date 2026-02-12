#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QImage>

class CameraController : public QObject
{
    Q_OBJECT

public:
    explicit CameraController(QObject *parent = nullptr);
    ~CameraController();

public slots:
    void initialize();           // 扫描并初始化相机
    void shutdown();             // 关闭相机
    void startCapture();         // 开始采集
    void stopCapture();          // 停止采集
    void setGain(int gain);      // 设置增益（1-255）
    void setExposure(int exposure);
    void setWhiteBalance(double r, double g, double b);
    void setGamma(double gamma);
    void setContrast(double contrast);

signals:
    void cameraInitialized(int width, int height);
    void cameraInitFailed(QString error);
    void newImageReady(const QImage &image);
    void parameterUpdated(QString paramName);

private slots:
    void captureFrame();         // 定时器触发的采集

private:
    bool initializeCamera();     // 内部初始化逻辑
    QImage convertToQImage(unsigned char *data, int width, int height);

    // 相机状态
    int m_deviceId;              // 设备ID
    bool m_isInitialized;        // 相机是否已初始化
    bool m_isCapturing;          // 是否正在采集
    QTimer *m_captureTimer;      // 200ms定时器

    // 图像信息
    int m_imageWidth;
    int m_imageHeight;
    unsigned char *m_imageBuffer;  // 图像数据缓冲区
    int m_bufferSize;

    // 相机参数
    int m_gain;                  // 增益 (1-255)
    int m_exposure;              // 曝光值
    double m_redGain;            // 红色增益
    double m_greenGain;          // 绿色增益
    double m_blueGain;           // 蓝色增益
    double m_gamma;              // Gamma
    double m_contrast;           // 对比度
};

#endif // CAMERACONTROLLER_H
