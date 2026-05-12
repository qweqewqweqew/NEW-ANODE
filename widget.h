#include <QDoubleSpinBox>

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QSettings>
#include <QLabel>
#include <QQueue>
#include <QHash>
#include <QSet>
#include <QMenu>
#include <QAction>
#include "radarcontroller.h"
#include "camerawidget.h"
#include "phoxicontroller.h"
#include "anodealgorithmworker.h"
#include "DataX.h"
#include "heatmapwidget.h"
#include "parametersettingsdialog.h"
#include "api/apitypes.h"

// API模块
class ApiServer;  // 前向声明
class WmsClient;  // 前向声明
class BatchFinePositioningManager;  // 前向声明
class CameraTestDialog;  // 前向声明

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    
    // ========================================
    // 供API服务器查询的公共接口
    // ========================================
    QString getCamera1Status() const;
    QString getCamera2Status() const;
    QString getRadarStatus() const;
    QString getAlgorithmStatus() const;  // ⚠️ Phase 1 返回固定值
    QString getSystemStatus() const;
    QString getLastTaskId() const { return m_lastTaskId; }
    QString getLastError() const { return m_lastError; }
    
    // 设置任务信息
    void setLastTaskId(const QString &taskId) { m_lastTaskId = taskId; }
    void setLastError(const QString &error) { m_lastError = error; }

    // 统一的雷达扫描触发接口（供界面按钮和 Web API 共用）
    bool startRadarScanAndDetect(const QString &triggerSource);
    
    // 处理雷达文件上传（供API调用）
    bool processRadarFileUpload(const QString &filePath);
    
    // 批量精定位测试模式设置（用于测试阶段）
    void setFinePositioningTestMode(bool enabled, const QList<ApiTypes::Point3D> &testCoordinates = QList<ApiTypes::Point3D>());
    
    // 打开相机测试对话框（公共方法，可通过菜单/快捷键调用）
    void openCameraTestDialog();
    // 打开接口连通性测试对话框
    void openInterfaceTestDialog();

private slots:
    // 相机状态槽函数
    void onCamera1StatusChanged(PhoxiStatus status);
    void onCamera1ImageReady(const QImage &image);
    void onCamera1ErrorOccurred(const QString &error);
    
    void onCamera2StatusChanged(PhoxiStatus status);
    void onCamera2ImageReady(const QImage &image);
    void onCamera2ErrorOccurred(const QString &error);
    
    // 统一相机连接按钮槽函数
    void onCameraConnectBtnClicked();
    // 连接超时槽函数
    void onConnectionTimeout();
    
    // 日志控制槽函数
    void onClearLogClicked();
    void onSaveLogClicked();
    
    // 雷达控制槽函数
    void onRadarConnectBtnClicked();
    void onRadarInitZeroBtnClicked();
    void onRadarSetParamsBtnClicked();
    void onRadarScanDetectBtnClicked();  // 新增：雷达扫描并检测
    void onCameraTriggerBtnClicked();    // 弹出相机拍照菜单
    void onCamera1ShotRequested();       // 相机1单次拍照
    void onCamera2ShotRequested();       // 相机2单次拍照
    void onRadarScanSaveBtnClicked();
    void onRadarDetailInfoBtnClicked();
    
    // 雷达状态槽函数
    void onRadarStatusChanged(RadarStatus status);
    void onRadarPointCloudUpdated(const PointCloud &pointCloud);
    void onRadarErrorOccurred(const QString &error);
    
    // 算法相关槽函数
    void onUploadRadarFileBtnClicked();  // 上传雷达文件按钮
    void onParameterSettingsBtnClicked();  // 参数设置弹窗
    void onTestCameraAlgorithmBtnClicked();  // 相机算法测试按钮（打开对话框）
    void onAlgorithmProcessingFinished(const s_PreADPlateARtsPara &result,
                                       const s_Lidar3d &processedImage,
                                       bool preAlignValid,
                                       const s_CalcPreAlignRtsPara &preAlignResult,
                                       const QString &preAlignError);
    void onAlgorithmProcessingFailed(const QString &error);
    void onAlgorithmProcessingProgress(int progress, const QString &message);
    
    // 批量精定位相关槽函数
    void onBatchPositioningRequestAdjustPosition(int blockIndex, double x, double y, double z, double deg);
    void onBatchPositioningRequestCameraCapture(int blockIndex);
    void onBatchPositioningCompleted(const ApiTypes::ScanResult &scanResult);
    void onBatchPositioningFailed(const QString &error);
    void onBatchPositioningProgressUpdated(int completed, int total, int currentBlockIndex);
    void onWmsPositionReady(int blockIndex);  // WMS回调：位置就绪
    void onCameraImageForFinePositioning(int blockIndex, const QImage &image);  // 相机图像用于精定位
    void onFinePositioningFinished(int blockIndex,
                                   const s_AccurateADPlateARtsPara &result,
                                   const s_CalcAccurateAlignRtsPara &alignResult);
    void onFinePositioningFailed(int blockIndex, const QString &error);
    // 相机测试模式（本地/内存）回调
    void onTestDualCameraFinished(const s_AccurateADPlateARtsPara &detectResult,
                                  const s_CalcAccurateAlignRtsPara &calcResult,
                                  const s_CalcAccurateAlignPara &calcPara);
    void onTestDualCameraFailed(const QString &error);
    void onTestDualCameraProgress(const QString &message);
    void onCameraLogInfo(const QString &message);
    
    // WMS测试功能（槽函数，用于UI按钮连接）
    void onTestWmsReportClicked();
    // 相机连接菜单动作
    void onConnectCamera1();
    void onConnectCamera2();
    void onDisconnectAllCameras();

