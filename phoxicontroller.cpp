#include "phoxicontroller.h"
#include "CPhoxiCamera.h"
#include "DataX.h"  // 包含 s_Image3dS 定义
#include <QDebug>
#include <QThread>
#include <QMutexLocker>
#include <QVector3D>
#include <QDateTime>
#include <QPainter>
#include <QFont>
#include "phoxi/PhoXi.h"  // PhoXi 3D相机 SDK v1.16.2
#include "phoxi/PhoXiDataTypes.h"  // PhoXi 数据类型定义

PhoxiWorker::PhoxiWorker(const QString &cameraId, QObject *parent)
    : QObject(parent)
    , m_cameraId(cameraId)
    , m_camera(nullptr)
    , m_isConnected(false)
    , m_isCapturing(false)
    , m_timeout(1000)
    , m_resolution(1)
    , m_connectionMode(PhoxiConnectionMode::HardwareOnly)  // 默认仅硬件模式
{
}

PhoxiWorker::~PhoxiWorker()
{
    if (m_isCapturing) {
        stopCapture();
    }
    if (m_isConnected) {
        disconnectCamera();
    }
    if (m_camera) {
        delete m_camera;
        m_camera = nullptr;
    }
}

// 连接相机：使用 CPhoxiCamera 作为底层实现
void PhoxiWorker::connectCamera()
{
    // 先检查状态，避免重复连接
    {
        QMutexLocker locker(&m_mutex);
        // 如果已经连接，先断开（在锁内快速检查）
        if (m_isConnected && m_camera) {
            // 先释放锁，避免在锁内执行耗时操作
            locker.unlock();
            // 断开连接（这会重新获取锁）
            disconnectCamera();
            // 重新获取锁继续
            locker.relock();
        }
        
        // 如果正在连接中，忽略重复请求
        // 注意：这里无法检查 PhoxiStatus，因为状态在 Controller 中
        // 但可以通过 m_isConnected 和 m_camera 来判断
        if (m_camera != nullptr && m_isConnected) {
            qWarning() << "[PhoxiWorker] 相机已连接，忽略重复连接请求";
            return;
        }
    } // 释放锁，准备执行耗时操作

    // 发送连接中状态（在锁外发送，避免阻塞）
    emit statusChanged(PhoxiStatus::Connecting);

    CPhoxiCamera *newCamera = nullptr;
    bool connectionSuccess = false;
    QString errorMessage;

    try {
        // 创建相机实例（在锁外执行，避免阻塞）
        newCamera = new CPhoxiCamera();

        // 连接设备（这是耗时操作，在锁外执行）
        std::string deviceId = m_cameraId.toStdString();
        if (!newCamera->Open(deviceId)) {
            errorMessage = QString("连接失败: %1").arg(m_cameraId);
            delete newCamera;
            newCamera = nullptr;
        } else {
            connectionSuccess = true;
        }

    } catch (const std::exception &e) {
        errorMessage = QString("连接异常: %1").arg(e.what());
        if (newCamera) {
            delete newCamera;
            newCamera = nullptr;
        }
    } catch (...) {
        errorMessage = "连接过程发生未知异常";
        if (newCamera) {
            delete newCamera;
            newCamera = nullptr;
        }
    }

    // 更新状态（在锁内快速更新）
    {
        QMutexLocker locker(&m_mutex);
        
        if (connectionSuccess && newCamera) {
            // 连接成功，更新状态
            m_camera = newCamera;
            m_isConnected = true;

            // 获取设备信息（简化版本）
            PhoxiCameraInfo info;
            info.id = m_cameraId;
            info.name = m_cameraId;
            info.type = "PhoXi";
            info.serialNumber = m_cameraId;
            info.firmwareVersion = "Unknown";
            info.isConnected = true;

            // 在锁外发送信号，避免阻塞
            locker.unlock();
            emit cameraInfoUpdated(info);
            emit statusChanged(PhoxiStatus::Connected);
            emit connectionResult(true, "相机连接成功");
        } else {
            // 连接失败，清理状态
            m_isConnected = false;
            if (newCamera) {
                delete newCamera;
                newCamera = nullptr;
            }
            
            // 在锁外发送信号
            locker.unlock();
            emit errorOccurred(errorMessage.isEmpty() ? "连接失败" : errorMessage);
            emit connectionResult(false, errorMessage.isEmpty() ? "连接失败" : errorMessage);
            emit statusChanged(PhoxiStatus::Error);
        }
    }
}

