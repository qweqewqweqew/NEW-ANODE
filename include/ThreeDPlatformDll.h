#pragma once

#ifdef _WIN32
#ifdef  ThreeDPAPI
#define ThreeDPAPI  __declspec(dllexport)
#else
#define ThreeDPAPI  __declspec(dllimport)
#endif
#else
#define ThreeDPAPI __attribute__((visibility("default")))
#endif

#include <string>  
#include <queue>
#include <thread>
#include "ThreeDPlatformData.h"

extern "C" class ThreeDPAPI ThreeDPlatform
{
public:
	ThreeDPlatform();
	virtual ~ThreeDPlatform(void);

	/*===========================  常用功能  ===========================*/

	/*
	**   分配资源,开启工作线程
	*@ err: 错误枚举值
	*@ deviceType: 设备型号，一定要正确设置设备型号，否则获取的点云坐标数据是错误的
	*@ lmsIPAddr: 传感器IP地址
	*@ controlSysIPAddr: 云台控制器IP地址
	*/
	bool Init(Common3DPlatformError& err, EPlatFormType deviceType, const std::string& lmsIPAddr, const std::string& controlSysIPAddr);

	/*
	**   销毁资源与子线程
	*/
	void Destory();

	/*
	**   设置扫描参数(起始角/结束角/角速度)
	*@ startAngle： 扫描起始角(度) ，PICO云台和双支撑云台的取值范围：0-180，其余类型的取值范围：2~358
	*@ stopAngle:   扫描结束角(度) ，PICO云台和双支撑云台的取值范围：0-180，其余类型的取值范围：2~358
	*@ vel:  扫描速度，度/秒，取值范围：0.1~50
	*@ err： 错误枚举值
	*@ 返回值: true成功，false失败（失败时具体原因参见err）
	*/
	bool SetScanParameter(float startAngle, float stopAngle, float vel, Common3DPlatformError& err);
	void GetScanParameter(float& startAngle, float& stopAngle, float& vel);

	/*
	**   开启单次3D扫描
	*/
	bool Start3DScanOnce(Common3DPlatformError& err);

	/*
	**   获取扫描后点云数据，已经转换到直角坐标系下，坐标单位:毫米
	**   注：当Echo Filter设置为First或All时，该接口获取的为首次回波的数据；当Echo Filter设置为Last时，该接口获取的为最后一次回波的数据。
	**   若开启All Echoes后还需要获取最后一次回波的点云数据，可调用GetNewest3DPointCloudFromLastEcho()
	*@ cloudData ：获取的点云数据
	*返回1表示：正常获取到点云数据; 
	*返回0表示：无可用的点云数据；
	*返回-1表示：云台的网络通讯异常，需要重新初始化；
	*返回-2表示：雷达的网络通讯异常，需要重新初始化；
	*/
	int Get3DPointCloud(Double3DPointVec& cloudData);   //获取队列中最先获取的点云，并将该点云从队列清除
	int GetNewest3DPointCloud(Double3DPointVec& cloudData);  //获取队列中最晚获取的点云，并清空队列

	/*
	**   开启All Echoes后，获取扫描后最后一次回波的点云数据，已经转换到直角坐标系下，坐标单位:毫米
	**   注：仅当Echo Filter设置为All时，且需要获取最后一次回波数据时调用。当Echo Filter设置为Fisrt或Last时，该接口不生效。
	**   注：若雷达已经通过其他方式设置为了All Echoes, 需要通过该接口获取最后一次回波数据时，还是需要先调用enableGetAllEchoData(true)。
	*@ cloudData ：获取的点云数据
	*返回1表示：正常获取到点云数据;
	*返回0表示：无可用的点云数据；
	*返回-1表示：云台的网络通讯异常，需要重新初始化；
	*返回-2表示：雷达的网络通讯异常，需要重新初始化；
	*返回-3表示：未设置All Echoes；
	*/
	int GetNewest3DPointCloudFromLastEcho(Double3DPointVec& cloudData);

	/*
	**   开启让云台以某个速度旋转到某个角度，不进行数据采集
	*@ stopAngle:   扫描结束角(度) ，PICO云台和双支撑云台的取值范围：0-180，其余类型的取值范围：2~358
	*@ velocity:  扫描速度，度/秒，取值范围：0.1~50
	*@ err： 错误枚举值
	*@ 返回值: true成功，false失败（失败时具体原因参见err）
	*/
	bool StartMoveToAnglePosition(float stopAngle, float velocity, Common3DPlatformError& err);

	/*
	**   获取单帧轮廓数据
	**   注：当Echo Filter设置为First或All时，该接口获取的为首次回波的数据；当Echo Filter设置为Last时，该接口获取的为最后一次回波的数据。
	**   若开启All Echoes后还需要获取最后一次回波的点云数据，可调用GetSingleProfileFromLastEcho()
	* @ singleProfile: 单帧轮廓数据
	* @ coorFlag: 轮廓依赖的坐标系，0表示基于雷达坐标系，1表示基于云台坐标系
	*返回1表示：成功获取LMS的单帧轮廓数据；
	*返回0表示：无可用的点云数据；
	*返回-1表示：云台的网络通讯异常，需要重新初始化；
	*返回-2表示：雷达的网络通讯异常，需要重新初始化；
	*返回-3表示：云台还没有旋转到指定的角度，还在运动过程中;
	*返回-4表示：当前扫描模式错误；
	*/
	int GetSingleProfile(std::vector<Point3D>& singleProfile, int coorFlag = 1);

	/*
	**   开启All Echoes后，获取扫描后最后一次回波的单帧轮廓数据
	**   注：仅当Echo Filter设置为All时，且需要获取最后一次回波数据时调用。当Echo Filter设置为Fisrt或Last时，该接口不生效。
	**   注：若雷达已经通过其他方式设置为了All Echoes, 需要通过该接口获取最后一次回波数据时，还是需要先调用enableGetAllEchoData(true)。
	* @ singleProfile: 单帧轮廓数据
	* @ coorFlag: 轮廓依赖的坐标系，0表示基于雷达坐标系，1表示基于云台坐标系
	*返回1表示：成功获取LMS的单帧轮廓数据；
	*返回0表示：无可用的点云数据；
	*返回-1表示：云台的网络通讯异常，需要重新初始化；
	*返回-2表示：雷达的网络通讯异常，需要重新初始化；
	*返回-3表示：云台还没有旋转到指定的角度，还在运动过程中
	*返回-4表示：未设置All Echoes
	*返回-5表示：当前扫描模式错误；
	*/
	int GetSingleProfileFromLastEcho(std::vector<Point3D>& singleProfile, int coorFlag = 1);

	/*
	**   雷达的回波设置为All Echoes时，如果想在一次扫描中同时获取第一次回波和最后一次回波的数据，需要先调用该函数启功该功能。
	**   注：仅当Echo Filter设置为All时，且需要获取最后一次回波数据时调用。当Echo Filter设置为Fisrt或Last时，该接口不生效。
	**   注：若需要使用该功能，仅需要在初始化后设置一次即可。
	* @ flag: 开启或关闭该功能。true:开启，false:关闭
	*返回1表示：设置成功；
	*返回0表示：设置失败，云台未连接；
	*返回-1表示：设置失败，雷达未开启All Echoes
	*/
	int enableGetAllEchoData(bool flag);

	/*
	**   获取云台传感器内部温度
	* @ temperature ：温度值
	* @ warnLevel ：温度报警信息，0：温度正常，1：低温警告，2：高温警告
	* @ 返回值：0表示获取失败，1表示获取成功
	*/
	int GetTemperature(float& temperature, int& warnLevel);

	/*
	**   获取云台状态
	*返回1表示：成功
	*返回0表示：云台未连接
	*返回-1表示：云台连接异常断开
	*返回-2表示：雷达连接异常断开
	*/
	int GetStatus(Common3DPlatformStatus& status);

	//获取设备类型
	EPlatFormType GetDeviceType();

	//模块内部线程是否均在正常运行，如果不正常，重新Init
	bool IsThreadRunningOK();

	//校准零点，并回到零点位置
	bool InitToZeroPoint(Common3DPlatformError& err);

	//获取固件版本号
	bool GetFirewareVersion(std::string& verNO);

	/*
	**   修改云台IP地址，修改成功后，设备需要断电重启
	*@ platformIP:  新的云台IP
	*@ LMSIP:  新的扫描仪IP
	*@ LMSUdpIP:  新的UDP目标IP地址。注意：当搭载的雷达为multiscan或picoscan时需要设置，其余型号雷达不需要设置该值
	*@ LMSUdpPort:  新的UDP目标端口号。注意：当搭载的雷达为multiscan或picoscan时需要设置，其余型号雷达不需要设置该值
	*返回1表示：修改成功
	*返回0表示：云台未连接
	*返回-1表示：IP地址格式错误
	*返回-2表示：修改云台IP地址失败
	*返回-3表示：修改雷达IP地址失败
	*/
	int ModifyIPAdress(std::string platformIP, std::string LMSIP, std::string LMSUdpIP = "", unsigned short LMSUdpPort = 2115);

	/*===========================  高级功能  ===========================*/

	/*
	**   修改和读取子网掩码
	*@ platformMask:  新的云台子网掩码
	*@ LMSMask:  新的扫描仪子网掩码
	*返回1表示：修改/读取成功
	*返回0表示：云台未连接
	*返回-1表示：子网掩码格式错误
	*返回-2表示：修改/读取云台子网掩码失败
	*返回-3表示：修改/读取雷达子网掩码失败
	*返回-4表示：当前云台型号不支持该功能
	*/
	int setSubnetMask(std::string platformMask, std::string LMSMask);
	int getSubnetMask(std::string& platformMask, std::string& LMSMask);

	/*
	**   修改和读取默认网关
	*@ platformGateway:  新的云台默认网关
	*@ LMSGateway:  新的扫描仪默认网关
	*返回1表示：修改/读取成功
	*返回0表示：云台未连接
	*返回-1表示：默认网关格式错误
	*返回-2表示：修改/读取云台默认网关失败
	*返回-3表示：修改/读取雷达默认网关失败
	*返回-4表示：当前云台型号不支持该功能
	*/
	int setDefaultGateway(std::string platformGateway, std::string LMSGateway);
	int getDefaultGateway(std::string& platformGateway, std::string& LMSGateway);

	/*
	**   控制云台断电重启，重启之后需要重新进行连接。该功能仅支持部分型号的云台
	*@ rebooTime:  控制断电之后多长时间之后再上电，单位：ms。取值范围3000~30000
	*返回1表示：重启成功
	*返回0表示：云台未连接
	*返回-1表示：控制重启失败
	*返回-2表示：当前云台型号不支持该功能
	*返回-3表示：运动过程中不能断电重启
	*/
	int rebootDevice(unsigned short rebooTime = 3000);

	/*
	**   控制云台电机停止转动，同时云台会恢复到初始状态，需要重新设置参数
	*返回1表示：成功
	*返回0表示：云台未连接
	*返回-1表示：控制电机停止失败
	*返回-2表示：当前云台型号不支持该功能
	*返回-3表示：当前状态不能或不需要电机停止转动（回零过程中不能停止电机转动，其他静止状态下不需要停止电机转动）
	*/
	int stopMotorAndBackToInitialStage();

	/*
	**   设置点云模型在云台正转或者反转的时候模型的偏移补偿值
	*@ offset:  补偿角度值
	*@ orient:  云台旋转方向，1正转，0反转
	*/
	bool setMoelAngleOffset(float offset, int orient);

	/*
	**   设置多线雷达每层的角度补偿矩阵
	*@ tranMat:  补偿值
	*/
	bool setMultiScanAngleOffset(float tranMat[16][4][4]);

	/*
	**   设置是否启用多线雷达每层补偿的功能
	*@ isOpen:  是否开启
	*/
	bool setOpenMultiScanTran(bool isOpen = false);

	/*===========================  雷达参数配置和获取相关接口  ===========================*/
	/*==  雷达参数配置后需要调用saveSettingsPermanently()接口进行永久保存  ==*/

	/*
	**   获取LMS传感器产品序列号
	*@ 返回值：0表示未能成功获取(诸如云台未初始化、传感器连接异常等等均会导致不能成功获取该序列号)
	*/
	unsigned int GetLmsSerialNo();

	/*
	**   获取LMS系列雷达的污染状态
	*@ level:  污染等级 0：无污染 ，1：污染警告，2：污染报错，3：检测模块错误
	*返回1表示：获取成功
	*返回0表示：云台未连接
	*返回-1表示：当前雷达不支持该功能
	*返回-2表示：云台扫描中，不支持获取
	*返回-3表示：获取出错
	*/
	int getLMSContaminationStatus(int& level);

	/*
	**   获取LRS4000系列雷达的污染状态
	*@ level: 每个扇区的污染等级 0：无污染 ，1：污染警告，2：污染报错，3：检测模块错误
	*返回1表示：获取成功
	*返回0表示：云台未连接
	*返回-1表示：当前雷达不支持该功能
	*返回-2表示：云台扫描中，不支持获取
	*返回-3表示：获取出错
	*/
	int getLRSContaminationStatus(std::vector<int>& level);

	/*
	**   读取激光雷达的温度
	*@ temperature:  温度值
	*返回1表示：读取成功
	*返回0表示：云台未连接
	*返回-2表示：云台扫描中，不支持读取
	*返回-3表示：读取出错
	*/
	int getLidarTemperature(float& temperature);

	/*
	**   永久保存雷达的参数，保存后断电不会丢失
	*返回1表示：成功
	*返回0表示：云台未连接
	*返回-1表示：保存失败
	*/
	int saveSettingsPermanently();

	/*
	**   设置和读取激光雷达的回波方式。注：设置期间无法扫描获取数据
	*@ echoFlag:   0:First echo, 1:All echoes, 2:Last echo
	*返回1表示：设置/读取成功
	*返回0表示：云台未连接
	*返回-1表示：输入参数有误
	*返回-2表示：云台扫描中，不支持设置/读取
	*返回-3表示：设置/读取出错
	*/
	int SetEchoFilter(unsigned int echoFlag);
	int getEchoFilter(unsigned int& echoFlag);

	/*
	**   设置和读取数据流输出的以太网配置。当搭载的雷达为multiscan或picoscan时支持
	*@ protocal:  数据传输方式，1：UDP，2：TCP
	*@ destinationIP:  目标IP地址
	*@ destinationPort:  目标端口号
	*返回1表示：设置/读取成功
	*返回0表示：云台未连接
	*返回-1表示：输入参数有误
	*返回-2表示：云台扫描中，不支持设置/读取
	*返回-3表示：设置/读取出错
	*返回-4表示：当前雷达不支持该功能
	*/
	int setStreamingEthernetSettings(unsigned short protocal, std::string destinationIP, unsigned short destinationPort);
	int getStreamingEthernetSettings(unsigned short& protocal, std::string& destinationIP, unsigned short& destinationPort);

	/*
	**   设置/读取是否开启数据流输出。当搭载的雷达为multiscan或picoscan时支持
	*@ isOpen:  开启标志位，0: 关闭，1:打开
	*返回1表示：设置/读取成功
	*返回0表示：云台未连接
	*返回-1表示：输入参数有误
	*返回-2表示：云台扫描中，不支持设置/读取
	*返回-3表示：设置/读取出错
	*返回-4表示：当前雷达不支持该功能
	*/
	int setScanDataEnable(unsigned int isOpen);
	int getScanDataEnable(unsigned int& isOpen);

	/*
	**   设置/读取数据流输出的格式。当搭载的雷达为multiscan或picoscan时支持
	*@ dataFormat:  开启标志位，1:MSGPACK, 2:Compact
	*返回1表示：设置/读取成功
	*返回0表示：云台未连接
	*返回-1表示：输入参数有误
	*返回-2表示：云台扫描中，不支持设置/读取
	*返回-3表示：设置/读取出错
	*返回-4表示：当前雷达不支持该功能
	*/
	int setScanDataFormat(int dataFormat);
	int getScanDataFormat(int& dataFormat);

	/*
	**   设置/读取雷达的输出角度范围
	*@ startAngle:  起始角度
	*@ stopAngle:  结束角度
	*返回1表示：设置/读取成功
	*返回0表示：云台未连接
	*返回-1表示：输入参数有误
	*返回-2表示：云台扫描中，不支持设置/读取
	*返回-3表示：设置/读取出错
	*返回-4表示：当前雷达不支持该功能
	*/
	int setAngleRange(float startAngle, float stopAngle);
	int getAngleRange(float& startAngle, float& stopAngle);

	/*
	**   设置/读取雷达的扫描频率和角度分辨率
	*@ frequency:  扫描频率
	*@ resolution:  角度分辨率
	*返回1表示：设置/读取成功
	*返回0表示：云台未连接
	*返回-1表示：输入参数有误
	*返回-2表示：云台扫描中，不支持设置/读取
	*返回-3表示：设置/读取出错
	*返回-4表示：当前雷达不支持该功能
	*/
	int setFrequencyAndResolution(float frequency, float resolution);
	int getFrequencyAndResolution(float& frequency, float& resolution);

	/*
	**   设置/读取数据输出内容的配置
	*@ rssi:  是否输出RSSI，0：否，1：是
	*@ rssiType:  RSSI值类型，0：8bit，1：16bit（针对LRS4000)
	*@ encoder:  是否输出编码器值，0：否，1：是
	*@ deviceName:  是否输出设备名称，0：否，1：是
	*@ timeStamp:  是否输出时间戳，0：否，1：是
	*@ outputinterval:  输出的轮廓间隔
	*返回1表示：设置/读取成功
	*返回0表示：云台未连接
	*返回-1表示：输入参数有误
	*返回-2表示：云台扫描中，不支持设置/读取
	*返回-3表示：设置/读取出错
	*返回-4表示：当前雷达不支持该功能
	*/
	int setDataContentConfigure(int rssi, int rssiType, int encoder, int deviceName, int timeStamp, int outputinterval = 1);
	int getDataContentConfigure(int& rssi, int& rssiType, int& encoder, int& deviceName, int& timeStamp, int& outputinterval);

	/*
	**   设置/读取雾过滤的配置
	*@ isActive:  开启标志位，1:开启, 0:关闭
	*@ sensitivityLevel：雾过滤等级，取值范围：1~6，该参数仅当雷达为LMS511时生效，其余型号雷达无需设置
	*返回1表示：设置/读取成功
	*返回0表示：云台未连接
	*返回-1表示：输入参数有误
	*返回-2表示：云台扫描中，不支持设置/读取
	*返回-3表示：设置/读取出错
	*返回-4表示：当前雷达不支持该功能
	*/
	int setFogFilter(unsigned short isActive, unsigned short sensitivityLevel = 3);
	int getFogFilter(unsigned short& isActive, unsigned short& sensitivityLevel);

	/*
	**   设置/读取粒子滤波配置
	*@ isActive:  开启标志位，1:开启, 0:关闭
	*返回1表示：设置/读取成功
	*返回0表示：云台未连接
	*返回-1表示：输入参数有误
	*返回-2表示：云台扫描中，不支持设置/读取
	*返回-3表示：设置/读取出错
	*返回-4表示：当前雷达不支持该功能
	*/
	int setParticleFilter(unsigned short isActive);
	int getParticleFilter(unsigned short& isActive);


	/*===========================  以下内容无需关注  ===========================*/
private:
	void	ClearResource();
	void	Add3DCloudToQue(const Double3DPointVec& cloudData);

	friend void LMS3DScanThreadCallbk(void* para);
	friend void CompactDataThreadCallbk(void* lpPara);
	friend void ControlSystemHeartBtThreadCallbk(void* para);

public:
	int innerSetParameter(std::string positon, std::string pw, float para);
	int innerReadParameter(std::string positon, float& para);
	int manualHeatingControl(bool isOpen, std::string pw);
	int getZeroPointTimeForTest(float &time); 

private:
	void* m_lidarSensor = nullptr;
	void* m_lidarCompactDataRecipient = nullptr;
	void* m_pControlSysSocket = nullptr;
	int m_isInitOk = 0;
	bool m_isChildThreadRunning;
	bool m_isCalculatePointOK;
	bool m_isNewest3DPointCloud;
	EPlatFormType m_deviceType;

	std::queue<Double3DPointVec> m_3dCloudQue;
	void* m_h3dCloudQueMutex;
	SLms3DPoint3DCloud m_sLms3DPoint3DCloud;
	Single3DPointVec m_sLmsSingleProfile;
	SLms3DPoint3DCloud m_sLms3DPointEcho2;
	Double3DPointVec m_3dCloudEcho2;
	Single3DPointVec m_singleProfileEcho2;

	std::vector<int> m_profileCounts;

#ifdef _WIN32
	void* m_laserScanThread;
	void* m_heartBtThread;
#else
	std::thread* m_laserScanThread;
	std::thread* m_heartBtThread;
#endif
	unsigned long m_laserScanThreadID;
	unsigned long m_heartBtThreadID;
	bool m_isLaserRunning = false;
	bool m_isPlatformRunning = false;

	//platform parameters
	std::string m_platformIP;
	std::string m_platformSubnetMask;
	std::string m_platformDefaultGateway;
	std::string m_firmwareVersionNo;

	float m_startAngle;
	float m_stopAngle;
	float m_scanVelocity;
	float m_stopPosition;
	float m_basicAngle;

	double m_deltaX;
	double m_deltaY;
	double m_deltaZ;
	float m_rotateX;
	float m_rotateY;
	float m_rotateZ;

	float m_modelOffset[2];
	float m_tranMat[16][4][4];
	bool m_isUseTranform;
	bool m_isAllEcho;
	double m_r[3][3];
	double m_t[3];

	//lidar parameters
	std::string m_lmsIP;
	std::string m_lmsSubnetMask;
	std::string m_lmsDefaultGateway;
	std::string m_lmsUdpIP;
	unsigned short m_lmsUdpPort;
	unsigned short m_lmsProtocal;

	unsigned int m_lmsScanDataEnableFlag;
	int m_lmsDataFormat;

	unsigned int m_lmsSerialNo;
	unsigned int m_lmsEchoFlag;
	float m_lmsFrequency;
	float m_lmsResolution;
	float m_lmsStartAngle;
	float m_lmsStopAngle;

	float m_lmsTemperature;
	int m_lmsRSSI;
	int m_lmsRSSIType;
	int m_lmsEncoder;
	int m_lmsDeviceName;
	int m_lmsTimeStamp;
	int m_lmsOutputInterval;
	int m_lmsScaleFactor;

	int m_lmsIncrementSource;
	int m_lmsEncoderSetting;

};