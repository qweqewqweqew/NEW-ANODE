#include "imageutils.h"
#include <QBuffer>
#include <QPainter>
#include <QFont>

/**
 * QImage转Base64（PNG格式）
 */
QString ImageUtils::imageToBase64(const QImage &image)
{
    if (image.isNull()) {
        return "";
    }
    
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    
    // 保存为PNG格式（无损，支持透明）
    image.save(&buffer, "PNG");
    
    // 转换为Base64
    return QString::fromLatin1(byteArray.toBase64());
}

/**
 * Base64转QImage（用于测试验证）
 */
QImage ImageUtils::base64ToImage(const QString &base64)
{
    if (base64.isEmpty()) {
        return QImage();
    }
    
    QByteArray byteArray = QByteArray::fromBase64(base64.toLatin1());
    QImage image;
    image.loadFromData(byteArray, "PNG");
    
    return image;
}

/**
 * 在图像上标注坐标点
 * ⚠️ Phase 1 暂不实现，返回原图
 */
QImage ImageUtils::drawCoordinates(const QImage &baseImage, 
                                   const QVector<ApiTypes::Point3D> &points,
                                   const QString &scanType)
{
    // ⚠️ Phase 1: 直接返回原图，不做标注
    // Phase 2 将实现：
    // 1. 创建QPainter在baseImage上绘制
    // 2. 遍历points，绘制红色圆点
    // 3. 在每个点旁边绘制文字标签 "(x, y, z)"
    // 4. 根据scanType调整标注样式
    
    Q_UNUSED(points);
    Q_UNUSED(scanType);
    
    return baseImage;
}

/**
 * 生成纯结果图（用于雷达扫描无原图的情况）
 * ⚠️ Phase 1 暂不实现，返回黑色占位图
 */
QImage ImageUtils::generateResultVisualization(const QVector<ApiTypes::Point3D> &points,
                                               int width,
                                               int height)
{
    // ⚠️ Phase 1: 返回黑色占位图
    // Phase 2 将实现：
    // 1. 创建黑色背景图
    // 2. 将3D坐标投影到2D平面
    // 3. 绘制点和标签
    // 4. 添加坐标轴、比例尺等辅助信息
    
    Q_UNUSED(points);
    
    QImage image(width, height, QImage::Format_RGB888);
    image.fill(Qt::black);
    
    // 绘制占位文字
    QPainter painter(&image);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 24));
    painter.drawText(image.rect(), Qt::AlignCenter, 
                     "Result Image\n(Phase 2 Implementation)");
    
    return image;
}








