 #include "widget.h"
 #include "ui_widget.h"
#include <QApplication>
#include <QDateTime>
#include <QDate>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>
#include <QTextStream>
#include <QDebug>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QCursor>
#include <QFileInfo>
#include <QFile>
#include <QThread>
#include <QMetaObject>
#include <QImage>
#include <QColor>
#include <QPainter>
#include <QTransform>
#include <QPolygon>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <cmath>
#include <cstring>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif
#include <QTabWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QGroupBox>
#include <QNetworkReply>
#include <QtConcurrent>
#include <QUrl>
#include <QTextEdit>
#include "halconcpp.h"
#include "parametersettingsdialog.h"
#include "cameratestdialog.h"
#include <QRegularExpression>

// API模块
#include "apiserver.h"
#include "apiconfig.h"
#include "wmsclient.h"
#include "batchfinepositioningmanager.h"
#include <QNetworkInterface>
#include <QHostAddress>
#include <QAbstractSocket>
#include "CHVisionAdvX.h"

Q_DECLARE_METATYPE(s_PreADPlateARtsPara)
Q_DECLARE_METATYPE(s_Lidar3d)
Q_DECLARE_METATYPE(s_CalcPreAlignRtsPara)
Q_DECLARE_METATYPE(s_AccurateADPlateARtsPara)
Q_DECLARE_METATYPE(s_CalcAccurateAlignRtsPara)
Q_DECLARE_METATYPE(s_CalcAccurateAlignPara)

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_radarController(new RadarController(this))
    , m_algorithmThread(nullptr)
    , m_algorithmWorker(nullptr)
    , m_camera1Widget(new CameraWidget("相机1", this))
    , m_camera2Widget(new CameraWidget("相机2", this))
    , m_camera1Controller(new PhoxiController("IPA-107", this))  // 相机1 设备ID
    , m_camera2Controller(new PhoxiController("PAJ-076", this))  // 相机2 设备ID
    , m_apiServer(nullptr)
    , m_timeTimer(new QTimer(this))
    , m_connectionTimer(new QTimer(this))
    , m_radarReadyTimer(new QTimer(this))
    , m_cameraTriggerMenu(new QMenu(this))
    , m_statusLabel(new QLabel(this))
    , m_isConnecting(false)
    , m_autoConnectingCamera1(false)
    , m_autoConnectingCamera2(false)
    , m_connectedCameraCount(0)
    , m_isFileMode(false)
    , m_operationMode(OperationMode::TestMode)  // 默认测试模式
    , m_lastTaskId("")
    , m_lastError("")
    , m_showDefects(true)  // 默认显示缺陷
    , m_algorithmResultWidget(nullptr)
    , m_algorithmStatus(AlgorithmStatus::Ready)
    , m_radarImageWidth(1201)
    , m_safeDistanceTbNear(350.0)
    , m_safeDistanceToCbOuter(400.0)
, m_currentFinePositioningBlockIndex(-1)
, m_captureNextRadarFrameForStorage(false)
{
    ui->setupUi(this);

    // 相机拍照弹出菜单
    m_cam1TriggerAction = m_cameraTriggerMenu->addAction("相机1拍照");
    m_cam2TriggerAction = m_cameraTriggerMenu->addAction("相机2拍照");
    connect(m_cam1TriggerAction, &QAction::triggered, this, &Widget::onCamera1ShotRequested);
    connect(m_cam2TriggerAction, &QAction::triggered, this, &Widget::onCamera2ShotRequested);

    // 相机连接下拉菜单
    m_cameraConnectMenu = new QMenu(this);
    m_connectCam1Action = m_cameraConnectMenu->addAction("连接相机1");
    m_connectCam2Action = m_cameraConnectMenu->addAction("连接相机2");
    m_disconnectAllCamerasAction = m_cameraConnectMenu->addAction("断开所有相机");
    connect(m_connectCam1Action, &QAction::triggered, this, &Widget::onConnectCamera1);
    connect(m_connectCam2Action, &QAction::triggered, this, &Widget::onConnectCamera2);
    connect(m_disconnectAllCamerasAction, &QAction::triggered, this, &Widget::onDisconnectAllCameras);

    qRegisterMetaType<s_AccurateADPlateARtsPara>("s_AccurateADPlateARtsPara");
    qRegisterMetaType<s_CalcAccurateAlignRtsPara>("s_CalcAccurateAlignRtsPara");
    qRegisterMetaType<s_CalcAccurateAlignPara>("s_CalcAccurateAlignPara");
    qRegisterMetaType<s_Image3dS>("s_Image3dS");
    // 注册QImage元类型，确保跨线程信号槽传递安全
    qRegisterMetaType<QImage>("QImage");
    m_lastScanTriggerSource = "UI";

    qRegisterMetaType<s_PreADPlateARtsPara>("s_PreADPlateARtsPara");
    qRegisterMetaType<s_Lidar3d>("s_Lidar3d");
    qRegisterMetaType<s_CalcPreAlignRtsPara>("s_CalcPreAlignRtsPara");
    
    // 设置状态栏标签
    m_statusLabel->setTextFormat(Qt::RichText);
    m_statusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_statusLabel->setStyleSheet("color: white; padding: 5px 10px; font-size: 12px;");
    ui->statusBarLayout->addWidget(m_statusLabel, 1);  // 占据整个状态栏
    
    // 设置相机容器
    QVBoxLayout *camera1Layout = new QVBoxLayout(ui->camera1Container);
    camera1Layout->setContentsMargins(0, 0, 0, 0);
    camera1Layout->addWidget(m_camera1Widget);
    
    QVBoxLayout *camera2Layout = new QVBoxLayout(ui->camera2Container);
    camera2Layout->setContentsMargins(0, 0, 0, 0);
    camera2Layout->addWidget(m_camera2Widget);
    
    // 设置热力图容器（用于显示算法结果）
    // 检查 heatmapContainer 是否存在
    QWidget *heatmapContainer = ui->heatmapContainer;
    if (heatmapContainer) {
        QVBoxLayout *heatmapLayout = new QVBoxLayout(heatmapContainer);
        heatmapLayout->setContentsMargins(0, 0, 0, 0);
        m_algorithmResultWidget = new HeatmapWidget(heatmapContainer);
        heatmapLayout->addWidget(m_algorithmResultWidget);
    } else {
        qWarning() << "heatmapContainer 不存在，算法结果显示功能将不可用";
        m_algorithmResultWidget = nullptr;
    }
    
    // 配置连接超时定时器（10秒）
    m_connectionTimer->setSingleShot(true);
    m_connectionTimer->setInterval(10000);  // 10秒超时
    
    // 初始化相机拍照位参数控件（优先使用 UI，找不到则运行时创建）
    ensureCameraPositionControls();
    
    setupConnections();
    loadSettings();
    
    // 启动定时器
    m_timeTimer->start(1000);  // 每秒更新时间

    // 初始化状态标签
    updateStatusLabels();
    
    // 自动连接设备（延迟1秒，确保UI完全初始化）
    QTimer::singleShot(1000, this, [this]() {
        logMessage("程序启动：开始自动连接设备...", "INFO");
        autoConnectDevices();
    });

    logMessage("青海阳极卸车识别系统启动完成");
    logMessage("系统将在1秒后自动尝试连接相机和雷达...");
    
    // ========================================
    // 启动API服务器
    // ========================================
    // 加载配置文件
    ApiConfig::instance().load();
    
    // 创建API服务器
    m_apiServer = new ApiServer(this, this);
    
    // 连接API服务器信号
    connect(m_apiServer, &ApiServer::requestReceived, this, 
        [this](const QString &endpoint, const QString &method) {
            logMessage(QString(" API请求: %1 %2").arg(method, endpoint), "INFO");
        });
    
    connect(m_apiServer, &ApiServer::serverStarted, this,
        [this](quint16 port) {
            logMessage(QString(" HTTP API服务器启动成功，端口: %1").arg(port), "SUCCESS");
            
            // 获取本机IP地址（用于显示实际访问地址）
            QStringList ipAddresses;
            QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
            for (int i = 0; i < allInterfaces.size(); ++i) {
                QNetworkInterface netInterface = allInterfaces.at(i);
                // 跳过回环接口和未启用的接口
                QNetworkInterface::InterfaceFlags flags = netInterface.flags();
                if ((flags & QNetworkInterface::IsLoopBack) || 
                    !(flags & QNetworkInterface::IsUp)) {
                    continue;
                }
                
                QList<QNetworkAddressEntry> entries = netInterface.addressEntries();
                for (int j = 0; j < entries.size(); ++j) {
                    QNetworkAddressEntry entry = entries.at(j);
                    QHostAddress addr = entry.ip();
                    // 只显示IPv4地址
                    if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
                        QString ipStr = addr.toString();
                        if (!ipAddresses.contains(ipStr)) {
                            ipAddresses.append(ipStr);
                        }
                    }
                }
            }
            
            // 显示访问地址
            if (!ipAddresses.isEmpty()) {
                logMessage(QString(" 接口地址（可通过以下IP访问）:"), "INFO");
                for (const QString &ip : ipAddresses) {
                    logMessage(QString("   http://%1:%2/api/...").arg(ip).arg(port), "INFO");
                }
            } else {
                logMessage(QString(" 接口地址: http://localhost:%1/api/... (未检测到网络IP)").arg(port), "INFO");
            }
        });
    
    connect(m_apiServer, &ApiServer::serverStopped, this,
        [this]() {
            logMessage(" HTTP API服务器已停止", "INFO");
        });
    
    // 启动服务器
    if (!m_apiServer->start()) {
        logMessage(" HTTP API服务器启动失败", "ERROR");
        m_lastError = "API服务器启动失败";
    }
    
    // ========================================
    // 初始化WMS客户端
    // ========================================
    m_wmsClient = new WmsClient(this);
    
    // 从配置文件读取WMS地址
    ApiConfig *config = &ApiConfig::instance();
    QString wmsUrl = config->wmsUrl();
    
    //   占位值处理：如果URL不包含路径，自动添加 /api/result
    // 后续WMS提供真实地址时，应该包含完整路径，例如：http://192.168.1.100:9090/api/result
    if (wmsUrl.isEmpty() || wmsUrl == "http://localhost:9090") {
        wmsUrl = "http://localhost:9090/api/result";  //   占位值：后续需要配置真实WMS地址
        logMessage(QString(" WMS地址使用占位值: %1 (后续需要配置真实地址)").arg(wmsUrl), "WARNING");
    } else if (!wmsUrl.contains("/api/result")) {
        // 如果配置的URL没有包含接口路径，自动添加
        if (!wmsUrl.endsWith("/")) {
            wmsUrl += "/";
        }
        wmsUrl += "api/result";
        logMessage(QString(" WMS客户端已配置（自动补全路径）: %1").arg(wmsUrl), "INFO");
    } else {
        logMessage(QString(" WMS客户端已配置: %1").arg(wmsUrl), "INFO");
    }
    
    m_wmsClient->setWmsUrl(wmsUrl);
    m_wmsClient->setTimeout(config->wmsTimeout());
    
    // 连接WMS客户端信号（用于日志输出）
    connect(m_wmsClient, &WmsClient::reportSucceeded, this,
        [this](const QString &uniqueCode) {
            logMessage(QString("  WMS上报成功: 任务编号 %1").arg(uniqueCode), "SUCCESS");
        });
    
    connect(m_wmsClient, &WmsClient::reportFailed, this,
        [this](const QString &uniqueCode, const QString &error) {
            logMessage(QString("  WMS上报失败: 任务编号 %1, 错误: %2").arg(uniqueCode, error), "ERROR");
            qDebug() << "[WMS] reportResult failed" << uniqueCode << error;
        });
    connect(m_wmsClient, &WmsClient::adjustPositionSucceeded, this,
        [this](int blockIndex) {
            ApiTypes::Point3D point = m_lastAdjustPayload.value(blockIndex);
            logMessage(QString("  WMS写入坐标成功: block=%1, (x=%2, y=%3, z=%4, deg=%5)")
                           .arg(blockIndex)
                           .arg(point.x, 0, 'f', 1).arg(point.y, 0, 'f', 1)
                           .arg(point.z, 0, 'f', 1).arg(point.deg, 0, 'f', 2),
                       "SUCCESS");
            qDebug() << "[WMS] adjustPosition ok"
                     << "block" << blockIndex
                     << "x" << point.x << "y" << point.y << "z" << point.z << "deg" << point.deg;
        });
    connect(m_wmsClient, &WmsClient::adjustPositionFailed, this,
        [this](int blockIndex, const QString &error) {
            ApiTypes::Point3D point = m_lastAdjustPayload.value(blockIndex);
            logMessage(QString("  WMS写入坐标失败: block=%1, (x=%2, y=%3, z=%4, deg=%5), 错误: %6")
                           .arg(blockIndex)
                           .arg(point.x, 0, 'f', 1).arg(point.y, 0, 'f', 1)
                           .arg(point.z, 0, 'f', 1).arg(point.deg, 0, 'f', 2)
                           .arg(error),
                       "ERROR");
            qDebug() << "[WMS] adjustPosition failed"
                     << "block" << blockIndex
                     << "x" << point.x << "y" << point.y << "z" << point.z << "deg" << point.deg
                     << "error" << error;
        });
    
    // ========================================
    // 初始化批量精定位管理器
    // ========================================
    m_batchFinePositioningManager = new BatchFinePositioningManager(this);
    m_batchFinePositioningManager->setAlignMax(5);         // 最大精定位重试次数
    m_batchFinePositioningManager->setCaptureDelayMs(5000); // 拍照延时 5s
    
    // 连接批量精定位管理器信号
    connect(m_batchFinePositioningManager, &BatchFinePositioningManager::requestAdjustPosition,
            this, &Widget::onBatchPositioningRequestAdjustPosition);
    connect(m_batchFinePositioningManager, &BatchFinePositioningManager::requestCameraCapture,
            this, &Widget::onBatchPositioningRequestCameraCapture);
    connect(m_batchFinePositioningManager, &BatchFinePositioningManager::batchPositioningCompleted,
            this, &Widget::onBatchPositioningCompleted);
    connect(m_batchFinePositioningManager, &BatchFinePositioningManager::batchPositioningFailed,
            this, &Widget::onBatchPositioningFailed);
    connect(m_batchFinePositioningManager, &BatchFinePositioningManager::progressUpdated,
            this, &Widget::onBatchPositioningProgressUpdated);
    
    // 连接ApiServer的WMS回调信号
    connect(m_apiServer, &ApiServer::positionReady, this, &Widget::onWmsPositionReady);
    
    // ========================================
    // 初始化算法工作线程
    // ========================================
    m_algorithmThread = new QThread(this);
    m_algorithmWorker = new AnodeAlgorithmWorker();
    m_algorithmWorker->moveToThread(m_algorithmThread);
    
    // 连接算法工作线程信号
    connect(m_algorithmWorker, &AnodeAlgorithmWorker::processingFinished,
            this, &Widget::onAlgorithmProcessingFinished);
    connect(m_algorithmWorker, &AnodeAlgorithmWorker::processingFailed,
            this, &Widget::onAlgorithmProcessingFailed);
    connect(m_algorithmWorker, &AnodeAlgorithmWorker::processingProgress,
            this, &Widget::onAlgorithmProcessingProgress);
    
    // 启动算法工作线程
    m_algorithmThread->start();

    // 初始同步算法安全参数，确保与 UI 默认值一致
    applyAlgorithmParameterSettings();
    
    // 初始同步相机拍照位参数
    onCameraPositionParameterChanged();

    connect(m_algorithmWorker, &AnodeAlgorithmWorker::finePositioningFinished,
            this, &Widget::onFinePositioningFinished);
    connect(m_algorithmWorker, &AnodeAlgorithmWorker::finePositioningFailed,
            this, &Widget::onFinePositioningFailed);
    
    // ========================================
    // 测试功能：可以通过代码直接调用测试WMS通信
    // ========================================
    // 注意：如果要测试，取消下面这行的注释，程序启动时会自动发送一次测试请求
    // testWmsReport();  //   测试时取消注释
    
    // ========================================
    // 批量精定位测试模式设置（用于测试阶段）
    // ========================================
    // 方式1：启用测试模式，使用拍照位坐标+偏移作为测试坐标
    // setFinePositioningTestMode(true);
    // 方式3：禁用测试模式（使用真实相机算法）
    setFinePositioningTestMode(false);
    //
    // 方式2：启用测试模式，使用指定的测试坐标
    // QList<ApiTypes::Point3D> testCoords;
    // ApiTypes::Point3D coord1;
    // coord1.x = 16487.6; coord1.y = 1796.3; coord1.z = 4216.6; coord1.deg = 84.93;
    // testCoords.append(coord1);
    // ApiTypes::Point3D coord2;
    // coord2.x = 19640.8; coord2.y = 1799.3; coord2.z = 4229.4; coord2.deg = 89.92;
    // testCoords.append(coord2);
    // setFinePositioningTestMode(true, testCoords);
    //

    m_isFinePositioningBusy = false;
    m_activeFinePositioningBlock = -1;
}

Widget::~Widget()
{
    saveSettings();
    
    // 停止API服务器
    if (m_apiServer) {
        m_apiServer->stop();
        delete m_apiServer;
        m_apiServer = nullptr;
    }
    
    // 清理WMS客户端
    if (m_wmsClient) {
        delete m_wmsClient;
        m_wmsClient = nullptr;
    }
    
    // 清理批量精定位管理器
    if (m_batchFinePositioningManager) {
        m_batchFinePositioningManager->stopBatchPositioning();
        delete m_batchFinePositioningManager;
        m_batchFinePositioningManager = nullptr;
    }
    
    if (m_radarController && m_radarController->isConnected()) {
        m_radarController->disconnectRadar();
    }
    
    // 停止算法工作线程
    if (m_algorithmThread) {
        m_algorithmThread->quit();
        m_algorithmThread->wait(3000);  // 等待最多3秒
        if (m_algorithmWorker) {
            delete m_algorithmWorker;
            m_algorithmWorker = nullptr;
        }
        delete m_algorithmThread;
        m_algorithmThread = nullptr;
    }
    
    delete ui;
}


