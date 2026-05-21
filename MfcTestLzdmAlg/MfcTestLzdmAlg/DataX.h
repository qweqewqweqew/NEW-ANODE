#pragma once

#include "halconcpp.h"
#include <vector>
#include <vector>
using namespace HalconCpp;

// 相机设备信息
struct s_CameraInfo
{
	std::string SN = "";                      //序列号
	std::string sDeviceUserID = "";           //使用名
	std::string sModelName = "";              //型号
	std::string IpAddr = "";                  //IP
};

//驱动信息
struct s_DevicesInfo
{
	int iNum;
	std::string sDevSource;
	s_CameraInfo sCameraInfo[20];
};

// 标定-- 相机坐标系转机床坐标系转换pose
class s_PoseH
{
public:

	double fTransX; //平移XYZ
	double fTransY;
	double fTransZ;
	double fRotX;  //旋转XYZ
	double fRotY;
	double fRotZ;
	int  dwType;  //类型


	s_PoseH()
	{
		fTransX = 0.0;
		fTransY = 0.0;
		fTransZ = 0.0;
		fRotX = 0.0;
		fRotY = 0.0;
		fRotZ = 0.0;
		dwType = 0;
	}

	void Reset()
	{
		fTransX = 0.0;
		fTransY = 0.0;
		fTransZ = 0.0;
		fRotX = 0.0;
		fRotY = 0.0;
		fRotZ = 0.0;
		dwType = 0;

	}

	s_PoseH DeepCopy()
	{
		s_PoseH As;


		As.fTransX = fTransX;
		As.fTransY = fTransY;
		As.fTransZ = fTransZ;
		As.fRotX = fRotX;
		As.fRotY = fRotY;
		As.fRotZ = fRotZ;
		As.dwType = dwType;


		return As;
	}



	s_PoseH(double X, double Y, double Z, double RX, double RY, double RZ, int dw)
	{
		fTransX = X;
		fTransY = Y;
		fTransZ = Z;
		fRotX = RX;
		fRotY = RY;
		fRotZ = RZ;
		dwType = dw;
	}

	bool IsAvailable()
	{
		if (fRotX == 0 && fRotY == 0 && fRotZ == 0)
		{
			return false;
		}

		if (fTransX == 0 && fTransY == 0 && fTransZ == 0)
		{
			return false;
		}

		return true;
	}


	HTuple  ToTuple()
	{


		HTuple hh;

		hh.Clear();
		hh[0] = fTransX;
		hh[1] = fTransY;
		hh[2] = fTransZ;
		hh[3] = fRotX;
		hh[4] = fRotY;
		hh[5] = fRotZ;
		hh[6] = dwType;

		return  hh;


	}

	s_PoseH(HTuple h)
	{
		try
		{
			fTransX = h[0].D();
			fTransY = h[1].D();
			fTransZ = h[2].D();
			fRotX = h[3].D();
			fRotY = h[4].D();
			fRotZ = h[5].D();
			dwType = h[6].I();
		}
		catch (HOperatorException ex)
		{

			fTransX = 0.0;
			fTransY = 0.0;
			fTransZ = 0.0;
			fRotX = 0.0;
			fRotY = 0.0;
			fRotZ = 0.0;
			dwType = 0;
		}

	}

	s_PoseH ToGba()
	{
		s_PoseH PoseH;;

		try
		{

			HTuple h1, h2;

			h1.Clear();
			h1[0] = fTransX;
			h1[1] = fTransY;
			h1[2] = fTransZ;
			h1[3] = fRotX;
			h1[4] = fRotY;
			h1[5] = fRotZ;
			h1[6] = dwType;

			ConvertPoseType(h1, "Rp+T", "gba", "point", &h2);
			PoseH = s_PoseH(h2);
		}
		catch (HOperatorException ex)
		{

		}


		return PoseH;

	}


	s_PoseH ToAbg()
	{
		s_PoseH PoseH;

		try
		{
			HTuple h1, h2;

			h1.Clear();
			h1[0] = fTransX;
			h1[1] = fTransY;
			h1[2] = fTransZ;
			h1[3] = fRotX;
			h1[4] = fRotY;
			h1[5] = fRotZ;
			h1[6] = dwType;

			ConvertPoseType(h1, "Rp+T", "abg", "point", &h2);
			PoseH = s_PoseH(h2);

		}
		catch (HOperatorException ex)
		{

		}


		return PoseH;

	}


	//返回rt转换矩阵  刚性 HomMat3DIdentity is stored as the tuple [1,0,0,0,0,1,0,0,0,0,1,0].
	std::vector<double> getHomMat3dValues()
	{
		std::vector<double> valList;

		try
		{
			HTuple hv_HomMat3D;
			PoseToHomMat3d(s_PoseH(fTransX, fTransY, fTransZ, fRotX, fRotY, fRotZ, dwType).ToTuple(), &hv_HomMat3D);
			for (int i = 0; i < hv_HomMat3D.Length(); i++)
			{
				valList.push_back(hv_HomMat3D[i]);
			}
		}
		catch (HOperatorException ex)
		{
			valList.clear();
		}

		return valList;
	}

};



//铜跺顶部位置结果
class s_TbUpRts
{
public:
	double fCenterRow = 0; //像素位置
	double fCenterCol = 0;

	double fCenterX = 0;
	double fCenterY = 0;
	double fCenterZ = 0;
	double fCenterDEG = 0;
	//int iEarDir;   //0:0度方向 1:90度方向  2:180度方向:3：270度方向

	double fCornerRow[4] = { 0,0,0,0 };
	double fCornerCol[4] = { 0,0,0,0 };
	double fCornerX[4] = { 0,0,0,0 };
	double fCornerY[4] = { 0,0,0,0 };
	double fCornerZ[4] = { 0,0,0,0 };


	s_TbUpRts()
	{
		fCenterRow = 0;
		fCenterCol = 0;

		fCenterX = 0;
		fCenterY = 0;
		fCenterZ = 0;
		fCenterDEG = 0;
		/*iEarDir = 0;*/

		for (size_t i = 0; i < 4; i++)
		{
			fCornerRow[i] = 0;
			fCornerCol[i] = 0;
			fCornerX[i] = 0;
			fCornerY[i] = 0;
			fCornerZ[i] = 0;
		}





	}

	void Reset()
	{
		fCenterRow = 0;
		fCenterCol = 0;

		fCenterX = 0;
		fCenterY = 0;
		fCenterZ = 0;
		fCenterDEG = 0;
		/*iEarDir = 0;*/

		for (size_t i = 0; i < 4; i++)
		{
			fCornerRow[i] = 0;
			fCornerCol[i] = 0;
			fCornerX[i] = 0;
			fCornerY[i] = 0;
			fCornerZ[i] = 0;
		}
	}

	s_TbUpRts DeepCopy()
	{
		s_TbUpRts As;

		As.fCenterRow = fCenterRow;
		As.fCenterCol = fCenterCol;

		As.fCenterX = fCenterX;
		As.fCenterY = fCenterY;
		As.fCenterZ = fCenterZ;
		As.fCenterDEG = fCenterDEG;
		/*As.iEarDir = iEarDir;*/

		for (size_t i = 0; i < 4; i++)
		{
			As.fCornerRow[i] = fCornerRow[i];
			As.fCornerCol[i] = fCornerCol[i];
			As.fCornerX[i] = fCornerX[i];
			As.fCornerY[i] = fCornerY[i];
			As.fCornerZ[i] = fCornerZ[i];
		}
		return As;
	}


};



//旋转矩形结构体
struct s_Rect2X
{
	double fX;
	double fY;
	double fDeg;
	double fLength1;
	double fLength2;



