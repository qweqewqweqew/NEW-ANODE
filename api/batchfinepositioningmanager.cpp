#include "batchfinepositioningmanager.h"
#include <QDebug>
#include <QDateTime>
#include <QImage>

BatchFinePositioningManager::BatchFinePositioningManager(QObject *parent)
    : QObject(parent)
    , m_currentIndex(-1)
    , m_isRunning(false)
    , m_cameraAlgorithm(nullptr)
    , m_testMode(false)
    , m_alignMax(5)
    , m_captureDelayMs(5000)
{
    // 创建WMS移动超时定时器（120秒）
    m_wmsMoveTimeoutTimer = new QTimer(this);
    m_wmsMoveTimeoutTimer->setSingleShot(true);
    m_wmsMoveTimeoutTimer->setInterval(120000); // 120秒
    connect(m_wmsMoveTimeoutTimer, &QTimer::timeout, this, &BatchFinePositioningManager::onWmsMoveTimeout);
}

BatchFinePositioningManager::~BatchFinePositioningManager()
{
    stopBatchPositioning();
}

void BatchFinePositioningManager::initialize(const QList<ApiTypes::Point3D> &photoPoses)
{
    m_photoPoses = photoPoses;
    m_results.clear();
    m_currentIndex = -1;
    m_isRunning = false;

    // 为每个拍照位创建精定位结果
    for (int i = 0; i < photoPoses.size(); ++i) {
        FinePositioningResult result;
        result.blockIndex = i;
        result.status = FinePositioningResult::Pending;
        m_results.append(result);
    }

    qDebug() << "[BatchFinePositioning] 初始化完成，拍照位数量:" << m_results.size();
}

void BatchFinePositioningManager::setTestMode(bool enabled, const QList<ApiTypes::Point3D> &testCoordinates)
{
    m_testMode = enabled;
    m_testCoordinates = testCoordinates;
    
    if (enabled) {
        if (testCoordinates.isEmpty()) {
            qDebug() << "[BatchFinePositioning] 测试模式已启用（将使用拍照位坐标+偏移作为测试坐标）";
        } else {
            qDebug() << QString("[BatchFinePositioning] 测试模式已启用，已设置 %1 个测试坐标").arg(testCoordinates.size());
        }
    } else {
        qDebug() << "[BatchFinePositioning] 测试模式已禁用";
    }
}

void BatchFinePositioningManager::startBatchPositioning()
{
    if (m_photoPoses.isEmpty()) {
        qWarning() << "[BatchFinePositioning] 拍照位列表为空，无法开始批量精定位";
        emit batchPositioningFailed("拍照位列表为空");
        return;
    }

    if (m_isRunning) {
        qWarning() << "[BatchFinePositioning] 批量精定位已在进行中";
        return;
    }

    m_isRunning = true;
    m_currentIndex = 0;

    qDebug() << "[BatchFinePositioning] 开始批量精定位，总数量:" << m_results.size();
    emit progressUpdated(0, m_results.size(), -1);

    // 开始处理第一个拍照位
    processNextPhotoPose();
}

void BatchFinePositioningManager::stopBatchPositioning()
{
    if (!m_isRunning) {
        return;
    }

    m_isRunning = false;
    m_wmsMoveTimeoutTimer->stop();

    qDebug() << "[BatchFinePositioning] 批量精定位已停止";
}

void BatchFinePositioningManager::processNextPhotoPose()
{
    if (!m_isRunning) {
        return;
    }

    // 查找下一个待处理的拍照位
    while (m_currentIndex < m_results.size()) {
        if (m_results[m_currentIndex].status == FinePositioningResult::Pending ||
            m_results[m_currentIndex].status == FinePositioningResult::Failed) {
            break;
        }
        m_currentIndex++;
    }

    // 检查是否所有拍照位都已完成
    if (m_currentIndex >= m_results.size()) {
        checkAndHandleCompletion();
        return;
    }

    // 获取当前拍照位坐标
    const ApiTypes::Point3D &photoPose = m_photoPoses[m_currentIndex];
    
    // 更新状态
    FinePositioningResult &result = m_results[m_currentIndex];
    result.status = FinePositioningResult::Positioning;
    result.retryCount = 0;
    result.positioningRequestCount = 1;  // 第一次请求
    // 保存第一次请求的坐标（用于第二次请求时+2）
    result.firstRequestX = photoPose.x;
    result.firstRequestY = photoPose.y;
    result.firstRequestZ = photoPose.z;
    result.firstRequestDeg = photoPose.deg;

    qDebug() << QString("[BatchFinePositioning] 开始处理拍照位 #%1 (索引:%2), 第一次移动请求，坐标:(%3, %4, %5, %6)")
                .arg(m_currentIndex + 1)
                .arg(m_currentIndex)
                .arg(photoPose.x, 0, 'f', 1)
                .arg(photoPose.y, 0, 'f', 1)
                .arg(photoPose.z, 0, 'f', 1)
                .arg(photoPose.deg, 0, 'f', 2);

    // 调用WMS写入坐标（第一次请求：使用原始拍照位坐标）
    emit requestAdjustPosition(m_currentIndex, photoPose.x, photoPose.y, photoPose.z, photoPose.deg);

    // 启动超时定时器（120秒）
    m_wmsMoveTimeoutTimer->start();
}

