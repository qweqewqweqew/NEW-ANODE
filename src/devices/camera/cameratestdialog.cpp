#include "cameratestdialog.h"
#include "anodealgorithmworker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QDateTime>
#include <QMetaObject>
#include <QMessageBox>

CameraTestDialog::CameraTestDialog(AnodeAlgorithmWorker *algorithmWorker, QWidget *parent)
    : QDialog(parent)
    , m_algorithmWorker(algorithmWorker)
{
    setWindowTitle(tr("相机算法测试"));
    resize(500, 250);
    
    setupUI();
    
    // 连接算法worker信号
    if (m_algorithmWorker) {
        connect(m_algorithmWorker, &AnodeAlgorithmWorker::testDualCameraFinished,
                this, &CameraTestDialog::onTestFinished);
        connect(m_algorithmWorker, &AnodeAlgorithmWorker::testDualCameraFailed,
                this, &CameraTestDialog::onTestFailed);
        connect(m_algorithmWorker, &AnodeAlgorithmWorker::testDualCameraProgress,
                this, [this](const QString &msg) {
                    emit logToMainWindow(msg, "INFO");
                });
    }
    
    logMessage("相机算法测试对话框已打开", "INFO");
    logMessage("请按顺序选择相机1和相机2的图像文件", "INFO");
}

CameraTestDialog::~CameraTestDialog()
{
}

void CameraTestDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 文件选择区域
    QGroupBox *fileGroup = new QGroupBox(tr("双相机图像选择"));
    QVBoxLayout *fileLayout = new QVBoxLayout(fileGroup);
    
    // 相机1
    QHBoxLayout *camera1Layout = new QHBoxLayout();
    QLabel *camera1Label = new QLabel(tr("相机1:"));
    camera1Label->setMinimumWidth(60);
    m_selectCamera1Btn = new QPushButton(tr("选择图像..."));
    camera1Layout->addWidget(camera1Label);
    camera1Layout->addWidget(m_selectCamera1Btn);
    fileLayout->addLayout(camera1Layout);
    
    // 相机2
    QHBoxLayout *camera2Layout = new QHBoxLayout();
    QLabel *camera2Label = new QLabel(tr("相机2:"));
    camera2Label->setMinimumWidth(60);
    m_selectCamera2Btn = new QPushButton(tr("选择图像..."));
    camera2Layout->addWidget(camera2Label);
    camera2Layout->addWidget(m_selectCamera2Btn);
    fileLayout->addLayout(camera2Layout);
    
    mainLayout->addWidget(fileGroup);
    
    // 状态提示
    m_statusLabel = new QLabel(tr("提示：选择*_IMG_Texture_8Bit.png格式的文件\n日志将输出到主窗口"));
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet("QLabel { color: #666; padding: 10px; }");
    mainLayout->addWidget(m_statusLabel);
    
    mainLayout->addStretch();
    
    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_runTestBtn = new QPushButton(tr("运行测试"));
    m_runTestBtn->setEnabled(false);
    m_runTestBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 10px; }");
    
    QPushButton *closeBtn = new QPushButton(tr("关闭"));
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_runTestBtn);
    buttonLayout->addWidget(closeBtn);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(m_selectCamera1Btn, &QPushButton::clicked, this, &CameraTestDialog::onSelectCamera1BtnClicked);
    connect(m_selectCamera2Btn, &QPushButton::clicked, this, &CameraTestDialog::onSelectCamera2BtnClicked);
    connect(m_runTestBtn, &QPushButton::clicked, this, &CameraTestDialog::onRunTestBtnClicked);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
}

void CameraTestDialog::logMessage(const QString &message, const QString &level)
{
    // 发送信号到主窗口
    emit logToMainWindow(message, level);
}