	s_Rect2X()
	{
		fX = 0;
		fY = 0;
		fDeg = 0;
		fLength1 = 0;
		fLength2 = 0;

	}

	s_Rect2X DeepCopy()
	{
		s_Rect2X As;

		As.fX = fX;
		As.fY = fY;
		As.fDeg = fDeg;
		As.fLength1 = fLength1;
		As.fLength2 = fLength2;
		return As;
	}

	void Reset()
	{
		fX = 0;
		fY = 0;
		fDeg = 0;
		fLength1 = 0;
		fLength2 = 0;
	}

	//// 计算旋转矩形的四个角点
	//std::vector<std::pair<double, double>> GetCorners() const
	//{
	//	// 将角度转为弧度
	//	double rad = fDeg * M_PI / 180.0;
	//	double cosA = std::cos(rad);
	//	double sinA = std::sin(rad);

	//	// 局部坐标系下的四个角点（相对于中心）
	//	// 假设 fLength1 是 x 方向半长，fLength2 是 y 方向半长
	//	double dx[4] = { fLength1, -fLength1, -fLength1,  fLength1 };
	//	double dy[4] = { fLength2,  fLength2, -fLength2, -fLength2 };

	//	std::vector<std::pair<double, double>> corners;
	//	corners.reserve(4);

	//	for (int i = 0; i < 4; ++i)
	//	{
	//		// 旋转 + 平移到世界坐标
	//		double worldX = fX + dx[i] * cosA - dy[i] * sinA;
	//		double worldY = fY + dx[i] * sinA + dy[i] * cosA;
	//		corners.emplace_back(worldX, worldY);
	//	}

	//	return corners;
	//}

};


//平行矩形结构体
struct s_RectX
{
	double X1;
	double Y1;
	double X2;
	double Y2;



	s_RectX()
	{
		X1 = 0;
		Y1 = 0;
		X2 = 0;
		Y2 = 0;


	}

	s_RectX(double x1, double y1, double x2, double y2)
	{
		X1 = x1;
		Y1 = y1;
		X2 = x2;
		Y2 = y2;


	}


	s_RectX DeepCopy()
	{
		s_RectX As;

		As.X1 = X1;
		As.Y1 = Y1;
		As.X2 = X2;
		As.Y2 = Y2;
		return As;
	}

	void Reset()
	{
		X1 = 0;
		Y1 = 0;
		X2 = 0;
		Y2 = 0;
	}



};

//任意四边形结构体
struct s_Polygon4
{

	//按上左 上右  下右 下左的顺序
	double fX1;
	double fX2;
	double fX3;
	double fX4;

	double fY1;
	double fY2;
	double fY3;
	double fY4;



	s_Polygon4()
	{
		fX1 = 0.0;
		fX2 = 0.0;
		fX3 = 0.0;
		fX4 = 0.0;

		fY1 = 0.0;
		fY2 = 0.0;
		fY3 = 0.0;
		fY4 = 0.0;

	}


	s_Polygon4 DeepCopy()
	{
		s_Polygon4 As;


		As.fX1 = fX1;
		As.fX2 = fX2;
		As.fX3 = fX3;
		As.fX4 = fX4;

		As.fY1 = fY1;
		As.fY2 = fY2;
		As.fY3 = fY3;
		As.fY4 = fY4;
		return As;
	}

	void Reset()
	{
		fX1 = 0.0;
		fX2 = 0.0;
		fX3 = 0.0;
		fX4 = 0.0;

		fY1 = 0.0;
		fY2 = 0.0;
		fY3 = 0.0;
		fY4 = 0.0;
	}

	void PushX(double x1, double x2, double x3, double x4)
	{
		fX1 = x1;
		fX2 = x2;
		fX3 = x3;
		fX4 = x4;
	}

	void PushY(double y1, double y2, double y3, double y4)
	{
		fY1 = y1;
		fY2 = y2;
		fY3 = y3;
		fY4 = y4;


	}




	bool IsAvailable()
	{

		if (sqrt((fX1 - fX2) * (fX1 - fX2) + (fY1 - fY2) * (fY1 - fY2)) < 10)
		{
			return false;
		}


		if (sqrt((fX3 - fX2) * (fX3 - fX2) + (fY3 - fY2) * (fY3 - fY2)) < 10)
		{
			return false;
		}

		if (sqrt((fX3 - fX4) * (fX3 - fX4) + (fY3 - fY4) * (fY3 - fY4)) < 10)
		{
			return false;
		}

		if (sqrt((fX1 - fX4) * (fX1 - fX4) + (fY1 - fY4) * (fY1 - fY4)) < 10)
		{
			return false;
		}


		return true;
	}

};



//3D结构光相机图像
class s_Image3dS
{
public:
	HObject Gray;
	HObject Color;
	HObject X;
	HObject Y;
	HObject Z;
	HObject NX;
	HObject NY;
	HObject NZ;
	HTuple ObjectModel3D;
	long ID; //图像ID

	s_Image3dS()
	{
		GenEmptyObj(&Gray);
		GenEmptyObj(&Color);
		GenEmptyObj(&X);
		GenEmptyObj(&Y);
		GenEmptyObj(&Z);
		GenEmptyObj(&NX);
		GenEmptyObj(&NY);
		GenEmptyObj(&NZ);
		ObjectModel3D.Clear();
		ID = -1;
	}


	void Clear()
	{

		Gray.Clear();
		Color.Clear();
		X.Clear();
		Y.Clear();
		Z.Clear();
		NX.Clear();
		NY.Clear();
		NZ.Clear();

		if (ObjectModel3D.Length() > 0)
		{
			ClearObjectModel3d(ObjectModel3D);
			ObjectModel3D.Clear();
		}
		ID = -1;
	}


	s_Image3dS DeepCopy()
	{
		s_Image3dS As;

		//深拷贝
		if (Gray.IsInitialized() && Gray.CountObj() > 0) { As.Gray = Gray.Clone(); };
		if (Color.IsInitialized() && Color.CountObj() > 0) { As.Color = Color.Clone(); };
		if (X.IsInitialized() && X.CountObj() > 0) { As.X = X.Clone(); };
		if (Y.IsInitialized() && Y.CountObj() > 0) { As.Y = Y.Clone(); };
		if (Z.IsInitialized() && Z.CountObj() > 0) { As.Z = Z.Clone(); };
		if (NX.IsInitialized() && NX.CountObj() > 0) { As.NX = NX.Clone(); };
		if (NY.IsInitialized() && NY.CountObj() > 0) { As.NY = NY.Clone(); };
		if (NZ.IsInitialized() && NZ.CountObj() > 0) { As.NZ = NZ.Clone(); };

		if (ObjectModel3D.Length() > 0)
		{
			CopyObjectModel3d(ObjectModel3D, "all", &As.ObjectModel3D);
		}

		As.ID = ID;

		return As;
	}



	int GetMapWidth()
	{
		int value = 0;

		try
		{
			HTuple hw, hh;
			GetImageSize(Z, &hw, &hh);
			value = hw[0].D();
		}
		catch (HalconCpp::HException& ex)
		{

		}

		return value;

	}


	int GetMapHeight()
	{
		int value = 0;

		try
		{
			HTuple hw, hh;
			GetImageSize(Z, &hw, &hh);
			value = hh[0].D();
		}
		catch (HalconCpp::HException& ex)
		{

		}

		return value;


	}

};


//雷达图像
class s_Lidar3d
{
public:
	HObject Intensity; //激光强度图
	HObject X;
	HObject Y;
	HObject Z;
	HObject NX;
	HObject NY;
	HObject NZ;
	HTuple ObjectModel3D;
	long ID; //图像ID

