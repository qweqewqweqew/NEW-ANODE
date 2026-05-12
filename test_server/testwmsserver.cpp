#include "testwmsserver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDateTime>
#include <QBuffer>
#include <QHttpServerResponse>

TestWmsServer::TestWmsServer(QObject *parent)
    : QObject(parent)
    , m_server(new QHttpServer(this))
    , m_port(9090)
    , m_running(false)
{
}

TestWmsServer::~TestWmsServer()
{
    stop();
}

bool TestWmsServer::start(quint16 port)
{
    if (m_running) {
        qWarning() << "TestWmsServer: 服务器已在运行";
        return false;
    }
    
    m_port = port;
    
    // 注册POST /api/result路由
    m_server->route("/api/result", QHttpServerRequest::Method::Post,
                    [this](const QHttpServerRequest &request) {
        return handleResultRequest(request);
    });
    
    // 启动服务器
    quint16 actualPort = m_server->listen(QHostAddress::Any, m_port);
    if (actualPort == 0) {
        qWarning() << "TestWmsServer: 启动失败，端口可能被占用";
        return false;
    }
    
    m_port = actualPort;
    m_running = true;
    
    qDebug() << "\n" << QString("=").repeated(60);
    qDebug() << "✅ WMS测试服务器启动成功";
    qDebug() << QString("=").repeated(60);
    qDebug() << "地址: http://localhost:" << m_port;
    qDebug() << "接收接口: http://localhost:" << m_port << "/api/result";
    qDebug() << QString("=").repeated(60);
    qDebug() << "\n⏳ 等待视觉系统上报数据...\n";
    
    emit serverStarted(m_port);
    return true;
}

void TestWmsServer::stop()
{
    if (!m_running) {
        return;
    }
    
    // QHttpServer没有close()方法，删除对象会自动停止
    // 但由于它是parent的对象，会在析构时自动删除
    m_running = false;
    
    qDebug() << "WMS测试服务器已停止";
    emit serverStopped();
}

QJsonObject TestWmsServer::handleResultRequest(const QHttpServerRequest &request)
{
    // 解析JSON数据
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(request.body(), &error);
    
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "TestWmsServer: JSON解析失败:" << error.errorString();
        QJsonObject errorObj;
        errorObj["success"] = false;
        errorObj["message"] = "无效的JSON格式";
        return errorObj;
    }
    
    QJsonObject data = doc.object();
    
    // 保存请求
    m_requests.append(data);
    
    // 输出到控制台
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    qDebug() << "\n" << QString("=").repeated(60);
    qDebug() << "✅ 收到WMS上报请求 -" << timestamp;
    qDebug() << QString("=").repeated(60);
    qDebug() << "任务编号:" << data.value("uniqueCode").toString();
    qDebug() << "类型:" << data.value("type").toString() 
             << (data.value("type").toString() == "1" ? "(雷达)" : data.value("type").toString() == "2" ? "(相机)" : "");
    qDebug() << "结果码:" << data.value("code").toInt() 
             << (data.value("code").toInt() == 1 ? "(成功)" : "(失败)");
    qDebug() << "消息:" << data.value("message").toString();
    
    QJsonArray dataArray = data.value("data").toArray();
    qDebug() << "坐标点数:" << dataArray.size();
    
    if (dataArray.size() > 0) {
        qDebug() << "前3个坐标点:";
        int maxCount = (dataArray.size() < 3) ? dataArray.size() : 3;
        for (int i = 0; i < maxCount; i++) {
            QJsonObject point = dataArray[i].toObject();
            qDebug() << QString("  [%1] X=%2, Y=%3, Z=%4, Flag=%5")
                        .arg(i + 1)
                        .arg(point.value("x").toDouble())
                        .arg(point.value("y").toDouble())
                        .arg(point.value("z").toDouble())
                        .arg(point.value("flag").toBool() ? "true" : "false");
        }
    }
    
    QString base64 = data.value("base64").toString();
    qDebug() << "图片Base64长度:" << base64.length() << "字符";
    qDebug() << "完整请求数据:";
    qDebug() << QJsonDocument(data).toJson(QJsonDocument::Indented);
    qDebug() << QString("=").repeated(60) << "\n";
    
    // 发送成功信号
    emit requestReceived(data);
    
    // 返回成功响应
    QJsonObject response;
    response["success"] = true;
    response["message"] = "结果已接收";
    response["taskId"] = data.value("uniqueCode").toString();
    response["receivedAt"] = timestamp;
    
    return response;
}

