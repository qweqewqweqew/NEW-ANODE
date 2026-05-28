#include "heatmapgenerator.h"
#include <QDebug>
#include <algorithm>
#include <cmath>

HeatmapGenerator::HeatmapGenerator(QObject *parent)
    : QObject(parent)
    , m_batchProcessingMode(false)
    , m_enableDataFilter(true)
    , m_enableNoiseReduction(true)
    , m_enableOutlierRemoval(true)
    , m_minDistance(0.1)      // 最小距离0.1米
    , m_maxDistance(50.0)     // 最大距离50米
    , m_minIntensity(10)      // 最小强度
    , m_maxIntensity(1000)    // 最大强度
    , m_outlierThreshold(3.0) // 异常值阈值3倍标准差
{
    // 初始化默认配置
    m_config = HeatmapConfig();
    
    // 创建空的热力图
    m_currentHeatmap = QImage(m_config.gridWidth, m_config.gridHeight, QImage::Format_RGB32);
    m_currentHeatmap.fill(Qt::black);
    
    // 初始化网格
    m_currentGrid.resize(m_config.gridHeight);
    m_countGrid.resize(m_config.gridHeight);
    for (int i = 0; i < m_config.gridHeight; ++i) {
        m_currentGrid[i].resize(m_config.gridWidth, 0.0);
        m_countGrid[i].resize(m_config.gridWidth, 0);
    }
}

HeatmapGenerator::~HeatmapGenerator()
{
}

void HeatmapGenerator::setConfig(const HeatmapConfig &config)
{
    m_config = config;
    
    // 重新创建网格
    m_currentGrid.resize(m_config.gridHeight);
    m_countGrid.resize(m_config.gridHeight);
    for (int i = 0; i < m_config.gridHeight; ++i) {
        m_currentGrid[i].resize(m_config.gridWidth, 0.0);
        m_countGrid[i].resize(m_config.gridWidth, 0);
    }
    
    // 重新生成热力图
    if (!m_cachedPointCloud.empty()) {
        updatePointCloud(m_cachedPointCloud);
    } else if (!m_cachedProfile.empty()) {
        updateProfile(m_cachedProfile);
    }
}

QImage HeatmapGenerator::generateHeatmap(const PointCloud &pointCloud)
{
    std::vector<std::vector<double>> grid;
    
    // 根据类型生成不同的网格
    switch (m_config.type) {
    case HeatmapType::Height:
        grid = generateHeightGrid(pointCloud);
        break;
    case HeatmapType::Intensity:
        grid = generateIntensityGrid(pointCloud);
        break;
    case HeatmapType::Density:
        grid = generateDensityGrid(pointCloud);
        break;
    }
    
    // 计算统计信息
    calculateStatistics(grid);
    
    // 转换为图像
    m_currentHeatmap = gridToImage(grid);
    m_currentGrid = grid;
    
    return m_currentHeatmap;
}

QImage HeatmapGenerator::generateHeatmap(const std::vector<Point3D> &profile)
{
    // 将单帧轮廓转换为点云格式
    PointCloud pointCloud;
    if (!profile.empty()) {
        pointCloud.push_back(profile);
    }
    
    return generateHeatmap(pointCloud);
}

void HeatmapGenerator::updatePointCloud(const PointCloud &pointCloud)
{
    m_cachedPointCloud = pointCloud;
    QImage heatmap = generateHeatmap(pointCloud);
    emit heatmapUpdated(heatmap);
    emit statisticsUpdated(m_statistics);
}

void HeatmapGenerator::updateProfile(const std::vector<Point3D> &profile)
{
    m_cachedProfile = profile;
    QImage heatmap = generateHeatmap(profile);
    emit heatmapUpdated(heatmap);
    emit statisticsUpdated(m_statistics);
}