void BatchFinePositioningManager::onPositionReady(int blockIndex)
{
    // 停止超时定时器
    m_wmsMoveTimeoutTimer->stop();

    // blockIndex完全由内部决定，忽略WMS传入的值
    // 首先尝试使用m_currentIndex（如果有效）
    int targetIndex = -1;
    
    if (m_currentIndex >= 0 && m_currentIndex < m_results.size()) {
        // 检查当前索引的状态是否适合处理
        FinePositioningResult::Status currentStatus = m_results[m_currentIndex].status;
        if (currentStatus == FinePositioningResult::Positioning || 
            currentStatus == FinePositioningResult::WaitingWms) {
            targetIndex = m_currentIndex;
            qDebug() << QString("[BatchFinePositioning] 使用当前索引: %1").arg(targetIndex);
        }
    }
    
    // 如果当前索引无效，查找正在处理或等待WMS的拍照位
    if (targetIndex < 0) {
        for (int i = 0; i < m_results.size(); ++i) {
            FinePositioningResult::Status status = m_results[i].status;
            if (status == FinePositioningResult::Positioning || 
                status == FinePositioningResult::WaitingWms) {
                targetIndex = i;
                m_currentIndex = i;
                qDebug() << QString("[BatchFinePositioning] 找到正在处理的拍照位，索引: %1").arg(targetIndex);
                break;
            }
        }
    }
    
    // 如果还是找不到，查找第一个待处理的拍照位
    if (targetIndex < 0) {
        for (int i = 0; i < m_results.size(); ++i) {
            FinePositioningResult::Status status = m_results[i].status;
            if (status == FinePositioningResult::Pending || 
                status == FinePositioningResult::Failed) {
                targetIndex = i;
                m_currentIndex = i;
                qDebug() << QString("[BatchFinePositioning] 找到待处理的拍照位，索引: %1").arg(targetIndex);
                break;
            }
        }
    }
    
    // 如果仍然找不到，系统内部自动使用索引1（blockIndex从1开始）
    if (targetIndex < 0) {
        qDebug() << "[BatchFinePositioning] 系统内部自动确定blockIndex为1";
        targetIndex = 1;
        m_currentIndex = 1;
        
        // 如果结果列表为空，创建临时结果（索引1）
        if (m_results.isEmpty()) {
            // 先创建索引0（占位），再创建索引1
            FinePositioningResult tempResult0;
            tempResult0.blockIndex = 0;
            tempResult0.status = FinePositioningResult::Pending;
            m_results.append(tempResult0);
            
            FinePositioningResult tempResult1;
            tempResult1.blockIndex = 1;
            tempResult1.status = FinePositioningResult::WaitingWms;
            m_results.append(tempResult1);
            qDebug() << "[BatchFinePositioning] 创建临时结果，索引1";
        } else {
            // 确保索引1存在，如果不存在则创建
            if (m_results.size() <= 1) {
                // 如果只有索引0，创建索引1
                while (m_results.size() <= 1) {
                    FinePositioningResult tempResult;
                    tempResult.blockIndex = m_results.size();
                    tempResult.status = (m_results.size() == 1) ? FinePositioningResult::WaitingWms : FinePositioningResult::Pending;
                    m_results.append(tempResult);
                }
                qDebug() << "[BatchFinePositioning] 创建临时结果，索引1";
            } else {
                // 更新索引1的结果状态
                m_results[1].status = FinePositioningResult::WaitingWms;
            }
        }
    }

    // 更新当前索引
    m_currentIndex = targetIndex;
    
    // 确保结果列表中有当前索引的结果
    if (m_currentIndex >= m_results.size()) {
        // 扩展结果列表到当前索引
        while (m_results.size() <= m_currentIndex) {
            FinePositioningResult tempResult;
            tempResult.blockIndex = m_results.size();
            tempResult.status = FinePositioningResult::WaitingWms;
            m_results.append(tempResult);
        }
        qDebug() << QString("[BatchFinePositioning] 扩展结果列表到索引: %1").arg(m_currentIndex);
    }
    
    FinePositioningResult &result = m_results[m_currentIndex];
    
    // 根据精定位请求次数决定下一步操作
    if (result.positioningRequestCount == 1) {
        if (m_testMode) {
            // 测试模式：第一次移动完成，发送第二次移动请求（坐标+2）
            result.positioningRequestCount = 2;
            result.status = FinePositioningResult::Positioning;
            
            // 计算第二次请求的坐标（原始坐标+2）
            double secondX = result.firstRequestX + 2.0;
            double secondY = result.firstRequestY + 2.0;
            double secondZ = result.firstRequestZ + 2.0;
            double secondDeg = result.firstRequestDeg;  // 角度不变
            
            qDebug() << QString("[BatchFinePositioning] (测试模式) 收到第一次移动完成通知，发送第二次移动请求（坐标+2）")
                     << QString("  拍照位索引: %1").arg(m_currentIndex)
                     << QString("  第一次坐标: (%2, %3, %4, %5)")
                        .arg(result.firstRequestX, 0, 'f', 1)
                        .arg(result.firstRequestY, 0, 'f', 1)
                        .arg(result.firstRequestZ, 0, 'f', 1)
                        .arg(result.firstRequestDeg, 0, 'f', 2)
                     << QString("  第二次坐标: (%1, %2, %3, %4)")
                        .arg(secondX, 0, 'f', 1)
                        .arg(secondY, 0, 'f', 1)
                        .arg(secondZ, 0, 'f', 1)
                        .arg(secondDeg, 0, 'f', 2);
            
            // 发送第二次移动请求
            emit requestAdjustPosition(m_currentIndex, secondX, secondY, secondZ, secondDeg);
            
            // 重新启动超时定时器
            m_wmsMoveTimeoutTimer->start();
        } else {
            // 正常模式：第一次移动完成后直接进入拍照流程（坐标由真实算法提供）
            qDebug() << QString("[BatchFinePositioning] (正常模式) 收到第一次移动完成通知，直接开始相机拍照，索引: %1")
                        .arg(m_currentIndex);
            result.positioningRequestCount = 2;
            result.status = FinePositioningResult::WaitingWms;
            QTimer::singleShot(m_captureDelayMs, this, [this, idx = m_currentIndex]() {
                emit requestCameraCapture(idx);
            });
        }
    } else if (result.positioningRequestCount == 2) {
        // 第二次移动完成，触发相机拍照
        result.status = FinePositioningResult::WaitingWms;
        
        qDebug() << QString("[BatchFinePositioning] 收到第二次移动完成通知，开始相机拍照")
                 << QString("  拍照位索引: %1").arg(m_currentIndex);

        // 触发相机拍照（带延时）
        QTimer::singleShot(m_captureDelayMs, this, [this, idx = m_currentIndex]() {
            emit requestCameraCapture(idx);
        });
    } else {
        // 异常情况：请求次数不是1或2
        qWarning() << QString("[BatchFinePositioning] 收到WMS移动完成通知，但请求次数异常: %1，拍照位索引: %2")
                      .arg(result.positioningRequestCount).arg(m_currentIndex);
        // 默认触发相机拍照
        result.status = FinePositioningResult::WaitingWms;
        emit requestCameraCapture(m_currentIndex);
    }
}