void PhoxiWorker::disconnectCamera()
{
    CPhoxiCamera *cameraToClose = nullptr;
    bool wasCapturing = false;
    
    // 快速获取需要关闭的资源（在锁内）
    {
        QMutexLocker locker(&m_mutex);
        
        wasCapturing = m_isCapturing;
        if (wasCapturing) {
            m_isCapturing = false;  // 先标记为停止采集
        }
        
        cameraToClose = m_camera;
        m_camera = nullptr;  // 先清空指针，避免其他操作使用
        m_isConnected = false;
    } // 释放锁，准备执行耗时操作

    // 在锁外执行耗时操作
    if (wasCapturing) {
        // 如果正在采集，停止采集（但这里没有实际的停止操作，因为连续采集模式未实现）
        // 只是更新状态
    }

    if (cameraToClose) {
        try {
            cameraToClose->Close();
        } catch (...) {
            qWarning() << "[PhoxiWorker] 关闭相机时发生异常";
        }
        delete cameraToClose;
    }

    // 发送断开状态（在锁外发送）
    emit statusChanged(PhoxiStatus::Disconnected);
}

// 连续采集模式：暂时不支持，推荐使用 captureSingleFrame()
void PhoxiWorker::startCapture()
{
    emit errorOccurred("连续采集模式暂不支持，请使用 captureSingleFrame() 进行单次拍照");
    qWarning() << "[PhoxiWorker] 连续采集模式暂不支持";
}

void PhoxiWorker::stopCapture()
{
    QMutexLocker locker(&m_mutex);
    m_isCapturing = false;
    emit statusChanged(PhoxiStatus::Connected);
}

