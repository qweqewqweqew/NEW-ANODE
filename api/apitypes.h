#ifndef APITYPES_H
#define APITYPES_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QDateTime>
#include "../include/ThreeDPlatformData.h"

/**
 * @file apitypes.h
 * @brief API数据结构定义
 * 
 * 本文件定义了所有与WMS通信的数据结构
 * 所有结构体都提供JSON序列化/反序列化功能
 */

namespace ApiTypes {

// ============================================
// 系统状态相关
// ============================================

/**
 * @brief 系统状态结构
 * 用于 GET /api/status 接口返回
 */
struct SystemStatus {
    QString system;        // 系统总体状态: "OK"(全部正常) / "Partial"(部分异常) / "Error"(严重错误)
    QString camera1;       // 相机1状态: "Connected" / "Disconnected" / "Busy"
    QString camera2;       // 相机2状态: "Connected" / "Disconnected" / "Busy"
    QString radar;         // 雷达状态: "Connected" / "Disconnected" / "Scanning"
    QString algorithm;     // 算法状态: "Ready" / "Processing" / "Error" (⚠️ 后续迭代)
    
    /**
     * 转换为JSON对象
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["system"] = system;
        obj["camera1"] = camera1;
        obj["camera2"] = camera2;
        obj["radar"] = radar;
        obj["algorithm"] = algorithm;
        return obj;
    }
};

// ============================================
// 扫描配置参数
// ============================================

/**
 * @brief 扫描配置参数结构
 * 用于 POST /api/config (设置) 和 GET /api/config (获取)
 * 
 * ⚠️ 注意：参数列表可能不完整，后续需要根据实际需求扩展
 */
struct ScanConfig {
    // === 当前已知参数 ===
    double fineTolerance;      // 精定位允许偏差值 (mm) - 文档提到
    double shrinkValue;        // 内缩值 (mm) - 文档提到
    
    // === 系统控制参数（合理假设） ===
    int radarScanTimeout;      // 雷达扫描超时 (秒)
    int cameraCaptureTimeout;  // 相机拍照超时 (秒)
    
    // === 后续可能需要的参数（占位，默认不使用） ===
    // double minPlateWidth;      // 最小板宽度 (mm)
    // double maxPlateWidth;      // 最大板宽度 (mm)
    // int minPlateCount;         // 最少板数
    // int maxPlateCount;         // 最多板数
    // double edgeThreshold;      // 边缘检测阈值
    // bool enableFilter;         // 是否启用点云滤波
    
    /**
     * 从JSON反序列化
     */
    static ScanConfig fromJson(const QJsonObject &obj) {
        ScanConfig config;
        config.fineTolerance = obj.value("fineTolerance").toDouble(2.0);
        config.shrinkValue = obj.value("shrinkValue").toDouble(8.5);
        config.radarScanTimeout = obj.value("radarScanTimeout").toInt(30);
        config.cameraCaptureTimeout = obj.value("cameraCaptureTimeout").toInt(10);
        return config;
    }
    
    /**
     * 序列化为JSON
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["fineTolerance"] = fineTolerance;
        obj["shrinkValue"] = shrinkValue;
        obj["radarScanTimeout"] = radarScanTimeout;
        obj["cameraCaptureTimeout"] = cameraCaptureTimeout;
        return obj;
    }
    
    /**
     * 默认配置
     */
    static ScanConfig defaultConfig() {
        ScanConfig config;
        config.fineTolerance = 2.0;         // 默认2mm偏差
        config.shrinkValue = 8.5;           // 默认8.5mm内缩
        config.radarScanTimeout = 30;       // 默认30秒超时
        config.cameraCaptureTimeout = 10;   // 默认10秒超时
        return config;
    }
    
