#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <QString>
#include <QImage>
#include <QVector>
#include "../api/apitypes.h"

/**
 * @file imageutils.h
 * @brief 图像处理工具类
 * 
 * 提供：
 * 1. QImage与Base64相互转换
 * 2. 在图像上标注坐标点（⚠️ Phase 2实现）
 * 3. 生成结果可视化图（⚠️ Phase 2实现）
 */
class ImageUtils
{
public:
    /**
     * 将QImage转换为Base64字符串（PNG格式）
     * @param image 输入图像
     * @return Base64编码的字符串
     * 
     * 使用示例：
     *   QImage img = ...;
     *   QString base64 = ImageUtils::imageToBase64(img);
     */
    static QString imageToBase64(const QImage &image);
    
    /**
     * 将Base64字符串转换为QImage（用于测试验证）
     * @param base64 Base64编码的字符串
     * @return 解码后的图像
     */
    static QImage base64ToImage(const QString &base64);
    
    // ========================================
    // ⚠️ 以下功能在 Phase 2 实现（当前为占位）
    // ========================================
    
    /**
     * 在图像上标注XYZ坐标点
     * @param baseImage 基础图像（相机拍摄的纹理图或雷达渲染图）
     * @param points 坐标点数组
     * @param scanType 扫描类型 ("1"=雷达, "2"=相机)
     * @return 标注后的图像
     * 
     * ⚠️ 后续需要确认：
     * - 标注样式（圆点大小、颜色、字体）
     * - 坐标显示格式（是否要单位、精度）
     * - 雷达扫描时如何生成baseImage（点云渲染？）
     */
    static QImage drawCoordinates(const QImage &baseImage, 
                                   const QVector<ApiTypes::Point3D> &points,
                                   const QString &scanType);
    
    /**
     * 生成纯结果图（无原图，只有坐标可视化）
     * @param points 坐标点数组
     * @param width 图像宽度
     * @param height 图像高度
     * @return 生成的结果图
     * 
     * ⚠️ 用于雷达扫描时无法提供原图的情况
     */
    static QImage generateResultVisualization(const QVector<ApiTypes::Point3D> &points,
                                               int width = 1920,
                                               int height = 1080);
};

#endif // IMAGEUTILS_H