void BatchFinePositioningManager::onCameraImageReady(int blockIndex, const QImage &image)
{
    if (blockIndex != m_currentIndex) {
        qWarning() << QString("[BatchFinePositioning] 相机图像blockIndex不匹配: 期望=%1, 实际=%2")
                      .arg(m_currentIndex).arg(blockIndex);
        return;
    }

    // 如果图像为空且当前为正常模式，直接报错（不允许使用假数据）
    if (image.isNull() && !m_testMode) {
        qWarning() << "[BatchFinePositioning] 相机图像为空（正常模式），无法生成精定位结果";
        onFinePositioningResult(blockIndex, false,
                                0, 0, 0, 0,
                                0, 0, 0, 0,
                                "相机图像为空，无法获取精定位数据");
        return;
    }

    // 测试模式：直接使用假的坐标，跳过相机算法
    if (m_testMode) {
        // blockIndex默认从1开始（如果为0，使用1）
        int actualBlockIndex = blockIndex > 0 ? blockIndex : 1;
        qDebug() << QString("[BatchFinePositioning] 测试模式：跳过相机算法，直接使用假的坐标，blockIndex: %1 -> %2")
                    .arg(blockIndex).arg(actualBlockIndex);
        
        ApiTypes::Point3D testCoord;
        // 测试坐标索引从1开始（索引0对应blockIndex=1）
        int testCoordIndex = actualBlockIndex - 1;
        
        if (testCoordIndex >= 0 && testCoordIndex < m_testCoordinates.size()) {
            // 使用指定的测试坐标
            testCoord = m_testCoordinates[testCoordIndex];
            qDebug() << QString("[BatchFinePositioning] 测试模式：使用指定的测试坐标，索引: %1").arg(testCoordIndex);
        } else if (blockIndex < m_photoPoses.size()) {
            // 如果没有指定测试坐标，使用拍照位坐标（加一个小的偏移模拟精定位结果）
            testCoord = m_photoPoses[blockIndex];
            testCoord.x += 2.0;  // 模拟精定位后的坐标偏移
            testCoord.y += 2.0;
            testCoord.z += 2.0;
            testCoord.deg += 1.0;
            qDebug() << QString("[BatchFinePositioning] 测试模式：使用拍照位坐标+偏移，blockIndex: %1").arg(blockIndex);
        } else if (!m_photoPoses.isEmpty()) {
            // 如果blockIndex超出范围，使用第一个拍照位坐标
            testCoord = m_photoPoses[0];
            testCoord.x += 2.0;
            testCoord.y += 2.0;
            testCoord.z += 2.0;
            testCoord.deg += 1.0;
            qDebug() << QString("[BatchFinePositioning] 测试模式：blockIndex超出范围，使用第一个拍照位坐标");
        } else {
            // 如果都没有，使用默认坐标
            testCoord.x = 16487.0;
            testCoord.y = 1796.0;
            testCoord.z = 4216.0;
            testCoord.deg = 90.0;
            qDebug() << "[BatchFinePositioning] 测试模式：使用默认测试坐标";
        }
        
        qDebug() << QString("[BatchFinePositioning] 测试模式：模拟坐标 (%1, %2, %3, %4)")
                    .arg(testCoord.x, 0, 'f', 1)
                    .arg(testCoord.y, 0, 'f', 1)
                    .arg(testCoord.z, 0, 'f', 1)
                    .arg(testCoord.deg, 0, 'f', 2);
        
        // 模拟精定位结果：直接返回可抓取，使用actualBlockIndex作为blockIndex
        onFinePositioningResult(actualBlockIndex, true,
                              testCoord.x, testCoord.y, testCoord.z, testCoord.deg,
                              0, 0, 0, 0,
                              QString());  // 清空错误信息，表示成功
        return;
    }

    qDebug() << QString("[BatchFinePositioning] 收到相机图像，拍照位 #%1, 图像尺寸: %2x%3")
                .arg(blockIndex)
                .arg(image.width())
                .arg(image.height());

    // 如果没有相机算法
    if (!m_cameraAlgorithm) {
        if (m_testMode) {
            qDebug() << "[BatchFinePositioning] 相机算法未设置（测试模式），使用模拟结果";
            // 使用拍照位坐标作为模拟结果（如果有的话）
            if (blockIndex < m_photoPoses.size()) {
                ApiTypes::Point3D mockCoord = m_photoPoses[blockIndex];
                mockCoord.x += 2.0;  // 模拟精定位后的坐标偏移
                mockCoord.y += 2.0;
                mockCoord.z += 2.0;
                mockCoord.deg += 1.0;
                onFinePositioningResult(blockIndex, true, 
                                        mockCoord.x, mockCoord.y, mockCoord.z, mockCoord.deg,
                                        0, 0, 0, 0,
                                        QString());  // 清空错误信息，表示成功
            } else {
                // 如果没有拍照位坐标，使用默认坐标
                onFinePositioningResult(blockIndex, true,
                                        16487.0, 1796.0, 4216.0, 90.0,
                                        0, 0, 0, 0,
                                        QString());  // 清空错误信息，表示成功
            }
        } else {
            qWarning() << "[BatchFinePositioning] 相机算法未设置（正常模式），无法继续精定位";
            onFinePositioningResult(blockIndex, false,
                                    0, 0, 0, 0,
                                    0, 0, 0, 0,
                                    "相机算法未设置");
        }
        return;
    }

    // 调用相机算法（如果有的话）
    try {
        CameraAlgorithm::FinePositioningResult algResult = m_cameraAlgorithm->performFinePositioning(image);
        
        // 如果算法返回了编号，使用算法的编号作为blockIndex；否则使用传入的blockIndex
        int actualBlockIndex = blockIndex;
        if (algResult.blockNumber >= 0) {
            actualBlockIndex = algResult.blockNumber;
            qDebug() << QString("[BatchFinePositioning] 算法返回铜垛编号: %1，使用此编号作为blockIndex").arg(actualBlockIndex);
        } else {
            qDebug() << QString("[BatchFinePositioning] 算法未返回编号，使用拍照位索引: %1").arg(blockIndex);
        }
        
        onFinePositioningResult(actualBlockIndex,
                               algResult.canGrab,
                               algResult.grabX, algResult.grabY, algResult.grabZ, algResult.grabDeg,
                               algResult.adjustX, algResult.adjustY, algResult.adjustZ, algResult.adjustDeg,
                               algResult.errorMessage);
    } catch (const std::exception &e) {
        qWarning() << "[BatchFinePositioning] 相机算法调用异常:" << e.what();
        if (m_testMode) {
            // 算法异常时（测试模式），也使用模拟结果
            if (blockIndex < m_photoPoses.size()) {
                ApiTypes::Point3D mockCoord = m_photoPoses[blockIndex];
                mockCoord.x += 2.0;
                mockCoord.y += 2.0;
                mockCoord.z += 2.0;
                mockCoord.deg += 1.0;
                onFinePositioningResult(blockIndex, true,
                                        mockCoord.x, mockCoord.y, mockCoord.z, mockCoord.deg,
                                        0, 0, 0, 0,
                                        QString());  // 清空错误信息，表示成功
            } else {
                onFinePositioningResult(blockIndex, true,
                                        16487.0, 1796.0, 4216.0, 90.0,
                                        0, 0, 0, 0,
                                        QString());  // 清空错误信息，表示成功
            }
        } else {
            // 正常模式下直接返回失败
            onFinePositioningResult(blockIndex, false,
                                    0, 0, 0, 0,
                                    0, 0, 0, 0,
                                    QString("相机算法异常: %1").arg(e.what()));
        }
    }
}