    /**
     * 参数验证
     * @return 验证通过返回空字符串，失败返回错误信息
     */
    QString validate() const {
        if (fineTolerance <= 0) {
            return "fineTolerance必须大于0";
        }
        if (shrinkValue < 0) {
            return "shrinkValue不能为负数";
        }
        if (radarScanTimeout <= 0) {
            return "radarScanTimeout必须大于0";
        }
        if (cameraCaptureTimeout <= 0) {
            return "cameraCaptureTimeout必须大于0";
        }
        return "";  // 验证通过
    }
};

// ============================================
// 扫描任务请求
// ============================================

/**
 * @brief 扫描任务请求结构
 * 用于 POST /api/scan 接口接收WMS下发的任务
 */
struct ScanRequest {
    QString type;          // 扫描类型: "1"=雷达粗定位, "2"=相机精定位
    QString uniqueCode;    // 任务唯一编号 (WMS生成，需在结果中回传)
    
    /**
     * 从JSON反序列化
     */
    static ScanRequest fromJson(const QJsonObject &obj) {
        ScanRequest req;
        req.type = obj.value("type").toString();
        req.uniqueCode = obj.value("uniqueCode").toString();
        return req;
    }
    
    /**
     * 请求验证
     */
    bool isValid() const {
        return (type == "1" || type == "2") && !uniqueCode.isEmpty();
    }
    
    /**
     * 获取类型描述
     */
    QString typeDescription() const {
        if (type == "1") return "雷达粗定位";
        if (type == "2") return "相机精定位";
        return "未知类型";
    }
};

// ============================================
// 3D坐标点（识别结果数据）
// ============================================

/**
 * @brief 三维坐标点结构
 * 用于上报识别结果中的XYZ坐标数组
 * 
 * ⚠️ 注意：
 * - 坐标单位假定为毫米(mm)，需与算法团队确认
 * - 坐标系原点位置需确认（雷达本地坐标系 or 统一世界坐标系）
 */
struct Point3D {
    double x = 0.0;     // X坐标 (mm)
    double y = 0.0;     // Y坐标 (mm)
    double z = 0.0;     // Z坐标 (mm)
    bool flag = true;   // 是否为有效坐标 (true=有效, false=无效/需忽略)
    double deg = 0.0;   // 姿态角（仅中心点使用）
    bool hasDeg = false;
    QString pointType;  // 点的类型：center / corner1~4 / 其他
    int blockIndex = -1; // 所属铜跺编号
    
    /**
     * 序列化为JSON
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["x"] = x;
        obj["y"] = y;
        obj["z"] = z;
        obj["flag"] = flag;
        if (hasDeg) {
            obj["deg"] = deg;
        }
        if (!pointType.isEmpty()) {
            obj["pointType"] = pointType;
        }
        if (blockIndex >= 0) {
            obj["blockIndex"] = blockIndex;
        }
        return obj;
    }
};

// ============================================
// 识别结果（上报给WMS）
// ============================================

/**
 * @brief 识别结果结构
 * 用于主动调用 POST {WMS_URL}/api/result 上报结果
 * 
 * ⚠️ 关键迭代点：
 * - base64字段：雷达扫描时如何生成图片？（目前未实现）
 * - data数组：粗定位返回几个点？精定位返回几个点？（待算法确认）
 */
struct ScanResult {
    int code;                      // 结果码: 0=失败, 1=成功
    QString message;               // 结果描述 (成功或失败原因)
    QString uniqueCode;            // 任务唯一编号 (回传WMS下发的编号)
    QString base64;                // PNG格式结果图片的Base64编码 (⚠️ 后续实现)
    QString type;                  // 扫描类型: "1"=雷达, "2"=相机
    QVector<Point3D> data;         // XYZ坐标数组
    
    /**
     * 序列化为JSON (完整格式，符合文档要求)
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["code"] = code;
        obj["message"] = message;
        obj["uniqueCode"] = uniqueCode;
        obj["base64"] = base64;
        obj["type"] = type;
        
        QJsonArray dataArray;
        for (const auto &point : data) {
            dataArray.append(point.toJson());
        }
        obj["data"] = dataArray;
        
        return obj;
    }
    
    /**
     * 创建成功结果
     */
    static ScanResult success(const QString &uniqueCode, const QString &type, const QString &message = "识别成功") {
        ScanResult result;
        result.code = 1;
        result.message = message;
        result.uniqueCode = uniqueCode;
        result.type = type;
        result.base64 = "";  // ⚠️ 暂时为空，Phase 2实现
        return result;
    }
    
