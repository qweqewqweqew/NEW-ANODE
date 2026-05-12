#include "radarcontroller.h"
#include <QDebug>
#include <QThread>
#include <QApplication>
#include <QDateTime>
#include <QtConcurrent>
#include <chrono>

// 扫描数据结果结构（用于异步调用）
struct ScanDataResult {
    int pointCloudResult;
    Double3DPointVec pointCloud;
    int profileResult;
    std::vector<Point3D> profile;
    bool hasError;
    QString errorMessage;
    
    ScanDataResult() : pointCloudResult(0), profileResult(0), hasError(false) {}
};

// 状态检查结果结构（用于异步调用）
struct StatusCheckResult {
    int statusResult;
    Common3DPlatformStatus platformStatus;
    DeviceInfo deviceInfo;
    bool hasError;
    QString errorMessage;
    
    StatusCheckResult() : statusResult(0), platformStatus(eIdleStatus), hasError(false) {}
};

RadarController::RadarController(QObject *parent)
    : QObject(parent)
    , m_platform(std::make_unique<ThreeDPlatform>())
    , m_status(RadarStatus::Disconnected)
    , m_deviceType(EPlatFormType::PICOSCAN_MODE2)  // Mini云台 PicoScan摆动模式
    , m_platformStatus(eIdleStatus)
    , m_scanTimer(new QTimer(this))
    , m_statusCheckTimer(new QTimer(this))
    , m_dataStreamTimer(new QTimer(this))
    , m_connectWatcher(new QFutureWatcher<bool>(this))
    , m_scanTaskRunning(false)
    , m_dataStreamTaskRunning(false)
    , m_statusCheckTaskRunning(false)
    , m_autoScan(false)
    , m_scanInterval(1000)  // 1秒扫描一次
    , m_dataStreamInterval(100)  // 100ms数据流间隔
    , m_dataStreamActive(false)
    , m_maxCacheSize(100)  // 最大缓存100帧数据
    , m_cacheIndex(0)
    , m_startAngle(85.0f)
    , m_stopAngle(115.0f)
    , m_velocity(2.0f)
{
    // 连接定时器
    connect(m_scanTimer, &QTimer::timeout, this, &RadarController::onScanTimer);
    connect(m_statusCheckTimer, &QTimer::timeout, this, &RadarController::onStatusCheckTimer);
    connect(m_dataStreamTimer, &QTimer::timeout, this, &RadarController::onDataStreamTimer);
    
    // 连接异步连接完成信号
    connect(m_connectWatcher, &QFutureWatcher<bool>::finished, this, [this]() {
        bool success = m_connectWatcher->result();
        if (success) {
            updateStatus(RadarStatus::Connected);
            qDebug() << "雷达连接成功（异步）";
            
            // 设置默认扫描参数
            setScanParameters(m_startAngle, m_stopAngle, m_velocity);
            
            // 如果启用自动扫描，开始定时扫描
            if (m_autoScan) {
                m_scanTimer->start(m_scanInterval);
            }
        } else {
            // 连接失败，更新状态并发送错误信号
            updateStatus(RadarStatus::Error);
            QString errorMsg = "雷达连接失败：无法连接到设备，请检查网络连接和设备状态";
            emit errorOccurred(errorMsg);
            qDebug() << "雷达连接失败（异步）:" << errorMsg;
        }
    });
    
    // 启动状态检查定时器（每500ms检查一次）
    m_statusCheckTimer->start(500);
    
    // 初始化数据缓存
    m_pointCloudCache.resize(m_maxCacheSize);
    m_profileCache.resize(m_maxCacheSize);
}

RadarController::~RadarController()
{
    disconnectRadar();
}