void BatchFinePositioningManager::onFinePositioningResult(int blockIndex,
                                                          bool canGrab,
                                                          double grabX, double grabY, double grabZ, double grabDeg,
                                                          double adjustX, double adjustY, double adjustZ, double adjustDeg,
                                                          const QString &errorMessage)
{
    // 如果blockIndex不匹配，但blockIndex有效，更新m_currentIndex
    if (blockIndex != m_currentIndex) {
        if (blockIndex > 0 && blockIndex < m_results.size()) {
            qDebug() << QString("[BatchFinePositioning] 精定位结果blockIndex不匹配，更新当前索引: %1 -> %2")
                        .arg(m_currentIndex).arg(blockIndex);
            m_currentIndex = blockIndex;
        } else {
            qWarning() << QString("[BatchFinePositioning] 精定位结果blockIndex不匹配: 期望=%1, 实际=%2")
                          .arg(m_currentIndex).arg(blockIndex);
            return;
        }
    }

    if (!errorMessage.isEmpty()) {
        qWarning() << QString("[BatchFinePositioning] 相机算法错误: %1").arg(errorMessage);
        // 相机算法错误，不向WMS发送坐标
        updatePositioningResult(blockIndex, false, 0, 0, 0, 0, 0, 0, 0, 0, errorMessage);
        // 标记为失败，继续处理下一个
        m_results[blockIndex].status = FinePositioningResult::Failed;
        m_currentIndex++;
        processNextPhotoPose();
        return;
    }

    if (canGrab) {
        // 可抓取，暂存抓取坐标
        qDebug() << QString("[BatchFinePositioning] 拍照位 #%1 精定位完成，可抓取，坐标:(%2, %3, %4, %5)")
                    .arg(blockIndex)
                    .arg(grabX, 0, 'f', 1)
                    .arg(grabY, 0, 'f', 1)
                    .arg(grabZ, 0, 'f', 1)
                    .arg(grabDeg, 0, 'f', 2);

        updatePositioningResult(blockIndex, true, grabX, grabY, grabZ, grabDeg, 0, 0, 0, 0);
        
        // 标记为完成
        m_results[blockIndex].status = FinePositioningResult::Completed;
        m_results[blockIndex].isCompleted = true;
        m_results[blockIndex].meetsGrabCondition = true;
        m_results[blockIndex].completedTime = QDateTime::currentDateTime();

        // 更新进度
        int completed = getCompletedCount();
        emit progressUpdated(completed, m_results.size(), blockIndex);

        // 在测试模式下，立即上报（不等待所有拍照位完成）
        if (m_testMode) {
            qDebug() << QString("[BatchFinePositioning] 测试模式：精定位完成，立即上报假坐标到 ReceiveUnloadPositionData 接口");
            reportBatchToWms();
            return;
        }

        // 处理下一个拍照位
        m_currentIndex++;
        processNextPhotoPose();
    } else {
        // 不可抓取，使用调整坐标重复精定位（受AlignMax限制）
        qDebug() << QString("[BatchFinePositioning] 拍照位 #%1 不可抓取，使用调整坐标重复精定位，调整坐标:(%2, %3, %4, %5)")
                    .arg(blockIndex)
                    .arg(adjustX, 0, 'f', 1)
                    .arg(adjustY, 0, 'f', 1)
                    .arg(adjustZ, 0, 'f', 1)
                    .arg(adjustDeg, 0, 'f', 2);

        // AlignMax：超过最大重试次数则判定失败
        m_results[blockIndex].retryCount++;
        if (m_results[blockIndex].retryCount > m_alignMax) {
            qWarning() << QString("[BatchFinePositioning] 超过最大重试次数(AlignMax=%1)，标记失败，索引:%2")
                          .arg(m_alignMax)
                          .arg(blockIndex);
            m_results[blockIndex].status = FinePositioningResult::Failed;
            m_results[blockIndex].errorMessage = QStringLiteral("精定位重试超过上限");
            m_currentIndex++;
            processNextPhotoPose();
            return;
        }

        // 调用WMS写入调整坐标
        emit requestAdjustPosition(blockIndex, adjustX, adjustY, adjustZ, adjustDeg);

        // 重新启动超时定时器
        m_wmsMoveTimeoutTimer->start();
    }
}