void Widget::setupConnections()
{
    // 操作模式切换连接（单个按钮切换模式）
    connect(ui->operationModeBtn, &QPushButton::clicked, this, [this]() {
        // 切换模式
        if (isTestMode()) {
            setOperationMode(OperationMode::ProductionMode);
        } else {
            setOperationMode(OperationMode::TestMode);
        }
    });
    
    // 初始化模式（默认测试模式）
    setOperationMode(OperationMode::TestMode);
    
    // 顶部按钮连接
    connect(ui->cameraConnectBtn, &QPushButton::clicked, this, &Widget::onCameraConnectBtnClicked);
    connect(ui->cameraTriggerBtn, &QPushButton::clicked, this, &Widget::onCameraTriggerBtnClicked);
    
    // 连接超时定时器
    connect(m_connectionTimer, &QTimer::timeout, this, &Widget::onConnectionTimeout);
    
    // 相机1连接
    connect(m_camera1Controller, &PhoxiController::statusChanged,
            this, &Widget::onCamera1StatusChanged);
    connect(m_camera1Controller, &PhoxiController::imageReady,
            this, &Widget::onCamera1ImageReady);
    connect(m_camera1Controller, &PhoxiController::image3dReady,
            this, [this](const s_Image3dS &img) {
                // 拍照完成：保存到内存
                m_manualImg1 = img;
                logMessage("相机1收到3D数据，已保存到内存", "INFO");
                
                // 1. 异步落盘（不等待，不阻塞）
                const QString camId = m_camera1Controller ? m_camera1Controller->getCameraId() : "cam1";
                const bool inFineFlow = m_batchFinePositioningManager &&
                                        m_batchFinePositioningManager->isRunning() &&
                                        m_currentFinePositioningBlockIndex >= 0;
                const int blockIndex = inFineFlow ? m_currentFinePositioningBlockIndex : -1;
                // 深拷贝确保异步线程有独立的数据副本，避免数据竞争，保证WriteImage3DX能保存完整的7个图像
                s_Image3dS imgCopy = m_manualImg1.DeepCopy();
                QtConcurrent::run([this, imgCopy, camId, blockIndex, inFineFlow]() {
                    CameraCaptureArtifacts art = saveImage3dViaWriteImage3DX(
                        imgCopy,
                        camId,
                        blockIndex,
                        inFineFlow ? "fine" : "manual");
                    // 仅批量精定位流程才保存路径到 m_cameraCaptureArtifacts
                    if (inFineFlow && blockIndex >= 0 && !art.datasetBasePaths.isEmpty()) {
                        QMetaObject::invokeMethod(this, [this, blockIndex, art]() {
                            CameraCaptureArtifacts a = m_cameraCaptureArtifacts.value(blockIndex);
                            if (a.datasetBasePaths.isEmpty())
                                a.datasetBasePaths = art.datasetBasePaths;
                            m_cameraCaptureArtifacts.insert(blockIndex, a);
                        }, Qt::QueuedConnection);
                    }
                });
                
                // 2. 批量精定位流程：保存到对应铜垛索引（相机1固定存储在index 0）
                if (inFineFlow && m_currentFinePositioningBlockIndex >= 0) {
                    const int blockIdx = m_currentFinePositioningBlockIndex;
                    CameraCaptureArtifacts artifacts = m_cameraCaptureArtifacts.value(blockIdx);
                    // 确保images至少有2个槽位
                    while (artifacts.images.size() < 2)
                        artifacts.images.append(s_Image3dS());
                    artifacts.images[0] = img;  // 相机1 → index 0
                    m_cameraCaptureArtifacts.insert(blockIdx, artifacts);

                    // 如果两路数据都已就绪（Z图像非空），排队处理
                    if (artifacts.images.size() >= 2
                        && artifacts.images[0].Z.IsInitialized()
                        && artifacts.images[1].Z.IsInitialized()) {
                        enqueueFinePositioningTask(blockIdx);
                    }
                }

                // 3. 手动拍照模式：如果两个相机都完成，从内存触发算法（不等待落盘，与落盘并行）
                if (m_waitingManualDualShot) {
                    tryStartManualDualCameraAlgorithmMemory();
                }
            });
    connect(m_camera1Controller, &PhoxiController::logInfo,
            this, &Widget::onCameraLogInfo);
    
    // 相机图像用于批量精定位（需要同时处理）
    // 注意：onCamera1ImageReady中会判断是否在批量精定位流程中
    connect(m_camera1Controller, &PhoxiController::errorOccurred,
            this, &Widget::onCamera1ErrorOccurred);
    
    // 相机2连接
    connect(m_camera2Controller, &PhoxiController::statusChanged,
            this, &Widget::onCamera2StatusChanged);
    connect(m_camera2Controller, &PhoxiController::imageReady,
            this, &Widget::onCamera2ImageReady);
    connect(m_camera2Controller, &PhoxiController::image3dReady,
            this, [this](const s_Image3dS &img) {
                // 拍照完成：保存到内存
                m_manualImg2 = img;
                logMessage("相机2收到3D数据，已保存到内存", "INFO");
                
                // 1. 异步落盘（不等待，不阻塞）
                const QString camId = m_camera2Controller ? m_camera2Controller->getCameraId() : "cam2";
                const bool inFineFlow = m_batchFinePositioningManager &&
                                        m_batchFinePositioningManager->isRunning() &&
                                        m_currentFinePositioningBlockIndex >= 0;
                const int blockIndex = inFineFlow ? m_currentFinePositioningBlockIndex : -1;
                // 深拷贝确保异步线程有独立的数据副本，避免数据竞争，保证WriteImage3DX能保存完整的7个图像
                s_Image3dS imgCopy = m_manualImg2.DeepCopy();
                QtConcurrent::run([this, imgCopy, camId, blockIndex, inFineFlow]() {
                    CameraCaptureArtifacts art = saveImage3dViaWriteImage3DX(
                        imgCopy,
                        camId,
                        blockIndex,
                        inFineFlow ? "fine" : "manual");
                    // 仅批量精定位流程才保存路径到 m_cameraCaptureArtifacts
                    if (inFineFlow && blockIndex >= 0 && !art.datasetBasePaths.isEmpty()) {
                        QMetaObject::invokeMethod(this, [this, blockIndex, art]() {
                            CameraCaptureArtifacts a = m_cameraCaptureArtifacts.value(blockIndex);
                            if (a.datasetBasePaths.isEmpty())
                                a.datasetBasePaths = art.datasetBasePaths;
                            m_cameraCaptureArtifacts.insert(blockIndex, a);
                        }, Qt::QueuedConnection);
                    }
                });
                
                // 2. 批量精定位流程：保存到对应铜垛索引（相机2固定存储在index 1）
                if (inFineFlow && m_currentFinePositioningBlockIndex >= 0) {
                    const int blockIdx = m_currentFinePositioningBlockIndex;
                    CameraCaptureArtifacts artifacts = m_cameraCaptureArtifacts.value(blockIdx);
                    // 确保images至少有2个槽位
                    while (artifacts.images.size() < 2)
                        artifacts.images.append(s_Image3dS());
                    artifacts.images[1] = img;  // 相机2 → index 1
                    m_cameraCaptureArtifacts.insert(blockIdx, artifacts);

                    // 如果两路数据都已就绪（Z图像非空），排队处理
                    if (artifacts.images.size() >= 2
                        && artifacts.images[0].Z.IsInitialized()
                        && artifacts.images[1].Z.IsInitialized()) {
                        enqueueFinePositioningTask(blockIdx);
                    }
                }

                // 3. 手动拍照模式：如果两个相机都完成，从内存触发算法（不等待落盘，与落盘并行）
                if (m_waitingManualDualShot) {
                    tryStartManualDualCameraAlgorithmMemory();
                }
            });
    connect(m_camera2Controller, &PhoxiController::errorOccurred,
            this, &Widget::onCamera2ErrorOccurred);
    connect(m_camera2Controller, &PhoxiController::logInfo,
            this, &Widget::onCameraLogInfo);
    
    // 相机控件点击连接
    connect(m_camera1Widget, &CameraWidget::cameraClicked,
            this, [this](const QString &name) { logMessage(QString("相机1点击: %1").arg(name)); });
    connect(m_camera2Widget, &CameraWidget::cameraClicked,
            this, [this](const QString &name) { logMessage(QString("相机2点击: %1").arg(name)); });
    
    // 定时器连接
    connect(m_timeTimer, &QTimer::timeout, [this]() {
        // 更新状态栏（包括时间和设备状态）
        updateStatusLabels();
    });
    m_radarReadyTimer->setInterval(2000);  // 2秒轮询一次
    connect(m_radarReadyTimer, &QTimer::timeout, this, &Widget::logRadarReadyStatus);

    // 日志按钮连接
    connect(ui->clearLogBtn, &QPushButton::clicked, this, &Widget::onClearLogClicked);
    connect(ui->saveLogBtn, &QPushButton::clicked, this, &Widget::onSaveLogClicked);

    // 相机拍照位参数连接（值改变时更新算法参数）
    if (m_cameraXSpin && m_cameraYSpin && m_cameraZSpin && m_cameraDegSpin) {
        connect(m_cameraXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &Widget::onCameraPositionParameterChanged);
        connect(m_cameraYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &Widget::onCameraPositionParameterChanged);
        connect(m_cameraZSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &Widget::onCameraPositionParameterChanged);
        connect(m_cameraDegSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &Widget::onCameraPositionParameterChanged);
    } else {
        logMessage("相机拍照位控件未找到，无法绑定参数变更信号（已启用运行时动态创建）", "ERROR");
    }
    
    // 雷达控制按钮连接
    connect(ui->radarConnectBtn, &QPushButton::clicked, this, &Widget::onRadarConnectBtnClicked);
    connect(ui->radarInitZeroBtn, &QPushButton::clicked, this, &Widget::onRadarInitZeroBtnClicked);
    connect(ui->radarSetParamsBtn, &QPushButton::clicked, this, &Widget::onRadarSetParamsBtnClicked);
    connect(ui->radarScanDetectBtn, &QPushButton::clicked, this, &Widget::onRadarScanDetectBtnClicked);
    connect(ui->cameraTriggerBtn, &QPushButton::clicked, this, &Widget::onCameraTriggerBtnClicked);
    connect(ui->interfaceTestBtn, &QPushButton::clicked, this, &Widget::openInterfaceTestDialog);

    // 雷达状态连接
    connect(m_radarController, &RadarController::statusChanged,
            this, &Widget::onRadarStatusChanged);
    connect(m_radarController, &RadarController::pointCloudUpdated,
            this, &Widget::onRadarPointCloudUpdated);
    connect(m_radarController, &RadarController::errorOccurred,
            this, &Widget::onRadarErrorOccurred);
    
    // 算法相关按钮连接
    connect(ui->uploadRadarFileBtn, &QPushButton::clicked, this, &Widget::onUploadRadarFileBtnClicked);
    connect(ui->parameterSettingsBtn, &QPushButton::clicked, this, &Widget::onParameterSettingsBtnClicked);
    connect(ui->testCameraAlgorithmBtn, &QPushButton::clicked, this, &Widget::onTestCameraAlgorithmBtnClicked);
    // 相机测试/内存模式回调（本地测试也输出到日志，并可上报WMS）
    connect(m_algorithmWorker, &AnodeAlgorithmWorker::testDualCameraFinished,
            this, &Widget::onTestDualCameraFinished);
    connect(m_algorithmWorker, &AnodeAlgorithmWorker::testDualCameraFailed,
            this, &Widget::onTestDualCameraFailed);
    connect(m_algorithmWorker, &AnodeAlgorithmWorker::testDualCameraProgress,
            this, &Widget::onTestDualCameraProgress);
}

void Widget::updateUI()
{
    // 主界面只负责相机显示，雷达控制通过弹窗进行b
    updateStatusLabels();
}

void Widget::updateStatusLabels()
{
    // 更新底部状态栏
    QString statusText;

    // 雷达状态
    statusText += "<span style='font-weight: bold;'>雷达:</span> ";
    if (m_radarController && m_radarController->isConnected()) {
        statusText += "<span style='color: #2ECC71; font-size: 16px;'>●</span> <span style='color: #2ECC71;'>已连接</span>　　";
    } else {
        statusText += "<span style='color: #E74C3C; font-size: 16px;'>●</span> <span style='color: #E74C3C;'>未连接</span>　　";
    }

    // 相机状态
    bool anyCameraConnected = false;
    if (m_camera1Controller && (m_camera1Controller->getStatus() == PhoxiStatus::Connected ||
                                  m_camera1Controller->getStatus() == PhoxiStatus::Capturing)) {
        anyCameraConnected = true;
    }
    if (m_camera2Controller && (m_camera2Controller->getStatus() == PhoxiStatus::Connected ||
                                  m_camera2Controller->getStatus() == PhoxiStatus::Capturing)) {
        anyCameraConnected = true;
    }

    statusText += "<span style='font-weight: bold;'>相机:</span> ";
    if (anyCameraConnected) {
        statusText += "<span style='color: #2ECC71; font-size: 16px;'>●</span> <span style='color: #2ECC71;'>已连接</span>　　";
    } else {
        statusText += "<span style='color: #E74C3C; font-size: 16px;'>●</span> <span style='color: #E74C3C;'>未连接</span>　　";
    }

    // 添加当前时间
    statusText += "<span style='color: #95A5A6;'>|</span>　";
    statusText += "<span style='color: #7F8C8D;'>" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "</span>";

    m_statusLabel->setText(statusText);
}

