#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class ParameterSettingsDialog;
}
QT_END_NAMESPACE

class Widget;  // 前向声明

class ParameterSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParameterSettingsDialog(QWidget *parent = nullptr);
    ~ParameterSettingsDialog();

    void setParameters(int imageWidth, double safeDistanceTbNear, double safeDistanceToCbOuter);

    int imageWidth() const;
    double safeDistanceTbNear() const;
    double safeDistanceToCbOuter() const;

private slots:
    void onCameraTestBtnClicked();

private:
    Ui::ParameterSettingsDialog *ui;
};