	s_Lidar3d()
	{
		GenEmptyObj(&Intensity);
		GenEmptyObj(&X);
		GenEmptyObj(&Y);
		GenEmptyObj(&Z);
		GenEmptyObj(&NX);
		GenEmptyObj(&NY);
		GenEmptyObj(&NZ);
		ObjectModel3D.Clear();
		ID = -1;
	}


	void Clear()
	{

		Intensity.Clear();
		X.Clear();
		Y.Clear();
		Z.Clear();
		NX.Clear();
		NY.Clear();
		NZ.Clear();

		if (ObjectModel3D.Length() > 0)
		{
			ClearObjectModel3d(ObjectModel3D);
			ObjectModel3D.Clear();
		}
		ID = -1;
	}


	s_Lidar3d DeepCopy()
	{
		s_Lidar3d As;

		//深拷贝
		if (Intensity.IsInitialized() && Intensity.CountObj() > 0) { As.Intensity = Intensity.Clone(); };
		if (X.IsInitialized() && X.CountObj() > 0) { As.X = X.Clone(); };
		if (Y.IsInitialized() && Y.CountObj() > 0) { As.Y = Y.Clone(); };
		if (Z.IsInitialized() && Z.CountObj() > 0) { As.Z = Z.Clone(); };
		if (NX.IsInitialized() && NX.CountObj() > 0) { As.NX = NX.Clone(); };
		if (NY.IsInitialized() && NY.CountObj() > 0) { As.NY = NY.Clone(); };
		if (NZ.IsInitialized() && NZ.CountObj() > 0) { As.NZ = NZ.Clone(); };

		if (ObjectModel3D.Length() > 0)
		{
			CopyObjectModel3d(ObjectModel3D, "all", &As.ObjectModel3D);
		}

		As.ID = ID;

		return As;
	}


	int GetMapWidth()
	{
		int value = 0;

		try
		{
			HTuple hw, hh;
			GetImageSize(Z, &hw, &hh);
			value = hw[0].D();
		}
		catch (HalconCpp::HException& ex)
		{

		}

		return value;

	}


	int GetMapHeight()
	{
		int value = 0;

		try
		{
			HTuple hw, hh;
			GetImageSize(Z, &hw, &hh);
			value = hh[0].D();
		}
		catch (HalconCpp::HException& ex)
		{

		}

		return value;


	}
};




//雷达数据坐标系调整参数
class s_LidarTrans
{

public:

	bool    Y_Z_Change;
	bool   X_Y_Change;
	bool    X_mirro;
	bool    Y_mirro;
	bool    Map_Rotate;


	s_LidarTrans()
	{
		Y_Z_Change = false;
		X_Y_Change = false;
		X_mirro = false;
		Y_mirro = false;
		Map_Rotate = false;
	}




	s_LidarTrans DeepCopy()
	{
		s_LidarTrans As;


		As.Y_Z_Change = Y_Z_Change;
		As.X_Y_Change = X_Y_Change;
		As.X_mirro = X_mirro;
		As.Y_mirro = Y_mirro;
		As.Map_Rotate = Map_Rotate;


	}

};




//预处理图像输入-雷达3d
class s_PreProcessLidar3dPara
{
public:

	int  ProcessModel;  // 0:不处理  1 fast   2 nomal 3 fine 4 coustom
	double fProSetResolutionX;
	double fProSetResolutionY;


	double fRoiZMin;            //空间ROI区域范围
	double fRoiZMax;
	double fRoiXMin;
	double fRoiXMax;
	double fRoiYMin;
	double fRoiYMax;

	double fThSeg0Dis;              //初步分割  领域距离值  聚点数量选择 直径选择等
	int iThSeg0NumPtsMin;
	int iThSeg0NumPtsMax;
	double fThSeg0DiameterMin;
	double fThSeg0DiameterMax;



	s_PreProcessLidar3dPara()
	{

		ProcessModel = 0;

		fRoiZMin = 0.0; //空间ROI区域范围
		fRoiZMax = 0.0;
		fRoiXMin = 0.0;
		fRoiXMax = 0.0;
		fRoiYMin = 0.0;
		fRoiYMax = 0.0;

		fThSeg0Dis = 0.0;              //初步分割  领域距离值  聚点数量选择 直径选择  Y方向聚类长度限制值等
		iThSeg0NumPtsMin = 0;
		iThSeg0NumPtsMax = 0;
		fThSeg0DiameterMin = 0.0;
		fThSeg0DiameterMax = 0.0;

		fProSetResolutionX = 0;
		fProSetResolutionY = 0;
	}


	s_PreProcessLidar3dPara DeepCopy()
	{
		s_PreProcessLidar3dPara As;

		As.ProcessModel = ProcessModel;
		As.fProSetResolutionX = fProSetResolutionX;
		As.fProSetResolutionY = fProSetResolutionY;


		As.fRoiZMin = fRoiZMin;
		As.fRoiZMax = fRoiZMax;
		As.fRoiXMin = fRoiXMin;
		As.fRoiXMax = fRoiXMax;
		As.fRoiYMin = fRoiYMin;
		As.fRoiYMax = fRoiYMax;

		As.fThSeg0Dis = fThSeg0Dis;              //初步分割  领域距离值  聚点数量选择 直径选择  Y方向聚类长度限制值等
		As.iThSeg0NumPtsMin = iThSeg0NumPtsMin;
		As.iThSeg0NumPtsMax = iThSeg0NumPtsMax;
		As.fThSeg0DiameterMin = fThSeg0DiameterMin;
		As.fThSeg0DiameterMax = fThSeg0DiameterMax;


		return As;
	}

};


//预处理图像结果-雷达3d
class s_PreProcessLidar3dRts
{

public:
	bool bTJG;          //综合判断
	double time;
	s_Lidar3d sImage3dPro;  //处理后的图像

	s_PreProcessLidar3dRts()
	{
		bTJG = false;
		time = 0.0;

	}

	s_PreProcessLidar3dRts DeepCopy()
	{
		s_PreProcessLidar3dRts As;

		As.bTJG = bTJG;
		As.time = time;
		As.sImage3dPro = sImage3dPro.DeepCopy();
		return As;
	}
};



//初定位+检测参数
class s_PreADPlateAPara
{
public:
	double fThNZAbsVal;  //提取平面法线参数

	double fThSegCbDis;     //车板分割距离参数
	double fThCbSelLMin;    //选取铜跺的长宽参数 点数参数
	double fThCbSelLMax;
	double fThCbSelWMin;
	double fThCbSelWMax;
	double fThCbSelPtsMin;
	double fThCbSelPtsMax;
	double fThCbSelZMin;
	double fThCbSelZMax;


	bool  bDefectSpaceObj;//是否检测空间异物 
	double fDefectSpaceThZMin;  //提取范围限制参数
	double fDefectSpaceThZMax;
	double fDefectSpaceThInnerLen; //内缩长度 单侧内缩距离
	double fDefectSpaceThInnerWidth;//内缩宽度 
	double fDefectSpaceThLen; //判定参数
	double fDefectSpaceThNumPts;


	bool  bDefectLbObj;//是否检测围栏
	double fDefectLbThZMin;  ////提取Z范围限制参数  相对于车板底面
	double fDefectLbThZMax;
	double fDefectLbThick;  //栏板厚度
	double fDefectLbThInnerLen; //内缩长度
	double fDefectLbThInnerWidth; //内缩宽度
	double fDefectLbThLen; //判定参数-栏板长度
	double fDefectLbThWidth;  //判定参数-栏板宽度
	double fDefectLbThNumPts;  //判定参数-栏板点数