void BatchFinePositioningManager::updatePositioningResult(int blockIndex, bool canGrab,
                                                          double grabX, double grabY, double grabZ, double grabDeg,
                                                          double adjustX, double adjustY, double adjustZ, double adjustDeg,
                                                          const QString &errorMessage)
{
    if (blockIndex < 0 || blockIndex >= m_results.size()) {
        return;
    }

    FinePositioningResult &result = m_results[blockIndex];
    
    if (canGrab) {
        result.grabX = grabX;
        result.grabY = grabY;
        result.grabZ = grabZ;
        result.grabDeg = grabDeg;
    }

    if (!errorMessage.isEmpty()) {
        result.errorMessage = errorMessage;
    }
}

void BatchFinePositioningManager::checkAndHandleCompletion()
{
    if (areAllCompleted()) {
        m_isRunning = false;
        qDebug() << "[BatchFinePositioning] 所有拍照位精定位完成";
        
        // 批量上报到WMS
        reportBatchToWms();
    } else {
        // 检查是否有失败的拍照位
        int failedCount = 0;
        for (const auto &result : m_results) {
            if (result.status == FinePositioningResult::Failed) {
                failedCount++;
            }
        }
        
        if (failedCount > 0) {
            qWarning() << QString("[BatchFinePositioning] 有 %1 个拍照位精定位失败").arg(failedCount);
        }
    }
}

