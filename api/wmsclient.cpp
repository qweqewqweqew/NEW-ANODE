#include "wmsclient.h"
#include "apiconfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QNetworkProxy>

/**
 * ⚠️ 占位值说明：
 * - WMS URL默认值：http://localhost:9090/api/result (后续需要配置真实地址)
 * - 请求体：当前只发送基础字段，详细数据格式后续迭代完善
 * - 错误处理：基础实现，详细的错误码映射和重试逻辑后续添加
 */

WmsClient::WmsClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_wmsUrl("http://192.168.5.175:8099/api/result")  // 占位值：需要后续配置真实WMS地址
    , m_timeout(10000)  // 默认10秒超时
    , m_timeoutTimer(new QTimer(this))
    , m_currentReply(nullptr)
{
    // 超时定时器设置
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &WmsClient::onTimeout);
    
    // 配置网络管理器：禁用系统代理（避免代理问题）
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    m_networkManager->setProxy(proxy);
    
    // 连接网络管理器的信号，用于调试
    connect(m_networkManager, &QNetworkAccessManager::finished, this, [](QNetworkReply *reply) {
        qDebug() << QString("[WmsClient] 网络请求完成 - URL: %1, 错误: %2")
                    .arg(reply->url().toString())
                    .arg(reply->error() == QNetworkReply::NoError ? "无错误" : reply->errorString());
    });
    
    qDebug() << "[WmsClient] WMS客户端已初始化（已禁用系统代理）";
}

WmsClient::~WmsClient()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

void WmsClient::setWmsUrl(const QString &url)
{
    m_wmsUrl = url;
    qDebug() << QString("WmsClient: WMS地址已更新为: %1").arg(m_wmsUrl);
}

void WmsClient::setTimeout(int timeout)
{
    m_timeout = timeout;
}

void WmsClient::reportResult(const ApiTypes::ScanResult &result)
{
    // 检查是否有正在进行的请求
    if (m_currentReply != nullptr) {
        qWarning() << QString("WmsClient: 已有请求正在进行，忽略新请求: %1")
                      .arg(result.uniqueCode);
        emit reportFailed(result.uniqueCode, "系统繁忙，已有请求在处理");
        return;
    }

    // 保存当前结果
    m_currentResult = result;

    // 从配置获取上报接口URL
    ApiConfig &config = ApiConfig::instance();
    QString reportUrl = config.wmsReportResultUrl();
    
    qDebug() << QString("WmsClient: reportResult - 从配置读取reportResultUrl: %1").arg(reportUrl);
    qDebug() << QString("WmsClient: reportResult - 同时读取adjustPositionUrl: %1").arg(config.wmsAdjustPositionUrl());
    
    if (reportUrl.isEmpty()) {
        // 兼容旧配置：从基础URL推导
        // 优先使用配置文件中的url字段，而不是m_wmsUrl（可能过时）
        QString baseUrl = config.wmsUrl();
        if (baseUrl.isEmpty() || baseUrl == "http://192.168.5.175:8099") {
            baseUrl = m_wmsUrl;  // 回退到WmsClient的成员变量
        }
        QUrl url(baseUrl);
        // 接口7：上报识别结果
        reportUrl = QString("http://%1:8099/api/OpenApi/ReceiveUnloadPositionData")
                     .arg(url.host());
        qDebug() << QString("WmsClient: reportResultUrl为空，从baseUrl推导: %1 -> %2")
                    .arg(baseUrl).arg(reportUrl);
    }

    // 构建请求URL
    QUrl url(reportUrl);
    if (!url.isValid()) {
        qWarning() << QString("WmsClient: 无效的WMS上报地址: %1").arg(reportUrl);
        emit reportFailed(result.uniqueCode, QString("无效的WMS上报地址: %1").arg(reportUrl));
        return;
    }

    // 创建网络请求
    QNetworkRequest request(url);
    // 使用与Postman一致的Content-Type格式
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    
    //  认证信息：后续如果需要Token等认证，在这里添加
    // request.setRawHeader("Authorization", "Bearer " + token.toUtf8());

    // 按照WMS接口格式构建JSON数组
    // 接口7格式：数组，每个元素包含 x, y, z, direction, angle
    QJsonArray dataArray;
    for (const auto &point : result.data) {
        if (point.flag) {  // 只上报有效坐标
            QJsonObject pointObj;
            // WMS要求整数坐标，使用qRound四舍五入确保为整数
            pointObj["x"] = qRound(point.x);
            pointObj["y"] = qRound(point.y);
            pointObj["z"] = qRound(point.z);
            // direction: 方向（1表示有效，0表示无效，或其他值？）
            pointObj["direction"] = 1;
            // angle: 角度（使用deg字段，转换为整数度）
            pointObj["angle"] = qRound(point.deg);
            dataArray.append(pointObj);
        }
    }

    if (dataArray.isEmpty()) {
        qWarning() << QString("WmsClient: 没有有效的坐标数据可上报");
        emit reportFailed(result.uniqueCode, "没有有效的坐标数据");
        return;
    }

    QJsonDocument doc(dataArray);  // 直接使用数组作为根
    // 使用格式化JSON（与Postman一致），使用4个空格缩进
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    qDebug() << QString("WmsClient: 开始上报结果到WMS")
             << QString("  任务编号: %1").arg(result.uniqueCode)
             << QString("  类型: %1").arg(result.type)
             << QString("  坐标点数: %1").arg(dataArray.size())
             << QString("  URL: %1").arg(reportUrl)
             << QString("  请求体JSON: %1").arg(QString::fromUtf8(jsonData));

    // 发送POST请求（异步）
    m_currentReply = m_networkManager->post(request, jsonData);

    // 连接完成信号（使用Qt::QueuedConnection确保线程安全）
    connect(m_currentReply, &QNetworkReply::finished, this, &WmsClient::onRequestFinished, Qt::QueuedConnection);

    // 启动超时定时器
    if (m_timeoutTimer) {
        m_timeoutTimer->start(m_timeout);
    }
}