	double fThSegTbDis; //铜跺分割距离
	double fThTbSelLMin;  //选取铜跺的长宽参数 点数参数
	double fThTbSelLMax;
	double fThTbSelWMin;
	double fThTbSelWMax;
	double fThTbSelPtsMin;
	double fThTbSelPtsMax;
	double fThTbSelZMin;
	double fThTbSelZMax;
	double fJgTbCloseLenMin;//判断铜跺接触最小长度
	double fJgTbClosePtsMin;//判断铜跺接触最小点数


	double fThSafeDisTbNear;  //铜跺之间的安全间距
	double fThSafeDisToCbOuter; //铜跺和车板外边界的安全间距

	double fSampleTb;


	s_PreADPlateAPara()
	{
		fThNZAbsVal = 0.8;  //提取平面法线参数

		fThSegCbDis = 80;   //车板分割距离参数
		fThCbSelLMin = 3000;   //车板提取参数 长宽 点数量 
		fThCbSelLMax = 18000;
		fThCbSelWMin = 800;
		fThCbSelWMax = 5000;
		fThCbSelPtsMin = 2000;
		fThCbSelPtsMax = 180000;
		fThCbSelZMin = 4900; //提取Z范围限制参数  相对于车板底面
		fThCbSelZMax = 5600;


		bDefectSpaceObj = true;//是否检测空间异物 
		fDefectSpaceThZMin = 900;  //提取范围限制参数
		fDefectSpaceThZMax = 3000;
		fDefectSpaceThInnerLen = 150; //内缩长度 单侧内缩距离
		fDefectSpaceThInnerWidth = 300; //内缩宽度 
		fDefectSpaceThLen = 500.00;  //判定参数
		fDefectSpaceThNumPts = 10;


		bDefectLbObj = true;  //是否检测栏板
		fDefectLbThZMin = 300;  //提取Z范围限制参数  相对于车板底面
		fDefectLbThZMax = 1600;
		fDefectLbThick = 300;
		fDefectLbThInnerLen = 200;  //内缩长度
		fDefectLbThInnerWidth = 100;//内缩宽度
		fDefectLbThLen = 2000.00;   //判定参数-栏板长度
		fDefectLbThWidth = 1000.00;  //判定参数-栏板宽度
		fDefectLbThNumPts = 100;   //判定参数-栏板点数


		fThSegTbDis = 60;   //铜板分割参数
		fThTbSelLMin = 800;
		fThTbSelLMax = 1500;
		fThTbSelWMin = 800;
		fThTbSelWMax = 1500;
		fThTbSelPtsMin = 3500;
		fThTbSelPtsMax = 17000;
		fThTbSelZMin = 4500;
		fThTbSelZMax = 5050;
		fJgTbCloseLenMin = 2000; //判断铜跺接触最小长度
		fJgTbClosePtsMin = 4500; //判断铜跺接触最小点数


		fThSafeDisTbNear = 350;  //铜跺之间的安全间距
		fThSafeDisToCbOuter = 400; //铜跺和车板外边界的安全间距

		fSampleTb = 80;  //铜跺下采样参数
	}

	s_PreADPlateAPara DeepCopy()
	{
		s_PreADPlateAPara As;

		As.fThNZAbsVal = fThNZAbsVal;

		As.fThSegCbDis = fThSegCbDis;
		As.fThCbSelLMin = fThCbSelLMin;
		As.fThCbSelLMax = fThCbSelLMax;
		As.fThCbSelWMin = fThCbSelWMin;
		As.fThCbSelWMax = fThCbSelWMax;
		As.fThCbSelPtsMin = fThCbSelPtsMin;
		As.fThCbSelPtsMax = fThCbSelPtsMax;
		As.fThCbSelZMin = fThCbSelPtsMin;
		As.fThCbSelZMax = fThCbSelPtsMax;




		As.bDefectSpaceObj = bDefectSpaceObj;
		As.fDefectSpaceThZMin = fDefectSpaceThZMin;
		As.fDefectSpaceThZMax = fDefectSpaceThZMax;
		As.fDefectSpaceThInnerLen = fDefectSpaceThInnerLen;
		As.fDefectSpaceThInnerWidth = fDefectSpaceThInnerWidth;
		As.fDefectSpaceThLen = fDefectSpaceThLen;
		As.fDefectSpaceThNumPts = fDefectSpaceThNumPts;


		As.bDefectLbObj = bDefectLbObj;
		As.fDefectLbThZMin = fDefectLbThZMin;
		As.fDefectLbThZMax = fDefectLbThZMax;
		As.fDefectLbThick = fDefectLbThick;
		As.fDefectLbThInnerLen = fDefectLbThInnerLen;
		As.fDefectLbThInnerWidth = fDefectLbThInnerWidth;
		As.fDefectLbThLen = fDefectLbThLen;
		As.fDefectLbThWidth = fDefectLbThWidth;
		As.fDefectLbThNumPts = fDefectLbThNumPts;



		As.fThSegTbDis = fThSegTbDis;
		As.fThTbSelLMin = fThTbSelLMin;
		As.fThTbSelLMax = fThTbSelLMax;
		As.fThTbSelWMin = fThTbSelWMin;
		As.fThTbSelWMax = fThTbSelWMax;
		As.fThTbSelPtsMin = fThTbSelPtsMin;
		As.fThTbSelPtsMax = fThTbSelPtsMax;
		As.fThTbSelZMin = fThTbSelZMin;
		As.fThTbSelZMax = fThTbSelZMax;
		As.fJgTbCloseLenMin = fJgTbCloseLenMin;
		As.fJgTbClosePtsMin = fJgTbClosePtsMin;


		As.fThSafeDisTbNear = fThSafeDisTbNear;
		As.fThSafeDisToCbOuter = fThSafeDisToCbOuter;

		As.fSampleTb = fSampleTb;

		return As;
	}

};



//初定位+检测结果参数
class s_PreADPlateARtsPara
{
public:
	bool bTJG;         //综合判断
	double time;


	bool  bExistDefectSpace;   //空间异物检测结果
	std::vector<int>  iDefectSpaceRtsListPtsSel;
	std::vector<double> fDefectSpaceRtsListXSel;
	std::vector<double> fDefectSpaceRtsListYSel;
	std::vector<double> fDefectSpaceRtsListZSel;//Z取最高点高度
	std::vector<double> fDefectSpaceRtsListLSel;//长度
	HObject RegionDefectSpaceRts;


	bool bExistDefectLb;  //围栏检测结果
	std::vector<int> iDefectLbRtsListPtsSel;
	std::vector<double>fDefectLbRtsListHeightSel;//栏板高度
	std::vector<double>fDefectLbRtsListLSel;//长度
	HObject RegionDefectLbRts;


	bool bExitTbClose; //存在铜跺之间有接触
	HObject  RegionDefectCloseRtsTb;
	bool bExitSafeDisTb;  //存在铜板安全间距异常
	HObject  RegionDefectTbNearRts;


	bool  bExitSafeDisToCbOuter; //存在到车板外围安全间距异常
	HObject  RegionDefectOuterRts;

	std::vector<s_TbUpRts> TbUpRtsList;
	HObject RegionTbsPlus;      //铜板区域--最终抓取
	HObject RectTbsPlus;

