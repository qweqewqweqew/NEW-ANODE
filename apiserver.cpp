#include "apiserver.h"
#include "apiconfig.h"
#include "widget.h"
#include <QDebug>
#include <QDateTime>

/**
 * 构造函数
 */
ApiServer::ApiServer(Widget *mainWidget, QObject *parent)
    : QObject(parent)
    , m_server(new QHttpServer(this))
    , m_mainWidget(mainWidget)
    , m_port(0)
{
    // 注册所有路由
    setupRoutes();
}

/**
 * 析构函数
 */
ApiServer::~ApiServer()
{
    stop();
}

/**
 * 启动HTTP服务器
 */
bool ApiServer::start(quint16 port)
{
    // 如果未指定端口，从配置文件读取
    if (port == 0) {
        port = ApiConfig::instance().serverPort();
    }
    
    // 从配置文件读取监听地址
    QString host = ApiConfig::instance().serverHost();
    QHostAddress listenAddress;
    
    // 如果host为空、"0.0.0.0"或"Any"，则监听所有接口
    if (host.isEmpty() || host == "0.0.0.0" || host.toLower() == "any") {
        listenAddress = QHostAddress::Any;
        qDebug() << "[ApiServer] 监听所有网络接口，端口:" << port;
    } else {
        // 尝试解析为IP地址
        listenAddress = QHostAddress(host);
        if (listenAddress.isNull()) {
            qWarning() << "[ApiServer] 无效的监听地址:" << host << "，使用默认值（所有接口）";
            listenAddress = QHostAddress::Any;
        } else {
            qDebug() << "[ApiServer] 监听指定IP地址:" << host << "，端口:" << port;
        }
    }
    
    // 监听指定地址和端口
    // QHttpServer::listen() 返回绑定的端口号，失败返回0
    quint16 boundPort = m_server->listen(listenAddress, port);
    
    if (boundPort > 0) {
        m_port = boundPort;
        qDebug() << "[ApiServer] HTTP服务器启动成功，监听地址:" << (host.isEmpty() || host == "0.0.0.0" ? "所有接口" : host) << "，端口:" << m_port;
        emit serverStarted(m_port);
        return true;
    } else {
        qWarning() << "[ApiServer] HTTP服务器启动失败，监听地址:" << host << "，端口:" << port;
        m_port = 0;
        return false;
    }
}

/**
 * 停止HTTP服务器
 */
void ApiServer::stop()
{
    if (m_port > 0) {
        qDebug() << "[ApiServer] HTTP服务器停止";
        m_port = 0;
        emit serverStopped();
    }
}

/**
 * 获取服务器运行状态
 */
bool ApiServer::isListening() const
{
    return m_port > 0;
}

/**
 * 获取服务器监听端口
 */
quint16 ApiServer::serverPort() const
{
    return m_port;
}

/**
 * 注册所有路由
 */