void HeatmapGenerator::calculateStatistics(const std::vector<std::vector<double>> &grid)
{
    m_statistics = Statistics();
    
    std::vector<double> values;
    double sum = 0.0;
    int count = 0;
    
    for (const auto &row : grid) {
        for (double value : row) {
            if (value != 0.0) {  // 忽略空值
                values.push_back(value);
                sum += value;
                count++;
            }
        }
    }
    
    if (count == 0) {
        return;
    }
    
    m_statistics.totalPoints = count;
    m_statistics.avgValue = sum / count;
    
    if (!values.empty()) {
        std::sort(values.begin(), values.end());
        m_statistics.minValue = values.front();
        m_statistics.maxValue = values.back();
        
        // 计算标准差
        double variance = 0.0;
        for (double value : values) {
            variance += (value - m_statistics.avgValue) * (value - m_statistics.avgValue);
        }
        m_statistics.stdDev = std::sqrt(variance / count);
    }
}

QColor HeatmapGenerator::interpolateColor(double value, double minVal, double maxVal) const
{
    if (maxVal == minVal) {
        return m_config.minColor;
    }
    
    // 归一化到0-1范围
    double normalized = (value - minVal) / (maxVal - minVal);
    normalized = std::max(0.0, std::min(1.0, normalized));
    
    // 线性插值颜色
    int r = static_cast<int>(m_config.minColor.red() + 
                            (m_config.maxColor.red() - m_config.minColor.red()) * normalized);
    int g = static_cast<int>(m_config.minColor.green() + 
                            (m_config.maxColor.green() - m_config.minColor.green()) * normalized);
    int b = static_cast<int>(m_config.minColor.blue() + 
                            (m_config.maxColor.blue() - m_config.minColor.blue()) * normalized);
    
    return QColor(r, g, b);
}

std::vector<std::vector<double>> HeatmapGenerator::generateHeightGrid(const PointCloud &pointCloud)
{
    // 重置网格
    for (auto &row : m_currentGrid) {
        std::fill(row.begin(), row.end(), 0.0);
    }
    for (auto &row : m_countGrid) {
        std::fill(row.begin(), row.end(), 0);
    }
    
    if (pointCloud.empty()) {
        return m_currentGrid;
    }
    
    // 如果启用自动范围，计算实际范围
    if (m_config.autoRange) {
        double minX = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();
        
        for (const auto &scanLine : pointCloud) {
            for (const auto &point : scanLine) {
                minX = std::min(minX, static_cast<double>(point.X));
                maxX = std::max(maxX, static_cast<double>(point.X));
                minY = std::min(minY, static_cast<double>(point.Y));
                maxY = std::max(maxY, static_cast<double>(point.Y));
            }
        }
        
        m_config.minX = minX;
        m_config.maxX = maxX;
        m_config.minY = minY;
        m_config.maxY = maxY;
    }
    
    // 计算范围
    double rangeX = m_config.maxX - m_config.minX;
    double rangeY = m_config.maxY - m_config.minY;
    
    if (rangeX <= 0 || rangeY <= 0) {
        qDebug() << "无效的范围设置";
        return m_currentGrid;
    }
    
    // 填充网格 - 基于X、Y坐标确定网格位置，Z坐标确定颜色
    int totalPoints = 0;
    for (const auto &scanLine : pointCloud) {
        for (const auto &point : scanLine) {
            // 将3D点投影到2D网格
            int gridX = static_cast<int>((point.X - m_config.minX) / rangeX * m_config.gridWidth);
            int gridY = static_cast<int>((point.Y - m_config.minY) / rangeY * m_config.gridHeight);
            
            if (gridX >= 0 && gridX < m_config.gridWidth && 
                gridY >= 0 && gridY < m_config.gridHeight) {
                
                // 累加Z轴高度值
                m_currentGrid[gridY][gridX] += point.Z;
                m_countGrid[gridY][gridX]++;
                totalPoints++;
            }
        }
    }
    
    qDebug() << QString("处理了 %1 个点云数据点").arg(totalPoints);
    
    // 计算平均高度
    int validCells = 0;
    for (int y = 0; y < m_config.gridHeight; ++y) {
        for (int x = 0; x < m_config.gridWidth; ++x) {
            if (m_countGrid[y][x] > 0) {
                m_currentGrid[y][x] /= m_countGrid[y][x];  // 计算平均高度
                validCells++;
            }
        }
    }
    
    qDebug() << QString("生成了 %1 个有效网格单元").arg(validCells);
    
    return m_currentGrid;
}