	s_PreADPlateARtsPara()
	{
		bTJG = false;
		time = 0.0;

		bExistDefectSpace = false;
		iDefectSpaceRtsListPtsSel.clear();
		fDefectSpaceRtsListXSel.clear();
		fDefectSpaceRtsListYSel.clear();
		fDefectSpaceRtsListZSel.clear();
		fDefectSpaceRtsListLSel.clear();
		GenEmptyObj(&RegionDefectSpaceRts);

		bExistDefectLb = false;
		iDefectLbRtsListPtsSel.clear();
		fDefectLbRtsListHeightSel.clear();
		fDefectLbRtsListLSel.clear();
		GenEmptyObj(&RegionDefectLbRts);

		bExitTbClose = false;
		GenEmptyObj(&RegionDefectCloseRtsTb);
		bExitSafeDisTb = false;
		GenEmptyObj(&RegionDefectTbNearRts);
		bExitSafeDisToCbOuter = false;
		GenEmptyObj(&RegionDefectOuterRts);




		TbUpRtsList.clear();
		GenEmptyObj(&RegionTbsPlus);
		GenEmptyObj(&RectTbsPlus);

		////3D结果  
		//GenEmptyObjectModel3d(&OM3ImageAdv);

	}

	s_PreADPlateARtsPara DeepCopy()
	{
		s_PreADPlateARtsPara As;

		As.bTJG = bTJG;
		As.time = time;

		As.bExistDefectSpace = bExistDefectSpace;
		As.iDefectSpaceRtsListPtsSel = iDefectSpaceRtsListPtsSel;
		As.fDefectSpaceRtsListXSel = fDefectSpaceRtsListXSel;
		As.fDefectSpaceRtsListYSel = fDefectSpaceRtsListYSel;
		As.fDefectSpaceRtsListZSel = fDefectSpaceRtsListZSel;
		As.fDefectSpaceRtsListLSel = fDefectSpaceRtsListLSel;
		if (RegionDefectSpaceRts.IsInitialized() && RegionDefectSpaceRts.CountObj() > 0)
			As.RegionDefectSpaceRts = RegionDefectSpaceRts.Clone();

		As.bExistDefectLb = bExistDefectLb;
		As.iDefectLbRtsListPtsSel = iDefectLbRtsListPtsSel;
		As.fDefectLbRtsListHeightSel = fDefectLbRtsListHeightSel;
		As.fDefectLbRtsListLSel = fDefectLbRtsListLSel;
		if (RegionDefectLbRts.IsInitialized() && RegionDefectLbRts.CountObj() > 0)
			As.RegionDefectLbRts = RegionDefectLbRts.Clone();

		As.bExitTbClose = bExitTbClose;
		if (RegionDefectCloseRtsTb.IsInitialized() && RegionDefectCloseRtsTb.CountObj() > 0)
			As.RegionDefectCloseRtsTb = RegionDefectCloseRtsTb.Clone();

		As.bExitSafeDisTb = bExitSafeDisTb;
		if (RegionDefectTbNearRts.IsInitialized() && RegionDefectTbNearRts.CountObj() > 0)
			As.RegionDefectTbNearRts = RegionDefectTbNearRts.Clone();

		As.bExitSafeDisToCbOuter = bExitSafeDisToCbOuter;
		if (RegionDefectOuterRts.IsInitialized() && RegionDefectOuterRts.CountObj() > 0)
			As.RegionDefectOuterRts = RegionDefectOuterRts.Clone();




		As.TbUpRtsList.clear();
		for (size_t i = 0; i < TbUpRtsList.size(); i++)
		{
			As.TbUpRtsList.push_back(TbUpRtsList[i].DeepCopy());
		}

		if (RegionTbsPlus.IsInitialized() && RegionTbsPlus.CountObj() > 0)
			As.RegionTbsPlus = RegionTbsPlus.Clone();

		if (RectTbsPlus.IsInitialized() && RectTbsPlus.CountObj() > 0)
			As.RectTbsPlus = RectTbsPlus.Clone();



		// 3D
		//if (OM3ImageAdv.Length() > 0)
		//{
		//	try { CopyObjectModel3d(OM3ImageAdv, "all", &As.OM3ImageAdv); }
		//	catch (...) { ; }

		//}

		return As;
	}

	void Reset()
	{
		bTJG = false;
		time = 0.0;

		bExistDefectSpace = false;
		iDefectSpaceRtsListPtsSel.clear();
		fDefectSpaceRtsListXSel.clear();
		fDefectSpaceRtsListYSel.clear();
		fDefectSpaceRtsListZSel.clear();
		fDefectSpaceRtsListLSel.clear();
		RegionDefectSpaceRts.Clear();
		GenEmptyObj(&RegionDefectSpaceRts);

		bExistDefectLb = false;
		iDefectLbRtsListPtsSel.clear();
		fDefectLbRtsListHeightSel.clear();
		fDefectLbRtsListLSel.clear();
		RegionDefectLbRts.Clear();
		GenEmptyObj(&RegionDefectLbRts);

		bExitTbClose = false;
		RegionDefectCloseRtsTb.Clear();
		GenEmptyObj(&RegionDefectCloseRtsTb);

		bExitSafeDisTb = false;
		RegionDefectTbNearRts.Clear();
		GenEmptyObj(&RegionDefectTbNearRts);


		bExitSafeDisToCbOuter = false;
		RegionDefectOuterRts.Clear();
		GenEmptyObj(&RegionDefectOuterRts);

		for (size_t i = 0; i < TbUpRtsList.size(); i++)
		{
			TbUpRtsList[i].Reset();
		}
		TbUpRtsList.clear();

		RegionTbsPlus.Clear();
		GenEmptyObj(&RegionTbsPlus);

		RectTbsPlus.Clear();
		GenEmptyObj(&RectTbsPlus);


		////3D结果  
		//GenEmptyObjectModel3d(&OM3ImageAdv);
		//if (OM3ImageAdv.Length() > 0)
		//{
		//	ClearObjectModel3d(OM3ImageAdv);
		//	OM3ImageAdv.Clear();
		//}

	}


};



//预处理图像输入-3DS
class s_PreProcess3DSPara
{
public:
	double fRoiZMin;            //空间ROI区域范围
	double fRoiZMax;
	double fRoiXMin;
	double fRoiXMax;
	double fRoiYMin;
	double fRoiYMax;

	double fThSeg0Dis;              //初步分割  领域距离值  聚点数量选择 直径选择等
	int iThSeg0NumPtsMin;
	int iThSeg0NumPtsMax;
	double fThSeg0DiameterMin;
	double fThSeg0DiameterMax;





	s_PreProcess3DSPara()
	{
		fRoiZMin = 0.0; //空间ROI区域范围
		fRoiZMax = 0.0;
		fRoiXMin = 0.0;
		fRoiXMax = 0.0;
		fRoiYMin = 0.0;
		fRoiYMax = 0.0;

		fThSeg0Dis = 0.0;              //初步分割  领域距离值  聚点数量选择 直径选择  Y方向聚类长度限制值等
		iThSeg0NumPtsMin = 0;
		iThSeg0NumPtsMax = 0;
		fThSeg0DiameterMin = 0.0;
		fThSeg0DiameterMax = 0.0;

	
	}


	s_PreProcess3DSPara DeepCopy()
	{
		s_PreProcess3DSPara As;

		As.fRoiZMin = fRoiZMin;
		As.fRoiZMax = fRoiZMax;
		As.fRoiXMin = fRoiXMin;
		As.fRoiXMax = fRoiXMax;
		As.fRoiYMin = fRoiYMin;
		As.fRoiYMax = fRoiYMax;

		As.fThSeg0Dis = fThSeg0Dis;              //初步分割  领域距离值  聚点数量选择 直径选择  Y方向聚类长度限制值等
		As.iThSeg0NumPtsMin = iThSeg0NumPtsMin;
		As.iThSeg0NumPtsMax = iThSeg0NumPtsMax;
		As.fThSeg0DiameterMin = fThSeg0DiameterMin;
		As.fThSeg0DiameterMax = fThSeg0DiameterMax;

	


		return As;
	}

};


//预处理图像结果-3DS
class s_PreProcess3DSResultPara
{

public:
	bool bTJG;          //综合判断
	double time;
	s_Image3dS sImage3dPro;  //处理后的图像

