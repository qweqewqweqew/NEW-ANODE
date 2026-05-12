#include "camerawidget.h"
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QEvent>

CameraWidget::CameraWidget(const QString &cameraName, QWidget *parent)
    : QWidget(parent)
    , m_cameraName(cameraName)
    , m_imageLabel(new QLabel(this))
{
    setupUI();
}

CameraWidget::~CameraWidget()
{
}

void CameraWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // 设置图像标签 - 黑色背景，居中显示
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet("QLabel { background-color: black; }");
    m_imageLabel->setScaledContents(false);
    
    layout->addWidget(m_imageLabel);
    
    // 使imageLabel可以接收鼠标点击事件
    m_imageLabel->installEventFilter(this);
    
    // 创建默认黑色图像
    QImage defaultImage(640, 480, QImage::Format_RGB32);
    defaultImage.fill(Qt::black);
    setImage(defaultImage);
}

void CameraWidget::setImage(const QImage &image)
{
    m_currentImage = image;
    updateDisplay();
}

void CameraWidget::clearImage()
{
    // 创建纯黑色图像
    QImage blackImage(640, 480, QImage::Format_RGB32);
    blackImage.fill(Qt::black);
    
    m_currentImage = blackImage;
    updateDisplay();
    
    qDebug() << "[CameraWidget]" << m_cameraName << "图像已清除";
}

void CameraWidget::setCameraName(const QString &name)
{
    m_cameraName = name;
}

void CameraWidget::updateDisplay()
{
    if (!m_currentImage.isNull()) {
        // 缩放图像以适应标签大小
        QPixmap pixmap = QPixmap::fromImage(m_currentImage);
        QSize labelSize = m_imageLabel->size();
        
        if (labelSize.width() > 0 && labelSize.height() > 0) {
            pixmap = pixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        
        m_imageLabel->setPixmap(pixmap);
    }
}

void CameraWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateDisplay();
}

bool CameraWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_imageLabel && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            emit cameraClicked(m_cameraName);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

