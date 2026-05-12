#include "anodealgorithmworker.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDateTime>
#include <windows.h>
#include <cmath>
#include <limits>
#include <QElapsedTimer>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif

AnodeAlgorithmWorker::AnodeAlgorithmWorker(QObject *parent)
    : QObject(parent)
    , m_algorithm(new CUnloadPlateA())
    , m_converter(new CHVisionAdvX())
    , m_safeDistanceTbNear(350.0)
    , m_safeDistanceToCbOuter(400.0)
{
    qDebug() << "[AnodeAlgorithmWorker]初始化算法工作线程";

    // 初始化Halcon环境
    using namespace HalconCpp;
    try {
        // 设置Halcon环境
        SetHcppInterfaceStringEncodingIsUtf8(false);  // 使用GBK编码（算法文件是GBK）
        SetSystem("clip_region", "false");  // 边界超出依然有效

        qDebug() << "[AnodeAlgorithmWorker] Halcon环境初始化完成";
        qDebug() << "[AnodeAlgorithmWorker]  clip_region: false";

        // 验证设置
        HTuple hv_ClipRegion;
        GetSystem("clip_region", &hv_ClipRegion);
        qDebug() << "[AnodeAlgorithmWorker]   - 验证clip_region:" << QString::fromLocal8Bit(hv_ClipRegion[0].S().Text());

    } catch (const HException &ex) {
        qDebug() << "[AnodeAlgorithmWorker] Halcon初始化异常:" << ex.ErrorMessage().Text();
    }

    // 使用默认参数（算法文件中定义的默认值）
    // s_PreProcessLidar3dPara, s_PreADPlateAPara, s_LidarTrans 的构造函数已设置默认值

    // 设置坐标系变换参数
    m_lidarTrans.Y_Z_Change = true;   // Y 和 Z 交换
    m_lidarTrans.X_Y_Change = false;  // X 和 Y 不交换
    m_lidarTrans.X_mirro = false;     // X 不镜像
    m_lidarTrans.Y_mirro = false;     // Y 不镜像
    m_lidarTrans.Map_Rotate = false;  // 不旋转

    // 覆盖算法参数（来自“雷达算法和参数更新1213”）
    // 预处理参数
    m_preProcessPara.fRoiXMin = -10000;
    m_preProcessPara.fRoiXMax = 11500;
    m_preProcessPara.fRoiYMin = -2000;
    m_preProcessPara.fRoiYMax = 2000;
    m_preProcessPara.fRoiZMin = 3500;
    m_preProcessPara.fRoiZMax = 7000;
    m_preProcessPara.fThSeg0Dis = 200;
    m_preProcessPara.iThSeg0NumPtsMin = 20;
    m_preProcessPara.iThSeg0NumPtsMax = 999999;
    m_preProcessPara.fThSeg0DiameterMin = 200;
    m_preProcessPara.fThSeg0DiameterMax = 999999;
    m_preProcessPara.ProcessModel = 0;
    m_preProcessPara.fProSetResolutionX = 20;
    m_preProcessPara.fProSetResolutionY = 10;

    // 检测参数（铜跺筛选与缺陷检测）
    m_detectPara.fThNZAbsVal = 0.8;
    m_detectPara.fThSegCbDis = 80;
    m_detectPara.fThCbSelLMin = 3000;
    m_detectPara.fThCbSelLMax = 22000;
    m_detectPara.fThCbSelWMin = 800;
    m_detectPara.fThCbSelWMax = 5000;
    m_detectPara.fThCbSelPtsMin = 2000;
    m_detectPara.fThCbSelPtsMax = 180000;
    m_detectPara.fThCbSelZMin = 6500;
    m_detectPara.fThCbSelZMax = 7200;

    // 空间异物
    m_detectPara.bDefectSpaceObj = true;
    m_detectPara.fDefectSpaceThZMin = 900;
    m_detectPara.fDefectSpaceThZMax = 3000;
    m_detectPara.fDefectSpaceThInnerLen = 150;
    m_detectPara.fDefectSpaceThInnerWidth = 300;
    m_detectPara.fDefectSpaceThLen = 500.0;
    m_detectPara.fDefectSpaceThNumPts = 10;

    // 边缘缺陷
    m_detectPara.bDefectLbObj = true;
    m_detectPara.fDefectLbThZMin = 300;
    m_detectPara.fDefectLbThZMax = 1600;
    m_detectPara.fDefectLbThick = 300;
    m_detectPara.fDefectLbThInnerLen = 200;
    m_detectPara.fDefectLbThInnerWidth = 100;
    m_detectPara.fDefectLbThLen = 2000.0;
    m_detectPara.fDefectLbThWidth = 1000.0;
    m_detectPara.fDefectLbThNumPts = 100;

    // 铜跺筛选（耳朵、采样等）
    m_detectPara.fThSegTbDis = 60;
    m_detectPara.fThTbSelLMin = 800;
    m_detectPara.fThTbSelLMax = 1500;
    m_detectPara.fThTbSelWMin = 800;
    m_detectPara.fThTbSelWMax = 1500;
    m_detectPara.fThTbSelPtsMin = 1000;
    m_detectPara.fThTbSelPtsMax = 17000;
    m_detectPara.fThTbSelZMin = 5900;
    m_detectPara.fThTbSelZMax = 6450;
    m_detectPara.fJgTbCloseLenMin = 2000;
    m_detectPara.fJgTbClosePtsMin = 4500;
    m_detectPara.fSampleTb = 80;

    initializeCalcPreAlignParameters();
    initializeAccurateParameters();

    qDebug() << "[AnodeAlgorithmWorker] 算法参数初始化完成";
}

AnodeAlgorithmWorker::~AnodeAlgorithmWorker()
{
    delete m_algorithm;
    delete m_converter;
}

void AnodeAlgorithmWorker::setSafeDistanceParameters(double tbNear, double toCbOuter)
{
    m_safeDistanceTbNear = tbNear;
    m_safeDistanceToCbOuter = toCbOuter;
}

void AnodeAlgorithmWorker::setCameraPositionParameters(double cameraX, double cameraY, double cameraZ, double cameraDeg)
{
    m_calcAccuratePara.fCameraX = cameraX;
    m_calcAccuratePara.fCameraY = cameraY;
    m_calcAccuratePara.fCameraZ = cameraZ;
    m_calcAccuratePara.fCameraDeg = cameraDeg;
    qDebug() << "[AnodeAlgorithmWorker] 相机拍照位参数更新:"
             << "X=" << cameraX << "Y=" << cameraY << "Z=" << cameraZ << "Deg=" << cameraDeg;
}

void AnodeAlgorithmWorker::processPointCloud(const PointCloud &pointCloud)
{
    emit processingProgress(10, "开始处理点云数据...");

    // 检查点云是否为空
    if (pointCloud.empty() || pointCloud[0].empty()) {
        emit processingFailed("点云数据为空");
        return;
    }

    emit processingProgress(20, "转换点云数据格式...");

    // 转换为算法格式
    s_Lidar3d lidar3d;
    QString convertError;
    if (!convertPointCloudToLidar3d(pointCloud, lidar3d, convertError)) {
        qDebug() << "点云数据转换失败:" << convertError;
        emit processingFailed("点云数据转换失败");
        return;
    }

    emit processingProgress(40, "执行算法检测...");

    // 执行算法
    s_PreADPlateARtsPara result;
    QString algorithmError;
    if (!executeAlgorithm(lidar3d, result, algorithmError)) {
        qDebug() << "算法检测失败:" << algorithmError;
        emit processingFailed("算法检测失败");
        return;
    }

    emit processingProgress(100, "检测完成");

    s_CalcPreAlignRtsPara preAlignResult;
    preAlignResult.Reset();
    QString preAlignError;
    bool preAlignValid = computeCameraPhotoPositions(result, preAlignResult, preAlignError);
    if (!preAlignValid) {
        qDebug() << "雷达定位计算失败:" << preAlignError;
    }

    // 发送完成信号
    emit processingFinished(result, lidar3d, preAlignValid, preAlignResult, preAlignError);
}

void AnodeAlgorithmWorker::processFinePositioningMemory(int blockIndex, const QList<s_Image3dS> &images)
{
    if (images.isEmpty()) {
        emit finePositioningFailed(blockIndex, QStringLiteral("未提供相机数据"));
        return;
    }

    std::vector<s_Image3dS> vec(images.begin(), images.end());

    s_AccurateADPlateARtsPara accurateResult;
    s_Rtnf accurateStatus = m_algorithm->OnAccurateAD(vec, m_accuratePara, accurateResult);
    if (accurateStatus.iCode != 0) {
        QString msg = QString::fromStdString(accurateStatus.strInfo);
        if (msg.isEmpty()) {
            msg = QStringLiteral("相机精定位检测失败");
        }
        emit finePositioningFailed(blockIndex, msg);
        return;
    }

    s_CalcAccurateAlignRtsPara alignResult;
    s_Rtnf alignStatus = m_algorithm->CalcAccurateAlignRts(accurateResult, m_calcAccuratePara, alignResult);
    if (alignStatus.iCode != 0) {
        QString msg = QString::fromStdString(alignStatus.strInfo);
        if (msg.isEmpty()) {
            msg = QStringLiteral("相机对位计算失败");
        }
        emit finePositioningFailed(blockIndex, msg);
        return;
    }

    emit finePositioningFinished(blockIndex, accurateResult, alignResult);
}