void Widget::logMessage(const QString &message, const QString &level)
{
    // 输出到日志框
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString colorCode;
    QString levelText = level.toUpper();

    // 格式化日志消息
    QString formattedMessage = QString("<span style='color: #95A5A6;'>[%1]</span> "
                                      "<span style='color: %2; font-weight: bold;'>[%3]</span> "
                                      "<span style='color: #ECF0F1;'>%4</span>")
                                    .arg(timestamp)
                                    .arg(colorCode)
                                    .arg(levelText)
                                    .arg(message);

    // 添加到日志框
    ui->logTextEdit->append(formattedMessage);

    // 同步写入日志文件（纯文本）
    const QString logDirPath = QDir(QCoreApplication::applicationDirPath()).filePath("logs");
    QDir().mkpath(logDirPath);
    const QString logFilePath = QDir(logDirPath).filePath(QDate::currentDate().toString("yyyyMMdd") + ".txt");

    QFile file(logFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream ts(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        ts.setCodec("UTF-8");
#endif
        const QString plainLine = QString("[%1][%2] %3").arg(timestamp, levelText, message);
        ts << plainLine << '\n';
    }

    // 自动滚动到底部
    QTextCursor cursor = ui->logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->logTextEdit->setTextCursor(cursor);
}

void Widget::saveSettings()
{
    QSettings settings("AnodeRadar", "MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveGeometry());

    settings.beginGroup("parameters");
    settings.setValue("imageWidth", m_radarImageWidth);
    settings.setValue("safeDistanceTbNear", m_safeDistanceTbNear);
    settings.setValue("safeDistanceToCbOuter", m_safeDistanceToCbOuter);
    settings.endGroup();
}

void Widget::loadSettings()
{
    QSettings settings("AnodeRadar", "MainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());

    settings.beginGroup("parameters");
    m_radarImageWidth = settings.value("imageWidth", m_radarImageWidth).toInt();
    m_safeDistanceTbNear = settings.value("safeDistanceTbNear", m_safeDistanceTbNear).toDouble();
    m_safeDistanceToCbOuter = settings.value("safeDistanceToCbOuter", m_safeDistanceToCbOuter).toDouble();
    settings.endGroup();
}

// 相机1槽函数实现
void Widget::onCamera1StatusChanged(PhoxiStatus status)
{
    QString statusText;

    switch (status) {
    case PhoxiStatus::Disconnected:
        statusText = "未连接";
        m_connectedCameraCount--;
        if (m_connectedCameraCount < 0) m_connectedCameraCount = 0;
        
        // 如果所有相机都断开了，重置所有状态和按钮
        if (m_connectedCameraCount == 0) {
            m_isFileMode = false;
            m_isConnecting = false;
            m_autoConnectingCamera1 = false;  // 清除自动连接标记
            m_autoConnectingCamera2 = false;
            
            // 重置硬件相机连接按钮
            ui->cameraConnectBtn->setText("连接相机");
            ui->cameraConnectBtn->setEnabled(true);
        }
        break;
    case PhoxiStatus::Connecting:
        statusText = "连接中";
        break;
    case PhoxiStatus::Connected:
    {
        statusText = "已连接";
        m_connectedCameraCount++;
        logMessage("相机1连接成功");
        
        // 检查是否所有尝试连接的相机都已连接完成
        // 如果只连接一个相机，连接成功后立即停止定时器
        // 如果连接两个相机，等待两个都连接完成或失败后再停止定时器
        bool cam1Connected = m_camera1Controller && m_camera1Controller->isConnected();
        bool cam2Connected = m_camera2Controller && m_camera2Controller->isConnected();
        
        // 判断是否应该停止定时器：
        // 1. 如果两个相机都连接成功
        // 2. 或者只连接了一个相机（另一个未尝试连接或已失败）
        bool shouldStopTimer = false;
        if (cam1Connected && cam2Connected) {
            // 两个相机都连接成功
            shouldStopTimer = true;
        } else if (cam1Connected) {
            // 相机1连接成功，检查相机2的状态
            if (!m_camera2Controller) {
                // 相机2不存在，立即停止
                shouldStopTimer = true;
            } else {
                // 相机2存在，检查是否已尝试连接
                PhoxiStatus cam2Status = m_camera2Controller->getStatus();
                // 关键修复：如果在自动连接场景下，相机2已被标记为会被连接，则不应提前停止
                // 只有当相机2确实未尝试连接（不是自动连接标记的）且状态是Disconnected或Error时，才停止
                if (cam2Status == PhoxiStatus::Disconnected && !m_autoConnectingCamera2) {
                    // 相机2未尝试连接（且不是自动连接计划中的），停止定时器
                    shouldStopTimer = true;
                } else if (cam2Status == PhoxiStatus::Error) {
                    // 相机2连接失败，停止定时器
                    shouldStopTimer = true;
                }
                // 如果相机2正在连接中，或者是在自动连接计划中但还未开始连接，继续等待
            }
        }
        
        if (shouldStopTimer) {
            // 所有尝试连接的相机都已处理完成，重置状态
            m_isConnecting = false;
            m_autoConnectingCamera1 = false;  // 清除自动连接标记
            m_autoConnectingCamera2 = false;
            m_connectionTimer->stop();
        }
        
        ui->cameraConnectBtn->setEnabled(true);
        ui->cameraConnectBtn->setText("断开相机");
        ui->cameraConnectBtn->setStyleSheet(
            "QPushButton {"
            "    background-color: #607D8B;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 5px;"
            "    font-size: 14px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background-color: #455A64;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #37474F;"
            "}"
        );
        QString msg = QString("成功连接 %1 台相机").arg(m_connectedCameraCount);
        logMessage(msg, "SUCCESS");
        break;
    }
    case PhoxiStatus::Capturing:
        statusText = "采集中";
        break;
    case PhoxiStatus::Error:
        statusText = "错误";
        if (m_isConnecting) {
            logMessage("相机1连接失败", "ERROR");
            // 检查是否所有相机都已处理（连接成功或失败）
            // 如果这是最后一个相机，重置连接状态
            bool cam1Done = m_camera1Controller && 
                           (m_camera1Controller->isConnected() || 
                            m_camera1Controller->getStatus() == PhoxiStatus::Error);
            bool cam2Done = !m_camera2Controller || 
                           (m_camera2Controller->isConnected() || 
                            m_camera2Controller->getStatus() == PhoxiStatus::Error ||
                            m_camera2Controller->getStatus() == PhoxiStatus::Disconnected);
            
            if (cam1Done && cam2Done) {
                m_isConnecting = false;
                m_autoConnectingCamera1 = false;  // 清除自动连接标记
                m_autoConnectingCamera2 = false;
                m_connectionTimer->stop();
                ui->cameraConnectBtn->setEnabled(true);
            }
        }
        break;
    }
    
    if (ui->cameraTriggerBtn) {
        ui->cameraTriggerBtn->setEnabled(m_connectedCameraCount > 0);
    }

    // 输出到日志框
    if (status == PhoxiStatus::Connected) {
        logMessage("相机1连接成功"   );
    } else if (status == PhoxiStatus::Error) {
        logMessage("相机1连接失败"  );
    } else if (status == PhoxiStatus::Capturing) {
        logMessage("相机1开始采集"  );
    }
}

void Widget::onCamera1ImageReady(const QImage &image)
{
    logMessage(QString("相机1收到纹理，尺寸: %1x%2").arg(image.width()).arg(image.height()), "INFO");
    // 纹理用于UI展示，保存逻辑改为在 image3dReady 回调内通过 WriteImage3DX 执行

    if (m_camera1Widget) {
        m_camera1Widget->setImage(image);
    }
}

void Widget::onCamera1ErrorOccurred(const QString &error)
{
    logMessage(QString("相机1错误: %1").arg(error)  );
}

// 相机2槽函数实现
void Widget::onCamera2StatusChanged(PhoxiStatus status)
{
    QString statusText;
    switch (status) {
    case PhoxiStatus::Disconnected:
        statusText = "未连接";
        m_connectedCameraCount--;
        if (m_connectedCameraCount < 0) m_connectedCameraCount = 0;
        
        // 如果所有相机都断开了，重置所有状态和按钮
        if (m_connectedCameraCount == 0) {
            m_isFileMode = false;
            m_isConnecting = false;
            m_autoConnectingCamera1 = false;  // 清除自动连接标记
            m_autoConnectingCamera2 = false;
            m_connectionTimer->stop();
            
            // 重置硬件相机连接按钮
            ui->cameraConnectBtn->setText("连接相机");
            ui->cameraConnectBtn->setEnabled(true);
            ui->cameraConnectBtn->setStyleSheet(
                "QPushButton {"
                "    background-color: #607D8B;"
                "    color: white;"
                "    border: none;"
                "    border-radius: 5px;"
                "    font-size: 14px;"
                "    font-weight: bold;"
                "}"
                "QPushButton:hover {"
                "    background-color: #455A64;"
                "}"
                "QPushButton:pressed {"
                "    background-color: #37474F;"
                "}"
            );
            logMessage("所有相机已断开", "INFO");
        }
        break;
    case PhoxiStatus::Connecting:
        statusText = "连接中";
        break;
    case PhoxiStatus::Connected:
    {
        statusText = "已连接";
        m_connectedCameraCount++;
        logMessage("相机2连接成功");
        
        // 检查是否所有尝试连接的相机都已连接完成
        // 如果只连接一个相机，连接成功后立即停止定时器
        // 如果连接两个相机，等待两个都连接完成或失败后再停止定时器
        bool cam1Connected = m_camera1Controller && m_camera1Controller->isConnected();
        bool cam2Connected = m_camera2Controller && m_camera2Controller->isConnected();
        
        // 判断是否应该停止定时器：
        // 1. 如果两个相机都连接成功
        // 2. 或者只连接了一个相机（另一个未尝试连接或已失败）
        bool shouldStopTimer = false;
        if (cam1Connected && cam2Connected) {
            // 两个相机都连接成功
            shouldStopTimer = true;
        } else if (cam2Connected) {
            // 相机2连接成功，检查相机1的状态
            if (!m_camera1Controller) {
                // 相机1不存在，立即停止
                shouldStopTimer = true;
            } else {
                // 相机1存在，检查是否已尝试连接
                PhoxiStatus cam1Status = m_camera1Controller->getStatus();
                // 关键修复：如果在自动连接场景下，相机1已被标记为会被连接，则不应提前停止
                // 只有当相机1确实未尝试连接（不是自动连接标记的）且状态是Disconnected或Error时，才停止
                if (cam1Status == PhoxiStatus::Disconnected && !m_autoConnectingCamera1) {
                    // 相机1未尝试连接（且不是自动连接计划中的），停止定时器
                    shouldStopTimer = true;
                } else if (cam1Status == PhoxiStatus::Error) {
                    // 相机1连接失败，停止定时器
                    shouldStopTimer = true;
                }
                // 如果相机1正在连接中，或者是在自动连接计划中但还未开始连接，继续等待
            }
        }
        
        if (shouldStopTimer) {
            // 所有尝试连接的相机都已处理完成，重置状态
            m_isConnecting = false;
            m_autoConnectingCamera1 = false;  // 清除自动连接标记
            m_autoConnectingCamera2 = false;
            m_connectionTimer->stop();
        }
        
        ui->cameraConnectBtn->setEnabled(true);
        ui->cameraConnectBtn->setText("断开相机");
        ui->cameraConnectBtn->setStyleSheet(
            "QPushButton {"
            "    background-color: #607D8B;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 5px;"
            "    font-size: 14px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background-color: #455A64;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #37474F;"
            "}"
        );
        QString msg = QString("成功连接 %1 台相机").arg(m_connectedCameraCount);
        logMessage(msg, "SUCCESS");
        break;
    }
    case PhoxiStatus::Capturing:
        statusText = "采集中";
        break;
    case PhoxiStatus::Error:
        statusText = "错误";
        if (m_isConnecting) {
            logMessage("相机2连接失败", "ERROR");
            // 检查是否所有相机都已处理（连接成功或失败）
            // 如果这是最后一个相机，重置连接状态
            bool cam1Done = !m_camera1Controller || 
                           (m_camera1Controller->isConnected() || 
                            m_camera1Controller->getStatus() == PhoxiStatus::Error ||
                            m_camera1Controller->getStatus() == PhoxiStatus::Disconnected);
            bool cam2Done = m_camera2Controller && 
                           (m_camera2Controller->isConnected() || 
                            m_camera2Controller->getStatus() == PhoxiStatus::Error);
            
            if (cam1Done && cam2Done) {
                m_isConnecting = false;
                m_autoConnectingCamera1 = false;  // 清除自动连接标记
                m_autoConnectingCamera2 = false;
                m_connectionTimer->stop();
                ui->cameraConnectBtn->setEnabled(true);
            }
        }
        break;
    }
    
    if (ui->cameraTriggerBtn) {
        ui->cameraTriggerBtn->setEnabled(m_connectedCameraCount > 0);
    }

    // 输出到日志框
    if (status == PhoxiStatus::Connected) {
        logMessage("相机2连接成功"   );
    } else if (status == PhoxiStatus::Error) {
        logMessage("相机2连接失败"  );
    } else if (status == PhoxiStatus::Capturing) {
        logMessage("相机2开始采集" );
    }
}

void Widget::onCamera2ImageReady(const QImage &image)
{
    logMessage(QString("相机2收到纹理，尺寸: %1x%2").arg(image.width()).arg(image.height()), "INFO");
    // 纹理用于UI展示，保存逻辑改为在 image3dReady 回调内通过 WriteImage3DX 执行
    if (m_camera2Widget) {
        m_camera2Widget->setImage(image);
    }
}

void Widget::onCamera2ErrorOccurred(const QString &error)
{
    logMessage(QString("相机2错误: %1").arg(error)  );
}

// 相机连接按钮槽函数实现
// 统一相机连接按钮槽函数
void Widget::onCameraConnectBtnClicked()
{
    if (m_cameraConnectMenu) {
        const QPoint globalPos = ui->cameraConnectBtn->mapToGlobal(QPoint(0, ui->cameraConnectBtn->height()));
        m_cameraConnectMenu->exec(globalPos);
    }
}

// 连接超时槽函数
void Widget::onConnectionTimeout()
{
    if (!m_isConnecting) {
        return;  // 已经不在连接状态，忽略超时
    }
    
    logMessage("相机连接超时（10秒），请检查网络和设备");
    onDisconnectAllCameras();
}

// 相机连接菜单动作
void Widget::onConnectCamera1()
{
    connectSingleCamera(m_camera1Controller, "相机1");
}

void Widget::onConnectCamera2()
{
    connectSingleCamera(m_camera2Controller, "相机2");
}

void Widget::onDisconnectAllCameras()
{
    // 先停止采集
    if (m_camera1Controller && m_camera1Controller->isCapturing()) {
        m_camera1Controller->stopCapture();
    }
    if (m_camera2Controller && m_camera2Controller->isCapturing()) {
        m_camera2Controller->stopCapture();
    }
    
    // 断开连接（异步操作，状态回调会稍后到达）
    if (m_camera1Controller && m_camera1Controller->isConnected()) {
        m_camera1Controller->disconnectCamera();
    }
    if (m_camera2Controller && m_camera2Controller->isConnected()) {
        m_camera2Controller->disconnectCamera();
    }
    
    // 注意：不要立即重置 m_connectedCameraCount，因为断开是异步的
    // 状态回调（onCamera1StatusChanged/onCamera2StatusChanged）会在断开完成后
    // 自动更新计数，并在所有相机都断开后重置状态
    
    // 但是可以立即停止连接定时器和重置连接标志
    m_isConnecting = false;
    m_autoConnectingCamera1 = false;  // 清除自动连接标记
    m_autoConnectingCamera2 = false;
    m_isFileMode = false;
    m_connectionTimer->stop();
    
    if (ui->cameraTriggerBtn) {
        ui->cameraTriggerBtn->setEnabled(false);
    }

    // 清理图像显示
    if (m_camera1Widget) m_camera1Widget->clearImage();
    if (m_camera2Widget) m_camera2Widget->clearImage();
    
    // 如果两个相机都已经断开（同步检查），立即更新UI
    bool cam1Disconnected = !m_camera1Controller || !m_camera1Controller->isConnected();
    bool cam2Disconnected = !m_camera2Controller || !m_camera2Controller->isConnected();
    
    if (cam1Disconnected && cam2Disconnected) {
        // 所有相机都已断开，立即重置计数和UI
        m_connectedCameraCount = 0;
        ui->cameraConnectBtn->setEnabled(true);
        ui->cameraConnectBtn->setText("连接相机");
        ui->cameraConnectBtn->setStyleSheet(
            "QPushButton {"
            "    background-color: #607D8B;"
            "    color: white;"
            "    border: none;"
            "    border-radius: 5px;"
            "    font-size: 14px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background-color: #455A64;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #37474F;"
            "}"
        );
        logMessage("所有相机已断开，图像已清除", "INFO");
    } else {
        // 还有相机在断开中，等待状态回调更新
        logMessage("正在断开相机连接...", "INFO");
    }
}

void Widget::connectSingleCamera(PhoxiController *controller, const QString &name)
{
    if (!controller) return;

    if (controller->isConnected()) {
        logMessage(QString("%1已连接").arg(name), "INFO");
        return;
    }

    // 检查是否正在连接中，如果是则只更新UI，不重复设置状态
    if (!m_isConnecting) {
        m_isConnecting = true;
        m_isFileMode = false;  // 硬件模式
        // 如果是手动连接（不是自动连接），清除自动连接标记
        // 注意：自动连接时，标记已在 autoConnectDevices 中设置
        if (!m_autoConnectingCamera1 && !m_autoConnectingCamera2) {
            m_autoConnectingCamera1 = false;
            m_autoConnectingCamera2 = false;
        }
        // 启动超时定时器（10秒）- 只在第一次连接时启动
        m_connectionTimer->start();
    }
    
    controller->setConnectionMode(PhoxiConnectionMode::HardwareOnly);
        
    ui->cameraConnectBtn->setEnabled(false);
    ui->cameraConnectBtn->setText(QString("连接%1...").arg(name));
    ui->cameraConnectBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #78909C;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "}"
    );
        
    logMessage(QString("正在连接%1...").arg(name));
        
    controller->connectCamera();
}

void Widget::autoConnectDevices()
{
    logMessage("=== 开始自动连接设备 ===", "INFO");
    
    // 标记哪些相机会被自动连接（在连接之前标记，用于状态判断）
    m_autoConnectingCamera1 = false;
    m_autoConnectingCamera2 = false;
    
    // 自动连接相机1和相机2
    if (m_camera1Controller && !m_camera1Controller->isConnected()) {
        logMessage("自动连接相机1...", "INFO");
        m_autoConnectingCamera1 = true;  // 标记相机1会被连接
        connectSingleCamera(m_camera1Controller, "相机1");
    } else if (m_camera1Controller && m_camera1Controller->isConnected()) {
        logMessage("相机1已连接，跳过", "INFO");
    }
    
    // 延迟500ms后连接相机2，避免同时连接造成冲突
    QTimer::singleShot(500, this, [this]() {
        if (m_camera2Controller && !m_camera2Controller->isConnected()) {
            logMessage("自动连接相机2...", "INFO");
            m_autoConnectingCamera2 = true;  // 标记相机2会被连接
            connectSingleCamera(m_camera2Controller, "相机2");
        } else if (m_camera2Controller && m_camera2Controller->isConnected()) {
            logMessage("相机2已连接，跳过", "INFO");
        }
    });
    
    // 延迟1000ms后连接雷达，给相机连接一些时间
    QTimer::singleShot(1000, this, [this]() {
        if (m_radarController && !m_radarController->isConnected()) {
            logMessage("自动连接雷达...", "INFO");
            // 使用默认IP地址
            QString platformIP = "192.168.0.10";
            QString lidarIP = "192.168.0.7";
            
            // 禁用按钮，防止重复点击
            if (ui->radarConnectBtn) {
                ui->radarConnectBtn->setEnabled(false);
                ui->radarConnectBtn->setText("连接中...");
            }
            
            // 连接雷达（异步连接）
            if (!m_radarController->connectRadar(platformIP, lidarIP)) {
                // 如果连接启动失败，恢复按钮状态
                if (ui->radarConnectBtn) {
                    ui->radarConnectBtn->setEnabled(true);
                    ui->radarConnectBtn->setText("连接雷达");
                }
                logMessage("雷达连接启动失败", "ERROR");
            } else {
                logMessage("雷达连接请求已发送，等待连接结果...", "INFO");
            }
        } else if (m_radarController && m_radarController->isConnected()) {
            logMessage("雷达已连接，跳过", "INFO");
        }
    });
    
    logMessage("自动连接设备请求已发送，请等待连接结果...", "INFO");
}

// 日志控制槽函数
void Widget::onClearLogClicked()
{
    ui->logTextEdit->clear();
    logMessage("日志已清除");
}

void Widget::onSaveLogClicked()
{
    QString defaultFileName = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + "_system_log.txt";
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString defaultPath = QDir(desktopPath).filePath(defaultFileName);

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "保存日志文件",
                                                    defaultPath,
                                                    "文本文件 (*.txt);;所有文件 (*.*)");

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        logMessage(QString("无法保存日志文件: %1").arg(file.errorString())  );
        return;
    }

    QTextStream out(&file);

    // 写入纯文本日志
    QString logContent = ui->logTextEdit->toPlainText();
    out << "青海阳极卸车识别系统 - 系统日志\n";
    out << "生成时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    out << logContent;

    file.close();

    logMessage(QString("日志已保存至: %1").arg(fileName)   );
}

// 雷达控制槽函数实现
void Widget::onRadarConnectBtnClicked()
{
    if (m_radarController->isConnected()) {
        // 断开雷达
        m_radarController->disconnectRadar();
        ui->radarConnectBtn->setText("连接雷达");
        ui->radarInitZeroBtn->setEnabled(false);
        ui->radarSetParamsBtn->setEnabled(false);
        logMessage("雷达已断开");
    } else {
        // 连接雷达（异步连接，不立即更新UI）
        QString platformIP = "192.168.0.10";  // 使用默认IP
        QString lidarIP = "192.168.0.7";

        // 禁用按钮，防止重复点击
        ui->radarConnectBtn->setEnabled(false);
        ui->radarConnectBtn->setText("连接中...");
        
        logMessage("正在连接雷达..."   );
        
        // 注意：connectRadar 是异步的，会立即返回 true
        // 实际连接结果通过 statusChanged 信号通知
        if (!m_radarController->connectRadar(platformIP, lidarIP)) {
            // 如果连接启动失败，立即恢复按钮状态
            ui->radarConnectBtn->setEnabled(true);
            ui->radarConnectBtn->setText("连接雷达");
            logMessage("雷达连接启动失败"  );
        }
        // 如果连接启动成功，等待 statusChanged 信号更新UI状态
    }
}