std::vector<std::vector<double>> HeatmapGenerator::generateIntensityGrid(const PointCloud &pointCloud)
{
    // 重置网格
    for (auto &row : m_currentGrid) {
        std::fill(row.begin(), row.end(), 0.0);
    }
    for (auto &row : m_countGrid) {
        std::fill(row.begin(), row.end(), 0);
    }
    
    // 如果启用自动范围，计算实际范围
    if (m_config.autoRange && !pointCloud.empty()) {
        double minX = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();
        
        for (const auto &scanLine : pointCloud) {
            for (const auto &point : scanLine) {
                minX = std::min(minX, static_cast<double>(point.X));
                maxX = std::max(maxX, static_cast<double>(point.X));
                minY = std::min(minY, static_cast<double>(point.Y));
                maxY = std::max(maxY, static_cast<double>(point.Y));
            }
        }
        
        m_config.minX = minX;
        m_config.maxX = maxX;
        m_config.minY = minY;
        m_config.maxY = maxY;
    }
    
    // 填充网格
    for (const auto &scanLine : pointCloud) {
        for (const auto &point : scanLine) {
            // 计算网格坐标
            int gridX = static_cast<int>((point.X - m_config.minX) / 
                                       (m_config.maxX - m_config.minX) * m_config.gridWidth);
            int gridY = static_cast<int>((point.Y - m_config.minY) / 
                                       (m_config.maxY - m_config.minY) * m_config.gridHeight);
            
            if (gridX >= 0 && gridX < m_config.gridWidth && 
                gridY >= 0 && gridY < m_config.gridHeight) {
                m_currentGrid[gridY][gridX] += point.RSSI;  // 累加强度
                m_countGrid[gridY][gridX]++;                     // 计数
            }
        }
    }
    
    // 计算平均强度
    for (int y = 0; y < m_config.gridHeight; ++y) {
        for (int x = 0; x < m_config.gridWidth; ++x) {
            if (m_countGrid[y][x] > 0) {
                m_currentGrid[y][x] /= m_countGrid[y][x];
            }
        }
    }
    
    return m_currentGrid;
}

std::vector<std::vector<double>> HeatmapGenerator::generateDensityGrid(const PointCloud &pointCloud)
{
    // 重置网格
    for (auto &row : m_currentGrid) {
        std::fill(row.begin(), row.end(), 0.0);
    }
    for (auto &row : m_countGrid) {
        std::fill(row.begin(), row.end(), 0);
    }
    
    // 如果启用自动范围，计算实际范围
    if (m_config.autoRange && !pointCloud.empty()) {
        double minX = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();
        
        for (const auto &scanLine : pointCloud) {
            for (const auto &point : scanLine) {
                minX = std::min(minX, static_cast<double>(point.X));
                maxX = std::max(maxX, static_cast<double>(point.X));
                minY = std::min(minY, static_cast<double>(point.Y));
                maxY = std::max(maxY, static_cast<double>(point.Y));
            }
        }
        
        m_config.minX = minX;
        m_config.maxX = maxX;
        m_config.minY = minY;
        m_config.maxY = maxY;
    }
    
    // 填充网格
    for (const auto &scanLine : pointCloud) {
        for (const auto &point : scanLine) {
            // 计算网格坐标
            int gridX = static_cast<int>((point.X - m_config.minX) / 
                                       (m_config.maxX - m_config.minX) * m_config.gridWidth);
            int gridY = static_cast<int>((point.Y - m_config.minY) / 
                                       (m_config.maxY - m_config.minY) * m_config.gridHeight);
            
            if (gridX >= 0 && gridX < m_config.gridWidth && 
                gridY >= 0 && gridY < m_config.gridHeight) {
                m_countGrid[gridY][gridX]++;  // 计数
            }
        }
    }
    
    // 密度就是点的数量
    for (int y = 0; y < m_config.gridHeight; ++y) {
        for (int x = 0; x < m_config.gridWidth; ++x) {
            m_currentGrid[y][x] = static_cast<double>(m_countGrid[y][x]);
        }
    }
    
    return m_currentGrid;
}