void AnodeAlgorithmWorker::processFinePositioning(int blockIndex, const QStringList &imageBasePaths)
{
    if (imageBasePaths.isEmpty()) {
        emit finePositioningFailed(blockIndex, QStringLiteral("未提供相机数据路径"));
        return;
    }

    std::vector<s_Image3dS> images;
    images.reserve(imageBasePaths.size());

    for (const QString &basePath : imageBasePaths) {
        s_Image3dS image;
        QString errorMsg;
        if (!loadImage3dFromBasePath(basePath, image, errorMsg)) {
            emit finePositioningFailed(blockIndex, errorMsg);
            return;
        }
        images.push_back(image);
    }

    s_AccurateADPlateARtsPara accurateResult;
    s_Rtnf accurateStatus = m_algorithm->OnAccurateAD(images, m_accuratePara, accurateResult);
    if (accurateStatus.iCode != 0) {
        QString msg = QString::fromStdString(accurateStatus.strInfo);
        if (msg.isEmpty()) {
            msg = QStringLiteral("相机精定位检测失败");
        }
        emit finePositioningFailed(blockIndex, msg);
        return;
    }

    s_CalcAccurateAlignRtsPara alignResult;
    s_Rtnf alignStatus = m_algorithm->CalcAccurateAlignRts(accurateResult, m_calcAccuratePara, alignResult);
    if (alignStatus.iCode != 0) {
        QString msg = QString::fromStdString(alignStatus.strInfo);
        if (msg.isEmpty()) {
            msg = QStringLiteral("相机对位计算失败");
        }
        emit finePositioningFailed(blockIndex, msg);
        return;
    }

    emit finePositioningFinished(blockIndex, accurateResult, alignResult);
}

// 辅助函数：检测路径是否包含非ASCII字符（中文等）
static bool containsNonAscii(const QString &path)
{
    for (int i = 0; i < path.length(); ++i) {
        QChar ch = path[i];
        if (ch.unicode() > 127) {
            return true;
        }
    }
    return false;
}

// 辅助函数：将长路径转换为短路径（8.3格式），解决中文路径问题
// 返回：成功返回短路径，失败返回空字符串
static QString convertToShortPath(const QString &longPath, bool &success)
{
    success = false;
    std::wstring wLongPath = longPath.toStdWString();
    wchar_t shortPath[MAX_PATH];
    DWORD result = GetShortPathNameW(wLongPath.c_str(), shortPath, MAX_PATH);

    if (result == 0 || result >= MAX_PATH) {
        DWORD error = GetLastError();
        qDebug() << "获取短路径失败，错误代码:" << error;
        if (error == ERROR_FILE_NOT_FOUND) {
            qDebug() << "文件不存在，无法获取短路径";
        } else if (error == ERROR_PATH_NOT_FOUND) {
            qDebug() << "路径不存在，无法获取短路径";
        } else if (error == ERROR_NOT_READY) {
            qDebug() << "卷未就绪，无法获取短路径";
        }
        return QString();
    }

    QString shortPathStr = QString::fromStdWString(std::wstring(shortPath));

    // 验证短路径是否真的不同（如果相同，说明已经是短路径或转换失败）
    if (shortPathStr.compare(longPath, Qt::CaseInsensitive) == 0) {
        qDebug() << "短路径与原路径相同，可能系统未启用8.3格式";
        return QString();
    }

    success = true;
    qDebug() << "路径转换成功 - 长路径:" << longPath;
    qDebug() << "路径转换成功 - 短路径:" << shortPathStr;
    return shortPathStr;
}

// 辅助函数：复制文件到临时目录（ASCII路径）
// 返回：成功返回临时文件路径，失败返回空字符串
static QString copyToTempDirectory(const QString &sourcePath)
{
    QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists()) {
        qDebug() << "源文件不存在，无法复制:" << sourcePath;
        return QString();
    }

    // 获取系统临时目录（通常是ASCII路径）
    QString tempDir = QDir::tempPath();
    qDebug() << "系统临时目录:" << tempDir;

    // 生成唯一的临时文件名（使用时间戳和随机数）
    QString fileName = sourceInfo.fileName();
    QString baseName = sourceInfo.completeBaseName();
    QString suffix = sourceInfo.suffix();

    // 确保文件名只包含ASCII字符（移除非ASCII字符）
    QString safeBaseName;
    for (int i = 0; i < baseName.length(); ++i) {
        QChar ch = baseName[i];
        if (ch.unicode() <= 127 && (ch.isLetterOrNumber() || ch == '_' || ch == '-')) {
            safeBaseName.append(ch);
        } else {
            safeBaseName.append('_');
        }
    }
    if (safeBaseName.isEmpty()) {
        safeBaseName = "temp_ply";
    }

    QString tempFileName = QString("%1_%2.%3")
                              .arg(safeBaseName)
                              .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"))
                              .arg(suffix);
    QString tempFilePath = QDir(tempDir).filePath(tempFileName);

    qDebug() << "准备复制文件到临时目录:";
    qDebug() << "  源文件:" << sourcePath;
    qDebug() << "  目标文件:" << tempFilePath;

    // 复制文件
    QFile sourceFile(sourcePath);
    if (!sourceFile.copy(tempFilePath)) {
        QString error = sourceFile.errorString();
        qDebug() << "文件复制失败:" << error;
        return QString();
    }

    qDebug() << "文件复制成功";
    return tempFilePath;
}

