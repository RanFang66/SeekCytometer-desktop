#include "CameraController.h"
#include "../camera_lib/ubuntu_x64/JHCap.h"
#include <QDebug>

CameraController::CameraController(QObject *parent)
    : QObject(parent),
    m_deviceId(0),
    m_name("-"),
    m_modelName("-"),
    m_modelId(0),
    m_productId(0),
    m_isInitialized(false),
    m_isCapturing(false),
    m_aec(false),
    m_awb(false),
    m_agc(false),
    m_aeTarget(100),
    m_captureTimer(new QTimer(this)),
    m_imageWidth(0),
    m_imageHeight(0),
    m_imageBuffer(nullptr),
    m_bufferSize(0),
    m_gain(5),
    m_exposure(20),
    m_redGain(1.0),
    m_greenGain(1.0),
    m_blueGain(1.0),
    m_gamma(1.0),
    m_contrast(1.0),
    m_saturation(1.0),
    m_blackLevel(1)
{
    // 设置定时器间隔为500ms（2fps）
    m_captureTimer->setInterval(500);
    connect(m_captureTimer, &QTimer::timeout, this, &CameraController::captureFrame);
}

CameraController::~CameraController()
{
    shutdown();
}

void CameraController::initialize()
{
    qDebug() << "CameraController::initialize() - Scanning for cameras...";

    if (initializeCamera()) {
        qDebug() << "Camera initialized successfully:" << m_imageWidth << "x" << m_imageHeight;
        emit cameraInitialized(m_imageWidth, m_imageHeight);
    }
}

bool CameraController::initializeCamera()
{
    int count = 0;
    API_STATUS ret = CameraGetCount(&count);


    // Check connected camera count
    if (ret != API_OK) {
        QString error = QString("Failed to scan cameras: error code %1").arg(ret);
        qWarning() << error;
        emit cameraInitFailed(error);
        return false;
    }

    qDebug() << "Found" << count << "camera(s)";

    if (count == 0) {
        QString error = "No camera found. Please connect a camera and restart.";
        qWarning() << error;
        emit cameraInitFailed(error);
        return false;
    }

    // Get camera information
    char name[64];
    char modelName[64];
    ret = CameraGetName(m_deviceId, name, modelName);
    if (ret == API_OK) {
        m_name = QString(name);
        m_modelName = QString(modelName);
        qDebug() << "Get camera name: " << m_name << m_modelName;
    }

    ret = CameraGetID(m_deviceId, &m_modelId, &m_productId);
    if (ret == API_OK) {
        qDebug() << "Get camera id:" << m_modelId << m_productId;
    }



    // 初始化第一个相机
    m_deviceId = 0;
    CameraFree(m_deviceId);
    ret = CameraInit(m_deviceId);
    if (ret != API_OK) {
        QString error = QString("Failed to initialize camera: error code %1").arg(ret);
        qWarning() << error;
        emit cameraInitFailed(error);
        return false;
    }



    // 获取图像尺寸
    ret = CameraGetImageSize(m_deviceId, &m_imageWidth, &m_imageHeight);
    if (ret != API_OK) {
        QString error = QString("Failed to get image size: error code %1").arg(ret);
        qWarning() << error;
        CameraFree(m_deviceId);
        emit cameraInitFailed(error);
        return false;
    }

    // 设置图像格式为BMP（BGR格式）
    // ret = CameraSetOption(m_deviceId, CAMERA_IMAGE_BMP);
    // if (ret != API_OK) {
    //     qWarning() << "Failed to set image format, using default";
    // }


    ret = CameraGetImageBufferSize(m_deviceId, &m_rawDataLen, CAMERA_IMAGE_RAW8);
    if (ret == API_OK && m_rawDataLen > 0) {
        m_rawBuffer = new unsigned char[m_rawDataLen];
        qDebug() << "Get Image RAW Data length Ok, len = " << m_rawDataLen;
    } else {
        qWarning() << "Get Image buffer size failed!" << ret;
    }
    ret = CameraGetISPImageBufferSize(m_deviceId, &m_bufferSize, m_imageWidth, m_imageHeight, CAMERA_IMAGE_RGB24);
    if (ret == API_OK && m_bufferSize > 0) {
        m_imageBuffer = new unsigned char[m_bufferSize];
        qDebug() << "Get ISP Image buffer size ok, size = " << m_bufferSize;
    } else {
        m_bufferSize = m_imageWidth * m_imageHeight * 3;
        m_imageBuffer = new unsigned char[m_bufferSize];
        qDebug() << "Image buffer allocated (estimated):" << m_bufferSize << "bytes";
    }
    // 分配图像缓冲区
    // ret = CameraGetImageBufferSize(m_deviceId, &m_bufferSize, CAMERA_IMAGE_RGB24);
    // if (ret == API_OK && m_bufferSize > 0) {
    //     m_imageBuffer = new unsigned char[m_bufferSize];
    //     qDebug() << "Image buffer allocated:" << m_bufferSize << "bytes";
    // } else {
    //     // 如果获取失败，使用估算值
    //     m_bufferSize = m_imageWidth * m_imageHeight * 3;
    //     m_imageBuffer = new unsigned char[m_bufferSize];
    //     qDebug() << "Image buffer allocated (estimated):" << m_bufferSize << "bytes";
    // }

    // 设置初始参数
    CameraSetTimeout(m_deviceId, 2000);
    CameraSetHighspeed(m_deviceId, false);
    CameraSetFrameRate(m_deviceId, false, 10);
    CameraSetGain(m_deviceId, m_gain);
    CameraSetAEC(m_deviceId, m_aec);
    CameraSetAGC(m_deviceId, m_agc);
    CameraSetAWB(m_deviceId, m_awb);
    CameraSetAETarget(m_deviceId, m_aeTarget);
    CameraSetExposure(m_deviceId, m_exposure);
    CameraSetWBGain(m_deviceId, m_redGain, m_greenGain, m_blueGain);
    CameraSetGamma(m_deviceId, m_gamma);
    CameraSetContrast(m_deviceId, m_contrast);
    CameraSetSaturation(m_deviceId, m_saturation);
    CameraSetBlackLevel(m_deviceId, m_blackLevel);
    m_isInitialized = true;
    return true;
}

