#include "CHVisionAdvX.h"

CHVisionAdvX::CHVisionAdvX()
{
}


CHVisionAdvX::~CHVisionAdvX()
{
}


bool CHVisionAdvX::is_directory_exists(const std::string& path)
{
    DWORD attrib = GetFileAttributesA(path.c_str());

    // 检查路径的有效性以及是否为目录
    return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool CHVisionAdvX::is_file_exists(const std::string& path) {
    DWORD attrib = GetFileAttributesA(path.c_str());

    // 检查路径是否有效且不是一个目录
    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}



/**
 * @brief 递归创建多级目录（如 "a/b/c"）
 * @param path 要创建的目录路径
 * @return true 创建成功，false 失败
 */
bool CHVisionAdvX::create_directories_recursive(const std::string& path)
{
    std::string current_path = "";
    size_t pos = 0;
    size_t last_pos = 0;

    // 处理绝对路径（如 C:\）或根路径
    if (path.length() > 0 && (path[0] == '\\' || path[0] == '/'))
    {
        // 如果是 "\xxx" 或 "/xxx"，保留根
        pos = path.find_first_of("\\/", 1);
        if (pos == std::string::npos) return true;
        current_path = path.substr(0, pos);
        last_pos = pos;
    }
    else if (path.length() >= 2 && path[1] == ':')
    {
        // 如果是 "C:\xxx"，保留盘符
        pos = path.find_first_of("\\/", 2);
        if (pos == std::string::npos) return true;
        current_path = path.substr(0, pos);
        last_pos = pos;
    }

    while ((pos = path.find_first_of("\\/", last_pos)) != std::string::npos)
    {
        if (pos > last_pos)
        {
            current_path += "\\" + path.substr(last_pos, pos - last_pos);
        }

        // 检查当前子路径是否存在
        DWORD attrib = GetFileAttributesA(current_path.c_str());
        if (attrib == INVALID_FILE_ATTRIBUTES)
        {
            // 不存在，尝试创建
            if (!CreateDirectoryA(current_path.c_str(), nullptr))
            {
                return false;  // 创建失败
            }
        }
        else if (!(attrib & FILE_ATTRIBUTE_DIRECTORY))
        {
            return false;  // 已存在但不是目录
        }

        last_pos = pos + 1;
    }

    // 创建最终路径
    if (last_pos < path.length())
    {
        current_path += "\\" + path.substr(last_pos);
        DWORD attrib = GetFileAttributesA(current_path.c_str());
        if (attrib == INVALID_FILE_ATTRIBUTES)
        {
            return CreateDirectoryA(current_path.c_str(), nullptr);
        }
        return (attrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    return true;
}


int CHVisionAdvX::ReadLidarDevDataX(Double3DPointVec& d3dPoints, int  MapWidth, std::string sFullPath)
{


	//文件是否存在
	if (!is_file_exists(sFullPath))
	{
		return 1;
	}


	try
	{

		HTuple ObjectModel3D, Status;
		ReadObjectModel3d(sFullPath.c_str(), "m", HTuple(), HTuple(), &ObjectModel3D, &Status);


		HObject  ho_X, ho_Y, ho_Z, ho_X1, ho_Y1, ho_Z1;
		HObject  ho_X2, ho_Y2, ho_Z2, ho_X3, ho_Y3, ho_Z3;

		HTuple  hv_ObjectModel3D, hv_Status, hv_Y_Z_Change;



		//原始数据
		ReadObjectModel3d(sFullPath.c_str(), "m", HTuple(), HTuple(), &hv_ObjectModel3D, &hv_Status);


		//Map
		PrepareObjectModel3d(hv_ObjectModel3D, "gen_xyz_mapping", "true", "xyz_map_width", MapWidth);
		ObjectModel3dToXyz(&ho_X, &ho_Y, &ho_Z, hv_ObjectModel3D, "from_xyz_map", HTuple(), HTuple());


		HTuple hWidth, hHeight;
		GetImageSize(ho_Z, &hWidth, &hHeight);

		int   Width = hWidth[0].I();
		int   Height = hHeight[0].I();

		HTuple hRows, hCols;
		GetRegionPoints(ho_Z, &hRows, &hCols);

		HTuple hValX, hValY, hValZ/*, hValI*/;
		GetGrayval(ho_X, hRows, hCols, &hValX);
		GetGrayval(ho_Y, hRows, hCols, &hValY);
		GetGrayval(ho_Z, hRows, hCols, &hValZ);


		d3dPoints.clear();
		for (int iRow = 0; iRow < Height; iRow++)
		{
			std::vector<Point3D> Single3DPointVec;

			for (int iCol = 0; iCol < Width; iCol++)
			{
				Point3D pt;
				pt.X = hValX[Width * iRow + iCol].D();
				pt.Y = hValY[Width * iRow + iCol].D();
				pt.Z = hValZ[Width * iRow + iCol].D();

				Single3DPointVec.push_back(pt);

			}

			d3dPoints.push_back(Single3DPointVec);
		}


	}
	catch (HalconCpp::HException& ex)
	{
		d3dPoints.clear();

		return 2;
	}


	return 0;
}


int CHVisionAdvX::TransLidarDevData2Lidar3ds(Double3DPointVec d3dPoints, s_LidarTrans sLidarTrans, s_Lidar3d &imageT)
{


	if (d3dPoints.size() == 0)
	{
		return 1;
	}


	// 获取宽高
	int height = d3dPoints.size();    // 行数（高）
	int width = d3dPoints[0].size();    // 列数（宽）
	int  totalPoints = width * height;

	// 在堆上分配数组
	float* ArrayX = new float[totalPoints];
	float* ArrayY = new float[totalPoints];
	float* ArrayZ = new float[totalPoints];
	float* ArrayI = new float[totalPoints];


	for (int iRow = 0; iRow < height; iRow++)
	{
		std::vector<Point3D> Single3DPointVec;
		for (int iCol = 0; iCol < width; iCol++)
		{
			ArrayX[iRow * width + iCol] = d3dPoints[iRow][iCol].X;
			ArrayY[iRow * width + iCol] = d3dPoints[iRow][iCol].Y;
			ArrayZ[iRow * width + iCol] = d3dPoints[iRow][iCol].Z;
			ArrayI[iRow * width + iCol] = d3dPoints[iRow][iCol].RSSI; //16位
		}
	}




	try
	{


		// 转换成halcon格式map图像
		HObject ho_X, ho_Y, ho_Z/*, ho_NX, ho_NY, ho_NZ*/;
		HObject hImageGray;

		GenImage1(&ho_X, "real", width, height, (Hlong)ArrayX);
		GenImage1(&ho_Y, "real", width, height, (Hlong)ArrayY);
		GenImage1(&ho_Z, "real", width, height, (Hlong)ArrayZ);
		GenImage1(&hImageGray, "uint2", width, height, (Hlong)ArrayI);


		if (sLidarTrans.Map_Rotate == true)
		{
			RotateImage(ho_X, &ho_X, 90, "constant");
			RotateImage(ho_Y, &ho_Y, 90, "constant");
			RotateImage(ho_Z, &ho_Z, 90, "constant");
			RotateImage(hImageGray, &hImageGray, 90, "constant");

		}



		HObject   ho_X1, ho_Y1, ho_Z1;
		HObject  ho_X2, ho_Y2, ho_Z2, ho_X3, ho_Y3, ho_Z3;

		if (sLidarTrans.Y_Z_Change == true)
		{
			ho_X1 = ho_X;
			ho_Y1 = ho_Z;
			ho_Z1 = ho_Y;
		}
		else
		{
			ho_X1 = ho_X;
			ho_Y1 = ho_Y;
			ho_Z1 = ho_Z;
		}



		if (sLidarTrans.X_Y_Change == true)
		{

			ho_X2 = ho_Y1;
			ho_Y2 = ho_X1;
			ho_Z2 = ho_Z1;
		}
		else
		{
			ho_X2 = ho_X1;
			ho_Y2 = ho_Y1;
			ho_Z2 = ho_Z1;
		}


		if (sLidarTrans.X_mirro == true)
		{
			ScaleImage(ho_X2, &ho_X3, -1, 0);
		}
		else
		{
			ho_X3 = ho_X2;
		}

		if (sLidarTrans.Y_mirro == true)
		{
			ScaleImage(ho_Y2, &ho_Y3, -1, 0);
		}
		else
		{
			ho_Y3 = ho_Y2;
		}
		ho_Z3 = ho_Z2;



		//添加到3ds
		s_Lidar3d As;
		As.Intensity = hImageGray.Clone();
		As.X = ho_X3.Clone();
		As.Y = ho_Y3.Clone();
		As.Z = ho_Z3.Clone();

		//As.NX = ho_NX.Clone();
		//As.NY = ho_NY.Clone();
		//As.NZ = ho_NZ.Clone();

       /// WriteImage(ho_Z3, "tiff", 0, "ho_Z");


		imageT = As;


	}
	catch (HalconCpp::HException& ex)
	{

		delete[] ArrayX;
		delete[] ArrayY;
		delete[] ArrayZ;
		delete[] ArrayI;

		ArrayX = nullptr;
		ArrayY = nullptr;
		ArrayZ = nullptr;
		ArrayI = nullptr;

		return 2;

	}


	delete[] ArrayX;
	delete[] ArrayY;
	delete[] ArrayZ;
	delete[] ArrayI;

	ArrayX = nullptr;
	ArrayY = nullptr;
	ArrayZ = nullptr;
	ArrayI = nullptr;



	return 0;

}


//int CHVisionAdvX::ReadImage3DX(s_Image3dS& imgTable,
//    std::string sFullPathNameRemoveSuffix,  //图像全路径去掉后缀及类型
//    std::string strGraySuffix ,      //灰度图后缀
//    std::string strPointCloudSuffix ,  //点云后缀
//    std::string strNormalSuffix,     //法线后缀
//    std::string strColorSuffix //彩色图后缀
//)
//{
//    std::string sFullPathGray = sFullPathNameRemoveSuffix + strGraySuffix + ".png";
//
//
//    //文件是否存在
//    if (!is_file_exists(sFullPathGray))
//    {
//        return 1;
//    }
//
//
//    //读取原始图像数据
//    HObject ho_X, ho_Y, ho_Z, ho_NX;
//    HObject ho_NY, ho_NZ, ho_Texture, ho_GrayImage, ho_ColorImage;
//    HObject ho_Region;
//
//
//    HTuple hv_ObjectModel3D;
//    HTuple hv_Rows, hv_Columns;
//    HTuple hv_NXV, hv_NYV, hv_NZV;
//    HTuple hv_Intensity;
//    HTuple hv_RV, hv_GV, hv_BV;
//
//    try
//    {
//        std::string  ss = (sFullPathNameRemoveSuffix + strGraySuffix + ".png").c_str();
//
//        ReadImage(&ho_Texture, (sFullPathNameRemoveSuffix + strGraySuffix + ".png").c_str());
//
//        Rgb1ToGray(ho_Texture, &ho_GrayImage);
//       
//        ReadImage(&ho_ColorImage, (sFullPathNameRemoveSuffix + strColorSuffix + ".png").c_str());
//        ReadImage(&ho_X, (sFullPathNameRemoveSuffix + strPointCloudSuffix + "X.tif").c_str());
//        ReadImage(&ho_Y, (sFullPathNameRemoveSuffix + strPointCloudSuffix + "Y.tif").c_str());
//        ReadImage(&ho_Z, (sFullPathNameRemoveSuffix + strPointCloudSuffix + "Z.tif").c_str());
//
//
//        bool bNormalExist = false;
//        try
//        {
//            ReadImage(&ho_NX, (sFullPathNameRemoveSuffix + strNormalSuffix + "X.tif").c_str());
//            ReadImage(&ho_NY, (sFullPathNameRemoveSuffix + strNormalSuffix + "Y.tif").c_str());
//            ReadImage(&ho_NZ, (sFullPathNameRemoveSuffix + strNormalSuffix + "Z.tif").c_str());
//
//            bNormalExist = true;
//        }
//        catch (HalconCpp::HException& ex)
//        {
//
//        }
//
//
//
//        //生成object3D 并添加各种元素
//
//        GetDomain(ho_Z, &ho_Region);
//        GetRegionPoints(ho_Region, &hv_Rows, &hv_Columns);
//
//        ReduceDomain(ho_GrayImage, ho_Region, &ho_GrayImage);
//        ReduceDomain(ho_X, ho_Region, &ho_X);
//        ReduceDomain(ho_Y, ho_Region, &ho_Y);
//        ReduceDomain(ho_Z, ho_Region, &ho_Z);
//
//        if (bNormalExist)
//        {
//            ReduceDomain(ho_NX, ho_Region, &ho_NX);
//            ReduceDomain(ho_NY, ho_Region, &ho_NY);
//            ReduceDomain(ho_NZ, ho_Region, &ho_NZ);
//        }
//
//
//
//        XyzToObjectModel3d(ho_X, ho_Y, ho_Z, &hv_ObjectModel3D);
//
//
//        //强度信息
//
//        GetGrayval(ho_GrayImage, hv_Rows, hv_Columns, &hv_Intensity);
//        SetObjectModel3dAttribMod(hv_ObjectModel3D, "&intensity", "points", hv_Intensity);
//
//
//
//        //法线信息
//        HObject ho_Image1, ho_Image2;
//        HObject ho_Image3, ho_ImageH, ho_ImageS;
//        HObject ho_ImageV, ho_ImageR, ho_ImageG;
//        HObject ho_ImageB, ho_Multichannel;
//
//        GenEmptyObj(&ho_Image1);
//        GenEmptyObj(&ho_Image2);
//        GenEmptyObj(&ho_Image3);
//        GenEmptyObj(&ho_ImageH);
//        GenEmptyObj(&ho_ImageS);
//        GenEmptyObj(&ho_ImageV);
//        GenEmptyObj(&ho_ImageR);
//        GenEmptyObj(&ho_ImageG);
//        GenEmptyObj(&ho_ImageB);
//        GenEmptyObj(&ho_Multichannel);
//
//
//        if (bNormalExist)
//        {
//
//            GetGrayval(ho_NX, hv_Rows, hv_Columns, &hv_NXV);
//            GetGrayval(ho_NY, hv_Rows, hv_Columns, &hv_NYV);
//            GetGrayval(ho_NZ, hv_Rows, hv_Columns, &hv_NZV);
//
//
//            SetObjectModel3dAttribMod(hv_ObjectModel3D, ((HTuple("point_normal_x").Append("point_normal_y")).Append("point_normal_z")),
//                HTuple(), (hv_NXV.TupleConcat(hv_NYV)).TupleConcat(hv_NZV));
//
//
//            //法线渲染成RGB
//
//
//            ScaleImageMax(ho_NX, &ho_Image1);
//            ScaleImageMax(ho_NY, &ho_Image2);
//            ScaleImageMax(ho_NZ, &ho_Image3);
//            TransFromRgb(ho_Image1, ho_Image2, ho_Image3, &ho_ImageH, &ho_ImageS, &ho_ImageV, "hsv");
//            TransToRgb(ho_ImageH, ho_ImageS, ho_ImageV, &ho_ImageR, &ho_ImageG, &ho_ImageB, "hsv");
//            Compose3(ho_ImageR, ho_ImageG, ho_ImageB, &ho_Multichannel);
//
//            GetGrayval(ho_ImageR, hv_Rows, hv_Columns, &hv_RV);
//            GetGrayval(ho_ImageG, hv_Rows, hv_Columns, &hv_GV);
//            GetGrayval(ho_ImageB, hv_Rows, hv_Columns, &hv_BV);
//            SetObjectModel3dAttribMod(hv_ObjectModel3D, "&red", "points", hv_RV);
//            SetObjectModel3dAttribMod(hv_ObjectModel3D, "&green", "points", hv_GV);
//            SetObjectModel3dAttribMod(hv_ObjectModel3D, "&blue", "points", hv_BV);
//        }
//
//
//        s_Image3dS As;
//        As.Gray = ho_GrayImage.Clone();
//        As.X = ho_X.Clone();
//        As.Y = ho_Y.Clone();
//        As.Z = ho_Z.Clone();
//
//        if (bNormalExist)
//        {
//            As.NX = ho_NX.Clone();
//            As.NY = ho_NY.Clone();
//            As.NZ = ho_NZ.Clone();
//        }
//
//        As.Color= ho_ColorImage.Clone();
//
//        CopyObjectModel3d(hv_ObjectModel3D, "all", &As.ObjectModel3D);
//
//        imgTable = As;
//
//        if (hv_ObjectModel3D.Length() > 0)
//        {
//            ClearObjectModel3d(hv_ObjectModel3D);
//        }
//
//
//
//    }
//    catch (HalconCpp::HException& ex)
//    {
//
//        return -1;
//    }
//
//    return 0;
//
//}


int CHVisionAdvX::ReadImage3DX(s_Image3dS& imgTable,
    std::string sFullPathNameRemoveSuffix,  //图像全路径去掉后缀及类型
    std::string strGraySuffix,      //灰度图后缀
    std::string strPointCloudSuffix,  //点云后缀
    std::string strNormalSuffix     //法线后缀

)
{
    std::string sFullPathGray = sFullPathNameRemoveSuffix + strGraySuffix + ".png";


    //文件是否存在
    if (!is_file_exists(sFullPathGray))
    {
        return 1;
    }


    //读取原始图像数据
    HObject ho_X, ho_Y, ho_Z, ho_NX;
    HObject ho_NY, ho_NZ, ho_Texture, ho_GrayImage, ho_ColorImage;
    HObject ho_Region;


    HTuple hv_ObjectModel3D;
    HTuple hv_Rows, hv_Columns;
    HTuple hv_NXV, hv_NYV, hv_NZV;
    HTuple hv_Intensity;
    HTuple hv_RV, hv_GV, hv_BV;

    try
    {
        std::string  ss = (sFullPathNameRemoveSuffix + strGraySuffix + ".png").c_str();

        ReadImage(&ho_Texture, (sFullPathNameRemoveSuffix + strGraySuffix + ".png").c_str());

        Rgb1ToGray(ho_Texture, &ho_GrayImage);

        ReadImage(&ho_X, (sFullPathNameRemoveSuffix + strPointCloudSuffix + "X.tif").c_str());
        ReadImage(&ho_Y, (sFullPathNameRemoveSuffix + strPointCloudSuffix + "Y.tif").c_str());
        ReadImage(&ho_Z, (sFullPathNameRemoveSuffix + strPointCloudSuffix + "Z.tif").c_str());


        bool bNormalExist = false;
        try
        {
            ReadImage(&ho_NX, (sFullPathNameRemoveSuffix + strNormalSuffix + "X.tif").c_str());
            ReadImage(&ho_NY, (sFullPathNameRemoveSuffix + strNormalSuffix + "Y.tif").c_str());
            ReadImage(&ho_NZ, (sFullPathNameRemoveSuffix + strNormalSuffix + "Z.tif").c_str());

            bNormalExist = true;
        }
        catch (HalconCpp::HException& ex)
        {

        }



        //生成object3D 并添加各种元素

        GetDomain(ho_Z, &ho_Region);
        GetRegionPoints(ho_Region, &hv_Rows, &hv_Columns);

        ReduceDomain(ho_GrayImage, ho_Region, &ho_GrayImage);
        ReduceDomain(ho_X, ho_Region, &ho_X);
        ReduceDomain(ho_Y, ho_Region, &ho_Y);
        ReduceDomain(ho_Z, ho_Region, &ho_Z);

        if (bNormalExist)
        {
            ReduceDomain(ho_NX, ho_Region, &ho_NX);
            ReduceDomain(ho_NY, ho_Region, &ho_NY);
            ReduceDomain(ho_NZ, ho_Region, &ho_NZ);
        }



        XyzToObjectModel3d(ho_X, ho_Y, ho_Z, &hv_ObjectModel3D);


        //强度信息

        GetGrayval(ho_GrayImage, hv_Rows, hv_Columns, &hv_Intensity);
        SetObjectModel3dAttribMod(hv_ObjectModel3D, "&intensity", "points", hv_Intensity);



        //法线信息
        HObject ho_Image1, ho_Image2;
        HObject ho_Image3, ho_ImageH, ho_ImageS;
        HObject ho_ImageV, ho_ImageR, ho_ImageG;
        HObject ho_ImageB, ho_Multichannel;

        GenEmptyObj(&ho_Image1);
        GenEmptyObj(&ho_Image2);
        GenEmptyObj(&ho_Image3);
        GenEmptyObj(&ho_ImageH);
        GenEmptyObj(&ho_ImageS);
        GenEmptyObj(&ho_ImageV);
        GenEmptyObj(&ho_ImageR);
        GenEmptyObj(&ho_ImageG);
        GenEmptyObj(&ho_ImageB);
        GenEmptyObj(&ho_Multichannel);


        if (bNormalExist)
        {

            GetGrayval(ho_NX, hv_Rows, hv_Columns, &hv_NXV);
            GetGrayval(ho_NY, hv_Rows, hv_Columns, &hv_NYV);
            GetGrayval(ho_NZ, hv_Rows, hv_Columns, &hv_NZV);


            SetObjectModel3dAttribMod(hv_ObjectModel3D, ((HTuple("point_normal_x").Append("point_normal_y")).Append("point_normal_z")),
                HTuple(), (hv_NXV.TupleConcat(hv_NYV)).TupleConcat(hv_NZV));


            //法线渲染成RGB


            ScaleImageMax(ho_NX, &ho_Image1);
            ScaleImageMax(ho_NY, &ho_Image2);
            ScaleImageMax(ho_NZ, &ho_Image3);
            TransFromRgb(ho_Image1, ho_Image2, ho_Image3, &ho_ImageH, &ho_ImageS, &ho_ImageV, "hsv");
            TransToRgb(ho_ImageH, ho_ImageS, ho_ImageV, &ho_ImageR, &ho_ImageG, &ho_ImageB, "hsv");
            Compose3(ho_ImageR, ho_ImageG, ho_ImageB, &ho_Multichannel);

            GetGrayval(ho_ImageR, hv_Rows, hv_Columns, &hv_RV);
            GetGrayval(ho_ImageG, hv_Rows, hv_Columns, &hv_GV);
            GetGrayval(ho_ImageB, hv_Rows, hv_Columns, &hv_BV);
            SetObjectModel3dAttribMod(hv_ObjectModel3D, "&red", "points", hv_RV);
            SetObjectModel3dAttribMod(hv_ObjectModel3D, "&green", "points", hv_GV);
            SetObjectModel3dAttribMod(hv_ObjectModel3D, "&blue", "points", hv_BV);
        }


        s_Image3dS As;
        As.Gray = ho_GrayImage.Clone();
        As.X = ho_X.Clone();
        As.Y = ho_Y.Clone();
        As.Z = ho_Z.Clone();

        if (bNormalExist)
        {
            As.NX = ho_NX.Clone();
            As.NY = ho_NY.Clone();
            As.NZ = ho_NZ.Clone();
        }


        CopyObjectModel3d(hv_ObjectModel3D, "all", &As.ObjectModel3D);

        imgTable = As;

        if (hv_ObjectModel3D.Length() > 0)
        {
            ClearObjectModel3d(hv_ObjectModel3D);
        }



    }
    catch (HalconCpp::HException& ex)
    {

        return -1;
    }

    return 0;

}


//int  CHVisionAdvX::WriteDefectPlateBRtsImage(s_DefectPlateBRtsPara Rts, std::string szFolderPath, std::string sName)
//{
//
//    //提取路径文件夹路径
//
//    if (!is_directory_exists(szFolderPath))
//        create_directories_recursive(szFolderPath);
//
//
//    if (!is_directory_exists(szFolderPath))
//        return 1;
//
//
//    std::string   sPathNoFileType = szFolderPath + "\\" + sName;
//
//    try
//    {
//
//        //HObject X1;  //图像坐标系--原始图像X-铜板区域
//        //HObject Y1;  //图像坐标系--原始图像Y-铜板区域
//        //HObject Z1;  //图像坐标系--原始图像Z-铜板区域
//
//        //HObject Z1PZero; //图像调零坐标系--原始图像X-铜板区域
//        //HObject X1PZero; //图像调零坐标系--原始图像Y-铜板区域
//        //HObject Y1PZero; //图像调零坐标系--原始图像Y-铜板区域
//
//        //HObject ImageSubZ1;  //高度图--相对周边区域
//        //HObject ImageZ1ZeroReal; //高度图--相对整个零平面
//
//        //HObject NzRender; //法线渲染图
//        //HObject ZTbRender; //铜板区域高度渲染图
//        //HObject ZLzRender; //粒子区域高度渲染图
//
//
//
//
//        WriteImage(Rts.X1, "tiff", 0, (sPathNoFileType + "_X1").c_str());
//        WriteImage(Rts.Y1, "tiff", 0, (sPathNoFileType + "_Y1").c_str());
//        WriteImage(Rts.Z1, "tiff", 0, (sPathNoFileType + "_Z1").c_str());
//
//
//        WriteImage(Rts.Z1PZero, "tiff", 0, (sPathNoFileType + "_Z1PZero").c_str());
//        WriteImage(Rts.ImageSubZ1, "tiff", 0, (sPathNoFileType + "_ImageSubZ1").c_str());
//        WriteImage(Rts.ImageZ1ZeroReal, "tiff", 0, (sPathNoFileType + "_ImageZ1ZeroReal").c_str());
//
//
//        WriteImage(Rts.NzRender, "png fastest", 0, (sPathNoFileType + "_NzRender").c_str());
//        WriteImage(Rts.ZTbRender, "png fastest", 0, (sPathNoFileType + "_ZTbRender").c_str());
//        WriteImage(Rts.ZLzRender, "png fastest", 0, (sPathNoFileType + "_ZLzRender").c_str());
//
//
//    }
//    catch (HalconCpp::HException& ex)
//    {
//
//
//        return -1;
//
//    }
//
//    return 0;
//
//}





int  CHVisionAdvX::WriteImage3DX(s_Image3dS imgTable, std::string szFolderPath, std::string sNameRemoveSuffix)
{

    //提取路径文件夹路径

    if (!is_directory_exists(szFolderPath))
        create_directories_recursive(szFolderPath);


    if (!is_directory_exists(szFolderPath))
        return 1;


    std::string   sPathNoFileType = szFolderPath + "\\" + sNameRemoveSuffix;

    try
    {

        HTuple hv_FilePathX = (sPathNoFileType + "_IMG_PointCloud_X").c_str();
        WriteImage(imgTable.X, "tiff", 0, hv_FilePathX);

        HTuple hv_FilePathY = (sPathNoFileType + "_IMG_PointCloud_Y").c_str();
        WriteImage(imgTable.Y, "tiff", 0, hv_FilePathY);

        HTuple hv_FilePathZ = (sPathNoFileType + "_IMG_PointCloud_Z").c_str();
        WriteImage(imgTable.Z, "tiff", 0, hv_FilePathZ);


        HTuple hv_FilePathNX = (sPathNoFileType + "_IMG_NormalMap_X").c_str();
        WriteImage(imgTable.NX, "tiff", 0, hv_FilePathNX);

        HTuple hv_FilePathNY = (sPathNoFileType + "_IMG_NormalMap_Y").c_str();
        WriteImage(imgTable.NY, "tiff", 0, hv_FilePathNY);

        HTuple hv_FilePathNZ = (sPathNoFileType + "_IMG_NormalMap_Z").c_str();
        WriteImage(imgTable.NZ, "tiff", 0, hv_FilePathNZ);

        HTuple hv_FilePathGray = (sPathNoFileType + "_IMG_Texture_8Bit").c_str();
        WriteImage(imgTable.Gray, "png fastest", 0, hv_FilePathGray);

        // HTuple hv_FilePathColor = (sPathNoFileType + "_IMG_Color").c_str();
        // WriteImage(imgTable.Color, "png fastest", 0, hv_FilePathColor);



    }
    catch (HalconCpp::HException& ex)
    {


        return -1;

    }

    return 0;

}



void CHVisionAdvX::gen_arrow_contour_xld(HObject* ho_Arrow, HTuple hv_Row1, HTuple hv_Column1,
    HTuple hv_Row2, HTuple hv_Column2, HTuple hv_HeadLength, HTuple hv_HeadWidth)
{

    // Local iconic variables
    HObject  ho_TempArrow;

    // Local control variables
    HTuple  hv_Length, hv_ZeroLengthIndices, hv_DR;
    HTuple  hv_DC, hv_HalfHeadWidth, hv_RowP1, hv_ColP1, hv_RowP2;
    HTuple  hv_ColP2, hv_Index;

    //This procedure generates arrow shaped XLD contours,
    //pointing from (Row1, Column1) to (Row2, Column2).
    //If starting and end point are identical, a contour consisting
    //of a single point is returned.
    //
    //input parameteres:
    //Row1, Column1: Coordinates of the arrows' starting points
    //Row2, Column2: Coordinates of the arrows' end points
    //HeadLength, HeadWidth: Size of the arrow heads in pixels
    //
    //output parameter:
    //Arrow: The resulting XLD contour
    //
    //The input tuples Row1, Column1, Row2, and Column2 have to be of
    //the same length.
    //HeadLength and HeadWidth either have to be of the same length as
    //Row1, Column1, Row2, and Column2 or have to be a single element.
    //If one of the above restrictions is violated, an error will occur.
    //
    //
    //Init
    GenEmptyObj(&(*ho_Arrow));
    //
    //Calculate the arrow length
    DistancePp(hv_Row1, hv_Column1, hv_Row2, hv_Column2, &hv_Length);
    //
    //Mark arrows with identical start and end point
    //(set Length to -1 to avoid division-by-zero exception)
    hv_ZeroLengthIndices = hv_Length.TupleFind(0);
    if (0 != (int(hv_ZeroLengthIndices != -1)))
    {
        hv_Length[hv_ZeroLengthIndices] = -1;
    }
    //
    //Calculate auxiliary variables.
    hv_DR = (1.0 * (hv_Row2 - hv_Row1)) / hv_Length;
    hv_DC = (1.0 * (hv_Column2 - hv_Column1)) / hv_Length;
    hv_HalfHeadWidth = hv_HeadWidth / 2.0;
    //
    //Calculate end points of the arrow head.
    hv_RowP1 = (hv_Row1 + ((hv_Length - hv_HeadLength) * hv_DR)) + (hv_HalfHeadWidth * hv_DC);
    hv_ColP1 = (hv_Column1 + ((hv_Length - hv_HeadLength) * hv_DC)) - (hv_HalfHeadWidth * hv_DR);
    hv_RowP2 = (hv_Row1 + ((hv_Length - hv_HeadLength) * hv_DR)) - (hv_HalfHeadWidth * hv_DC);
    hv_ColP2 = (hv_Column1 + ((hv_Length - hv_HeadLength) * hv_DC)) + (hv_HalfHeadWidth * hv_DR);
    //
    //Finally create output XLD contour for each input point pair
    {
        HTuple end_val45 = (hv_Length.TupleLength()) - 1;
        HTuple step_val45 = 1;
        for (hv_Index = 0; hv_Index.Continue(end_val45, step_val45); hv_Index += step_val45)
        {
            if (0 != (int(HTuple(hv_Length[hv_Index]) == -1)))
            {
                //Create_ single points for arrows with identical start and end point
                GenContourPolygonXld(&ho_TempArrow, HTuple(hv_Row1[hv_Index]), HTuple(hv_Column1[hv_Index]));
            }
            else
            {
                //Create arrow contour
                GenContourPolygonXld(&ho_TempArrow, ((((HTuple(hv_Row1[hv_Index]).TupleConcat(HTuple(hv_Row2[hv_Index]))).TupleConcat(HTuple(hv_RowP1[hv_Index]))).TupleConcat(HTuple(hv_Row2[hv_Index]))).TupleConcat(HTuple(hv_RowP2[hv_Index]))).TupleConcat(HTuple(hv_Row2[hv_Index])),
                    ((((HTuple(hv_Column1[hv_Index]).TupleConcat(HTuple(hv_Column2[hv_Index]))).TupleConcat(HTuple(hv_ColP1[hv_Index]))).TupleConcat(HTuple(hv_Column2[hv_Index]))).TupleConcat(HTuple(hv_ColP2[hv_Index]))).TupleConcat(HTuple(hv_Column2[hv_Index])));
            }
            ConcatObj((*ho_Arrow), ho_TempArrow, &(*ho_Arrow));
        }
    }
    return;
}



// Chapter: Filters / Arithmetic
// Short Description: Scale the gray values of an image from the interval [Min,Max] to [0,255] 
void CHVisionAdvX::scale_image_range(HObject ho_Image, HObject* ho_ImageScaled, HTuple hv_Min,
    HTuple hv_Max)
{

    // Local iconic variables
    HObject  ho_ImageSelected, ho_SelectedChannel;
    HObject  ho_LowerRegion, ho_UpperRegion, ho_ImageSelectedScaled;

    // Local control variables
    HTuple  hv_LowerLimit, hv_UpperLimit, hv_Mult;
    HTuple  hv_Add, hv_NumImages, hv_ImageIndex, hv_Channels;
    HTuple  hv_ChannelIndex, hv_MinGray, hv_MaxGray, hv_Range;

    //Convenience procedure to scale the gray values of the
    //input image Image from the interval [Min,Max]
    //to the interval [0,255] (default).
    //Gray values < 0 or > 255 (after scaling) are clipped.
    //
    //If the image shall be scaled to an interval different from [0,255],
    //this can be achieved by passing tuples with 2 values [From, To]
    //as Min and Max.
    //Example:
    //scale_image_range(Image:ImageScaled:[100,50],[200,250])
    //maps the gray values of Image from the interval [100,200] to [50,250].
    //All other gray values will be clipped.
    //
    //input parameters:
    //Image: the input image
    //Min: the minimum gray value which will be mapped to 0
    //     If a tuple with two values is given, the first value will
    //     be mapped to the second value.
    //Max: The maximum gray value which will be mapped to 255
    //     If a tuple with two values is given, the first value will
    //     be mapped to the second value.
    //
    //Output parameter:
    //ImageScale: the resulting scaled image.
    //
    if (0 != (int((hv_Min.TupleLength()) == 2)))
    {
        hv_LowerLimit = ((const HTuple&)hv_Min)[1];
        hv_Min = ((const HTuple&)hv_Min)[0];
    }
    else
    {
        hv_LowerLimit = 0.0;
    }
    if (0 != (int((hv_Max.TupleLength()) == 2)))
    {
        hv_UpperLimit = ((const HTuple&)hv_Max)[1];
        hv_Max = ((const HTuple&)hv_Max)[0];
    }
    else
    {
        hv_UpperLimit = 255.0;
    }
    //
    //Calculate scaling parameters.
    //Only scale if the scaling range is not zero.
    if (0 != (HTuple(int(((hv_Max - hv_Min).TupleAbs()) < 1.0E-6)).TupleNot()))
    {
        hv_Mult = ((hv_UpperLimit - hv_LowerLimit).TupleReal()) / (hv_Max - hv_Min);
        hv_Add = ((-hv_Mult) * hv_Min) + hv_LowerLimit;
        //Scale image.
        ScaleImage(ho_Image, &ho_Image, hv_Mult, hv_Add);
    }
    //
    //Clip gray values if necessary.
    //This must be done for each image and channel separately.
    GenEmptyObj(&(*ho_ImageScaled));
    CountObj(ho_Image, &hv_NumImages);
    {
        HTuple end_val51 = hv_NumImages;
        HTuple step_val51 = 1;
        for (hv_ImageIndex = 1; hv_ImageIndex.Continue(end_val51, step_val51); hv_ImageIndex += step_val51)
        {
            SelectObj(ho_Image, &ho_ImageSelected, hv_ImageIndex);
            CountChannels(ho_ImageSelected, &hv_Channels);
            {
                HTuple end_val54 = hv_Channels;
                HTuple step_val54 = 1;
                for (hv_ChannelIndex = 1; hv_ChannelIndex.Continue(end_val54, step_val54); hv_ChannelIndex += step_val54)
                {
                    AccessChannel(ho_ImageSelected, &ho_SelectedChannel, hv_ChannelIndex);
                    MinMaxGray(ho_SelectedChannel, ho_SelectedChannel, 0, &hv_MinGray, &hv_MaxGray,
                        &hv_Range);
                    Threshold(ho_SelectedChannel, &ho_LowerRegion, (hv_MinGray.TupleConcat(hv_LowerLimit)).TupleMin(),
                        hv_LowerLimit);
                    Threshold(ho_SelectedChannel, &ho_UpperRegion, hv_UpperLimit, (hv_UpperLimit.TupleConcat(hv_MaxGray)).TupleMax());
                    PaintRegion(ho_LowerRegion, ho_SelectedChannel, &ho_SelectedChannel, hv_LowerLimit,
                        "fill");
                    PaintRegion(ho_UpperRegion, ho_SelectedChannel, &ho_SelectedChannel, hv_UpperLimit,
                        "fill");
                    if (0 != (int(hv_ChannelIndex == 1)))
                    {
                        CopyObj(ho_SelectedChannel, &ho_ImageSelectedScaled, 1, 1);
                    }
                    else
                    {
                        AppendChannel(ho_ImageSelectedScaled, ho_SelectedChannel, &ho_ImageSelectedScaled
                        );
                    }
                }
            }
            ConcatObj((*ho_ImageScaled), ho_ImageSelectedScaled, &(*ho_ImageScaled));
        }
    }
    return;
}