	s_PreProcess3DSResultPara()
	{
		bTJG = false;
		time = 0.0;

	}

	s_PreProcess3DSResultPara DeepCopy()
	{
		s_PreProcess3DSResultPara As;

		As.bTJG = bTJG;
		As.time = time;
		As.sImage3dPro = sImage3dPro.DeepCopy();
		return As;
	}
};


//精确定位+检测参数
struct s_AccurateADPlateAPara
{


	double fThCbNZAbsValMin= 0.96;  //车板法线筛选
	double fThCbNZAbsValMax = 1;

	double fThCbNXAbsValMin = 0;
	double fThCbNXAbsValMax = 0.35;
	double fThCbNYAbsValMin = 0;
	double fThCbNYAbsValMax = 0.35;

	double fThSegCbDis = 20;   //车板分割距离
	double fThCbSelLMin = 500;  //选择车板的长宽 点数 
	double fThCbSelLMax = 5000;
	double fThCbSelWMin = 500;
	double fThCbSelWMax = 5000;
	double fThCbSelPtsMin = 2000;
	double fThCbSelPtsMax = 1000000;
	double fThCbSelZMin = -2500;   //选择车板的Z值范围  该值在标定后坐标系下 为车板相对于夹爪底部的距离范围
	double fThCbSelZMax = -1800;


	double fThTbNZAbsVal = 0.90; //铜跺法线提取
	double fThSegTbDis = 10;  //铜跺分割距离
	double fThTbSelLMin = 800;    //选择铜跺的长宽 点数 
	double fThTbSelLMax = 1500;
	double fThTbSelWMin = 800;
	double fThTbSelWMax = 1500;
	double fThTbSelPtsMin = 50000;
	double fThTbSelPtsMax = 1000000;
	double fThTbSelZMin = -2000;    //选择铜跺的Z值范围  该值在标定后坐标系下 为铜跺相对于夹爪底部的距离范围 
	double fThTbSelZMax = -1200;


	double fThSafeDisTdNear=350;   //铜跺安全距离
	double fDefectTdNearSelZRefCb=100; //检测范围相对于车板高度
	int iDefectTdNearRoiWDilation=201; //检测范围宽度方向扩展像素
	int  iDefectTdNearRoiLErosion=101; //检测范围长度方向内缩像素 

	int iBmRoiWDilationW=301; //板面宽度方向ROI在宽度方向扩展
	int iBmRoiWErosionL=51; //板面宽度方向ROI在长度方向内缩
	int iBmRoiLDilationL=101; //板面长度方向ROI在长度方向扩展

	double  fOffsetZSelTbRelCb = 30; //选择相对于车板高度的铜跺 

	double fEdgeWLenUse = 800;  //宽度方向边缘长度使用
	double fGndSelEdgeWOffset = 300;  //宽度方向边缘偏离一定距离作为近底面
	double fGndSelEdgeWLen = 600;  //近底面的长度方向长度
	double fGripUp = 0;  //吊钩底部相对于底板上偏移

	double fLimitDegMin = 80;  //限制范围报警-铜跺角度
	double fLimitDegMax = 100;

	double fLimitTdWMin = 900;  //限制范围报警-铜跺的长宽高
	double fLimitTdWMax = 1100;
	double fLimitTdLMin = 980;
	double fLimitTdLMax = 11200;
	double fLimitTdHMin = 300;
	double fLimitTdHMax = 600;

	double  fThTbLayerOuter = 50;//层错阈值  定义成下面的板子相对于最上面一块板的外偏移（爪子位置范围内）


	double  fDefectGripSpaceRoiXOffeset = 10; //检测夹爪区域ROI范围--偏离铜跺外边缘作为X起始
	double  fDefectGripSpaceRoiXLen = 200; //检测夹爪区域ROI范围--Y方向长度
	double  fDefectGripSpaceRoiYLen = 600; //检测夹爪区域ROI范围--Y方向长度
	double  fDefectGripSpaceRoiZOffesetCb = 35; //检测夹爪区域ROI范围--高于底面一定高度的空间数据
	double  fDefectGripSpaceRoiZLen = 500;//检测夹爪区域ROI范围--Z方向长度
	double  fDefectGripSpaceThLen = 50.00;  //判定参数--抓取范围内空间异物
	double  fDefectGripSpaceThNumPts = 10;


};



//精确定位+检测结果
class s_AccurateADPlateARtsPara
{
public:
	bool bTJG;         //综合判断
	double time;

	bool  bExitSafeDisTd;     //存在铜跺安全距离超限
	double fDefectTdNearRtsDis1;  //间距1
	double fDefectTdNearRtsDis2;  //间距2

	double TdX;    //铜跺抓取位置的 XYZ+DEG
	double TdY;
	double TdZ;
	double TdDeg;
	int   TdEarDir; //0:0度方向 1:90度方向  2:180度方向:3：270度方向
	double CbZ[2];   //车板Z

	double TdW;     //铜跺的长宽高
	double TdL;
	double TdH;

	bool bExitLimitSize;  //存在角度和尺寸数据超限制

	bool bExitTbLayerOuter; //存在层错
	double fTbLayerOuterDis1;
	double fTbLayerOuterDis2;

	bool bExistDefectGripSpace;  //存在夹爪空间缺陷
	std::vector<int> iDefectGripSpaceRtsListPtsSel;
	std::vector<double> fDefectGripSpaceRtsListLSel;//长度
	HObject RegionDefectGripSpaceRts1;
	HObject RegionDefectGripSpaceRts2;

	HObject  RegionTDW1;   //宽度方向区域
	HObject  RegionTDW2;   //宽度方向区域
	HObject  RegionTDL1;   //长度方向区域
	HObject  RegionTDL2;   //长度方向区域

	s_AccurateADPlateARtsPara()
	{
		bTJG = false;
		time = 0.0;

		bExitSafeDisTd = false;
		fDefectTdNearRtsDis1 = 0.0;
		fDefectTdNearRtsDis2 = 0.0;

		TdX = 0.0;
		TdY = 0.0;
		TdZ = 0.0;
		TdDeg = 0.0;
		TdEarDir = 0;
		CbZ[0] = 0.0;
		CbZ[1] = 0.0;

		TdW = 0.0;
		TdL = 0.0;
		TdH = 0.0;

		bExitLimitSize = false;

		bExitTbLayerOuter = false; //存在层错
		fTbLayerOuterDis1 = 0.0;
		fTbLayerOuterDis2 = 0.0;

		bExistDefectGripSpace = false;  //存在夹爪空间缺陷
		iDefectGripSpaceRtsListPtsSel.clear();
		fDefectGripSpaceRtsListLSel.clear();
		GenEmptyObj(&RegionDefectGripSpaceRts1);
		GenEmptyObj(&RegionDefectGripSpaceRts2);

		GenEmptyObj(&RegionTDW1);
		GenEmptyObj(&RegionTDL1);
		GenEmptyObj(&RegionTDW2);
		GenEmptyObj(&RegionTDL2);
	}