bool RadarController::connectRadar(const QString &platformIP, const QString &lidarIP, EPlatFormType deviceType)
{
    if (m_status == RadarStatus::Connecting || m_status == RadarStatus::Connected) {
        emit errorOccurred("雷达正在连接或已连接");
        return false;
    }

    updateStatus(RadarStatus::Connecting);
    
    m_platformIP = platformIP;
    m_lidarIP = lidarIP;
    m_deviceType = deviceType;

    // 使用 QtConcurrent 在后台线程执行连接操作
    QFuture<bool> future = QtConcurrent::run([this, platformIP, lidarIP, deviceType]() -> bool {
        try {
            Common3DPlatformError error;
            std::string platformAddr = platformIP.toStdString();
            std::string lidarAddr = lidarIP.toStdString();
            
            qDebug() << "[异步连接] 开始连接雷达:" << platformIP << lidarIP;
            bool success = m_platform->Init(error, deviceType, lidarAddr, platformAddr);
            
            if (success) {
                qDebug() << "[异步连接] 雷达初始化成功";
                // 更新设备信息
                updateDeviceInfo();
                return true;
            } else {
                QString errorMsg = getErrorString(error);
                qDebug() << "[异步连接] 雷达初始化失败:" << errorMsg;
                // 注意：在后台线程中不能直接 emit 信号，需要通过 QMetaObject::invokeMethod 或信号队列
                // 这里先记录错误，在 finished 信号处理中发送
                return false;
            }
        } catch (const std::exception &e) {
            QString errorMsg = QString("雷达连接异常: %1").arg(e.what());
            qDebug() << "[异步连接] 异常:" << errorMsg;
            // 注意：在后台线程中不能直接 emit 信号
            return false;
        }
    });
    
    m_connectWatcher->setFuture(future);
    return true;  // 返回true表示已经开始连接
}

void RadarController::disconnectRadar()
{
    if (m_status == RadarStatus::Disconnected) {
        return;
    }

    // 停止扫描
    stopScan();
    
    // 断开连接
    if (m_platform) {
        m_platform->Destory();
    }
    
    updateStatus(RadarStatus::Disconnected);
    qDebug() << "雷达已断开连接";
}

bool RadarController::startScan()
{
    if (m_status != RadarStatus::Connected) {
        emit errorOccurred("雷达未连接，无法开始扫描");
        return false;
    }

    try {
        Common3DPlatformError error;
        bool success = m_platform->Start3DScanOnce(error);
        
        if (success) {
            updateStatus(RadarStatus::Scanning);
            qDebug() << "开始单次扫描，云台开始旋转...";
            
            // 启动扫描定时器，定期检查扫描状态并获取数据
            // 每500ms检查一次是否有新的点云数据
            if (!m_scanTimer->isActive()) {
                m_scanTimer->start(500);  // 500ms间隔
                qDebug() << "启动扫描监控定时器（500ms间隔）";
            }
            
            return true;
        } else {
            QString errorMsg = QString("开始扫描失败: %1").arg(QString::fromStdString(error.prompt));
            emit errorOccurred(errorMsg);
            qDebug() << errorMsg;
            return false;
        }
    } catch (const std::exception &e) {
        QString errorMsg = QString("扫描异常: %1").arg(e.what());
        emit errorOccurred(errorMsg);
        qDebug() << errorMsg;
        return false;
    }
}

bool RadarController::stopScan()
{
    if (m_status != RadarStatus::Scanning) {
        return true;
    }

    // 停止自动扫描定时器
    m_scanTimer->stop();
    
    updateStatus(RadarStatus::Connected);
    qDebug() << "停止扫描";
    return true;
}

bool RadarController::setScanParameters(float startAngle, float stopAngle, float velocity)
{
    if (m_status == RadarStatus::Disconnected) {
        return false;
    }

    try {
        Common3DPlatformError error;
        bool success = m_platform->SetScanParameter(startAngle, stopAngle, velocity, error);
        
        if (success) {
            m_startAngle = startAngle;
            m_stopAngle = stopAngle;
            m_velocity = velocity;
            qDebug() << QString("扫描参数设置成功: 起始角度=%1, 结束角度=%2, 速度=%3")
                        .arg(startAngle).arg(stopAngle).arg(velocity);
            return true;
        } else {
            QString errorMsg = QString("设置扫描参数失败: %1").arg(QString::fromStdString(error.prompt));
            emit errorOccurred(errorMsg);
            qDebug() << errorMsg;
            return false;
        }
    } catch (const std::exception &e) {
        QString errorMsg = QString("设置参数异常: %1").arg(e.what());
        emit errorOccurred(errorMsg);
        qDebug() << errorMsg;
        return false;
    }
}

