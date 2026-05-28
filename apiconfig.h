#ifndef APICONFIG_H
#define APICONFIG_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include "apitypes.h"

/**
 * @file apiconfig.h
 * @brief API配置管理类（单例）
 * 
 * 负责：
 * 1. 读写配置文件 config/scan_config.json
 * 2. 管理扫描参数、WMS地址、服务器端口等配置
 * 3. 提供默认配置
 * 
 * ⚠️ 后续迭代点：
 * - WMS地址目前为占位值，需要实际环境地址
 * - 参数列表可能不完整，需根据算法需求扩展
 */
class ApiConfig
{
public:
    struct PreAlignConfig {
        double sx = 0.0;
        double currentX = 0.0;
        double lidar2Gnd = 0.0;
        QVector<double> matLidar2Crane;
        double offsetTbX = 0.0;
        double offsetTbY = 0.0;
        double offsetTbZ = 0.0;
        double offsetTbDeg = 0.0;
        double cameraOffsetX = 0.0;
        double cameraOffsetY = 0.0;
        double cameraOffsetZ = 0.0;
        double cameraOffsetDeg = 0.0;
        double limitCameraOffsetXMin = 0.0;
        double limitCameraOffsetXMax = 0.0;
        double limitCameraOffsetYMin = 0.0;
        double limitCameraOffsetYMax = 0.0;
        double limitCameraOffsetZMin = 0.0;
        double limitCameraOffsetZMax = 0.0;
        double limitCameraOffsetDegMin = 0.0;
        double limitCameraOffsetDegMax = 0.0;

        QJsonObject toJson() const {
            QJsonObject obj;
            obj["sx"] = sx;
            obj["currentX"] = currentX;
            obj["lidar2Gnd"] = lidar2Gnd;

            QJsonArray matArr;
            for (double value : matLidar2Crane) {
                matArr.append(value);
            }
            obj["matLidar2Crane"] = matArr;

            obj["offsetTbX"] = offsetTbX;
            obj["offsetTbY"] = offsetTbY;
            obj["offsetTbZ"] = offsetTbZ;
            obj["offsetTbDeg"] = offsetTbDeg;

            obj["cameraOffsetX"] = cameraOffsetX;
            obj["cameraOffsetY"] = cameraOffsetY;
            obj["cameraOffsetZ"] = cameraOffsetZ;
            obj["cameraOffsetDeg"] = cameraOffsetDeg;

            obj["limitCameraOffsetXMin"] = limitCameraOffsetXMin;
            obj["limitCameraOffsetXMax"] = limitCameraOffsetXMax;
            obj["limitCameraOffsetYMin"] = limitCameraOffsetYMin;
            obj["limitCameraOffsetYMax"] = limitCameraOffsetYMax;
            obj["limitCameraOffsetZMin"] = limitCameraOffsetZMin;
            obj["limitCameraOffsetZMax"] = limitCameraOffsetZMax;
            obj["limitCameraOffsetDegMin"] = limitCameraOffsetDegMin;
            obj["limitCameraOffsetDegMax"] = limitCameraOffsetDegMax;
            return obj;
        }

        static PreAlignConfig fromJson(const QJsonObject &obj) {
            PreAlignConfig cfg;
            cfg.sx = obj.value("sx").toDouble(0.0);
            cfg.currentX = obj.value("currentX").toDouble(0.0);
            cfg.lidar2Gnd = obj.value("lidar2Gnd").toDouble(0.0);
            if (obj.contains("matLidar2Crane")) {
                QJsonArray arr = obj.value("matLidar2Crane").toArray();
                cfg.matLidar2Crane.reserve(arr.size());
                for (const auto &value : arr) {
                    cfg.matLidar2Crane.append(value.toDouble(0.0));
                }
            }
            cfg.offsetTbX = obj.value("offsetTbX").toDouble(0.0);
            cfg.offsetTbY = obj.value("offsetTbY").toDouble(0.0);
            cfg.offsetTbZ = obj.value("offsetTbZ").toDouble(0.0);
            cfg.offsetTbDeg = obj.value("offsetTbDeg").toDouble(0.0);

            cfg.cameraOffsetX = obj.value("cameraOffsetX").toDouble(0.0);
            cfg.cameraOffsetY = obj.value("cameraOffsetY").toDouble(0.0);
            cfg.cameraOffsetZ = obj.value("cameraOffsetZ").toDouble(0.0);
            cfg.cameraOffsetDeg = obj.value("cameraOffsetDeg").toDouble(0.0);

            cfg.limitCameraOffsetXMin = obj.value("limitCameraOffsetXMin").toDouble(0.0);
            cfg.limitCameraOffsetXMax = obj.value("limitCameraOffsetXMax").toDouble(0.0);
            cfg.limitCameraOffsetYMin = obj.value("limitCameraOffsetYMin").toDouble(0.0);
            cfg.limitCameraOffsetYMax = obj.value("limitCameraOffsetYMax").toDouble(0.0);
            cfg.limitCameraOffsetZMin = obj.value("limitCameraOffsetZMin").toDouble(0.0);
            cfg.limitCameraOffsetZMax = obj.value("limitCameraOffsetZMax").toDouble(0.0);
            cfg.limitCameraOffsetDegMin = obj.value("limitCameraOffsetDegMin").toDouble(0.0);
            cfg.limitCameraOffsetDegMax = obj.value("limitCameraOffsetDegMax").toDouble(0.0);
            return cfg;
        }

