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
    const QString &modelName() const {
        return m_modelName;
    }
    const QString &name() const {
        return m_name;
    }


public slots:
    void initialize();           // 扫描并初始化相机
    void shutdown();             // 关闭相机
    void startCapture();         // 开始采集
    void stopCapture();          // 停止采集
    void setGain(int gain);      // 设置增益（1-255）
    void setExposure(int exposure);
    void setAutoExposure(bool checked);
    void setAutoGain(bool checked);
    void setAETarget(int target);
    void setAutoWhiteBalance(bool checked);
    void setWhiteBalance(double r, double g, double b);
    void setGamma(double gamma);
    void setContrast(double contrast);
    void setSaturation(double saturation);
    void setBlackLevel(int blackLevel);

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
    int m_modelId;
    int m_productId;
    QString m_name;
    QString m_modelName;
    bool m_isInitialized;        // 相机是否已初始化
    bool m_isCapturing;          // 是否正在采集
    QTimer *m_captureTimer;      // 200ms定时器

    // 图像信息
    int m_imageWidth;
    int m_imageHeight;
    int m_rawDataLen;
    unsigned char *m_imageBuffer;  // 图像数据缓冲区
    unsigned char *m_rawBuffer;
    int m_bufferSize;

    // 相机参数
    bool m_aec;                 // 自动曝光
    bool m_agc;                 // 自动增益
    int m_aeTarget;             // 自动曝光目标
    bool m_awb;                 // 自动白平衡
    int m_gain;                  // 增益 (1-255)
    int m_exposure;              // 曝光值
    double m_redGain;            // 红色增益
    double m_greenGain;          // 绿色增益
    double m_blueGain;           // 蓝色增益
    double m_gamma;              // Gamma
    double m_contrast;           // 对比度
    double m_saturation;         // 饱和度
    int    m_blackLevel;
};

#endif // CAMERACONTROLLER_H
