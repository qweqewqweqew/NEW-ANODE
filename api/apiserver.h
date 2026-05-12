#ifndef APISERVER_H
#define APISERVER_H

#include <QObject>
#include <QHttpServer>
#include <QJsonObject>
#include "apitypes.h"

class Widget;  // 前向声明，避免循环依赖

/**
 * @file apiserver.h
 * @brief HTTP API服务器
 * 
 * 功能：
 * 1. 监听8080端口，提供4个RESTful API接口
 * 2. GET  /api/status - 获取系统状态
 * 3. POST /api/config - 设置扫描参数
 * 4. GET  /api/config - 获取扫描参数
 * 5. POST /api/scan   - 接收扫描任务
 * 
 * Phase 1 实现范围：
 * - ✅ /api/status 返回真实设备状态（从Widget查询）
 * - ✅ /api/config 读写配置文件
 * - ⚠️ /api/scan 接收请求但暂不触发设备（返回"功能开发中"）
 */
class ApiServer : public QObject
{
    Q_OBJECT
    
public:
    /**
     * 构造函数
     * @param mainWidget 主窗口指针（用于查询设备状态）
     * @param parent 父对象
     */
    explicit ApiServer(Widget *mainWidget, QObject *parent = nullptr);
    
    /**
     * 析构函数
     */
    ~ApiServer();
    
    /**
     * 启动HTTP服务器
     * @param port 监听端口（默认从配置文件读取，通常为8080）
     * @return 启动成功返回true
     */
    bool start(quint16 port = 0);
    
    /**
     * 停止HTTP服务器
     */
    void stop();
    
    /**
     * 获取服务器运行状态
     * @return 正在监听返回true
     */
    bool isListening() const;
    
    /**
     * 获取服务器监听端口
     * @return 端口号，未启动返回0
     */
    quint16 serverPort() const;
    
signals:
    /**
     * 服务器启动成功信号
     * @param port 监听端口
     */
    void serverStarted(quint16 port);
    
    /**
     * 服务器停止信号
     */
    void serverStopped();
    
    /**
     * 收到API请求信号（用于日志）
     * @param endpoint 接口路径（如 "/api/status"）
     * @param method 请求方法（如 "GET"）
     */
    void requestReceived(const QString &endpoint, const QString &method);

    /**
     * 收到API请求（含原始body，供测试/监听）
     * @param endpoint 接口路径
     * @param method HTTP方法
     * @param body 原始请求体
     */
    void requestReceivedWithBody(const QString &endpoint, const QString &method, const QByteArray &body);
    
private:
    QHttpServer *m_server;     // HTTP服务器实例
    Widget *m_mainWidget;      // 主窗口指针（用于查询设备状态）
    quint16 m_port;            // 当前监听端口
    
    /**
     * 注册所有路由
     */
    void setupRoutes();
    
    // ========================================
    // 4个接口处理函数
    // ========================================
    
    /**
     * 处理 GET /api/status
     * @return 系统状态JSON
     */
    QJsonObject handleGetStatus();
    
    /**
     * 处理 POST /api/config
     * @param request 请求体JSON（包含配置参数）
     * @return 响应JSON
     */
    QJsonObject handleSetConfig(const QJsonObject &request);
    
    /**
     * 处理 GET /api/config
     * @return 当前配置JSON
     */
    QJsonObject handleGetConfig();
    
    /**
     * 处理 POST /api/scan
     * @param request 请求体JSON（包含type和uniqueCode）
     * @return 响应JSON
     * 
     * ⚠️ Phase 1: 只验证参数，不触发真实设备
     */
    QJsonObject handleScan(const QJsonObject &request);
    
    /**
     * 处理 POST /api/upload-radar-file
     * @param filePath 雷达文件路径（本地文件路径）
     * @return 响应JSON
     * 
     * 上传雷达文件并触发算法处理
     */
    QJsonObject handleUploadRadarFile(const QString &filePath);
    
    /**
     * 处理 POST /api/position-ready
     * @param request 请求体JSON（只需要status字段，值为"ready"）
     * @return 响应JSON
     * 
     * WMS回调接口：WMS移动天车完成后通知我们
     * 请求体格式：{"status": "ready"}
     */
    QJsonObject handlePositionReady(const QJsonObject &request);
    
    // ========================================
    // 辅助函数
    // ========================================
    
    /**
     * 创建错误响应
     * @param message 错误消息
     * @param errorCode 错误码（可选）
     * @return 错误响应JSON
     */
    QJsonObject errorResponse(const QString &message, const QString &errorCode = "");
    
    /**
     * 记录请求日志
     * @param method HTTP方法
     * @param path 请求路径
     */
    void logRequest(const QString &method, const QString &path);

signals:
    /**
     * WMS位置就绪信号（转发给批量精定位管理器）
     * @param blockIndex 铜垛索引
     */
    void positionReady(int blockIndex);
};

#endif // APISERVER_H