PointCloud RadarController::getLatestPointCloud()
{
    QMutexLocker locker(&m_dataMutex);
    return m_latestPointCloud;
}

std::vector<Point3D> RadarController::getLatestProfile()
{
    QMutexLocker locker(&m_dataMutex);
    return m_latestProfile;
}

QString RadarController::getStatusString() const
{
    switch (m_status) {
    case RadarStatus::Disconnected:
        return "未连接";
    case RadarStatus::Connecting:
        return "连接中";
    case RadarStatus::Connected:
        return "已连接";
    case RadarStatus::Scanning:
        return "扫描中";
    case RadarStatus::Error:
        return "错误";
    default:
        return "未知";
    }
}

void RadarController::onScanTimer()
{
    if (m_status != RadarStatus::Connected && m_status != RadarStatus::Scanning) {
        m_scanTimer->stop();
        return;
    }
    
    // 如果上一次异步任务还在运行，跳过本次调用
    if (m_scanTaskRunning) {
        qDebug() << "扫描任务正在执行中，跳过本次调用";
        return;
    }
    
    m_scanTaskRunning = true;
    
    // 异步执行SDK调用
    QFuture<ScanDataResult> future = QtConcurrent::run([this]() -> ScanDataResult {
        ScanDataResult result;
        
        try {
            // 使用互斥锁保护SDK调用（SDK可能不是线程安全的）
            QMutexLocker locker(&m_sdkMutex);
            
            // 获取最新点云数据
            result.pointCloudResult = m_platform->GetNewest3DPointCloud(result.pointCloud);
            
            // 获取单帧轮廓数据
            result.profileResult = m_platform->GetSingleProfile(result.profile, 1);
            
        } catch (const std::exception &e) {
            result.hasError = true;
            result.errorMessage = QString("获取数据异常: %1").arg(e.what());
        }
        
        return result;
    });
    
    // 使用QFutureWatcher监控任务完成
    QFutureWatcher<ScanDataResult> *watcher = new QFutureWatcher<ScanDataResult>(this);
    connect(watcher, &QFutureWatcher<ScanDataResult>::finished, this, [this, watcher]() {
        m_scanTaskRunning = false;
        
        ScanDataResult result = watcher->result();
        watcher->deleteLater();
        
        // 处理点云数据结果
        if (result.hasError) {
            emit errorOccurred(result.errorMessage);
            qDebug() << result.errorMessage;
            m_scanTimer->stop();
            updateStatus(RadarStatus::Error);
            return;
        }
        
        if (result.pointCloudResult == 1) {
            // 成功获取到点云数据，表示扫描完成
            PointCloud pointCloud = result.pointCloud;
            
            {
                QMutexLocker locker(&m_dataMutex);
                m_latestPointCloud = pointCloud;
            }
            
            // 统计点数
            int totalPoints = 0;
            for (const auto &scanLine : pointCloud) {
                totalPoints += scanLine.size();
            }
            
            qDebug() << QString("扫描完成！获取点云数据: 扫描线数=%1, 总点数=%2")
                        .arg(pointCloud.size()).arg(totalPoints);
            
            emit pointCloudUpdated(pointCloud);
            
            // 单次扫描完成后，停止定时器并更新状态
            if (m_status == RadarStatus::Scanning && !m_autoScan) {
                m_scanTimer->stop();
                updateStatus(RadarStatus::Connected);
                qDebug() << "单次扫描结束，云台已停止";
            }
        } else if (result.pointCloudResult == 0) {
            // 无可用数据，扫描还在进行中
            qDebug() << "扫描进行中，等待数据...";
        } else if (result.pointCloudResult == -1) {
            QString errorMsg = "云台网络通讯异常";
            emit errorOccurred(errorMsg);
            qDebug() << errorMsg;
            m_scanTimer->stop();
            updateStatus(RadarStatus::Error);
        } else if (result.pointCloudResult == -2) {
            QString errorMsg = "雷达网络通讯异常";
            emit errorOccurred(errorMsg);
            qDebug() << errorMsg;
            m_scanTimer->stop();
            updateStatus(RadarStatus::Error);
        }
        
        // 处理轮廓数据结果
        if (result.profileResult == 1) {
            std::vector<Point3D> profile = result.profile;
            
            {
                QMutexLocker locker(&m_dataMutex);
                m_latestProfile = profile;
            }
            
            emit profileUpdated(profile);
        } else if (result.profileResult == -3) {
            // 云台还在运动过程中
            qDebug() << "云台正在旋转到目标位置...";
        }
    });
    
    watcher->setFuture(future);
}