void WmsClient::onRequestFinished()
{
    if (!m_currentReply) {
        return;
    }

    // 停止超时定时器
    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
    }

    // 保存reply指针和必要信息，避免在清理后访问
    QNetworkReply *reply = m_currentReply;
    m_currentReply = nullptr;  // 立即清空，避免重复处理

    // 获取响应信息（在清理前获取所有需要的数据）
    QNetworkReply::NetworkError error = reply->error();
    QByteArray responseData = reply->readAll();
    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString uniqueCode = m_currentResult.uniqueCode;
    QString errorString = reply->errorString();  // 提前获取错误字符串

    qDebug() << QString("WmsClient: 请求完成")
             << QString("  任务编号: %1").arg(uniqueCode)
             << QString("  HTTP状态码: %1").arg(httpStatusCode)
             << QString("  网络错误码: %1").arg(error)
             << QString("  响应长度: %1 字节").arg(responseData.size());

    if (error == QNetworkReply::NoError) {
        // 解析WMS响应（格式：{"code": 1, "status": "success", "message": ""}）
        bool success = true;
        QString errorMsg;
        if (!responseData.isEmpty()) {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                QJsonObject obj = doc.object();
                int code = obj.value("code").toInt(0);
                QString status = obj.value("status").toString();
                QString message = obj.value("message").toString();
                
                if (code == -1 || status != "success") {
                    success = false;
                    errorMsg = message.isEmpty() ? QString("WMS返回失败，code=%1").arg(code) : message;
                }
                
                qDebug() << QString("WmsClient: WMS响应解析 - code=%1, status=%2, message=%3")
                            .arg(code).arg(status).arg(message);
            } else {
                qWarning() << QString("WmsClient: WMS响应JSON解析失败: %1").arg(parseError.errorString());
            }
        }
        
        if (success) {
            qDebug() << QString("WmsClient: 上报成功 - %1").arg(uniqueCode);
            emit reportSucceeded(uniqueCode);
        } else {
            qWarning() << QString("WmsClient: 上报失败 - %1, 错误: %2").arg(uniqueCode, errorMsg);
            emit reportFailed(uniqueCode, errorMsg);
        }
    } else {
        // 请求失败
        QString errorMsg;
        switch (error) {
        case QNetworkReply::TimeoutError:
            errorMsg = QString("请求超时（%1秒）").arg(m_timeout / 1000);
            break;
        case QNetworkReply::ConnectionRefusedError:
            errorMsg = "WMS服务器拒绝连接，请检查地址和端口";
            break;
        case QNetworkReply::HostNotFoundError:
            errorMsg = "无法找到WMS服务器，请检查网络和地址";
            break;
        default:
            errorMsg = QString("网络错误: %1").arg(errorString);
            break;
        }
        
        qWarning() << QString("WmsClient: 上报失败 - %1, 错误: %2")
                      .arg(uniqueCode, errorMsg);
        
        emit reportFailed(uniqueCode, errorMsg);
    }

    // 清理reply（在emit信号之后）
    reply->deleteLater();
}