private:
    // 操作模式枚举（测试模式 vs 正式模式）
    enum class OperationMode {
        TestMode,        // 测试模式：手动操作，不发送WMS，不触发批量精定位
        ProductionMode   // 正式模式：API触发，发送WMS，完整流程
    };
    
    // 算法状态枚举（当前仅包含雷达算法，后续可扩展为“雷达+相机”综合状态）
    enum class AlgorithmStatus {
        Ready,
        Processing,
        Error
    };

    struct CameraCaptureArtifacts {
        QString prawPath;
        QString datasetFolder;
        QStringList datasetBasePaths;
        QList<s_Image3dS> images; // 直接内存数据

        bool isValid() const { return !prawPath.isEmpty() || !datasetBasePaths.isEmpty() || !images.isEmpty(); }
    };

    void setupConnections();
    void updateUI();
    void updateStatusLabels();
    void logMessage(const QString &message, const QString &level = "INFO");
    void saveSettings();
    void loadSettings();
    
    // 自动连接设备
    void autoConnectDevices();
    
    // 操作模式相关
    void setOperationMode(OperationMode mode);  // 设置操作模式
    OperationMode getOperationMode() const { return m_operationMode; }  // 获取操作模式
    bool isTestMode() const { return m_operationMode == OperationMode::TestMode; }  // 是否为测试模式
    bool isProductionMode() const { return m_operationMode == OperationMode::ProductionMode; }  // 是否为正式模式
    void onOperationModeChanged();  // 模式切换槽函数
    
    // 本地数据上传辅助函数（新增）
    bool savePointCloudAsPly(const PointCloud &pointCloud, const QString &filePath);
    
    // 算法相关辅助函数
    void startAlgorithmProcessing(const PointCloud &pointCloud);  // 启动算法处理
    void displayAlgorithmResult(const s_PreADPlateARtsPara &result, const s_Lidar3d &processedImage);  // 显示算法处理后的图像（包含缺陷区域）
    void saveAlgorithmResultToMarkdown(const s_PreADPlateARtsPara &result,
                                       bool preAlignValid,
                                       const s_CalcPreAlignRtsPara &preAlignResult,
                                       const QString &preAlignError);  // 保存结果到MD文档
    QImage convertHalconRegionToQImage(const HalconCpp::HObject &region, int width, int height, const QColor &color);  // 将Halcon区域转换为QImage
    QImage convertLidar3dToQImage(const s_Lidar3d &lidar3d);  // 将s_Lidar3d转换为QImage用于显示
    void applyAlgorithmParameterSettings();  // 将界面参数同步到算法线程
    void onCameraPositionParameterChanged();  // 相机拍照位参数改变时调用
    void ensureCameraPositionControls();     // 确保相机拍照位控件可用（找不到则运行时创建）
    ApiTypes::ScanResult buildRadarScanResult(bool preAlignValid,
                                              const s_CalcPreAlignRtsPara &preAlignResult,
                                              const QString &preAlignError) const; // 构建上报给WMS的结果
    void reportScanResultToWms(const ApiTypes::ScanResult &scanResult); // 调用WMS接口上报结果
    QString resolveProjectRootPath() const;
    QString ensureDailyDataDir(const QString &category) const;
    QString sanitizeForFilename(const QString &value) const;
    QString currentTaskIdForStorage() const;
    QString saveRadarPointCloudSnapshot(const PointCloud &pointCloud, const QString &stage);
    CameraCaptureArtifacts saveImage3dViaWriteImage3DX(s_Image3dS img,
                                                       const QString &cameraId,
                                                 int blockIndex,
                                                 const QString &stage);
    QStringList collectImage3dBasePaths(const QString &datasetPath) const;
    void enqueueFinePositioningTask(int blockIndex);
    void processNextFinePositioningTask();
    void logRadarReadyStatus();  // 定期打印雷达就绪状态
    void tryStartManualDualCameraAlgorithm(); // 手动双相机拍照完成后触发算法
    void tryStartManualDualCameraAlgorithmMemory(); // 使用内存s_Image3dS触发算法（拍照后自动触发）
    void startNewManualDualShotCycle(); // 重置手动双机采集缓存
    QImage convertImage3dSToQImage(const s_Image3dS &img3d); // 将s_Image3dS的Gray图像转换为QImage用于显示
    void connectSingleCamera(PhoxiController *controller, const QString &name);
    
    // WMS测试功能
    void testWmsReport();  // 测试WMS上报功能

