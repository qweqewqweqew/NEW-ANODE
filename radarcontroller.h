#ifndef RADARCONTROLLER_H
#define RADARCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <QString>
#include <QFuture>
#include <QFutureWatcher>
#include <vector>
#include <memory>
#include "ThreeDPlatformDll.h"

// 使用SICK SDK的数据结构
using Point3D = ::Point3D;  // 使用SDK中的Point3D结构
using PointCloud = Double3DPointVec;  // 使用SDK中的点云类型

// 设备信息结构
struct DeviceInfo {
    QString serialNumber;
    QString firmwareVersion;
    float temperature;
    int temperatureWarningLevel;
    int contaminationLevel;
    bool isValid;
    
    DeviceInfo() : temperature(0.0), temperatureWarningLevel(0), 
                   contaminationLevel(0), isValid(false) {}
};

// 扫描统计信息
struct ScanStatistics {
    int totalScans;
    int successfulScans;
    int failedScans;
    double averageScanTime;
    int totalPoints;
    
    ScanStatistics() : totalScans(0), successfulScans(0), 
                       failedScans(0), averageScanTime(0.0), totalPoints(0) {}
};

// 雷达状态枚举
enum class RadarStatus {
    Disconnected,
    Connecting,
    Connected,
    Scanning,
    Error
};

// 雷达控制类
class RadarController : public QObject
{
    Q_OBJECT

public:
    explicit RadarController(QObject *parent = nullptr);
    ~RadarController();

    // 连接和断开
    bool connectRadar(const QString &platformIP, const QString &lidarIP, 
                     EPlatFormType deviceType = EPlatFormType::PICOSCAN_MODE2);  // 默认使用Mini云台PicoScan摆动
    void disconnectRadar();
    bool reconnectRadar();  // 重新连接
    bool isConnectionStable() const;  // 检查连接稳定性

    // 扫描控制
    bool startScan();
    bool stopScan();
    bool setScanParameters(float startAngle, float stopAngle, float velocity);

    // 数据获取
    PointCloud getLatestPointCloud();
    std::vector<Point3D> getLatestProfile();
    
    // 数据流控制
    void startDataStream();  // 开始数据流
    void stopDataStream();   // 停止数据流
    void setDataStreamInterval(int intervalMs);  // 设置数据流间隔
    bool isDataStreamActive() const;  // 检查数据流状态
    
    // 数据缓存管理
    void clearDataCache();  // 清空数据缓存
    int getCacheSize() const;  // 获取缓存大小
    void setMaxCacheSize(int maxSize);  // 设置最大缓存大小

    // 状态查询
    RadarStatus getStatus() const { return m_status; }
    QString getStatusString() const;
    bool isConnected() const { return m_status == RadarStatus::Connected || m_status == RadarStatus::Scanning; }
    Common3DPlatformStatus getPlatformStatus() const;
    DeviceInfo getDeviceInfo() const;
    ScanStatistics getScanStatistics() const;
    QString getPlatformStatusText() const;  // 返回当前云台状态描述
    
    // 配置参数
    void setAutoScan(bool enabled) { m_autoScan = enabled; }
    void setScanInterval(int intervalMs) { m_scanInterval = intervalMs; }
    
    // 高级功能
    bool initToZeroPoint();
    bool rebootDevice();
    bool stopMotorAndBackToInitialStage();
    bool modifyIPAddress(const QString &platformIP, const QString &lidarIP, 
                        const QString &udpIP = "", unsigned short udpPort = 2115);
    
    // 雷达参数设置
    bool setEchoFilter(unsigned int echoFlag);
    bool setAngleRange(float startAngle, float stopAngle);
    bool setFrequencyAndResolution(float frequency, float resolution);
    bool saveSettingsPermanently();

signals:
    void statusChanged(RadarStatus status);
    void pointCloudUpdated(const PointCloud &pointCloud);
    void profileUpdated(const std::vector<Point3D> &profile);
    void errorOccurred(const QString &error);
    void deviceInfoUpdated(const DeviceInfo &info);
    void scanStatisticsUpdated(const ScanStatistics &stats);
    void dataStreamStarted();
    void dataStreamStopped();
    void connectionLost();
    void connectionRestored();

private slots:
    void onScanTimer();
    void onStatusCheckTimer();
    void onDataStreamTimer();

private:
    void updateStatus(RadarStatus status);
    void updateDeviceInfo();
    void updateScanStatistics(bool success, double scanTime, int pointCount);
    QString getErrorString(const Common3DPlatformError &error) const;
    QString getStatusString(Common3DPlatformStatus status) const;
    bool checkConnection();

private:
    // SICK SDK对象
    std::unique_ptr<ThreeDPlatform> m_platform;
    
    // 状态管理
    RadarStatus m_status;
    QString m_platformIP;
    QString m_lidarIP;
    EPlatFormType m_deviceType;
    Common3DPlatformStatus m_platformStatus;
    
    // 扫描控制
    QTimer *m_scanTimer;
    QTimer *m_statusCheckTimer;
    QTimer *m_dataStreamTimer;
    
    // 异步连接相关
    QFutureWatcher<bool> *m_connectWatcher;
    
    // 异步SDK调用相关（用于定时器回调）
    QMutex m_sdkMutex;  // 保护SDK调用（SDK可能不是线程安全的）
    bool m_scanTaskRunning;  // 扫描任务是否正在运行
    bool m_dataStreamTaskRunning;  // 数据流任务是否正在运行
    bool m_statusCheckTaskRunning;  // 状态检查任务是否正在运行
    
    bool m_autoScan;
    int m_scanInterval;
    int m_dataStreamInterval;
    bool m_dataStreamActive;
    
    // 数据缓存
    std::vector<PointCloud> m_pointCloudCache;
    std::vector<std::vector<Point3D>> m_profileCache;
    int m_maxCacheSize;
    int m_cacheIndex;
    
    // 数据缓存
    PointCloud m_latestPointCloud;
    std::vector<Point3D> m_latestProfile;
    mutable QMutex m_dataMutex;
    
    // 设备信息和统计
    DeviceInfo m_deviceInfo;
    ScanStatistics m_scanStats;
    
    // 扫描参数
    float m_startAngle;
    float m_stopAngle;
    float m_velocity;
};

#endif // RADARCONTROLLER_H
