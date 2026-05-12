#include "CPhoxiCamera.h"



CPhoxiCamera::CPhoxiCamera()
{
}


CPhoxiCamera::~CPhoxiCamera()
{
}


bool CPhoxiCamera::EnumDevices(s_DevicesInfo& sInfo)
{

	if (!Factory.isPhoXiControlRunning())
	{
		//std::cout << "PhoXi Control is not Running " << std::endl;

		return false;;

	}

	//std::cout << "PhoXi Control Version: " << Factory.GetPhoXiControlVersion() << std::endl;
	//std::cout << "PhoXi API Version: " << Factory.GetAPIVersion() << std::endl;


	std::vector <pho::api::PhoXiDeviceInformation> DeviceList;
	DeviceList = Factory.GetDeviceList();
	if (DeviceList.size()==0)
	{
		return false;
	}

	sInfo.iNum = DeviceList.size();
	for (int i = 0; i < DeviceList.size(); i++)
	{

		pho::api::PhoXiDeviceInformation* DeviceInfo;
		DeviceInfo = &DeviceList[i];

		//std::cout << "Device: " << i << std::endl;
		//std::cout << "  Name:                    " << DeviceInfo->Name << std::endl;
		//std::cout << "  Hardware Identification: " << DeviceInfo->HWIdentification << std::endl;
		//std::cout << "  Type:                    " << std::string(DeviceInfo->Type) << std::endl;
		//std::cout << "  Firmware version:        " << DeviceInfo->FirmwareVersion << std::endl;
		//std::cout << "  Variant:                 " << DeviceInfo->Variant << std::endl;
		//std::cout << "  IsFileCamera:            " << (DeviceInfo->IsFileCamera ? "Yes" : "No") << std::endl;
		//std::cout << "  Feature-Alpha:           " << (DeviceInfo->CheckFeature("Alpha") ? "Yes" : "No") << std::endl;
		//std::cout << "  Feature-Color:           " << (DeviceInfo->CheckFeature("Color") ? "Yes" : "No") << std::endl;
		//std::cout << "  Status:                  "
		//	<< (DeviceInfo->Status.Attached ? "Attached to PhoXi Control. " : "Not Attached to PhoXi Control. ")
		//	<< (DeviceInfo->Status.Ready ? "Ready to connect" : "Occupied")
		//	<< std::endl << std::endl;


		sInfo.sCameraInfo[i].SN = DeviceList[i].HWIdentification;
		sInfo.sCameraInfo[i].sModelName = std::string(DeviceList[i].Type);
		sInfo.sCameraInfo[i].sDeviceUserID = DeviceList[i].Name;
		//sInfo.sCameraInfo[i].IpAddr= DeviceList[i].ipAddress;                  //IP


	}

    return true;


}


bool CPhoxiCamera::IsOpen()
{
	return m_bOpen;
}


bool CPhoxiCamera::Open(std::string HardwareIdentification)
{

	if (HardwareIdentification == "")
	{
		return false;
	}

	//发现设备
	std::vector <pho::api::PhoXiDeviceInformation> DeviceList;
	DeviceList = Factory.GetDeviceList();
	if (DeviceList.size() == 0)
	{
		return false;
	}


	// 打开设备 --直接以id打开
	pho::api::PhoXiTimeout Timeout = pho::api::PhoXiTimeout::ZeroTimeout;
	PhoXiDevice = Factory.CreateAndConnect(HardwareIdentification, Timeout);
	if (PhoXiDevice == NULL)
	{
		return false;
	}

	// 检查设备是否真正连接
	if (!PhoXiDevice->isConnected())
	{
		return false;
	}

	// 设置触发模式为 Software（参考 PhoXi SDK 示例代码）
	// 如果设备不在 Software 触发模式，需要切换模式
	if (PhoXiDevice->TriggerMode != pho::api::PhoXiTriggerMode::Software)
	{
		// 如果设备正在采集，需要先停止采集
		if (PhoXiDevice->isAcquiring())
		{
			if (!PhoXiDevice->StopAcquisition())
			{
				return false;
			}
		}

		// 切换到 Software 触发模式
		PhoXiDevice->TriggerMode = pho::api::PhoXiTriggerMode::Software;

		// 检查设置是否成功
		if (!PhoXiDevice->TriggerMode.isLastOperationSuccessful())
		{
			return false;
		}
	}

	m_bOpen = true;

	return true;


}