// 单次拍照：使用 CPhoxiCamera 的 SoftwareTrigger 方法
// 同时获取 QImage（用于界面显示）和 s_Image3dS（用于算法处理和保存）
void PhoxiWorker::captureSingleFrame()
{
    QMutexLocker locker(&m_mutex);

    // 检查设备连接状态
    if (!m_isConnected || !m_camera || !m_camera->IsOpen()) {
        emit errorOccurred("相机未连接");
        return;
    }

    locker.unlock();  // 释放锁，避免阻塞拍照

    try {
        // 1. 首先调用返回 s_Image3dS 的版本（用于算法处理和保存）
        s_Image3dS image3d;
        if (!m_camera->SoftwareTrigger(image3d, m_timeout)) {
            emit errorOccurred("拍照失败（获取3D数据）");
            return;
        }

        // 检查3D数据有效性
        if (!image3d.Gray.IsInitialized() || image3d.Gray.CountObj() == 0) {
            emit errorOccurred("3D图像数据无效（Gray未初始化）");
            return;
        }

        // 1. 优先提取灰度图用于界面显示（第一时间显示，提升用户体验）
        try {
            using namespace HalconCpp;
            
            HTuple width, height;
            GetImageSize(image3d.Gray, &width, &height);
            int w = width[0].I();
            int h = height[0].I();
            
            if (w <= 0 || h <= 0) {
                qWarning() << "[PhoxiWorker] 图像尺寸无效:" << w << h;
            } else {
                // 获取图像数据指针和类型
                HTuple pointer, type, widthVal, heightVal;
                GetImagePointer1(image3d.Gray, &pointer, &type, &widthVal, &heightVal);
                
                // 创建QImage用于界面显示（使用RGB888格式以便在界面显示）
                QImage image(w, h, QImage::Format_RGB888);
                
                // 根据数据类型转换
                if (type[0].S() == "byte") {
                    // 8位灰度图
                    Hlong ptr = pointer[0].L();
                    if (ptr != 0) {
                        const uchar *srcData = reinterpret_cast<const uchar*>(ptr);
                        for (int y = 0; y < h; ++y) {
                            uchar *scanLine = image.scanLine(y);
                            const uchar *srcLine = srcData + y * w;
                            for (int x = 0; x < w; ++x) {
                                uchar grayValue = srcLine[x];
                                scanLine[x * 3 + 0] = grayValue;
                                scanLine[x * 3 + 1] = grayValue;
                                scanLine[x * 3 + 2] = grayValue;
                            }
                        }
                    } else {
                        qWarning() << "[PhoxiWorker] 图像指针为空";
                        image.fill(128);  // 使用默认灰色图像
                    }
                } else if (type[0].S() == "real" || type[0].S() == "float") {
                    // 浮点灰度图，需要归一化到0-255
                    Hlong ptr = pointer[0].L();
                    if (ptr != 0) {
                        const float *srcData = reinterpret_cast<const float*>(ptr);
                        // 先找到最大值和最小值用于归一化
                        float minVal = srcData[0];
                        float maxVal = srcData[0];
                        for (int i = 1; i < w * h; ++i) {
                            if (srcData[i] < minVal) minVal = srcData[i];
                            if (srcData[i] > maxVal) maxVal = srcData[i];
                        }
                        float range = maxVal - minVal;
                        if (range <= 0) range = 1.0f;
                        
                        for (int y = 0; y < h; ++y) {
                            uchar *scanLine = image.scanLine(y);
                            const float *srcLine = srcData + y * w;
                            for (int x = 0; x < w; ++x) {
                                int grayValue = qBound(0, static_cast<int>((srcLine[x] - minVal) / range * 255.0f), 255);
                                scanLine[x * 3 + 0] = grayValue;
                                scanLine[x * 3 + 1] = grayValue;
                                scanLine[x * 3 + 2] = grayValue;
                            }
                        }
                    } else {
                        qWarning() << "[PhoxiWorker] 图像指针为空";
                        image.fill(128);  // 使用默认灰色图像
                    }
                } else {
                    qWarning() << "[PhoxiWorker] 不支持的图像类型:" << type[0].S().Text();
                    // 创建一个默认灰色图像
                    image.fill(128);
                }
                
                // 第一时间发送用于界面显示的图像（优先显示，提升用户体验）
                // 注意：QImage是隐式共享的，但为了确保跨线程安全，创建深拷贝
                // 这样即使工作线程中的原始数据被修改，也不会影响UI显示
                QImage imageCopy = image.copy();  // 创建深拷贝
                emit imageReady(imageCopy);
            }
            
        } catch (const HalconCpp::HException &ex) {
            qWarning() << "[PhoxiWorker] 转换图像时发生Halcon异常:" << QString::fromLocal8Bit(ex.ErrorMessage().Text());
            // 即使转换失败，仍然发送3D数据，不影响算法处理
        } catch (const std::exception &e) {
            qWarning() << "[PhoxiWorker] 转换图像时发生异常:" << e.what();
            // 即使转换失败，仍然发送3D数据，不影响算法处理
        }

        // 2. 发送3D数据信号（用于算法处理和保存）
        // 注意：在界面显示信号之后发送，确保用户能第一时间看到图像
        emit image3dReady(image3d);

    } catch (const std::exception &e) {
        emit errorOccurred(QString("拍照异常: %1").arg(e.what()));
    } catch (...) {
        emit errorOccurred("拍照未知异常");
    }
}

bool PhoxiWorker::saveLastOutput(const QString &filePath)
{
    QMutexLocker locker(&m_mutex);

    if (!m_camera || !m_camera->IsOpen()) {
        qWarning() << "[PhoxiWorker] saveLastOutput: 设备未连接，无法保存" << filePath;
        return false;
    }

    // CPhoxiCamera 没有提供 SaveLastOutput 接口，暂时不支持
    qWarning() << "[PhoxiWorker] saveLastOutput: 暂不支持此功能";
    return false;
}

void PhoxiWorker::updateSettings(int timeout, int resolution)
{
    QMutexLocker locker(&m_mutex);
    m_timeout = timeout;
    m_resolution = resolution;
    // CPhoxiCamera 内部管理超时，这里只保存参数
}

// 应用扫描预设：CPhoxiCamera 暂不支持，保留接口
void PhoxiWorker::applyPreset(ScanPreset preset)
{
    QMutexLocker locker(&m_mutex);
    // CPhoxiCamera 暂不支持预设配置
    qWarning() << "[PhoxiWorker] applyPreset: 暂不支持预设配置功能";
}

// 连续采集循环：暂不支持
void PhoxiWorker::captureLoop()
{
    qWarning() << "[PhoxiWorker] captureLoop: 连续采集模式暂不支持";
}

QImage PhoxiWorker::convertToQImage(void* data, int width, int height)
{
    // TODO: 实际的图像转换代码
    // 这里需要根据PhoXi SDK返回的数据格式进行转换
    // 例如从深度图、灰度图或RGB图转换为QImage

    QImage image(width, height, QImage::Format_RGB888);
    // 转换逻辑...
    return image;
}


