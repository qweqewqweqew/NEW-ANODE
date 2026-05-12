#ifndef PHOXICONTROLLER_H
#define PHOXICONTROLLER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QImage>
#include <QTimer>
#include <QString>
#include <QVector>
#include <QVector3D>

// 前向声明
class CPhoxiCamera;
struct s_Image3dS;  // 前向声明，定义在 DataX.h 中

// PhoXi相机状态
enum class PhoxiStatus {
    Disconnected,   // 未连接
    Connecting,     // 连接中
    Connected,      // 已连接
    Capturing,      // 采集中
    Error           // 错误
};

// PhoXi连接模式
enum class PhoxiConnectionMode {
    HardwareOnly,   // 仅真实硬件设备
    FileOnly,       // 仅文件设备（测试用）
    AutoDetect      // 自动检测（优先硬件）
};

// PhoXi扫描预设（配方）
enum class ScanPreset {
    Normal = 0,              // 正常环境
    StrongAmbientLight = 1   // 强光环境
};

// 相机信息
struct PhoxiCameraInfo {
    QString id;              // 相机ID
    QString name;            // 相机名称
    QString type;            // 相机类型
    QString serialNumber;    // 序列号
    QString firmwareVersion; // 固件版本
    bool isConnected;        // 是否已连接
};

// PhoXi工作线程类
class PhoxiWorker : public QObject
{
    Q_OBJECT

public:
    explicit PhoxiWorker(const QString &cameraId, QObject *parent = nullptr);
    ~PhoxiWorker();
    
    // 设置连接模式
    void setConnectionMode(PhoxiConnectionMode mode) { m_connectionMode = mode; }

public slots:
    void connectCamera();
    void disconnectCamera();
    void startCapture();               // 启动连续采集模式（用于实时预览）
    void stopCapture();                // 停止连续采集
    void captureSingleFrame();         // 单次拍照：统一的拍照接口，自动管理采集状态
    bool saveLastOutput(const QString &filePath);
    void updateSettings(int timeout, int resolution);
    void applyPreset(ScanPreset preset);  // 应用扫描预设

signals:
    void statusChanged(PhoxiStatus status);
    void imageReady(const QImage &image);
    void image3dReady(const s_Image3dS &image3d);  // 3D图像数据（用于算法处理和保存）
    void pointCloudReady(const QVector<QVector3D> &pointCloud);
    void errorOccurred(const QString &error);
    void cameraInfoUpdated(const PhoxiCameraInfo &info);
    void connectionResult(bool success, const QString &message);
    void logInfo(const QString &message);  // 日志信息信号

private:
    void captureLoop();                 // 连续采集循环（暂不支持）
    QImage convertToQImage(void* data, int width, int height);

private:
    QString m_cameraId;
    CPhoxiCamera* m_camera;      // 使用示例代码的相机类作为底层实现
    bool m_isConnected;
    bool m_isCapturing;
    int m_timeout;
    int m_resolution;
    PhoxiConnectionMode m_connectionMode;  // 连接模式
    QMutex m_mutex;
};

// PhoXi相机控制器类
class PhoxiController : public QObject
{
    Q_OBJECT

public:
    explicit PhoxiController(const QString &cameraId, QObject *parent = nullptr);
    ~PhoxiController();

    // 设置连接模式
    void setConnectionMode(PhoxiConnectionMode mode);
    
    // 设置扫描预设,算法调用这个函数
    void setScanPreset(ScanPreset preset);
    
    // 获取当前使用的预设
    ScanPreset getCurrentPreset() const;
    
    // 获取预设名称
    static QString getPresetName(ScanPreset preset);

    // 基本控制
    void connectCamera();
    void disconnectCamera();
    void startCapture();               // 启动连续采集模式（用于实时预览）
    void stopCapture();                // 停止连续采集
    void captureSingleFrame();         // 单次拍照：推荐使用此接口，自动管理采集状态
    void captureSingleFrame3d();       // 单次拍照（获取3D数据）：captureSingleFrame的别名
    void triggerFrame();               // 触发拍照：captureSingleFrame的别名
    bool saveLastOutput(const QString &filePath);

    // 设置
    void setTimeout(int timeout);
    void setResolution(int resolution);

    // 状态查询
    PhoxiStatus getStatus() const { return m_status; }
    bool isConnected() const { return m_status == PhoxiStatus::Connected || m_status == PhoxiStatus::Capturing; }
    bool isCapturing() const { return m_status == PhoxiStatus::Capturing; }
    QString getCameraId() const { return m_cameraId; }
    PhoxiCameraInfo getCameraInfo() const { return m_cameraInfo; }

    // 静态方法 - 获取可用相机列表
    static QVector<PhoxiCameraInfo> getAvailableCameras();

signals:
    void statusChanged(PhoxiStatus status);
    void imageReady(const QImage &image);
    void image3dReady(const s_Image3dS &image3d);  // 3D图像数据（用于算法处理和保存）
    void pointCloudReady(const QVector<QVector3D> &pointCloud);
    void errorOccurred(const QString &error);
    void cameraInfoUpdated(const PhoxiCameraInfo &info);
    void connectionResult(bool success, const QString &message);
    void logInfo(const QString &message);  // 日志信息信号

    // 内部信号
    void requestConnect();
    void requestDisconnect();
    void requestStartCapture();
    void requestStopCapture();
    void requestCaptureSingleFrame();
    void requestUpdateSettings(int timeout, int resolution);
    void requestSetPreset(ScanPreset preset);

private slots:
    void onStatusChanged(PhoxiStatus status);
    void onImageReady(const QImage &image);
    void onImage3dReady(const s_Image3dS &image3d);
    void onPointCloudReady(const QVector<QVector3D> &pointCloud);
    void onErrorOccurred(const QString &error);
    void onCameraInfoUpdated(const PhoxiCameraInfo &info);
    void onConnectionResult(bool success, const QString &message);
    void onLogInfo(const QString &message);

private:
    QString m_cameraId;
    PhoxiStatus m_status;
    PhoxiCameraInfo m_cameraInfo;
    ScanPreset m_currentPreset;  // 当前使用的预设
    
    // 线程相关
    QThread *m_workerThread;
    PhoxiWorker *m_worker;
    
    // 设置
    int m_timeout;
    int m_resolution;
    
    mutable QMutex m_mutex;
};

#endif // PHOXICONTROLLER_H

