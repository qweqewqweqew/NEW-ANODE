#ifndef BATCHFINEPOSITIONINGMANAGER_H
#define BATCHFINEPOSITIONINGMANAGER_H

#include <QObject>
#include <QTimer>
#include <QList>
#include <QDateTime>
#include <QtGlobal>
#include "apitypes.h"

// 单个铜垛的精定位结果
struct FinePositioningResult {
    int blockIndex;                    // 铜垛索引（对应拍照位）
    bool isCompleted;                   // 是否完成精定位
    bool meetsGrabCondition;            // 是否符合抓取条件
    double grabX, grabY, grabZ, grabDeg; // 最终抓取坐标
    int retryCount;                     // 重试次数
    int positioningRequestCount;        // 精定位请求次数（0=未开始，1=第一次请求，2=第二次请求完成）
    double firstRequestX, firstRequestY, firstRequestZ, firstRequestDeg; // 第一次请求的坐标（用于第二次请求时+2）
    QString errorMessage;               // 错误信息
    QDateTime completedTime;            // 完成时间
    
    // 当前状态
    enum Status {
        Pending,        // 待处理
        Positioning,    // 精定位中
        WaitingWms,     // 等待WMS移动完成
        Completed,      // 已完成
        Failed          // 失败
    } status;
    
    FinePositioningResult() 
        : blockIndex(-1)
        , isCompleted(false)
        , meetsGrabCondition(false)
        , grabX(0.0), grabY(0.0), grabZ(0.0), grabDeg(0.0)
        , retryCount(0)
        , positioningRequestCount(0)
        , firstRequestX(0.0), firstRequestY(0.0), firstRequestZ(0.0), firstRequestDeg(0.0)
        , status(Pending)
    {}
};

// 相机算法接口（预留，待封装）
class CameraAlgorithm : public QObject
{
    Q_OBJECT

public:
    // 精定位识别结果
    struct FinePositioningResult {
        bool canGrab;              // 是否可抓取
        double grabX, grabY, grabZ, grabDeg;      // 抓取坐标
        double adjustX, adjustY, adjustZ, adjustDeg; // 调整坐标（如果不可抓取）
        int blockNumber;           // 算法检测到的铜垛编号（从0或1开始，由算法决定）
        QString errorMessage;      // 错误信息
        
        FinePositioningResult() 
            : canGrab(false)
            , grabX(0.0), grabY(0.0), grabZ(0.0), grabDeg(0.0)
            , adjustX(0.0), adjustY(0.0), adjustZ(0.0), adjustDeg(0.0)
            , blockNumber(-1)  // -1表示未设置编号
        {}
    };
    
    explicit CameraAlgorithm(QObject *parent = nullptr) : QObject(parent) {}
    
    // 执行精定位识别（预留接口，待实现）
    // 输入：相机图像
    // 输出：精定位结果
    virtual FinePositioningResult performFinePositioning(const QImage &image) = 0;
};

// 批量精定位管理器
class BatchFinePositioningManager : public QObject
{
    Q_OBJECT

public:
    explicit BatchFinePositioningManager(QObject *parent = nullptr);
    ~BatchFinePositioningManager();

    /**
     * @brief 初始化批量精定位（从雷达扫描结果获取拍照位坐标）
     * @param photoPoses 拍照位坐标列表
     */
    void initialize(const QList<ApiTypes::Point3D> &photoPoses);

    /**
     * @brief 开始批量精定位
     */
    void startBatchPositioning();

    /**
     * @brief 停止批量精定位
     */
    void stopBatchPositioning();

    /**
     * @brief WMS回调：收到位置就绪通知
     * @param blockIndex 铜垛索引
     */
    void onPositionReady(int blockIndex);

    /**
     * @brief 相机算法回调：精定位结果
     * @param blockIndex 铜垛索引
     * @param canGrab 是否可抓取
     * @param grabX/Y/Z/Deg 抓取坐标
     * @param adjustX/Y/Z/Deg 调整坐标（如果不可抓取）
     * @param errorMessage 错误信息
     */
    void onFinePositioningResult(int blockIndex,
                                 bool canGrab,
                                 double grabX, double grabY, double grabZ, double grabDeg,
                                 double adjustX, double adjustY, double adjustZ, double adjustDeg,
                                 const QString &errorMessage = QString());