PhoxiController::PhoxiController(const QString &cameraId, QObject *parent)
    : QObject(parent)
    , m_cameraId(cameraId)
    , m_status(PhoxiStatus::Disconnected)
    , m_currentPreset(ScanPreset::Normal)
    , m_workerThread(new QThread(this))
    , m_worker(nullptr)
    , m_timeout(1000)
    , m_resolution(1)
{
    // 创建工作对象并移到线程
    m_worker = new PhoxiWorker(m_cameraId);
    m_worker->moveToThread(m_workerThread);

    // 连接信号和槽（明确指定连接类型以确保线程安全）
    // Worker到Controller：使用QueuedConnection确保槽函数在主线程执行（访问UI安全）
    connect(m_worker, &PhoxiWorker::statusChanged, this, &PhoxiController::onStatusChanged, Qt::QueuedConnection);
    connect(m_worker, &PhoxiWorker::imageReady, this, &PhoxiController::onImageReady, Qt::QueuedConnection);
    connect(m_worker, &PhoxiWorker::image3dReady, this, &PhoxiController::onImage3dReady, Qt::QueuedConnection);
    connect(m_worker, &PhoxiWorker::pointCloudReady, this, &PhoxiController::onPointCloudReady, Qt::QueuedConnection);
    connect(m_worker, &PhoxiWorker::errorOccurred, this, &PhoxiController::onErrorOccurred, Qt::QueuedConnection);
    connect(m_worker, &PhoxiWorker::cameraInfoUpdated, this, &PhoxiController::onCameraInfoUpdated, Qt::QueuedConnection);
    connect(m_worker, &PhoxiWorker::connectionResult, this, &PhoxiController::onConnectionResult, Qt::QueuedConnection);
    connect(m_worker, &PhoxiWorker::logInfo, this, &PhoxiController::onLogInfo, Qt::QueuedConnection);

    // Controller到Worker：使用QueuedConnection确保槽函数在工作线程执行
    connect(this, &PhoxiController::requestConnect, m_worker, &PhoxiWorker::connectCamera, Qt::QueuedConnection);
    connect(this, &PhoxiController::requestDisconnect, m_worker, &PhoxiWorker::disconnectCamera, Qt::QueuedConnection);
    connect(this, &PhoxiController::requestStartCapture, m_worker, &PhoxiWorker::startCapture, Qt::QueuedConnection);
    connect(this, &PhoxiController::requestStopCapture, m_worker, &PhoxiWorker::stopCapture, Qt::QueuedConnection);
    connect(this, &PhoxiController::requestCaptureSingleFrame, m_worker, &PhoxiWorker::captureSingleFrame, Qt::QueuedConnection);
    connect(this, &PhoxiController::requestUpdateSettings, m_worker, &PhoxiWorker::updateSettings, Qt::QueuedConnection);
    connect(this, &PhoxiController::requestSetPreset, m_worker, &PhoxiWorker::applyPreset, Qt::QueuedConnection);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_workerThread->start();
}

PhoxiController::~PhoxiController()
{
    if (m_workerThread->isRunning()) {
        emit requestStopCapture();
        emit requestDisconnect();

        m_workerThread->quit();
        if (!m_workerThread->wait(3000)) {
            qWarning() << "[PhoxiController] 线程未能正常退出，强制终止";
            m_workerThread->terminate();
            m_workerThread->wait();
        }
    }
}

void PhoxiController::setConnectionMode(PhoxiConnectionMode mode)
{
    if (m_worker) {
        m_worker->setConnectionMode(mode);
    }
}

void PhoxiController::connectCamera()
{
    // 检查当前状态，避免重复连接
    QMutexLocker locker(&m_mutex);
    if (m_status == PhoxiStatus::Connected || m_status == PhoxiStatus::Connecting) {
        qWarning() << "[PhoxiController] 相机已连接或正在连接中，忽略重复连接请求";
        return;
    }
    locker.unlock();
    
    emit requestConnect();
}

void PhoxiController::disconnectCamera()
{
    // 检查当前状态
    QMutexLocker locker(&m_mutex);
    if (m_status == PhoxiStatus::Disconnected) {
        qWarning() << "[PhoxiController] 相机已断开，忽略重复断开请求";
        return;
    }
    locker.unlock();
    
    emit requestDisconnect();
}

