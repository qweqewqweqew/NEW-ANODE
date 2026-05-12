#ifndef HEATMAPCONTROLDIALOG_H
#define HEATMAPCONTROLDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QPushButton>
#include "heatmapgenerator.h"
#include "heatmapwidget.h"

class HeatmapControlDialog : public QDialog {
    Q_OBJECT
public:
    explicit HeatmapControlDialog(HeatmapGenerator *generator,
                                  HeatmapWidget *viewer,
                                  QWidget *parent = nullptr);

private slots:
    void onTypeChanged(int idx);
    void onConfigChanged();
    void onAutoFitToggled(bool on);
    void onShowGridToggled(bool on);
    void onShowColorBarToggled(bool on);
    void onZoomChanged(int value);
    void onResetView();

private:
    void setupUI();
    void syncFromGenerator();

private:
    HeatmapGenerator *m_generator;
    HeatmapWidget *m_viewer;

    QComboBox *m_typeCombo;
    QSpinBox *m_gridWidthSpin;
    QSpinBox *m_gridHeightSpin;
    QDoubleSpinBox *m_minXSpin;
    QDoubleSpinBox *m_maxXSpin;
    QDoubleSpinBox *m_minYSpin;
    QDoubleSpinBox *m_maxYSpin;
    QCheckBox *m_autoRangeCheck;
    QCheckBox *m_autoFitCheck;
    QCheckBox *m_showGridCheck;
    QCheckBox *m_showColorBarCheck;
    QSlider *m_zoomSlider;
    QLabel *m_zoomLabel;
    QPushButton *m_resetViewBtn;
};

#endif // HEATMAPCONTROLDIALOG_H