private:
    Ui::Widget *ui;
    
    // 控制器
    RadarController *m_radarController;
    
    // 算法工作线程
    QThread *m_algorithmThread;
    AnodeAlgorithmWorker *m_algorithmWorker;
    
    // 相机控件
    CameraWidget *m_camera1Widget;
    CameraWidget *m_camera2Widget;
    
    // 相机控制器
    PhoxiController *m_camera1Controller;
    PhoxiController *m_camera2Controller;
    
    // API服务器
    ApiServer *m_apiServer;
    
    // WMS客户端（用于上报结果）
    WmsClient *m_wmsClient;
    
    // 批量精定位管理器
    BatchFinePositioningManager *m_batchFinePositioningManager;
    
    // 定时器
    QTimer *m_timeTimer;
    QTimer *m_connectionTimer;  // 连接超时定时器
    QTimer *m_radarReadyTimer;  // 雷达就绪状态轮询
    
    // 状态栏标签
    QLabel *m_statusLabel;
    
    // 状态变量
    bool m_isConnecting;  // 是否正在连接相机
    bool m_autoConnectingCamera1;  // 自动连接时是否已请求连接相机1
    bool m_autoConnectingCamera2;  // 自动连接时是否已请求连接相机2
    int m_connectedCameraCount;  // 已连接的相机数量
    bool m_isFileMode;  // 是否为文件测试模式
    OperationMode m_operationMode;  // 操作模式（测试模式/正式模式）
    
    // 任务信息（供API使用）
    QString m_lastTaskId;   // 最后一次任务ID
    QString m_lastError;    // 最后一次错误信息
    
    // 算法结果显示控制
    bool m_showDefects;     // 是否显示缺陷区域（图层控制）
    HeatmapWidget *m_algorithmResultWidget;  // 算法结果显示控件

    // 算法当前状态（对外通过 getAlgorithmStatus 暴露为字符串）
    AlgorithmStatus m_algorithmStatus;

    // 相机拍照位参数输入控件
    QDoubleSpinBox *m_cameraXSpin;
    QDoubleSpinBox *m_cameraYSpin;
    QDoubleSpinBox *m_cameraZSpin;
    QDoubleSpinBox *m_cameraDegSpin;
    
    // 参数缓存
    int m_radarImageWidth;
    double m_safeDistanceTbNear;
    double m_safeDistanceToCbOuter;
    
    // 批量精定位临时变量
    int m_currentFinePositioningBlockIndex;  // 当前精定位的铜垛索引（用于相机回调识别）
    mutable QString m_cachedProjectRootPath;
    bool m_captureNextRadarFrameForStorage;
    QString m_lastScanTriggerSource;
    QHash<int, CameraCaptureArtifacts> m_cameraCaptureArtifacts;
    QQueue<int> m_pendingFineBlocks;
    QSet<int> m_pendingFineBlockSet;
    bool m_isFinePositioningBusy = false;
    int m_activeFinePositioningBlock = -1;
    QHash<int, ApiTypes::Point3D> m_lastAdjustPayload; // 记录最近发送给WMS的调整/拍照位坐标

    // 手动拍照用于相机算法的缓存（拍照是前提，落盘和算法检测都基于拍照结果）
    bool m_waitingManualDualShot = false; // 双相机等待标志
    QStringList m_manualCam1Paths;
    QStringList m_manualCam2Paths;
    s_Image3dS m_manualImg1;   // 相机1数据缓存（拍照后保存到内存）
    s_Image3dS m_manualImg2;   // 相机2数据缓存（拍照后保存到内存）

    // 相机拍照弹出菜单
    QMenu *m_cameraTriggerMenu = nullptr;
    QAction *m_cam1TriggerAction = nullptr;
    QAction *m_cam2TriggerAction = nullptr;
    QMenu *m_cameraConnectMenu = nullptr;
    QAction *m_connectCam1Action = nullptr;
    QAction *m_connectCam2Action = nullptr;
    QAction *m_disconnectAllCamerasAction = nullptr;
};

#endif // WIDGET_H
