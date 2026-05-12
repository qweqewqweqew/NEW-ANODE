#pragma once
#include <string>

#include <thread>
#include <algorithm>
#include <chrono>

#include "PhoXi.h"

//注意以上头文件 需要在<windows.h> 之前 

#include "DataX.h"



class CPhoxiCamera
{
public:
	CPhoxiCamera();
	virtual ~CPhoxiCamera();

private:
	pho::api::PhoXiFactory Factory;
	pho::api::PPhoXi PhoXiDevice;


private:

	//mmind::eye::Camera  m_camera ; // 相机设备对象

	bool m_bOpen = false;


	bool m_bFileCamera = false;

     int m_iFrameID=-1;

public:
	bool EnumDevices(s_DevicesInfo& sInfo);

	bool IsOpen();


    bool Open(std::string HardwareIdentification); //以id打开

	bool SoftwareTrigger(pho::api::PFrame& Frame, unsigned int iTimeout);//触发--获取原始数据

	bool SoftwareTrigger( s_Image3dS &image3dS, unsigned int iTime= 10000);//触发--直接获得算法H格式图像数据

	void Close();




};