void AnodeAlgorithmWorker::processLocalFile(const QString &filePath, int mapWidth)
{
    qDebug() << "   = 开始处理本地雷达文件    =";
    qDebug() << "文件路径:" << filePath;
    qDebug() << "图像宽度:" << mapWidth;

    emit processingProgress(10, QString("读取文件: %1").arg(filePath));

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QString error = QString("文件不存在: %1").arg(filePath);
        qDebug() << "错误:" << error;
        emit processingFailed(error);
        return;
    }

    // 根据文件扩展名选择读取方式
    QString suffix = fileInfo.suffix().toLower();
    qDebug() << "文件扩展名:" << suffix;

    // 算法只支持PLY格式（Halcon的ReadObjectModel3d虽然可能支持其他格式，但算法测试中只使用了PLY）
    if (suffix != "ply") {
        QString error = QString("算法只支持PLY格式的点云文件，当前文件格式: %1。请将文件转换为PLY格式后再试。").arg(suffix);
        qDebug() << "错误:" << error;
        emit processingFailed(error);
        return;
    }

    // 验证宽度值
    if (mapWidth <= 0) {
        QString error = "图像宽度必须大于0，请检查输入值";
        qDebug() << "错误:" << error;
        emit processingFailed(error);
        return;
    }

    // 读取PLY格式文件
    emit processingProgress(20, QString("读取PLY点云文件（宽度: %1）...").arg(mapWidth));
    qDebug() << "开始读取PLY文件...";

    // 在try块外定义，以便在catch块中也能访问
    QString finalPath = filePath;  // 最终使用的路径
    QString tempFilePath;          // 临时文件路径（如果需要清理）
    bool needCleanup = false;      // 是否需要清理临时文件

        try {
            // 使用 CHVisionAdvX::ReadLidarDevDataX 读取文件
            // 注意：算法代码使用 ANSI API (GetFileAttributesA)，不支持中文路径
            // 解决方案4（混合方案）：先尝试短路径，失败则复制到临时目录
            Double3DPointVec d3dPoints;

            // 检测路径是否包含非ASCII字符
            bool hasNonAscii = containsNonAscii(filePath);

            if (hasNonAscii) {
                qDebug() << "检测到路径包含非ASCII字符，需要路径转换";
                qDebug() << "原始路径:" << filePath;

                // 策略1：尝试使用短路径（快速方案）
                bool shortPathSuccess = false;
                QString shortPath = convertToShortPath(filePath, shortPathSuccess);

                if (shortPathSuccess && !shortPath.isEmpty()) {
                    finalPath = shortPath;
                    qDebug() << "策略1成功：使用短路径";
                } else {
                    // 策略2：复制到临时目录（可靠方案）
                    qDebug() << "策略1失败，尝试策略2：复制文件到临时目录";
                    tempFilePath = copyToTempDirectory(filePath);

                    if (!tempFilePath.isEmpty()) {
                        finalPath = tempFilePath;
                        needCleanup = true;
                        qDebug() << "策略2成功：文件已复制到临时目录";
                    } else {
                        QString error = QString("无法处理中文路径：短路径获取失败，文件复制也失败。路径: %1").arg(filePath);
                        qDebug() << "错误:" << error;
                        emit processingFailed(error);
                        return;
                    }
                }
            } else {
                qDebug() << "路径只包含ASCII字符，直接使用原路径";
            }

            // 将最终路径转换为 std::string（使用本地代码页）
            std::wstring wstr = finalPath.toStdWString();
            int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
            if (size_needed <= 0) {
                QString error = QString("路径转换失败: %1").arg(finalPath);
                qDebug() << "错误:" << error;
                if (needCleanup && !tempFilePath.isEmpty()) {
                    QFile::remove(tempFilePath);
                    qDebug() << "已清理临时文件:" << tempFilePath;
                }
                emit processingFailed(error);
                return;
            }
            std::string strTo(size_needed, 0);
            WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &strTo[0], size_needed, NULL, NULL);
            std::string filePathStd = std::string(strTo.c_str(), size_needed - 1); // -1 去掉末尾的 \0

            qDebug() << "路径处理结果";
            qDebug() << "原始文件路径 (UTF-8):" << filePath;
            qDebug() << "最终使用路径 (UTF-16):" << finalPath;
            qDebug() << "传递给算法的路径 (ANSI):" << filePathStd.c_str();
            qDebug() << "是否需要清理临时文件:" << needCleanup;
            qDebug() << "调用 ReadLidarDevDataX，宽度:" << mapWidth;

            // 验证文件是否存在（使用 Qt 的 API，支持 UTF-8）
            QFileInfo fileCheck(finalPath);
            if (!fileCheck.exists()) {
                QString error = QString("文件不存在: %1").arg(finalPath);
                qDebug() << "错误:" << error;
                if (needCleanup && !tempFilePath.isEmpty()) {
                    QFile::remove(tempFilePath);
                    qDebug() << "已清理临时文件:" << tempFilePath;
                }
                emit processingFailed(error);
                return;
            }
            qDebug() << "文件存在验证通过";

            int result = m_converter->ReadLidarDevDataX(d3dPoints, mapWidth, filePathStd);

            qDebug() << "ReadLidarDevDataX 返回结果:" << result;
            qDebug() << "读取的点云数据行数:" << d3dPoints.size();
            if (!d3dPoints.empty()) {
                qDebug() << "第一行点数:" << d3dPoints[0].size();
            }

            if (result != 0) {
                QString error = QString("读取文件失败，错误代码: %1").arg(result);
                qDebug() << "错误:" << error;
                // 清理临时文件（如果使用了）
                if (needCleanup && !tempFilePath.isEmpty()) {
                    if (QFile::remove(tempFilePath)) {
                        qDebug() << "已清理临时文件:" << tempFilePath;
                    } else {
                        qDebug() << "警告：临时文件清理失败:" << tempFilePath;
                    }
                }
                emit processingFailed(error);
                return;
            }

            if (d3dPoints.empty() || d3dPoints[0].empty()) {
                QString error = "文件数据为空";
                qDebug() << "错误:" << error;
                qDebug() << "点云数据行数:" << d3dPoints.size();
                if (!d3dPoints.empty()) {
                    qDebug() << "第一行点数:" << d3dPoints[0].size();
                }
                // 清理临时文件（如果使用了）
                if (needCleanup && !tempFilePath.isEmpty()) {
                    if (QFile::remove(tempFilePath)) {
                        qDebug() << "已清理临时文件:" << tempFilePath;
                    } else {
                        qDebug() << "警告：临时文件清理失败:" << tempFilePath;
                    }
                }
                emit processingFailed(error);
                return;
            }

            // 文件读取成功，数据已加载到内存，可以清理临时文件
            if (needCleanup && !tempFilePath.isEmpty()) {
                if (QFile::remove(tempFilePath)) {
                    qDebug() << "临时文件已清理（文件已成功读取到内存）:" << tempFilePath;
                    needCleanup = false; // 标记已清理，避免后续重复清理
                } else {
                    qDebug() << "警告：临时文件清理失败:" << tempFilePath;
                }
            }

            emit processingProgress(40, "转换点云数据格式...");
            qDebug() << "开始转换点云数据格式...";

            // 检查点云数据的 Z 值范围
            if (!d3dPoints.empty() && !d3dPoints[0].empty()) {
                double minZ = d3dPoints[0][0].Z;
                double maxZ = d3dPoints[0][0].Z;
                int validPointCount = 0;
                int invalidPointCount = 0;

                for (size_t i = 0; i < d3dPoints.size(); i++) {
                    for (size_t j = 0; j < d3dPoints[i].size(); j++) {
                        double z = d3dPoints[i][j].Z;
                        if (std::isnan(z) || std::isinf(z)) {
                            invalidPointCount++;
                        } else {
                            validPointCount++;
                            if (z < minZ) minZ = z;
                            if (z > maxZ) maxZ = z;
                        }
                    }
                }
                qDebug() << "点云数据 Z 值范围 - 最小值:" << minZ << "最大值:" << maxZ;
                qDebug() << "点云数据统计 - 有效点数:" << validPointCount << "无效点数:" << invalidPointCount;
            }

            // 转换为算法格式
            s_Lidar3d lidar3d;
            QString convertError;
            if (!convertPointCloudToLidar3d(d3dPoints, lidar3d, convertError)) {
                qDebug() << "点云数据转换失败:" << convertError;
                // 清理临时文件（如果使用了）
                if (needCleanup && !tempFilePath.isEmpty()) {
                    QFile::remove(tempFilePath);
                    qDebug() << "已清理临时文件:" << tempFilePath;
                }
                emit processingFailed(QString("点云数据转换失败: %1").arg(convertError));
                return;
            }
            qDebug() << "点云数据转换成功";

            emit processingProgress(60, "执行算法检测...");
            qDebug() << "开始执行算法检测...";

            // 执行算法
            s_PreADPlateARtsPara detectResult;
            QString algorithmError;
            if (!executeAlgorithm(lidar3d, detectResult, algorithmError)) {
                qDebug() << "算法检测失败:" << algorithmError;
                // 清理临时文件（如果使用了）
                if (needCleanup && !tempFilePath.isEmpty()) {
                    QFile::remove(tempFilePath);
                    qDebug() << "已清理临时文件:" << tempFilePath;
                }
                emit processingFailed(QString("算法检测失败: %1").arg(algorithmError));
                return;
            }
            qDebug() << "算法检测成功";

            emit processingProgress(100, "检测完成");
            qDebug() << "   = 处理完成    =";

            s_CalcPreAlignRtsPara preAlignResult;
            preAlignResult.Reset();
            QString preAlignError;
            bool preAlignValid = computeCameraPhotoPositions(detectResult, preAlignResult, preAlignError);
            if (!preAlignValid) {
                qDebug() << "雷达定位计算失败:" << preAlignError;
            }

            // 发送完成信号
            emit processingFinished(detectResult, lidar3d, preAlignValid, preAlignResult, preAlignError);

        } catch (const std::exception &e) {
            QString error = QString("处理文件时发生异常: %1").arg(e.what());
            qDebug() << "异常:" << error;
            // 清理临时文件（如果使用了）
            if (needCleanup && !tempFilePath.isEmpty()) {
                if (QFile::remove(tempFilePath)) {
                    qDebug() << "异常处理：临时文件已清理:" << tempFilePath;
                } else {
                    qDebug() << "警告：异常处理时临时文件清理失败:" << tempFilePath;
                }
            }
            emit processingFailed(error);
        } catch (...) {
            QString error = "处理文件时发生未知异常";
            qDebug() << "未知异常:" << error;
            // 清理临时文件（如果使用了）
            if (needCleanup && !tempFilePath.isEmpty()) {
                if (QFile::remove(tempFilePath)) {
                    qDebug() << "异常处理：临时文件已清理:" << tempFilePath;
                } else {
                    qDebug() << "警告：异常处理时临时文件清理失败:" << tempFilePath;
                }
            }
            emit processingFailed(error);
        }
}

int AnodeAlgorithmWorker::readWidthFromPCD(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return -1;
    }

    QTextStream in(&file);
    QString line;
    int width = -1;
    int height = -1;

    // 读取PCD文件头，查找WIDTH和HEIGHT行
    while (!in.atEnd()) {
        line = in.readLine().trimmed();

        // 跳过注释行
        if (line.startsWith("#")) {
            continue;
        }

        // 查找WIDTH行
        if (line.startsWith("WIDTH", Qt::CaseInsensitive)) {
            QRegularExpression re(R"(WIDTH\s+(\d+))", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                bool ok;
                width = match.captured(1).toInt(&ok);
                if (!ok || width <= 0) {
                    width = -1;
                }
            }
        }

        // 查找HEIGHT行
        if (line.startsWith("HEIGHT", Qt::CaseInsensitive)) {
            QRegularExpression re(R"(HEIGHT\s+(\d+))", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                bool ok;
                height = match.captured(1).toInt(&ok);
                if (!ok || height <= 0) {
                    height = -1;
                }
            }
        }

        // 如果遇到DATA行，说明文件头结束，停止查找
        if (line.startsWith("DATA", Qt::CaseInsensitive)) {
            break;
        }
    }

    file.close();

    // 如果HEIGHT > 1，说明WIDTH是每行的点数，这就是我们需要的mapWidth
    // 如果HEIGHT = 1，说明WIDTH是总点数，无法直接确定mapWidth
    if (width > 0 && height > 1) {
        return width;  // 每行的点数
    } else if (width > 0 && height == 1) {
        // HEIGHT=1时，WIDTH是总点数，无法确定原始扫描宽度
        // 返回-1表示无法确定
        return -1;
    }

    return -1;  // 未找到宽度信息
}

int AnodeAlgorithmWorker::calculateWidthFromPointCloud(const PointCloud &pointCloud)
{
    if (pointCloud.empty() || pointCloud[0].empty()) {
        return -1;
    }

    // 如果点云是规则排列的（所有行的点数相同），返回第一行的点数
    int firstRowSize = pointCloud[0].size();
    bool isRegular = true;

    for (size_t i = 1; i < pointCloud.size(); i++) {
        if (pointCloud[i].size() != firstRowSize) {
            isRegular = false;
            break;
        }
    }

    if (isRegular && firstRowSize > 0) {
        return firstRowSize;
    }

    return -1;  // 点云不是规则排列，无法确定宽度
}

bool AnodeAlgorithmWorker::convertPointCloudToLidar3d(const PointCloud &pointCloud, s_Lidar3d &lidar3d, QString &errorMsg)
{
    try {
        // 使用 CHVisionAdvX::TransLidarDevData2Lidar3ds 进行转换
        int result = m_converter->TransLidarDevData2Lidar3ds(pointCloud, m_lidarTrans, lidar3d);

        if (result != 0) {
            errorMsg = QString("转换函数返回错误代码: %1").arg(result);
            qDebug() << "点云转换失败，错误代码:" << result;
            qDebug() << "点云数据行数:" << pointCloud.size();
            if (!pointCloud.empty()) {
                qDebug() << "第一行点数:" << pointCloud[0].size();
            }
            return false;
        }

        return true;
    } catch (const std::exception &e) {
        errorMsg = QString("转换异常: %1").arg(e.what());
        qDebug() << "点云转换异常:" << e.what();
        return false;
    } catch (...) {
        errorMsg = "转换时发生未知异常";
        qDebug() << "点云转换时发生未知异常";
        return false;
    }
}

