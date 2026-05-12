#include "cameracontroldialog.h"
#include "ui_cameracontroldialog.h"
#include <QDateTime>
#include <QDebug>

CameraControlDialog::CameraControlDialog(PhoxiController *camera1Controller,
                                           PhoxiController *camera2Controller,
                                           QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CameraControlDialog)
    , m_camera1Controller(camera1Controller)
    , m_camera2Controller(camera2Controller)
{
    ui->setupUi(this);
    setupConnections();
    updateCamera1UI();
    updateCamera2UI();
    
    logMessage("相机控制面板启动");
}

CameraControlDialog::~CameraControlDialog()
{
    delete ui;
}

void CameraControlDialog::setupConnections()
{
    // 相机1控制按钮连接
    connect(ui->camera1ConnectBtn, &QPushButton::clicked, this, &CameraControlDialog::onCamera1ConnectClicked);
    connect(ui->camera1DisconnectBtn, &QPushButton::clicked, this, &CameraControlDialog::onCamera1DisconnectClicked);
    connect(ui->camera1StartCaptureBtn, &QPushButton::clicked, this, &CameraControlDialog::onCamera1StartCaptureClicked);
    connect(ui->camera1StopCaptureBtn, &QPushButton::clicked, this, &CameraControlDialog::onCamera1StopCaptureClicked);
    connect(ui->camera1TriggerBtn, &QPushButton::clicked, this, &CameraControlDialog::onCamera1TriggerClicked);
    
    // 相机2控制按钮连接
    connect(ui->camera2ConnectBtn, &QPushButton::clicked, this, &CameraControlDialog::onCamera2ConnectClicked);
    connect(ui->camera2DisconnectBtn, &QPushButton::clicked, this, &CameraControlDialog::onCamera2DisconnectClicked);
    connect(ui->camera2StartCaptureBtn, &QPushButton::clicked, this, &CameraControlDialog::onCamera2StartCaptureClicked);
    connect(ui->camera2StopCaptureBtn, &QPushButton::clicked, this, &CameraControlDialog::onCamera2StopCaptureClicked);
    connect(ui->camera2TriggerBtn, &QPushButton::clicked, this, &CameraControlDialog::onCamera2TriggerClicked);
    
    // 设置按钮连接
    connect(ui->applySettingsBtn, &QPushButton::clicked, this, &CameraControlDialog::onApplySettingsClicked);
    connect(ui->clearLogBtn, &QPushButton::clicked, this, &CameraControlDialog::onClearLogClicked);
    connect(ui->closeBtn, &QPushButton::clicked, this, &QDialog::close);
    
    // 相机1状态信号连接
    if (m_camera1Controller) {
        connect(m_camera1Controller, &PhoxiController::statusChanged,
                this, &CameraControlDialog::onCamera1StatusChanged);
        connect(m_camera1Controller, &PhoxiController::cameraInfoUpdated,
                this, &CameraControlDialog::onCamera1InfoUpdated);
        connect(m_camera1Controller, &PhoxiController::connectionResult,
                this, &CameraControlDialog::onCamera1ConnectionResult);
        connect(m_camera1Controller, &PhoxiController::errorOccurred,
                this, &CameraControlDialog::onCamera1Error);
    }
    
    // 相机2状态信号连接
    if (m_camera2Controller) {
        connect(m_camera2Controller, &PhoxiController::statusChanged,
                this, &CameraControlDialog::onCamera2StatusChanged);
        connect(m_camera2Controller, &PhoxiController::cameraInfoUpdated,
                this, &CameraControlDialog::onCamera2InfoUpdated);
        connect(m_camera2Controller, &PhoxiController::connectionResult,
                this, &CameraControlDialog::onCamera2ConnectionResult);
        connect(m_camera2Controller, &PhoxiController::errorOccurred,
                this, &CameraControlDialog::onCamera2Error);
    }
}

// 相机1控制槽实现
void CameraControlDialog::onCamera1ConnectClicked()
{
    if (m_camera1Controller) {
        logMessage("正在连接相机1...");
        m_camera1Controller->connectCamera();
    }
}

void CameraControlDialog::onCamera1DisconnectClicked()
{
    if (m_camera1Controller) {
        logMessage("正在断开相机1...");
        m_camera1Controller->disconnectCamera();
    }
}

void CameraControlDialog::onCamera1StartCaptureClicked()
{
    if (m_camera1Controller) {
        logMessage("相机1开始采集");
        m_camera1Controller->startCapture();
    }
}

void CameraControlDialog::onCamera1StopCaptureClicked()
{
    if (m_camera1Controller) {
        logMessage("相机1停止采集");
        m_camera1Controller->stopCapture();
    }
}

void CameraControlDialog::onCamera1TriggerClicked()
{
    if (m_camera1Controller) {
        logMessage("触发相机1单帧采集");
        m_camera1Controller->triggerFrame();
    }
}

// 相机2控制槽实现
void CameraControlDialog::onCamera2ConnectClicked()
{
    if (m_camera2Controller) {
        logMessage("正在连接相机2...");
        m_camera2Controller->connectCamera();
    }
}

void CameraControlDialog::onCamera2DisconnectClicked()
{
    if (m_camera2Controller) {
        logMessage("正在断开相机2...");
        m_camera2Controller->disconnectCamera();
    }
}

void CameraControlDialog::onCamera2StartCaptureClicked()
{
    if (m_camera2Controller) {
        logMessage("相机2开始采集");
        m_camera2Controller->startCapture();
    }
}