void ApiServer::setupRoutes()
{
    // ========================================
    // 接口1: GET /api/status - 获取系统状态
    // ========================================
    m_server->route("/api/status", QHttpServerRequest::Method::Get,
        [this]() {
            logRequest("GET", "/api/status");
            return handleGetStatus();
        });
    
    // ========================================
    // 接口2: POST /api/config - 设置扫描参数
    // ========================================
    m_server->route("/api/config", QHttpServerRequest::Method::Post,
        [this](const QHttpServerRequest &request) {
            logRequest("POST", "/api/config");
            
            // 解析JSON请求体
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
            
            if (parseError.error != QJsonParseError::NoError) {
                return errorResponse("JSON解析失败: " + parseError.errorString(), 
                                    ApiTypes::ErrorCodes::INVALID_PARAMETER);
            }
            
            return handleSetConfig(doc.object());
        });
    
    // ========================================
    // 接口3: GET /api/config - 获取扫描参数
    // ========================================
    m_server->route("/api/config", QHttpServerRequest::Method::Get,
        [this]() {
            logRequest("GET", "/api/config");
            return handleGetConfig();
        });
    
    // ========================================
    // 接口4: POST /api/scan - 接收扫描任务
    // ========================================
    m_server->route("/api/scan", QHttpServerRequest::Method::Post,
        [this](const QHttpServerRequest &request) {
            logRequest("POST", "/api/scan");
            
            // 解析JSON请求体
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
            
            if (parseError.error != QJsonParseError::NoError) {
                return errorResponse("JSON解析失败: " + parseError.errorString(), 
                                    ApiTypes::ErrorCodes::INVALID_PARAMETER);
            }
            emit requestReceivedWithBody("/api/scan", "POST", request.body());
            return handleScan(doc.object());
        });
    
    // ========================================
    // 接口5: POST /api/upload-radar-file - 上传雷达文件
    // ========================================
    m_server->route("/api/upload-radar-file", QHttpServerRequest::Method::Post,
        [this](const QHttpServerRequest &request) {
            logRequest("POST", "/api/upload-radar-file");
            
            // 解析JSON格式的文件路径
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
            
            if (parseError.error != QJsonParseError::NoError) {
                return errorResponse("JSON解析失败: " + parseError.errorString(), 
                                    ApiTypes::ErrorCodes::INVALID_PARAMETER);
            }
            
            QJsonObject obj = doc.object();
            QString filePath = obj.value("filePath").toString();
            
            if (filePath.isEmpty()) {
                return errorResponse("filePath字段不能为空", 
                                    ApiTypes::ErrorCodes::INVALID_PARAMETER);
            }
            
            return handleUploadRadarFile(filePath);
        });
    
    // ========================================
    // 接口6: POST /api/position-ready - WMS回调接口
    // ========================================
    m_server->route("/api/position-ready", QHttpServerRequest::Method::Post,
        [this](const QHttpServerRequest &request) {
            logRequest("POST", "/api/position-ready");
            
            // 解析JSON请求体
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
            
            if (parseError.error != QJsonParseError::NoError) {
                return errorResponse("JSON解析失败: " + parseError.errorString(), 
                                    ApiTypes::ErrorCodes::INVALID_PARAMETER);
            }
            emit requestReceivedWithBody("/api/position-ready", "POST", request.body());
            return handlePositionReady(doc.object());
        });
    
    qDebug() << "[ApiServer] 路由注册完成：5个接口";
}

/**
 * 处理 GET /api/status
 */
QJsonObject ApiServer::handleGetStatus()
{
    ApiTypes::SystemStatus status;
    
    // 从主窗口查询真实设备状态
    status.camera1 = m_mainWidget->getCamera1Status();
    status.camera2 = m_mainWidget->getCamera2Status();
    status.radar = m_mainWidget->getRadarStatus();
    status.algorithm = m_mainWidget->getAlgorithmStatus();  // 固定返回ready
    
    // 计算系统总体状态（仅根据设备连接状态）
    int connectedCount = 0;
    if (status.camera1 == "Connected") connectedCount++;
    if (status.camera2 == "Connected") connectedCount++;
    if (status.radar == "Connected") connectedCount++;
    
    if (connectedCount == 3) {
        status.system = "OK";
    } else if (connectedCount > 0) {
        status.system = "Partial";
    } else {
        status.system = "Error";
    }
    
    return status.toJson();
}

/**
 * 处理 POST /api/config
 */
QJsonObject ApiServer::handleSetConfig(const QJsonObject &request)
{
    // 解析配置参数
    ApiTypes::ScanConfig config = ApiTypes::ScanConfig::fromJson(request);
    
    // 验证参数
    QString validationError = config.validate();
    if (!validationError.isEmpty()) {
        return errorResponse(validationError, ApiTypes::ErrorCodes::INVALID_PARAMETER);
    }
    
    // 保存配置
    ApiConfig::instance().setScanConfig(config);
    
    // 返回成功响应
    ApiTypes::ApiResponse response = ApiTypes::ApiResponse::successResponse("参数设置成功");
    return response.toJson();
}

/**
 * 处理 GET /api/config
 */
QJsonObject ApiServer::handleGetConfig()
{
    // 从配置文件读取当前参数
    ApiTypes::ScanConfig config = ApiConfig::instance().scanConfig();
    
    // 返回配置JSON
    return config.toJson();
}