void RadarController::updateStatus(RadarStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}


// 新增方法实现
Common3DPlatformStatus RadarController::getPlatformStatus() const
{
    return m_platformStatus;
}

DeviceInfo RadarController::getDeviceInfo() const
{
    return m_deviceInfo;
}

ScanStatistics RadarController::getScanStatistics() const
{
    return m_scanStats;
}

bool RadarController::initToZeroPoint()
{
    if (!isConnected()) {
        emit errorOccurred("雷达未连接，无法回零点");
        return false;
    }

    try {
        Common3DPlatformError error;
        bool success = m_platform->InitToZeroPoint(error);
        
        if (success) {
            qDebug() << "回零点成功";
            return true;
        } else {
            QString errorMsg = getErrorString(error);
            emit errorOccurred(QString("回零点失败: %1").arg(errorMsg));
            return false;
        }
    } catch (const std::exception &e) {
        QString errorMsg = QString("回零点异常: %1").arg(e.what());
        emit errorOccurred(errorMsg);
        return false;
    }
}

bool RadarController::rebootDevice()
{
    if (!isConnected()) {
        emit errorOccurred("雷达未连接，无法重启");
        return false;
    }

    try {
        int result = m_platform->rebootDevice();
        
        if (result == 1) {
            qDebug() << "设备重启成功";
            return true;
        } else {
            emit errorOccurred(QString("设备重启失败，返回值: %1").arg(result));
            return false;
        }
    } catch (const std::exception &e) {
        QString errorMsg = QString("设备重启异常: %1").arg(e.what());
        emit errorOccurred(errorMsg);
        return false;
    }
}

bool RadarController::stopMotorAndBackToInitialStage()
{
    if (!isConnected()) {
        emit errorOccurred("雷达未连接，无法停止电机");
        return false;
    }

    try {
        int result = m_platform->stopMotorAndBackToInitialStage();
        
        if (result == 1) {
            qDebug() << "电机停止成功";
            return true;
        } else {
            emit errorOccurred(QString("电机停止失败，返回值: %1").arg(result));
            return false;
        }
    } catch (const std::exception &e) {
        QString errorMsg = QString("电机停止异常: %1").arg(e.what());
        emit errorOccurred(errorMsg);
        return false;
    }
}

bool RadarController::modifyIPAddress(const QString &platformIP, const QString &lidarIP, 
                                     const QString &udpIP, unsigned short udpPort)
{
    if (!isConnected()) {
        emit errorOccurred("雷达未连接，无法修改IP");
        return false;
    }

    try {
        std::string platformAddr = platformIP.toStdString();
        std::string lidarAddr = lidarIP.toStdString();
        std::string udpAddr = udpIP.toStdString();
        
        int result = m_platform->ModifyIPAdress(platformAddr, lidarAddr, udpAddr, udpPort);
        
        if (result == 1) {
            qDebug() << "IP修改成功，请断电重启设备";
            return true;
        } else {
            emit errorOccurred(QString("IP修改失败，返回值: %1").arg(result));
            return false;
        }
    } catch (const std::exception &e) {
        QString errorMsg = QString("IP修改异常: %1").arg(e.what());
        emit errorOccurred(errorMsg);
        return false;
    }
}