QImage HeatmapGenerator::gridToImage(const std::vector<std::vector<double>> &grid)
{
    QImage image(m_config.gridWidth, m_config.gridHeight, QImage::Format_RGB32);
    
    // 找到非零值的最小值和最大值
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();
    
    for (const auto &row : grid) {
        for (double value : row) {
            if (value != 0.0) {
                minVal = std::min(minVal, value);
                maxVal = std::max(maxVal, value);
            }
        }
    }
    
    // 如果所有值都是0，返回黑色图像
    if (minVal == std::numeric_limits<double>::max()) {
        image.fill(Qt::black);
        return image;
    }
    
    // 生成图像
    for (int y = 0; y < m_config.gridHeight; ++y) {
        for (int x = 0; x < m_config.gridWidth; ++x) {
            double value = grid[y][x];
            QColor color;
            
            if (value == 0.0) {
                color = Qt::black;  // 无数据区域为黑色
            } else {
                color = interpolateColor(value, minVal, maxVal);
            }
            
            image.setPixel(x, y, color.rgb());
        }
    }
    
    return image;
}

// 新增功能实现

void HeatmapGenerator::updatePointCloudBatch(const std::vector<PointCloud> &pointClouds)
{
    if (pointClouds.empty()) {
        return;
    }
    
    qDebug() << QString("批量处理 %1 个点云数据").arg(pointClouds.size());
    
    // 生成批量网格
    std::vector<std::vector<double>> grid = generateGridFromBatch(pointClouds);
    
    // 计算统计信息
    calculateStatistics(grid);
    
    // 转换为图像
    m_currentHeatmap = gridToImage(grid);
    m_currentGrid = grid;
    
    // 发送更新信号
    emit heatmapUpdated(m_currentHeatmap);
    emit statisticsUpdated(m_statistics);
}

void HeatmapGenerator::setProcessingMode(bool enableBatchProcessing)
{
    m_batchProcessingMode = enableBatchProcessing;
    qDebug() << "处理模式已设置为:" << (enableBatchProcessing ? "批量处理" : "实时处理");
}

void HeatmapGenerator::setDataFilter(bool enableFilter)
{
    m_enableDataFilter = enableFilter;
    qDebug() << "数据过滤已" << (enableFilter ? "启用" : "禁用");
}

void HeatmapGenerator::setNoiseReduction(bool enableNoiseReduction)
{
    m_enableNoiseReduction = enableNoiseReduction;
    qDebug() << "噪声减少已" << (enableNoiseReduction ? "启用" : "禁用");
}

void HeatmapGenerator::setOutlierRemoval(bool enableOutlierRemoval)
{
    m_enableOutlierRemoval = enableOutlierRemoval;
    qDebug() << "异常值移除已" << (enableOutlierRemoval ? "启用" : "禁用");
}

PointCloud HeatmapGenerator::filterPointCloud(const PointCloud &pointCloud)
{
    if (!m_enableDataFilter) {
        return pointCloud;
    }
    
    PointCloud filteredPointCloud;
    
    for (const auto &scanLine : pointCloud) {
        std::vector<Point3D> filteredScanLine;
        
        for (const auto &point : scanLine) {
            if (isValidPoint(point)) {
                filteredScanLine.push_back(point);
            }
        }
        
        if (!filteredScanLine.empty()) {
            filteredPointCloud.push_back(filteredScanLine);
        }
    }
    
    qDebug() << QString("数据过滤: 原始点数 %1, 过滤后点数 %2")
                .arg(getTotalPointCount(pointCloud))
                .arg(getTotalPointCount(filteredPointCloud));
    
    return filteredPointCloud;
}