bool AnodeAlgorithmWorker::executeAlgorithm(s_Lidar3d &lidar3d, s_PreADPlateARtsPara &result, QString &errorMsg)
{
    try {
        // 检查输入数据
        int mapWidth = lidar3d.GetMapWidth();
        int mapHeight = lidar3d.GetMapHeight();
        qDebug() << "算法输入数据 - 宽度:" << mapWidth << "高度:" << mapHeight;

        // 检查 Halcon 对象是否初始化
        bool xInitialized = lidar3d.X.IsInitialized();
        bool yInitialized = lidar3d.Y.IsInitialized();
        bool zInitialized = lidar3d.Z.IsInitialized();
        qDebug() << "Halcon 对象初始化状态 - X:" << xInitialized << "Y:" << yInitialized << "Z:" << zInitialized;

        if (xInitialized) {
            qDebug() << "X 图像对象数量:" << lidar3d.X.CountObj();
        }
        if (yInitialized) {
            qDebug() << "Y 图像对象数量:" << lidar3d.Y.CountObj();
        }
        if (zInitialized) {
            qDebug() << "Z 图像对象数量:" << lidar3d.Z.CountObj();
        }

        if (mapWidth <= 0 || mapHeight <= 0) {
            errorMsg = QString("输入数据无效: 宽度=%1, 高度=%2").arg(mapWidth).arg(mapHeight);
            qDebug() << "算法输入数据无效:" << errorMsg;
            return false;
        }

        if (!xInitialized || !yInitialized || !zInitialized) {
            errorMsg = QString("Halcon 图像对象未正确初始化: X=%1, Y=%2, Z=%3")
                          .arg(xInitialized ? "是" : "否")
                          .arg(yInitialized ? "是" : "否")
                          .arg(zInitialized ? "是" : "否");
            qDebug() << "错误:" << errorMsg;
            return false;
        }

        // 必须先进行预处理
        // 预处理会设置 ROI 范围，特别是 Z 值范围，使算法参数能够匹配
        qDebug() << "开始预处理点云数据...";

        // 设置预处理参数
        m_preProcessPara.fRoiXMin = -10000;
        m_preProcessPara.fRoiXMax = 11500;
        m_preProcessPara.fRoiYMin = -2000;
        m_preProcessPara.fRoiYMax = 2000;
        m_preProcessPara.fRoiZMin = 3500;  // 关键：限制 Z 值范围
        m_preProcessPara.fRoiZMax = 7000;

        m_preProcessPara.fThSeg0Dis = 200;
        m_preProcessPara.iThSeg0NumPtsMin = 20;
        m_preProcessPara.iThSeg0NumPtsMax = 999999;
        m_preProcessPara.fThSeg0DiameterMin = 200;
        m_preProcessPara.fThSeg0DiameterMax = 999999;

        m_preProcessPara.ProcessModel = 0;
        m_preProcessPara.fProSetResolutionX = 20;
        m_preProcessPara.fProSetResolutionY = 10;

        s_PreProcessLidar3dRts preProcessResult;
        s_Rtnf preProcessRet = m_algorithm->OnPreProcess(lidar3d, m_preProcessPara, preProcessResult);
        if (preProcessRet.iCode != 0) {
            // 预处理错误信息是 GBK 编码，需要转换
            QString preProcessErrorStr;
            if (!preProcessRet.strInfo.empty()) {
                QByteArray gbkBytes(preProcessRet.strInfo.c_str(), preProcessRet.strInfo.length());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QTextCodec *codec = QTextCodec::codecForName("GBK");
                preProcessErrorStr = codec->toUnicode(gbkBytes);
#else
                std::string gbkStr = preProcessRet.strInfo;
                int size_needed = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
                std::wstring wstr(size_needed, 0);
                MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, &wstr[0], size_needed);
                preProcessErrorStr = QString::fromStdWString(wstr);
#endif
            } else {
                preProcessErrorStr = "未知错误";
            }
            errorMsg = QString("预处理失败: %1 (错误代码: %2)").arg(preProcessErrorStr).arg(preProcessRet.iCode);
            qDebug() << "预处理失败:" << errorMsg;
            return false;
        }
        qDebug() << "预处理成功";

        // 使用预处理后的数据
        lidar3d = preProcessResult.sImage3dPro;

        // 执行检测算法
        qDebug() << "开始执行算法检测...";

        // 设置检测参数
        m_detectPara.fThNZAbsVal = 0.8;  // 提取平面法线参数

        m_detectPara.fThSegCbDis = 80; // 铜跺分割距离
        m_detectPara.fThCbSelLMin = 3000; // 选取铜跺的长宽参数 点数参数
        m_detectPara.fThCbSelLMax = 22000;
        m_detectPara.fThCbSelWMin = 800;
        m_detectPara.fThCbSelWMax = 5000;
        m_detectPara.fThCbSelPtsMin = 2000;
        m_detectPara.fThCbSelPtsMax = 180000;
        m_detectPara.fThCbSelZMin = 4900+1600;
        m_detectPara.fThCbSelZMax = 5600+1600;

        m_detectPara.bDefectSpaceObj = true; // 是否检测空间异物
        m_detectPara.fDefectSpaceThZMin = 900;  // 提取范围限制参数
        m_detectPara.fDefectSpaceThZMax = 3000;
        m_detectPara.fDefectSpaceThInnerLen = 150; // 内缩长度 单侧内缩距离
        m_detectPara.fDefectSpaceThInnerWidth = 300; // 内缩宽度
        m_detectPara.fDefectSpaceThLen = 500.00;  // 判定参数
        m_detectPara.fDefectSpaceThNumPts = 10;

        m_detectPara.bDefectLbObj = true;  // 是否检测栏板
        m_detectPara.fDefectLbThZMin = 300;  // 提取Z范围限制参数  相对于车板底面
        m_detectPara.fDefectLbThZMax = 1600;
        m_detectPara.fDefectLbThick = 300;
        m_detectPara.fDefectLbThInnerLen = 200;  // 内缩长度
        m_detectPara.fDefectLbThInnerWidth = 100; // 内缩宽度
        m_detectPara.fDefectLbThLen = 2000.00;   // 判定参数-栏板长度
        m_detectPara.fDefectLbThWidth = 1000.00;  // 判定参数-栏板宽度
        m_detectPara.fDefectLbThNumPts = 100;   // 判定参数-栏板点数

        m_detectPara.fThSegTbDis = 60;   // 铜板分割参数
        m_detectPara.fThTbSelLMin = 800;
        m_detectPara.fThTbSelLMax = 1500;
        m_detectPara.fThTbSelWMin = 800;
        m_detectPara.fThTbSelWMax = 1500;
        m_detectPara.fThTbSelPtsMin = 3500-2500;
        m_detectPara.fThTbSelPtsMax = 17000;
        m_detectPara.fThTbSelZMin = 4500+1400;
        m_detectPara.fThTbSelZMax = 5050+1400;
        m_detectPara.fJgTbCloseLenMin = 2000; // 判断铜跺接触最小长度
        m_detectPara.fJgTbClosePtsMin = 4500; // 判断铜跺接触最小点数

        m_detectPara.fThSafeDisTbNear = m_safeDistanceTbNear;  // 铜跺之间的安全间距
        m_detectPara.fThSafeDisToCbOuter = m_safeDistanceToCbOuter; // 铜跺和车板外边界的安全间距

        m_detectPara.fSampleTb = 80;  // 铜跺下采样参数

        s_Rtnf detectRet = m_algorithm->OnPreAD(lidar3d, m_detectPara, result);

        if (detectRet.iCode != 0) {
            // 算法错误信息是 GBK 编码，需要转换
            QString errorInfoStr;
            if (!detectRet.strInfo.empty()) {
                // 将 GBK 编码的 std::string 转换为 QString
                QByteArray gbkBytes(detectRet.strInfo.c_str(), detectRet.strInfo.length());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QTextCodec *codec = QTextCodec::codecForName("GBK");
                errorInfoStr = codec->toUnicode(gbkBytes);
#else
                // Qt 6: 使用 Windows API 转换
                std::string gbkStr = detectRet.strInfo;
                int size_needed = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
                std::wstring wstr(size_needed, 0);
                MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, &wstr[0], size_needed);
                errorInfoStr = QString::fromStdWString(wstr);
#endif
            } else {
                errorInfoStr = "未知错误";
            }

            // 根据错误代码提供更详细的说明
            QString errorDescription;
            switch (detectRet.iCode) {
                case -1:
                    errorDescription = "算法初始化失败或参数错误";
                    break;
                case 1:
                    errorDescription = "图像为空或无效";
                    break;
                case 2:
                    errorDescription = "定位异常（未找到足够的有效点）";
                    break;
                default:
                    errorDescription = QString("算法返回错误代码: %1").arg(detectRet.iCode);
                    break;
            }

            errorMsg = QString("错误代码: %1 (%2), 错误信息: %3")
                          .arg(detectRet.iCode)
                          .arg(errorDescription)
                          .arg(errorInfoStr);
            qDebug() << "算法检测失败 - 错误代码:" << detectRet.iCode;
            qDebug() << "算法检测失败 - 错误描述:" << errorDescription;
            qDebug() << "算法检测失败 - 错误信息:" << errorInfoStr;
            return false;
        }

        qDebug() << "算法检测成功";
        return true;
    } catch (const std::exception &e) {
        errorMsg = QString("执行异常: %1").arg(e.what());
        qDebug() << "算法执行异常:" << e.what();
        return false;
    } catch (...) {
        errorMsg = "执行时发生未知异常";
        qDebug() << "算法执行时发生未知异常";
        return false;
    }
}

