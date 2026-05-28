#ifndef CAMERACONTROLDIALOG_H
#define CAMERACONTROLDIALOG_H

#include <QDialog>
#include <QString>
#include "phoxicontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class CameraControlDialog;
}
QT_END_NAMESPACE

class CameraControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraControlDialog(PhoxiController *camera1Controller,
                                  PhoxiController *camera2Controller,
                                  QWidget *parent = nullptr);
    ~CameraControlDialog();

private slots:
    // 相机1控制槽
    void onCamera1ConnectClicked();
    void onCamera1DisconnectClicked();
    void onCamera1StartCaptureClicked();
    void onCamera1StopCaptureClicked();
    void onCamera1TriggerClicked();
    
    // 相机2控制槽
    void onCamera2ConnectClicked();
    void onCamera2DisconnectClicked();
    void onCamera2StartCaptureClicked();
    void onCamera2StopCaptureClicked();
    void onCamera2TriggerClicked();
    
    // 设置槽
    void onApplySettingsClicked();
    void onClearLogClicked();
    
    // 状态更新槽
    void onCamera1StatusChanged(PhoxiStatus status);
    void onCamera1InfoUpdated(const PhoxiCameraInfo &info);
    void onCamera1ConnectionResult(bool success, const QString &message);
    void onCamera1Error(const QString &error);
    
    void onCamera2StatusChanged(PhoxiStatus status);
    void onCamera2InfoUpdated(const PhoxiCameraInfo &info);
    void onCamera2ConnectionResult(bool success, const QString &message);
    void onCamera2Error(const QString &error);

private:
    void setupConnections();
    void updateCamera1UI();
    void updateCamera2UI();
    void logMessage(const QString &message);

private:
    Ui::CameraControlDialog *ui;
    
    PhoxiController *m_camera1Controller;
    PhoxiController *m_camera2Controller;
};

#endif // CAMERACONTROLDIALOG_H