PointCloud HeatmapGenerator::removeOutliers(const PointCloud &pointCloud)
{
    if (!m_enableOutlierRemoval || pointCloud.empty()) {
        return pointCloud;
    }
    
    // 计算Z轴高度的统计信息
    std::vector<double> heights;
    for (const auto &scanLine : pointCloud) {
        for (const auto &point : scanLine) {
            heights.push_back(point.Z);
        }
    }
    
    if (heights.size() < 10) {
        return pointCloud;  // 数据太少，不进行异常值移除
    }
    
    // 计算均值和标准差
    double sum = 0.0;
    for (double height : heights) {
        sum += height;
    }
    double mean = sum / heights.size();
    
    double variance = 0.0;
    for (double height : heights) {
        variance += (height - mean) * (height - mean);
    }
    double stdDev = std::sqrt(variance / heights.size());
    
    // 移除异常值
    PointCloud filteredPointCloud;
    double threshold = m_outlierThreshold * stdDev;
    
    for (const auto &scanLine : pointCloud) {
        std::vector<Point3D> filteredScanLine;
        
        for (const auto &point : scanLine) {
            if (std::abs(point.Z - mean) <= threshold) {
                filteredScanLine.push_back(point);
            }
        }
        
        if (!filteredScanLine.empty()) {
            filteredPointCloud.push_back(filteredScanLine);
        }
    }
    
    qDebug() << QString("异常值移除: 原始点数 %1, 移除后点数 %2, 阈值: ±%3")
                .arg(heights.size())
                .arg(getTotalPointCount(filteredPointCloud))
                .arg(threshold);
    
    return filteredPointCloud;
}

PointCloud HeatmapGenerator::reduceNoise(const PointCloud &pointCloud)
{
    if (!m_enableNoiseReduction || pointCloud.empty()) {
        return pointCloud;
    }
    
    // 简单的噪声减少：对相邻点进行平均
    PointCloud denoisedPointCloud;
    
    for (const auto &scanLine : pointCloud) {
        if (scanLine.size() < 3) {
            denoisedPointCloud.push_back(scanLine);
            continue;
        }
        
        std::vector<Point3D> denoisedScanLine;
        
        // 第一个点保持不变
        denoisedScanLine.push_back(scanLine[0]);
        
        // 中间的点进行3点平均
        for (size_t i = 1; i < scanLine.size() - 1; ++i) {
            Point3D averagedPoint;
            averagedPoint.X = (scanLine[i-1].X + scanLine[i].X + scanLine[i+1].X) / 3.0;
            averagedPoint.Y = (scanLine[i-1].Y + scanLine[i].Y + scanLine[i+1].Y) / 3.0;
            averagedPoint.Z = (scanLine[i-1].Z + scanLine[i].Z + scanLine[i+1].Z) / 3.0;
            averagedPoint.RSSI = (scanLine[i-1].RSSI + scanLine[i].RSSI + scanLine[i+1].RSSI) / 3;
            
            denoisedScanLine.push_back(averagedPoint);
        }
        
        // 最后一个点保持不变
        if (scanLine.size() > 1) {
            denoisedScanLine.push_back(scanLine.back());
        }
        
        denoisedPointCloud.push_back(denoisedScanLine);
    }
    
    return denoisedPointCloud;
}

bool HeatmapGenerator::isValidPoint(const Point3D &point) const
{
    // 检查距离范围
    double distance = std::sqrt(point.X * point.X + point.Y * point.Y + point.Z * point.Z);
    if (distance < m_minDistance || distance > m_maxDistance) {
        return false;
    }
    
    // 检查强度范围
    if (point.RSSI < m_minIntensity || point.RSSI > m_maxIntensity) {
        return false;
    }
    
    // 检查坐标是否有效
    if (std::isnan(point.X) || std::isnan(point.Y) || std::isnan(point.Z)) {
        return false;
    }
    
    if (std::isinf(point.X) || std::isinf(point.Y) || std::isinf(point.Z)) {
        return false;
    }
    
    return true;
}