    /**
     * @brief 相机拍照完成回调
     * @param blockIndex 铜垛索引
     * @param image 相机图像
     */
    void onCameraImageReady(int blockIndex, const QImage &image);

    /**
     * @brief 检查是否所有精定位都完成
     */
    bool areAllCompleted() const;

    /**
     * @brief 批量上报到WMS
     */
    void reportBatchToWms();

    /**
     * @brief 获取当前状态
     */
    int getCompletedCount() const;
    int getTotalCount() const;
    int getCurrentIndex() const { return m_currentIndex; }
    bool isRunning() const { return m_isRunning; }

    /**
     * @brief 获取指定铜垛的拍照位坐标
     * @param blockIndex 铜垛索引
     * @return 拍照位坐标，索引无效时返回空坐标
     */
    ApiTypes::Point3D getPhotoPose(int blockIndex) const;

    /**
     * @brief 设置精定位最大重试次数（AlignMax）
     */
    void setAlignMax(int maxRetry) { m_alignMax = qMax(1, maxRetry); }

    /**
     * @brief 设置拍照延时（毫秒）
     */
    void setCaptureDelayMs(int delayMs) { m_captureDelayMs = qMax(0, delayMs); }

    /**
     * @brief 设置相机算法实例（预留）
     */
    void setCameraAlgorithm(CameraAlgorithm *algorithm) { m_cameraAlgorithm = algorithm; }

    /**
     * @brief 设置测试模式（用于测试阶段，传入假的相机坐标）
     * @param enabled 是否启用测试模式
     * @param testCoordinates 测试坐标列表（每个拍照位对应一个坐标）
     *                         如果为空，则使用拍照位坐标作为抓取坐标
     */
    void setTestMode(bool enabled, const QList<ApiTypes::Point3D> &testCoordinates = QList<ApiTypes::Point3D>());
    
    /**
     * @brief 检查是否处于测试模式
     */
    bool isTestMode() const { return m_testMode; }

signals:
    /**
     * @brief 需要调用WMS写入坐标
     * @param blockIndex 铜垛索引
     * @param x/y/z/deg 坐标
     */
    void requestAdjustPosition(int blockIndex, double x, double y, double z, double deg);

    /**
     * @brief 需要触发相机拍照
     * @param blockIndex 铜垛索引
     */
    void requestCameraCapture(int blockIndex);

    /**
     * @brief 批量精定位完成（需要上报）
     * @param scanResult 批量上报的扫描结果
     */
    void batchPositioningCompleted(const ApiTypes::ScanResult &scanResult);

    /**
     * @brief 批量精定位失败
     * @param error 错误信息
     */
    void batchPositioningFailed(const QString &error);

    /**
     * @brief 进度更新
     * @param completed 已完成数量
     * @param total 总数量
     * @param currentBlockIndex 当前处理的铜垛索引
     */
    void progressUpdated(int completed, int total, int currentBlockIndex);

private slots:
    /**
     * @brief 处理下一个拍照位
     */
    void processNextPhotoPose();

    /**
     * @brief WMS移动超时处理（120秒）
     */
    void onWmsMoveTimeout();

private:
    QList<FinePositioningResult> m_results;           // 所有精定位结果
    QList<ApiTypes::Point3D> m_photoPoses;            // 拍照位坐标列表
    int m_currentIndex;                                // 当前处理的拍照位索引
    bool m_isRunning;                                  // 是否正在批量精定位
    QTimer *m_wmsMoveTimeoutTimer;                    // WMS移动超时定时器（120秒）
    CameraAlgorithm *m_cameraAlgorithm;                // 相机算法实例（预留）
    bool m_testMode;                                   // 测试模式标志
    QList<ApiTypes::Point3D> m_testCoordinates;        // 测试坐标列表（测试模式使用）
    int m_alignMax;                                    // 精定位最大重试次数
    int m_captureDelayMs;                              // 拍照延时（毫秒）
    
    /**
     * @brief 更新精定位结果
     */
    void updatePositioningResult(int blockIndex, bool canGrab,
                                 double grabX, double grabY, double grabZ, double grabDeg,
                                 double adjustX, double adjustY, double adjustZ, double adjustDeg,
                                 const QString &errorMessage = QString());

    /**
     * @brief 检查并处理完成状态
     */
    void checkAndHandleCompletion();
};

#endif // BATCHFINEPOSITIONINGMANAGER_H

