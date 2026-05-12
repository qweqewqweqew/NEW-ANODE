#ifndef TESTWMSSERVER_H
#define TESTWMSSERVER_H

#include <QObject>
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponder>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QList>
#include <QHostAddress>

/**
 * @brief WMS测试服务器
 * 
 * 用于测试视觉系统上报结果功能
 * 接收 POST /api/result 请求并显示接收到的数据
 */
class TestWmsServer : public QObject
{
    Q_OBJECT

public:
    explicit TestWmsServer(QObject *parent = nullptr);
    ~TestWmsServer();
    
    /**
     * @brief 启动服务器
     * @param port 端口号（默认9090）
     * @return 是否启动成功
     */
    bool start(quint16 port = 9090);
    
    /**
     * @brief 停止服务器
     */
    void stop();
    
    /**
     * @brief 获取服务器端口
     */
    quint16 port() const { return m_port; }
    
    /**
     * @brief 是否正在运行
     */
    bool isRunning() const { return m_running; }
    
    /**
     * @brief 获取接收到的请求数量
     */
    int requestCount() const { return m_requests.size(); }

signals:
    /**
     * @brief 收到请求信号
     */
    void requestReceived(const QJsonObject &data);
    
    /**
     * @brief 服务器启动信号
     */
    void serverStarted(quint16 port);
    
    /**
     * @brief 服务器停止信号
     */
    void serverStopped();

private:
    /**
     * @brief 处理POST /api/result请求
     * @param request HTTP请求对象
     * @return JSON响应对象
     */
    QJsonObject handleResultRequest(const QHttpServerRequest &request);

private:
    QHttpServer *m_server;
    quint16 m_port;
    bool m_running;
    QList<QJsonObject> m_requests;  // 存储收到的请求
};

#endif // TESTWMSSERVER_H