bool RadarController::setEchoFilter(unsigned int echoFlag)
{
    if (!isConnected()) {
        emit errorOccurred("雷达未连接，无法设置回波模式");
        return false;
    }

    try {
        int result = m_platform->SetEchoFilter(echoFlag);
        
        if (result == 1) {
            qDebug() << QString("回波模式设置成功: %1").arg(echoFlag);
            return true;
        } else {
            emit errorOccurred(QString("回波模式设置失败，返回值: %1").arg(result));
            return false;
        }
    } catch (const std::exception &e) {
        QString errorMsg = QString("回波模式设置异常: %1").arg(e.what());
        emit errorOccurred(errorMsg);
        return false;
    }
}

bool RadarController::setAngleRange(float startAngle, float stopAngle)
{
    if (!isConnected()) {
        emit errorOccurred("雷达未连接，无法设置角度范围");
        return false;
    }

    try {
        int result = m_platform->setAngleRange(startAngle, stopAngle);
        
        if (result == 1) {
            qDebug() << QString("角度范围设置成功: %1 - %2").arg(startAngle).arg(stopAngle);
            return true;
        } else {
            emit errorOccurred(QString("角度范围设置失败，返回值: %1").arg(result));
            return false;
        }
    } catch (const std::exception &e) {
        QString errorMsg = QString("角度范围设置异常: %1").arg(e.what());
        emit errorOccurred(errorMsg);
        return false;
    }
}

bool RadarController::setFrequencyAndResolution(float frequency, float resolution)
{
    if (!isConnected()) {
        emit errorOccurred("雷达未连接，无法设置频率和分辨率");
        return false;
    }

    try {
        int result = m_platform->setFrequencyAndResolution(frequency, resolution);
        
        if (result == 1) {
            qDebug() << QString("频率和分辨率设置成功: 频率=%1, 分辨率=%2").arg(frequency).arg(resolution);
            return true;
        } else {
            emit errorOccurred(QString("频率和分辨率设置失败，返回值: %1").arg(result));
            return false;
        }
    } catch (const std::exception &e) {
        QString errorMsg = QString("频率和分辨率设置异常: %1").arg(e.what());
        emit errorOccurred(errorMsg);
        return false;
    }
}

bool RadarController::saveSettingsPermanently()
{
    if (!isConnected()) {
        emit errorOccurred("雷达未连接，无法保存设置");
        return false;
    }

    try {
        int result = m_platform->saveSettingsPermanently();
        
        if (result == 1) {
            qDebug() << "设置保存成功";
            return true;
        } else {
            emit errorOccurred(QString("设置保存失败，返回值: %1").arg(result));
            return false;
        }
    } catch (const std::exception &e) {
        QString errorMsg = QString("设置保存异常: %1").arg(e.what());
        emit errorOccurred(errorMsg);
        return false;
    }
}

