#pragma once
#include <string>
#include <vector>

//3D扫描云台设备类型
enum EPlatFormType
{
	UnKnown_Type = 1,

	//轻量型云台系列
	ECO_LMS_511 = 2,	//   轻量型版LMS 511
	ECO_LMS_111 = 3,	//   轻量型版LMS 111
	ECO_NANOSCAN = 4,	//   轻量型版Nanoscan
	ECO_LRS4000 = 5,	//   轻量型版LRS4000
	ECO_Multiscan136 = 6,	//   轻量型版Multiscan

	//Mini云台系列
	PICOSCAN_MODE1=101, //Pico云台旋转模式，暂不支持
	PICOSCAN_MODE2=102, //Pico云台摆动模式

	//高精度云台系列，High Resolution Perception
	HRP_LMS511=201,
	HRP_LRS4000=202,

	//散料云台系列，Standard Range Perception
	SRP_LMS511 =301,
	SRP_LRS4000 = 302
};

//激光雷达设备类型
enum class SensorType
{
	UnKnownType = 0,
	LMS511 = 1,	//LMS 511
	LMS111 = 2,	//LMS 111
	LRS4000 = 3,	//LRS4000
	MULTISCAN = 4,	// Multiscan
	PICOSCAN = 5, //PicoScan
};

//云台错误代码
struct Common3DPlatformError
{
	//错误代码定义
	enum ErrorCode
	{
		NO_PLATFORMERROR = 0,
		LMS_CONNECT_FAIL = 1,                     //连接LMS传感器失败
		LMS_LOGIN_FAIL = 2,                       //登陆LMS传感器失败
		LMS_START_MEASURE_FAIL = 3,               //使LMS开始测量失败
		LMS_CREATE_ScanTHREAD_FAIL = 4,           //创建LMS扫描线程失败

		CONTROL_SYS_SOCKET_CREATE_FAIL = 5,        //创建控制系统套接字失败
		CONTROL_SYS_SOCKET_CONNECT_FAIL = 6,	   //与云台控制系统连接失败
		CONTROL_SYS_ENABLE_MOTOR_FAIL = 7,		   //使能电机指令发送失败
		CONTROL_SYS_CREATE_HeartBtTHREAD_FAIL = 8, //创建与云台控制系统的心跳交互线程失败
		CONTROL_SYS_CREATE_RecvMsgTHREAD_FAIL = 9, //创建接收来自于云台控制系统的消息的线程失败
		CONTROL_SYS_HeartBtUnnormal = 10,		   //与云台控制板之间的心跳异常
		CONTROL_SYS_SET_PARAM_ERR = 11,			   //设置的扫描参数错误
		CONTROL_SYS_SEND_COMMAND_FAIL = 12,        //发送消息至云台控制器失败

		PLATEFORM_IS_MOVING_NEED_WAITTING = 13,    //平台已经在执行动作，请稍后。
		PLATEFORM_NOT_INIT = 14,				   //平台未初始化
		PLATEFORM_RES_INIT_FAIL = 15,              //平台底层初始化分配资源失败
		PLATFORM_CALCULATING_DATA = 16,            //平台正在执行点云转换计算，请稍后

		PLATFORM_MOTOR_NOENABLE = 17,			        //云台电机还未使能
		PLATFORM_InitialStage_ToZeroPoint = 18,         //云台还在初始化找零点中
		PLATFORM_InitialStage_ToStartAngle = 19,        //云台初始化，去StartAngle过程中
		PLATFORM_InitialStage_WaitToSetScanAngle = 20,  //等待设置扫描起始角/结束角/角速度
		PLATFORM_MOTOR_POS_ERR = 21,					//控制器位置报错

		PLATFORM_SET_DEVICE_TYPE_ERR = 22,              //设置设备类型错误
		PLATFORM_INIT_TO_ZERO_POINT_ERR = 23,   //找零点错误
		FIREWARE_VERSION_ERR = 24,   //固件版本与SDK版本不匹配
		CONTROL_SYS_RECV_MESS_ERR = 25,  //接收控制板信息失败
		PLATFORM_TEMPERATURE_LOW_ERR = 26,  //低温报警，温度低于-10摄氏度，请等待云台自动加热
		PLATFORM_TEMPERATURE_HIGH_WARN = 27, //高温报警，温度高于50摄氏度

		LMS_RECV_MESS_ERR=28, //从扫描仪获取信息失败
		LMS_SET_PARAM_ERR=29, //修改扫描仪配置失败
		LMS_DEVICE_TYPE_ERR=30,  //输入的类型和实际不匹配
		LMS_CONTAMINATION_WARN=31, //雷达镜面污染报警，请清理雷达窗口
		LMS_CONTAMINATION_ERR=32, //雷达镜面污染报错，请清理雷达窗口