void CameraTestDialog::onSelectCamera1BtnClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("选择相机1图像文件 (*_IMG_Texture_8Bit.png)"),
        QDir::currentPath(),
        tr("PNG Files (*_IMG_Texture_8Bit.png);;All Files (*.*)")
    );
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // 提取基础路径
    QString basePath = filePath;
    basePath.replace("_IMG_Texture_8Bit.png", "");
    
    if (!validateFiles(basePath, "相机1")) {
        return;
    }
    
    m_camera1BasePath = basePath;
    m_selectCamera1Btn->setText(tr("相机1: %1").arg(QFileInfo(filePath).fileName()));
    m_selectCamera1Btn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; }");
    
    logMessage(QString("相机1路径已设置: %1").arg(basePath), "SUCCESS");
    QImage preview(filePath);
    if (!preview.isNull()) {
        emit cameraImageSelected(1, preview);
    }
    
    // 检查是否可以运行测试
    if (!m_camera1BasePath.isEmpty() && !m_camera2BasePath.isEmpty()) {
        m_runTestBtn->setEnabled(true);
    }
}

void CameraTestDialog::onSelectCamera2BtnClicked()
{
    QString startDir = m_camera1BasePath.isEmpty() ? QDir::currentPath() : QFileInfo(m_camera1BasePath).absolutePath();
    
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("选择相机2图像文件 (*_IMG_Texture_8Bit.png)"),
        startDir,
        tr("PNG Files (*_IMG_Texture_8Bit.png);;All Files (*.*)")
    );
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // 提取基础路径
    QString basePath = filePath;
    basePath.replace("_IMG_Texture_8Bit.png", "");
    
    if (!validateFiles(basePath, "相机2")) {
        return;
    }
    
    m_camera2BasePath = basePath;
    m_selectCamera2Btn->setText(tr("相机2: %1").arg(QFileInfo(filePath).fileName()));
    m_selectCamera2Btn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; }");
    
    logMessage(QString("相机2路径已设置: %1").arg(basePath), "SUCCESS");
    QImage preview(filePath);
    if (!preview.isNull()) {
        emit cameraImageSelected(2, preview);
    }
    
    // 检查是否可以运行测试
    if (!m_camera1BasePath.isEmpty() && !m_camera2BasePath.isEmpty()) {
        m_runTestBtn->setEnabled(true);
    }
}

bool CameraTestDialog::validateFiles(const QString &basePath, const QString &cameraName)
{
    QStringList requiredFiles;
    requiredFiles << basePath + "_IMG_Texture_8Bit.png"
                  //<< basePath + "_IMG_Texture.tif"
                  << basePath + "_IMG_NormalMap_Z.tif";
    
    for (const QString &file : requiredFiles) {
        if (!QFile::exists(file)) {
            logMessage(QString("%1缺少文件: %2").arg(cameraName, QFileInfo(file).fileName()), "ERROR");
            QMessageBox::warning(this, tr("文件验证失败"),
                               QString("%1图像文件不完整，缺少:\n%2").arg(cameraName, QFileInfo(file).fileName()));
            return false;
        }
    }
    
    return true;
}

void CameraTestDialog::onRunTestBtnClicked()
{
    if (m_camera1BasePath.isEmpty() || m_camera2BasePath.isEmpty()) {
        logMessage("请先选择两个相机的图像文件", "ERROR");
        return;
    }
    
    if (!m_algorithmWorker) {
        logMessage("算法工作线程未初始化", "ERROR");
        return;
    }
    
    logMessage("========== 开始相机算法测试 ==========", "INFO");
    logMessage(QString("相机1: %1").arg(m_camera1BasePath), "INFO");
    logMessage(QString("相机2: %1").arg(m_camera2BasePath), "INFO");
    
    QStringList imageBasePaths;
    imageBasePaths << m_camera1BasePath << m_camera2BasePath;
    
    m_runTestBtn->setEnabled(false);
    m_runTestBtn->setText(tr("测试运行中..."));
    
    QMetaObject::invokeMethod(m_algorithmWorker, "processTestDualCamera",
                              Qt::QueuedConnection,
                              Q_ARG(QStringList, imageBasePaths));
    
    logMessage("测试任务已提交到算法线程", "INFO");

    // 预处理图像展示：尝试加载 NormalMap 作为预处理结果
    auto emitPreprocessed = [this](int camIdx, const QString &basePath) {
        QString prePath = basePath + "_IMG_NormalMap_Z.tif";
        QImage preImg(prePath);
        if (!preImg.isNull()) {
            emit cameraPreprocessedReady(camIdx, preImg);
        }
    };
    emitPreprocessed(1, m_camera1BasePath);
    emitPreprocessed(2, m_camera2BasePath);

    // 测试运行中：隐藏窗口但保持对象存活，等待算法回调
    this->hide();
}