void RadarController::onStatusCheckTimer()
{
    if (!isConnected()) {
        return;
    }
    
    // 如果上一次异步任务还在运行，跳过本次调用
    if (m_statusCheckTaskRunning) {
        return;
    }
    
    m_statusCheckTaskRunning = true;
    
    // 异步执行SDK调用
    QFuture<StatusCheckResult> future = QtConcurrent::run([this]() -> StatusCheckResult {
        StatusCheckResult result;
        
        try {
            // 使用互斥锁保护SDK调用
            QMutexLocker locker(&m_sdkMutex);
            
            // 检查平台状态
            result.statusResult = m_platform->GetStatus(result.platformStatus);
            
            // 获取设备信息
            unsigned int serialNo = m_platform->GetLmsSerialNo();
            result.deviceInfo.serialNumber = QString::number(serialNo);
            
            std::string firmwareVersion;
            if (m_platform->GetFirewareVersion(firmwareVersion)) {
                result.deviceInfo.firmwareVersion = QString::fromStdString(firmwareVersion);
            }
            
            int tempResult = m_platform->GetTemperature(result.deviceInfo.temperature, 
                                                         result.deviceInfo.temperatureWarningLevel);
            if (tempResult != 0) {
                result.deviceInfo.isValid = true;
            }
            
            int contaminationLevel;
            if (m_platform->getLMSContaminationStatus(contaminationLevel) == 1) {
                result.deviceInfo.contaminationLevel = contaminationLevel;
            }
            
        } catch (const std::exception &e) {
            result.hasError = true;
            result.errorMessage = QString("状态检查异常: %1").arg(e.what());
        }
        
        return result;
    });
    
    // 使用QFutureWatcher监控任务完成
    QFutureWatcher<StatusCheckResult> *watcher = new QFutureWatcher<StatusCheckResult>(this);
    connect(watcher, &QFutureWatcher<StatusCheckResult>::finished, this, [this, watcher]() {
        m_statusCheckTaskRunning = false;
        
        StatusCheckResult result = watcher->result();
        watcher->deleteLater();
        
        if (result.hasError) {
            qDebug() << result.errorMessage;
            return;
        }
        
        // 更新平台状态
        if (result.statusResult == 1) {
            m_platformStatus = result.platformStatus;
        }
        
        // 更新设备信息
        m_deviceInfo = result.deviceInfo;
        emit deviceInfoUpdated(m_deviceInfo);
    });
    
    watcher->setFuture(future);
}

void RadarController::updateDeviceInfo()
{
    if (!isConnected()) {
        return;
    }

    try {
        // 获取序列号
        unsigned int serialNo = m_platform->GetLmsSerialNo();
        m_deviceInfo.serialNumber = QString::number(serialNo);
        
        // 获取固件版本
        std::string firmwareVersion;
        if (m_platform->GetFirewareVersion(firmwareVersion)) {
            m_deviceInfo.firmwareVersion = QString::fromStdString(firmwareVersion);
        }
        
        // 获取温度
        int result = m_platform->GetTemperature(m_deviceInfo.temperature, m_deviceInfo.temperatureWarningLevel);
        if (result != 0) {
            m_deviceInfo.isValid = true;
        }
        
        // 获取污染状态
        int contaminationLevel;
        if (m_platform->getLMSContaminationStatus(contaminationLevel) == 1) {
            m_deviceInfo.contaminationLevel = contaminationLevel;
        }
        
        emit deviceInfoUpdated(m_deviceInfo);
        
    } catch (const std::exception &e) {
        qDebug() << "更新设备信息异常:" << e.what();
    }
}

void RadarController::updateScanStatistics(bool success, double scanTime, int pointCount)
{
    m_scanStats.totalScans++;
    
    if (success) {
        m_scanStats.successfulScans++;
        m_scanStats.totalPoints += pointCount;
        
        // 更新平均扫描时间
        double totalTime = m_scanStats.averageScanTime * (m_scanStats.successfulScans - 1);
        m_scanStats.averageScanTime = (totalTime + scanTime) / m_scanStats.successfulScans;
    } else {
        m_scanStats.failedScans++;
    }
    
    emit scanStatisticsUpdated(m_scanStats);
}

QString RadarController::getErrorString(const Common3DPlatformError &error) const
{
    QString errorMsg = QString("错误代码: %1").arg(static_cast<int>(error.code));
    
    if (!error.prompt.empty()) {
        errorMsg += QString(", 提示: %1").arg(QString::fromStdString(error.prompt));
    }
    
    // 添加常见错误的中文说明
    switch (error.code) {
    case Common3DPlatformError::LMS_CONNECT_FAIL:
        errorMsg += " - LMS雷达连接失败";
        break;
    case Common3DPlatformError::LMS_LOGIN_FAIL:
        errorMsg += " - LMS雷达登录失败";
        break;
    case Common3DPlatformError::CONTROL_SYS_SOCKET_CONNECT_FAIL:
        errorMsg += " - 云台控制系统连接失败";
        break;
    case Common3DPlatformError::PLATEFORM_NOT_INIT:
        errorMsg += " - 云台未初始化";
        break;
    case Common3DPlatformError::PLATFORM_TEMPERATURE_LOW_ERR:
        errorMsg += " - 温度过低，设备自动关闭";
        break;
    case Common3DPlatformError::PLATFORM_TEMPERATURE_HIGH_WARN:
        errorMsg += " - 温度过高警告";
        break;
    case Common3DPlatformError::LMS_CONTAMINATION_WARN:
        errorMsg += " - 雷达镜头污染警告";
        break;
    case Common3DPlatformError::LMS_CONTAMINATION_ERR:
        errorMsg += " - 雷达镜头污染严重";
        break;
    default:
        break;
    }
    
    return errorMsg;
}