        static PreAlignConfig defaultConfig() {
            PreAlignConfig cfg;
            cfg.matLidar2Crane = QVector<double>(12, 0.0);
            return cfg;
        }
    };

    /**
     * 获取单例实例
     */
    static ApiConfig& instance() {
        static ApiConfig instance;
        return instance;
    }
    
    // 禁止拷贝和赋值
    ApiConfig(const ApiConfig&) = delete;
    ApiConfig& operator=(const ApiConfig&) = delete;
    
    /**
     * 从文件加载配置
     * @param filePath 配置文件路径（如果为空，则使用相对于exe的路径）
     * @return 加载成功返回true
     */
    bool load(const QString &filePath = "") {
        // 如果未指定路径，尝试多个可能的路径
        if (filePath.isEmpty()) {
            // 获取exe所在目录
            QString exeDir = QCoreApplication::applicationDirPath();
            
            // 尝试路径1：exe目录下的config（用于部署后的程序）
            QString path1 = exeDir + "/config/scan_config.json";
            // 尝试路径2：项目根目录的bin/config（用于开发环境）
            // 从exe目录向上查找，找到包含bin目录的路径
            QString path2 = "";
            QDir dir(exeDir);
            while (!dir.isRoot()) {
                QString binPath = dir.absolutePath() + "/bin/config/scan_config.json";
                if (QFile::exists(binPath)) {
                    path2 = binPath;
                    break;
                }
                dir.cdUp();
            }
            
            // 优先使用存在的配置文件
            if (QFile::exists(path1)) {
                m_configFilePath = path1;
            } else if (!path2.isEmpty() && QFile::exists(path2)) {
                m_configFilePath = path2;
            } else {
                // 都不存在，使用exe目录下的路径（会创建新文件）
                m_configFilePath = path1;
            }
        } else {
            m_configFilePath = filePath;
        }
        
        qDebug() << QString("[ApiConfig] 尝试加载配置文件: %1").arg(m_configFilePath);
        
        QFile file(m_configFilePath);
        if (!file.exists()) {
            qWarning() << "配置文件不存在，使用默认配置:" << m_configFilePath;
            // 创建默认配置文件
            m_scanConfig = ApiTypes::ScanConfig::defaultConfig();
            return save();
        }
        
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "无法打开配置文件:" << m_configFilePath << file.errorString();
            m_scanConfig = ApiTypes::ScanConfig::defaultConfig();
            return false;
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "配置文件JSON解析失败:" << parseError.errorString();
            m_scanConfig = ApiTypes::ScanConfig::defaultConfig();
            return false;
        }
        
        QJsonObject root = doc.object();
        
        // 解析扫描配置
        if (root.contains("scanConfig")) {
            m_scanConfig = ApiTypes::ScanConfig::fromJson(root["scanConfig"].toObject());
        } else {
            m_scanConfig = ApiTypes::ScanConfig::defaultConfig();
        }
        