void Widget::onRadarInitZeroBtnClicked()
{
    if (!m_radarController->isConnected()) {
        logMessage("请先连接雷达"    );
        return;
    }

    if (m_radarController->initToZeroPoint()) {
        logMessage("雷达回零点成功"   );
    } else {
        logMessage("雷达回零点失败"  );
    }
}

void Widget::onRadarSetParamsBtnClicked()
{
    if (!m_radarController->isConnected()) {
        logMessage("请先连接雷达");
        return;
    }

    double startAngle = ui->radarStartAngleSpin->value();
    double stopAngle = ui->radarStopAngleSpin->value();
    double velocity = ui->radarVelocitySpin->value();

    if (m_radarController->setScanParameters(startAngle, stopAngle, velocity)) {
        logMessage(QString("扫描参数设置成功: 起始=%1°, 结束=%2°, 速度=%3°/s")
                  .arg(startAngle, 0, 'f', 1)
                  .arg(stopAngle, 0, 'f', 1)
                  .arg(velocity, 0, 'f', 1)   );
        logMessage("开始监控雷达归位/就绪状态，每2秒打印一次", "INFO");
        if (m_radarReadyTimer && !m_radarReadyTimer->isActive()) {
            m_radarReadyTimer->start();
        }
    } else {
        logMessage("扫描参数设置失败"  );
    }
}

void Widget::onRadarScanDetectBtnClicked()
{
    if (!m_radarController || !m_radarController->isConnected()) {
        logMessage("请先连接雷达", "ERROR");
        return;
    }

    ui->radarScanDetectBtn->setEnabled(false);
    if (!startRadarScanAndDetect("UI")) {
        ui->radarScanDetectBtn->setEnabled(true);
        return;
    }

    logMessage("已触发雷达扫描并启动算法检测", "INFO");
}

void Widget::onCameraTriggerBtnClicked()
{
    if (!m_cameraTriggerMenu) {
        return;
    }
    // 按钮位置弹出菜单，供选择相机1/相机2单拍
    QPoint globalPos = ui->cameraTriggerBtn->mapToGlobal(QPoint(0, ui->cameraTriggerBtn->height()));
    m_cameraTriggerMenu->exec(globalPos);
}

void Widget::onCamera1ShotRequested()
{
    // UI手动触发：自动切换到测试模式
    if (isProductionMode()) {
        logMessage("检测到UI手动触发，自动切换到测试模式", "INFO");
        setOperationMode(OperationMode::TestMode);
    }
    
    if (!m_camera1Controller || !m_camera1Controller->isConnected()) {
        logMessage("相机1未连接，无法拍照", "ERROR");
        return;
    }
    logMessage(" [测试模式] 触发相机1单次采集", "INFO");
    if (!m_waitingManualDualShot) {
        // 开启新的双机采集循环，清空缓存
        m_manualCam1Paths.clear();
        m_manualCam2Paths.clear();
        m_manualImg1.Clear();
        m_manualImg2.Clear();
        logMessage("开始新的双机采集循环，缓存已清空，等待两路数据", "INFO");
    }
    m_waitingManualDualShot = true;
    m_manualImg1.Clear();
    m_camera1Controller->captureSingleFrame3d();
}

void Widget::onCamera2ShotRequested()
{
    // UI手动触发：自动切换到测试模式
    if (isProductionMode()) {
        logMessage("检测到UI手动触发，自动切换到测试模式", "INFO");
        setOperationMode(OperationMode::TestMode);
    }
    
    if (!m_camera2Controller || !m_camera2Controller->isConnected()) {
        logMessage("相机2未连接，无法拍照", "ERROR");
        return;
    }
    logMessage(" [测试模式] 触发相机2单次采集", "INFO");
    if (!m_waitingManualDualShot) {
        // 开启新的双机采集循环，清空缓存
        m_manualCam1Paths.clear();
        m_manualCam2Paths.clear();
        m_manualImg1.Clear();
        m_manualImg2.Clear();
        logMessage("开始新的双机采集循环，缓存已清空，等待两路数据", "INFO");
    }
    m_waitingManualDualShot = true;
    m_manualImg2.Clear();
    m_camera2Controller->captureSingleFrame3d();
}


/**
 * 统一的雷达扫描触发接口
 * triggerSource 用于区分触发来源（例如 "UI"、"API"）
 */
bool Widget::startRadarScanAndDetect(const QString &triggerSource)
{
    // 根据触发源自动设置操作模式
    if (triggerSource == "API") {
        // API触发：自动切换到正式模式
        if (isTestMode()) {
            logMessage("检测到API触发，自动切换到正式模式", "INFO");
            setOperationMode(OperationMode::ProductionMode);
        }
    } else if (triggerSource == "UI") {
        // UI触发：自动切换到测试模式
        if (isProductionMode()) {
            logMessage("检测到UI手动触发，自动切换到测试模式", "INFO");
            setOperationMode(OperationMode::TestMode);
        }
    }
    
    if (!m_radarController || !m_radarController->isConnected()) {
        logMessage(QString("无法开始雷达扫描（触发源: %1）：雷达未连接").arg(triggerSource), "ERROR");
        return false;
    }

    const RadarStatus currentStatus = m_radarController->getStatus();
    logMessage(QString("准备启动雷达扫描（触发源: %1，模式: %2），当前状态: %3")
                   .arg(triggerSource)
                   .arg(isTestMode() ? "测试模式" : "正式模式")
                   .arg(m_radarController->getStatusString()),
               "INFO");

    // 如果上一次扫描未正常结束，先尝试停止
    if (currentStatus == RadarStatus::Scanning) {
        logMessage("检测到雷达仍在扫描，尝试先停止再重启", "WARNING");
        m_radarController->stopScan();
        QThread::msleep(100);
    }

    // 启动单次扫描，扫描完成后由 RadarController 通过 pointCloudUpdated 信号
    // 触发 Widget::onRadarPointCloudUpdated，从而自动进入算法检测流程
    if (m_radarController->startScan()) {
        logMessage(QString("开始雷达扫描（触发源: %1）").arg(triggerSource), "INFO");
        m_captureNextRadarFrameForStorage = true;
        m_lastScanTriggerSource = triggerSource;
        return true;
    } else {
        logMessage(QString("启动雷达扫描失败（触发源: %1，状态: %2）")
                       .arg(triggerSource, m_radarController->getStatusString()),
                   "ERROR");
        if (isProductionMode()) {
            reportFailureToWms("雷达扫描启动失败");
        }
        return false;
    }
}

// 雷达状态槽函数实现
void Widget::onRadarStatusChanged(RadarStatus status)
{
    QString statusText;
    switch (status) {
    case RadarStatus::Disconnected:
        statusText = "未连接";
        ui->radarConnectBtn->setEnabled(true);
        ui->radarConnectBtn->setText("连接雷达");
        ui->radarInitZeroBtn->setEnabled(false);
        ui->radarSetParamsBtn->setEnabled(false);
        ui->radarScanDetectBtn->setEnabled(false);
        break;
    case RadarStatus::Connecting:
        statusText = "连接中";
        ui->radarConnectBtn->setEnabled(false);
        ui->radarConnectBtn->setText("连接中...");
        ui->radarInitZeroBtn->setEnabled(false);
        ui->radarSetParamsBtn->setEnabled(false);
        ui->radarScanDetectBtn->setEnabled(false);
        break;
    case RadarStatus::Connected:
        statusText = "已连接";
        ui->radarConnectBtn->setEnabled(true);
        ui->radarConnectBtn->setText("断开雷达");
        ui->radarInitZeroBtn->setEnabled(true);
        ui->radarSetParamsBtn->setEnabled(true);
        ui->radarScanDetectBtn->setEnabled(true);
        logMessage("雷达连接成功", "SUCCESS");
        break;
    case RadarStatus::Scanning:
        statusText = "扫描中";
        ui->radarScanDetectBtn->setEnabled(false);
        break;
    case RadarStatus::Error:
        statusText = "错误";
        ui->radarConnectBtn->setEnabled(true);
        ui->radarConnectBtn->setText("连接雷达");
        ui->radarInitZeroBtn->setEnabled(false);
        ui->radarSetParamsBtn->setEnabled(false);
        ui->radarScanDetectBtn->setEnabled(false);
        logMessage("雷达连接失败或发生错误", "ERROR");
        break;
    }

    logMessage(QString("雷达状态: %1").arg(statusText));
}

void Widget::onRadarScanSaveBtnClicked()
{
    if (!m_radarController || !m_radarController->isConnected()) {
        logMessage("请先连接雷达", "ERROR");
        return;
    }

    // 一次性扫描并保存：订阅下一次点云回调，触发一次扫描，拿到点云后保存并停止扫描
    logMessage("准备进行一次扫描并保存PCD...", "INFO");

    // 一次性连接
    QMetaObject::Connection conn;
    conn = connect(m_radarController, &RadarController::pointCloudUpdated, this,
                   [this, conn](const PointCloud &cloud) {
        // 断开一次性连接
        disconnect(conn);

        // 停止扫描（如果在扫描）
        m_radarController->stopScan();

        saveRadarPointCloudSnapshot(cloud, "manual");
    });

    // 触发一次扫描
    if (!m_radarController->startScan()) {
        disconnect(conn);
        logMessage("启动扫描失败，无法保存PCD", "ERROR");
        return;
    }
    logMessage("扫描已启动，正在等待数据...", "INFO");
}

bool Widget::savePointCloudAsPly(const PointCloud &pointCloud, const QString &filePath)
{
    size_t totalPoints = 0;
    for (const auto &line : pointCloud) {
        totalPoints += line.size();
    }

    if (totalPoints == 0) {
        qWarning() << "[Widget] savePointCloudAsPly: 点云为空";
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "[Widget] savePointCloudAsPly: 无法写入" << filePath;
        return false;
    }

    QTextStream out(&file);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(6);

    out << "ply\n";
    out << "format ascii 1.0\n";
    out << "element vertex " << totalPoints << "\n";
    out << "property float x\n";
    out << "property float y\n";
    out << "property float z\n";
    out << "end_header\n";

    for (const auto &line : pointCloud) {
        for (const auto &p : line) {
            out << p.X << " " << p.Y << " " << p.Z << "\n";
        }
    }

    file.close();
    return true;
}
void Widget::onRadarPointCloudUpdated(const PointCloud &pointCloud)
{
    if (ui->radarScanDetectBtn) {
        ui->radarScanDetectBtn->setEnabled(true);
    }
    if (m_radarReadyTimer && m_radarReadyTimer->isActive()) {
        m_radarReadyTimer->stop();
    }

    if (m_captureNextRadarFrameForStorage) {
        const QString stage = m_lastScanTriggerSource.isEmpty()
                                  ? QStringLiteral("scan")
                                  : sanitizeForFilename(m_lastScanTriggerSource);
        saveRadarPointCloudSnapshot(pointCloud, stage);
        m_captureNextRadarFrameForStorage = false;
    }

    // 点云数据更新
    int totalPoints = 0;
    for (const auto &scanLine : pointCloud) {
        totalPoints += scanLine.size();
    }
    logMessage(QString("点云数据更新: %1条线, %2个点").arg(pointCloud.size()).arg(totalPoints));
    
    // 自动启动算法检测（如果算法工作线程已初始化）
    if (m_algorithmWorker) {
        startAlgorithmProcessing(pointCloud);
    }
}

void Widget::onRadarErrorOccurred(const QString &error)
{
    logMessage(QString("雷达错误: %1").arg(error), "ERROR");
    
    // 如果连接过程中发生错误，恢复按钮状态
    if (m_radarController->getStatus() == RadarStatus::Connecting || 
        m_radarController->getStatus() == RadarStatus::Error) {
        ui->radarConnectBtn->setEnabled(true);
        ui->radarConnectBtn->setText("连接雷达");
        ui->radarInitZeroBtn->setEnabled(false);
        ui->radarSetParamsBtn->setEnabled(false);
    }
    if (ui->radarScanDetectBtn) {
        ui->radarScanDetectBtn->setEnabled(m_radarController && m_radarController->isConnected());
    }
    if (m_radarReadyTimer && m_radarReadyTimer->isActive()) {
        m_radarReadyTimer->stop();
    }
}

void Widget::logRadarReadyStatus()
{
    if (!m_radarController || !m_radarController->isConnected()) {
        logMessage("雷达未连接，停止就绪状态监控", "WARNING");
        m_radarReadyTimer->stop();
        return;
    }

    Common3DPlatformStatus status = m_radarController->getPlatformStatus();
    QString statusText = m_radarController->getPlatformStatusText();
    logMessage(QString("雷达状态监控：%1").arg(statusText), "INFO");

    // 到达起始位或空闲即视为就绪
    if (status == eStopAtStartPosition_S3 || status == eIdleStatus) {
        logMessage("雷达已就绪，停止状态监控", "SUCCESS");
        m_radarReadyTimer->stop();
    }
}

void Widget::tryStartManualDualCameraAlgorithm()
{
    if (!m_waitingManualDualShot) {
        return;
    }
    if (m_manualCam1Paths.isEmpty() || m_manualCam2Paths.isEmpty()) {
        return;
    }
    if (!m_algorithmWorker) {
        logMessage("算法工作线程未初始化，无法处理相机数据", "ERROR");
        m_waitingManualDualShot = false;
        return;
    }

    logMessage("收到双相机数据，启动相机算法检测", "INFO");
    m_waitingManualDualShot = false;

    // 合并两路基础路径为一个 QStringList（顺序：cam1, cam2）
    QStringList imageBasePaths;
    imageBasePaths << m_manualCam1Paths << m_manualCam2Paths;

    QMetaObject::invokeMethod(m_algorithmWorker, "processTestDualCamera",
                              Qt::QueuedConnection,
                              Q_ARG(QStringList, imageBasePaths));
}

void Widget::tryStartManualDualCameraAlgorithmMemory()
{
    if (!m_waitingManualDualShot) {
        return;
    }
    const bool cam1Ready = m_manualImg1.Gray.IsInitialized();
    const bool cam2Ready = m_manualImg2.Gray.IsInitialized();
    if (cam1Ready && cam2Ready) {
        if (!m_algorithmWorker) {
            logMessage("算法工作线程未初始化，无法处理相机数据", "ERROR");
            m_waitingManualDualShot = false;
            return;
        }
        logMessage("收到双相机内存数据，启动相机算法检测（内存，不等待落盘）", "INFO");
        m_waitingManualDualShot = false;
        
        // 在调用算法前，同步相机拍照位参数
        if (m_cameraXSpin && m_cameraYSpin && m_cameraZSpin && m_cameraDegSpin) {
            QMetaObject::invokeMethod(m_algorithmWorker,
                                      "setCameraPositionParameters",
                                      Qt::BlockingQueuedConnection,
                                      Q_ARG(double, m_cameraXSpin->value()),
                                      Q_ARG(double, m_cameraYSpin->value()),
                                      Q_ARG(double, m_cameraZSpin->value()),
                                      Q_ARG(double, m_cameraDegSpin->value()));
        } else {
            logMessage("相机拍照位控件未初始化，无法同步参数到算法线程", "ERROR");
        }
        
        s_Image3dS img1 = m_manualImg1.DeepCopy();
        s_Image3dS img2 = m_manualImg2.DeepCopy();
        QMetaObject::invokeMethod(m_algorithmWorker, "processTestDualCameraMemory",
                                  Qt::QueuedConnection,
                                  Q_ARG(s_Image3dS, img1),
                                  Q_ARG(s_Image3dS, img2));
    } else {
        logMessage(QString("等待双相机内存数据齐备... (相机1:%1, 相机2:%2)")
                       .arg(cam1Ready ? "就绪" : "未就绪")
                       .arg(cam2Ready ? "就绪" : "未就绪"),
                   "INFO");
    }
}

void Widget::onRadarDetailInfoBtnClicked()
{
    logMessage("详细雷达信息面板已移除，使用主界面按钮进行操作", "INFO");
}

// ============================================
// API服务器查询接口实现
// ============================================

/**
 * 获取相机1状态
 */
QString Widget::getCamera1Status() const
{
    if (!m_camera1Controller) {
        return "Unknown";
    }
    
    PhoxiStatus status = m_camera1Controller->getStatus();
    switch (status) {
        case PhoxiStatus::Disconnected:
            return "Disconnected";
        case PhoxiStatus::Connected:
            return "Connected";
        case PhoxiStatus::Capturing:
            return "Busy";
        case PhoxiStatus::Error:
            return "Error";
        default:
            return "Unknown";
    }
}

/**
 * 获取相机2状态
 */
QString Widget::getCamera2Status() const
{
    if (!m_camera2Controller) {
        return "Unknown";
    }
    
    PhoxiStatus status = m_camera2Controller->getStatus();
    switch (status) {
        case PhoxiStatus::Disconnected:
            return "Disconnected";
        case PhoxiStatus::Connected:
            return "Connected";
        case PhoxiStatus::Capturing:
            return "Busy";
        case PhoxiStatus::Error:
            return "Error";
        default:
            return "Unknown";
    }
}

/**
 * 获取雷达状态
 */
QString Widget::getRadarStatus() const
{
    if (!m_radarController) {
        return "Unknown";
    }
    
    RadarStatus status = m_radarController->getStatus();
    switch (status) {
        case RadarStatus::Disconnected:
            return "Disconnected";
        case RadarStatus::Connected:
            return "Connected";
        case RadarStatus::Scanning:
            return "Scanning";
        case RadarStatus::Error:
            return "Error";
        default:
            return "Unknown";
    }
}

QString Widget::getAlgorithmStatus() const
{
    switch (m_algorithmStatus) {
    case AlgorithmStatus::Ready:
        return "Ready";
    case AlgorithmStatus::Processing:
        return "Processing";
    case AlgorithmStatus::Error:
        return "Error";
    default:
        return "Unknown";
    }
}

/**
 * 获取系统总体状态
 */
QString Widget::getSystemStatus() const
{
    QString cam1 = getCamera1Status();
    QString cam2 = getCamera2Status();
    QString radar = getRadarStatus();
    
    int connectedCount = 0;
    if (cam1 == "Connected") connectedCount++;
    if (cam2 == "Connected") connectedCount++;
    if (radar == "Connected") connectedCount++;
    
    if (connectedCount == 3) {
        return "OK";
    } else if (connectedCount > 0) {
        return "Partial";
    } else {
        return "Error";
    }
}

