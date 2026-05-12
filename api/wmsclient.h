#ifndef WMSCLIENT_H
#define WMSCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include "apitypes.h"

/**
 * @brief WMS客户端类
 * 
 * 功能：主动向WMS服务器上报扫描结果
 * 
 * ⚠️ 占位值说明：
 * - WMS URL：当前使用占位值 "http://localhost:9090/api/result"
 * - 请求体：当前发送简化数据，详细字段后续迭代
 * - 认证：暂不实现，后续添加
 * - 错误处理：基础实现，详细错误码映射后续添加
 */
class WmsClient : public QObject
{
    Q_OBJECT

public:
    explicit WmsClient(QObject *parent = nullptr);
    ~WmsClient();

    /**
     * @brief 上报扫描结果到WMS
     * @param result 扫描结果数据
     * 
     * ⚠️ 当前实现：
     * - 发送基础的JSON数据到WMS
     * - 数据格式已简化，详细字段后续迭代
     */
    void reportResult(const ApiTypes::ScanResult &result);

    /**
     * @brief 向WMS写入调整坐标（用于精定位）
     * @param blockIndex 铜垛索引
     * @param x/y/z/deg 坐标
     * 
     * WMS收到后会移动天车到指定坐标
     */
    void adjustPosition(int blockIndex, double x, double y, double z, double deg);

    /**
     * @brief 设置WMS服务器地址
     * @param url 完整URL（例如：http://192.168.1.100:9090/api/result）
     * 
     * ⚠️ 当前默认值："http://localhost:9090/api/result" (占位值)
     */
    void setWmsUrl(const QString &url);
    QString wmsUrl() const { return m_wmsUrl; }

    /**
     * @brief 设置请求超时时间（毫秒）
     * ⚠️ 当前默认值：10000 (10秒)
     */
    void setTimeout(int timeout);
    int timeout() const { return m_timeout; }

signals:
    /**
     * @brief 上报成功信号
     * @param uniqueCode 任务编号
     */
    void reportSucceeded(const QString &uniqueCode);

    /**
     * @brief 上报失败信号
     * @param uniqueCode 任务编号
     * @param error 错误信息
     */
    void reportFailed(const QString &uniqueCode, const QString &error);

    /**
     * @brief 写入坐标成功信号
     * @param blockIndex 铜垛索引
     */
    void adjustPositionSucceeded(int blockIndex);

    /**
     * @brief 写入坐标失败信号
     * @param blockIndex 铜垛索引
     * @param error 错误信息
     */
    void adjustPositionFailed(int blockIndex, const QString &error);

private slots:
    /**
     * @brief 网络请求完成回调
     */
    void onRequestFinished();

    /**
     * @brief 请求超时处理
     */
    void onTimeout();

private:
    QNetworkAccessManager *m_networkManager;  // 网络管理器
    QString m_wmsUrl;                        // WMS服务器地址 ⚠️ 占位值
    int m_timeout;                           // 超时时间（毫秒）
    QTimer *m_timeoutTimer;                  // 超时定时器
    QNetworkReply *m_currentReply;          // 当前请求对象
    ApiTypes::ScanResult m_currentResult;              // 当前上报的结果数据
};

#endif // WMSCLIENT_H