        // 解析WMS配置
        if (root.contains("wmsConfig")) {
            QJsonObject wmsObj = root["wmsConfig"].toObject();
            qDebug() << QString("[ApiConfig] wmsConfig对象键列表: %1").arg(wmsObj.keys().join(", "));
            
            // 兼容旧配置：如果只有url字段，则同时设置两个接口URL
            QString baseUrl = wmsObj.value("url").toString("http://localhost:9090");
            qDebug() << QString("[ApiConfig] 读取baseUrl: %1").arg(baseUrl);
            
            if (wmsObj.contains("reportResultUrl")) {
                m_wmsReportResultUrl = wmsObj.value("reportResultUrl").toString();
                qDebug() << QString("[ApiConfig] 读取reportResultUrl: %1").arg(m_wmsReportResultUrl);
            } else {
                // 从旧配置推导：http://localhost:9090 -> http://localhost:8099/api/OpenApi/ReceiveUnloadPositionData
                // 接口7：上报识别结果
                QUrl url(baseUrl);
                m_wmsReportResultUrl = QString("http://%1:8099/api/OpenApi/ReceiveUnloadPositionData")
                                        .arg(url.host());
                qDebug() << QString("[ApiConfig] reportResultUrl未配置，从baseUrl推导: %1").arg(m_wmsReportResultUrl);
            }
            if (wmsObj.contains("adjustPositionUrl")) {
                m_wmsAdjustPositionUrl = wmsObj.value("adjustPositionUrl").toString();
                qDebug() << QString("[ApiConfig] 读取adjustPositionUrl: %1").arg(m_wmsAdjustPositionUrl);
            } else {
                // 从旧配置推导：http://localhost:9090 -> http://localhost:8099/api/OpenApi/RequstMoveForActualLocation
                // 接口8：请求精定位移动
                QUrl url(baseUrl);
                m_wmsAdjustPositionUrl = QString("http://%1:8099/api/OpenApi/RequstMoveForActualLocation")
                                          .arg(url.host());
                qDebug() << QString("[ApiConfig] adjustPositionUrl未配置，从baseUrl推导: %1").arg(m_wmsAdjustPositionUrl);
            }
            // 保留旧字段用于兼容
            m_wmsUrl = baseUrl;
            m_wmsTimeout = wmsObj.value("timeout").toInt(10000);
            m_wmsRetryCount = wmsObj.value("retryCount").toInt(3);
            qDebug() << QString("[ApiConfig] WMS配置加载完成 - baseUrl: %1, reportUrl: %2, adjustUrl: %3")
                        .arg(m_wmsUrl).arg(m_wmsReportResultUrl).arg(m_wmsAdjustPositionUrl);
            qDebug() << QString("[ApiConfig] 配置文件路径: %1").arg(m_configFilePath);
        } else {
            qWarning() << "[ApiConfig] 配置文件中未找到wmsConfig节点";
        }
        
        // 解析服务器配置
        if (root.contains("serverConfig")) {
            QJsonObject serverObj = root["serverConfig"].toObject();
            m_serverPort = serverObj.value("port").toInt(8080);
            m_logLevel = serverObj.value("logLevel").toString("INFO");
            // host为空或"0.0.0.0"表示监听所有接口，否则监听指定IP
            m_serverHost = serverObj.value("host").toString("0.0.0.0");
        }

        if (root.contains("preAlignConfig")) {
            m_preAlignConfig = PreAlignConfig::fromJson(root["preAlignConfig"].toObject());
        } else {
            m_preAlignConfig = PreAlignConfig::defaultConfig();
        }
        
