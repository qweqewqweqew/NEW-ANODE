#include "heatmapcontroldialog.h"

HeatmapControlDialog::HeatmapControlDialog(HeatmapGenerator *generator,
                                           HeatmapWidget *viewer,
                                           QWidget *parent)
    : QDialog(parent)
    , m_generator(generator)
    , m_viewer(viewer)
{
    setWindowTitle("热力图控制");
    setModal(false);
    setupUI();
    syncFromGenerator();
}

void HeatmapControlDialog::setupUI()
{
    QGridLayout *layout = new QGridLayout(this);

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItems({"高度", "强度", "密度"});
    m_gridWidthSpin = new QSpinBox(this);
    m_gridHeightSpin = new QSpinBox(this);
    m_gridWidthSpin->setRange(100, 2000);
    m_gridHeightSpin->setRange(100, 2000);
    m_minXSpin = new QDoubleSpinBox(this);
    m_maxXSpin = new QDoubleSpinBox(this);
    m_minYSpin = new QDoubleSpinBox(this);
    m_maxYSpin = new QDoubleSpinBox(this);
    for (auto s : {m_minXSpin, m_maxXSpin, m_minYSpin, m_maxYSpin}) {
        s->setDecimals(2);
        s->setRange(-100.0, 100.0);
    }
    m_autoRangeCheck = new QCheckBox("自动范围", this);
    m_autoFitCheck = new QCheckBox("自动适应", this);
    m_showGridCheck = new QCheckBox("显示网格", this);
    m_showColorBarCheck = new QCheckBox("显示颜色条", this);
    m_zoomSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSlider->setRange(10, 1000);
    m_zoomSlider->setValue(100);
    m_zoomLabel = new QLabel("缩放: 100%", this);
    m_resetViewBtn = new QPushButton("重置视图", this);

    int r = 0;
    layout->addWidget(new QLabel("类型:"), r, 0);
    layout->addWidget(m_typeCombo, r, 1);
    layout->addWidget(new QLabel("宽度:"), r, 2);
    layout->addWidget(m_gridWidthSpin, r, 3);
    r++;
    layout->addWidget(new QLabel("高度:"), r, 0);
    layout->addWidget(m_gridHeightSpin, r, 1);
    layout->addWidget(new QLabel("X范围:"), r, 2);
    layout->addWidget(m_minXSpin, r, 3);
    r++;
    layout->addWidget(m_maxXSpin, r, 3);
    layout->addWidget(new QLabel("Y范围:"), r, 2);
    layout->addWidget(m_minYSpin, r, 1);
    r++;
    layout->addWidget(m_maxYSpin, r, 1);
    layout->addWidget(m_autoRangeCheck, r, 2);
    layout->addWidget(m_autoFitCheck, r, 3);
    r++;
    layout->addWidget(m_showGridCheck, r, 2);
    layout->addWidget(m_showColorBarCheck, r, 3);
    r++;
    layout->addWidget(m_zoomLabel, r, 0);
    layout->addWidget(m_zoomSlider, r, 1, 1, 2);
    layout->addWidget(m_resetViewBtn, r, 3);

    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HeatmapControlDialog::onTypeChanged);
    auto connectCfg = [this]() {
        connect(m_gridWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &HeatmapControlDialog::onConfigChanged);
        connect(m_gridHeightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &HeatmapControlDialog::onConfigChanged);
        connect(m_minXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &HeatmapControlDialog::onConfigChanged);
        connect(m_maxXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &HeatmapControlDialog::onConfigChanged);
        connect(m_minYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &HeatmapControlDialog::onConfigChanged);
        connect(m_maxYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &HeatmapControlDialog::onConfigChanged);
    }; connectCfg();
    connect(m_autoRangeCheck, &QCheckBox::toggled, this, &HeatmapControlDialog::onConfigChanged);
    connect(m_autoFitCheck, &QCheckBox::toggled, this, &HeatmapControlDialog::onAutoFitToggled);
    connect(m_showGridCheck, &QCheckBox::toggled, this, &HeatmapControlDialog::onShowGridToggled);
    connect(m_showColorBarCheck, &QCheckBox::toggled, this, &HeatmapControlDialog::onShowColorBarToggled);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &HeatmapControlDialog::onZoomChanged);
    connect(m_resetViewBtn, &QPushButton::clicked, this, &HeatmapControlDialog::onResetView);
}

void HeatmapControlDialog::syncFromGenerator()
{
    if (!m_generator) return;
    HeatmapConfig cfg = m_generator->getConfig();
    m_typeCombo->setCurrentIndex(static_cast<int>(cfg.type));
    m_gridWidthSpin->setValue(cfg.gridWidth);
    m_gridHeightSpin->setValue(cfg.gridHeight);
    m_minXSpin->setValue(cfg.minX);
    m_maxXSpin->setValue(cfg.maxX);
    m_minYSpin->setValue(cfg.minY);
    m_maxYSpin->setValue(cfg.maxY);
    m_autoRangeCheck->setChecked(cfg.autoRange);
    m_autoFitCheck->setChecked(m_viewer ? m_viewer->isAutoFit() : true);
    m_showGridCheck->setChecked(m_viewer ? m_viewer->isShowGrid() : true);
    m_showColorBarCheck->setChecked(m_viewer ? m_viewer->isShowColorBar() : true);
}

void HeatmapControlDialog::onTypeChanged(int idx)
{
    if (!m_generator) return;
    HeatmapConfig cfg = m_generator->getConfig();
    cfg.type = static_cast<HeatmapType>(idx);
    m_generator->setConfig(cfg);
}

void HeatmapControlDialog::onConfigChanged()
{
    if (!m_generator) return;
    HeatmapConfig cfg = m_generator->getConfig();
    cfg.gridWidth = m_gridWidthSpin->value();
    cfg.gridHeight = m_gridHeightSpin->value();
    cfg.minX = m_minXSpin->value();
    cfg.maxX = m_maxXSpin->value();
    cfg.minY = m_minYSpin->value();
    cfg.maxY = m_maxYSpin->value();
    cfg.autoRange = m_autoRangeCheck->isChecked();
    m_generator->setConfig(cfg);
}

void HeatmapControlDialog::onAutoFitToggled(bool on)
{
    if (m_viewer) m_viewer->setAutoFit(on);
}

void HeatmapControlDialog::onShowGridToggled(bool on)
{
    if (m_viewer) m_viewer->setShowGrid(on);
}

void HeatmapControlDialog::onShowColorBarToggled(bool on)
{
    if (m_viewer) m_viewer->setShowColorBar(on);
}

void HeatmapControlDialog::onZoomChanged(int value)
{
    if (m_viewer) m_viewer->setZoomLevel(value / 100.0);
    if (m_zoomLabel) m_zoomLabel->setText(QString("缩放: %1%" ).arg(value));
}

void HeatmapControlDialog::onResetView()
{
    if (m_viewer) m_viewer->resetView();
}