void AnodeAlgorithmWorker::initializeCalcPreAlignParameters()
{
    m_calcPreAlignPara.Reset();
    m_calcPreAlignPara.sPoseLidar2Crane.Reset();
    m_calcPreAlignPara.fSX = 15021.0;
    m_calcPreAlignPara.fX = 17719.0;
    m_calcPreAlignPara.fLidar2Gnd = 6700.0;

    m_calcPreAlignPara.sPoseLidar2Crane.fTransX = 20187.4;
    m_calcPreAlignPara.sPoseLidar2Crane.fTransY = -443.479;
    m_calcPreAlignPara.sPoseLidar2Crane.fTransZ = 7258.91;
    m_calcPreAlignPara.sPoseLidar2Crane.fRotX = 191.076;
    m_calcPreAlignPara.sPoseLidar2Crane.fRotY = 348.546;
    m_calcPreAlignPara.sPoseLidar2Crane.fRotZ = 359.312;

    const double mat[12] = {1.01534, -0.016309, 0.109129, 16687.8,
                            0.00147841, 1.00859, -0.0347664, 1071.03,
                            0.0149552, -0.0191281, -0.979451, 6605.62};
    for (int i = 0; i < 12; ++i) {
        m_calcPreAlignPara.MatLidar2Crane[i] = mat[i];
    }

    m_calcPreAlignPara.fOffsetTbX = 0.0;
    m_calcPreAlignPara.fOffsetTbY = 0.0;
    m_calcPreAlignPara.fOffsetTbZ = 0.0;
    m_calcPreAlignPara.fOffsetTbDeg = 0.0;

    m_calcPreAlignPara.fCameraOffsetX = 0.0;
    m_calcPreAlignPara.fCameraOffsetY = 0.0;
    m_calcPreAlignPara.fCameraOffsetZ = 2500.0;
    m_calcPreAlignPara.fCameraOffsetDeg = 0.0;

    m_calcPreAlignPara.fLimitCameraOffsetZMin = 0.0;
    m_calcPreAlignPara.fLimitCameraOffsetZMax = 7000.0;
    m_calcPreAlignPara.fLimitCameraOffsetXMin = 0.0;
    m_calcPreAlignPara.fLimitCameraOffsetXMax = 50000.0;
    m_calcPreAlignPara.fLimitCameraOffsetYMin = 0.0;
    m_calcPreAlignPara.fLimitCameraOffsetYMax = 4000.0;
    m_calcPreAlignPara.fLimitCameraOffsetDegMin = 80.0;
    m_calcPreAlignPara.fLimitCameraOffsetDegMax = 105.0;
}

void AnodeAlgorithmWorker::initializeAccurateParameters()
{
    m_accuratePara = s_AccurateADPlateAPara();
    m_calcAccuratePara = s_CalcAccurateAlignPara();

    m_calcAccuratePara.fSX = 0.0;
    m_calcAccuratePara.fSY = 0.0;
    m_calcAccuratePara.fSZ = -2100.0;
    m_calcAccuratePara.fSDeg = 90.0;

    m_calcAccuratePara.fCameraSX = 18000.0;
    m_calcAccuratePara.fCameraSY = 1500.0;
    m_calcAccuratePara.fCameraSZ = 3500.0;
    m_calcAccuratePara.fCameraSDeg = 90.0;

    m_calcAccuratePara.fCameraX = 19558.0;
    m_calcAccuratePara.fCameraY = 1747.0;
    m_calcAccuratePara.fCameraZ = 4000.0;
    m_calcAccuratePara.fCameraDeg = 88.675;

    m_calcAccuratePara.fOffsetTbX = 0.0;
    m_calcAccuratePara.fOffsetTbY = 0.0;
    m_calcAccuratePara.fOffsetTbZ = 0.0;
    m_calcAccuratePara.fOffsetTbDeg = 0.0;

    m_calcAccuratePara.fLimitRefMaxZ = 500.0;
    m_calcAccuratePara.fLimitRefMaxX = 500.0;
    m_calcAccuratePara.fLimitRefMaxY = 500.0;
    m_calcAccuratePara.fLimitRefMaxDeg = 10.0;

    m_calcAccuratePara.fLimitGripZMin = 0.0;
    m_calcAccuratePara.fLimitGripZMax = 7000.0;
    m_calcAccuratePara.fLimitGripXMin = 0.0;
    m_calcAccuratePara.fLimitGripXMax = 50000.0;
    m_calcAccuratePara.fLimitGripYMin = 0.0;
    m_calcAccuratePara.fLimitGripYMax = 4000.0;
    m_calcAccuratePara.fLimitGripDegMin = 60.0;
    m_calcAccuratePara.fLimitGripDegMax = 300.0;

    m_calcAccuratePara.fAccurateX = 15.0;
    m_calcAccuratePara.fAccurateY = 15.0;
    m_calcAccuratePara.fAccurateZ = 15.0;
    m_calcAccuratePara.fAccurateDeg = 1.0;

    for (int i = 0; i < 12; ++i) {
        m_calcAccuratePara.MatCameraA2Grip[i] = 0.0;
        m_calcAccuratePara.MatCameraB2Grip[i] = 0.0;
    }
    m_calcAccuratePara.MatCameraA2Grip[0] = 1.0;
    m_calcAccuratePara.MatCameraA2Grip[5] = 1.0;
    m_calcAccuratePara.MatCameraA2Grip[10] = 1.0;
    m_calcAccuratePara.MatCameraB2Grip[0] = 1.0;
    m_calcAccuratePara.MatCameraB2Grip[5] = 1.0;
    m_calcAccuratePara.MatCameraB2Grip[10] = 1.0;
}

bool AnodeAlgorithmWorker::loadImage3dFromBasePath(const QString &basePath,
                                                   s_Image3dS &image3d,
                                                   QString &errorMsg)
{
    if (basePath.isEmpty()) {
        errorMsg = QStringLiteral("图像路径为空");
        return false;
    }

    const QString texturePath = basePath + QStringLiteral("_IMG_Texture_8Bit.png");
    if (!QFileInfo::exists(texturePath)) {
        errorMsg = QStringLiteral("未找到原始相机数据: %1").arg(texturePath);
        return false;
    }

    const std::string nativeBase = QDir::toNativeSeparators(basePath).toLocal8Bit().constData();
    int ret = m_converter->ReadImage3DX(image3d, nativeBase);
    if (ret != 0) {
        errorMsg = QStringLiteral("读取3D图像失败，错误码: %1").arg(ret);
        return false;
    }

    return true;
}

QString AnodeAlgorithmWorker::decodeAlgorithmMessage(const std::string &info) const
{
    if (info.empty()) {
        return QString();
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    return codec ? codec->toUnicode(info.c_str()) : QString::fromLocal8Bit(info.c_str());
#else
    int size_needed = MultiByteToWideChar(CP_ACP, 0, info.c_str(), -1, NULL, 0);
    if (size_needed <= 0) {
        return QString::fromLocal8Bit(info.c_str());
    }
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, info.c_str(), -1, wstr.data(), size_needed);
    return QString::fromStdWString(wstr);
#endif
}

bool AnodeAlgorithmWorker::computeCameraPhotoPositions(const s_PreADPlateARtsPara &result,
                                                       s_CalcPreAlignRtsPara &preAlignResult,
                                                       QString &errorMsg)
{
    if (result.TbUpRtsList.empty()) {
        errorMsg = "雷达未检测到铜跺，无法计算拍照位";
        return false;
    }

    s_CalcPreAlignPara para = m_calcPreAlignPara;
    s_Rtnf ret = m_algorithm->CalcPreAlignRts(result, para, preAlignResult);

    if (ret.iCode != 0) {
        QString message = decodeAlgorithmMessage(ret.strInfo);
        if (message.isEmpty()) {
            message = QString("CalcPreAlignRts 返回错误代码 %1").arg(ret.iCode);
        }
        errorMsg = message;
        return false;
    }

    if (!preAlignResult.bTJG) {
        errorMsg = "雷达定位计算结果判定为NG";
        return false;
    }

    if (preAlignResult.fCamaraPosCrane.empty()) {
        errorMsg = "雷达定位计算未输出拍照位";
        return false;
    }

    return true;
}