void CameraController::shutdown()
{
    qDebug() << "CameraController::shutdown()";

    if (m_isCapturing) {
        stopCapture();
    }

    if (m_isInitialized) {
        CameraFree(m_deviceId);
        m_isInitialized = false;
        qDebug() << "Camera released";
    }

    if (m_imageBuffer) {
        delete[] m_imageBuffer;
        m_imageBuffer = nullptr;
        m_bufferSize = 0;
    }

    if (m_rawBuffer) {
        delete[] m_rawBuffer;
        m_rawBuffer = nullptr;
        m_rawDataLen = 0;
    }
}

void CameraController::startCapture()
{
    if (!m_isInitialized) {
        qWarning() << "Cannot start capture: camera not initialized";
        return;
    }

    if (m_isCapturing) {
        qDebug() << "Capture already started";
        return;
    }

    qDebug() << "Starting image capture at 5fps";
    m_isCapturing = true;
    m_captureTimer->start();
}

void CameraController::stopCapture()
{
    if (!m_isCapturing) {
        return;
    }

    qDebug() << "Stopping image capture";
    m_captureTimer->stop();
    m_isCapturing = false;
}

void CameraController::captureFrame()
{
    if (!m_isInitialized || !m_imageBuffer) {
        return;
    }


    // int width, height, len, size;


    // CameraGetImageSize(m_deviceId, &width, &height);
    // CameraGetImageBufferSize(m_deviceId, &len, CAMERA_IMAGE_RAW8);
    // CameraGetISPImageBufferSize(m_deviceId, &size, width, height, CAMERA_IMAGE_RGB24);
    // unsigned char *buffer = new unsigned char[len];
    // unsigned char *ISPbuf = new unsigned char[size];
    API_STATUS ret = CameraQueryImage(m_deviceId, m_rawBuffer, &m_rawDataLen, CAMERA_IMAGE_RAW8);
    if(ret == API_OK)
    {

        // QImage img(buffer, width, height, QImage::Format_Indexed8);
        // img.setColorTable(grayColourTable);
        if(CameraISP(m_deviceId, m_rawBuffer, m_imageBuffer, m_imageWidth, m_imageHeight, CAMERA_IMAGE_RGB24)==API_OK)
        {
            QImage img(m_imageBuffer, m_imageWidth, m_imageHeight, QImage::Format_RGB888);
            // qint64 ts = QDateTime::currentMSecsSinceEpoch();
            // emit captured(img, ISPbuf, ts);
            emit newImageReady(img);
        }

    } else if (ret != API_OK) {
        int lastErr = 0;
        CameraGetLastError(&lastErr);
        qWarning() << "Failed to query image: error code" << lastErr << ret;
    }




    // int length = 0;
    // API_STATUS ret = CameraQueryImage(m_deviceId, m_imageBuffer, &length, CAMERA_IMAGE_RGB24 | CAMERA_IMAGE_BGR);

    // if (ret == API_OK && length > 0) {
    //     QImage image = convertToQImage(m_imageBuffer, m_imageWidth, m_imageHeight);
    //     if (!image.isNull()) {
    //         emit newImageReady(image);
    //     }
    // } else if (ret != API_OK) {
    //     int lastErr = 0;
    //     CameraGetLastError(&lastErr);
    //     qWarning() << "Failed to query image: error code" << lastErr << ret << length;
    // }
}