QString RadarController::getStatusString(Common3DPlatformStatus status) const
{
    switch (status) {
    case eIdleStatus:
        return "空闲状态";
    case eInitialStage_ToZeroPoint_S6:
        return "初始化中-回零点";
    case eInitialStage_WaitToSetScanAngle_S7:
        return "初始化中-等待设置扫描角度";
    case eNOEnable_S5:
        return "未启用";
    case eInitialStage_ToStartAngle_S8:
        return "初始化中-移动到起始角度";
    case eStopAtStartPosition_S3:
        return "停在起始位置";
    case eStopAtStopPosition_S4:
        return "停在结束位置";
    case eStaticMode_Clock_SB:
        return "静态扫描模式-顺时针";
    case eStaticMode_ClockWise_SC:
        return "静态扫描模式-逆时针";
    case eSingleMoving_Clock_S1:
        return "单次移动-顺时针";
    case eSingleMoving_ClockWise_S2:
        return "单次移动-逆时针";
    case eMotorPosErr_S9:
        return "电机位置错误";
    case eNOHB_ii:
        return "心跳异常";
    case eStaticMode_AtStopPosition:
        return "静态模式-停在结束位置";
    case eZeroPointError:
        return "零点错误";
    case eMotorStop:
        return "电机停止";
    default:
        return "未知状态";
    }
}

QString RadarController::getPlatformStatusText() const
{
    return getStatusString(m_platformStatus);
}

bool RadarController::checkConnection()
{
    if (!m_platform) {
        return false;
    }
    
    try {
        return m_platform->IsThreadRunningOK();
    } catch (const std::exception &e) {
        qDebug() << "检查连接异常:" << e.what();
        return false;
    }
}

// 新增功能实现

bool RadarController::reconnectRadar()
{
    if (m_status == RadarStatus::Connected || m_status == RadarStatus::Scanning) {
        disconnectRadar();
        QThread::msleep(1000);  // 等待1秒
    }
    
    return connectRadar(m_platformIP, m_lidarIP, m_deviceType);
}

bool RadarController::isConnectionStable() const
{
    if (!isConnected()) {
        return false;
    }
    
    try {
        return m_platform->IsThreadRunningOK();
    } catch (const std::exception &e) {
        qDebug() << "连接稳定性检查异常:" << e.what();
        return false;
    }
}

void RadarController::startDataStream()
{
    if (!isConnected()) {
        emit errorOccurred("雷达未连接，无法开始数据流");
        return;
    }
    
    if (m_dataStreamActive) {
        return;
    }
    
    m_dataStreamActive = true;
    m_dataStreamTimer->start(m_dataStreamInterval);
    emit dataStreamStarted();
    qDebug() << "数据流已启动，间隔:" << m_dataStreamInterval << "ms";
}

void RadarController::stopDataStream()
{
    if (!m_dataStreamActive) {
        return;
    }
    
    m_dataStreamActive = false;
    m_dataStreamTimer->stop();
    emit dataStreamStopped();
    qDebug() << "数据流已停止";
}

void RadarController::setDataStreamInterval(int intervalMs)
{
    m_dataStreamInterval = qMax(50, intervalMs);  // 最小50ms间隔
    
    if (m_dataStreamActive) {
        m_dataStreamTimer->start(m_dataStreamInterval);
    }
    
    qDebug() << "数据流间隔已设置为:" << m_dataStreamInterval << "ms";
}