    /**
     * 创建失败结果
     */
    static ScanResult failure(const QString &uniqueCode, const QString &type, const QString &reason) {
        ScanResult result;
        result.code = 0;
        result.message = reason;
        result.uniqueCode = uniqueCode;
        result.type = type;
        result.base64 = "";  // 失败时也可能需要图片（显示问题区域）
        return result;
    }
};

// ============================================
// API响应基类
// ============================================

/**
 * @brief 通用API响应结构
 * 用于除了特定数据外的所有接口响应
 */
struct ApiResponse {
    bool success;       // 操作是否成功
    QString message;    // 响应消息
    QString error;      // 错误码 (可选，失败时填充)
    
    /**
     * 序列化为JSON
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["success"] = success;
        obj["message"] = message;
        if (!error.isEmpty()) {
            obj["error"] = error;
        }
        return obj;
    }
    
    /**
     * 创建成功响应
     */
    static ApiResponse successResponse(const QString &msg = "操作成功") {
        return {true, msg, ""};
    }
    
    /**
     * 创建错误响应
     */
    static ApiResponse errorResponse(const QString &msg, const QString &errorCode = "") {
        return {false, msg, errorCode};
    }
};

// ============================================
// 扫描任务数据（与旧代码D3DDataStruct对应）
// ============================================

/**
 * @brief 扫描任务数据结构（兼容旧代码命名）
 * 对应旧代码中的 D3DDataStruct，使用 d3dPoints 字段名保持一致性
 * 
 * ⚠️ 注意：
 * - d3dPoints 类型为 Double3DPointVec（即 PointCloud），与旧代码一致
 * - 此结构用于内部任务处理时保存完整的扫描上下文
 */
struct ScanTaskData {
    QString uniqueCode;                    // 任务唯一编号 (WMS下发的uniqueCode)
    QString type;                          // 扫描类型: "1"=雷达粗定位, "2"=相机精定位
    Double3DPointVec d3dPoints;             // 点云数据（与旧代码D3DDataStruct::d3dPoints命名一致）
    QDateTime timestamp;                   // 扫描时间戳
    ScanConfig config;                     // 扫描时的配置参数（可选）
    
    /**
     * 默认构造函数
     */
    ScanTaskData() : timestamp(QDateTime::currentDateTime()) {}
    
    /**
     * 从点云数据创建
     */
    ScanTaskData(const QString &uniqueCode, const QString &type, const Double3DPointVec &pointCloud)
        : uniqueCode(uniqueCode), type(type), d3dPoints(pointCloud), 
          timestamp(QDateTime::currentDateTime()) {}
    
    /**
     * 清空点云数据
     */
    void clear() {
        for (auto &scanLine : d3dPoints) {
            scanLine.clear();
        }
        d3dPoints.clear();
    }
    
    /**
     * 检查是否有效
     */
    bool isValid() const {
        return !uniqueCode.isEmpty() && !d3dPoints.empty();
    }
    
    /**
     * 获取点云数据大小
     */
    int getPointCount() const {
        int count = 0;
        for (const auto &scanLine : d3dPoints) {
            count += scanLine.size();
        }
        return count;
    }
};

// ============================================
// 错误码定义
// ============================================

/**
 * @brief 标准错误码
 * 用于API响应中的error字段
 */
namespace ErrorCodes {
    constexpr const char* SUCCESS = "SUCCESS";
    constexpr const char* DEVICE_NOT_CONNECTED = "DEVICE_NOT_CONNECTED";     // 设备未连接
    constexpr const char* SYSTEM_BUSY = "SYSTEM_BUSY";                       // 系统繁忙
    constexpr const char* INVALID_PARAMETER = "INVALID_PARAMETER";           // 参数错误
    constexpr const char* TIMEOUT = "TIMEOUT";                               // 超时
    constexpr const char* ALGORITHM_FAILED = "ALGORITHM_FAILED";             // 算法失败 (⚠️ 后续实现)
    constexpr const char* NETWORK_ERROR = "NETWORK_ERROR";                   // 网络错误
    constexpr const char* NOT_IMPLEMENTED = "NOT_IMPLEMENTED";               // 功能未实现 (Phase 1使用)
}

} // namespace ApiTypes

#endif // APITYPES_H