void WmsClient::adjustPosition(int blockIndex, double x, double y, double z, double deg)
{
    // 从配置获取精定位移动接口URL
    ApiConfig &config = ApiConfig::instance();
    QString adjustUrlStr = config.wmsAdjustPositionUrl();
    
    qDebug() << QString("WmsClient: adjustPosition - 从配置读取adjustPositionUrl: %1").arg(adjustUrlStr);
    qDebug() << QString("WmsClient: adjustPosition - 同时读取reportResultUrl: %1").arg(config.wmsReportResultUrl());
    
    if (adjustUrlStr.isEmpty()) {
        // 兼容旧配置：从基础URL推导
        // 优先使用配置文件中的url字段，而不是m_wmsUrl（可能过时）
        QString baseUrl = config.wmsUrl();
        if (baseUrl.isEmpty() || baseUrl == "http://localhost:9090") {
            baseUrl = m_wmsUrl;  // 回退到WmsClient的成员变量
        }
        QUrl url(baseUrl);
        // 接口8：请求精定位移动
        adjustUrlStr = QString("http://%1:8099/api/OpenApi/RequstMoveForActualLocation")
                        .arg(url.host());
        qDebug() << QString("WmsClient: adjustPositionUrl为空，从baseUrl推导: %1 -> %2")
                    .arg(baseUrl).arg(adjustUrlStr);
    }

    QUrl adjustUrl(adjustUrlStr);
    if (!adjustUrl.isValid()) {
        qWarning() << QString("WmsClient: 无效的调整坐标URL: %1").arg(adjustUrlStr);
        emit adjustPositionFailed(blockIndex, QString("无效的调整坐标URL: %1").arg(adjustUrlStr));
        return;
    }

    // 按照WMS接口格式构建JSON请求体
    // 接口8：发送位置与角度（x, y, z, angle/deg）
    QJsonObject jsonObj;
    // WMS要求整数坐标，使用qRound四舍五入确保为整数
    jsonObj["x"] = qRound(x);
    jsonObj["y"] = qRound(y);
    jsonObj["z"] = qRound(z);
    jsonObj["angle"] = qRound(deg);  // 角度：与接口文档中的 angle 对齐

    QJsonDocument doc(jsonObj);
    // 使用格式化JSON（与Postman一致），使用4个空格缩进
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    // 创建网络请求
    QNetworkRequest request(adjustUrl);
    // 使用与Postman一致的Content-Type格式
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");

    // 输出详细的请求信息用于调试
    QString jsonStr = QString::fromUtf8(jsonData);
    qDebug() << QString("WmsClient: 开始写入调整坐标到WMS")
             << QString("  铜垛索引: %1").arg(blockIndex)
            << QString("  坐标: (%2, %3, %4)")
                .arg(qRound(x))
                .arg(qRound(y))
                .arg(qRound(z))
            << QString("  角度: %1").arg(qRound(deg))
             << QString("  URL: %1").arg(adjustUrlStr)
             << QString("  请求体JSON: %1").arg(jsonStr)
             << QString("  请求体字节数: %1").arg(jsonData.size())
             << QString("  Content-Type: application/json; charset=utf-8");

    // 发送POST请求（异步）
    // 注意：这里使用新的QNetworkReply，不阻塞reportResult请求
    qDebug() << QString("WmsClient: 准备发送POST请求")
             << QString("  URL: %1").arg(adjustUrl.toString())
             << QString("  Content-Type: %1").arg(request.header(QNetworkRequest::ContentTypeHeader).toString())
             << QString("  请求体大小: %1 字节").arg(jsonData.size());
    
    QNetworkReply *reply = m_networkManager->post(request, jsonData);
    
    if (!reply) {
        qWarning() << QString("WmsClient: 创建网络请求失败 - blockIndex=%1").arg(blockIndex);
        emit adjustPositionFailed(blockIndex, "创建网络请求失败");
        return;
    }
    
    qDebug() << QString("WmsClient: 网络请求已发送，等待响应...");

    // 使用lambda捕获blockIndex，处理响应
    connect(reply, &QNetworkReply::finished, this, [this, blockIndex, reply]() {
        QNetworkReply::NetworkError error = reply->error();
        QByteArray responseData = reply->readAll();
        int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString errorString = reply->errorString();

        qDebug() << QString("WmsClient: 写入坐标请求完成")
                 << QString("  铜垛索引: %1").arg(blockIndex)
                 << QString("  HTTP状态码: %1").arg(httpStatusCode)
                 << QString("  网络错误码: %1").arg(error)
                 << QString("  错误信息: %1").arg(errorString)
                 << QString("  响应数据: %1").arg(QString::fromUtf8(responseData));

        if (error == QNetworkReply::NoError) {
            // 解析WMS响应（格式：{"code": 1, "status": "success", "message": ""}）
            bool success = true;
            QString errorMsg;
            if (!responseData.isEmpty()) {
                QJsonParseError parseError;
                QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
                if (parseError.error == QJsonParseError::NoError) {
                    QJsonObject obj = doc.object();
                    int code = obj.value("code").toInt(0);
                    QString status = obj.value("status").toString();
                    QString message = obj.value("message").toString();
                    
                    if (code == -1 || status != "success") {
                        success = false;
                        errorMsg = message.isEmpty() ? QString("WMS返回失败，code=%1").arg(code) : message;
                    }
                    
                    qDebug() << QString("WmsClient: WMS响应解析 - code=%1, status=%2, message=%3")
                                .arg(code).arg(status).arg(message);
                } else {
                    qWarning() << QString("WmsClient: WMS响应JSON解析失败: %1").arg(parseError.errorString());
                }
            }
            
            if (success) {
                qDebug() << QString("WmsClient: 写入坐标成功 - blockIndex=%1").arg(blockIndex);
                emit adjustPositionSucceeded(blockIndex);
            } else {
                qWarning() << QString("WmsClient: 写入坐标失败 - blockIndex=%1, 错误: %2")
                              .arg(blockIndex).arg(errorMsg);
                emit adjustPositionFailed(blockIndex, errorMsg);
            }
        } else {
            QString errorMsg;
            switch (error) {
            case QNetworkReply::TimeoutError:
                errorMsg = QString("请求超时（%1秒）").arg(m_timeout / 1000);
                break;
            case QNetworkReply::ConnectionRefusedError:
                errorMsg = "WMS服务器拒绝连接，请检查地址和端口";
                break;
            case QNetworkReply::HostNotFoundError:
                errorMsg = "无法找到WMS服务器，请检查网络和地址";
                break;
            default:
                errorMsg = QString("网络错误: %1").arg(reply->errorString());
                break;
            }
            qWarning() << QString("WmsClient: 写入坐标失败 - blockIndex=%1, 错误: %2")
                          .arg(blockIndex).arg(errorMsg);
            emit adjustPositionFailed(blockIndex, errorMsg);
        }

        reply->deleteLater();
    });

    // 连接错误信号，获取更详细的错误信息（Qt 6 使用 errorOccurred 信号）
    connect(reply, &QNetworkReply::errorOccurred,
            this, [blockIndex, reply](QNetworkReply::NetworkError error) {
        qWarning() << QString("WmsClient: 写入坐标网络错误 - blockIndex=%1, 错误码=%2, 错误信息=%3")
                      .arg(blockIndex).arg(error).arg(reply->errorString());
        
        // 输出详细的错误信息
        QString errorType;
        switch (error) {
        case QNetworkReply::TimeoutError:
            errorType = "请求超时";
            break;
        case QNetworkReply::ConnectionRefusedError:
            errorType = "连接被拒绝（服务器可能未启动或端口错误）";
            break;
        case QNetworkReply::HostNotFoundError:
            errorType = "主机未找到（DNS解析失败或地址错误）";
            break;
        case QNetworkReply::RemoteHostClosedError:
            errorType = "远程主机关闭连接";
            break;
        case QNetworkReply::OperationCanceledError:
            errorType = "操作被取消";
            break;
        case QNetworkReply::ProxyConnectionRefusedError:
        case QNetworkReply::ProxyConnectionClosedError:
        case QNetworkReply::ProxyNotFoundError:
        case QNetworkReply::ProxyTimeoutError:
        case QNetworkReply::ProxyAuthenticationRequiredError:
            errorType = "代理相关错误";
            break;
        default:
            errorType = QString("其他网络错误 (错误码: %1)").arg(error);
            break;
        }
        qWarning() << QString("  错误类型: %1").arg(errorType);
    });
    
    // 连接SSL错误信号（如果使用HTTPS）
    #ifndef QT_NO_SSL
    connect(reply, &QNetworkReply::sslErrors, this, [blockIndex, reply](const QList<QSslError> &errors) {
        qWarning() << QString("WmsClient: SSL错误 - blockIndex=%1").arg(blockIndex);
        for (const auto &sslError : errors) {
            qWarning() << "  SSL错误:" << sslError.errorString();
        }
    });
    #endif
    
    // 连接上传进度信号（用于调试）
    connect(reply, &QNetworkReply::uploadProgress, this, [blockIndex](qint64 bytesSent, qint64 bytesTotal) {
        if (bytesTotal > 0) {
            qDebug() << QString("WmsClient: 上传进度 - blockIndex=%1, %2/%3 字节 (%4%)")
                        .arg(blockIndex)
                        .arg(bytesSent)
                        .arg(bytesTotal)
                        .arg((bytesSent * 100) / bytesTotal);
        }
    });
    
    // 连接下载进度信号（用于调试）
    connect(reply, &QNetworkReply::downloadProgress, this, [blockIndex](qint64 bytesReceived, qint64 bytesTotal) {
        if (bytesTotal > 0) {
            qDebug() << QString("WmsClient: 下载进度 - blockIndex=%1, %2/%3 字节 (%4%)")
                        .arg(blockIndex)
                        .arg(bytesReceived)
                        .arg(bytesTotal)
                        .arg((bytesReceived * 100) / bytesTotal);
        } else if (bytesReceived > 0) {
            qDebug() << QString("WmsClient: 已接收数据 - blockIndex=%1, %2 字节")
                        .arg(blockIndex).arg(bytesReceived);
        }
    });
    
    // 设置超时（60秒，WMS移动超时时间）
    QTimer::singleShot(60000, reply, [reply, blockIndex, this]() {
        if (reply->isRunning()) {
            qWarning() << QString("WmsClient: 写入坐标请求超时 - blockIndex=%1")
                          .arg(blockIndex)
                       << QString("  URL: %1").arg(reply->url().toString())
                       << QString("  请求状态: %1").arg(reply->isRunning() ? "运行中" : "已停止")
                       << QString("  错误信息: %1").arg(reply->errorString());
            reply->abort();
            emit adjustPositionFailed(blockIndex, "请求超时（60秒）");
            reply->deleteLater();
        }
    });
}

void WmsClient::onTimeout()
{
    if (m_currentReply) {
        qWarning() << QString("WmsClient: 请求超时 - %1").arg(m_currentResult.uniqueCode);
        
        m_currentReply->abort();
        emit reportFailed(m_currentResult.uniqueCode, 
                         QString("请求超时（%1秒）").arg(m_timeout / 1000));
        
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
}