QImage CameraController::convertToQImage(unsigned char *data, int width, int height)
{
    if (!data || width <= 0 || height <= 0) {
        return QImage();
    }

    // SDK返回BGR格式，创建QImage并转换为RGB
    QImage image(data, width, height, width * 3, QImage::Format_RGB888);

    // rgbSwapped()将BGR转换为RGB
    return image.rgbSwapped().copy();  // copy()确保数据独立
}

void CameraController::setGain(int gain)
{
    if (gain < 1 || gain > 255) {
        qWarning() << "Invalid gain value:" << gain << "(valid range: 1-255)";
        return;
    }

    if (!m_isInitialized) {
        qWarning() << "Camera not initialized";
        return;
    }

    m_gain = gain;
    API_STATUS ret = CameraSetGain(m_deviceId, gain);

    if (ret == API_OK) {
        qDebug() << "Gain set to:" << gain;
        emit parameterUpdated("Gain");
    } else {
        qWarning() << "Failed to set gain:" << ret;
    }
}

void CameraController::setExposure(int exposure)
{
    if (exposure < 1) {
        qWarning() << "Invalid exposure value:" << exposure;
        return;
    }

    if (!m_isInitialized) {
        qWarning() << "Camera not initialized";
        return;
    }

    m_exposure = exposure;
    API_STATUS ret = CameraSetExposure(m_deviceId, exposure);

    if (ret == API_OK) {
        qDebug() << "Exposure set to:" << exposure;
        emit parameterUpdated("Exposure");
    } else {
        qWarning() << "Failed to set exposure:" << ret;
    }
}

void CameraController::setAutoExposure(bool checked)
{
    if (!m_isInitialized) {
        qWarning() << "Camera not initialized";
        return;
    }

    m_aec = checked;
    API_STATUS ret = CameraSetAEC(m_deviceId, checked);
    QString str = checked ? "Set Auto Exposure" : "Unset Auto Exposure";
    if (ret == API_OK) {
        qDebug() << str;
        emit parameterUpdated("AEC");
    } else {
        qWarning() << "Failed to " << str << ret;
    }
}

void CameraController::setAutoGain(bool checked)
{
    if (!m_isInitialized) {
        qWarning() << "Camera not initialized";
        return;
    }

    m_agc = checked;
    API_STATUS ret = CameraSetAGC(m_deviceId, checked);
    QString str = checked ? "Set Auto Gain" : "Unset Auto Gain";
    if (ret == API_OK) {
        qDebug() << str;
        emit parameterUpdated("AGC");
    } else {
        qWarning() << "Failed to " << str << ret;
    }
}

void CameraController::setAETarget(int target)
{
    if (target < 1 || target > 255) {
        qWarning() << "Invalid Auto Exposure Target:" << target;
        return;
    }

    if (!m_isInitialized) {
        qWarning() << "Camera not initialized";
        return;
    }

    m_aeTarget = target;
    API_STATUS ret = CameraSetAETarget(m_deviceId, target);
    if (ret == API_OK) {
        qDebug() << "Auto Exposure Target Set To " << target;
        emit parameterUpdated("AETarget");
    } else {
        qWarning() << "Failed to Set Auto Exposure Target" << ret;
    }
}