void CameraControlDialog::onCamera2StopCaptureClicked()
{
    if (m_camera2Controller) {
        logMessage("相机2停止采集");
        m_camera2Controller->stopCapture();
    }
}

void CameraControlDialog::onCamera2TriggerClicked()
{
    if (m_camera2Controller) {
        logMessage("触发相机2单帧采集");
        m_camera2Controller->triggerFrame();
    }
}

// 设置槽实现
void CameraControlDialog::onApplySettingsClicked()
{
    int timeout = ui->timeoutSpin->value();
    int resolution = ui->resolutionCombo->currentIndex();
    
    if (m_camera1Controller) {
        m_camera1Controller->setTimeout(timeout);
        m_camera1Controller->setResolution(resolution);
    }
    
    if (m_camera2Controller) {
        m_camera2Controller->setTimeout(timeout);
        m_camera2Controller->setResolution(resolution);
    }
    
    logMessage(QString("已应用设置: 超时=%1ms, 分辨率=%2").arg(timeout).arg(resolution));
}

void CameraControlDialog::onClearLogClicked()
{
    ui->logText->clear();
    logMessage("日志已清除");
}

// 相机1状态更新槽
void CameraControlDialog::onCamera1StatusChanged(PhoxiStatus status)
{
    updateCamera1UI();
    
    QString statusText;
    switch (status) {
    case PhoxiStatus::Disconnected:
        statusText = "未连接";
        break;
    case PhoxiStatus::Connecting:
        statusText = "连接中";
        break;
    case PhoxiStatus::Connected:
        statusText = "已连接";
        break;
    case PhoxiStatus::Capturing:
        statusText = "采集中";
        break;
    case PhoxiStatus::Error:
        statusText = "错误";
        break;
    }
    
    ui->camera1StatusLabel->setText(QString("状态: %1").arg(statusText));
    logMessage(QString("相机1状态: %1").arg(statusText));
}

void CameraControlDialog::onCamera1InfoUpdated(const PhoxiCameraInfo &info)
{
    ui->camera1InfoLabel->setText(QString("相机ID: %1").arg(info.id));
    logMessage(QString("相机1信息: %1 (%2)").arg(info.name).arg(info.serialNumber));
}

void CameraControlDialog::onCamera1ConnectionResult(bool success, const QString &message)
{
    if (success) {
        logMessage(QString("相机1: %1").arg(message));
    } else {
        logMessage(QString("相机1错误: %1").arg(message));
    }
}

void CameraControlDialog::onCamera1Error(const QString &error)
{
    logMessage(QString("相机1错误: %1").arg(error));
}

// 相机2状态更新槽
void CameraControlDialog::onCamera2StatusChanged(PhoxiStatus status)
{
    updateCamera2UI();
    
    QString statusText;
    switch (status) {
    case PhoxiStatus::Disconnected:
        statusText = "未连接";
        break;
    case PhoxiStatus::Connecting:
        statusText = "连接中";
        break;
    case PhoxiStatus::Connected:
        statusText = "已连接";
        break;
    case PhoxiStatus::Capturing:
        statusText = "采集中";
        break;
    case PhoxiStatus::Error:
        statusText = "错误";
        break;
    }
    
    ui->camera2StatusLabel->setText(QString("状态: %1").arg(statusText));
    logMessage(QString("相机2状态: %1").arg(statusText));
}

void CameraControlDialog::onCamera2InfoUpdated(const PhoxiCameraInfo &info)
{
    ui->camera2InfoLabel->setText(QString("相机ID: %1").arg(info.id));
    logMessage(QString("相机2信息: %1 (%2)").arg(info.name).arg(info.serialNumber));
}

void CameraControlDialog::onCamera2ConnectionResult(bool success, const QString &message)
{
    if (success) {
        logMessage(QString("相机2: %1").arg(message));
    } else {
        logMessage(QString("相机2错误: %1").arg(message));
    }
}

void CameraControlDialog::onCamera2Error(const QString &error)
{
    logMessage(QString("相机2错误: %1").arg(error));
}

// UI更新方法
void CameraControlDialog::updateCamera1UI()
{
    if (!m_camera1Controller) {
        return;
    }
    
    PhoxiStatus status = m_camera1Controller->getStatus();
    bool isConnected = m_camera1Controller->isConnected();
    bool isCapturing = m_camera1Controller->isCapturing();
    
    ui->camera1ConnectBtn->setEnabled(!isConnected);
    ui->camera1DisconnectBtn->setEnabled(isConnected);
    ui->camera1StartCaptureBtn->setEnabled(isConnected && !isCapturing);
    ui->camera1StopCaptureBtn->setEnabled(isCapturing);
    ui->camera1TriggerBtn->setEnabled(isConnected);
}

void CameraControlDialog::updateCamera2UI()
{
    if (!m_camera2Controller) {
        return;
    }
    
    PhoxiStatus status = m_camera2Controller->getStatus();
    bool isConnected = m_camera2Controller->isConnected();
    bool isCapturing = m_camera2Controller->isCapturing();
    
    ui->camera2ConnectBtn->setEnabled(!isConnected);
    ui->camera2DisconnectBtn->setEnabled(isConnected);
    ui->camera2StartCaptureBtn->setEnabled(isConnected && !isCapturing);
    ui->camera2StopCaptureBtn->setEnabled(isCapturing);
    ui->camera2TriggerBtn->setEnabled(isConnected);
}

void CameraControlDialog::logMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    ui->logText->append(QString("[%1] %2").arg(timestamp).arg(message));
}



