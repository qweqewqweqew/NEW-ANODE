#pragma once

#include <windows.h> // 包含Windows API所需的头文件
#include "DataX.h"
#include "ThreeDPlatformData.h"


class CHVisionAdvX
{

public:
    CHVisionAdvX();
    virtual ~CHVisionAdvX();


  
    bool is_directory_exists(const std::string& path);

    bool is_file_exists(const std::string& path);

    bool create_directories_recursive(const std::string& path);

    //int ReadImage3DX(s_Image3dS& imgTable,
    //    std::string sFullPathNameRemoveSuffix,  //图像全路径去掉后缀及类型
    //    std::string strGraySuffix = "_IMG_Texture_8Bit",      //灰度图后缀
    //    std::string strPointCloudSuffix = "_IMG_PointCloud_",  //点云后缀
    //    std::string strNormalSuffix = "_IMG_NormalMap_",     //法线后缀
    //    std::string strColorSuffix = "_IMG_Color"     //彩色图后缀
    //);
    

    int ReadImage3DX(s_Image3dS& imgTable,
        std::string sFullPathNameRemoveSuffix,  //图像全路径去掉后缀及类型
        std::string strGraySuffix = "_IMG_Texture_8Bit",      //灰度图后缀
        std::string strPointCloudSuffix = "_IMG_PointCloud_",  //点云后缀
        std::string strNormalSuffix = "_IMG_NormalMap_"     //法线后缀
       
    );

    int  WriteImage3DX(s_Image3dS imgTable, std::string szFolderPath, std::string sNameRemoveSuffix);

    void gen_arrow_contour_xld(HObject* ho_Arrow, HTuple hv_Row1, HTuple hv_Column1, HTuple hv_Row2, HTuple hv_Column2, HTuple hv_HeadLength, HTuple hv_HeadWidth);

    void scale_image_range(HObject ho_Image, HObject* ho_ImageScaled, HTuple hv_Min, HTuple hv_Max);


    //int  WriteDefectPlateBRtsImage(s_DefectPlateBRtsPara Rts, std::string szFolderPath, std::string sNameRemoveSuffix);



    int ReadLidarDevDataX(Double3DPointVec& d3dPoints, int  MapWidth,  std::string sFullPathName);
 

	int TransLidarDevData2Lidar3ds(Double3DPointVec d3dPoints,s_LidarTrans sLidarTrans, s_Lidar3d &imageT);




};