//              =
// 测试模式：双相机精定位
//              =
void AnodeAlgorithmWorker::processTestDualCamera(const QStringList &imageBasePaths)
{
    if (imageBasePaths.size() != 2) {
        emit testDualCameraFailed(QString("需要提供2个相机图像路径，当前提供了%1个").arg(imageBasePaths.size()));
        return;
    }

    try {
        qDebug() << "[TestDualCamera]  == 开始双相机测试） ==";

        // 打印Halcon初始化状态
        qDebug() << "[TestDualCamera]   Halcon环境检查  ";
        using namespace HalconCpp;
        try {
            HTuple hv_Version, hv_ClipRegion;
            GetSystem("clip_region", &hv_ClipRegion);
            qDebug() << "[TestDualCamera] Halcon clip_region:" << QString::fromLocal8Bit(hv_ClipRegion[0].S().Text());

            // 检查是否设置了UTF8编码
            HTuple hv_Utf8Status;
            // GetHcppInterfaceStringEncodingIsUtf8(&hv_Utf8Status);  // 这个函数可能不存在
            qDebug() << "[TestDualCamera] Halcon初始化检查完成";
        } catch (const HException &ex) {
            qDebug() << "[TestDualCamera] Halcon环境检查异常（非致命）:" << ex.ErrorMessage().Text();
        }
        qDebug() << "[TestDualCamera]        ==";
        qDebug() << "";

        //   步骤1：读取两个相机的原始数据
        qDebug() << "[TestDualCamera] 步骤1：读取相机原始数据";
        s_Image3dS imgOrg[2];
        QString errorMsg;

        for (int iCam = 0; iCam < 2; iCam++) {
            qDebug() << QString("[TestDualCamera] 读取相机%1数据: %2").arg(iCam + 1).arg(imageBasePaths[iCam]);
            if (!loadImage3dFromBasePath(imageBasePaths[iCam], imgOrg[iCam], errorMsg)) {
                emit testDualCameraFailed(QString("相机%1数据读取失败: %2").arg(iCam + 1).arg(errorMsg));
                return;
            }

            // 检查图像数据完整性
            HTuple width, height;
            GetImageSize(imgOrg[iCam].X, &width, &height);
            qDebug() << QString("[TestDualCamera] 相机%1数据读取成功 - 尺寸: %2x%3")
                        .arg(iCam + 1)
                        .arg(width[0].I())
                        .arg(height[0].I());

            // 检查是否有有效数据
            HTuple minVal, maxVal, rangeVal;
            MinMaxGray(imgOrg[iCam].Z, imgOrg[iCam].Z, 0, &minVal, &maxVal, &rangeVal);
            qDebug() << QString("[TestDualCamera] 相机%1 Z值范围: [%2, %3]")
                        .arg(iCam + 1)
                        .arg(minVal[0].D())
                        .arg(maxVal[0].D());

            // 测试：直接用原始图像创建ObjectModel3D，检查是否有xyz_mapping
            HTuple hv_TestObjModel;
            XyzToObjectModel3d(imgOrg[iCam].X, imgOrg[iCam].Y, imgOrg[iCam].Z, &hv_TestObjModel);
            // 注意：has_xyz_mappings参数名可能不存在，先注释掉避免中断流程
            // 后续通过ObjectModel3dToXyz的成功与否来判断xyz_mapping是否存在
            qDebug() << QString("[TestDualCamera] 相机%1原始数据的ObjectModel3D已创建").arg(iCam + 1);
        }

        //   步骤2：坐标系变换（相机坐标系→工具坐标系/夹爪坐标系）
        qDebug() << "[TestDualCamera] 步骤2：坐标系变换（相机→工具坐标系）";
        s_Image3dS imgTool[2];

        // 使用标定矩阵
        // 这些是标定得到的变换矩阵，需保留7位有效小数
        // 相机转工具坐标系矩阵（保留 7 位有效小数）
        double Mat1[12] = {0.0037917, -0.986526, -0.163562, 802.976,
                           -0.990748, -0.0258952, 0.13322, -437.525,
                           -0.13566, 0.161544, -0.977497, 2879.82 - 2207};
        double Mat2[12] = {-0.00692093, 0.992505, 0.122012, -669.506,
                           0.992222, 0.0219815, -0.122526, 415.139,
                           -0.12429, 0.120215, -0.984937, 2884.28 - 2207};

        for (int iCam = 0; iCam < 2; iCam++) {
            using namespace HalconCpp;

            qDebug() << QString("[TestDualCamera] 相机%1开始坐标变换").arg(iCam + 1);

            HTuple hv_HomMat3D;
            hv_HomMat3D.Clear();

            // 选择对应相机的标定矩阵
            double* mat = (iCam == 0) ? Mat1 : Mat2;
            for (int i = 0; i < 12; i++) {
                hv_HomMat3D[i] = mat[i];
            }
            qDebug() << QString("[TestDualCamera] 相机%1标定矩阵已设置").arg(iCam + 1);

            // Halcon 3D坐标变换
            HTuple hv_ObjectModel3D, hv_ObjectModel3DT;
            qDebug() << QString("[TestDualCamera] 相机%1执行XyzToObjectModel3d").arg(iCam + 1);
            XyzToObjectModel3d(imgOrg[iCam].X, imgOrg[iCam].Y, imgOrg[iCam].Z, &hv_ObjectModel3D);

            qDebug() << QString("[TestDualCamera] 相机%1执行AffineTransObjectModel3d").arg(iCam + 1);
            AffineTransObjectModel3d(hv_ObjectModel3D, hv_HomMat3D, &hv_ObjectModel3DT);

            qDebug() << QString("[TestDualCamera] 相机%1坐标变换完成，准备提取XYZ").arg(iCam + 1);


            qDebug() << QString("[TestDualCamera] 相机%1执行ObjectModel3dToXyz (Method: from_xyz_map)").arg(iCam + 1);
            HObject ho_X, ho_Y, ho_Z;
            ObjectModel3dToXyz(&ho_X, &ho_Y, &ho_Z, hv_ObjectModel3DT, "from_xyz_map", HTuple(), HTuple());

            imgTool[iCam] = imgOrg[iCam].DeepCopy();
            imgTool[iCam].X = ho_X.Clone();
            imgTool[iCam].Y = ho_Y.Clone();
            imgTool[iCam].Z = ho_Z.Clone();

            qDebug() << QString("[TestDualCamera] 相机%1坐标变换完成").arg(iCam + 1);
        }

        //   步骤3：预处理两个相机数据
        qDebug() << "[TestDualCamera] 步骤3：预处理相机数据";
        s_PreProcess3DSResultPara preProcessRts[2];

        for (int iCam = 0; iCam < 2; iCam++) {
            s_PreProcess3DSPara preProcessPara;

            // 设置预处理参数
            if (iCam == 0) {
                // 相机1预处理参数
                preProcessPara.fRoiZMin = -2800;
                preProcessPara.fRoiZMax = -1200;
                preProcessPara.fRoiXMin = -400;
                preProcessPara.fRoiXMax = 1000;
                preProcessPara.fRoiYMin = -1000;
                preProcessPara.fRoiYMax = 1000;
            } else {
                // 相机2预处理参数
                preProcessPara.fRoiZMin = -2800;
                preProcessPara.fRoiZMax = -1200;
                preProcessPara.fRoiXMin = -1000;
                preProcessPara.fRoiXMax = 400;
                preProcessPara.fRoiYMin = -1000;
                preProcessPara.fRoiYMax = 1000;
            }

            preProcessPara.fThSeg0Dis = 20;
            preProcessPara.iThSeg0NumPtsMin = 3;
            preProcessPara.iThSeg0NumPtsMax = 99999999;
            preProcessPara.fThSeg0DiameterMin = 5;
            preProcessPara.fThSeg0DiameterMax = 3000;

            s_Rtnf rts = m_algorithm->OnPreProcess(imgTool[iCam], preProcessPara, preProcessRts[iCam]);

            if (rts.iCode != 0) {
                QString errorMsg = decodeAlgorithmMessage(rts.strInfo);
                emit testDualCameraFailed(QString("相机%1预处理失败: %2 (错误码:%3)")
                                          .arg(iCam + 1)
                                          .arg(errorMsg)
                                          .arg(rts.iCode));
                return;
            }

            int preProcessTime = static_cast<int>(preProcessRts[iCam].time * 1000);
            qDebug() << QString("[TestDualCamera] 相机%1预处理完成，耗时=%2ms")
                        .arg(iCam + 1)
                        .arg(preProcessTime);

            // 输出到主窗口
            emit testDualCameraProgress(QString("自动：预处理图像完成--耗时=%1 ms").arg(preProcessTime));
        }

        //   步骤4：执行精定位检测
        qDebug() << "[TestDualCamera] 步骤4：执行精定位检测";
        QElapsedTimer timerAcc;
        timerAcc.start();
        emit testDualCameraProgress("步骤4/5 精定位检测 - 调用算法");
        std::vector<s_Image3dS> imgList;
        imgList.push_back(preProcessRts[0].sImage3dPro.DeepCopy());
        imgList.push_back(preProcessRts[1].sImage3dPro.DeepCopy());

        // 设置精定位检测参数
        m_accuratePara.fThCbNZAbsValMin = 0.95;
        m_accuratePara.fThCbNZAbsValMax = 1;
        m_accuratePara.fThCbNXAbsValMin = 0;
        m_accuratePara.fThCbNXAbsValMax = 0.4;
        m_accuratePara.fThCbNYAbsValMin = 0;
        m_accuratePara.fThCbNYAbsValMax = 0.35;

        m_accuratePara.fThSegCbDis = 20;
        m_accuratePara.fThCbSelLMin = 500;
        m_accuratePara.fThCbSelLMax = 5000;
        m_accuratePara.fThCbSelWMin = 500;
        m_accuratePara.fThCbSelWMax = 5000;
        m_accuratePara.fThCbSelPtsMin = 2000;
        m_accuratePara.fThCbSelPtsMax = 2000000;
        m_accuratePara.fThCbSelZMin = -2600;
        m_accuratePara.fThCbSelZMax = -2000;

        m_accuratePara.fThTbNZAbsVal = 0.90;
        m_accuratePara.fThSegTbDis = 10;
        m_accuratePara.fThTbSelLMin = 800;
        m_accuratePara.fThTbSelLMax = 1500;
        m_accuratePara.fThTbSelWMin = 800;
        m_accuratePara.fThTbSelWMax = 1500;
        m_accuratePara.fThTbSelPtsMin = 50000;
        m_accuratePara.fThTbSelPtsMax = 1500000;
        m_accuratePara.fThTbSelZMin = -2100;
        m_accuratePara.fThTbSelZMax = -1600;

        m_accuratePara.fThSafeDisTdNear = 350;
        m_accuratePara.fDefectTdNearSelZRefCb = 100;
        m_accuratePara.iDefectTdNearRoiWDilation = 201;
        m_accuratePara.iDefectTdNearRoiLErosion = 401;

        m_accuratePara.iBmRoiWDilationW = 301;
        m_accuratePara.iBmRoiWErosionL = 65;
        m_accuratePara.iBmRoiLDilationL = 101;
        m_accuratePara.fOffsetZSelTbRelCb = 30;

        m_accuratePara.fEdgeWLenUse = 800;
        m_accuratePara.fGndSelEdgeWOffset = 300;
        m_accuratePara.fGndSelEdgeWLen = 600;
        m_accuratePara.fGripUp = 0;

        m_accuratePara.fLimitDegMin = 80;
        m_accuratePara.fLimitDegMax = 100;
        m_accuratePara.fLimitTdWMin = 900;
        m_accuratePara.fLimitTdWMax = 1100.0;
        m_accuratePara.fLimitTdLMin = 980;
        m_accuratePara.fLimitTdLMax = 1200;
        m_accuratePara.fLimitTdHMin = 300;
        m_accuratePara.fLimitTdHMax = 600;

        m_accuratePara.fThTbLayerOuter = 100;
        m_accuratePara.fDefectGripSpaceRoiXOffeset = 10;
        m_accuratePara.fDefectGripSpaceRoiXLen = 200;
        m_accuratePara.fDefectGripSpaceRoiYLen = 600;
        m_accuratePara.fDefectGripSpaceRoiZOffesetCb = 35;
        m_accuratePara.fDefectGripSpaceRoiZLen = 500;
        m_accuratePara.fDefectGripSpaceThLen = 50.00;
        m_accuratePara.fDefectGripSpaceThNumPts = 10;

        s_AccurateADPlateARtsPara detectResult;
        s_Rtnf rts = m_algorithm->OnAccurateAD(imgList, m_accuratePara, detectResult);
        qDebug() << "[TestDualCamera] OnAccurateAD 返回, iCode=" << rts.iCode
                 << ", 耗时(ms)=" << timerAcc.elapsed();

        if (rts.iCode != 0) {
            QString errorMsg = decodeAlgorithmMessage(rts.strInfo);
            qDebug() << "[TestDualCamera] OnAccurateAD 失败, 错误码:" << rts.iCode
                     << ", 错误信息:" << errorMsg;
            emit testDualCameraFailed(QString("精定位检测失败: %1 (错误码:%2)")
                                      .arg(errorMsg)
                                      .arg(rts.iCode));
            return;
        }

        qDebug() << QString("[TestDualCamera] 精定位检测完成，耗时=%1ms, 综合判定=%2")
                    .arg(static_cast<int>(detectResult.time * 1000))
                    .arg(detectResult.bTJG ? "OK" : "NG");

        //   步骤5：计算坐标结果
        qDebug() << "[TestDualCamera] 步骤5：计算抓取坐标";

        // 设置计算参数
        m_calcAccuratePara.fSX = 0;
        m_calcAccuratePara.fSY = 0;
        m_calcAccuratePara.fSZ = -2100;
        m_calcAccuratePara.fSDeg = 90;

        // 使用从UI设置的相机拍照位坐标
        // （这些值在调用processTestDualCameraMemory之前已经通过setCameraPositionParameters更新）

        m_calcAccuratePara.fCameraSX = 20692.0;
        m_calcAccuratePara.fCameraSY = 2432.0;
        m_calcAccuratePara.fCameraSZ = 2207.0;
        m_calcAccuratePara.fCameraSDeg = 90.0;

        m_calcAccuratePara.fOffsetTbX = 0.1;
        m_calcAccuratePara.fOffsetTbY = 0.2;
        m_calcAccuratePara.fOffsetTbZ = 0.3;
        m_calcAccuratePara.fOffsetTbDeg = -0.01;

        m_calcAccuratePara.fLimitRefMaxZ = 500;
        m_calcAccuratePara.fLimitRefMaxX = 500;
        m_calcAccuratePara.fLimitRefMaxY = 500;
        m_calcAccuratePara.fLimitRefMaxDeg = 10;

        m_calcAccuratePara.fLimitGripZMin = -50;
        m_calcAccuratePara.fLimitGripZMax = 7000;
        m_calcAccuratePara.fLimitGripXMin = 0;
        m_calcAccuratePara.fLimitGripXMax = 50000;
        m_calcAccuratePara.fLimitGripYMin = 0;
        m_calcAccuratePara.fLimitGripYMax = 4000;
        m_calcAccuratePara.fLimitGripDegMin = 60;
        m_calcAccuratePara.fLimitGripDegMax = 300;

        m_calcAccuratePara.fAccurateX = 20;
        m_calcAccuratePara.fAccurateY = 20;
        m_calcAccuratePara.fAccurateZ = 20;
        m_calcAccuratePara.fAccurateDeg = 1.5;

        QElapsedTimer timerCalc;
        timerCalc.start();
        s_CalcAccurateAlignRtsPara calcResult;
        qDebug() << "[TestDualCamera] 开始 CalcAccurateAlignRts";
        rts = m_algorithm->CalcAccurateAlignRts(detectResult, m_calcAccuratePara, calcResult);
        qDebug() << "[TestDualCamera] CalcAccurateAlignRts 返回, iCode=" << rts.iCode
                 << ", 耗时(ms)=" << timerCalc.elapsed();

        if (rts.iCode != 0) {
            QString errorMsg = decodeAlgorithmMessage(rts.strInfo);
            emit testDualCameraFailed(QString("坐标计算失败: %1 (错误码:%2)")
                                      .arg(errorMsg)
                                      .arg(rts.iCode));
            return;
        }

        qDebug() << QString("[TestDualCamera] 坐标计算完成，状态=%1, 抓取位=(%2, %3, %4, %5)")
                    .arg(calcResult.iStatus)
                    .arg(calcResult.fGripX, 0, 'f', 1)
                    .arg(calcResult.fGripY, 0, 'f', 1)
                    .arg(calcResult.fGripZ, 0, 'f', 1)
                    .arg(calcResult.fGripDeg, 0, 'f', 2);

        qDebug() << "[TestDualCamera]  == 双相机测试完成  ==";

        // 发送成功信号（包含输入参数，用于日志输出）
        emit testDualCameraFinished(detectResult, calcResult, m_calcAccuratePara);

    } catch (const HalconCpp::HException &ex) {
        QString errorMsg = QString("Halcon异常: %1 (过程:%2)")
                           .arg(QString::fromLocal8Bit(ex.ErrorMessage().Text()))
                           .arg(QString::fromLocal8Bit(ex.ProcName().Text()));
        qDebug() << "[TestDualCamera] " << errorMsg;
        emit testDualCameraFailed(errorMsg);
    } catch (const std::exception &ex) {
        QString errorMsg = QString("标准异常: %1").arg(ex.what());
        qDebug() << "[TestDualCamera] " << errorMsg;
        emit testDualCameraFailed(errorMsg);
    } catch (...) {
        qDebug() << "[TestDualCamera] 未知异常";
        emit testDualCameraFailed("未知异常");
    }
}