void PhoxiController::startCapture()
{
    emit requestStartCapture();
}

void PhoxiController::stopCapture()
{
    emit requestStopCapture();
}

void PhoxiController::captureSingleFrame()
{
    emit requestCaptureSingleFrame();
}

void PhoxiController::captureSingleFrame3d()
{
    // captureSingleFrame3d 是 captureSingleFrame 的别名，功能相同
    emit requestCaptureSingleFrame();
}

void PhoxiController::triggerFrame()
{
    // triggerFrame 是 captureSingleFrame 的别名，功能相同
    emit requestCaptureSingleFrame();
}

bool PhoxiController::saveLastOutput(const QString &filePath)
{
    if (!m_worker) {
        qWarning() << "[PhoxiController] saveLastOutput: worker未初始化";
        return false;
    }

    bool result = false;
    QMetaObject::invokeMethod(
        m_worker,
        "saveLastOutput",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(bool, result),
        Q_ARG(QString, filePath));
    return result;
}

void PhoxiController::setTimeout(int timeout)
{
    QMutexLocker locker(&m_mutex);
    m_timeout = timeout;
    emit requestUpdateSettings(m_timeout, m_resolution);
}

void PhoxiController::setResolution(int resolution)
{
    QMutexLocker locker(&m_mutex);
    m_resolution = resolution;
    emit requestUpdateSettings(m_timeout, m_resolution);
}

// 设置扫描预设
void PhoxiController::setScanPreset(ScanPreset preset)
{
    QMutexLocker locker(&m_mutex);
    m_currentPreset = preset;
    emit requestSetPreset(preset);
}

// 获取当前预设
ScanPreset PhoxiController::getCurrentPreset() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentPreset;
}

// 获取预设名称
QString PhoxiController::getPresetName(ScanPreset preset)
{
    switch (preset) {
    case ScanPreset::Normal:
        return "正常环境";
    case ScanPreset::StrongAmbientLight:
        return "强光环境";
    default:
        return "未知";
    }
}

QVector<PhoxiCameraInfo> PhoxiController::getAvailableCameras()
{
    QVector<PhoxiCameraInfo> cameras;

    try {
        pho::api::PhoXiFactory factory;

        if (!factory.isPhoXiControlRunning()) {
            qWarning() << "[PhoxiController] PhoXi Control未运行";
            return cameras;
        }

        std::vector<pho::api::PhoXiDeviceInformation> deviceList = factory.GetDeviceList();

        for (const auto& deviceInfo : deviceList) {
            PhoxiCameraInfo info;
            info.id = QString::fromStdString(deviceInfo.HWIdentification);
            info.name = QString::fromStdString(deviceInfo.Name);
            info.type = QString::fromStdString(deviceInfo.Type);
            info.serialNumber = QString::fromStdString(deviceInfo.HWIdentification);
            info.firmwareVersion = QString::fromStdString(deviceInfo.FirmwareVersion);
            info.isConnected = deviceInfo.Status.Ready;
            cameras.append(info);
        }

    } catch (const std::exception &e) {
        qCritical() << "[PhoxiController] 枚举相机异常:" << e.what();
    }

    return cameras;
}

// 槽函数实现
void PhoxiController::onStatusChanged(PhoxiStatus status)
{
    QMutexLocker locker(&m_mutex);
    m_status = status;
    locker.unlock();

    emit statusChanged(status);
}

void PhoxiController::onImageReady(const QImage &image)
{
    emit imageReady(image);
}

void PhoxiController::onImage3dReady(const s_Image3dS &image3d)
{
    emit image3dReady(image3d);
}

void PhoxiController::onPointCloudReady(const QVector<QVector3D> &pointCloud)
{
    emit pointCloudReady(pointCloud);
}

void PhoxiController::onErrorOccurred(const QString &error)
{
    emit errorOccurred(error);
}

void PhoxiController::onCameraInfoUpdated(const PhoxiCameraInfo &info)
{
    QMutexLocker locker(&m_mutex);
    m_cameraInfo = info;
    locker.unlock();

    emit cameraInfoUpdated(info);
}

void PhoxiController::onConnectionResult(bool success, const QString &message)
{
    emit connectionResult(success, message);
}

void PhoxiController::onLogInfo(const QString &message)
{
    emit logInfo(message);
}