/**
 * @brief 测试WMS上报功能
 * 
 * 功能：创建一个测试结果并发送到WMS
 * 使用：在构造函数中调用 testWmsReport() 进行测试
 * 
 *   测试前请确保：
 * 1. 已配置正确的WMS地址（在 config/scan_config.json 中）
 * 2. WMS服务器或Mock Server已启动并可以接收请求
 */
void Widget::testWmsReport()
{
    if (!m_wmsClient) {
        logMessage("  WMS客户端未初始化，无法测试", "ERROR");
        return;
    }
    
    logMessage(" 开始测试WMS上报功能...", "INFO");
    
    // 创建测试数据
    ApiTypes::ScanResult testResult = ApiTypes::ScanResult::success(
        "TEST_" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss"),  // 任务编号
        "1",  // 类型：1=雷达，2=相机
        "这是测试数据，用于验证WMS通信功能"
    );
    
    // 添加测试坐标点
    testResult.data.append(ApiTypes::Point3D{1234.56, 2345.67, 3456.78, true});
    testResult.data.append(ApiTypes::Point3D{1250.00, 2360.00, 3460.00, true});
    testResult.data.append(ApiTypes::Point3D{1265.00, 2375.00, 3465.00, true});
    
    // base64图片
    testResult.base64 = "";  //   实际使用时需要生成结果图片
    
    logMessage(QString(" 发送测试数据到WMS:")
               + QString("\n   - 任务编号: %1").arg(testResult.uniqueCode)
               + QString("\n   - 类型: %1").arg(testResult.type)
               + QString("\n   - 坐标点数: %1").arg(testResult.data.size())
               + QString("\n   - WMS地址: %1").arg(m_wmsClient->wmsUrl()),
               "INFO");
    
    // 发送请求
    m_wmsClient->reportResult(testResult);
    
    logMessage(" 等待WMS响应...", "INFO");
}

/**
 * @brief WMS测试按钮槽函数
 * 
 * 当UI中有按钮连接到这个槽函数时，点击按钮会触发WMS测试
 */
void Widget::onTestWmsReportClicked()
{
    // 直接调用测试函数
    testWmsReport();
}

// ========================================
// 算法相关函数实现
// ========================================

bool Widget::processRadarFileUpload(const QString &filePath)
{
    qDebug() << "[ProcessRadarFile] API上传文件:" << filePath;

    // 使用当前参数设置中的宽度值
    int mapWidth = qMax(1, m_radarImageWidth);
    qDebug() << "[ProcessRadarFile] Current parameters -> width:" << mapWidth
             << "tbNear:" << m_safeDistanceTbNear
             << "toCbOuter:" << m_safeDistanceToCbOuter;
    
    if (mapWidth <= 0) {
        logMessage("图像宽度必须大于0，请检查参数设置", "ERROR");
        qDebug() << "[ProcessRadarFile] Invalid map width:" << mapWidth;
        return false;
    }
    
    // 检查文件是否存在
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        logMessage(QString("文件不存在: %1").arg(filePath), "ERROR");
        qDebug() << "[ProcessRadarFile] File not exists:" << filePath;
        return false;
    }
    
    logMessage(QString("开始处理雷达文件: %1 (宽度: %2)").arg(filePath).arg(mapWidth), "INFO");
    qDebug() << "[ProcessRadarFile] Processing file:" << filePath;
    
    // 在算法工作线程中处理文件
    if (m_algorithmWorker) {
        m_algorithmStatus = AlgorithmStatus::Processing;
        qDebug() << "[ProcessRadarFile] Worker exists, dispatching task";

        // 将界面上的安全间距参数同步到算法线程
        applyAlgorithmParameterSettings();

        QMetaObject::invokeMethod(m_algorithmWorker, "processLocalFile",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, filePath),
                                  Q_ARG(int, mapWidth));
        return true;
    } else {
        logMessage("算法工作线程未初始化，无法处理本地雷达文件", "ERROR");
        qDebug() << "[ProcessRadarFile] Worker is null, cannot process file";
        m_algorithmStatus = AlgorithmStatus::Error;
        return false;
    }
}

void Widget::onUploadRadarFileBtnClicked()
{
    qDebug() << "[UploadRadarFile] Triggered";

    // 使用当前参数设置中的宽度值
    int mapWidth = qMax(1, m_radarImageWidth);
    qDebug() << "[UploadRadarFile] Current parameters -> width:" << mapWidth
             << "tbNear:" << m_safeDistanceTbNear
             << "toCbOuter:" << m_safeDistanceToCbOuter;
    
    if (mapWidth <= 0) {
        logMessage("图像宽度必须大于0，请重新输入", "ERROR");
        qDebug() << "[UploadRadarFile] Invalid map width:" << mapWidth;
        return;
    }
    
    // 打开文件选择对话框（算法只支持PLY格式）
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择雷达点云文件（仅支持PLY格式）",
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        "PLY点云文件 (*.ply);;所有文件 (*.*)"
    );
    
    if (filePath.isEmpty()) {
        qDebug() << "[UploadRadarFile] User canceled file dialog";
        return;
    }
    
    // 调用统一处理方法
    processRadarFileUpload(filePath);
}

void Widget::onParameterSettingsBtnClicked()
{
    ParameterSettingsDialog dialog(this);
    dialog.setParameters(m_radarImageWidth, m_safeDistanceTbNear, m_safeDistanceToCbOuter);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    m_radarImageWidth = dialog.imageWidth();
    m_safeDistanceTbNear = dialog.safeDistanceTbNear();
    m_safeDistanceToCbOuter = dialog.safeDistanceToCbOuter();

    applyAlgorithmParameterSettings();
    saveSettings();

    logMessage(QString("参数已更新：宽度=%1, 铜跺间距=%2, 边缘间距=%3")
                   .arg(m_radarImageWidth)
                   .arg(QString::number(m_safeDistanceTbNear, 'f', 1))
                   .arg(QString::number(m_safeDistanceToCbOuter, 'f', 1)),
               "INFO");
}

void Widget::onTestCameraAlgorithmBtnClicked()
{
    openCameraTestDialog();
}

void Widget::openCameraTestDialog()
{
    // 创建并显示相机测试对话框
    CameraTestDialog *dialog = new CameraTestDialog(m_algorithmWorker, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);  // 关闭时自动删除
    
    // 连接对话框的日志信号到主窗口
    connect(dialog, &CameraTestDialog::logToMainWindow, this, &Widget::logMessage);
    connect(dialog, &CameraTestDialog::cameraImageSelected, this, [this](int camIdx, const QImage &img) {
        if (camIdx == 1 && m_camera1Widget) {
            m_camera1Widget->setImage(img);
        } else if (camIdx == 2 && m_camera2Widget) {
            m_camera2Widget->setImage(img);
        }
    });
    connect(dialog, &CameraTestDialog::cameraPreprocessedReady, this, [this](int camIdx, const QImage &img) {
        if (camIdx == 1 && m_camera1Widget) {
            m_camera1Widget->setImage(img);
        } else if (camIdx == 2 && m_camera2Widget) {
            m_camera2Widget->setImage(img);
        }
    });
    
    dialog->show();
    
    logMessage("打开相机算法测试对话框", "INFO");
}

void Widget::openInterfaceTestDialog()
{
    QDialog *dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(QStringLiteral("接口连通性测试"));
    dlg->resize(540, 520);

    // ===== Helpers =====
    auto sendLocalJson = [this, dlg](const QString &path, const QJsonObject &obj) {
        const QString url = QString("http://127.0.0.1:%1%2").arg(ApiConfig::instance().serverPort()).arg(path);
        QNetworkRequest request{QUrl(url)};
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        auto *mgr = new QNetworkAccessManager(dlg);
        QNetworkReply *reply = mgr->post(request, QJsonDocument(obj).toJson());
        connect(reply, &QNetworkReply::finished, dlg, [this, reply, path]() {
            QByteArray resp = reply->readAll();
            int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            logMessage(QString("接口测试 -> %1 返回 %2: %3").arg(path).arg(code).arg(QString::fromUtf8(resp)), code == 200 ? "SUCCESS" : "ERROR");
            qDebug() << "[API TEST]" << path << "code" << code << "body" << resp;
            reply->deleteLater();
        });
    };

    auto sendWmsJson = [this, dlg](const QString &url, const QByteArray &body, const QString &title) {
        QNetworkRequest request{QUrl(url)};
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        auto *mgr = new QNetworkAccessManager(dlg);
        QNetworkReply *reply = mgr->post(request, body);
        connect(reply, &QNetworkReply::finished, dlg, [this, reply, title, body]() {
            QByteArray resp = reply->readAll();
            int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            logMessage(QString("%1 返回 %2").arg(title).arg(code), code == 200 ? "SUCCESS" : "ERROR");
            logMessage(QString("请求体: %1").arg(QString::fromUtf8(body)), "INFO");
            logMessage(QString("响应体: %1").arg(QString::fromUtf8(resp)), "INFO");
            qDebug() << "[API TEST]" << title << "code" << code << "resp" << resp;
            reply->deleteLater();
        });
    };

    auto *tabs = new QTabWidget(dlg);

    // Tab 1: /api/scan (仅监听显示，显示收到的请求体)
    QWidget *tabScan = new QWidget(tabs);
    QVBoxLayout *layoutScan = new QVBoxLayout(tabScan);
    QTextEdit *scanLog = new QTextEdit(tabScan);
    scanLog->setReadOnly(true);
    scanLog->setPlaceholderText(QStringLiteral("显示收到的 /api/scan 请求体"));
    layoutScan->addWidget(scanLog);
    layoutScan->addStretch(1);
    tabs->addTab(tabScan, QStringLiteral("/api/scan"));

    // Tab 2: /api/position-ready (仅监听显示)
    QWidget *tabPosReady = new QWidget(tabs);
    QVBoxLayout *layoutReady = new QVBoxLayout(tabPosReady);
    QTextEdit *readyLog = new QTextEdit(tabPosReady);
    readyLog->setReadOnly(true);
    readyLog->setPlaceholderText(QStringLiteral("显示收到的 /api/position-ready 请求体"));
    layoutReady->addWidget(readyLog);
    layoutReady->addStretch(1);
    tabs->addTab(tabPosReady, QStringLiteral("/api/position-ready"));

    // Tab 3: RequstMoveForActualLocation (WMS 发送)
    QWidget *tabMove = new QWidget(tabs);
    {
        QVBoxLayout *layout = new QVBoxLayout(tabMove);
    QGroupBox *box = new QGroupBox(QStringLiteral("请求精定位移动：RequstMoveForActualLocation"));
        QFormLayout *form = new QFormLayout(box);
        QLineEdit *moveX = new QLineEdit("16487", box);
        QLineEdit *moveY = new QLineEdit("1796", box);
        QLineEdit *moveZ = new QLineEdit("4216", box);
        QLineEdit *moveDeg = new QLineEdit("0", box);
        QPushButton *sendBtn = new QPushButton(QStringLiteral("发送精定位移动"), box);
        form->addRow("x", moveX);
        form->addRow("y", moveY);
        form->addRow("z", moveZ);
        form->addRow("angle", moveDeg);
        form->addRow(sendBtn);
        layout->addWidget(box);
        layout->addStretch(1);
        connect(sendBtn, &QPushButton::clicked, dlg, [=]() {
            bool okX, okY, okZ, okD;
            double x = moveX->text().toDouble(&okX);
            double y = moveY->text().toDouble(&okY);
            double z = moveZ->text().toDouble(&okZ);
            double d = moveDeg->text().toDouble(&okD);
            if (!(okX && okY && okZ && okD)) {
                logMessage("输入无效：x/y/z/angle 需为数字", "ERROR");
                return;
            }
            QJsonObject obj;
            obj["x"] = x;
            obj["y"] = y;
            obj["z"] = z;
            obj["angle"] = d;
            const QString url = ApiConfig::instance().wmsAdjustPositionUrl();
            logMessage(QString("接口测试 -> RequstMoveForActualLocation, 坐标=(%1,%2,%3,%4)").arg(x).arg(y).arg(z).arg(d), "INFO");
            sendWmsJson(url, QJsonDocument(obj).toJson(), "RequstMoveForActualLocation");
        });
    }
    tabs->addTab(tabMove, QStringLiteral("RequstMove"));

    // Tab 4: ReceiveUnloadPositionData (WMS 发送)
    QWidget *tabReport = new QWidget(tabs);
    QVBoxLayout *layoutReport = new QVBoxLayout(tabReport);
    {
        QGroupBox *box = new QGroupBox(QStringLiteral("批量上报抓取位：ReceiveUnloadPositionData"));
        QVBoxLayout *reportLayout = new QVBoxLayout(box);
        QLabel *hint = new QLabel(QStringLiteral("输入数组JSON，每项含 x/y/z/angle。例如：\n"
                                                 "[\n  {\"x\":16487,\"y\":1796,\"z\":4216,\"angle\":1},\n"
                                                 "  {\"x\":16500,\"y\":1800,\"z\":4220,\"angle\":1}\n]"));
        hint->setWordWrap(true);
        QPlainTextEdit *edit = new QPlainTextEdit(box);
        edit->setPlainText("[\n  {\"x\":16487,\"y\":1796,\"z\":4216,\"angle\":1}\n]");
        QPushButton *sendBtn = new QPushButton(QStringLiteral("发送抓取位上报"), box);
        reportLayout->addWidget(hint);
        reportLayout->addWidget(edit);
        reportLayout->addWidget(sendBtn);
        layoutReport->addWidget(box);
        layoutReport->addStretch(1);
        connect(sendBtn, &QPushButton::clicked, dlg, [=]() {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(edit->toPlainText().toUtf8(), &err);
            if (err.error != QJsonParseError::NoError || !doc.isArray()) {
                logMessage("输入无效：需为数组JSON", "ERROR");
                return;
            }
            QJsonArray arr = doc.array();
            for (const auto &v : arr) {
                if (!v.isObject()) {
                    logMessage("输入无效：数组元素必须是对象", "ERROR");
                    return;
                }
                QJsonObject o = v.toObject();
                if (!o.contains("x") || !o.contains("y") || !o.contains("z") || !o.contains("angle")) {
                    logMessage("输入无效：每个元素需含 x/y/z/angle", "ERROR");
                    return;
                }
            }
            const QString url = ApiConfig::instance().wmsReportResultUrl();
            logMessage(QString("接口测试 -> ReceiveUnloadPositionData, 条目数=%1").arg(arr.size()), "INFO");
            sendWmsJson(url, QJsonDocument(arr).toJson(), "ReceiveUnloadPositionData");
        });
    }
    tabs->addTab(tabReport, QStringLiteral("ReceiveUnload"));

    QVBoxLayout *mainLayout = new QVBoxLayout(dlg);
    mainLayout->addWidget(tabs);
    dlg->setLayout(mainLayout);
    dlg->show();
}

void Widget::onAlgorithmProcessingFinished(const s_PreADPlateARtsPara &result,
                                           const s_Lidar3d &processedImage,
                                           bool preAlignValid,
                                           const s_CalcPreAlignRtsPara &preAlignResult,
                                           const QString &preAlignError)
{
    logMessage("算法检测完成", "SUCCESS");
    m_algorithmStatus = AlgorithmStatus::Ready;
    qDebug() << "[Algorithm] Processing finished successfully";
    
    // 显示处理后的图像（包含缺陷区域）
    displayAlgorithmResult(result, processedImage);

    if (!preAlignValid) {
        logMessage(QString(" 雷达定位计算失败：%1").arg(preAlignError), "ERROR");
    } else {
        // 打印全部拍照位（抓取位）坐标
        if (!preAlignResult.fCamaraPosCrane.empty()) {
            int count = static_cast<int>(preAlignResult.fCamaraPosCrane.size());
            logMessage(QString("雷达定位计算成功，共生成拍照位/抓取位 %1 个").arg(count), "INFO");
            for (int i = 0; i < count; ++i) {
                const auto &p = preAlignResult.fCamaraPosCrane[i];
                logMessage(QString("拍照位#%1: X=%2  Y=%3  Z=%4  Deg=%5")
                               .arg(i + 1)
                               .arg(p.fX, 0, 'f', 1)
                               .arg(p.fY, 0, 'f', 1)
                               .arg(p.fZ, 0, 'f', 1)
                               .arg(p.fDeg, 0, 'f', 2),
                           "INFO");
            }
        } else {
            logMessage("雷达定位结果为空：未生成拍照位/抓取位", "WARNING");
        }
    }
    
    // 保存结果到MD文档（已关闭）
    // saveAlgorithmResultToMarkdown(result, preAlignValid, preAlignResult, preAlignError);

    // 构建并上报WMS结果
    // 注意：新流程改为批量精定位，这里不再立即上报，而是启动批量精定位
    if (preAlignValid && !preAlignResult.fCamaraPosCrane.empty()) {
        // 构建拍照位坐标列表
        QList<ApiTypes::Point3D> photoPoses;
        auto roundToDecimals = [](double value, int decimals) -> double {
            const double factor = std::pow(10.0, static_cast<double>(decimals));
            return std::round(value * factor) / factor;
        };
        
        for (size_t i = 0; i < preAlignResult.fCamaraPosCrane.size(); ++i) {
            const auto &pose = preAlignResult.fCamaraPosCrane[i];
            ApiTypes::Point3D point;
            point.x = roundToDecimals(pose.fX, 1);
            point.y = roundToDecimals(pose.fY, 1);
            point.z = roundToDecimals(pose.fZ, 1);
            point.flag = true;
            point.hasDeg = true;
            point.deg = roundToDecimals(pose.fDeg, 2);
            point.pointType = QStringLiteral("photoPose");
            point.blockIndex = static_cast<int>(i);
            photoPoses.append(point);
        }
        
        // 模式检查：测试模式不启动批量精定位
        if (isTestMode()) {
            logMessage(QString(" [测试模式] 跳过批量精定位: 拍照位数量=%1 (测试模式不启动批量精定位)")
                          .arg(photoPoses.size()),
                      "INFO");
            // 测试模式下，只保存结果，不上报WMS
            return;
        }
        
        // 初始化并启动批量精定位（仅正式模式）
        if (m_batchFinePositioningManager) {
            m_batchFinePositioningManager->initialize(photoPoses);
            m_batchFinePositioningManager->startBatchPositioning();
            logMessage(QString(" [正式模式] 批量精定位已启动，拍照位数量: %1").arg(photoPoses.size()), "INFO");
        } else {
            logMessage("批量精定位管理器未初始化，使用旧流程上报", "WARNING");
            // 回退到旧流程
            ApiTypes::ScanResult scanResult = buildRadarScanResult(preAlignValid, preAlignResult, preAlignError);
            reportScanResultToWms(scanResult);
        }
    } else {
        // 没有拍照位，直接上报失败结果
        ApiTypes::ScanResult scanResult = buildRadarScanResult(preAlignValid, preAlignResult, preAlignError);
        reportScanResultToWms(scanResult);
    }
}