std::vector<std::vector<double>> HeatmapGenerator::generateGridFromBatch(const std::vector<PointCloud> &pointClouds)
{
    // 重置网格
    for (auto &row : m_currentGrid) {
        std::fill(row.begin(), row.end(), 0.0);
    }
    for (auto &row : m_countGrid) {
        std::fill(row.begin(), row.end(), 0);
    }
    
    if (pointClouds.empty()) {
        return m_currentGrid;
    }
    
    // 计算所有点云数据的范围
    if (m_config.autoRange) {
        double minX = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();
        
        for (const auto &pointCloud : pointClouds) {
            for (const auto &scanLine : pointCloud) {
                for (const auto &point : scanLine) {
                    minX = std::min(minX, static_cast<double>(point.X));
                    maxX = std::max(maxX, static_cast<double>(point.X));
                    minY = std::min(minY, static_cast<double>(point.Y));
                    maxY = std::max(maxY, static_cast<double>(point.Y));
                }
            }
        }
        
        // 添加边距
        double marginX = (maxX - minX) * 0.05;
        double marginY = (maxY - minY) * 0.05;
        
        m_config.minX = minX - marginX;
        m_config.maxX = maxX + marginX;
        m_config.minY = minY - marginY;
        m_config.maxY = maxY + marginY;
    }
    
    // 计算范围
    double rangeX = m_config.maxX - m_config.minX;
    double rangeY = m_config.maxY - m_config.minY;
    
    if (rangeX <= 0 || rangeY <= 0) {
        return m_currentGrid;
    }
    
    // 处理所有点云数据
    int totalPoints = 0;
    for (const auto &pointCloud : pointClouds) {
        // 数据预处理
        PointCloud processedPointCloud = pointCloud;
        
        if (m_enableDataFilter) {
            processedPointCloud = filterPointCloud(processedPointCloud);
        }
        
        if (m_enableOutlierRemoval) {
            processedPointCloud = removeOutliers(processedPointCloud);
        }
        
        if (m_enableNoiseReduction) {
            processedPointCloud = reduceNoise(processedPointCloud);
        }
        
        // 填充网格
        for (const auto &scanLine : processedPointCloud) {
            for (const auto &point : scanLine) {
                int gridX = static_cast<int>((point.X - m_config.minX) / rangeX * m_config.gridWidth);
                int gridY = static_cast<int>((point.Y - m_config.minY) / rangeY * m_config.gridHeight);
                
                if (gridX >= 0 && gridX < m_config.gridWidth && 
                    gridY >= 0 && gridY < m_config.gridHeight) {
                    
                    m_currentGrid[gridY][gridX] += point.Z;
                    m_countGrid[gridY][gridX]++;
                    totalPoints++;
                }
            }
        }
    }
    
    // 计算平均高度
    int validCells = 0;
    for (int y = 0; y < m_config.gridHeight; ++y) {
        for (int x = 0; x < m_config.gridWidth; ++x) {
            if (m_countGrid[y][x] > 0) {
                m_currentGrid[y][x] /= m_countGrid[y][x];
                validCells++;
            }
        }
    }
    
    qDebug() << QString("批量处理完成: 处理了 %1 个点云, 总点数 %2, 有效网格 %3")
                .arg(pointClouds.size()).arg(totalPoints).arg(validCells);
    
    return m_currentGrid;
}

// 辅助函数
int HeatmapGenerator::getTotalPointCount(const PointCloud &pointCloud) const
{
    int count = 0;
    for (const auto &scanLine : pointCloud) {
        count += scanLine.size();
    }
    return count;
}