void CameraTestDialog::onTestFinished(const s_AccurateADPlateARtsPara &detectResult,
                                     const s_CalcAccurateAlignRtsPara &calcResult,
                                     const s_CalcAccurateAlignPara &calcPara)
{
    m_runTestBtn->setEnabled(true);
    m_runTestBtn->setText(tr("运行测试"));
    
    // 完全仿照MFC程序的日志格式输出（参考MFC Line 1417-1495）
    
    // 定位检测耗时
    logMessage(QString("相机：定位检测 耗时=%1 ms").arg(static_cast<int>(detectResult.time * 1000)), "INFO");
    
    // 层错值
    logMessage(QString("相机： 层错值=[%1,%2 ]")
               .arg(detectResult.fTbLayerOuterDis1, 0, 'f', 1)
               .arg(detectResult.fTbLayerOuterDis2, 0, 'f', 1), "INFO");
    
    if (detectResult.bExitTbLayerOuter) {
        logMessage("相机：存在层错超限制.", "WARNING");
    }
    
    // 安全距离检测
    if (detectResult.bExitSafeDisTd) {
        logMessage(QString("相机：存在铜跺安全距离超限制 两侧距离=[%1  ,%2]  ")
                   .arg(detectResult.fDefectTdNearRtsDis1, 0, 'f', 1)
                   .arg(detectResult.fDefectTdNearRtsDis2, 0, 'f', 1), "WARNING");
    }
    
    // 尺寸和角度超限
    if (detectResult.bExitLimitSize) {
        logMessage("相机：存在角度和尺寸数据超限制.", "WARNING");
    }
    
    // 夹爪区域空间干涉
    if (detectResult.bExistDefectGripSpace) {
        logMessage("相机：存在夹爪区域空间干涉.", "WARNING");
        for (size_t i = 0; i < detectResult.iDefectGripSpaceRtsListPtsSel.size(); i++) {
            logMessage(QString("相机：干涉点云数量点=[%1] 长度=[%2]")
                       .arg(detectResult.iDefectGripSpaceRtsListPtsSel[i])
                       .arg(detectResult.fDefectGripSpaceRtsListLSel[i], 0, 'f', 1), "WARNING");
        }
    }
    
    // 铜跺尺寸（第一次输出）
    logMessage(QString("相机：铜跺尺寸 L=[%1] W==[%2 ] H=[%3] ")
               .arg(detectResult.TdL, 0, 'f', 1)
               .arg(detectResult.TdW, 0, 'f', 1)
               .arg(detectResult.TdH, 0, 'f', 1), "INFO");
    
    // 综合判定
    logMessage(QString("相机：综合判定=%1  ").arg(detectResult.bTJG ? "OK" : "NG"), 
               detectResult.bTJG ? "SUCCESS" : "WARNING");
    
    if (!detectResult.bTJG) {
        logMessage("相机：综合判定NG，不抓取", "WARNING");
        return;
    }
    
    // 铜跺抓取位置（工具坐标系下 - detectResult中的）
    logMessage(QString("相机：铜跺抓取位置 TdX=[%1] TdY=[%2 ] TdZ=[%3] TdDeg==[%4 ]  ")
               .arg(detectResult.TdX, 0, 'f', 1)
               .arg(detectResult.TdY, 0, 'f', 1)
               .arg(detectResult.TdZ, 0, 'f', 1)
               .arg(detectResult.TdDeg, 0, 'f', 1), "INFO");
    
    // 铜跺方向（第一次输出 - detectResult中的）
    logMessage(QString("相机：铜跺方向  |0:0度方向 1:90度方向  2:180度方向:3：270度方向| iTdEarDir=[%1]   ")
               .arg(detectResult.TdEarDir), "INFO");
    
    // 精定位计算结果
    logMessage(QString("相机：精定位计算结果 iStatus==%1").arg(calcResult.iStatus), "INFO");
    
    // 相机基准拍照位（从输入参数读取，与MFC Line 1273一致）
    logMessage(QString("相机：相机基准拍照位SX=[%1] SY==[%2 ] SZ=[%3] SDeg==[%4 ]  ")
               .arg(calcPara.fSX, 0, 'f', 1)
               .arg(calcPara.fSY, 0, 'f', 1)
               .arg(calcPara.fSZ, 0, 'f', 1)
               .arg(calcPara.fSDeg, 0, 'f', 1), "INFO");
    
    // 相机当前拍照位（从输入参数读取，与MFC Line 1276一致）
    logMessage(QString("相机：相机当前拍照位X=[%1] Y==[%2 ] Z=[%3] Deg==[%4 ]  ")
               .arg(calcPara.fCameraX, 0, 'f', 1)
               .arg(calcPara.fCameraY, 0, 'f', 1)
               .arg(calcPara.fCameraZ, 0, 'f', 1)
               .arg(calcPara.fCameraDeg, 0, 'f', 1), "INFO");
    
    // 对位偏差
    logMessage(QString("相机：对位偏差dX=[%1] dY==[%2 ] dZ=[%3] dDeg==[%4 ]  ")
               .arg(calcResult.dX, 0, 'f', 1)
               .arg(calcResult.dY, 0, 'f', 1)
               .arg(calcResult.dZ, 0, 'f', 1)
               .arg(calcResult.dDeg, 0, 'f', 1), "INFO");
    
    // 下一个拍照位坐标
    logMessage(QString("相机：下一个拍照位坐标 NextCameraX=[%1] NextCameraY==[%2 ] NextCameraZ=[%3] NextCameraDeg==[%4 ]  ")
               .arg(calcResult.fNextCameraX, 0, 'f', 1)
               .arg(calcResult.fNextCameraY, 0, 'f', 1)
               .arg(calcResult.fNextCameraZ, 0, 'f', 1)
               .arg(calcResult.fNextCameraDeg, 0, 'f', 1), "INFO");
    
    // 抓取位坐标（天车坐标系下）
    logMessage(QString("相机：抓取位坐标 fGripX=[%1] fGripY==[%2 ] fGripZ=[%3] fGripDeg==[%4 ]  ")
               .arg(calcResult.fGripX, 0, 'f', 1)
               .arg(calcResult.fGripY, 0, 'f', 1)
               .arg(calcResult.fGripZ, 0, 'f', 1)
               .arg(calcResult.fGripDeg, 0, 'f', 1), "SUCCESS");
    
    // 铜跺方向（重复输出，与MFC保持一致）
    logMessage(QString("相机：铜跺方向  |0:0度方向 1:90度方向  2:180度方向:3：270度方向| iTdEarDir=[%1]   ")
               .arg(calcResult.iTdEarDir), "INFO");
    
    // 铜跺尺寸（重复输出，与MFC保持一致）
    logMessage(QString("相机：铜跺尺寸 L=[%1] W==[%2 ] H=[%3] ")
               .arg(calcResult.TdL, 0, 'f', 1)
               .arg(calcResult.TdW, 0, 'f', 1)
               .arg(calcResult.TdH, 0, 'f', 1), "INFO");
    
    // 状态判断
    QString statusMsg;
    if (calcResult.iStatus == 1) {
        statusMsg = "相机：对位完成.";
        logMessage(statusMsg, "SUCCESS");
    } else if (calcResult.iStatus == 2) {
        statusMsg = "相机：对位失败.";
        logMessage(statusMsg, "ERROR");
    } else if (calcResult.iStatus == 5) {
        statusMsg = "相机：继续对位.";
        logMessage(statusMsg, "WARNING");
    } else {
        statusMsg = "相机：对位状态异常.";
        logMessage(statusMsg, "ERROR");
    }
}

void CameraTestDialog::onTestFailed(const QString &error)
{
    m_runTestBtn->setEnabled(true);
    m_runTestBtn->setText(tr("运行测试"));
    
    logMessage("========== 相机算法测试失败 ==========", "ERROR");
    logMessage(QString("错误信息: %1").arg(error), "ERROR");
    logMessage("========================================", "ERROR");
    
    QMessageBox::critical(this, tr("测试失败"), QString("算法测试失败:\n%1").arg(error));
}