void Widget::onAlgorithmProcessingFailed(const QString &error)
{
    logMessage(QString("算法检测失败: %1").arg(error), "ERROR");
    m_algorithmStatus = AlgorithmStatus::Error;
    qDebug() << "[Algorithm] Processing failed with error:" << error;

    if (isProductionMode()) {
        reportFailureToWms(QString("雷达算法检测失败: %1").arg(error));
    }
}

void Widget::onAlgorithmProcessingProgress(int progress, const QString &message)
{
    logMessage(QString("[%1%] %2").arg(progress).arg(message), "INFO");
}

void Widget::applyAlgorithmParameterSettings()
{
    if (!m_algorithmWorker) {
        return;
    }

    QMetaObject::invokeMethod(m_algorithmWorker,
                              "setSafeDistanceParameters",
                              Qt::QueuedConnection,
                              Q_ARG(double, m_safeDistanceTbNear),
                              Q_ARG(double, m_safeDistanceToCbOuter));
}

// 确保相机拍照位控件存在；如果UI未生成，运行时动态创建并插入左侧面板顶部
void Widget::ensureCameraPositionControls()
{
    // 尝试从UI查找
    m_cameraXSpin = findChild<QDoubleSpinBox*>("cameraXSpin");
    m_cameraYSpin = findChild<QDoubleSpinBox*>("cameraYSpin");
    m_cameraZSpin = findChild<QDoubleSpinBox*>("cameraZSpin");
    m_cameraDegSpin = findChild<QDoubleSpinBox*>("cameraDegSpin");

    if (m_cameraXSpin && m_cameraYSpin && m_cameraZSpin && m_cameraDegSpin) {
        // UI 已生成，直接初始化
        m_cameraXSpin->setVisible(true);
        m_cameraYSpin->setVisible(true);
        m_cameraZSpin->setVisible(true);
        m_cameraDegSpin->setVisible(true);

        m_cameraXSpin->setValue(0.0);
        m_cameraYSpin->setValue(0.0);
        m_cameraZSpin->setValue(0.0);
        m_cameraDegSpin->setValue(0.0);

        // 正式模式下设为只读，测试模式下可编辑
        bool isProd = isProductionMode();
        m_cameraXSpin->setReadOnly(isProd);
        m_cameraYSpin->setReadOnly(isProd);
        m_cameraZSpin->setReadOnly(isProd);
        m_cameraDegSpin->setReadOnly(isProd);

        logMessage(QString("相机拍照位控件已加载（正式模式只读）: X=%1 Y=%2 Z=%3 Deg=%4")
                       .arg(m_cameraXSpin->value())
                       .arg(m_cameraYSpin->value())
                       .arg(m_cameraZSpin->value())
                       .arg(m_cameraDegSpin->value()),
                   "INFO");
        return;
    }

    // 若未找到，运行时动态创建一个 GroupBox + 4 个 SpinBox，插入左侧面板顶端
    // [已注释] 不再动态创建控件，默认值应来自 UI 文件（cameraXSpin/Y/Z、cameraDegSpin）
    logMessage("UI未生成相机拍照位控件，跳过动态创建", "WARNING");
}

void Widget::onCameraPositionParameterChanged()
{
    // 当相机拍照位参数改变时，更新算法线程中的参数
    if (!m_algorithmWorker || !m_cameraXSpin || !m_cameraYSpin || !m_cameraZSpin || !m_cameraDegSpin) {
        logMessage("相机拍照位控件未初始化，无法更新算法参数", "ERROR");
        return;
    }
    
    double cameraX = m_cameraXSpin->value();
    double cameraY = m_cameraYSpin->value();
    double cameraZ = m_cameraZSpin->value();
    double cameraDeg = m_cameraDegSpin->value();
    
    // 更新算法线程中的参数
    QMetaObject::invokeMethod(m_algorithmWorker,
                              "setCameraPositionParameters",
                              Qt::QueuedConnection,
                              Q_ARG(double, cameraX),
                              Q_ARG(double, cameraY),
                              Q_ARG(double, cameraZ),
                              Q_ARG(double, cameraDeg));
    
    logMessage(QString("相机拍照位参数已更新: X=%1, Y=%2, Z=%3, Deg=%4")
               .arg(cameraX, 0, 'f', 1)
               .arg(cameraY, 0, 'f', 1)
               .arg(cameraZ, 0, 'f', 1)
               .arg(cameraDeg, 0, 'f', 3),
               "INFO");
}

ApiTypes::ScanResult Widget::buildRadarScanResult(bool preAlignValid,
                                                  const s_CalcPreAlignRtsPara &preAlignResult,
                                                  const QString &preAlignError) const
{
    QString uniqueCode = m_lastTaskId;
    if (uniqueCode.isEmpty()) {
        uniqueCode = QString("LOCAL_%1").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    }

    if (!preAlignValid || preAlignResult.fCamaraPosCrane.empty()) {
        QString message = preAlignError.isEmpty() ? "未计算出相机拍照位" : preAlignError;
        return ApiTypes::ScanResult::failure(uniqueCode, "1", message);
    }

    ApiTypes::ScanResult scanResult = ApiTypes::ScanResult::success(uniqueCode, "1", "相机拍照位已生成");
    auto roundToDecimals = [](double value, int decimals) -> double {
        const double factor = std::pow(10.0, static_cast<double>(decimals));
        return std::round(value * factor) / factor;
    };

    for (size_t i = 0; i < preAlignResult.fCamaraPosCrane.size(); ++i) {
        const auto &pose = preAlignResult.fCamaraPosCrane[i];
        ApiTypes::Point3D point;
        point.x = roundToDecimals(pose.fX, 1);
        point.y = roundToDecimals(pose.fY, 1);
        point.z = roundToDecimals(pose.fZ, 1);
        point.flag = true;
        point.hasDeg = true;
        point.deg = roundToDecimals(pose.fDeg, 2);
        point.pointType = QStringLiteral("photoPose");
        point.blockIndex = static_cast<int>(i);
        scanResult.data.append(point);
    }

    return scanResult;
}

void Widget::reportScanResultToWms(const ApiTypes::ScanResult &scanResult)
{
    // 模式检查：测试模式不发送WMS上报
    if (isTestMode()) {
        logMessage(QString(" [测试模式] 跳过WMS上报: 任务=%1, 点数=%2 (测试模式不发送WMS)")
                       .arg(scanResult.uniqueCode)
                       .arg(scanResult.data.size()),
                   "INFO");
        return;
    }
    
    if (!m_wmsClient) {
        logMessage(" WMS客户端未初始化，检测结果未上报", "WARNING");
        return;
    }

    logMessage(QString(" [正式模式] 正在向WMS上报结果: 任务=%1, 点数=%2")
                   .arg(scanResult.uniqueCode)
                   .arg(scanResult.data.size()),
               "INFO");

    // 详细输出上报坐标
    int idx = 0;
    for (const auto &p : scanResult.data) {
        ++idx;
        QString line = QString("   #%1: x=%2, y=%3, z=%4%5")
                           .arg(idx)
                           .arg(p.x, 0, 'f', 1)
                           .arg(p.y, 0, 'f', 1)
                           .arg(p.z, 0, 'f', 1)
                           .arg(p.hasDeg ? QString(", deg=%1").arg(p.deg, 0, 'f', 2) : QString());
        logMessage(line, "INFO");
        qDebug() << "[WMS] reportResult payload"
                 << "idx" << idx << "x" << p.x << "y" << p.y << "z" << p.z << "deg" << p.deg;
    }

    m_wmsClient->reportResult(scanResult);
}

void Widget::reportFailureToWms(const QString &reason)
{
    logMessage(QString("检测失败，上报WMS (全-1): %1").arg(reason), "WARNING");

    ApiTypes::ScanResult scanResult;
    scanResult.uniqueCode = m_lastTaskId.isEmpty()
        ? QString("FAIL_%1").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"))
        : m_lastTaskId;
    scanResult.type = "1";
    scanResult.code = 0;
    scanResult.message = reason;

    ApiTypes::Point3D failPoint;
    failPoint.x = -1;
    failPoint.y = -1;
    failPoint.z = -1;
    failPoint.deg = -1;
    failPoint.hasDeg = true;
    failPoint.flag = true;
    failPoint.pointType = QStringLiteral("grabPose");
    failPoint.blockIndex = -1;
    scanResult.data.append(failPoint);

    reportScanResultToWms(scanResult);
}

QString Widget::resolveProjectRootPath() const
{
    if (!m_cachedProjectRootPath.isEmpty()) {
        return m_cachedProjectRootPath;
    }

    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 8; ++i) {
        if (dir.exists("Anode.pro") || dir.exists("data")) {
            m_cachedProjectRootPath = dir.absolutePath();
            return m_cachedProjectRootPath;
        }
        if (!dir.cdUp()) {
            break;
        }
    }

    m_cachedProjectRootPath = QCoreApplication::applicationDirPath();
    return m_cachedProjectRootPath;
}

QString Widget::ensureDailyDataDir(const QString &category) const
{
    const QString rootPath = resolveProjectRootPath();
    QDir rootDir(rootPath);

    if (!rootDir.exists("data") && !rootDir.mkdir("data")) {
        return rootPath;
    }
    if (!rootDir.cd("data")) {
        return rootPath;
    }

    if (!rootDir.exists(category) && !rootDir.mkdir(category)) {
        return rootDir.absolutePath();
    }
    if (!rootDir.cd(category)) {
        return rootDir.absolutePath();
    }

    const QString dayFolder = QDate::currentDate().toString("yyyyMMdd");
    if (!rootDir.exists(dayFolder) && !rootDir.mkdir(dayFolder)) {
        return rootDir.absolutePath();
    }
    rootDir.cd(dayFolder);

    return rootDir.absolutePath();
}

QString Widget::sanitizeForFilename(const QString &value) const
{
    QString sanitized = value;
    if (sanitized.isEmpty()) {
        sanitized = "unknown";
    }
    sanitized.replace(QRegularExpression("[\\\\/:*?\"<>|\\s]+"), "_");
    sanitized = sanitized.trimmed();
    if (sanitized.isEmpty()) {
        sanitized = "unknown";
    }
    return sanitized.left(80);
}

QString Widget::currentTaskIdForStorage() const
{
    if (!m_lastTaskId.isEmpty()) {
        return sanitizeForFilename(m_lastTaskId);
    }
    return QString("LOCAL_%1").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
}

QString Widget::saveRadarPointCloudSnapshot(const PointCloud &pointCloud, const QString &stage)
{
    const QString dirPath = ensureDailyDataDir("radar");
    const QString timestamp = QDateTime::currentDateTime().toString("HHmmsszzz");
    const QString stageTag = sanitizeForFilename(stage.isEmpty() ? "scan" : stage);

    QDir dir(dirPath);
    const QString fileName = QString("%1_%2_%3.ply")
                                 .arg(currentTaskIdForStorage(),
                                      stageTag,
                                      timestamp);
    const QString filePath = dir.filePath(fileName);

    if (savePointCloudAsPly(pointCloud, filePath)) {
        logMessage(QString("雷达点云已保存: %1").arg(QDir::toNativeSeparators(filePath)), "SUCCESS");
        return filePath;
    }

    logMessage(QString("保存雷达点云失败: %1").arg(QDir::toNativeSeparators(filePath)), "ERROR");
    return QString();
}

Widget::CameraCaptureArtifacts Widget::saveImage3dViaWriteImage3DX(s_Image3dS img,
                                                                   const QString &cameraId,
                                                                   int blockIndex,
                                                                   const QString &stage)
{
    CameraCaptureArtifacts artifacts;

    // 基本路径与命名
    const QString dirPath = ensureDailyDataDir("camera");
    const QString timestamp = QDateTime::currentDateTime().toString("HHmmsszzz");
    const QString stageTag = sanitizeForFilename(stage.isEmpty() ? "capture" : stage);
    const QString blockTag = blockIndex >= 0
                                 ? QString("B%1").arg(blockIndex, 2, 10, QChar('0'))
                                 : QString("Bxx");

    QDir dir(dirPath);
    const QString folderName = QString("%1_%2_%3_%4_%5")
                                   .arg(currentTaskIdForStorage(),
                                        stageTag,
                                        sanitizeForFilename(cameraId),
                                        sanitizeForFilename(blockTag),
                                        timestamp);
    const QString folderPath = dir.filePath(folderName);

    if (!dir.mkpath(folderName)) {
        logMessage(QString("创建相机数据目录失败: %1").arg(QDir::toNativeSeparators(folderPath)), "ERROR");
        return artifacts;
    }

    // 确保法线图存在（否则 WriteImage3DX 在写 NX/NY/NZ 时会抛异常提前返回）
    // 注意：相机数据现在已包含通过 SurfaceNormalsObjectModel3d 计算的真实法线
    // 此检查主要用于兼容其他数据源（如离线文件读取时法线可能不存在）
    try {
        using namespace HalconCpp;
        if (!img.NX.IsInitialized() || img.NX.CountObj() == 0 ||
            !img.NY.IsInitialized() || img.NY.CountObj() == 0 ||
            !img.NZ.IsInitialized() || img.NZ.CountObj() == 0) {
            HTuple w, h;
            // 优先用 X 的尺寸，若未初始化则用 Gray
            if (img.X.IsInitialized() && img.X.CountObj() > 0) {
                GetImageSize(img.X, &w, &h);
            } else if (img.Gray.IsInitialized() && img.Gray.CountObj() > 0) {
                GetImageSize(img.Gray, &w, &h);
            }
            if (w.Length() == 0 || h.Length() == 0) {
                logMessage("无法获取图像尺寸，跳过保存3D数据", "ERROR");
                return artifacts;
            }
            // 如果法线不存在，生成空法线图（作为占位符）
           // GenImageConst(&img.NX, "real", w, h);
           // GenImageConst(&img.NY, "real", w, h);
          //  GenImageConst(&img.NZ, "real", w, h)
            logMessage("警告：数据源未提供法线，已生成空法线图占位", "WARNING");
        }
    } catch (const HalconCpp::HException &ex) {
        logMessage(QString("准备法线图失败: %1").arg(QString::fromLocal8Bit(ex.ErrorMessage().Text())), "ERROR");
        return artifacts;
    }

    // 调用算法侧写盘接口（Color 写盘保持关闭）
    CHVisionAdvX converter;
    const int ret = converter.WriteImage3DX(img,
                                            QDir::toNativeSeparators(folderPath).toStdString(),
                                            folderName.toStdString());

    if (ret != 0) {
        logMessage(QString("保存相机3D数据失败(WriteImage3DX返回%1): %2")
                       .arg(ret)
                       .arg(QDir::toNativeSeparators(folderPath)),
                   "ERROR");
        return artifacts;
    }

    artifacts.datasetFolder = folderPath;
    artifacts.datasetBasePaths = collectImage3dBasePaths(folderPath);

    if (artifacts.datasetBasePaths.isEmpty()) {
        logMessage(QString("相机3D数据写盘完成，但未找到纹理基路径: %1").arg(QDir::toNativeSeparators(folderPath)),
                   "WARNING");
    } else {
        logMessage(QString("相机3D数据已保存: %1").arg(QDir::toNativeSeparators(folderPath)),
                   "INFO");
    }

    return artifacts;
}

void Widget::startAlgorithmProcessing(const PointCloud &pointCloud)
{
    if (!m_algorithmWorker) {
        logMessage("算法工作线程未初始化", "ERROR");
        m_algorithmStatus = AlgorithmStatus::Error;
        return;
    }
    
    logMessage("开始算法检测...", "INFO");
    m_algorithmStatus = AlgorithmStatus::Processing;

    // 在触发算法之前，同步最新的安全间距参数
    applyAlgorithmParameterSettings();
    
    // 在算法工作线程中处理点云
    QMetaObject::invokeMethod(m_algorithmWorker, "processPointCloud",
                              Qt::QueuedConnection,
                              Q_ARG(PointCloud, pointCloud));
}

