#ifndef HEATMAPWIDGET_H
#define HEATMAPWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QSlider>
#include <QProgressBar>
#include "heatmapgenerator.h"

// 热力图显示控件
class HeatmapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HeatmapWidget(QWidget *parent = nullptr);
    ~HeatmapWidget();

    // 设置热力图
    void setHeatmap(const QImage &heatmap);
    void setHeatmapGenerator(HeatmapGenerator *generator);

    // 显示控制
    void setAutoFit(bool enabled);
    void setShowGrid(bool enabled);
    void setShowColorBar(bool enabled);
    void setZoomLevel(double zoom);
    void resetView();

    // 获取当前状态
    bool isAutoFit() const { return m_autoFit; }
    bool isShowGrid() const { return m_showGrid; }
    bool isShowColorBar() const { return m_showColorBar; }
    double getZoomLevel() const { return m_zoomLevel; }

signals:
    void zoomChanged(double zoom);
    void positionChanged(const QPointF &position);

private slots:
    void onHeatmapUpdated(const QImage &heatmap);
    void onStatisticsUpdated(const HeatmapGenerator::Statistics &stats);

private:
    // 绘制相关
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    
    // 内部方法
    void updateDisplay();
    void drawHeatmap(QPainter &painter);
    QPointF widgetToImage(const QPoint &widgetPos) const;
    QPoint imageToWidget(const QPointF &imagePos) const;

private:
    // 显示数据
    QImage m_heatmap;
    QPixmap m_displayPixmap;
    HeatmapGenerator *m_generator;
    HeatmapGenerator::Statistics m_statistics;
    QLabel *m_imageLabel;

    // 显示控制
    bool m_autoFit;
    bool m_showGrid;
    bool m_showColorBar;
    double m_zoomLevel;
    QPointF m_offset;
    QPointF m_lastPanPos;
    bool m_panning;

    // 颜色条
    QImage m_colorBar;
    static const int COLOR_BAR_WIDTH = 20;
    static const int COLOR_BAR_HEIGHT = 200;

    // 无UI控件 - 纯净显示
};

#endif // HEATMAPWIDGET_H