	s_AccurateADPlateARtsPara DeepCopy()
	{
		s_AccurateADPlateARtsPara As;

		As.bTJG = bTJG;
		As.time = time;

		As.bExitSafeDisTd = bExitSafeDisTd;
		As.fDefectTdNearRtsDis1 = fDefectTdNearRtsDis1;
		As.fDefectTdNearRtsDis2 = fDefectTdNearRtsDis2;

		As.TdX = TdX;
		As.TdY = TdY;
		As.TdZ = TdZ;
		As.TdDeg = TdDeg;
		As.TdEarDir = TdEarDir;
		As.CbZ[0] = CbZ[0];
		As.CbZ[1] = CbZ[1];

		As.TdW = TdW;
		As.TdL = TdL;
		As.TdH = TdH;

		As.bExitLimitSize = bExitLimitSize;


		As.bExitTbLayerOuter = bExitTbLayerOuter; 
		As.fTbLayerOuterDis1 = fTbLayerOuterDis1;
		As.fTbLayerOuterDis2 = fTbLayerOuterDis2;

		As.bExistDefectGripSpace = bExistDefectGripSpace;  
		As.iDefectGripSpaceRtsListPtsSel = iDefectGripSpaceRtsListPtsSel;
		As.fDefectGripSpaceRtsListLSel = fDefectGripSpaceRtsListLSel;//长度

  

		if (RegionDefectGripSpaceRts1.IsInitialized() && RegionDefectGripSpaceRts1.CountObj() > 0)
			As.RegionDefectGripSpaceRts1 = RegionDefectGripSpaceRts1.Clone();
		else
			GenEmptyObj(&RegionDefectGripSpaceRts1);

		if (RegionDefectGripSpaceRts2.IsInitialized() && RegionDefectGripSpaceRts2.CountObj() > 0)
			As.RegionDefectGripSpaceRts2 = RegionDefectGripSpaceRts2.Clone();
		else
			GenEmptyObj(&RegionDefectGripSpaceRts2);



		// 深拷贝 RegionTDW
		if (RegionTDW1.IsInitialized() && RegionTDW1.CountObj() > 0)
			As.RegionTDW1 = RegionTDW1.Clone();
		else
			GenEmptyObj(&As.RegionTDW1);

		// 深拷贝 RegionTDL
		if (RegionTDL1.IsInitialized() && RegionTDL1.CountObj() > 0)
			As.RegionTDL1 = RegionTDL1.Clone();
		else
			GenEmptyObj(&As.RegionTDL1);



		if (RegionTDW2.IsInitialized() && RegionTDW2.CountObj() > 0)
			As.RegionTDW2 = RegionTDW2.Clone();
		else
			GenEmptyObj(&As.RegionTDW2);

		// 深拷贝 RegionTDL
		if (RegionTDL2.IsInitialized() && RegionTDL2.CountObj() > 0)
			As.RegionTDL2 = RegionTDL2.Clone();
		else
			GenEmptyObj(&As.RegionTDL2);


		return As;
	}

	void Reset()
	{
		bTJG = false;
		time = 0.0;

		bExitSafeDisTd = false;
		fDefectTdNearRtsDis1 = 0.0;
		fDefectTdNearRtsDis2 = 0.0;

		TdX = 0.0;
		TdY = 0.0;
		TdZ = 0.0;
		TdDeg = 0.0;
		TdEarDir = 0;
		CbZ[0] = 0.0;
		CbZ[1] = 0.0;

		TdW = 0.0;
		TdL = 0.0;
		TdH = 0.0;

		bExitLimitSize = false;

		bExitTbLayerOuter = false; //存在层错
		fTbLayerOuterDis1 = 0.0;
		fTbLayerOuterDis2 = 0.0;

		bExistDefectGripSpace = false;  //存在夹爪空间缺陷
		iDefectGripSpaceRtsListPtsSel.clear();
		fDefectGripSpaceRtsListLSel.clear();
		GenEmptyObj(&RegionDefectGripSpaceRts1);
		GenEmptyObj(&RegionDefectGripSpaceRts2);

		GenEmptyObj(&RegionTDW1);
		GenEmptyObj(&RegionTDL1);
		GenEmptyObj(&RegionTDW2);
		GenEmptyObj(&RegionTDL2);
	}
};


//点位结构体
struct s_PPts
{
	double fX = 0; //坐标点
	double fY = 0;
	double fZ = 0;
	double fDeg = 0;

	double fRow = 0;  //图像行坐标
	double fCol = 0;  //图像列坐标

    //int iEarDir;   //0:0度方向 1:90度方向  2:180度方向:3：270度方向
};


//铜跺预定位计算输入
class s_CalcPreAlignPara
{
public:

	double  fSX;   //天车拍照基准位
	s_PoseH sPoseLidar2Crane; //雷达转天车Pose
	double fLidar2Gnd;        //雷达到地面距离
	double  MatLidar2Crane[12];//雷达转天车矩阵


	double  fX;   //当前拍照位

	double fOffsetTbX; //铜跺固定补偿
	double fOffsetTbY;
	double fOffsetTbZ;
	double fOffsetTbDeg;

	double fCameraOffsetX;  //相机拍照位相对于铜跺偏移量XYZ Deg
	double fCameraOffsetY;
	double fCameraOffsetZ;
	double fCameraOffsetDeg;


	double fLimitCameraOffsetZMin; //拍照位限制范围
	double fLimitCameraOffsetZMax; //
	double fLimitCameraOffsetXMin;
	double fLimitCameraOffsetXMax;
	double fLimitCameraOffsetYMin;
	double fLimitCameraOffsetYMax;
	double fLimitCameraOffsetDegMin;
	double fLimitCameraOffsetDegMax;



	s_CalcPreAlignPara()
	{
		fSX = 0;   //天车拍照基准位
		sPoseLidar2Crane.Reset(); //雷达转天车矩阵
		fLidar2Gnd = 0;
		for (size_t i = 0; i < 12; i++)
		{
			MatLidar2Crane[i] = 0;
		}
		

		fX = 0;   //当前拍照位

		fOffsetTbX = 0; //铜跺固定补偿
		fOffsetTbY = 0;
		fOffsetTbZ = 0;
		fOffsetTbDeg = 0;

		fCameraOffsetX = 0;  //相机拍照位相对于铜跺偏移量XYZ Deg
		fCameraOffsetY = 0;
		fCameraOffsetZ = 0;
		fCameraOffsetDeg = 0;

		fLimitCameraOffsetZMin = 0; //拍照位限制范围
		fLimitCameraOffsetZMax = 0; //
		fLimitCameraOffsetXMin = 0;
		fLimitCameraOffsetXMax = 0;
		fLimitCameraOffsetYMin = 0;
		fLimitCameraOffsetYMax = 0;
		fLimitCameraOffsetDegMin = 0;
		fLimitCameraOffsetDegMax = 0;

	}

	s_CalcPreAlignPara DeepCopy()
	{
		s_CalcPreAlignPara As;

		As.fSX = fSX;
		As.sPoseLidar2Crane = sPoseLidar2Crane.DeepCopy();
		As.fLidar2Gnd = fLidar2Gnd;
		for (size_t i = 0; i < 12; i++)
		{
			As.MatLidar2Crane[i] = MatLidar2Crane[i];
		}


		As.fX = 0;   //当前拍照位

		As.fOffsetTbX = fOffsetTbX; //铜跺固定补偿
		As.fOffsetTbY = fOffsetTbY;
		As.fOffsetTbZ = fOffsetTbZ;
		As.fOffsetTbDeg = fOffsetTbDeg;


		As.fCameraOffsetX = fCameraOffsetX;  //相机拍照位相对于铜跺偏移量XYZ Deg
		As.fCameraOffsetY = fCameraOffsetY;
		As.fCameraOffsetZ = fCameraOffsetZ;
		As.fCameraOffsetDeg = fCameraOffsetDeg;

		As.fLimitCameraOffsetZMin = fLimitCameraOffsetZMin; //拍照位限制范围
		As.fLimitCameraOffsetZMax = fLimitCameraOffsetZMax; //
		As.fLimitCameraOffsetXMin = fLimitCameraOffsetXMin;
		As.fLimitCameraOffsetXMax = fLimitCameraOffsetXMax;
		As.fLimitCameraOffsetYMin = fLimitCameraOffsetYMin;
		As.fLimitCameraOffsetYMax = fLimitCameraOffsetYMax;
		As.fLimitCameraOffsetDegMin = fLimitCameraOffsetDegMin;
		As.fLimitCameraOffsetDegMax = fLimitCameraOffsetDegMax;


		return As;
	}


