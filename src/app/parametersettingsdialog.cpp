#include "parametersettingsdialog.h"
#include "ui_parametersettingsdialog.h"
#include "widget.h"

#include <QDialogButtonBox>
#include <cmath>

ParameterSettingsDialog::ParameterSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ParameterSettingsDialog)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("参数设置"));
    setModal(true);

    ui->imageWidthSpin->setRange(1, 20000);
    ui->safeDistanceTbNearSpin->setRange(0, 10000);
    ui->safeDistanceOuterSpin->setRange(0, 10000);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ParameterSettingsDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ParameterSettingsDialog::reject);
    connect(ui->cameraTestBtn, &QPushButton::clicked, this, &ParameterSettingsDialog::onCameraTestBtnClicked);
}

ParameterSettingsDialog::~ParameterSettingsDialog()
{
    delete ui;
}

void ParameterSettingsDialog::setParameters(int imageWidth, double safeDistanceTbNear, double safeDistanceToCbOuter)
{
    ui->imageWidthSpin->setValue(imageWidth);
    ui->safeDistanceTbNearSpin->setValue(static_cast<int>(std::round(safeDistanceTbNear)));
    ui->safeDistanceOuterSpin->setValue(static_cast<int>(std::round(safeDistanceToCbOuter)));
}

int ParameterSettingsDialog::imageWidth() const
{
    return ui->imageWidthSpin->value();
}

double ParameterSettingsDialog::safeDistanceTbNear() const
{
    return static_cast<double>(ui->safeDistanceTbNearSpin->value());
}

double ParameterSettingsDialog::safeDistanceToCbOuter() const
{
    return static_cast<double>(ui->safeDistanceOuterSpin->value());
}

void ParameterSettingsDialog::onCameraTestBtnClicked()
{
    // 调用主窗口的相机测试对话框
    Widget *mainWidget = qobject_cast<Widget*>(parentWidget());
    if (mainWidget) {
        mainWidget->openCameraTestDialog();
    }
}