QJsonObject ApiServer::handleScan(const QJsonObject &request)
{
    // 解析请求
    ApiTypes::ScanRequest scanReq = ApiTypes::ScanRequest::fromJson(request);
    
    // 验证请求
    if (!scanReq.isValid()) {
        QString errorMsg = "参数错误：";
        if (scanReq.type != "1" && scanReq.type != "2") {
            errorMsg += "type必须为\"1\"或\"2\"；";
        }
        if (scanReq.uniqueCode.isEmpty()) {
            errorMsg += "uniqueCode不能为空";
        }
        return errorResponse(errorMsg, ApiTypes::ErrorCodes::INVALID_PARAMETER);
    }
    // 记录任务信息到主窗口
    m_mainWidget->setLastTaskId(scanReq.uniqueCode);
    
    // 打印日志
    qDebug() << "[ApiServer] 收到扫描任务:"
             << "type=" << scanReq.type
             << "(" << scanReq.typeDescription() << ")"
             << "uniqueCode=" << scanReq.uniqueCode;

    if (scanReq.type == "1") {
        // 触发一次雷达扫描，由主窗口统一接口处理点云与后续流程
        bool started = m_mainWidget->startRadarScanAndDetect("API");
        if (!started) {
            m_mainWidget->setLastError("无法启动雷达扫描：设备未连接或正在忙");
            return errorResponse("无法启动雷达扫描，请检查雷达连接状态",
                                 ApiTypes::ErrorCodes::DEVICE_NOT_CONNECTED);
        }

        QJsonObject response;
        response["success"] = true;
        response["message"] = QString("任务已接收并开始雷达扫描 (%1)").arg(scanReq.typeDescription());
        response["uniqueCode"] = scanReq.uniqueCode;
        response["phase"] = "scanning";
        return response;
    }
    
    // 相机精定位（type=2）暂未实现
    return errorResponse("相机精定位功能尚未开放", ApiTypes::ErrorCodes::NOT_IMPLEMENTED);
}

/**
 * 创建错误响应
 */
QJsonObject ApiServer::errorResponse(const QString &message, const QString &errorCode)
{
    ApiTypes::ApiResponse response = ApiTypes::ApiResponse::errorResponse(message, errorCode);
    return response.toJson();
}

/**
 * 处理 POST /api/upload-radar-file
 */
QJsonObject ApiServer::handleUploadRadarFile(const QString &filePath)
{
    // 验证文件路径
    if (filePath.isEmpty()) {
        return errorResponse("文件路径不能为空", ApiTypes::ErrorCodes::INVALID_PARAMETER);
    }
    
    // 检查文件是否存在
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return errorResponse(QString("文件不存在: %1").arg(filePath), 
                            ApiTypes::ErrorCodes::INVALID_PARAMETER);
    }
    
    // 检查文件扩展名（仅支持PLY格式）
    if (fileInfo.suffix().toLower() != "ply") {
        return errorResponse("仅支持PLY格式的点云文件", ApiTypes::ErrorCodes::INVALID_PARAMETER);
    }
    
    qDebug() << QString("[ApiServer] 收到雷达文件上传请求: %1").arg(filePath);
    
    // 调用主窗口处理文件上传
    bool success = m_mainWidget->processRadarFileUpload(filePath);
    
    if (success) {
        QJsonObject response;
        response["success"] = true;
        response["message"] = QString("文件已接收，正在处理: %1").arg(filePath);
        response["filePath"] = filePath;
        return response;
    } else {
        return errorResponse("文件处理失败，请检查系统状态", ApiTypes::ErrorCodes::SYSTEM_BUSY);
    }
}

/**
 * 处理 POST /api/position-ready
 */
QJsonObject ApiServer::handlePositionReady(const QJsonObject &request)
{
    // 解析请求参数
    QString status = request.value("status").toString();

    // 验证参数
    if (status != "ready") {
        return errorResponse("status必须为\"ready\"", ApiTypes::ErrorCodes::INVALID_PARAMETER);
    }

    qDebug() << QString("[ApiServer] 收到WMS位置就绪通知: status=%1").arg(status);

    // 发送信号给Widget，由Widget转发给批量精定位管理器
    // 传递-1表示使用当前处理的拍照位索引（由BatchFinePositioningManager内部获取）
    emit positionReady(-1);

    // 返回成功响应
    QJsonObject response;
    response["code"] = 1;
    response["message"] = "已收到通知";
    return response;
}

/**
 * 记录请求日志
 */
void ApiServer::logRequest(const QString &method, const QString &path)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    qDebug() << QString("[ApiServer] %1 %2 %3").arg(timestamp, method, path);
    emit requestReceived(path, method);
}