void BatchFinePositioningManager::onWmsMoveTimeout()
{
    if (!m_isRunning) {
        return;
    }

    if (m_currentIndex >= 0 && m_currentIndex < m_results.size()) {
        qWarning() << QString("[BatchFinePositioning] 长时间未收到WMS移动完成通知，拍照位 #%1")
                      .arg(m_currentIndex);
        
        // 标记为失败
        m_results[m_currentIndex].status = FinePositioningResult::Failed;
        m_results[m_currentIndex].errorMessage = "WMS移动超时（120秒）";
        
        // 继续处理下一个
        m_currentIndex++;
        processNextPhotoPose();
    }
}

bool BatchFinePositioningManager::areAllCompleted() const
{
    for (const auto &result : m_results) {
        if (result.status != FinePositioningResult::Completed && 
            result.status != FinePositioningResult::Failed) {
            return false;
        }
    }
    return true;
}

void BatchFinePositioningManager::reportBatchToWms()
{
    // 收集所有已完成的抓取坐标
    QList<ApiTypes::Point3D> grabCoordinates;
    
    for (const auto &result : m_results) {
        if (result.status == FinePositioningResult::Completed && result.meetsGrabCondition) {
            ApiTypes::Point3D point;
            point.x = result.grabX;
            point.y = result.grabY;
            point.z = result.grabZ;
            point.deg = result.grabDeg;
            point.hasDeg = true;
            point.flag = true;
            point.pointType = QStringLiteral("grabPose");
            point.blockIndex = result.blockIndex;
            grabCoordinates.append(point);
        }
    }

    // 在测试模式下，如果没有可上报的坐标，使用假的测试坐标
    if (grabCoordinates.isEmpty() && m_testMode) {
        qDebug() << "[BatchFinePositioning] 测试模式：没有完成的结果，使用假的测试坐标上报";
        ApiTypes::Point3D testPoint;
        testPoint.x = 16487.0;
        testPoint.y = 1796.0;
        testPoint.z = 4216.0;
        testPoint.deg = 90.0;
        testPoint.hasDeg = true;
        testPoint.flag = true;
        testPoint.pointType = QStringLiteral("grabPose");
        testPoint.blockIndex = 1;  // 测试模式下使用blockIndex=1
        grabCoordinates.append(testPoint);
        qDebug() << QString("[BatchFinePositioning] 测试模式：添加假的测试坐标 (%1, %2, %3, %4)")
                    .arg(testPoint.x, 0, 'f', 1)
                    .arg(testPoint.y, 0, 'f', 1)
                    .arg(testPoint.z, 0, 'f', 1)
                    .arg(testPoint.deg, 0, 'f', 2);
    }

    if (grabCoordinates.isEmpty()) {
        qWarning() << "[BatchFinePositioning] 没有可上报的抓取坐标";
        emit batchPositioningFailed("没有可上报的抓取坐标");
        return;
    }

    qDebug() << QString("[BatchFinePositioning] 准备批量上报 %1 个抓取坐标到 ReceiveUnloadPositionData 接口").arg(grabCoordinates.size());

    // 构建批量上报的ScanResult
    ApiTypes::ScanResult scanResult;
    scanResult.uniqueCode = QString("BATCH_%1").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    scanResult.type = "1";
    scanResult.code = 1;
    scanResult.message = m_testMode ? "测试模式：批量抓取坐标已生成（使用假坐标）" : "批量抓取坐标已生成";
    scanResult.data = grabCoordinates;

    // 输出上报的坐标信息（用于调试）
    for (int i = 0; i < grabCoordinates.size(); ++i) {
        const auto &point = grabCoordinates[i];
        qDebug() << QString("[BatchFinePositioning] 上报坐标 #%1: blockIndex=%2, x=%3, y=%4, z=%5, deg=%6")
                    .arg(i + 1)
                    .arg(point.blockIndex)
                    .arg(point.x, 0, 'f', 1)
                    .arg(point.y, 0, 'f', 1)
                    .arg(point.z, 0, 'f', 1)
                    .arg(point.deg, 0, 'f', 2);
    }

    // 发送信号，由Widget处理上报
    emit batchPositioningCompleted(scanResult);
}

int BatchFinePositioningManager::getCompletedCount() const
{
    int count = 0;
    for (const auto &result : m_results) {
        if (result.status == FinePositioningResult::Completed) {
            count++;
        }
    }
    return count;
}

int BatchFinePositioningManager::getTotalCount() const
{
    return m_results.size();
}