        qDebug() << "配置文件加载成功:" << filePath;
        return true;
    }
    
    /**
     * 保存配置到文件
     * @param filePath 配置文件路径（空则使用load时的路径）
     * @return 保存成功返回true
     */
    bool save(const QString &filePath = "") {
        QString savePath = filePath.isEmpty() ? m_configFilePath : filePath;
        
        // 确保目录存在
        QFileInfo fileInfo(savePath);
        QDir dir = fileInfo.absoluteDir();
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                qWarning() << "无法创建配置目录:" << dir.path();
                return false;
            }
        }
        
        // 构建JSON
        QJsonObject root;
        
        // 扫描配置
        root["scanConfig"] = m_scanConfig.toJson();
        
        // WMS配置
        QJsonObject wmsObj;
        wmsObj["url"] = m_wmsUrl;  // 保留旧字段用于兼容
        wmsObj["reportResultUrl"] = m_wmsReportResultUrl;
        wmsObj["adjustPositionUrl"] = m_wmsAdjustPositionUrl;
        wmsObj["timeout"] = m_wmsTimeout;
        wmsObj["retryCount"] = m_wmsRetryCount;
        root["wmsConfig"] = wmsObj;
        
        // 服务器配置
        QJsonObject serverObj;
        serverObj["port"] = m_serverPort;
        serverObj["logLevel"] = m_logLevel;
        serverObj["host"] = m_serverHost;
        root["serverConfig"] = serverObj;

        root["preAlignConfig"] = m_preAlignConfig.toJson();
        
        // 版本信息
        root["version"] = "1.0";
        root["description"] = "青海阳极卸车视觉系统配置文件";
        root["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        // 写入文件
        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "无法写入配置文件:" << savePath << file.errorString();
            return false;
        }
        
        QJsonDocument doc(root);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        
        qDebug() << "配置文件保存成功:" << savePath;
        return true;
    }
    
    // ========================================
    // 访问器：扫描配置
    // ========================================
    
    ApiTypes::ScanConfig scanConfig() const { 
        return m_scanConfig; 
    }
    
    void setScanConfig(const ApiTypes::ScanConfig &config) {
        m_scanConfig = config;
        save();  // 自动保存
    }
    
    // ========================================
    // 访问器：WMS配置
    // ⚠️ 注意：wmsUrl 目前为占位值，需要实际配置
    // ========================================
    
    QString wmsUrl() const { 
        return m_wmsUrl; 
    }
    
    void setWmsUrl(const QString &url) { 
        m_wmsUrl = url; 
        save(); 
    }
    
    int wmsTimeout() const { 
        return m_wmsTimeout; 
    }
    
    void setWmsTimeout(int timeout) { 
        m_wmsTimeout = timeout; 
        save(); 
    }
    
    int wmsRetryCount() const { 
        return m_wmsRetryCount; 
    }
    
    void setWmsRetryCount(int count) { 
        m_wmsRetryCount = count; 
        save(); 
    }
    
    QString wmsReportResultUrl() const {
        return m_wmsReportResultUrl;
    }
    
    void setWmsReportResultUrl(const QString &url) {
        m_wmsReportResultUrl = url;
        save();
    }
    
    QString wmsAdjustPositionUrl() const {
        return m_wmsAdjustPositionUrl;
    }
    
    void setWmsAdjustPositionUrl(const QString &url) {
        m_wmsAdjustPositionUrl = url;
        save();
    }
    
    // ========================================
    // 访问器：服务器配置
    // ========================================
    
    quint16 serverPort() const { 
        return m_serverPort; 
    }
    
    void setServerPort(quint16 port) { 
        m_serverPort = port; 
        save(); 
    }
    
    QString logLevel() const { 
        return m_logLevel; 
    }
    
    void setLogLevel(const QString &level) { 
        m_logLevel = level; 
        save(); 
    }
    
    QString serverHost() const { 
        return m_serverHost; 
    }
    
    void setServerHost(const QString &host) { 
        m_serverHost = host; 
        save(); 
    }

    PreAlignConfig preAlignConfig() const {
        return m_preAlignConfig;
    }

    void setPreAlignConfig(const PreAlignConfig &config) {
        m_preAlignConfig = config;
        save();
    }
    
private:
    ApiConfig() {
        // 默认值
        m_scanConfig = ApiTypes::ScanConfig::defaultConfig();
        m_wmsUrl = "http://192.168.3.94:8099";  // WMS服务器地址
        m_wmsReportResultUrl = "http://192.168.3.94:8099/api/OpenApi/ReceiveUnloadPositionData";  // 接口7：上报识别结果
        m_wmsAdjustPositionUrl = "http://192.168.3.94:8099/api/OpenApi/RequstMoveForActualLocation";  // 接口8：请求精定位移动
        m_wmsTimeout = 10000;     // 10秒
        m_wmsRetryCount = 3;
        m_serverPort = 8080;
        m_logLevel = "INFO";
        m_serverHost = "0.0.0.0";  // 默认监听所有接口
        // 默认路径将在load()时根据exe位置自动设置
        m_configFilePath = "";
        m_preAlignConfig = PreAlignConfig::defaultConfig();
    }
    
    ~ApiConfig() = default;
    
    // 配置数据
    ApiTypes::ScanConfig m_scanConfig;
    QString m_wmsUrl;  // 旧字段，保留用于兼容
    QString m_wmsReportResultUrl;  // 上报识别结果接口URL
    QString m_wmsAdjustPositionUrl;  // 请求精定位移动接口URL
    int m_wmsTimeout;
    int m_wmsRetryCount;
    quint16 m_serverPort;
    QString m_logLevel;
    QString m_serverHost;  // 服务器监听地址，"0.0.0.0"表示所有接口，否则监听指定IP
    QString m_configFilePath;
    PreAlignConfig m_preAlignConfig;
};

#endif // APICONFIG_H