void Widget::displayAlgorithmResult(const s_PreADPlateARtsPara &result, const s_Lidar3d &processedImage)
{
    try {
        using namespace HalconCpp;
        
        // 1. 转换基础Z图像
        QImage baseImage = convertLidar3dToQImage(processedImage);
        if (baseImage.isNull()) {
            qDebug() << "无法转换基础Z图像";
            return;
        }
        
        int width = baseImage.width();
        int height = baseImage.height();
        
        // 2. 创建结果图像（RGB32格式，支持透明叠加）
        QImage resultImage = baseImage.convertToFormat(QImage::Format_RGB32);
        
        // 3. 如果启用显示缺陷，叠加各种区域
        if (m_showDefects) {
            QPainter painter(&resultImage);
            painter.setRenderHint(QPainter::Antialiasing);
            
            // 叠加各种缺陷区域
            // 绿色：铜跺区域
            if (result.RegionTbsPlus.IsInitialized() && result.RegionTbsPlus.CountObj() > 0) {
                QImage regionImg = convertHalconRegionToQImage(result.RegionTbsPlus, width, height, QColor(0, 255, 0));  // 绿色
                painter.drawImage(0, 0, regionImg);
            }
            
            // 白色：栏板缺陷
            if (result.RegionDefectLbRts.IsInitialized() && result.RegionDefectLbRts.CountObj() > 0) {
                QImage regionImg = convertHalconRegionToQImage(result.RegionDefectLbRts, width, height, QColor(255, 255, 255));  // 白色
                painter.drawImage(0, 0, regionImg);
            }
            
            // 红色：空间异物
            if (result.RegionDefectSpaceRts.IsInitialized() && result.RegionDefectSpaceRts.CountObj() > 0) {
                QImage regionImg = convertHalconRegionToQImage(result.RegionDefectSpaceRts, width, height, QColor(255, 0, 0));  // 红色
                painter.drawImage(0, 0, regionImg);
            }
            
            // 黄色：铜跺接触
            if (result.RegionDefectCloseRtsTb.IsInitialized() && result.RegionDefectCloseRtsTb.CountObj() > 0) {
                QImage regionImg = convertHalconRegionToQImage(result.RegionDefectCloseRtsTb, width, height, QColor(255, 255, 0));  // 黄色
                painter.drawImage(0, 0, regionImg);
            }
            
            // 蓝色：铜跺安全间距
            if (result.RegionDefectTbNearRts.IsInitialized() && result.RegionDefectTbNearRts.CountObj() > 0) {
                QImage regionImg = convertHalconRegionToQImage(result.RegionDefectTbNearRts, width, height, QColor(0, 0, 255));  // 蓝色
                painter.drawImage(0, 0, regionImg);
            }
            
            // 青色：车板边缘安全间距
            if (result.RegionDefectOuterRts.IsInitialized() && result.RegionDefectOuterRts.CountObj() > 0) {
                QImage regionImg = convertHalconRegionToQImage(result.RegionDefectOuterRts, width, height, QColor(0, 255, 255));  // 青色
                painter.drawImage(0, 0, regionImg);
            }

            // 绘制铜跺编号文本
            if (!result.TbUpRtsList.empty()) {
                QFont labelFont = painter.font();
                labelFont.setBold(true);
                labelFont.setPointSize(12);
                painter.setFont(labelFont);
                painter.setPen(QPen(QColor(0, 255, 0), 2));

                for (size_t i = 0; i < result.TbUpRtsList.size(); ++i) {
                    int col = static_cast<int>(std::round(result.TbUpRtsList[i].fCenterCol));
                    int row = static_cast<int>(std::round(result.TbUpRtsList[i].fCenterRow));

                    if (col >= 0 && col < width && row >= 0 && row < height) {
                        painter.drawText(col, row, QString::number(static_cast<int>(i + 1)));
                    }
                }
            }
        }
        
        // 4. 旋转图像90度（顺时针）
        QImage rotatedImage = resultImage.transformed(QTransform().rotate(90));
        
        // 5. 显示到热力图控件
        if (m_algorithmResultWidget) {
            m_algorithmResultWidget->setHeatmap(rotatedImage);
            logMessage(QString("算法结果图像已显示 (原始尺寸: %1x%2, 旋转后: %3x%4)")
                      .arg(width).arg(height).arg(rotatedImage.width()).arg(rotatedImage.height()), "INFO");
        } else {
            qDebug() << "算法结果显示控件未初始化";
            logMessage("算法结果显示控件未初始化", "WARNING");
        }
        
    } catch (const std::exception &e) {
        qWarning() << "显示算法结果失败:" << e.what();
    }
}


QImage Widget::convertHalconRegionToQImage(const HalconCpp::HObject &region, int width, int height, const QColor &color)
{
    try {
        using namespace HalconCpp;
        
        // 创建透明图像
        QImage regionImage(width, height, QImage::Format_ARGB32);
        regionImage.fill(Qt::transparent);
        
        if (!region.IsInitialized() || region.CountObj() == 0) {
            return regionImage;
        }
        
        QPainter painter(&regionImage);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // 只绘制边框，不填充（保持原图颜色）
        painter.setBrush(Qt::NoBrush);  // 不填充
        painter.setPen(QPen(color, 3));  // 边框颜色，线宽3像素
        
        // 遍历所有区域对象
        HTuple numObj = region.CountObj();
        for (int i = 1; i <= numObj[0].I(); i++) {
            HObject obj;
            SelectObj(region, &obj, i);
            
            // 获取区域的轮廓点（使用轮廓而不是所有点，更高效）
            HTuple rows, cols;
            GetRegionContour(obj, &rows, &cols);
            
            if (rows.Length() > 0 && cols.Length() > 0) {
                // 创建多边形路径
                QPolygon polygon;
                for (int j = 0; j < rows.Length(); j++) {
                    int row = rows[j].I();
                    int col = cols[j].I();
                    if (row >= 0 && row < height && col >= 0 && col < width) {
                        polygon << QPoint(col, row);
                    }
                }
                
                // 只绘制边框
                if (polygon.size() > 2) {
                    painter.drawPolygon(polygon);
                }
            }
        }
        
        return regionImage;
        
    } catch (const std::exception &e) {
        qWarning() << "转换Halcon区域失败:" << e.what();
        return QImage(width, height, QImage::Format_ARGB32);
    }
}

QImage Widget::convertLidar3dToQImage(const s_Lidar3d &lidar3d)
{
    try {
        using namespace HalconCpp;

        if (!lidar3d.X.IsInitialized() || lidar3d.X.CountObj() == 0 ||
            !lidar3d.Y.IsInitialized() || lidar3d.Y.CountObj() == 0 ||
            !lidar3d.Z.IsInitialized() || lidar3d.Z.CountObj() == 0) {
            return QImage();
        }

        HTuple width, height;
        GetImageSize(lidar3d.Z, &width, &height);
        int w = width[0].I();
        int h = height[0].I();

        if (w <= 0 || h <= 0) {
            return QImage();
        }

        // 计算 X/Y/Z 的数值范围，用于归一化
        HTuple minValX, maxValX, rangeX;
        HTuple minValY, maxValY, rangeY;
        HTuple minValZ, maxValZ, rangeZ;
        MinMaxGray(lidar3d.X, lidar3d.X, 0, &minValX, &maxValX, &rangeX);
        MinMaxGray(lidar3d.Y, lidar3d.Y, 0, &minValY, &maxValY, &rangeY);
        MinMaxGray(lidar3d.Z, lidar3d.Z, 0, &minValZ, &maxValZ, &rangeZ);

        double minX = minValX[0].D();
        double maxX = maxValX[0].D();
        double minY = minValY[0].D();
        double maxY = maxValY[0].D();
        double minZ = minValZ[0].D();
        double maxZ = maxValZ[0].D();

        auto normalizeChannel = [](double value, double minValue, double maxValue) -> int {
            if (!std::isfinite(value) || maxValue <= minValue) {
                return 0;
            }
            double ratio = (value - minValue) / (maxValue - minValue);
            if (ratio < 0.0) {
                ratio = 0.0;
            } else if (ratio > 1.0) {
                ratio = 1.0;
            }
            return qBound(0, static_cast<int>(ratio * 255.0), 255);
        };

        auto sampleChannel = [](const HalconCpp::HObject &channel, int row, int col, double &value) -> bool {
            HTuple tupleValue;
            GetGrayval(channel, row, col, &tupleValue);
            if (tupleValue.Length() > 0) {
                value = tupleValue[0].D();
                return std::isfinite(value);
            }
            return false;
        };

        QImage image(w, h, QImage::Format_RGB32);

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                double xValue = 0.0;
                double yValue = 0.0;
                double zValue = 0.0;

                bool hasX = sampleChannel(lidar3d.X, y, x, xValue);
                bool hasY = sampleChannel(lidar3d.Y, y, x, yValue);
                bool hasZ = sampleChannel(lidar3d.Z, y, x, zValue);

                int r = hasX ? normalizeChannel(xValue, minX, maxX) : 0;
                int g = hasY ? normalizeChannel(yValue, minY, maxY) : 0;
                int b = hasZ ? normalizeChannel(zValue, minZ, maxZ) : 0;

                image.setPixel(x, y, qRgb(r, g, b));
            }
        }

        return image;

    } catch (const std::exception &e) {
        qWarning() << "转换图像失败:" << e.what();
        return QImage();
    }
}

void Widget::saveAlgorithmResultToMarkdown(const s_PreADPlateARtsPara &result,
                                           bool preAlignValid,
                                           const s_CalcPreAlignRtsPara &preAlignResult,
                                           const QString &preAlignError)
{
    // 生成文件名（桌面）
    QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString fileName = QString("算法检测结果_%1.md")
                          .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    QString filePath = QDir(desktop).filePath(fileName);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        logMessage(QString("无法创建结果文件: %1").arg(filePath), "ERROR");
        return;
    }
    
    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif
    
    // 写入Markdown内容
    out << "# 阳极板算法检测结果\n\n";
    out << "**检测时间:** " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n\n";
    out << "---\n\n";
    
    // 综合判断
    out << "## 综合判断结果\n\n";
    out << "- **是否合格:** " << (result.bTJG ? "✓ 合格" : "✗ 不合格") << "\n";
    out << "- **处理耗时:** " << QString::number(result.time, 'f', 3) << " 秒\n\n";
    
    // 铜板位置信息
    out << "## 铜板位置信息\n\n";
    if (result.TbUpRtsList.empty()) {
        out << "未检测到铜板\n\n";
    } else {
        out << "检测到 **" << result.TbUpRtsList.size() << "** 个铜板（以下为雷达定位计算后的真实 XYZ 坐标与姿态角）：\n\n";
        for (size_t i = 0; i < result.TbUpRtsList.size(); i++) {
            const auto &tb = result.TbUpRtsList[i];
            out << "### 铜板 #" << (i + 1) << "\n\n";
            out << "- **中心位置:** (" 
                << QString::number(tb.fCenterX, 'f', 2) << ", "
                << QString::number(tb.fCenterY, 'f', 2) << ", "
                << QString::number(tb.fCenterZ, 'f', 2) << ")\n";
            out << "- **角度:** " << QString::number(tb.fCenterDEG, 'f', 2) << "°\n";
            out << "- **图像坐标:** (" 
                << QString::number(tb.fCenterRow, 'f', 2) << ", "
                << QString::number(tb.fCenterCol, 'f', 2) << ")\n";
            out << "- **四个角点坐标:**\n";
            for (int j = 0; j < 4; j++) {
                out << "  - 角点" << (j + 1) << ": ("
                    << QString::number(tb.fCornerX[j], 'f', 2) << ", "
                    << QString::number(tb.fCornerY[j], 'f', 2) << ", "
                    << QString::number(tb.fCornerZ[j], 'f', 2) << ")\n";
            }
            out << "\n";
        }
    }

    out << "## 相机拍照位\n\n";
    if (preAlignValid && !preAlignResult.fCamaraPosCrane.empty()) {
        out << "共计算出 **" << preAlignResult.fCamaraPosCrane.size() << "** 个拍照位：\n\n";
        for (size_t i = 0; i < preAlignResult.fCamaraPosCrane.size(); ++i) {
            const auto &pose = preAlignResult.fCamaraPosCrane[i];
            out << "- 拍照位 #" << (i + 1) << ": ("
                << QString::number(pose.fX, 'f', 2) << ", "
                << QString::number(pose.fY, 'f', 2) << ", "
                << QString::number(pose.fZ, 'f', 2) << "), 角度="
                << QString::number(pose.fDeg, 'f', 2) << "°\n";
        }
        out << "\n";
    } else {
        if (!preAlignError.isEmpty()) {
            out << "未能计算出相机拍照位，原因：" << preAlignError << "\n\n";
        } else {
            out << "未能计算出相机拍照位，请检查雷达定位配置或算法日志。\n\n";
        }
    }
    
    // 缺陷信息
    out << "## 缺陷检测结果\n\n";
    
    out << "### 空间缺陷\n\n";
    out << "- **是否存在:** " << (result.bExistDefectSpace ? "✗ 是" : "✓ 否") << "\n";
    if (result.bExistDefectSpace && !result.fDefectSpaceRtsListXSel.empty()) {
        out << "- **缺陷数量:** " << result.fDefectSpaceRtsListXSel.size() << "\n";
        out << "- **缺陷位置:**\n";
        for (size_t i = 0; i < result.fDefectSpaceRtsListXSel.size(); i++) {
            out << "  - 缺陷" << (i + 1) << ": ("
                << QString::number(result.fDefectSpaceRtsListXSel[i], 'f', 2) << ", "
                << QString::number(result.fDefectSpaceRtsListYSel[i], 'f', 2) << ", "
                << QString::number(result.fDefectSpaceRtsListZSel[i], 'f', 2) << "), "
                << "长度: " << QString::number(result.fDefectSpaceRtsListLSel[i], 'f', 2) << "mm\n";
        }
    }
    out << "\n";
    
    out << "### 围边缺陷\n\n";
    out << "- **是否存在:** " << (result.bExistDefectLb ? "✗ 是" : "✓ 否") << "\n";
    if (result.bExistDefectLb && !result.fDefectLbRtsListLSel.empty()) {
        out << "- **缺陷数量:** " << result.fDefectLbRtsListLSel.size() << "\n";
        out << "- **缺陷信息:**\n";
        for (size_t i = 0; i < result.fDefectLbRtsListLSel.size(); i++) {
            out << "  - 缺陷" << (i + 1) << ": 高度="
                << QString::number(result.fDefectLbRtsListHeightSel[i], 'f', 2) << "mm, "
                << "长度=" << QString::number(result.fDefectLbRtsListLSel[i], 'f', 2) << "mm\n";
        }
    }
    out << "\n";
    
    out << "### 安全距离检测\n\n";
    out << "- **铜板之间接触:** " << (result.bExitTbClose ? "✗ 是" : "✓ 否") << "\n";
    out << "- **铜板安全距离异常:** " << (result.bExitSafeDisTb ? "✗ 是" : "✓ 否") << "\n";
    out << "- **铜板到车板边界安全距离异常:** " << (result.bExitSafeDisToCbOuter ? "✗ 是" : "✓ 否") << "\n\n";
    
    out << "---\n\n";
    out << "*报告生成时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "*\n";
    
    file.close();
    
    logMessage(QString("检测结果已保存: %1").arg(filePath), "SUCCESS");
}

// ========================================
// 批量精定位相关槽函数实现
// ========================================

void Widget::onBatchPositioningRequestAdjustPosition(int blockIndex, double x, double y, double z, double deg)
{
    // 更新UI显示当前拍照位坐标（天车正移动到该位置）
    if (m_cameraXSpin && m_cameraYSpin && m_cameraZSpin && m_cameraDegSpin) {
        m_cameraXSpin->setValue(x);
        m_cameraYSpin->setValue(y);
        m_cameraZSpin->setValue(z);
        m_cameraDegSpin->setValue(deg);
    }

    // 记录最后一次发给WMS的移动坐标（供精定位算法获取实际拍照位使用）
    m_lastAdjustPayload.insert(blockIndex, ApiTypes::Point3D{ x, y, z, true, deg, true, QStringLiteral("movePose"), blockIndex });

    // 模式检查：测试模式不发送WMS请求
    if (isTestMode()) {
        logMessage(QString(" [测试模式] 跳过WMS移动请求: 铜垛索引=%1, 坐标=(%2, %3, %4, %5) (测试模式不发送WMS)")
                      .arg(blockIndex)
                      .arg(x, 0, 'f', 1)
                      .arg(y, 0, 'f', 1)
                      .arg(z, 0, 'f', 1)
                      .arg(deg, 0, 'f', 2),
                  "INFO");
        // 测试模式下，模拟WMS回调，直接触发拍照
        // 延迟500ms模拟WMS移动完成
        QTimer::singleShot(500, this, [this, blockIndex]() {
            if (m_apiServer) {
                // 模拟WMS回调，通知位置就绪
                emit m_apiServer->positionReady(blockIndex);
            }
        });
        return;
    }
    
    if (!m_wmsClient) {
        logMessage("WMS客户端未初始化，无法写入调整坐标", "ERROR");
        return;
    }

    logMessage(QString(" [正式模式] 精定位移动请求 -> WMS (可能为初次/重复微调): 铜垛索引=%1, 坐标=(%2, %3, %4, %5)")
                  .arg(blockIndex)
                  .arg(x, 0, 'f', 1)
                  .arg(y, 0, 'f', 1)
                  .arg(z, 0, 'f', 1)
                  .arg(deg, 0, 'f', 2),
              "INFO");
    qDebug() << "[WMS] adjustPosition send"
             << "block" << blockIndex
             << "x" << x << "y" << y << "z" << z << "deg" << deg;

    m_wmsClient->adjustPosition(blockIndex, x, y, z, deg);
}

void Widget::onBatchPositioningRequestCameraCapture(int blockIndex)
{
    logMessage(QString("请求相机拍照: 铜垛索引=%1").arg(blockIndex), "INFO");

    m_currentFinePositioningBlockIndex = blockIndex;

    bool cam1Ok = m_camera1Controller && m_camera1Controller->isConnected();
    bool cam2Ok = m_camera2Controller && m_camera2Controller->isConnected();

    if (cam1Ok && cam2Ok) {
        m_camera1Controller->triggerFrame();
        m_camera2Controller->triggerFrame();
    } else if (cam1Ok || cam2Ok) {
        logMessage("仅单相机连接，触发已连接的相机", "WARNING");
        if (cam1Ok) m_camera1Controller->triggerFrame();
        if (cam2Ok) m_camera2Controller->triggerFrame();
    } else {
        logMessage("相机未连接，使用模拟结果", "INFO");
        if (m_batchFinePositioningManager) {
            QImage emptyImage;
            m_batchFinePositioningManager->onCameraImageReady(blockIndex, emptyImage);
        }
    }
}

void Widget::onBatchPositioningCompleted(const ApiTypes::ScanResult &scanResult)
{
    logMessage(QString("批量精定位完成，准备上报: 任务=%1, 抓取坐标数量=%2")
                  .arg(scanResult.uniqueCode)
                  .arg(scanResult.data.size()),
               "SUCCESS");

    m_pendingFineBlocks.clear();
    m_pendingFineBlockSet.clear();
    m_cameraCaptureArtifacts.clear();
    m_isFinePositioningBusy = false;
    m_activeFinePositioningBlock = -1;

    // 上报到WMS
    reportScanResultToWms(scanResult);
}