bool CPhoxiCamera::SoftwareTrigger(pho::api::PFrame& Frame, unsigned int iTimeout)
{
	//Check if the device is connected
	if (!PhoXiDevice || !PhoXiDevice->isConnected())
	{
		return false;
	}
	//If it is not in Software trigger mode, we need to switch the modes
	if (PhoXiDevice->TriggerMode != pho::api::PhoXiTriggerMode::Software)
	{
		return false;
	}


	//Start the device acquisition, if necessary
	if (!PhoXiDevice->isAcquiring())
	{
		if (!PhoXiDevice->StartAcquisition())
		{
			return false;
		}
	}
	//We can clear the current Acquisition buffer -- This will not clear Frames that arrives to the PC after the Clear command is performed
	int ClearedFrames = PhoXiDevice->ClearBuffer();


	//While we checked the state of the StartAcquisition call, this check is not necessary, but it is a good practice
	if (!PhoXiDevice->isAcquiring())
	{
		return false;
	}


	int FrameID = PhoXiDevice->TriggerFrame(/*If false is passed here, the device will reject the frame if it is not ready to be triggered, if true us supplied, it will wait for the trigger*/);
	if (FrameID < 0)
	{
		//If negative number is returned trigger was unsuccessful
		return false;
	}

	 std::cout << "Frame was triggered, Frame Id: " << FrameID << std::endl;

	 //std::cout << "Waiting for frame "  << std::endl;
	 //Wait for a frame with specific FrameID. There is a possibility, that frame triggered before the trigger will arrive after the trigger call, and will be retrieved before requested frame
	 //  Because of this, the TriggerFrame call returns the requested frame ID, so it can than be retrieved from the Frame structure. This call is doing that internally in background


	/*pho::api::PFrame*/ Frame = PhoXiDevice->GetSpecificFrame(FrameID/*,iTimeout*//*, You can specify Timeout here - default is the Timeout stored in Timeout Feature -> Infinity by default*/);
	if (!Frame->Successful)
	{
		return false;
	}

	 m_iFrameID= FrameID;


	PhoXiDevice->StopAcquisition();

	return true;


}


