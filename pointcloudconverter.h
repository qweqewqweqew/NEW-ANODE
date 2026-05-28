#ifndef POINTCLOUDCONVERTER_H
#define POINTCLOUDCONVERTER_H

#include <QObject>
#include <QElapsedTimer>
#include "ThreeDPlatformDll.h"
#include "halcondatatypes.h"
#include "halconcpp.h"

using namespace HalconCpp;

// 点云统计信息
struct PointCloudStatistics {
    double minX, maxX, meanX;
    double minY, maxY, meanY;
    double minZ, maxZ, meanZ;
    int totalPoints;
    int validPoints;       // 有效点数（非0点）
    int invalidPoints;     // 无效点数（0点）
    
    PointCloudStatistics() {
        minX = minY = minZ = 999999;
        maxX = maxY = maxZ = -999999;
        meanX = meanY = meanZ = 0;
        totalPoints = validPoints = invalidPoints = 0;
    }
};

class PointCloudConverter : public QObject
{
    Q_OBJECT

public:
    explicit PointCloudConverter(QObject *parent = nullptr);
    ~PointCloudConverter();

    
    // 转换为Halcon图像（参考MFC项目）
    // 注意：无效点（x=0 && y=0 && z=0）在Halcon图像中也设为0
    bool convertToHalconImage(const PointCloud &sickPointCloud,
                              HalconImage3D &halconImage);
    
    // 转换为Halcon 3D对象模型
    bool convertToObjectModel3D(const PointCloud &sickPointCloud,
                                HTuple &objectModel3D);
    
    // 完整转换（图像 + 3D模型）
    bool convertComplete(const PointCloud &sickPointCloud,
                         HalconImage3D &halconImage);
    
    // ========== ROI过滤（参考MFC项目） ==========
    bool applyROIFilter(HalconImage3D &halconImage,
                        const HalconROIParams &roiParams);
    
    // ========== 配置函数 ==========
    void setROIParams(const HalconROIParams &params);
    void enableROI(bool enable);
    void enableNormalCalculation(bool enable);
    
    // ========== 统计函数 ==========
    static PointCloudStatistics calculateSickStatistics(const PointCloud &pointCloud);
    static PointCloudStatistics calculateHalconStatistics(const HalconImage3D &halconImage);
    static bool compareStatistics(const PointCloudStatistics &sick,
                                  const PointCloudStatistics &halcon,
                                  double tolerance = 0.01);
    static QString generateComparisonReport(const PointCloudStatistics &sick,
                                           const PointCloudStatistics &halcon);

signals:
    void conversionCompleted(const HalconImage3D &halconImage);
    void conversionFailed(const QString &error);
    void conversionProgress(int progress);

private:
    // 内部转换实现
    bool createXYZImages(const PointCloud &pointCloud,
                         HObject &hoX, HObject &hoY, HObject &hoZ,
                         int &width, int &height);
    
    bool calculateNormals(const HObject &hoX, const HObject &hoY,
                         const HObject &hoZ,
                         HObject &hoNX, HObject &hoNY, HObject &hoNZ);
    
    bool createObjectModel3D(const HObject &hoX, const HObject &hoY,
                            const HObject &hoZ, HTuple &objectModel);
    
    // 数据验证
    bool validatePointCloud(const PointCloud &pointCloud);
    bool validateHalconImage(const HalconImage3D &image);
    
    // 检查点是否为无效点（所有坐标都为0）
    inline bool isInvalidPoint(const Point3D &pt) const {
        return (pt.x == 0.0 && pt.y == 0.0 && pt.z == 0.0);
    }

private:
    HalconROIParams m_roiParams;
    bool m_enableNormals;
    bool m_enableROI;
};

#endif // POINTCLOUDCONVERTER_H