void Widget::onBatchPositioningFailed(const QString &error)
{
    logMessage(QString("批量精定位失败: %1").arg(error), "ERROR");

    m_pendingFineBlocks.clear();
    m_pendingFineBlockSet.clear();
    m_cameraCaptureArtifacts.clear();
    m_isFinePositioningBusy = false;
    m_activeFinePositioningBlock = -1;

    if (isProductionMode()) {
        reportFailureToWms(QString("批量精定位失败: %1").arg(error));
    }
}

void Widget::onBatchPositioningProgressUpdated(int completed, int total, int currentBlockIndex)
{
    logMessage(QString("批量精定位进度: %1/%2, 当前处理: 铜垛索引=%3")
                  .arg(completed)
                  .arg(total)
                  .arg(currentBlockIndex >= 0 ? QString::number(currentBlockIndex) : "无"),
               "INFO");
}

void Widget::onWmsPositionReady(int blockIndex)
{
    logMessage(QString("收到WMS位置就绪通知: 铜垛索引=%1").arg(blockIndex), "INFO");

    if (m_batchFinePositioningManager) {
        m_batchFinePositioningManager->onPositionReady(blockIndex);
    }
}

void Widget::onCameraImageForFinePositioning(int blockIndex, const QImage &image)
{
    if (m_batchFinePositioningManager) {
        m_batchFinePositioningManager->onCameraImageReady(blockIndex, image);
    }
}

void Widget::setFinePositioningTestMode(bool enabled, const QList<ApiTypes::Point3D> &testCoordinates)
{
    if (m_batchFinePositioningManager) {
        m_batchFinePositioningManager->setTestMode(enabled, testCoordinates);
        if (enabled) {
            if (testCoordinates.isEmpty()) {
                logMessage("批量精定位测试模式已启用（将使用拍照位坐标+偏移作为测试坐标）", "INFO");
            } else {
                logMessage(QString("批量精定位测试模式已启用，已设置 %1 个测试坐标").arg(testCoordinates.size()), "INFO");
            }
        } else {
            logMessage("批量精定位测试模式已禁用", "INFO");
        }
    } else {
        logMessage("批量精定位管理器未初始化，无法设置测试模式", "WARNING");
    }
}

void Widget::onFinePositioningFinished(int blockIndex,
                                       const s_AccurateADPlateARtsPara &result,
                                       const s_CalcAccurateAlignRtsPara &alignResult)
{
    m_isFinePositioningBusy = false;
    m_activeFinePositioningBlock = -1;

    const bool canGrab = (alignResult.iStatus == 1);
    const bool needAdjust = (alignResult.iStatus == 5);
    QString errorMessage;

    if (!canGrab && !needAdjust) {
        errorMessage = QStringLiteral("相机精定位失败 (状态=%1)").arg(alignResult.iStatus);
    }

    if (m_batchFinePositioningManager) {
        m_batchFinePositioningManager->onFinePositioningResult(blockIndex,
                                                               canGrab,
                                                               alignResult.fGripX,
                                                               alignResult.fGripY,
                                                               alignResult.fGripZ,
                                                               alignResult.fGripDeg,
                                                               alignResult.fNextCameraX,
                                                               alignResult.fNextCameraY,
                                                               alignResult.fNextCameraZ,
                                                               alignResult.fNextCameraDeg,
                                                               errorMessage);
    }

    if (canGrab || !needAdjust) {
        m_cameraCaptureArtifacts.remove(blockIndex);
    }

    processNextFinePositioningTask();
}

void Widget::onFinePositioningFailed(int blockIndex, const QString &error)
{
    m_isFinePositioningBusy = false;
    m_activeFinePositioningBlock = -1;

    if (m_batchFinePositioningManager) {
        m_batchFinePositioningManager->onFinePositioningResult(blockIndex,
                                                               false,
                                                               0, 0, 0, 0,
                                                               0, 0, 0, 0,
                                                               error.isEmpty() ? QStringLiteral("相机精定位失败") : error);
    }

    m_cameraCaptureArtifacts.remove(blockIndex);
    processNextFinePositioningTask();
}

void Widget::enqueueFinePositioningTask(int blockIndex)
{
    if (m_pendingFineBlockSet.contains(blockIndex)) {
        qDebug() << QString("[Widget] 铜垛索引 %1 已在精定位队列中，跳过").arg(blockIndex);
        return;
    }

    m_pendingFineBlocks.enqueue(blockIndex);
    m_pendingFineBlockSet.insert(blockIndex);
    qDebug() << QString("[Widget] 已加入精定位队列: 铜垛索引=%1, 队列长度=%2").arg(blockIndex).arg(m_pendingFineBlocks.size());

    // 如果当前没有正在处理的精定位任务，立即处理下一个
    if (!m_isFinePositioningBusy) {
        processNextFinePositioningTask();
    }
}

void Widget::processNextFinePositioningTask()
{
    if (m_isFinePositioningBusy) {
        qDebug() << "[Widget] 精定位任务正在处理中，等待完成";
        return;
    }

    if (m_pendingFineBlocks.isEmpty()) {
        qDebug() << "[Widget] 精定位队列为空";
        return;
    }

    int blockIndex = m_pendingFineBlocks.dequeue();
    m_pendingFineBlockSet.remove(blockIndex);
    m_isFinePositioningBusy = true;
    m_activeFinePositioningBlock = blockIndex;

    qDebug() << QString("[Widget] 开始处理精定位任务: 铜垛索引=%1").arg(blockIndex);

    // 获取相机捕获的数据（内存）
    if (!m_cameraCaptureArtifacts.contains(blockIndex)) {
        logMessage(QString("铜垛索引 %1 的相机数据不存在").arg(blockIndex), "ERROR");
        m_isFinePositioningBusy = false;
        m_activeFinePositioningBlock = -1;
        processNextFinePositioningTask(); // 继续处理下一个
        return;
    }

    CameraCaptureArtifacts artifacts = m_cameraCaptureArtifacts[blockIndex];
    QList<s_Image3dS> images = artifacts.images;

    if (images.isEmpty()) {
        logMessage(QString("铜垛索引 %1 的相机数据为空").arg(blockIndex), "ERROR");
        m_isFinePositioningBusy = false;
        m_activeFinePositioningBlock = -1;
        m_cameraCaptureArtifacts.remove(blockIndex);
        processNextFinePositioningTask(); // 继续处理下一个
        return;
    }

    logMessage(QString("开始相机精定位(内存): 铜垛索引=%1, 图像数量=%2").arg(blockIndex).arg(images.size()), "INFO");

    // 获取当前相机位姿
    // 优先使用 m_lastAdjustPayload：它记录的是最后一次发给WMS的实际移动坐标，
    // 二次调位场景下是调位后的坐标而非原始拍照位
    ApiTypes::Point3D currentCameraPose{0, 0, 0, 0};
    if (m_lastAdjustPayload.contains(blockIndex)) {
        currentCameraPose = m_lastAdjustPayload[blockIndex];
    } else if (m_batchFinePositioningManager) {
        currentCameraPose = m_batchFinePositioningManager->getPhotoPose(blockIndex);
    }

    // 更新UI显示当前拍照位坐标
    if (m_cameraXSpin && m_cameraYSpin && m_cameraZSpin && m_cameraDegSpin) {
        m_cameraXSpin->setValue(currentCameraPose.x);
        m_cameraYSpin->setValue(currentCameraPose.y);
        m_cameraZSpin->setValue(currentCameraPose.z);
        m_cameraDegSpin->setValue(currentCameraPose.deg);
    }

    // 先更新算法的拍照位参数，再调用精定位算法
    if (m_algorithmWorker) {
        QMetaObject::invokeMethod(m_algorithmWorker, "setCameraPositionParameters",
                                  Qt::QueuedConnection,
                                  Q_ARG(double, currentCameraPose.x),
                                  Q_ARG(double, currentCameraPose.y),
                                  Q_ARG(double, currentCameraPose.z),
                                  Q_ARG(double, currentCameraPose.deg));

        QMetaObject::invokeMethod(m_algorithmWorker, "processFinePositioningMemory",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, blockIndex),
                                  Q_ARG(QList<s_Image3dS>, images));
    } else {
        logMessage("算法工作线程未初始化，无法处理精定位", "ERROR");
        m_isFinePositioningBusy = false;
        m_activeFinePositioningBlock = -1;
        m_cameraCaptureArtifacts.remove(blockIndex);
        processNextFinePositioningTask();
    }
}

QStringList Widget::collectImage3dBasePaths(const QString &datasetPath) const
{
    QStringList basePaths;

    if (datasetPath.isEmpty()) {
        return basePaths;
    }

    QDir datasetDir(datasetPath);
    if (!datasetDir.exists()) {
        qWarning() << "[Widget] 数据集目录不存在:" << datasetPath;
        return basePaths;
    }

    // 查找所有以 _IMG_Texture_8Bit.png 结尾的文件
    QDirIterator it(datasetPath, QStringList() << "*_IMG_Texture_8Bit.png", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString texturePath = it.next();
        QFileInfo fileInfo(texturePath);
        
        // 提取基础路径（去掉 _IMG_Texture_8Bit.png 后缀）
        QString baseName = fileInfo.completeBaseName(); // 获取不带扩展名的文件名
        QString basePath = fileInfo.absolutePath() + "/" + baseName;
        
        // 移除 _IMG_Texture_8Bit 后缀
        basePath.replace("_IMG_Texture_8Bit", "");
        
        if (!basePaths.contains(basePath)) {
            basePaths.append(basePath);
        }
    }

    qDebug() << QString("[Widget] 从数据集目录收集到 %1 个图像基础路径: %2").arg(basePaths.size()).arg(datasetPath);
    return basePaths;
}

// ==============================
// 相机测试/内存模式：回调（接WMS）
// ==============================
void Widget::onTestDualCameraFinished(const s_AccurateADPlateARtsPara &detectResult,
                                      const s_CalcAccurateAlignRtsPara &calcResult,
                                      const s_CalcAccurateAlignPara &calcPara)
{
    logMessage("相机测试模式完成", "SUCCESS");

    // 模式检查：测试模式不发送WMS
    if (isTestMode()) {
        logMessage(QString(" [测试模式] 跳过WMS上报: 抓取位=(%1, %2, %3, %4) (测试模式不发送WMS)")
                      .arg(calcResult.fGripX, 0, 'f', 1)
                      .arg(calcResult.fGripY, 0, 'f', 1)
                      .arg(calcResult.fGripZ, 0, 'f', 1)
                      .arg(calcResult.fGripDeg, 0, 'f', 2),
                  "INFO");
        return;
    }

    // 正式模式：发送WMS请求
    logMessage(" [正式模式] 准备通过WMS接口请求移动并上报抓取位", "SUCCESS");

    // 1) 移动请求（使用抓取位坐标）
    if (m_wmsClient) {
        logMessage(QString("WMS移动请求(正式模式)：x=%1 y=%2 z=%3 deg=%4")
                       .arg(calcResult.fGripX, 0, 'f', 1)
                       .arg(calcResult.fGripY, 0, 'f', 1)
                       .arg(calcResult.fGripZ, 0, 'f', 1)
                       .arg(calcResult.fGripDeg, 0, 'f', 2),
                   "INFO");
        m_wmsClient->adjustPosition(-1, calcResult.fGripX, calcResult.fGripY, calcResult.fGripZ, calcResult.fGripDeg);
    } else {
        logMessage("WMS客户端未初始化，无法发送移动请求", "ERROR");
    }

    // 2) 上报抓取结果（单点）
    ApiTypes::ScanResult scanResult;
    scanResult.code = 1;
    scanResult.message = "camera_test";
    scanResult.type = "2";  // 约定：2表示相机测试/精定位结果
    scanResult.uniqueCode = m_lastTaskId.isEmpty() ? "LOCAL_CAMERA_TEST" : m_lastTaskId;

    ApiTypes::Point3D p;
    p.x = calcResult.fGripX;
    p.y = calcResult.fGripY;
    p.z = calcResult.fGripZ;
    p.hasDeg = true;
    p.deg = calcResult.fGripDeg;
    p.pointType = QStringLiteral("grab");
    scanResult.data.append(p);

    // 模式检查：测试模式不发送WMS上报
    if (isTestMode()) {
        logMessage(QString(" [测试模式] 跳过WMS上报: 抓取位=(%1, %2, %3, %4) (测试模式不发送WMS)")
                      .arg(calcResult.fGripX, 0, 'f', 1)
                      .arg(calcResult.fGripY, 0, 'f', 1)
                      .arg(calcResult.fGripZ, 0, 'f', 1)
                      .arg(calcResult.fGripDeg, 0, 'f', 2),
                  "INFO");
        return;
    }
    
    if (m_wmsClient) {
        logMessage(" [正式模式] WMS结果上报：1 个抓取位", "INFO");
        reportScanResultToWms(scanResult);
    } else {
        logMessage("WMS客户端未初始化，无法上报抓取结果", "ERROR");
    }
}

void Widget::onTestDualCameraFailed(const QString &error)
{
    logMessage(QString("相机测试模式失败: %1").arg(error), "ERROR");
}

void Widget::onTestDualCameraProgress(const QString &message)
{
    logMessage(message, "INFO");
}

void Widget::onCameraLogInfo(const QString &message)
{
    logMessage(message, "INFO");
}

QImage Widget::convertImage3dSToQImage(const s_Image3dS &img3d)
{
    try {
        using namespace HalconCpp;
        
        if (!img3d.Gray.IsInitialized() || img3d.Gray.CountObj() == 0) {
            qWarning() << "[Widget] convertImage3dSToQImage: Gray图像未初始化";
            return QImage();
        }
        
        HTuple width, height;
        GetImageSize(img3d.Gray, &width, &height);
        int w = width[0].I();
        int h = height[0].I();
        
        if (w <= 0 || h <= 0) {
            qWarning() << "[Widget] convertImage3dSToQImage: 图像尺寸无效" << w << h;
            return QImage();
        }
        
        // 获取图像数据
        HTuple pointer, type, bitsPerPixel;
        GetImagePointer1(img3d.Gray, &pointer, &type, &width, &height);
        
        if (type[0].S() != "byte") {
            qWarning() << "[Widget] convertImage3dSToQImage: 图像类型不是byte，而是" << type[0].S().Text();
            return QImage();
        }
        
        // 创建QImage
        QImage image(w, h, QImage::Format_Grayscale8);
        
        // 从Halcon图像复制数据到QImage
        Hlong ptr = pointer[0].L();
        if (ptr != 0) {
            const uchar *srcData = reinterpret_cast<const uchar*>(ptr);
            for (int y = 0; y < h; ++y) {
                uchar *dstLine = image.scanLine(y);
                const uchar *srcLine = srcData + y * w;
                std::memcpy(dstLine, srcLine, w);
            }
        } else {
            qWarning() << "[Widget] convertImage3dSToQImage: 图像指针为空";
            return QImage();
        }
        
        return image;
        
    } catch (const HalconCpp::HException &ex) {
        qWarning() << "[Widget] convertImage3dSToQImage Halcon异常:" << ex.ErrorMessage().Text();
        return QImage();
    } catch (const std::exception &e) {
        qWarning() << "[Widget] convertImage3dSToQImage 异常:" << e.what();
        return QImage();
    }
}

// ========================================
// 操作模式管理
// ========================================

void Widget::setOperationMode(OperationMode mode)
{
    if (m_operationMode == mode) {
        return;  // 模式未改变，无需处理
    }
    
    OperationMode oldMode = m_operationMode;
    m_operationMode = mode;
    
    QString modeName = (mode == OperationMode::TestMode) ? "测试模式" : "正式模式";
    logMessage(QString("操作模式已切换: %1").arg(modeName), "INFO");
    
    // 更新按钮文本和样式
    if (ui->operationModeBtn) {
        ui->operationModeBtn->setText(modeName);
        if (mode == OperationMode::TestMode) {
            // 测试模式：红色
            ui->operationModeBtn->setStyleSheet(
                "QPushButton {"
                "    background-color: #E74C3C;"
                "    color: white;"
                "    border: none;"
                "    border-radius: 5px;"
                "    font-size: 13px;"
                "    font-weight: bold;"
                "    padding: 10px;"
                "    text-align: center;"
                "}"
                "QPushButton:hover {"
                "    background-color: #C0392B;"
                "}"
                "QPushButton:pressed {"
                "    background-color: #A93226;"
                "}"
            );
        } else {
            // 正式模式：绿色
            ui->operationModeBtn->setStyleSheet(
                "QPushButton {"
                "    background-color: #27AE60;"
                "    color: white;"
                "    border: none;"
                "    border-radius: 5px;"
                "    font-size: 13px;"
                "    font-weight: bold;"
                "    padding: 10px;"
                "    text-align: center;"
                "}"
                "QPushButton:hover {"
                "    background-color: #229954;"
                "}"
                "QPushButton:pressed {"
                "    background-color: #1E8449;"
                "}"
            );
        }
    }
    
    // 触发模式变更处理
    onOperationModeChanged();
}

void Widget::onOperationModeChanged()
{
    QString modeName = isTestMode() ? "测试模式" : "正式模式";
    
    // 更新UI提示
    if (m_statusLabel) {
        QString statusText = QString("当前模式: %1").arg(modeName);
        m_statusLabel->setText(statusText);
    }
    
    // 相机拍照位控件：测试模式可编辑，正式模式只读
    if (m_cameraXSpin && m_cameraYSpin && m_cameraZSpin && m_cameraDegSpin) {
        bool editable = isTestMode();
        m_cameraXSpin->setReadOnly(!editable);
        m_cameraYSpin->setReadOnly(!editable);
        m_cameraZSpin->setReadOnly(!editable);
        m_cameraDegSpin->setReadOnly(!editable);
    }
    
    // 根据模式更新按钮状态
    if (isProductionMode()) {
        // 正式模式：禁用或提示关键UI按钮（可选）
        // 这里可以根据需要禁用某些按钮，或者只是提示
        logMessage("已切换到正式模式：UI手动操作将被自动切换到测试模式", "INFO");
    } else {
        // 测试模式：所有UI按钮可用
        logMessage("已切换到测试模式：所有手动操作可用，不会发送WMS", "INFO");
    }
}
