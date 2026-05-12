#ifndef CAMERATESTDIALOG_H
#define CAMERATESTDIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include "DataX.h"
#include <QImage>

class AnodeAlgorithmWorker;
class QPushButton;
class QLabel;

/**
 * @brief 相机算法测试对话框
 * 独立封装双相机测试功能，日志输出到主窗口
 */
class CameraTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraTestDialog(AnodeAlgorithmWorker *algorithmWorker, QWidget *parent = nullptr);
    ~CameraTestDialog();

signals:
    void logToMainWindow(const QString &message, const QString &level);
    void cameraImageSelected(int cameraIndex, const QImage &image);
    void cameraPreprocessedReady(int cameraIndex, const QImage &image);

private slots:
    void onSelectCamera1BtnClicked();
    void onSelectCamera2BtnClicked();
    void onRunTestBtnClicked();
    
    // 算法回调
    void onTestFinished(const s_AccurateADPlateARtsPara &detectResult,
                       const s_CalcAccurateAlignRtsPara &calcResult,
                       const s_CalcAccurateAlignPara &calcPara);
    void onTestFailed(const QString &error);

private:
    void setupUI();
    void logMessage(const QString &message, const QString &level = "INFO");
    bool validateFiles(const QString &basePath, const QString &cameraName);

private:
    AnodeAlgorithmWorker *m_algorithmWorker;
    
    // UI组件
    QPushButton *m_selectCamera1Btn;
    QPushButton *m_selectCamera2Btn;
    QPushButton *m_runTestBtn;
    QLabel *m_statusLabel;
    
    // 数据
    QString m_camera1BasePath;
    QString m_camera2BasePath;
};

#endif // CAMERATESTDIALOG_H