		PLATFORM_NEED_TO_ZERO_POINT = 33,			        //当前处于初始状态，需要手动进行回零点操作
		PLATFORM_READ_INNER_PARAM_ERR = 34			        //读取系统内置参数异常
	};

	ErrorCode code; //具体的错误代码
	std::string prompt; //出错之后的提示信息，可根据提示信息处理相关错误。注：并非所有错误代码都有相应的提示信息。
};

//云台状态定义
enum Common3DPlatformStatus
{
	eIdleStatus = 0,//初始化，没状态
	eInitialStage_ToZeroPoint_S6 = 1,//初始化，找零中
	eInitialStage_WaitToSetScanAngle_S7 = 2,//初始化，等待设置起始/结束角度
	eNOEnable_S5 = 3,              //未Enable，预留
	eInitialStage_ToStartAngle_S8 = 4,//初始化，去StarAngle过程中
	eStopAtStartPosition_S3 = 5,   //停在起始位置
	eStopAtStopPosition_S4 = 6,    //停在结束位置
	eStaticMode_Clock_SB = 7,//静态扫描模式，正转中
	eStaticMode_ClockWise_SC = 8,//静态扫描模式，反转中
	eSingleMoving_Clock_S1 = 9,//单次运行，正转（从小角度->大角度）	
	eSingleMoving_ClockWise_S2 = 10,//单次运行，反转中			
	eMotorPosErr_S9 = 11,//MotorPosError，pos_err_stop=1
	eNOHB_ii = 12,//10s内未接到上位机“HB”心跳
	eStaticMode_AtStopPosition = 13, //静态扫描模式停在结束位置
	eZeroPointError = 14, //零位开关检测失败
	eMotorStop = 15 //电机被手动停止，需要重新找零点
};

struct Point2D
{
	Point2D() { X = 0; Z = 0; }
	float X;
	float Z;
};

struct Point3D
{
	Point3D() 
	{
		X = 0; Y = 0; Z = 0; Distance = 0; AngleInProfile = 0; Elevation = 0; AngleOnRotation = 0; RSSI = 0; LevelLable = 0;
	}

	float X;
	float Y;
	float Z;
	int Distance;//单点距离值-极坐标
	float AngleInProfile;//单条轮廓中的角度值-极坐标,或者多线雷达中的方位角Azimuth
	float Elevation;//多线雷达坐标系下点的俯仰角，（仅使用多线雷达时有实际意义）
	float AngleOnRotation;//云台旋转方向的角度值
	unsigned short RSSI;//激光能量反射值
	unsigned short LevelLable;  //层数序号，（仅使用多线雷达时有实际意义）

	void clear()
	{
		X = 0; Y = 0; Z = 0; Distance = 0; AngleInProfile = 0; Elevation = 0; AngleOnRotation = 0; RSSI = 0; LevelLable = 0;
	}

	Point3D DeepCopy()
	{
		Point3D As;
		As.X = X; As.Y = Y; As.Z = Z; As.Distance = Distance; As.AngleInProfile = AngleInProfile; As.Elevation = Elevation; As.AngleOnRotation = AngleOnRotation; As.RSSI = RSSI; As.LevelLable = LevelLable;
	
		return As;
	}

};

typedef std::vector<Point3D> Single3DPointVec;
typedef std::vector<Single3DPointVec>	Double3DPointVec;

//LMS激光传感器扫描得到的3D点云数据(还未转换为云台直角坐标系内的值)
struct SLms3DPoint3DCloud
{
	int iSensorScanFreq;    // 传感器扫描频率
	std::vector<int> vEncoder;           // 编码器读数
	std::vector<float> curAngleVec;        //角度值
	Double3DPointVec vvPointsCloud;      // LMS传感器直角测量坐标系中的坐标    

	void clear()
	{
		//iSensorScanFreq = 0;//这里不能清零，只有在LMS扫描线程中会初始化该值。所以不可清零。
		vEncoder.clear();
		curAngleVec.clear();
		vvPointsCloud.clear();
	}
};

//LMS单帧轮廓数据
struct SLmsSingleProfile
{
	double dStartAngleOfSingleProfile;   // LMS单条轮廓的起始角度
	double dAngleOfYunTai;               // 当前云台旋转的角度
	double dAngleResolution;             // LMS的角度分辨率
	std::vector<float> vDistanceOfProfile;           // LMS的轮廓上每个点的距离值 
	int iTelegramCount;               // 该轮廓的Number号

	Single3DPointVec vMultiscanProfile;            //多线雷达单帧轮廓数据
	void clear()
	{
		vDistanceOfProfile.clear();
		vMultiscanProfile.clear();
	}
};

