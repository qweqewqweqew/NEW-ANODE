#include "heatmapwidget.h"
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QApplication>

HeatmapWidget::HeatmapWidget(QWidget *parent)
    : QWidget(parent)
    , m_generator(nullptr)
    , m_autoFit(true)
    , m_showGrid(false)
    , m_showColorBar(false)
    , m_zoomLevel(1.0)
    , m_offset(0, 0)
    , m_panning(false)
{
    // 纯净显示 - 无UI控件
    setStyleSheet("QWidget { background-color: black; }");
    setMinimumSize(400, 300);
    
    // 注意：m_imageLabel 未使用，已移除相关代码
}

HeatmapWidget::~HeatmapWidget()
{
}

void HeatmapWidget::setHeatmap(const QImage &heatmap)
{
    m_heatmap = heatmap;
    updateDisplay();
}

void HeatmapWidget::setHeatmapGenerator(HeatmapGenerator *generator)
{
    if (m_generator) {
        disconnect(m_generator, nullptr, this, nullptr);
    }
    
    m_generator = generator;
    
    if (m_generator) {
        connect(m_generator, &HeatmapGenerator::heatmapUpdated,
                this, &HeatmapWidget::onHeatmapUpdated);
        connect(m_generator, &HeatmapGenerator::statisticsUpdated,
                this, &HeatmapWidget::onStatisticsUpdated);
    }
}

void HeatmapWidget::setAutoFit(bool enabled)
{
    m_autoFit = enabled;
    updateDisplay();
}

void HeatmapWidget::setShowGrid(bool enabled)
{
    m_showGrid = enabled;
    update();
}

void HeatmapWidget::setShowColorBar(bool enabled)
{
    m_showColorBar = enabled;
    update();
}

void HeatmapWidget::setZoomLevel(double zoom)
{
    m_zoomLevel = qBound(0.1, zoom, 10.0);
    emit zoomChanged(m_zoomLevel);
    updateDisplay();
}

void HeatmapWidget::resetView()
{
    m_zoomLevel = 1.0;
    m_offset = QPointF(0, 0);
    updateDisplay();
}

void HeatmapWidget::onHeatmapUpdated(const QImage &heatmap)
{
    setHeatmap(heatmap);
}

void HeatmapWidget::onStatisticsUpdated(const HeatmapGenerator::Statistics &stats)
{
    m_statistics = stats;
    update();
}


void HeatmapWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 纯净显示 - 只绘制热力图
    painter.fillRect(rect(), Qt::black);
    drawHeatmap(painter);
}

void HeatmapWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    updateDisplay();
}

// setupUI方法已删除 - 纯净显示不需要UI控件

void HeatmapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_panning = true;
        m_lastPanPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void HeatmapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panning) {
        QPointF delta = event->pos() - m_lastPanPos;
        m_offset += delta;
        m_lastPanPos = event->pos();
        updateDisplay();
    }
}

void HeatmapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
    }
}

void HeatmapWidget::wheelEvent(QWheelEvent *event)
{
    double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        setZoomLevel(m_zoomLevel * scaleFactor);
    } else {
        setZoomLevel(m_zoomLevel / scaleFactor);
    }
}

void HeatmapWidget::updateDisplay()
{
    update();
}

void HeatmapWidget::drawHeatmap(QPainter &painter)
{
    if (m_heatmap.isNull()) {
        return;
    }
    
    // 计算显示区域
    QRect displayRect = rect();
    
    // 计算缩放后的图像大小
    QSize scaledSize = m_heatmap.size() * m_zoomLevel;
    
    // 如果启用自动适应，计算合适的缩放比例（保持比例，允许留白）
    if (m_autoFit) {
        double scaleX = (double)displayRect.width() / m_heatmap.width();
        double scaleY = (double)displayRect.height() / m_heatmap.height();
        // 使用较小的缩放比例，确保图像完全显示，保持比例
        double autoScale = qMin(scaleX, scaleY) * 0.95;  // 留一些边距
        scaledSize = m_heatmap.size() * autoScale;
        m_zoomLevel = autoScale;
    }
    
    // 创建缩放后的Pixmap并绘制
    QPixmap displayPixmap = QPixmap::fromImage(m_heatmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    // 计算绘制位置（居中+偏移）
    QPoint drawPos = displayRect.center() - displayPixmap.rect().center() + m_offset.toPoint();
    
    // 绘制热力图
    painter.drawPixmap(drawPos, displayPixmap);
}

QPointF HeatmapWidget::widgetToImage(const QPoint &widgetPos) const
{
    if (m_heatmap.isNull()) {
        return QPointF();
    }
    
    QRect displayRect = rect();
    QSize scaledSize = m_heatmap.size() * m_zoomLevel;
    QPoint drawPos = displayRect.center() - QRect(QPoint(0, 0), scaledSize).center() + m_offset.toPoint();
    
    QPointF relativePos = widgetPos - drawPos;
    return QPointF(relativePos.x() / m_zoomLevel, relativePos.y() / m_zoomLevel);
}

QPoint HeatmapWidget::imageToWidget(const QPointF &imagePos) const
{
    if (m_heatmap.isNull()) {
        return QPoint();
    }
    
    QRect displayRect = rect();
    QSize scaledSize = m_heatmap.size() * m_zoomLevel;
    QPoint drawPos = displayRect.center() - QRect(QPoint(0, 0), scaledSize).center() + m_offset.toPoint();
    
    QPointF scaledPos = imagePos * m_zoomLevel;
    return (scaledPos + QPointF(drawPos)).toPoint();
}

