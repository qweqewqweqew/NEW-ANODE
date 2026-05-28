#ifndef ANODEALGORITHMWORKER_H
#define ANODEALGORITHMWORKER_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QStringList>
#include "DataX.h"
#include "CUnloadPlateA.h"
#include "CHVisionAdvX.h"
#include "ThreeDPlatformData.h"
#include "halconcpp.h"

// 使用SICK SDK的数据结构
using PointCloud = Double3DPointVec;

/**
 * @brief 阳极板算法工作类（异步执行）
 * 在独立线程中执行算法，避免阻塞主线程
 */
class AnodeAlgorithmWorker : public QObject
{
    Q_OBJECT

public:
    explicit AnodeAlgorithmWorker(QObject *parent = nullptr);
    ~AnodeAlgorithmWorker();

public slots:
    /**
     * @brief 处理点云数据（从雷达扫描获取）
     * @param pointCloud 点云数据
     */
    void processPointCloud(const PointCloud &pointCloud);
    void processFinePositioning(int blockIndex, const QStringList &imageBasePaths);
    void processTestDualCameraMemory(const s_Image3dS &img1, const s_Image3dS &img2);

    /**
     * @brief 处理本地文件（从文件读取）
     * @param filePath 文件路径（仅支持.ply格式）
     * @param mapWidth 图像宽度（用于转换，默认1201）
     */
    void processLocalFile(const QString &filePath, int mapWidth = 1201);

    /**
     * @brief 更新安全间距参数（与 MFC 1119 版保持一致，可由 UI 配置）
     * @param tbNear 铜跺之间的安全间距
     * @param toCbOuter 铜跺到车板外边界的安全间距
     */
    void setSafeDistanceParameters(double tbNear, double toCbOuter);
    
    /**
     * @brief 设置相机拍照位参数
     * @param cameraX X坐标
     * @param cameraY Y坐标
     * @param cameraZ Z坐标
     * @param cameraDeg 旋转角度
     */
    void setCameraPositionParameters(double cameraX, double cameraY, double cameraZ, double cameraDeg);
    
    /**
     * @brief 测试模式：处理双相机数据（按照MFC流程）
     * @param imageBasePaths 两个相机的图像基础路径（必须是2个）
     */
    void processTestDualCamera(const QStringList &imageBasePaths);

signals:
    /**
     * @brief 处理完成信号
     * @param result 检测结果（包含所有缺陷区域信息）
     * @param processedImage 预处理后的雷达图像（s_Lidar3d格式，用于显示基础Z图像）
     */
    void processingFinished(const s_PreADPlateARtsPara &result,
                            const s_Lidar3d &processedImage,
                            bool preAlignValid,
                            const s_CalcPreAlignRtsPara &preAlignResult,
                            const QString &preAlignError);

    /**
     * @brief 处理失败信号
     * @param error 错误信息
     */
    void processingFailed(const QString &error);

    /**
     * @brief 处理进度信号（可选，用于显示进度）
     * @param progress 进度百分比 (0-100)
     * @param message 进度消息
     */
    void processingProgress(int progress, const QString &message);
    void finePositioningFinished(int blockIndex,
                                 const s_AccurateADPlateARtsPara &result,
                                 const s_CalcAccurateAlignRtsPara &alignResult);
    void finePositioningFailed(int blockIndex, const QString &error);
    
    /**
     * @brief 测试模式双相机处理完成信号
     */
    void testDualCameraFinished(const s_AccurateADPlateARtsPara &detectResult,
                                const s_CalcAccurateAlignRtsPara &calcResult,
                                const s_CalcAccurateAlignPara &calcPara);
    void testDualCameraFailed(const QString &error);
    void testDualCameraProgress(const QString &message);

private:
    /**
     * @brief 从PCD文件读取宽度
     * @param filePath 文件路径
     * @return 宽度值，如果读取失败返回-1
     */
    int readWidthFromPCD(const QString &filePath);

    /**
     * @brief 从点云数据计算宽度（如果点云是规则排列的）
     * @param pointCloud 点云数据
     * @return 宽度值，如果无法计算返回-1
     */
    int calculateWidthFromPointCloud(const PointCloud &pointCloud);

    /**
     * @brief 将点云转换为算法格式
     * @param pointCloud 输入点云
     * @param lidar3d 输出格式
     * @param errorMsg 输出错误信息（如果失败）
     * @return 是否成功
     */
    bool convertPointCloudToLidar3d(const PointCloud &pointCloud, s_Lidar3d &lidar3d, QString &errorMsg);

    /**
     * @brief 执行算法检测
     * @param lidar3d 输入数据（非const，因为需要调用非const方法）
     * @param result 输出结果
     * @param errorMsg 输出错误信息（如果失败）
     * @return 是否成功
     */
    bool executeAlgorithm(s_Lidar3d &lidar3d, s_PreADPlateARtsPara &result, QString &errorMsg);
    // 精定位：直接使用内存中的图像数据
    Q_INVOKABLE void processFinePositioningMemory(int blockIndex, const QList<s_Image3dS> &images);
    bool computeCameraPhotoPositions(const s_PreADPlateARtsPara &result,
                                     s_CalcPreAlignRtsPara &preAlignResult,
                                     QString &errorMsg);
    bool loadImage3dFromBasePath(const QString &basePath, s_Image3dS &image3d, QString &errorMsg);
    void initializeAccurateParameters();
    void initializeCalcPreAlignParameters();
    QString decodeAlgorithmMessage(const std::string &info) const;

private:
    CUnloadPlateA *m_algorithm;
    CHVisionAdvX *m_converter;
    s_PreProcessLidar3dPara m_preProcessPara;  // 预处理参数（使用默认值）
    s_PreADPlateAPara m_detectPara;            // 检测参数（使用默认值）
    s_LidarTrans m_lidarTrans;                 // 坐标系变换参数（使用默认值）
    s_CalcPreAlignPara m_calcPreAlignPara;     // 雷达定位计算参数（与算法保持一致）
    s_AccurateADPlateAPara m_accuratePara;     // 精定位检测参数
    s_CalcAccurateAlignPara m_calcAccuratePara;// 精定位对位参数

    double m_safeDistanceTbNear;               // 动态安全间距（铜跺之间）
    double m_safeDistanceToCbOuter;            // 动态安全间距（铜跺到车板边缘）
};

#endif // ANODEALGORITHMWORKER_H