void CameraController::setAutoWhiteBalance(bool checked)
{
    if (!m_isInitialized) {
        qWarning() << "Camera not initialized";
        return;
    }

    m_awb = checked;
    API_STATUS ret = CameraSetAWB(m_deviceId, checked);
    QString str = checked ? "Set Auto White Balance" : "Unset Auto White Balance";
    if (ret == API_OK) {
        qDebug() << str;
        emit parameterUpdated("AWB");
    } else {
        qWarning() << "Failed to " << str << ret;
    }
}

void CameraController::setWhiteBalance(double r, double g, double b)
{
    if (r < 0.0 || r > 3.0 || g < 0.0 || g > 3.0 || b < 0.0 || b > 3.0) {
        qWarning() << "Invalid white balance values:" << r << g << b << "(valid range: 0.0-3.0)";
        return;
    }

    if (!m_isInitialized) {
        qWarning() << "Camera not initialized";
        return;
    }

    m_redGain = r;
    m_greenGain = g;
    m_blueGain = b;

    API_STATUS ret = CameraSetWBGain(m_deviceId, r, g, b);

    if (ret == API_OK) {
        qDebug() << "White balance set to: R=" << r << "G=" << g << "B=" << b;
        emit parameterUpdated("WhiteBalance");
    } else {
        qWarning() << "Failed to set white balance:" << ret;
    }
}

void CameraController::setGamma(double gamma)
{
    if (gamma < 0.1 || gamma > 10.0) {
        qWarning() << "Invalid gamma value:" << gamma << "(valid range: 0.1-10.0)";
        return;
    }

    if (!m_isInitialized) {
        qWarning() << "Camera not initialized";
        return;
    }

    m_gamma = gamma;
    API_STATUS ret = CameraSetGamma(m_deviceId, gamma);

    if (ret == API_OK) {
        qDebug() << "Gamma set to:" << gamma;
        emit parameterUpdated("Gamma");
    } else {
        qWarning() << "Failed to set gamma:" << ret;
    }
}

void CameraController::setContrast(double contrast)
{
    if (contrast < 0.0 || contrast > 2.0) {
        qWarning() << "Invalid contrast value:" << contrast << "(valid range: 0.0-2.0)";
        return;
    }

    if (!m_isInitialized) {
        qWarning() << "Camera not initialized";
        return;
    }

    m_contrast = contrast;
    API_STATUS ret = CameraSetContrast(m_deviceId, contrast);

    if (ret == API_OK) {
        qDebug() << "Contrast set to:" << contrast;
        emit parameterUpdated("Contrast");
    } else {
        qWarning() << "Failed to set contrast:" << ret;
    }
}

void CameraController::setSaturation(double saturation)
{
    if (saturation < 0.0 || saturation > 2.0) {
        qWarning() << "Invalid saturation value:" << saturation << "(valid range: 0.0-2.0)";
        return;
    }

    if (!m_isInitialized) {
        qWarning() << "Camera not initialized";
        return;
    }

    m_saturation = saturation;
    API_STATUS ret = CameraSetSaturation(m_deviceId, saturation);

    if (ret == API_OK) {
        qDebug() << "Saturation set to:" << saturation;
        emit parameterUpdated("Saturation");
    } else {
        qWarning() << "Failed to set Saturation:" << ret;
    }
}

void CameraController::setBlackLevel(int blackLevel)
{
    if (blackLevel < 0 || blackLevel > 255) {
        qWarning() << "Invalid gain value:" << blackLevel << "(valid range: 1-255)";
        return;
    }

    if (!m_isInitialized) {
        qWarning() << "Camera not initialized";
        return;
    }

    m_blackLevel = blackLevel;
    API_STATUS ret = CameraSetBlackLevel(m_deviceId, blackLevel);

    if (ret == API_OK) {
        qDebug() << "Black Level Set to:" << blackLevel;
        emit parameterUpdated("BlackLevel");
    } else {
        qWarning() << "Failed to set Black Level:" << ret;
    }
}