bool CPhoxiCamera::SoftwareTrigger(s_Image3dS& image3dS, unsigned int iTimeout)
{

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // 触发
    //Check if the device is connected
    if (!PhoXiDevice || !PhoXiDevice->isConnected())
    {
        return false;
    }
    //If it is not in Software trigger mode, we need to switch the modes
    if (PhoXiDevice->TriggerMode != pho::api::PhoXiTriggerMode::Software)
    {
        return false;
    }


    //Start the device acquisition, if necessary
    if (!PhoXiDevice->isAcquiring())
    {
        if (!PhoXiDevice->StartAcquisition())
        {
            return false;
        }
    }
    //We can clear the current Acquisition buffer -- This will not clear Frames that arrives to the PC after the Clear command is performed
    int ClearedFrames = PhoXiDevice->ClearBuffer();


    //While we checked the state of the StartAcquisition call, this check is not necessary, but it is a good practice
    if (!PhoXiDevice->isAcquiring())
    {
        return false;
    }


    int FrameID = PhoXiDevice->TriggerFrame(/*If false is passed here, the device will reject the frame if it is not ready to be triggered, if true us supplied, it will wait for the trigger*/);
    if (FrameID < 0)
    {
        //If negative number is returned trigger was unsuccessful
        return false;
    }

    std::cout << "Frame was triggered, Frame Id: " << FrameID << std::endl;

    //std::cout << "Waiting for frame "  << std::endl;
    //Wait for a frame with specific FrameID. There is a possibility, that frame triggered before the trigger will arrive after the trigger call, and will be retrieved before requested frame
    //  Because of this, the TriggerFrame call returns the requested frame ID, so it can than be retrieved from the Frame structure. This call is doing that internally in background


    pho::api::PFrame Frame = PhoXiDevice->GetSpecificFrame(FrameID/*,iTimeout*//*, You can specify Timeout here - default is the Timeout stored in Timeout Feature -> Infinity by default*/);
    if (!Frame->Successful)
    {
        return false;
    }

    m_iFrameID = FrameID;


    PhoXiDevice->StopAcquisition();


    //数据内容的有效性
    if (Frame->Empty())
    {
        return false;
    }

    if (Frame->PointCloud.Empty())
    {
        return false;
    }
    if (Frame->NormalMap.Empty())
    {
        return false;

    }

    if (Frame->Texture.Empty())
    {
        /*return false;*/
        double ddd = 0;

    }

    if (Frame->TextureRGB.Empty())
    {
        /*return false;*/
        double ddd = 0;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////
    //图像提取到halcon格式
    ///////////////////////////////////////////////////////////////////////////////////////////

    try
    {
        ////////////////////////////////////////////////////////////////////////////////////////////////
        //2D图--灰度
        HObject hImageGray;


        if (!Frame->Texture.Empty())
        {
            HObject hTexture;
            GenImage1(&hTexture, "real", (HTuple)Frame->Texture.Size.Width, (HTuple)Frame->Texture.Size.Height, (Hlong)Frame->Texture.GetDataPtr());
            ScaleImageMax(hTexture, &hImageGray);

        }
        else if(!Frame->TextureRGB.Empty())
        {
            //用彩色图转灰度

            /*return false;*/
            double ddd = 0;

            int iTextureRGBLen = Frame->TextureRGB.Size.Width * Frame->TextureRGB.Size.Height;

            uint16_t* redArr = new uint16_t[iTextureRGBLen];
            uint16_t* greenArr = new uint16_t[iTextureRGBLen];
            uint16_t* blueArr = new uint16_t[iTextureRGBLen];

            for (int idx = 0; idx < iTextureRGBLen; ++idx)
            {
                int i = idx / Frame->TextureRGB.Size.Width;
                int j = idx % Frame->TextureRGB.Size.Width;

                auto rgb = Frame->TextureRGB.At(i, j);


                redArr[idx] = rgb.r;
                greenArr[idx] = rgb.g;
                blueArr[idx] = rgb.b;
            }

            try
            {
                HObject hImageColor;
                HObject hTexture;

                GenImage3(&hImageColor, "uint2", (int) Frame->TextureRGB.Size.Width, (int)Frame->TextureRGB.Size.Height, (Hlong)redArr, (Hlong)greenArr, (Hlong)blueArr);
                //WriteImage(hImageColor, "bmp", 0, "hImageColor");

                Rgb1ToGray(hImageColor, &hTexture);
                ScaleImageMax(hTexture, &hImageGray);


                delete[] redArr;
                delete[] greenArr;
                delete[] blueArr;

                redArr = nullptr;
                greenArr = nullptr;
                blueArr = nullptr;

            }
            catch (HalconCpp::HException& ex)
            {


                delete[] redArr;
                delete[] greenArr;
                delete[] blueArr;

                redArr = nullptr;
                greenArr = nullptr;
                blueArr = nullptr;

            }




        }
        else
        {

            return false;
        }


        //WriteImage(hImageGray, "bmp", 0, "hImageGray");
        std::cout << "hImageGray finish" << std::endl;


        ////////////////////////////////////////////////////////////////////////////////////////////////

        // 获取宽高
        int height = Frame->PointCloud.Size.Height;    // 行数（高）
        int width = Frame->PointCloud.Size.Width;    // 列数（宽）
        int  totalPoints = width * height;

        // 在堆上分配数组
        float* ArrayX = new float[totalPoints];
        float* ArrayY = new float[totalPoints];
        float* ArrayZ = new float[totalPoints];
        float* ArrayNX = new float[totalPoints];
        float* ArrayNY = new float[totalPoints];
        float* ArrayNZ = new float[totalPoints];


        //? 并行提取（OpenMP）
#pragma omp parallel for if(totalPoints > 50000)
        for (int idx = 0; idx < totalPoints; ++idx)
        {
            int i = idx / width;
            int j = idx % width;

            auto pt = Frame->PointCloud.At(i, j);
            auto nm = Frame->NormalMap.At(i, j);

            ArrayX[idx] = pt.x;
            ArrayY[idx] = pt.y;
            ArrayZ[idx] = pt.z;

            ArrayNX[idx] = nm.x;
            ArrayNY[idx] = nm.y;
            ArrayNZ[idx] = nm.z;
        }



        // 转换成halcon格式map图像
        HObject ho_X, ho_Y, ho_Z, ho_NX, ho_NY, ho_NZ;

        try
        {
            GenImage1(&ho_X, "real", width, height, (Hlong)ArrayX);
            GenImage1(&ho_Y, "real", width, height, (Hlong)ArrayY);
            GenImage1(&ho_Z, "real", width, height, (Hlong)ArrayZ);

            GenImage1(&ho_NX, "real", width, height, (Hlong)ArrayNX);
            GenImage1(&ho_NY, "real", width, height, (Hlong)ArrayNY);
            GenImage1(&ho_NZ, "real", width, height, (Hlong)ArrayNZ);

            delete[] ArrayX;
            delete[] ArrayY;
            delete[] ArrayZ;
            delete[] ArrayNX;
            delete[] ArrayNY;
            delete[] ArrayNZ;


            ArrayX = nullptr;
            ArrayY = nullptr;
            ArrayZ = nullptr;
            ArrayNX = nullptr;
            ArrayNY = nullptr;
            ArrayNZ = nullptr;

        }
        catch (HalconCpp::HException& ex)
        {

            delete[] ArrayX;
            delete[] ArrayY;
            delete[] ArrayZ;
            delete[] ArrayNX;
            delete[] ArrayNY;
            delete[] ArrayNZ;


            ArrayX = nullptr;
            ArrayY = nullptr;
            ArrayZ = nullptr;
            ArrayNX = nullptr;
            ArrayNY = nullptr;
            ArrayNZ = nullptr;

        }

        std::cout << "xyznxnynz finish" << std::endl;

        //去除无效数据
        HObject ho_Region;
        Threshold(ho_Z, &ho_Region, 1, 99999);
        ReduceDomain(ho_X, ho_Region, &ho_X);
        ReduceDomain(ho_Y, ho_Region, &ho_Y);
        ReduceDomain(ho_Z, ho_Region, &ho_Z);

        ReduceDomain(ho_NX, ho_Region, &ho_NX);
        ReduceDomain(ho_NY, ho_Region, &ho_NY);
        ReduceDomain(ho_NZ, ho_Region, &ho_NZ);




        std::cout << "hImage3d finish" << std::endl;

        //WriteImage(ho_X, "tiff", 0, "ho_X");
        //WriteImage(ho_Y, "tiff", 0, "ho_Y");
        //WriteImage(ho_Z, "tiff", 0, "ho_Z");

        //WriteImage(ho_NX, "tiff", 0, "ho_NX");
        //WriteImage(ho_NY, "tiff", 0, "ho_NY");
        //WriteImage(ho_NZ, "tiff", 0, "ho_NZ");


        //添加到3ds

        m_iFrameID++;

        //拷贝原始信息
        s_Image3dS As;
        As.Gray = hImageGray.Clone();
        //As.Color = hImageColor.Clone();
        As.X = ho_X.Clone();
        As.Y = ho_Y.Clone();
        As.Z = ho_Z.Clone();

        As.NX = ho_NX.Clone();
        As.NY = ho_NY.Clone();
        As.NZ = ho_NZ.Clone();

        As.ID = m_iFrameID;

        image3dS = As;

    }
    catch (HalconCpp::HException& ex)
    {

        return false;

    }


    return true;


}



void CPhoxiCamera::Close()
{
	if (PhoXiDevice && PhoXiDevice->isConnected())
	{
		// 如果设备正在采集，先停止采集
		if (PhoXiDevice->isAcquiring())
		{
			PhoXiDevice->StopAcquisition();
		}

		// 断开设备连接
		PhoXiDevice->Disconnect(true);
	}

	// 重置状态标志
	m_bOpen = false;
	m_iFrameID = -1;
}

