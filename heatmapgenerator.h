#ifndef HEATMAPGENERATOR_H
#define HEATMAPGENERATOR_H

#include <QObject>
#include <QImage>
#include <QColor>
#include <vector>
#include <memory>
#include "radarcontroller.h"

// 热力图类型枚举
enum class HeatmapType {
    Height,      // 基于Z轴高度的热力图（主要类型）
    Intensity,   // 基于反射强度的热力图
    Density      // 基于点密度的热力图
};

// 热力图配置结构
struct HeatmapConfig {
    int gridWidth = 800;            // 网格宽度
    int gridHeight = 600;           // 网格高度
    double minX = -5.0;             // X轴最小值（米）
    double maxX = 5.0;              // X轴最大值（米）
    double minY = -5.0;             // Y轴最小值（米）
    double maxY = 5.0;              // Y轴最大值（米）
    double minZ = -2.0;             // Z轴最小值（米）
    double maxZ = 3.0;              // Z轴最大值（米）
    HeatmapType type = HeatmapType::Height;  // 热力图类型（默认高度）
    bool autoRange = true;          // 自动计算范围
    QColor minColor = QColor(0, 0, 255);     // 最小值颜色（蓝色-低高度）
    QColor maxColor = QColor(255, 0, 0);     // 最大值颜色（红色-高高度）
    bool showGrid = true;           // 显示网格线
    bool showColorBar = true;       // 显示颜色条
};

// 热力图生成器类
class HeatmapGenerator : public QObject
{
    Q_OBJECT

public:
    explicit HeatmapGenerator(QObject *parent = nullptr);
    ~HeatmapGenerator();

    // 配置设置
    void setConfig(const HeatmapConfig &config);
    HeatmapConfig getConfig() const { return m_config; }

    // 热力图生成
    QImage generateHeatmap(const PointCloud &pointCloud);
    QImage generateHeatmap(const std::vector<Point3D> &profile);

    // 实时更新
    void updatePointCloud(const PointCloud &pointCloud);
    void updateProfile(const std::vector<Point3D> &profile);
    
    // 批量处理
    void updatePointCloudBatch(const std::vector<PointCloud> &pointClouds);
    void setProcessingMode(bool enableBatchProcessing);
    
    // 数据预处理
    void setDataFilter(bool enableFilter);
    void setNoiseReduction(bool enableNoiseReduction);
    void setOutlierRemoval(bool enableOutlierRemoval);

    // 获取当前热力图
    QImage getCurrentHeatmap() const { return m_currentHeatmap; }

    // 数据统计
    struct Statistics {
        int totalPoints = 0;
        double minValue = 0.0;
        double maxValue = 0.0;
        double avgValue = 0.0;
        double stdDev = 0.0;
    };
    Statistics getStatistics() const { return m_statistics; }

signals:
    void heatmapUpdated(const QImage &heatmap);
    void statisticsUpdated(const Statistics &stats);

private:
    // 内部方法
    void calculateStatistics(const std::vector<std::vector<double>> &grid);
    void updateStatistics(const PointCloud &pointCloud);
    QColor interpolateColor(double value, double minVal, double maxVal) const;
    
    // 不同类型的网格生成
    std::vector<std::vector<double>> generateHeightGrid(const PointCloud &pointCloud);
    std::vector<std::vector<double>> generateIntensityGrid(const PointCloud &pointCloud);
    std::vector<std::vector<double>> generateDensityGrid(const PointCloud &pointCloud);
    
    // 网格到图像的转换
    QImage gridToImage(const std::vector<std::vector<double>> &grid);
    
    // 数据预处理
    PointCloud filterPointCloud(const PointCloud &pointCloud);
    PointCloud removeOutliers(const PointCloud &pointCloud);
    PointCloud reduceNoise(const PointCloud &pointCloud);
    bool isValidPoint(const Point3D &point) const;
    
    // 批量处理
    std::vector<std::vector<double>> generateGridFromBatch(const std::vector<PointCloud> &pointClouds);
    
    // 辅助函数
    int getTotalPointCount(const PointCloud &pointCloud) const;

private:
    HeatmapConfig m_config;
    QImage m_currentHeatmap;
    Statistics m_statistics;
    
    // 数据缓存
    PointCloud m_cachedPointCloud;
    std::vector<Point3D> m_cachedProfile;
    
    // 网格数据
    std::vector<std::vector<double>> m_currentGrid;
    std::vector<std::vector<int>> m_countGrid;  // 用于密度计算
    
    // 处理模式
    bool m_batchProcessingMode;
    bool m_enableDataFilter;
    bool m_enableNoiseReduction;
    bool m_enableOutlierRemoval;
    
    // 数据过滤参数
    double m_minDistance;      // 最小距离
    double m_maxDistance;      // 最大距离
    double m_minIntensity;     // 最小强度
    double m_maxIntensity;     // 最大强度
    double m_outlierThreshold; // 异常值阈值
};

#endif // HEATMAPGENERATOR_H