	void Reset()
	{

		fSX = 0;   //天车拍照基准位
		sPoseLidar2Crane.Reset(); //雷达转天车矩阵
		fLidar2Gnd = 0;
		for (size_t i = 0; i < 12; i++)
		{
			MatLidar2Crane[i] = 0;
		}


		fX = 0;   //当前拍照位

		fOffsetTbX = 0; //铜跺固定补偿
		fOffsetTbY = 0;
		fOffsetTbZ = 0;
		fOffsetTbDeg = 0;

		fCameraOffsetX = 0;  //相机拍照位相对于铜跺偏移量XYZ Deg
		fCameraOffsetY = 0;
		fCameraOffsetZ = 0;
		fCameraOffsetDeg = 0;

		fLimitCameraOffsetZMin = 0; //拍照位限制范围
		fLimitCameraOffsetZMax = 0; //
		fLimitCameraOffsetXMin = 0;
		fLimitCameraOffsetXMax = 0;
		fLimitCameraOffsetYMin = 0;
		fLimitCameraOffsetYMax = 0;
		fLimitCameraOffsetDegMin = 0;
		fLimitCameraOffsetDegMax = 0;
	}


};


//铜跺预定位结果
struct s_CalcPreAlignRtsPara
{
	bool bTJG = false;          //综合判断
	std::vector<s_PPts> fTbCenterCrane;
	std::vector<s_PPts> fCamaraPosCrane;

	void Reset()
	{
		bool bTJG = false;          //综合判断
		fTbCenterCrane.clear();
		fCamaraPosCrane.clear();
	}

};


//铜跺精定位计算输入
struct s_CalcAccurateAlignPara
{

	double  fSX = 0;   //对位基准 即标定板的位置 工具坐标下  
	                   // 将标定板的中心严格对准夹爪中心，并让标定板和天车运动方向平行 
	//                 故XY = 0 DEG = 0 SZ为测量夹爪下方到标定板的垂直高度
	double  fSY = 0;
	double  fSZ = 0;
	double  fSDeg = 0;

	double  fCameraSX = 0;   //天车拍照基准位（标定位）XYZ+DEG 天车坐标系下
	double  fCameraSY = 0;
	double  fCameraSZ = 0;
	double  fCameraSDeg = 0;

	double  MatCameraA2Grip[12];//相机1转工具坐标系矩阵 
	double  MatCameraB2Grip[12];//相机2转工具坐标系矩阵 

	double  fCameraX = 0;   //当前拍照位 XYZ+DEG  天车坐标系下
	double  fCameraY = 0;
	double  fCameraZ = 0;
	double  fCameraDeg = 0;


	double fOffsetTbX = 0; //铜跺固定补偿
	double fOffsetTbY = 0;
	double fOffsetTbZ = 0;
	double fOffsetTbDeg = 0;


	double fLimitRefMaxZ = 0; //对位数据最大值  为相对基准数据  每次对位发现超过最大值 直接报警 并结束对位
	double fLimitRefMaxX = 0;
	double fLimitRefMaxY = 0;
	double fLimitRefMaxDeg = 0;


	double fLimitGripZMin = 0; //抓取位限制范围
	double fLimitGripZMax = 0;
	double fLimitGripXMin = 0;
	double fLimitGripXMax = 0;
	double fLimitGripYMin = 0;
	double fLimitGripYMax = 0;
	double fLimitGripDegMin = 0;
	double fLimitGripDegMax = 0;

	double fAccurateX = 0;   //对位精度xyz+deg
	double fAccurateY = 0;
	double fAccurateZ = 0;
	double fAccurateDeg = 0;

	s_CalcAccurateAlignPara()
	{


		//雷达转天车矩阵
		for (size_t i = 0; i < 12; i++)
		{
			MatCameraA2Grip[i] = 0;
			MatCameraB2Grip[i] = 0;
		}


		

	}




	void Reset()
	{
		 fSX = 0;   //对位基准 即标定板的位置 工具坐标下  
		 fSY = 0;
		 fSZ = 0;
		 fSDeg = 0;


		fCameraSX = 0;    //天车拍照基准位（标定位）XYZ+DEG 实际计算结果和相机拍照基准位没有关系
		fCameraSY = 0;
		fCameraSZ = 0;
		fCameraSDeg = 0;


		//雷达转天车矩阵
		for (size_t i = 0; i < 12; i++)
		{
			MatCameraA2Grip[i] = 0;
			MatCameraB2Grip[i] = 0;
		}


		fCameraX = 0;//当前拍照位 XYZ+DEG 
		fCameraY = 0;
		fCameraZ = 0;
		fCameraDeg = 0;

		fOffsetTbX = 0; //铜跺固定补偿
		fOffsetTbY = 0;
		fOffsetTbZ = 0;
		fOffsetTbDeg = 0;

		fLimitRefMaxZ = 0; //对位数据最大值  为相对基准数据  每次对位发现超过最大值 直接报警 并结束对位
		fLimitRefMaxX = 0;
		fLimitRefMaxY = 0;
		fLimitRefMaxDeg = 0;

		fLimitGripZMin = 0; //抓取位限制范围
		fLimitGripZMax = 0; //
		fLimitGripXMin = 0;
		fLimitGripXMax = 0;
		fLimitGripYMin = 0;
		fLimitGripYMax = 0;
		fLimitGripDegMin = 0;
		fLimitGripDegMax = 0;

		 fAccurateX = 0;   //对位精度xyz+deg
		 fAccurateY = 0;
		 fAccurateZ = 0;
		 fAccurateDeg = 0;
	}


};


//铜跺精定位计算结果
struct s_CalcAccurateAlignRtsPara
{


	int iStatus = 0;      //状态 1 ：OK 2:NG  5:继续拍照 

	double dX = 0; //对位偏差值
	double dY = 0;
	double dZ = 0;
	double dDeg = 0;


	double fNextCameraX = 0; //下一个拍照位坐标
	double fNextCameraY = 0;
	double fNextCameraZ = 0;
	double fNextCameraDeg = 0;


	double fGripX = 0; //抓取位坐标点
	double fGripY = 0;
	double fGripZ = 0;
	double fGripDeg = 0; //抓取角度 实际不使用 客户已经根据铜跺的角度自己计算

	int iTdEarDir;   //0:0度方向 1:90度方向  2:180度方向:3：270度方向
	double fTdEarDeg;  //铜跺的角度 转换成天车旋转坐标系下的值
	


	double TdW = 0;   //铜跺的长宽高
	double TdL = 0;
	double TdH = 0;


	void Reset()
	{
		iStatus = 0;      //状态 1 ：OK 2:NG  5:继续拍照 

		dX = 0; //对位偏差值
		dY = 0;
		dZ = 0;
		dDeg = 0;


		fNextCameraX = 0; //下一个拍照位坐标
		fNextCameraY = 0;
		fNextCameraZ = 0;
		fNextCameraDeg = 0;


		fGripX = 0; //抓取位坐标点
		fGripY = 0;
		fGripZ = 0;
		fGripDeg = 0;

		iTdEarDir = 1;   //0:0度方向 1:90度方向  2:180度方向:3：270度方向
		fTdEarDeg = 0;


		TdW = 0;   //铜跺的长宽高
		TdL = 0;
		TdH = 0;

	}

};


// 函数返回
struct s_Rtnf
{
public:
	int iCode = 0;                  // 返回代码
	std::string strInfo = "";       // 返回描述

};