bool RadarController::isDataStreamActive() const
{
    return m_dataStreamActive;
}

void RadarController::clearDataCache()
{
    QMutexLocker locker(&m_dataMutex);
    
    for (auto &cache : m_pointCloudCache) {
        cache.clear();
    }
    for (auto &cache : m_profileCache) {
        cache.clear();
    }
    
    m_cacheIndex = 0;
    qDebug() << "数据缓存已清空";
}

int RadarController::getCacheSize() const
{
    QMutexLocker locker(&m_dataMutex);
    return m_cacheIndex;
}

void RadarController::setMaxCacheSize(int maxSize)
{
    QMutexLocker locker(&m_dataMutex);
    
    m_maxCacheSize = qMax(10, maxSize);  // 最小缓存10帧
    
    // 调整缓存大小
    m_pointCloudCache.resize(m_maxCacheSize);
    m_profileCache.resize(m_maxCacheSize);
    
    // 如果当前索引超出范围，重置索引
    if (m_cacheIndex >= m_maxCacheSize) {
        m_cacheIndex = 0;
    }
    
    qDebug() << "最大缓存大小已设置为:" << m_maxCacheSize;
}

void RadarController::onDataStreamTimer()
{
    if (!isConnected() || !m_dataStreamActive) {
        return;
    }
    
    // 如果上一次异步任务还在运行，跳过本次调用
    if (m_dataStreamTaskRunning) {
        return;  // 数据流频率高，静默跳过
    }
    
    m_dataStreamTaskRunning = true;
    
    // 异步执行SDK调用
    QFuture<ScanDataResult> future = QtConcurrent::run([this]() -> ScanDataResult {
        ScanDataResult result;
        
        try {
            // 使用互斥锁保护SDK调用
            QMutexLocker locker(&m_sdkMutex);
            
            // 获取最新点云数据
            result.pointCloudResult = m_platform->GetNewest3DPointCloud(result.pointCloud);
            
            // 获取单帧轮廓数据
            result.profileResult = m_platform->GetSingleProfile(result.profile, 1);
            
        } catch (const std::exception &e) {
            result.hasError = true;
            result.errorMessage = QString("数据流获取异常: %1").arg(e.what());
        }
        
        return result;
    });
    
    // 使用QFutureWatcher监控任务完成
    QFutureWatcher<ScanDataResult> *watcher = new QFutureWatcher<ScanDataResult>(this);
    connect(watcher, &QFutureWatcher<ScanDataResult>::finished, this, [this, watcher]() {
        m_dataStreamTaskRunning = false;
        
        ScanDataResult result = watcher->result();
        watcher->deleteLater();
        
        if (result.hasError) {
            emit errorOccurred(result.errorMessage);
            qDebug() << result.errorMessage;
            
            // 检查连接是否丢失
            if (!checkConnection()) {
                emit connectionLost();
                updateStatus(RadarStatus::Error);
            }
            return;
        }
        
        if (result.pointCloudResult == 1) {
            PointCloud pointCloud = result.pointCloud;
            
            // 更新缓存
            {
                QMutexLocker locker(&m_dataMutex);
                m_pointCloudCache[m_cacheIndex] = pointCloud;
                m_latestPointCloud = pointCloud;
            }
            
            // 更新缓存索引
            int currentIndex = m_cacheIndex;
            m_cacheIndex = (m_cacheIndex + 1) % m_maxCacheSize;
            
            // 发送数据更新信号
            emit pointCloudUpdated(pointCloud);
            
            // 处理轮廓数据
            if (result.profileResult == 1) {
                std::vector<Point3D> profile = result.profile;
                
                {
                    QMutexLocker locker(&m_dataMutex);
                    m_profileCache[currentIndex] = profile;
                    m_latestProfile = profile;
                }
                
                emit profileUpdated(profile);
            }
            
            // 更新扫描统计
            updateScanStatistics(true, 0.1, pointCloud.size());
            
        } else {
            // 数据获取失败
            updateScanStatistics(false, 0.1, 0);
        }
    });
    
    watcher->setFuture(future);
}