// 内存版：直接使用传入的 s_Image3dS，绕过磁盘读取
void AnodeAlgorithmWorker::processTestDualCameraMemory(const s_Image3dS &img1, const s_Image3dS &img2)
{
    QString currentStep = "读取原始数据";
    try {
        qDebug() << "[TestDualCamera][Memory]  == 开始双相机测试（内存） ==";
        emit testDualCameraProgress("步骤1/5 读取原始数据");
        using namespace HalconCpp;

        s_Image3dS imgOrg[2];
        imgOrg[0] = img1;
        imgOrg[1] = img2;

        //   步骤2：坐标系变换（相机→工具坐标系）
        currentStep = "坐标系变换";
        emit testDualCameraProgress("步骤2/5 坐标系变换（相机→工具坐标系）");
        qDebug() << "[TestDualCamera][Memory] 步骤2：坐标系变换（相机→工具坐标系）";
        s_Image3dS imgTool[2];

        double Mat1[12] = {0.0037917, -0.986526, -0.163562, 802.976,
                           -0.990748, -0.0258952, 0.13322, -437.525,
                           -0.13566, 0.161544, -0.977497, 2879.82 - 2207};
        double Mat2[12] = {-0.00692093, 0.992505, 0.122012, -669.506,
                           0.992222, 0.0219815, -0.122526, 415.139,
                           -0.12429, 0.120215, -0.984937, 2884.28 - 2207};

        for (int iCam = 0; iCam < 2; iCam++) {
            HTuple hv_HomMat3D;
            hv_HomMat3D.Clear();
            double* mat = (iCam == 0) ? Mat1 : Mat2;
            for (int i = 0; i < 12; i++) {
                hv_HomMat3D[i] = mat[i];
            }

            HTuple hv_ObjectModel3D, hv_ObjectModel3DT;
            XyzToObjectModel3d(imgOrg[iCam].X, imgOrg[iCam].Y, imgOrg[iCam].Z, &hv_ObjectModel3D);
            AffineTransObjectModel3d(hv_ObjectModel3D, hv_HomMat3D, &hv_ObjectModel3DT);

            HObject ho_X, ho_Y, ho_Z;
            ObjectModel3dToXyz(&ho_X, &ho_Y, &ho_Z, hv_ObjectModel3DT, "from_xyz_map", HTuple(), HTuple());

            imgTool[iCam] = imgOrg[iCam].DeepCopy();
            imgTool[iCam].X = ho_X.Clone();
            imgTool[iCam].Y = ho_Y.Clone();
            imgTool[iCam].Z = ho_Z.Clone();
        }

        //   步骤3：预处理两个相机数据
        currentStep = "图像预处理";
        emit testDualCameraProgress("步骤3/5 图像预处理");
        qDebug() << "[TestDualCamera][Memory] 步骤3：预处理相机数据";
        QElapsedTimer timerPre;
        timerPre.start();
        s_PreProcess3DSResultPara preProcessRts[2];

        for (int iCam = 0; iCam < 2; iCam++) {
            s_PreProcess3DSPara preProcessPara;
            if (iCam == 0) {
                preProcessPara.fRoiZMin = -2800;
                preProcessPara.fRoiZMax = -1200;
                preProcessPara.fRoiXMin = -400;
                preProcessPara.fRoiXMax = 1000;
                preProcessPara.fRoiYMin = -1000;
                preProcessPara.fRoiYMax = 1000;
            } else {
                preProcessPara.fRoiZMin = -2800;
                preProcessPara.fRoiZMax = -1200;
                preProcessPara.fRoiXMin = -1000;
                preProcessPara.fRoiXMax = 400;
                preProcessPara.fRoiYMin = -1000;
                preProcessPara.fRoiYMax = 1000;
            }
            preProcessPara.fThSeg0Dis = 20;
            preProcessPara.iThSeg0NumPtsMin = 3;
            preProcessPara.iThSeg0NumPtsMax = 99999999;
            preProcessPara.fThSeg0DiameterMin = 5;
            preProcessPara.fThSeg0DiameterMax = 3000;

            s_Rtnf rts = m_algorithm->OnPreProcess(imgTool[iCam], preProcessPara, preProcessRts[iCam]);
            if (rts.iCode != 0) {
                QString errorMsg = decodeAlgorithmMessage(rts.strInfo);
                emit testDualCameraFailed(QString("相机%1预处理失败: %2 (错误码:%3)")
                                          .arg(iCam + 1)
                                          .arg(errorMsg)
                                          .arg(rts.iCode));
                return;
            }
        }

        qDebug() << "[TestDualCamera][Memory] 步骤3预处理总耗时(ms)=" << timerPre.elapsed();

        // 步骤4：执行精定位检测
        currentStep = "精定位检测";
        emit testDualCameraProgress("步骤4/5 精定位检测");
        qDebug() << "[TestDualCamera][Memory] 步骤4：执行精定位检测";
        std::vector<s_Image3dS> imgList;
        imgList.push_back(preProcessRts[0].sImage3dPro.DeepCopy());
        imgList.push_back(preProcessRts[1].sImage3dPro.DeepCopy());

        // 使用与文件模式一致的精定位参数（每次调用前重置，避免外部修改导致异常）
        m_accuratePara.fThCbNZAbsValMin = 0.95;
        m_accuratePara.fThCbNZAbsValMax = 1;
        m_accuratePara.fThCbNXAbsValMin = 0;
        m_accuratePara.fThCbNXAbsValMax = 0.4;
        m_accuratePara.fThCbNYAbsValMin = 0;
        m_accuratePara.fThCbNYAbsValMax = 0.35;

        m_accuratePara.fThSegCbDis = 20;
        m_accuratePara.fThCbSelLMin = 500;
        m_accuratePara.fThCbSelLMax = 5000;
        m_accuratePara.fThCbSelWMin = 500;
        m_accuratePara.fThCbSelWMax = 5000;
        m_accuratePara.fThCbSelPtsMin = 2000;
        m_accuratePara.fThCbSelPtsMax = 2000000;
        m_accuratePara.fThCbSelZMin = -2600;
        m_accuratePara.fThCbSelZMax = -2000;

        m_accuratePara.fThTbNZAbsVal = 0.90;
        m_accuratePara.fThSegTbDis = 10;
        m_accuratePara.fThTbSelLMin = 800;
        m_accuratePara.fThTbSelLMax = 1500;
        m_accuratePara.fThTbSelWMin = 800;
        m_accuratePara.fThTbSelWMax = 1500;
        m_accuratePara.fThTbSelPtsMin = 50000;
        m_accuratePara.fThTbSelPtsMax = 1500000;
        m_accuratePara.fThTbSelZMin = -2100;
        m_accuratePara.fThTbSelZMax = -1600;

        m_accuratePara.fThSafeDisTdNear = 350;
        m_accuratePara.fDefectTdNearSelZRefCb = 100;
        m_accuratePara.iDefectTdNearRoiWDilation = 201;
        m_accuratePara.iDefectTdNearRoiLErosion = 401;

        m_accuratePara.iBmRoiWDilationW = 301;
        m_accuratePara.iBmRoiWErosionL = 65;
        m_accuratePara.iBmRoiLDilationL =101;
        m_accuratePara.fOffsetZSelTbRelCb = 30;

        m_accuratePara.fEdgeWLenUse = 800;
        m_accuratePara.fGndSelEdgeWOffset = 300;
        m_accuratePara.fGndSelEdgeWLen = 600;
        m_accuratePara.fGripUp = 0;

        m_accuratePara.fLimitDegMin = 80;
        m_accuratePara.fLimitDegMax = 100;
        m_accuratePara.fLimitTdWMin = 900;
        m_accuratePara.fLimitTdWMax = 1100.0;
        m_accuratePara.fLimitTdLMin = 980;
        m_accuratePara.fLimitTdLMax = 1200;
        m_accuratePara.fLimitTdHMin = 300;
        m_accuratePara.fLimitTdHMax = 600;

        m_accuratePara.fThTbLayerOuter = 100;
        m_accuratePara.fDefectGripSpaceRoiXOffeset = 10;
        m_accuratePara.fDefectGripSpaceRoiXLen = 200;
        m_accuratePara.fDefectGripSpaceRoiYLen = 600;
        m_accuratePara.fDefectGripSpaceRoiZOffesetCb = 35;
        m_accuratePara.fDefectGripSpaceRoiZLen = 500;
        m_accuratePara.fDefectGripSpaceThLen = 50.00;
        m_accuratePara.fDefectGripSpaceThNumPts = 10;

        // 计算参数（保持与文件模式一致）
        m_calcAccuratePara.fSX = 0;
        m_calcAccuratePara.fSY = 0;
        m_calcAccuratePara.fSZ = -2100;
        m_calcAccuratePara.fSDeg = 90;

        m_calcAccuratePara.fCameraSX = 20692.0;
        m_calcAccuratePara.fCameraSY = 2432.0;
        m_calcAccuratePara.fCameraSZ = 2207.0;
        m_calcAccuratePara.fCameraSDeg = 90.0;

        m_calcAccuratePara.fOffsetTbX = 0.1;
        m_calcAccuratePara.fOffsetTbY = 0.2;
        m_calcAccuratePara.fOffsetTbZ = 0.3;
        m_calcAccuratePara.fOffsetTbDeg = -0.01;

        m_calcAccuratePara.fLimitRefMaxZ = 500;
        m_calcAccuratePara.fLimitRefMaxX = 500;
        m_calcAccuratePara.fLimitRefMaxY = 500;
        m_calcAccuratePara.fLimitRefMaxDeg = 10;

        m_calcAccuratePara.fLimitGripZMin = -50;
        m_calcAccuratePara.fLimitGripZMax = 7000;
        m_calcAccuratePara.fLimitGripXMin = 0;
        m_calcAccuratePara.fLimitGripXMax = 50000;
        m_calcAccuratePara.fLimitGripYMin = 0;
        m_calcAccuratePara.fLimitGripYMax = 4000;
        m_calcAccuratePara.fLimitGripDegMin = 60;
        m_calcAccuratePara.fLimitGripDegMax = 300;

        m_calcAccuratePara.fAccurateX = 20;
        m_calcAccuratePara.fAccurateY = 20;
        m_calcAccuratePara.fAccurateZ = 20;
        m_calcAccuratePara.fAccurateDeg = 1.5;

        s_AccurateADPlateARtsPara detectResult;
        QElapsedTimer timerAcc;
        timerAcc.start();
        s_Rtnf rts = m_algorithm->OnAccurateAD(imgList, m_accuratePara, detectResult);
        qDebug() << "[TestDualCamera][Memory] OnAccurateAD 返回, 耗时(ms)=" << timerAcc.elapsed();
        if (rts.iCode != 0) {
            QString errorMsg = decodeAlgorithmMessage(rts.strInfo);
            emit testDualCameraFailed(QString("精定位检测失败: %1 (错误码:%2)")
                                      .arg(errorMsg)
                                      .arg(rts.iCode));
            return;
        }

        qDebug() << QString("[TestDualCamera][Memory] 精定位检测完成，耗时=%1ms, 综合判定=%2")
                    .arg(static_cast<int>(detectResult.time * 1000))
                    .arg(detectResult.bTJG ? "OK" : "NG");

        // 步骤5：计算抓取坐标
        currentStep = "计算坐标结果";
        emit testDualCameraProgress("步骤5/5 计算坐标结果");
        s_CalcAccurateAlignRtsPara calcResult;
        QElapsedTimer timerCalc;
        timerCalc.start();
        rts = m_algorithm->CalcAccurateAlignRts(detectResult, m_calcAccuratePara, calcResult);
        qDebug() << "[TestDualCamera][Memory] CalcAccurateAlignRts 返回, iCode=" << rts.iCode
                 << ", 耗时(ms)=" << timerCalc.elapsed();
        if (rts.iCode != 0) {
            QString errorMsg = decodeAlgorithmMessage(rts.strInfo);
            emit testDualCameraFailed(QString("坐标计算失败: %1 (错误码:%2)")
                                      .arg(errorMsg)
                                      .arg(rts.iCode));
            return;
        }

        qDebug() << QString("[TestDualCamera][Memory] 坐标计算完成，状态=%1, 抓取位=(%2, %3, %4, %5)")
                    .arg(calcResult.iStatus)
                    .arg(calcResult.fGripX, 0, 'f', 1)
                    .arg(calcResult.fGripY, 0, 'f', 1)
                    .arg(calcResult.fGripZ, 0, 'f', 1)
                    .arg(calcResult.fGripDeg, 0, 'f', 2);

        qDebug() << QString("[TestDualCamera][Memory] 对位偏差 dX=%1 dY=%2 dZ=%3 dDeg=%4")
                    .arg(calcResult.dX, 0, 'f', 3)
                    .arg(calcResult.dY, 0, 'f', 3)
                    .arg(calcResult.dZ, 0, 'f', 3)
                    .arg(calcResult.dDeg, 0, 'f', 3);

        qDebug() << QString("[TestDualCamera][Memory] 下一个拍照位 NextCamera=(%1, %2, %3, %4)")
                    .arg(calcResult.fNextCameraX, 0, 'f', 3)
                    .arg(calcResult.fNextCameraY, 0, 'f', 3)
                    .arg(calcResult.fNextCameraZ, 0, 'f', 3)
                    .arg(calcResult.fNextCameraDeg, 0, 'f', 3);

        qDebug() << QString("[TestDualCamera][Memory] 铜跺尺寸 W=%1 L=%2 H=%3，方向=%4")
                    .arg(calcResult.TdW, 0, 'f', 3)
                    .arg(calcResult.TdL, 0, 'f', 3)
                    .arg(calcResult.TdH, 0, 'f', 3)
                    .arg(calcResult.iTdEarDir);

        emit testDualCameraFinished(detectResult, calcResult, m_calcAccuratePara);
        emit testDualCameraProgress("相机内存双机检测完成");

    } catch (const HalconCpp::HException &ex) {
        QString errorMsg = QString("Halcon异常[%1]: %2 (过程:%3)")
                           .arg(currentStep)
                           .arg(QString::fromLocal8Bit(ex.ErrorMessage().Text()))
                           .arg(QString::fromLocal8Bit(ex.ProcName().Text()));
        emit testDualCameraFailed(errorMsg);
    } catch (const std::exception &ex) {
        emit testDualCameraFailed(QString("标准异常[%1]: %2").arg(currentStep).arg(ex.what()));
    } catch (...) {
        emit testDualCameraFailed(QString("未知异常[%1]").arg("流程步骤失败"));
    }
}

