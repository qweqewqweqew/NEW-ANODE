#include "pch.h"
#include "CUnloadPlateA.h"

#include "CHVisionAdvX.h"


CUnloadPlateA::CUnloadPlateA()
{
}


CUnloadPlateA::~CUnloadPlateA()
{


}



void CUnloadPlateA::calc_plane_point_z_x(HTuple hv_PlanePara, HTuple hv_x, HTuple hv_y, HTuple* hv_z)
{

	// Local iconic variables



	////AX+BY+CZ-D=0;
	////Z = (-AX-BY+D)/C = -(AX+BY-D)/C
	if (0 != (int((hv_PlanePara.TupleLength()) < 4)))
	{
		return;
	}

	if (0 != (int(HTuple(hv_PlanePara[3]) == 0)))
	{
		return;
	}


	(*hv_z) = ((((-HTuple(hv_PlanePara[0])) * hv_x) - (HTuple(hv_PlanePara[1]) * hv_y)) + HTuple(hv_PlanePara[3])) / HTuple(hv_PlanePara[2]);


	return;
}



void CUnloadPlateA::get_box_corner_x(HTuple hv_Length11, HTuple hv_Length21, HTuple hv_Length31,
	HTuple hv_PoseBox, HTuple* hv_CornersWorldX, HTuple* hv_CornersWorldY, HTuple* hv_CornersWorldZ)
{

	// Local iconic variables

	// Local control variables
	HTuple  hv_Length, hv_Width, hv_Height, hv_L2;
	HTuple  hv_W2, hv_H2, hv_CornersObjX, hv_CornersObjY, hv_CornersObjZ;
	HTuple  hv_HomMat3D, hv_OM3CrossList, hv_i, hv_Xw, hv_Yw;
	HTuple  hv_Zw;


	//Given: Pose, Length, Width, Height
	hv_Length = hv_Length11;
	hv_Width = hv_Length21;
	hv_Height = hv_Length31;

	//Local corners
	hv_L2 = hv_Length / 2.0;
	hv_W2 = hv_Width / 2.0;
	hv_H2 = hv_Height / 2.0;

	hv_CornersObjX.Clear();
	hv_CornersObjX.Append(-hv_L2);
	hv_CornersObjX.Append(hv_L2);
	hv_CornersObjX.Append(hv_L2);
	hv_CornersObjX.Append(-hv_L2);
	hv_CornersObjX.Append(-hv_L2);
	hv_CornersObjX.Append(hv_L2);
	hv_CornersObjX.Append(hv_L2);
	hv_CornersObjX.Append(-hv_L2);
	hv_CornersObjY.Clear();
	hv_CornersObjY.Append(-hv_W2);
	hv_CornersObjY.Append(-hv_W2);
	hv_CornersObjY.Append(hv_W2);
	hv_CornersObjY.Append(hv_W2);
	hv_CornersObjY.Append(-hv_W2);
	hv_CornersObjY.Append(-hv_W2);
	hv_CornersObjY.Append(hv_W2);
	hv_CornersObjY.Append(hv_W2);
	hv_CornersObjZ.Clear();
	hv_CornersObjZ.Append(-hv_H2);
	hv_CornersObjZ.Append(-hv_H2);
	hv_CornersObjZ.Append(-hv_H2);
	hv_CornersObjZ.Append(-hv_H2);
	hv_CornersObjZ.Append(hv_H2);
	hv_CornersObjZ.Append(hv_H2);
	hv_CornersObjZ.Append(hv_H2);
	hv_CornersObjZ.Append(hv_H2);


	PoseToHomMat3d(hv_PoseBox, &hv_HomMat3D);

	//Transform
	(*hv_CornersWorldX) = HTuple();
	(*hv_CornersWorldY) = HTuple();
	(*hv_CornersWorldZ) = HTuple();

	hv_OM3CrossList = HTuple();
	for (hv_i = 0; hv_i <= 7; hv_i += 1)
	{
		AffineTransPoint3d(hv_HomMat3D, HTuple(hv_CornersObjX[hv_i]), HTuple(hv_CornersObjY[hv_i]),
			HTuple(hv_CornersObjZ[hv_i]), &hv_Xw, &hv_Yw, &hv_Zw);
		(*hv_CornersWorldX) = (*hv_CornersWorldX).TupleConcat(hv_Xw);
		(*hv_CornersWorldY) = (*hv_CornersWorldY).TupleConcat(hv_Yw);
		(*hv_CornersWorldZ) = (*hv_CornersWorldZ).TupleConcat(hv_Zw);

	}
	return;
}



// Chapter: Graphics / Output
// Short Description: Compute the center of all given 3D object models. 
void CUnloadPlateA::get_object_models_center(HTuple hv_ObjectModel3DID, HTuple* hv_Center)
{

	// Local iconic variables

	// Local control variables
	HTuple  hv_Diameters, hv_Index, hv_Diameter, hv_C;
	HTuple  hv_Exception, hv_MD, hv_Weight, hv_SumW, hv_ObjectModel3DIDSelected;
	HTuple  hv_InvSum;

	//Compute the mean of all model centers (weighted by the diameter of the object models)
	hv_Diameters = HTuple(hv_ObjectModel3DID.TupleLength(), 0.0);
	{
		HTuple end_val2 = (hv_ObjectModel3DID.TupleLength()) - 1;
		HTuple step_val2 = 1;
		for (hv_Index = 0; hv_Index.Continue(end_val2, step_val2); hv_Index += step_val2)
		{
			try
			{
				GetObjectModel3dParams(HTuple(hv_ObjectModel3DID[hv_Index]), "diameter_axis_aligned_bounding_box",
					&hv_Diameter);
				GetObjectModel3dParams(HTuple(hv_ObjectModel3DID[hv_Index]), "center", &hv_C);
				hv_Diameters[hv_Index] = hv_Diameter;
			}
			// catch (Exception) 
			catch (HException& HDevExpDefaultException)
			{
				HDevExpDefaultException.ToHTuple(&hv_Exception);
				//Object model is empty, has no center etc. -> ignore it by leaving its diameter at zero
			}
		}
	}

	if (0 != (int((hv_Diameters.TupleSum()) > 0)))
	{
		//Normalize Diameter to use it as weights for a weighted mean of the individual centers
		hv_MD = (hv_Diameters.TupleSelectMask(hv_Diameters.TupleGreaterElem(0))).TupleMean();
		if (0 != (int(hv_MD > 1e-10)))
		{
			hv_Weight = hv_Diameters / hv_MD;
		}
		else
		{
			hv_Weight = hv_Diameters;
		}
		hv_SumW = hv_Weight.TupleSum();
		if (0 != (int(hv_SumW < 1e-10)))
		{
			hv_Weight = HTuple(hv_Weight.TupleLength(), 1.0);
			hv_SumW = hv_Weight.TupleSum();
		}
		(*hv_Center).Clear();
		(*hv_Center)[0] = 0;
		(*hv_Center)[1] = 0;
		(*hv_Center)[2] = 0;
		{
			HTuple end_val26 = (hv_ObjectModel3DID.TupleLength()) - 1;
			HTuple step_val26 = 1;
			for (hv_Index = 0; hv_Index.Continue(end_val26, step_val26); hv_Index += step_val26)
			{
				if (0 != (int(HTuple(hv_Diameters[hv_Index]) > 0)))
				{
					hv_ObjectModel3DIDSelected = HTuple(hv_ObjectModel3DID[hv_Index]);
					GetObjectModel3dParams(hv_ObjectModel3DIDSelected, "center", &hv_C);
					(*hv_Center)[0] = HTuple((*hv_Center)[0]) + (HTuple(hv_C[0]) * HTuple(hv_Weight[hv_Index]));
					(*hv_Center)[1] = HTuple((*hv_Center)[1]) + (HTuple(hv_C[1]) * HTuple(hv_Weight[hv_Index]));
					(*hv_Center)[2] = HTuple((*hv_Center)[2]) + (HTuple(hv_C[2]) * HTuple(hv_Weight[hv_Index]));
				}
			}
		}
		hv_InvSum = 1.0 / hv_SumW;
		(*hv_Center)[0] = HTuple((*hv_Center)[0]) * hv_InvSum;
		(*hv_Center)[1] = HTuple((*hv_Center)[1]) * hv_InvSum;
		(*hv_Center)[2] = HTuple((*hv_Center)[2]) * hv_InvSum;
	}
	else
	{
		(*hv_Center) = HTuple();
	}
	return;
}

void CUnloadPlateA::calc_rectangle2_points_x(HTuple hv_rowCenter, HTuple hv_columnCenter, HTuple hv_phi,
	HTuple hv_length1, HTuple hv_length2, HTuple* hv_pt1, HTuple* hv_pt2, HTuple* hv_pt3,
	HTuple* hv_pt4)
{

	// Local iconic variables
	HObject  ho_Cross;

	// Local control variables
	HTuple  hv_pi, hv_Tem, hv_la, hv_lb, hv_tem1;
	HTuple  hv_tem2, hv_xLength1, hv_xLength2, hv_yLength1;
	HTuple  hv_yLength2, hv_mColumnUpLeft, hv_nRowUpLeft, hv_mColumnDownLeft;
	HTuple  hv_nRowDownLeft, hv_mColumnUpRight, hv_nRowUpRight;
	HTuple  hv_mColumnDownRight, hv_nRowDownRight, hv_row, hv_column;


	(*hv_pt1) = HTuple();
	(*hv_pt2) = HTuple();
	(*hv_pt3) = HTuple();
	(*hv_pt4) = HTuple();


	//注：以与水平方向所成角度较大的边中线为轴线
	//定义输入参数
	GenEmptyObj(&ho_Cross);
	hv_pi = HTuple(180).TupleRad();

	//假设 rowCenter, columnCenter, phi, length1, length2 已知并已赋值

	if (0 != (HTuple(int(hv_phi >= 0)).TupleAnd(int(hv_phi < (hv_pi / 4)))))
	{
		hv_phi = hv_phi - (hv_pi / 2);
		hv_Tem = hv_length1;
		hv_length1 = hv_length2;
		hv_length2 = hv_Tem;
	}
	else if (0 != (HTuple(int(hv_phi > ((-hv_pi) / 4))).TupleAnd(int(hv_phi < 0))))
	{
		hv_phi += hv_pi / 2;
		hv_Tem = hv_length1;
		hv_length1 = hv_length2;
		hv_length2 = hv_Tem;
	}

	if (0 != (int(hv_phi >= 0)))
	{
		hv_la = hv_phi;
		hv_lb = hv_la - (hv_pi / 2);
		hv_tem1 = hv_la.TupleTan();
		hv_tem2 = hv_lb.TupleTan();
		hv_xLength1 = ((hv_length1 * hv_length1) / (1 + (hv_tem1 * hv_tem1))).TupleSqrt();
		hv_xLength2 = ((hv_length2 * hv_length2) / (1 + (hv_tem2 * hv_tem2))).TupleSqrt();
		hv_yLength1 = ((((hv_tem1 * hv_tem1) * hv_length1) * hv_length1) / (1 + (hv_tem1 * hv_tem1))).TupleSqrt();
		hv_yLength2 = ((((hv_tem2 * hv_tem2) * hv_length2) * hv_length2) / (1 + (hv_tem2 * hv_tem2))).TupleSqrt();

		hv_mColumnUpLeft = (hv_columnCenter + hv_xLength1) - hv_xLength2;
		hv_nRowUpLeft = (hv_rowCenter - hv_yLength1) - hv_yLength2;
		hv_mColumnDownLeft = (hv_columnCenter - hv_xLength1) - hv_xLength2;
		hv_nRowDownLeft = (hv_rowCenter + hv_yLength1) - hv_yLength2;
		hv_mColumnUpRight = (hv_columnCenter + hv_xLength1) + hv_xLength2;
		hv_nRowUpRight = (hv_rowCenter - hv_yLength1) + hv_yLength2;
		hv_mColumnDownRight = (hv_columnCenter - hv_xLength1) + hv_xLength2;
		hv_nRowDownRight = (hv_rowCenter + hv_yLength1) + hv_yLength2;
	}
	else
	{
		hv_la = hv_phi;
		hv_lb = hv_la - (hv_pi / 2);
		hv_tem1 = hv_la.TupleTan();
		hv_tem2 = hv_lb.TupleTan();
		hv_xLength1 = ((hv_length1 * hv_length1) / (1 + (hv_tem1 * hv_tem1))).TupleSqrt();
		hv_xLength2 = ((hv_length2 * hv_length2) / (1 + (hv_tem2 * hv_tem2))).TupleSqrt();
		hv_yLength1 = ((((hv_tem1 * hv_tem1) * hv_length1) * hv_length1) / (1 + (hv_tem1 * hv_tem1))).TupleSqrt();
		hv_yLength2 = ((((hv_tem2 * hv_tem2) * hv_length2) * hv_length2) / (1 + (hv_tem2 * hv_tem2))).TupleSqrt();

		hv_mColumnUpLeft = (hv_columnCenter - hv_xLength1) - hv_xLength2;
		hv_nRowUpLeft = (hv_rowCenter - hv_yLength1) + hv_yLength2;
		hv_mColumnDownLeft = (hv_columnCenter + hv_xLength1) - hv_xLength2;
		hv_nRowDownLeft = (hv_rowCenter + hv_yLength1) + hv_yLength2;
		hv_mColumnUpRight = (hv_columnCenter - hv_xLength1) + hv_xLength2;
		hv_nRowUpRight = (hv_rowCenter - hv_yLength1) - hv_yLength2;
		hv_mColumnDownRight = (hv_columnCenter + hv_xLength1) + hv_xLength2;
		hv_nRowDownRight = (hv_rowCenter + hv_yLength1) - hv_yLength2;
	}

	hv_row.Clear();
	hv_row.Append(hv_nRowUpLeft);
	hv_row.Append(hv_nRowUpRight);
	hv_row.Append(hv_nRowDownRight);
	hv_row.Append(hv_nRowDownLeft);
	hv_column.Clear();
	hv_column.Append(hv_mColumnUpLeft);
	hv_column.Append(hv_mColumnUpRight);
	hv_column.Append(hv_mColumnDownRight);
	hv_column.Append(hv_mColumnDownLeft);

	//输出点坐标

	(*hv_pt1)[0] = HTuple(hv_column[0]);
	(*hv_pt1)[1] = HTuple(hv_row[0]);

	(*hv_pt2)[0] = HTuple(hv_column[1]);
	(*hv_pt2)[1] = HTuple(hv_row[1]);

	(*hv_pt3)[0] = HTuple(hv_column[2]);
	(*hv_pt3)[1] = HTuple(hv_row[2]);

	(*hv_pt4)[0] = HTuple(hv_column[3]);
	(*hv_pt4)[1] = HTuple(hv_row[3]);



	return;
}



// Procedures 
void CUnloadPlateA::gen_cross3d_x(HTuple hv_CrossThickness, HTuple hv_CrossLength, HTuple hv_CrossPoseT,
	HTuple* hv_OM3Cross)
{

	// Local control variables
	HTuple  hv_CubeBoxX, hv_CubeBoxY, hv_CubeBoxZ;
	HTuple  hv_ConvexX, hv_ConvexY, hv_ConvexZ, hv_ConvexCross3DZero;
	HTuple  hv_ArrowPose, hv_HomMat3D;



	GenBoxObjectModel3d(((((((HTuple(0).Append(0)).Append(0)).Append(0)).Append(0)).Append(0)).Append(0)),
		hv_CrossLength * 1.2, hv_CrossThickness, hv_CrossThickness, &hv_CubeBoxX);
	GenBoxObjectModel3d(((((((HTuple(0).Append(0)).Append(0)).Append(0)).Append(0)).Append(0)).Append(0)),
		hv_CrossThickness, hv_CrossLength * 1.0, hv_CrossThickness, &hv_CubeBoxY);
	GenBoxObjectModel3d(((((((HTuple(0).Append(0)).Append(0)).Append(0)).Append(0)).Append(0)).Append(0)),
		hv_CrossThickness, hv_CrossThickness, hv_CrossLength * 0.8, &hv_CubeBoxZ);


	ConvexHullObjectModel3d(hv_CubeBoxX, &hv_ConvexX);
	ConvexHullObjectModel3d(hv_CubeBoxY, &hv_ConvexY);
	ConvexHullObjectModel3d(hv_CubeBoxZ, &hv_ConvexZ);
	UnionObjectModel3d((hv_ConvexZ.TupleConcat(hv_ConvexY)).TupleConcat(hv_ConvexX),
		"points_surface", &hv_ConvexCross3DZero);

	CreatePose(HTuple(hv_CrossPoseT[0]), HTuple(hv_CrossPoseT[1]), HTuple(hv_CrossPoseT[2]),
		HTuple(hv_CrossPoseT[3]), HTuple(hv_CrossPoseT[4]), HTuple(hv_CrossPoseT[5]),
		"Rp+T", "gba", "point", &hv_ArrowPose);
	PoseToHomMat3d(hv_ArrowPose, &hv_HomMat3D);
	AffineTransObjectModel3d(hv_ConvexCross3DZero, hv_HomMat3D, &(*hv_OM3Cross));


	ClearObjectModel3d((hv_ConvexZ.TupleConcat(hv_ConvexY)).TupleConcat(hv_ConvexX));
	ClearObjectModel3d(hv_ConvexCross3DZero);

	return;
}


//s_Rtnf CUnloadPlateA::OnPreProcess(s_Image3dS sImage3dS, s_PreProcess3DSPara spara1, s_PreProcess3DSResultPara& spara2)
//{
//
//	s_Rtnf sError;
//
//	sError.iCode = -1;
//	sError.strInfo = "";
//
//	spara2.bTJG = false;
//	spara2.time = 0.0;
//	spara2.sImage3dPro.Clear();
//
//
//	HTuple time_start = 0.0;
//	HTuple time_end = 0.0;
//	CountSeconds(&time_start);
//
//
//	// Local iconic variables 
//	HObject ho_X, ho_Y, ho_Z, ho_NX;
//	HObject ho_NY, ho_NZ, /*ho_Texture ,*/ ho_GrayImage, ho_ColorImage;
//	HObject ho_Region, ho_X1, ho_Y1, ho_Z1;
//	HObject ho_Region1, ho_Region2, ho_Region3;
//	HObject ho_NX1, ho_NY1, ho_NZ1;
//	HObject ho_Domain, ho_GrayImage1, ho_ColorImage1;
//
//	HObject ho_Image1, ho_Image2;
//	HObject ho_Image3, ho_ImageH, ho_ImageS;
//	HObject ho_ImageV, ho_ImageR, ho_ImageG;
//	HObject ho_ImageB, ho_Multichannel;
//
//	HObject X, Y, Z;
//	HObject NX, NY, NZ;
//	HObject Gray, Color;
//
//
//	// Local control variables 
//
//	HTuple hv_ImageWidth;
//	HTuple hv_ImageHeight, hv_WindowWidth;
//	HTuple hv_WindowHeight;
//	HTuple hv_GenParamName, hv_GenParamValue;
//
//	HTuple hv_ObjectModel3D;
//	HTuple hv_Rows, hv_Columns;
//	HTuple hv_NXV, hv_NYV, hv_NZV;
//	HTuple hv_Intensity, hv_Pose;
//	HTuple hv_ObjectModel3DConnected, hv_ObjectModel3DSelected;
//	HTuple hv_ObjectModel3DSelected2, hv_UnionObjectModel3D;
//
//	HTuple hv_SampledObjectModel3DS1;
//	HTuple hv_ObjectModel3DConnectedS1, hv_GPVS1;
//	HTuple hv_MaxS1, hv_ObjectModel3DSelectedS1a;
//	HTuple hv_ObjectModel3DConnectedS1a, hv_GPVS1a;
//	HTuple hv_MaxS1a, hv_ObjectModel3DSelectedS1b;
//	HTuple hv_UnionObjectModel3DS1, hv_OM3DPlane;
//	HTuple hv_ParFitting, hv_ValFitting;
//	HTuple hv_ObjectModel3DOutID, hv_GenParamPosePlane;
//	HTuple hv_PlaneDatum, hv_GenParamValueDistancePlane;
//	HTuple hv_ObjectModel3DThresholdedS1c, hv_PoseOutA;
//	HTuple hv_UnionObjectModel3DS12, hv_ObjectModel3DConnectedS1d;
//	HTuple hv_ObjectModel3DSelectedS1d;
//
//
//	HTuple hv_RV, hv_GV, hv_BV;
//	HTuple hv_GvBoundingBox1S0;
//	HTuple hv_lObjectModel3DS0, hv_ObjectModel3DSelectedS0;
//
//	HTuple hv_NumPts;
//
//
//
//	try
//	{
//
//		s_Image3dS Image = sImage3dS;
//
//
//		//判断输入
//		if (!Image.Z.IsInitialized() || Image.Z.CountObj() == 0)
//		{
//			CountSeconds(&time_end);
//			spara2.time = time_end - time_start;
//
//			sError.iCode = 1;
//			sError.strInfo = "图像为空!";
//			return sError;
//		}
//
//
//		//处理算法
//
//
//
//		//////特殊处理--图像进行旋转及点云坐标系方向做调整  20251027 相机旋转方向后取消
//
//		////图像旋转及坐标系变换
//		//RotateImage(Image.Gray, &Gray, 180, "constant");
//		//RotateImage(Image.Color, &Color, 180, "constant");
//		//RotateImage(Image.X, &X, 180, "constant");
//		//RotateImage(Image.Y, &Y, 180, "constant");
//		//RotateImage(Image.Z, &Z, 180, "constant");
//		//RotateImage(Image.NX, &NX, 180, "constant");
//		//RotateImage(Image.NY, &NY, 180, "constant");
//		//RotateImage(Image.NZ, &NZ, 180, "constant");
//
//		Gray = Image.Gray.Clone();
//		Color = Image.Color.Clone();
//
//
//		//旋转后无效值变成0了 去除
//		Threshold(Image.Z, &ho_Region, 10, 3000);
//		ReduceDomain(Image.X, ho_Region, &X);
//		ReduceDomain(Image.Y, ho_Region, &Y);
//		ReduceDomain(Image.Z, ho_Region, &Z);
//		ReduceDomain(Image.NX, ho_Region, &NX);
//		ReduceDomain(Image.NY, ho_Region, &NY);
//		ReduceDomain(Image.NZ, ho_Region, &NZ);
//
//
//		//ScaleImage(X, &X, -1, 0);//X方向反向   20251027 相机旋转方向后取消
//		//ScaleImage(Y, &Y, -1, 0); //Y方向反向
//		//
//
//		//灰度自动调整--更新外部光源控制后 需去掉
//		HObject ho_ROI_0, ho_ImageTextureReduced, ho_ImageScaled;
//		HTuple hv_Mean, hv_Deviation;
//
//		CHVisionAdvX lHVisionAdvXObj;
//		GenRectangle1(&ho_ROI_0, 446.399, 751.255, 893.64, 1353.46);
//		ReduceDomain(Gray, ho_ROI_0, &ho_ImageTextureReduced);
//		Intensity(ho_ROI_0, ho_ImageTextureReduced, &hv_Mean, &hv_Deviation);
//		lHVisionAdvXObj.scale_image_range(Gray, &ho_ImageScaled, hv_Mean - 6 * hv_Deviation, hv_Mean + 6 * hv_Deviation);
//		CopyImage(ho_ImageScaled, &Gray);
//
//
//
//		//ROI区域图像 并给点云附加属性
//		Threshold(Z, &ho_Region1, spara1.fRoiZMin, spara1.fRoiZMax);
//		Threshold(X, &ho_Region2, spara1.fRoiXMin, spara1.fRoiXMax);
//		Threshold(Y, &ho_Region3, spara1.fRoiYMin, spara1.fRoiYMax);
//		Intersection(ho_Region1, ho_Region2, &ho_Region);
//		Intersection(ho_Region3, ho_Region, &ho_Region);
//
//
//		ReduceDomain(X, ho_Region, &ho_X);
//		ReduceDomain(Y, ho_Region, &ho_Y);
//		ReduceDomain(Z, ho_Region, &ho_Z);
//		ReduceDomain(NX, ho_Region, &ho_NX);
//		ReduceDomain(NY, ho_Region, &ho_NY);
//		ReduceDomain(NZ, ho_Region, &ho_NZ);
//		//ReduceDomain(Gray, ho_Region, &ho_GrayImage);   //灰度图不截取 
//		//ReduceDomain(Color, ho_Region, &ho_ColorImage);
//		ho_GrayImage = Gray.Clone();
//		ho_ColorImage = Color.Clone();
//
//
//		XyzToObjectModel3d(ho_X, ho_Y, ho_Z, &hv_ObjectModel3D);
//		GetObjectModel3dParams(hv_ObjectModel3D, "num_points", &hv_NumPts);
//		GetRegionPoints(ho_Region, &hv_Rows, &hv_Columns);
//
//
//
//
//		//法线属性
//		try
//		{
//
//			GetGrayval(ho_NX, hv_Rows, hv_Columns, &hv_NXV);
//			GetGrayval(ho_NY, hv_Rows, hv_Columns, &hv_NYV);
//			GetGrayval(ho_NZ, hv_Rows, hv_Columns, &hv_NZV);
//
//
//			SetObjectModel3dAttribMod(hv_ObjectModel3D, ((HTuple("point_normal_x").Append("point_normal_y")).Append("point_normal_z")),
//				HTuple(), (hv_NXV.TupleConcat(hv_NYV)).TupleConcat(hv_NZV));
//
//		}
//		catch (...)
//		{
//
//
//		}
//
//
//		//灰度属性
//		try
//		{
//
//			GetGrayval(ho_GrayImage, hv_Rows, hv_Columns, &hv_Intensity);
//			SetObjectModel3dAttribMod(hv_ObjectModel3D, "&intensity", "points", hv_Intensity);
//		}
//		catch (...)
//		{
//
//
//		}
//
//
//
//		//try
//		//{
//		//	//法线渲染成RGB
//
//
//		//	ScaleImageMax(ho_NX, &ho_Image1);
//		//	ScaleImageMax(ho_NY, &ho_Image2);
//		//	ScaleImageMax(ho_NZ, &ho_Image3);
//		//	TransFromRgb(ho_Image1, ho_Image2, ho_Image3, &ho_ImageH, &ho_ImageS, &ho_ImageV, "hsv");
//		//	TransToRgb(ho_ImageH, ho_ImageS, ho_ImageV, &ho_ImageR, &ho_ImageG, &ho_ImageB, "hsv");
//		//	Compose3(ho_ImageR, ho_ImageG, ho_ImageB, &ho_Multichannel);
//
//		//	GetGrayval(ho_ImageR, hv_Rows, hv_Columns, &hv_RV);
//		//	GetGrayval(ho_ImageG, hv_Rows, hv_Columns, &hv_GV);
//		//	GetGrayval(ho_ImageB, hv_Rows, hv_Columns, &hv_BV);
//		//	SetObjectModel3dAttribMod(hv_ObjectModel3D, "&red", "points", hv_RV);
//		//	SetObjectModel3dAttribMod(hv_ObjectModel3D, "&green", "points", hv_GV);
//		//	SetObjectModel3dAttribMod(hv_ObjectModel3D, "&blue", "points", hv_BV);
//		//}
//		//catch (...)
//		//{
//
//
//		//}
//
//
//
//
//
//		//聚类分割  根据点数 直径
//		if (true)
//		{
//
//			ConnectionObjectModel3d(hv_ObjectModel3D, "distance_3d", spara1.fThSeg0Dis,
//				&hv_ObjectModel3DConnected);
//
//			SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points",
//				"and", spara1.iThSeg0NumPtsMin, spara1.iThSeg0NumPtsMax, &hv_ObjectModel3DSelected);
//
//			SelectObjectModel3d(hv_ObjectModel3DSelected, "diameter_bounding_box",
//				"and", spara1.fThSeg0DiameterMin, spara1.fThSeg0DiameterMax, &hv_ObjectModel3DSelected2);
//
//			UnionObjectModel3d(hv_ObjectModel3DSelected2, "points_surface", &hv_UnionObjectModel3D);
//		}
//
//
//
//
//		GetObjectModel3dParams(hv_UnionObjectModel3D, "num_points", &hv_NumPts);
//
//		//根据处理后点云区域 获得XYZ NXYZ Gray 
//
//		ObjectModel3dToXyz(&ho_X1, &ho_Y1, &ho_Z1, hv_UnionObjectModel3D, "from_xyz_map", HTuple(), HTuple());
//
//		GetDomain(ho_Z1, &ho_Domain);
//
//		ReduceDomain(ho_NX, ho_Domain, &ho_NX1);
//		ReduceDomain(ho_NY, ho_Domain, &ho_NY1);
//		ReduceDomain(ho_NZ, ho_Domain, &ho_NZ1);
//
//		//ReduceDomain(ho_GrayImage, ho_Domain, &ho_GrayImage1);
//		//ReduceDomain(ho_ColorImage, ho_Domain, &ho_ColorImage1);
//		ho_GrayImage1 = Gray.Clone();
//		ho_ColorImage1 = Color.Clone();
//
//
//		//结果数据
//		spara2.sImage3dPro.ID = Image.ID;
//
//		if (hv_UnionObjectModel3D.Length() > 0)
//		{
//			CopyObjectModel3d(hv_UnionObjectModel3D, "all", &spara2.sImage3dPro.ObjectModel3D);
//		}
//
//		spara2.sImage3dPro.Gray = ho_GrayImage1.Clone();
//		spara2.sImage3dPro.Color = ho_ColorImage1.Clone();
//
//		spara2.sImage3dPro.X = ho_X1.Clone();
//		spara2.sImage3dPro.Y = ho_Y1.Clone();
//		spara2.sImage3dPro.Z = ho_Z1.Clone();
//		spara2.sImage3dPro.NX = ho_NX1.Clone();
//		spara2.sImage3dPro.NY = ho_NY1.Clone();
//		spara2.sImage3dPro.NZ = ho_NZ1.Clone();
//
//
//		//释放所有临时变量
//
//
//		spara2.bTJG = true;
//
//		CountSeconds(&time_end);
//		spara2.time = time_end - time_start;
//
//
//		sError.iCode = 0;
//		sError.strInfo = "";
//		return sError;
//	}
//	catch (HalconCpp::HException& ex)
//	{
//
//
//		//string str = ex.ToString();
//		//CLogTxt.WriteTxt(str);
//
//		CountSeconds(&time_end);
//		spara2.time = time_end - time_start;
//
//		sError.iCode = -1;
//		sError.strInfo = "预处理失败!";
//		return sError;
//
//	}
//}



s_Rtnf CUnloadPlateA::OnPreProcess(s_Image3dS sImage3dS, s_PreProcess3DSPara spara1, s_PreProcess3DSResultPara& spara2)
{

	s_Rtnf sError;

	sError.iCode = -1;
	sError.strInfo = "";

	spara2.bTJG = false;
	spara2.time = 0.0;
	spara2.sImage3dPro.Clear();


	HTuple time_start = 0.0;
	HTuple time_end = 0.0;
	CountSeconds(&time_start);


	// Local iconic variables 
	HObject ho_X, ho_Y, ho_Z, ho_NX;
	HObject ho_NY, ho_NZ, /*ho_Texture ,*/ ho_GrayImage;
	HObject ho_Region, ho_X1, ho_Y1, ho_Z1;
	HObject ho_Region1, ho_Region2, ho_Region3;
	HObject ho_NX1, ho_NY1, ho_NZ1;
	HObject ho_Domain, ho_GrayImage1;

	HObject ho_Image1, ho_Image2;
	HObject ho_Image3, ho_ImageH, ho_ImageS;
	HObject ho_ImageV, ho_ImageR, ho_ImageG;
	HObject ho_ImageB, ho_Multichannel;

	// Local control variables 

	HTuple hv_ImageWidth;
	HTuple hv_ImageHeight, hv_WindowWidth;
	HTuple hv_WindowHeight;
	HTuple hv_GenParamName, hv_GenParamValue;

	HTuple hv_ObjectModel3D;
	HTuple hv_Rows, hv_Columns;
	HTuple hv_NXV, hv_NYV, hv_NZV;
	HTuple hv_Intensity, hv_Pose;
	HTuple hv_ObjectModel3DConnected, hv_ObjectModel3DSelected;
	HTuple hv_ObjectModel3DSelected2, hv_UnionObjectModel3D;

	HTuple hv_SampledObjectModel3DS1;
	HTuple hv_ObjectModel3DConnectedS1, hv_GPVS1;
	HTuple hv_MaxS1, hv_ObjectModel3DSelectedS1a;
	HTuple hv_ObjectModel3DConnectedS1a, hv_GPVS1a;
	HTuple hv_MaxS1a, hv_ObjectModel3DSelectedS1b;
	HTuple hv_UnionObjectModel3DS1, hv_OM3DPlane;
	HTuple hv_ParFitting, hv_ValFitting;
	HTuple hv_ObjectModel3DOutID, hv_GenParamPosePlane;
	HTuple hv_PlaneDatum, hv_GenParamValueDistancePlane;
	HTuple hv_ObjectModel3DThresholdedS1c, hv_PoseOutA;
	HTuple hv_UnionObjectModel3DS12, hv_ObjectModel3DConnectedS1d;
	HTuple hv_ObjectModel3DSelectedS1d;


	HTuple hv_RV, hv_GV, hv_BV;
	HTuple hv_GvBoundingBox1S0;
	HTuple hv_lObjectModel3DS0, hv_ObjectModel3DSelectedS0;

	HTuple hv_NumPts;

	// Initialize local and output iconic variables 
	GenEmptyObj(&ho_X);
	GenEmptyObj(&ho_Y);
	GenEmptyObj(&ho_Z);
	GenEmptyObj(&ho_NX);
	GenEmptyObj(&ho_NY);
	GenEmptyObj(&ho_NZ);
	//HOperatorSet.GenEmptyObj(&ho_Texture);
	GenEmptyObj(&ho_GrayImage);
	GenEmptyObj(&ho_Region);
	GenEmptyObj(&ho_Region1);
	GenEmptyObj(&ho_Region2);
	GenEmptyObj(&ho_Region3);
	GenEmptyObj(&ho_X1);
	GenEmptyObj(&ho_Y1);
	GenEmptyObj(&ho_Z1);
	GenEmptyObj(&ho_NX1);
	GenEmptyObj(&ho_NY1);
	GenEmptyObj(&ho_NZ1);
	GenEmptyObj(&ho_Domain);
	GenEmptyObj(&ho_GrayImage1);

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

	try
	{

		s_Image3dS Image = sImage3dS;


		//判断输入
		if (!Image.Z.IsInitialized() || Image.Z.CountObj() == 0)
		{
			CountSeconds(&time_end);
			spara2.time = time_end - time_start;

			sError.iCode = 1;
			sError.strInfo = "图像为空!";
			return sError;
		}


		//处理算法




		//ROI区域图像 并给点云附加属性

		Threshold(Image.Z, &ho_Region1, spara1.fRoiZMin, spara1.fRoiZMax);
		Threshold(Image.X, &ho_Region2, spara1.fRoiXMin, spara1.fRoiXMax);
		Threshold(Image.Y, &ho_Region3, spara1.fRoiYMin, spara1.fRoiYMax);
		Intersection(ho_Region1, ho_Region2, &ho_Region);
		Intersection(ho_Region3, ho_Region, &ho_Region);


		ReduceDomain(Image.X, ho_Region, &ho_X);
		ReduceDomain(Image.Y, ho_Region, &ho_Y);
		ReduceDomain(Image.Z, ho_Region, &ho_Z);
		ReduceDomain(Image.NX, ho_Region, &ho_NX);
		ReduceDomain(Image.NY, ho_Region, &ho_NY);
		ReduceDomain(Image.NZ, ho_Region, &ho_NZ);
		ReduceDomain(Image.Gray, ho_Region, &ho_GrayImage);

		////灰度自动调整
		//{
		//    HTuple hv_Min = new HTuple(), hv_Max = new HTuple(), hv_Range = new HTuple(), hv_Mean = new HTuple(), hv_Deviation = new HTuple();

		//    HObject ho_ROI_0, ho_ImageTextureReduced, ho_ImageScaled;
		//   GenEmptyObj(&ho_ROI_0);
		//   GenEmptyObj(&ho_ImageTextureReduced);
		//   GenEmptyObj(&ho_ImageScaled);


		//    hv_Min.Dispose(); hv_Max.Dispose(); hv_Range.Dispose();
		//   MinMaxGray(ho_GrayImage, ho_GrayImage, 0, &hv_Min, &hv_Max,
		//        &hv_Range);
		//    ho_ROI_0.Dispose();
		//   GenRectangle1(&ho_ROI_0, 565.043, 729.802, 1220.68, 1362.76);
		//    ho_ImageTextureReduced.Dispose();
		//   ReduceDomain(ho_GrayImage, ho_ROI_0, &ho_ImageTextureReduced
		//    );
		//    hv_Mean.Dispose(); hv_Deviation.Dispose();
		//   Intensity(ho_ROI_0, ho_ImageTextureReduced, &hv_Mean, &hv_Deviation);
		//    using (HDevDisposeHelper dh = new HDevDisposeHelper())
		//    {
		//        ho_ImageScaled.Dispose();
		//        new HVisionAdv().scale_image_range(ho_GrayImage, &ho_ImageScaled, (new HTuple(0)).TupleConcat(
		//            0), ((hv_Mean * 1.2)).TupleConcat(255));
		//    }
		//    ho_GrayImage.Dispose();
		//   CopyImage(ho_ImageScaled, &ho_GrayImage);

		//}


		XyzToObjectModel3d(ho_X, ho_Y, ho_Z, &hv_ObjectModel3D);
		GetObjectModel3dParams(hv_ObjectModel3D, "num_points", &hv_NumPts);
		GetRegionPoints(ho_Region, &hv_Rows, &hv_Columns);




		//法线属性
		try
		{

			GetGrayval(ho_NX, hv_Rows, hv_Columns, &hv_NXV);
			GetGrayval(ho_NY, hv_Rows, hv_Columns, &hv_NYV);
			GetGrayval(ho_NZ, hv_Rows, hv_Columns, &hv_NZV);


			SetObjectModel3dAttribMod(hv_ObjectModel3D, ((HTuple("point_normal_x").Append("point_normal_y")).Append("point_normal_z")),
				HTuple(), (hv_NXV.TupleConcat(hv_NYV)).TupleConcat(hv_NZV));

		}
		catch (...)
		{


		}


		//灰度属性
		try
		{

			GetGrayval(ho_GrayImage, hv_Rows, hv_Columns, &hv_Intensity);
			SetObjectModel3dAttribMod(hv_ObjectModel3D, "&intensity", "points", hv_Intensity);
		}
		catch (...)
		{


		}



		try
		{
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
		catch (...)
		{


		}





		//聚类分割  根据点数 直径
		if (true)
		{

			ConnectionObjectModel3d(hv_ObjectModel3D, "distance_3d", spara1.fThSeg0Dis,
				&hv_ObjectModel3DConnected);

			SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points",
				"and", spara1.iThSeg0NumPtsMin, spara1.iThSeg0NumPtsMax, &hv_ObjectModel3DSelected);

			SelectObjectModel3d(hv_ObjectModel3DSelected, "diameter_bounding_box",
				"and", spara1.fThSeg0DiameterMin, spara1.fThSeg0DiameterMax, &hv_ObjectModel3DSelected2);

			UnionObjectModel3d(hv_ObjectModel3DSelected2, "points_surface", &hv_UnionObjectModel3D);
		}




		GetObjectModel3dParams(hv_UnionObjectModel3D, "num_points", &hv_NumPts);

		//根据处理后点云区域 获得XYZ NXYZ Gray 

		ObjectModel3dToXyz(&ho_X1, &ho_Y1, &ho_Z1, hv_UnionObjectModel3D, "from_xyz_map", HTuple(), HTuple());

		GetDomain(ho_Z1, &ho_Domain);

		//HOperatorSet.ReduceDomain(ho_X, ho_Domain, &ho_X1);
		//HOperatorSet.ReduceDomain(ho_Y, ho_Domain, &ho_Y1);
		//HOperatorSet.ReduceDomain(ho_Z, ho_Domain, &ho_Z1);
		ReduceDomain(ho_NX, ho_Domain, &ho_NX1);
		ReduceDomain(ho_NY, ho_Domain, &ho_NY1);
		ReduceDomain(ho_NZ, ho_Domain, &ho_NZ1);

		ReduceDomain(ho_GrayImage, ho_Domain, &ho_GrayImage1);


		//结果数据

		spara2.sImage3dPro.ID = Image.ID;

		if (hv_UnionObjectModel3D.Length() > 0)
		{
			CopyObjectModel3d(hv_UnionObjectModel3D, "all", &spara2.sImage3dPro.ObjectModel3D);
		}

		spara2.sImage3dPro.Gray = ho_GrayImage1.Clone();
		spara2.sImage3dPro.X = ho_X1.Clone();
		spara2.sImage3dPro.Y = ho_Y1.Clone();
		spara2.sImage3dPro.Z = ho_Z1.Clone();
		spara2.sImage3dPro.NX = ho_NX1.Clone();
		spara2.sImage3dPro.NY = ho_NY1.Clone();
		spara2.sImage3dPro.NZ = ho_NZ1.Clone();


		//释放所有临时变量


		spara2.bTJG = true;

		CountSeconds(&time_end);
		spara2.time = time_end - time_start;


		sError.iCode = 0;
		sError.strInfo = "";
		return sError;
	}
	catch (HalconCpp::HException& ex)
	{


		//string str = ex.ToString();
		//CLogTxt.WriteTxt(str);

		CountSeconds(&time_end);
		spara2.time = time_end - time_start;

		sError.iCode = -1;
		sError.strInfo = "预处理失败!";
		return sError;

	}
}





s_Rtnf CUnloadPlateA::OnPreProcess(s_Lidar3d imageT, s_PreProcessLidar3dPara spara1, s_PreProcessLidar3dRts& spara2)
{

	s_Rtnf sError;

	sError.iCode = -1;
	sError.strInfo = "";

	spara2.bTJG = false;
	spara2.time = 0.0;
	spara2.sImage3dPro.Clear();


	HTuple time_start = 0.0;
	HTuple time_end = 0.0;
	CountSeconds(&time_start);


	// Local iconic variables 
	HObject ho_X, ho_Y, ho_Z, ho_NX;
	HObject ho_NY, ho_NZ, /*ho_Texture ,*/ ho_GrayImage;
	HObject ho_Region, ho_X1, ho_Y1, ho_Z1;
	HObject ho_Region1, ho_Region2, ho_Region3;
	HObject ho_NX1, ho_NY1, ho_NZ1;
	HObject ho_Domain, ho_GrayImage1, ho_ColorImage1;

	HObject ho_Image1, ho_Image2;
	HObject ho_Image3, ho_ImageH, ho_ImageS;
	HObject ho_ImageV, ho_ImageR, ho_ImageG;
	HObject ho_ImageB, ho_Multichannel;

	HObject X, Y, Z;
	HObject NX, NY, NZ;
	HObject Gray, Color;

	HObject ho_UsefulRegion1, ho_UsefulRegion2, ho_UsefulRegion;
	HObject ho_NewImageX, ho_NewImageY, ho_NewImageZ, ho_NewImageP;

	// Local control variables 

	HTuple hv_ImageWidth;
	HTuple hv_ImageHeight, hv_WindowWidth;
	HTuple hv_WindowHeight;
	HTuple hv_GenParamName, hv_GenParamValue;

	HTuple hv_ObjectModel3D;
	HTuple hv_Rows, hv_Columns;
	HTuple hv_NXV, hv_NYV, hv_NZV;
	HTuple hv_Intensity, hv_Pose;
	HTuple hv_ObjectModel3DConnected, hv_ObjectModel3DSelected;
	HTuple hv_ObjectModel3DSelected2, hv_UnionObjectModel3D;

	HTuple hv_SampledObjectModel3DS1;
	HTuple hv_ObjectModel3DConnectedS1, hv_GPVS1;
	HTuple hv_MaxS1, hv_ObjectModel3DSelectedS1a;
	HTuple hv_ObjectModel3DConnectedS1a, hv_GPVS1a;
	HTuple hv_MaxS1a, hv_ObjectModel3DSelectedS1b;
	HTuple hv_UnionObjectModel3DS1, hv_OM3DPlane;
	HTuple hv_ParFitting, hv_ValFitting;
	HTuple hv_ObjectModel3DOutID, hv_GenParamPosePlane;
	HTuple hv_PlaneDatum, hv_GenParamValueDistancePlane;
	HTuple hv_ObjectModel3DThresholdedS1c, hv_PoseOutA;
	HTuple hv_UnionObjectModel3DS12, hv_ObjectModel3DConnectedS1d;
	HTuple hv_ObjectModel3DSelectedS1d;


	HTuple hv_RV, hv_GV, hv_BV;
	HTuple hv_GvBoundingBox1S0;
	HTuple hv_lObjectModel3DS0, hv_ObjectModel3DSelectedS0;

	HTuple hv_NumPts;

	HTuple hv_ImageW, hv_ImageH;
	HTuple hv_MinX, hv_MaxX, hv_RangeX;
	HTuple	hv_MinY, hv_MaxY, hv_RangeY;

	HTuple hv_ImageWtt, hv_RangeXtt, hv_RangeYtt, hv_PerX, hv_PerY, hv_PerX_Old, hv_PerY_Old;
	HTuple hv_NewImageW, hv_NewImageH, hv_Cols, hv_GrayY, hv_GrayX, hv_GrayZ, hv_GrayP, hv_Xpos, hv_Ypos;

	HTuple hv_ProcessModel, hv_fProSetResolutionX, hv_fProSetResolutionY;
	HObject  ho_ImageX, ho_ImageY, ho_ImageZ;




	try
	{

		s_Lidar3d Image = imageT;


		//判断输入
		if (!Image.Z.IsInitialized() || Image.Z.CountObj() == 0)
		{
			CountSeconds(&time_end);
			spara2.time = time_end - time_start;

			sError.iCode = 1;
			sError.strInfo = "图像为空!";
			return sError;
		}




		//处理算法

		Gray = Image.Intensity.Clone();


		//ROI区域图像 并给点云附加属性
		Threshold(Image.Z, &ho_Region1, spara1.fRoiZMin, spara1.fRoiZMax);
		Threshold(Image.X, &ho_Region2, spara1.fRoiXMin, spara1.fRoiXMax);
		Threshold(Image.Y, &ho_Region3, spara1.fRoiYMin, spara1.fRoiYMax);
		Intersection(ho_Region1, ho_Region2, &ho_Region);
		Intersection(ho_Region3, ho_Region, &ho_Region);


		ReduceDomain(Image.X, ho_Region, &ho_X);
		ReduceDomain(Image.Y, ho_Region, &ho_Y);
		ReduceDomain(Image.Z, ho_Region, &ho_Z);

		ho_GrayImage = Gray.Clone();

		XyzToObjectModel3d(ho_X, ho_Y, ho_Z, &hv_ObjectModel3D);
		GetObjectModel3dParams(hv_ObjectModel3D, "num_points", &hv_NumPts);
		GetRegionPoints(ho_Region, &hv_Rows, &hv_Columns);


		//灰度属性
		try
		{
			GetGrayval(ho_GrayImage, hv_Rows, hv_Columns, &hv_Intensity);
			SetObjectModel3dAttribMod(hv_ObjectModel3D, "&intensity", "points", hv_Intensity);
		}
		catch (...)
		{


		}




		//////////////////////////////////////////////////////////////////////////////////////
		 //将图像的像素分辨率均匀化，重新生成新的map
	////ProcessModel:0:不处理  1 fast   2 nomal 3 fine 4 coustom

		hv_ProcessModel = spara1.ProcessModel;
		hv_fProSetResolutionX = spara1.fProSetResolutionX;
		hv_fProSetResolutionY = spara1.fProSetResolutionY;
		if (0 != (int(hv_ProcessModel > 0)))
		{
			ho_ImageX = ho_X;
			ho_ImageY = ho_Y;
			ho_ImageZ = ho_Z;

			GetImageSize(Image.Z, &hv_ImageW, &hv_ImageH);
			Decompose3(ho_ImageZ, &ho_ImageX, &ho_ImageY, &ho_ImageZ);


			Threshold(ho_ImageZ, &ho_UsefulRegion1, -999999, -0.01);
			Threshold(ho_ImageZ, &ho_UsefulRegion2, 0.01, 999999);
			Union2(ho_UsefulRegion1, ho_UsefulRegion2, &ho_UsefulRegion);

			MinMaxGray(ho_UsefulRegion, ho_ImageX, 0, &hv_MinX, &hv_MaxX, &hv_RangeX);
			MinMaxGray(ho_UsefulRegion, ho_ImageY, 0, &hv_MinY, &hv_MaxY, &hv_RangeY);
			hv_ImageWtt = hv_ImageW;
			hv_RangeXtt = hv_RangeX;
			hv_RangeYtt = hv_RangeY;
			hv_PerX = hv_RangeX / (hv_ImageW - 1);
			hv_PerY = hv_RangeY / (hv_ImageH - 1);

			hv_PerX_Old = hv_PerX;
			hv_PerY_Old = hv_PerY;

			if (0 != (int(hv_ProcessModel == 2)))
			{
				if (0 != (int(hv_PerX > hv_PerY)))
				{
					hv_PerY = hv_PerX;
				}
				else
				{
					hv_PerX = hv_PerY;
				}
			}

			if (0 != (int(hv_ProcessModel == 3)))
			{

				if (0 != (int(hv_PerX < hv_PerY)))
				{
					hv_PerY = hv_PerX;
				}
				else
				{
					hv_PerX = hv_PerY;
				}
			}

			if (0 != (int(hv_ProcessModel == 4)))
			{

				hv_PerX = hv_fProSetResolutionX;
				hv_PerY = hv_fProSetResolutionY;
			}


			hv_NewImageW = (hv_RangeXtt / hv_PerX) + 1;
			hv_NewImageH = (hv_RangeYtt / hv_PerY) + 1;
			GenImageConst(&ho_NewImageX, "real", hv_NewImageW, hv_NewImageH);
			GenImageConst(&ho_NewImageY, "real", hv_NewImageW, hv_NewImageH);
			GenImageConst(&ho_NewImageZ, "real", hv_NewImageW, hv_NewImageH);


			GetRegionPoints(ho_UsefulRegion, &hv_Rows, &hv_Cols);

			GetGrayval(ho_ImageX, hv_Rows, hv_Cols, &hv_GrayX);
			hv_Xpos = (hv_GrayX - hv_MinX) / hv_PerX;

			GetGrayval(ho_ImageY, hv_Rows, hv_Cols, &hv_GrayY);
			hv_Ypos = (hv_GrayY - hv_MinY) / hv_PerY;

			GetGrayval(ho_ImageZ, hv_Rows, hv_Cols, &hv_GrayZ);

			SetGrayval(ho_NewImageX, hv_Ypos, hv_Xpos, hv_GrayX);
			SetGrayval(ho_NewImageY, hv_Ypos, hv_Xpos, hv_GrayY);
			SetGrayval(ho_NewImageZ, hv_Ypos, hv_Xpos, hv_GrayZ);


			XyzToObjectModel3d(ho_NewImageX, ho_NewImageY, ho_NewImageZ, &hv_ObjectModel3D);


		}





		////////////////////////////////////////////////////////////////////////////////////////
		//聚类分割  根据点数 直径
		if (true)
		{

			ConnectionObjectModel3d(hv_ObjectModel3D, "distance_3d", spara1.fThSeg0Dis,
				&hv_ObjectModel3DConnected);

			SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points",
				"and", spara1.iThSeg0NumPtsMin, spara1.iThSeg0NumPtsMax, &hv_ObjectModel3DSelected);

			SelectObjectModel3d(hv_ObjectModel3DSelected, "diameter_bounding_box",
				"and", spara1.fThSeg0DiameterMin, spara1.fThSeg0DiameterMax, &hv_ObjectModel3DSelected2);

			UnionObjectModel3d(hv_ObjectModel3DSelected2, "points_surface", &hv_UnionObjectModel3D);
		}




		GetObjectModel3dParams(hv_UnionObjectModel3D, "num_points", &hv_NumPts);

		//根据处理后点云区域 获得XYZ NXYZ Gray 

		ObjectModel3dToXyz(&ho_X1, &ho_Y1, &ho_Z1, hv_UnionObjectModel3D, "from_xyz_map", HTuple(), HTuple());

		GetDomain(ho_Z1, &ho_Domain);

		//ReduceDomain(ho_NX, ho_Domain, &ho_NX1);
		//ReduceDomain(ho_NY, ho_Domain, &ho_NY1);
		//ReduceDomain(ho_NZ, ho_Domain, &ho_NZ1);
		ReduceDomain(ho_GrayImage, ho_Domain, &ho_GrayImage1);




		//结果数据
		spara2.sImage3dPro.ID = Image.ID;

		if (hv_UnionObjectModel3D.Length() > 0)
		{
			CopyObjectModel3d(hv_UnionObjectModel3D, "all", &spara2.sImage3dPro.ObjectModel3D);
		}

		spara2.sImage3dPro.Intensity = ho_GrayImage1.Clone();
		spara2.sImage3dPro.X = ho_X1.Clone();
		spara2.sImage3dPro.Y = ho_Y1.Clone();
		spara2.sImage3dPro.Z = ho_Z1.Clone();
		//spara2.sImage3dPro.NX = ho_NX1.Clone();
		//spara2.sImage3dPro.NY = ho_NY1.Clone();
		//spara2.sImage3dPro.NZ = ho_NZ1.Clone();


		//释放所有临时变量


		spara2.bTJG = true;

		CountSeconds(&time_end);
		spara2.time = time_end - time_start;


		sError.iCode = 0;
		sError.strInfo = "";
		return sError;
	}
	catch (HalconCpp::HException& ex)
	{


		//string str = ex.ToString();
		//CLogTxt.WriteTxt(str);

		CountSeconds(&time_end);
		spara2.time = time_end - time_start;

		sError.iCode = -1;
		sError.strInfo = "预处理失败!";
		return sError;

	}
}


s_Rtnf CUnloadPlateA::OnPreAD(s_Lidar3d Image3d, s_PreADPlateAPara spara1, s_PreADPlateARtsPara& spara2)
{

	HTuple time_start = 0.0;
	HTuple time_end = 0.0;
	CountSeconds(&time_start);

	s_Rtnf sError;
	sError.iCode = -1;
	sError.strInfo = "";


	//复位结果数据
	spara2.Reset();


	// Local iconic variables
	HObject  ho_X, ho_Y, ho_Z, ho_XYZ, ho_RegionDefectSpaceRts;
	HObject  ho_Xs, ho_Ys, ho_Zs, ho_Domain, ho_ConnectedRegions;
	HObject  ho_SelectedRegions, ho_Rectangle, ho_RegionDefectLbRts;
	HObject  ho_RegionDefectCloseRtsTb, ho_RegionDefectTbNearRts;
	HObject  ho_Region, ho_RegionFillUp, ho_RegionClosing, ho_RegionOpening;
	HObject  ho_Region1, ho_Region2, ho_RegionUnion, ho_RegionDefectOuterRts;
	HObject  ho_Xdb, ho_Ydb, ho_Zdb, ho_Rectangle1, ho_RegionTbsPlus;
	HObject  ho_Zs1, ho_RegionClosing1, ho_RegionSelcted, ho_RegionContours;
	HObject  ho_Contour, ho_Cross, ho_SmoothedContours, ho_CrossCenter;
	HObject  ho_RectTbsPlus, ho_ObjectSelected, ho_RectTbsPlusUnion;

	// Local control variables
	HTuple  hv_WindowHandle, hv_Om3DScene, hv_PoseOut1;
	HTuple  hv_fThNZAbsVal, hv_ObjectModel3DNormals, hv_PoseOut;
	HTuple  hv_ValNz, hv_ValNzAbs, hv_ObjectModel3DThresholded;
	HTuple  hv_ObjectModel3DP, hv_fThSegCbDis, hv_fThCbSelLMin;
	HTuple  hv_fThCbSelLMax, hv_fThCbSelWMin, hv_fThCbSelWMax;
	HTuple  hv_fThCbSelPtsMin, hv_fThCbSelPtsMax, hv_fThCbSelZMin;
	HTuple  hv_fThCbSelZMax, hv_ObjectModel3D, hv_Pose, hv_ObjectModel3DConnected;
	HTuple  hv_GenParamValue, hv_Max, hv_ObjectModel3DSelected;
	HTuple  hv_Om3dCb, hv_i, hv_Pose1, hv_Length1, hv_Length2;
	HTuple  hv_Length3, hv_Om3dCbUnion, hv_bWaitOp, hv_bDefectSpaceObj;
	HTuple  hv_fDefectSpaceThZMin, hv_fDefectSpaceThZMax, hv_fDefectSpaceThInnerLen;
	HTuple  hv_fDefectSpaceThInnerWidth, hv_fDefectSpaceThLen;
	HTuple  hv_fDefectSpaceThNumPts, hv_iDefectSpaceRtsListPtsSel;
	HTuple  hv_fDefectSpaceRtsListXSel, hv_fDefectSpaceRtsListYSel;
	HTuple  hv_fDefectSpaceRtsListZSel, hv_fDefectSpaceRtsListLSel;
	HTuple  hv_PoseBox, hv_Length11, hv_Length21, hv_Length31;
	HTuple  hv_ObjectModel3DBox, hv_ObjectModel3DBoxInner, hv_BoxOffSetZ;
	HTuple  hv_BoxHeight, hv_PoseBoxSpace, hv_ObjectModel3DBoxSpace;
	HTuple  hv_Om3Defect, hv_Om3dSpaceDefectlist, hv_iNum, hv_iDefect;
	HTuple  hv_lOm3D, hv_Diameter, hv_Center, hv_Row1, hv_Column1;
	HTuple  hv_Row2, hv_Column2, hv_Number, hv_bDefectLbObj;
	HTuple  hv_fDefectLbThZMin, hv_fDefectLbThZMax, hv_fDefectLbThick;
	HTuple  hv_fDefectLbThInnerLen, hv_fDefectLbThInnerWidth;
	HTuple  hv_fDefectLbThLen, hv_fDefectLbThWidth, hv_fDefectLbThNumPts;
	HTuple  hv_iDefectLbRtsListPtsSel, hv_fDefectLbRtsListHeightSel;
	HTuple  hv_fDefectLbRtsListLSel, hv_CornersX1, hv_CornersY1;
	HTuple  hv_CornersZ1, hv_OM3CrossList, hv_OM3Cross, hv_Zdm;
	HTuple  hv_Length1Lb, hv_Length2Lb, hv_Length3Lb, hv_PoseBoxL1;
	HTuple  hv_Om3dBoxL1, hv_Om3dBoxL2, hv_Om3dBoxW1, hv_Om3dBoxW2;
	HTuple  hv_Om3dBoxLbList, hv_lOm3dBox, hv_UnionObjectModel3D;
	HTuple  hv_Gv, hv_Length12, hv_Length22, hv_Length32, hv_fThSegTbDis;
	HTuple  hv_fThTbSelLMin, hv_fThTbSelLMax, hv_fThTbSelWMin;
	HTuple  hv_fThTbSelWMax, hv_fThTbSelPtsMin, hv_fThTbSelPtsMax;
	HTuple  hv_fThTbSelZMin, hv_fThTbSelZMax, hv_fJgTbCloseLenMin;
	HTuple  hv_fJgTbClosePtsMin, hv_Om3dTbClose;
	HTuple  hv_lOm3TbSub, hv_Row11, hv_Column11, hv_Row21, hv_Column21;
	HTuple  hv_CopiedObjectModel3D, hv_Om3dTbCloseUnion, hv_Sorted;
	HTuple  hv_Inverted, hv_Om3dTb, hv_Om3dTbUnion, hv_fThSafeDisTbNear;
	HTuple  hv_Om3dTb2, hv_Val, hv_Min, hv_Om3dTb2Union, hv_Om3dTbAdv;
	HTuple  hv_PoseOutA, hv_ObjectModel3DUnion, hv_Om3TbSort;
	HTuple  hv_fXList, hv_Indices, hv_indexNotSafe;
	HTuple  hv_lOm3D1, hv_lOm3D2, hv_lOm3DRmEar1, hv_lOm3DRmEar2;
	HTuple  hv_Area, hv_Row, hv_Column, hv_Uniq, hv_Om3TbSortRemoveNotSafe;
	HTuple  hv_fThSafeDisToCbOuter;
	HTuple  hv_Om3dTbPlus, hv_Row12, hv_Column12, hv_Row22;
	HTuple  hv_Column22, hv_Min1, hv_Max1, hv_Range, hv_Min2;
	HTuple  hv_Max2, hv_aa, hv_bb, hv_dis, hv_Om3dTbPlusUnion;
	HTuple  hv_RtsCenterPts, hv_RtsConner1Pts, hv_RtsConner2Pts;
	HTuple  hv_RtsConner3Pts, hv_RtsConner4Pts, hv_RtsEarDir;
	HTuple  hv_RtsDegs, hv_OM3CrossCenterList, hv_OM3CrossAlist;
	HTuple  hv_OM3CrossBlist, hv_NumberTb, hv_fSampleTb, hv_SampledObjectModel3D;
	HTuple  hv_ObjectModel3DOutID, hv_TempFitParam, hv_GenParamPosePlane;
	HTuple  hv_PlaneDatum, hv_Om3dTbBm, hv_GenParamValue1, hv_GenParamValue2;
	HTuple  hv_EarDir, hv_Rows, hv_Cols, hv_GrayX, hv_GrayY;
	HTuple  hv_TupleZero, hv_Greatereq, hv_GrayX1, hv_GrayY1;
	HTuple  hv_GrayX2, hv_GrayY2, hv_CenterRow, hv_CenterCol;
	HTuple  hv_RectPI, hv_hv_PointerOrder, hv_Deg, hv_pt1, hv_pt2;
	HTuple  hv_pt3, hv_pt4, hv_pt0, hv_pt0z, hv_pt1z, hv_pt2z;
	HTuple  hv_pt3z, hv_pt4z, hv_OM3CrossCenter, hv_OM3CrossA1;
	HTuple  hv_OM3CrossA2, hv_OM3CrossA3, hv_OM3CrossA4, hv_I;
	HTuple hv_RtsCenterRow, hv_RtsCenterCol;


	while (true)
	{
		try
		{

			ho_X = Image3d.X.Clone();
			ho_Y = Image3d.Y.Clone();
			ho_Z = Image3d.Z.Clone();


			//判断输入
			if (!ho_Z.IsInitialized() || ho_Z.CountObj() == 0 || !ho_X.IsInitialized() || ho_X.CountObj() == 0 || !ho_Y.IsInitialized() || ho_Y.CountObj() == 0)
			{
				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 1;
				sError.strInfo = "图像为空!";
				break;
			}


			//算法处理



	   //***************************************************************************
	   //**算法定位
	   //***************************************************************************

			CHVisionAdvX lCHVisionAdvXObj;

			XyzToObjectModel3d(ho_X, ho_Y, ho_Z, &hv_Om3DScene);
			Compose3(ho_X, ho_Y, ho_Z, &ho_XYZ);


			//step1  提取车板点云数据

			hv_fThNZAbsVal = /*0.90*/spara1.fThNZAbsVal;


			//法线范围
			SurfaceNormalsObjectModel3d(hv_Om3DScene, "mls", "mls_force_inwards", "true", &hv_ObjectModel3DNormals);

			GetObjectModel3dParams(hv_ObjectModel3DNormals, "point_normal_z", &hv_ValNz);
			hv_ValNzAbs = hv_ValNz.TupleAbs();
			SelectPointsObjectModel3d(hv_ObjectModel3DNormals, hv_ValNzAbs, hv_fThNZAbsVal,
				1, &hv_ObjectModel3DThresholded);
			CopyObjectModel3d(hv_ObjectModel3DThresholded, "all", &hv_ObjectModel3DP);



			hv_fThSegCbDis = spara1.fThSegCbDis;        // 80
			hv_fThCbSelLMin = spara1.fThCbSelLMin;      // 3000
			hv_fThCbSelLMax = spara1.fThCbSelLMax;      // 18000
			hv_fThCbSelWMin = spara1.fThCbSelWMin;      // 800
			hv_fThCbSelWMax = spara1.fThCbSelWMax;      // 5000
			hv_fThCbSelPtsMin = spara1.fThCbSelPtsMin;  // 2000
			hv_fThCbSelPtsMax = spara1.fThCbSelPtsMax;  // 180000
			hv_fThCbSelZMin = spara1.fThCbSelZMin;      // 4900
			hv_fThCbSelZMax = spara1.fThCbSelZMax;      // 5600



			//选取车板底面

			SelectPointsObjectModel3d(hv_ObjectModel3DP, "point_coord_z", hv_fThCbSelZMin,
				hv_fThCbSelZMax, &hv_ObjectModel3D);


			ConnectionObjectModel3d(hv_ObjectModel3D, "distance_3d", hv_fThSegCbDis, &hv_ObjectModel3DConnected);

			GetObjectModel3dParams(hv_ObjectModel3DConnected, "num_points", &hv_GenParamValue);
			TupleMax(hv_GenParamValue, &hv_Max);
			SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_fThCbSelPtsMin, hv_fThCbSelPtsMax, &hv_ObjectModel3DSelected);


			hv_Om3dCb = HTuple();
			{
				HTuple end_val67 = (hv_ObjectModel3DSelected.TupleLength()) - 1;
				HTuple step_val67 = 1;
				for (hv_i = 0; hv_i.Continue(end_val67, step_val67); hv_i += step_val67)
				{

					//外形尺寸和车板不符合的点云块
					SmallestBoundingBoxObjectModel3d(HTuple(hv_ObjectModel3DSelected[hv_i]), "axis_aligned",
						&hv_Pose1, &hv_Length1, &hv_Length2, &hv_Length3);
					if (0 != (HTuple(int(hv_Length1 < hv_fThCbSelLMin)).TupleOr(int(hv_Length1 > hv_fThCbSelLMax))))
					{
						continue;
					}

					if (0 != (HTuple(int(hv_Length2 < hv_fThCbSelWMin)).TupleAnd(int(hv_Length2 > hv_fThCbSelWMax))))
					{
						continue;
					}


					hv_Om3dCb = hv_Om3dCb.TupleConcat(HTuple(hv_ObjectModel3DSelected[hv_i]));

				}
			}
			UnionObjectModel3d(hv_Om3dCb, "points_surface", &hv_Om3dCbUnion);
			GetObjectModel3dParams(hv_Om3dCb, "num_points", &hv_GenParamValue);

			int iOm3dCbPts = hv_GenParamValue[0].I();
			if (iOm3dCbPts < 100)
			{
				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 2;
				sError.strInfo = "车板定位异常";
				break;
			}


			//******************************************************************************
			//step2  判断空间异物

			hv_bDefectSpaceObj = spara1.bDefectSpaceObj;        // 1
			hv_fDefectSpaceThZMin = spara1.fDefectSpaceThZMin;  // 900
			hv_fDefectSpaceThZMax = spara1.fDefectSpaceThZMax;  // 3000
			hv_fDefectSpaceThInnerLen = spara1.fDefectSpaceThInnerLen;   // 150
			hv_fDefectSpaceThInnerWidth = spara1.fDefectSpaceThInnerWidth; // 300
			hv_fDefectSpaceThLen = spara1.fDefectSpaceThLen;    // 500.00
			hv_fDefectSpaceThNumPts = spara1.fDefectSpaceThNumPts; // 10


			bool bExistDefectSpace = false;
			hv_iDefectSpaceRtsListPtsSel = HTuple();
			hv_fDefectSpaceRtsListXSel = HTuple();
			hv_fDefectSpaceRtsListYSel = HTuple();
			hv_fDefectSpaceRtsListZSel = HTuple();
			hv_fDefectSpaceRtsListLSel = HTuple();


			GenEmptyObj(&ho_RegionDefectSpaceRts);

			while (0 != hv_bDefectSpaceObj)
			{

				SmallestBoundingBoxObjectModel3d(hv_Om3dCbUnion, "oriented", &hv_PoseBox, &hv_Length11,
					&hv_Length21, &hv_Length31);
				GenBoxObjectModel3d(hv_PoseBox, hv_Length11, hv_Length21, hv_Length31, &hv_ObjectModel3DBox);


				GenBoxObjectModel3d(hv_PoseBox, hv_Length11 - (2 * hv_fDefectSpaceThInnerLen), hv_Length21 - (2 * hv_fDefectSpaceThInnerWidth),
					hv_Length31, &hv_ObjectModel3DBoxInner);


				hv_BoxOffSetZ = (hv_fDefectSpaceThZMin + hv_fDefectSpaceThZMax) / 2;
				hv_BoxHeight = hv_fDefectSpaceThZMax - hv_fDefectSpaceThZMin;

				CreatePose(HTuple(hv_PoseBox[0]), HTuple(hv_PoseBox[1]), HTuple(hv_PoseBox[2]) - hv_BoxOffSetZ,
					HTuple(hv_PoseBox[3]), HTuple(hv_PoseBox[4]), HTuple(hv_PoseBox[5]), "Rp+T",
					"gba", "point", &hv_PoseBoxSpace);
				GenBoxObjectModel3d(hv_PoseBoxSpace, hv_Length11 - (2 * hv_fDefectSpaceThInnerLen),
					hv_Length21 - (2 * hv_fDefectSpaceThInnerWidth), hv_BoxHeight, &hv_ObjectModel3DBoxSpace);



				//取空间区域内的点云并提取
				DistanceObjectModel3d(hv_ObjectModel3DNormals, hv_ObjectModel3DBoxSpace, HTuple(),
					0, "signed_distances", "true");
				GetObjectModel3dParams(hv_ObjectModel3DNormals, "&distance", &hv_GenParamValue);
				SelectPointsObjectModel3d(hv_ObjectModel3DNormals, "&distance", -999999, -10,
					&hv_Om3Defect);
				GetObjectModel3dParams(hv_Om3Defect, "num_points", &hv_GenParamValue);
				if (0 != (int(hv_GenParamValue == 0)))
				{
					break;
				}



				ConnectionObjectModel3d(hv_Om3Defect, "distance_3d", 300, &hv_ObjectModel3DConnected);
				SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_fDefectSpaceThNumPts,
					999999, &hv_ObjectModel3DSelected);
				GetObjectModel3dParams(hv_ObjectModel3DSelected, "num_points", &hv_GenParamValue);
				if (0 != (int(hv_GenParamValue == 0)))
				{
					break;
				}

				//找到有异常点云

				hv_Om3dSpaceDefectlist = hv_ObjectModel3DSelected;
				hv_iNum = hv_ObjectModel3DSelected.TupleLength();
				{
					HTuple end_val163 = hv_iNum - 1;
					HTuple step_val163 = 1;
					for (hv_iDefect = 0; hv_iDefect.Continue(end_val163, step_val163); hv_iDefect += step_val163)
					{

						hv_lOm3D = HTuple(hv_Om3dSpaceDefectlist[hv_iDefect]);
						MaxDiameterObjectModel3d(hv_lOm3D, &hv_Diameter);
						if (0 != (int(hv_Diameter < hv_fDefectSpaceThLen)))
						{
							continue;
						}

						GetObjectModel3dParams(hv_lOm3D, "num_points", &hv_GenParamValue);
						hv_iDefectSpaceRtsListPtsSel = hv_iDefectSpaceRtsListPtsSel.TupleConcat(HTuple(hv_GenParamValue[0]));
						hv_fDefectSpaceRtsListLSel = hv_fDefectSpaceRtsListLSel.TupleConcat(hv_Diameter);


						get_object_models_center(hv_lOm3D, &hv_Center);
						GetObjectModel3dParams(hv_lOm3D, "point_coord_z", &hv_GenParamValue);
						TupleMax(hv_GenParamValue, &hv_Max);
						hv_fDefectSpaceRtsListXSel = hv_fDefectSpaceRtsListXSel.TupleConcat(HTuple(hv_Center[0]));
						hv_fDefectSpaceRtsListYSel = hv_fDefectSpaceRtsListYSel.TupleConcat(HTuple(hv_Center[1]));
						hv_fDefectSpaceRtsListZSel = hv_fDefectSpaceRtsListZSel.TupleConcat(hv_Max);

						ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_lOm3D, "from_xyz_map", HTuple(),
							HTuple());
						GetDomain(ho_Zs, &ho_Domain);
						Connection(ho_Domain, &ho_ConnectedRegions);
						SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
						SmallestRectangle1(ho_SelectedRegions, &hv_Row1, &hv_Column1, &hv_Row2, &hv_Column2);
						GenRectangle1(&ho_Rectangle, hv_Row1, hv_Column1, hv_Row2, hv_Column2);


						ConcatObj(ho_RegionDefectSpaceRts, ho_Rectangle, &ho_RegionDefectSpaceRts);


					}
				}

				CountObj(ho_RegionDefectSpaceRts, &hv_Number);
				if (0 != (int(hv_Number > 0)))
				{
					bExistDefectSpace = 1;
				}

				break;
			}


			//******************************************************************************8
			//step3 判断四周围栏


			hv_bDefectLbObj = spara1.bDefectLbObj;                // 1
			hv_fDefectLbThZMin = spara1.fDefectLbThZMin;          // 300
			hv_fDefectLbThZMax = spara1.fDefectLbThZMax;          // 1600
			hv_fDefectLbThick = spara1.fDefectLbThick;            // 300
			hv_fDefectLbThInnerLen = spara1.fDefectLbThInnerLen;  // 200
			hv_fDefectLbThInnerWidth = spara1.fDefectLbThInnerWidth; // 100
			hv_fDefectLbThLen = spara1.fDefectLbThLen;            // 2000.00
			hv_fDefectLbThWidth = spara1.fDefectLbThWidth;        // 1000.00
			hv_fDefectLbThNumPts = spara1.fDefectLbThNumPts;      // 100


			bool  bExistDefectLb = 0;
			hv_iDefectLbRtsListPtsSel = HTuple();
			hv_fDefectLbRtsListHeightSel = HTuple();
			hv_fDefectLbRtsListLSel = HTuple();


			GenEmptyObj(&ho_RegionDefectLbRts);

			while (0 != hv_bDefectLbObj)
			{

				SmallestBoundingBoxObjectModel3d(hv_Om3dCbUnion, "oriented", &hv_PoseBox, &hv_Length11,
					&hv_Length21, &hv_Length31);

				get_box_corner_x(hv_Length11, hv_Length21, hv_Length31, hv_PoseBox, &hv_CornersX1,
					&hv_CornersY1, &hv_CornersZ1);


				hv_OM3CrossList = HTuple();
				for (hv_i = 0; hv_i <= 7; hv_i += 1)
				{
					gen_cross3d_x(20, 300, ((HTuple(hv_CornersX1[hv_i]).TupleConcat(HTuple(hv_CornersY1[hv_i]))).TupleConcat(HTuple(hv_CornersZ1[hv_i]))).TupleConcat(((HTuple(0).Append(0)).Append(0))),
						&hv_OM3Cross);
					hv_OM3CrossList = hv_OM3CrossList.TupleConcat(hv_OM3Cross);
				}




				//长围栏

				get_object_models_center(hv_Om3dCbUnion, &hv_Center);
				hv_Zdm = ((const HTuple&)hv_Center)[2];

				hv_Length1Lb = ((HTuple(hv_CornersX1[0]) - HTuple(hv_CornersX1[1])).TupleAbs()) - (2 * hv_fDefectLbThInnerLen);
				hv_Length2Lb = hv_fDefectLbThick;
				hv_Length3Lb = hv_fDefectLbThZMax - hv_fDefectLbThZMin;
				hv_PoseBoxL1.Clear();
				hv_PoseBoxL1.Append((HTuple(hv_CornersX1[0]) / 2) + (HTuple(hv_CornersX1[1]) / 2));
				hv_PoseBoxL1.Append((HTuple(hv_CornersY1[0]) / 2) + (HTuple(hv_CornersY1[1]) / 2));
				hv_PoseBoxL1.Append((hv_Zdm - (hv_fDefectLbThZMax / 2)) - (hv_fDefectLbThZMin / 2));
				hv_PoseBoxL1.Append(0);
				hv_PoseBoxL1.Append(0);
				hv_PoseBoxL1.Append(0);
				hv_PoseBoxL1.Append(0);
				GenBoxObjectModel3d(hv_PoseBoxL1, hv_Length1Lb, hv_Length2Lb, hv_Length3Lb, &hv_Om3dBoxL1);



				hv_Length1Lb = ((HTuple(hv_CornersX1[0 + 2]) - HTuple(hv_CornersX1[1 + 2])).TupleAbs()) - (2 * hv_fDefectLbThInnerLen);
				hv_Length2Lb = hv_fDefectLbThick;
				hv_Length3Lb = hv_fDefectLbThZMax - hv_fDefectLbThZMin;
				hv_PoseBoxL1.Clear();
				hv_PoseBoxL1.Append((HTuple(hv_CornersX1[0 + 2]) / 2) + (HTuple(hv_CornersX1[1 + 2]) / 2));
				hv_PoseBoxL1.Append((HTuple(hv_CornersY1[0 + 2]) / 2) + (HTuple(hv_CornersY1[1 + 2]) / 2));
				hv_PoseBoxL1.Append((hv_Zdm - (hv_fDefectLbThZMax / 2)) - (hv_fDefectLbThZMin / 2));
				hv_PoseBoxL1.Append(0);
				hv_PoseBoxL1.Append(0);
				hv_PoseBoxL1.Append(0);
				hv_PoseBoxL1.Append(0);
				GenBoxObjectModel3d(hv_PoseBoxL1, hv_Length1Lb, hv_Length2Lb, hv_Length3Lb, &hv_Om3dBoxL2);

				//短围栏
				hv_Length1Lb = hv_fDefectLbThick;
				hv_Length2Lb = ((HTuple(hv_CornersY1[1]) - HTuple(hv_CornersY1[2])).TupleAbs()) - (2 * hv_fDefectLbThInnerWidth);
				hv_Length3Lb = hv_fDefectLbThZMax - hv_fDefectLbThZMin;
				hv_PoseBoxL1.Clear();
				hv_PoseBoxL1.Append((HTuple(hv_CornersX1[1]) / 2) + (HTuple(hv_CornersX1[2]) / 2));
				hv_PoseBoxL1.Append((HTuple(hv_CornersY1[1]) / 2) + (HTuple(hv_CornersY1[2]) / 2));
				hv_PoseBoxL1.Append((hv_Zdm - (hv_fDefectLbThZMax / 2)) - (hv_fDefectLbThZMin / 2));
				hv_PoseBoxL1.Append(0);
				hv_PoseBoxL1.Append(0);
				hv_PoseBoxL1.Append(0);
				hv_PoseBoxL1.Append(0);
				GenBoxObjectModel3d(hv_PoseBoxL1, hv_Length1Lb, hv_Length2Lb, hv_Length3Lb, &hv_Om3dBoxW1);



				hv_Length1Lb = hv_fDefectLbThick;
				hv_Length2Lb = ((HTuple(hv_CornersY1[3]) - HTuple(hv_CornersY1[0])).TupleAbs()) - (2 * hv_fDefectLbThInnerWidth);
				hv_Length3Lb = hv_fDefectLbThZMax - hv_fDefectLbThZMin;
				hv_PoseBoxL1.Clear();
				hv_PoseBoxL1.Append((HTuple(hv_CornersX1[3]) / 2) + (HTuple(hv_CornersX1[0]) / 2));
				hv_PoseBoxL1.Append((HTuple(hv_CornersY1[3]) / 2) + (HTuple(hv_CornersY1[0]) / 2));
				hv_PoseBoxL1.Append((hv_Zdm - (hv_fDefectLbThZMax / 2)) - (hv_fDefectLbThZMin / 2));
				hv_PoseBoxL1.Append(0);
				hv_PoseBoxL1.Append(0);
				hv_PoseBoxL1.Append(0);
				hv_PoseBoxL1.Append(0);
				GenBoxObjectModel3d(hv_PoseBoxL1, hv_Length1Lb, hv_Length2Lb, hv_Length3Lb, &hv_Om3dBoxW2);



				hv_Om3dBoxLbList.Clear();
				hv_Om3dBoxLbList.Append(hv_Om3dBoxL1);
				hv_Om3dBoxLbList.Append(hv_Om3dBoxL2);
				hv_Om3dBoxLbList.Append(hv_Om3dBoxW1);
				hv_Om3dBoxLbList.Append(hv_Om3dBoxW2);
				{
					HTuple end_val299 = (hv_Om3dBoxLbList.TupleLength()) - 1;
					HTuple step_val299 = 1;
					for (hv_i = 0; hv_i.Continue(end_val299, step_val299); hv_i += step_val299)
					{

						hv_lOm3dBox = HTuple(hv_Om3dBoxLbList[hv_i]);

						//取空间区域内的点云并提取-栏板
						DistanceObjectModel3d(hv_ObjectModel3DNormals, hv_lOm3dBox, HTuple(), 0, "signed_distances",
							"true");
						GetObjectModel3dParams(hv_ObjectModel3DNormals, "&distance", &hv_GenParamValue);
						SelectPointsObjectModel3d(hv_ObjectModel3DNormals, "&distance", -999999, -10,
							&hv_Om3Defect);
						GetObjectModel3dParams(hv_Om3Defect, "num_points", &hv_GenParamValue);

						if (0 != (int(hv_GenParamValue > 0)))
						{



							ConnectionObjectModel3d(hv_Om3Defect, "distance_3d", 500, &hv_ObjectModel3DConnected);
							SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_fDefectLbThNumPts,
								999999, &hv_ObjectModel3DSelected);
							if (0 != (HTuple(int(hv_i == 0)).TupleOr(int(hv_i == 1))))
							{
								SelectObjectModel3d(hv_ObjectModel3DSelected, "diameter_axis_aligned_bounding_box",
									"and", hv_fDefectLbThLen / 2, 999999, &hv_ObjectModel3DSelected);
							}
							else
							{
								SelectObjectModel3d(hv_ObjectModel3DSelected, "diameter_axis_aligned_bounding_box",
									"and", hv_fDefectLbThWidth / 2, 999999, &hv_ObjectModel3DSelected);
							}

							UnionObjectModel3d(hv_ObjectModel3DSelected, "points_surface", &hv_UnionObjectModel3D);


							GetObjectModel3dParams(hv_UnionObjectModel3D, "num_points", &hv_GenParamValue);
							if (0 != (int(hv_GenParamValue == 0)))
							{
								continue;
							}


							hv_lOm3D = hv_UnionObjectModel3D;
							GetObjectModel3dParams(hv_UnionObjectModel3D, "bounding_box1", &hv_Gv);
							hv_Length12 = HTuple(hv_Gv[3]) - HTuple(hv_Gv[0]);
							hv_Length22 = HTuple(hv_Gv[4]) - HTuple(hv_Gv[1]);
							hv_Length32 = HTuple(hv_Gv[5]) - HTuple(hv_Gv[2]);
							if (0 != (HTuple(int(hv_i == 0)).TupleOr(int(hv_i == 1))))
							{
								if (0 != (int(hv_Length12 < hv_fDefectLbThLen)))
								{
									continue;
								}
							}
							else
							{
								if (0 != (int(hv_Length22 < hv_fDefectLbThWidth)))
								{
									continue;
								}
							}

							GetObjectModel3dParams(hv_lOm3D, "num_points", &hv_GenParamValue);
							hv_iDefectLbRtsListPtsSel = hv_iDefectLbRtsListPtsSel.TupleConcat(HTuple(hv_GenParamValue[0]));

							TupleMax(hv_Length12.TupleConcat(hv_Length22), &hv_Max);
							hv_fDefectLbRtsListLSel = hv_fDefectLbRtsListLSel.TupleConcat(hv_Max);
							hv_fDefectLbRtsListHeightSel = hv_fDefectLbRtsListHeightSel.TupleConcat(hv_Length32);

							ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_lOm3D, "from_xyz_map", HTuple(),
								HTuple());
							GetDomain(ho_Zs, &ho_Domain);

							SmallestRectangle1(ho_Domain, &hv_Row1, &hv_Column1, &hv_Row2, &hv_Column2);
							GenRectangle1(&ho_Rectangle, hv_Row1, hv_Column1, hv_Row2, hv_Column2);


							ConcatObj(ho_RegionDefectLbRts, ho_Rectangle, &ho_RegionDefectLbRts);
						}


					}
				}

				break;
			}

			Union1(ho_RegionDefectLbRts, &ho_RegionUnion);
			AreaCenter(ho_RegionUnion, &hv_Area, &hv_Row, &hv_Column);
			if (0 != (int(hv_Area > 0)))
			{
				bExistDefectLb = 1;
			}


			//**********************************************************************
			//step4选取铜跺 并对铜跺偏上层之间接触情况，净距离过小的情况做报警

			hv_fThSegTbDis = spara1.fThSegTbDis;          // 60
			hv_fThTbSelLMin = spara1.fThTbSelLMin;        // 800
			hv_fThTbSelLMax = spara1.fThTbSelLMax;        // 1500
			hv_fThTbSelWMin = spara1.fThTbSelWMin;        // 800
			hv_fThTbSelWMax = spara1.fThTbSelWMax;        // 1500
			hv_fThTbSelPtsMin = spara1.fThTbSelPtsMin;    // 3500
			hv_fThTbSelPtsMax = spara1.fThTbSelPtsMax;    // 17000
			hv_fThTbSelZMin = spara1.fThTbSelZMin;        // 4500
			hv_fThTbSelZMax = spara1.fThTbSelZMax;        // 5050

			hv_fJgTbCloseLenMin = spara1.fJgTbCloseLenMin;   // 2000
			hv_fJgTbClosePtsMin = spara1.fJgTbClosePtsMin;   // 4500


			bool bJgTbClose = 0;
			GenEmptyObj(&ho_RegionDefectCloseRtsTb);

			SelectPointsObjectModel3d(hv_ObjectModel3DP, "point_coord_z", hv_fThTbSelZMin,
				hv_fThTbSelZMax, &hv_ObjectModel3D);

			ConnectionObjectModel3d(hv_ObjectModel3D, "distance_3d", hv_fThSegTbDis, &hv_ObjectModel3DConnected);


			//简单判断一下是否存在铜跺接触的情形

			//判断X方向长度过长的情况


			hv_Om3dTbClose = HTuple();

			SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_fJgTbClosePtsMin,
				9999999, &hv_ObjectModel3DSelected);
			SelectObjectModel3d(hv_ObjectModel3DSelected, "diameter_axis_aligned_bounding_box",
				"and", hv_fJgTbCloseLenMin, 9999999, &hv_ObjectModel3DSelected);

			if (0 != (int((hv_ObjectModel3DSelected.TupleLength()) > 0)))
			{
				hv_Om3dTbClose = HTuple();
				if (0 != (int((hv_ObjectModel3DSelected.TupleLength()) > 0)))
				{
					bJgTbClose = 1;
					{
						HTuple end_val429 = (hv_ObjectModel3DSelected.TupleLength()) - 1;
						HTuple step_val429 = 1;
						for (hv_i = 0; hv_i.Continue(end_val429, step_val429); hv_i += step_val429)
						{
							hv_lOm3TbSub = HTuple(hv_ObjectModel3DSelected[hv_i]);

							ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_lOm3TbSub, "from_xyz_map",
								HTuple(), HTuple());
							GetDomain(ho_Xs, &ho_Domain);


							SmallestRectangle1(ho_Domain, &hv_Row11, &hv_Column11, &hv_Row21, &hv_Column21);
							GenRectangle1(&ho_Rectangle, hv_Row11, hv_Column11, hv_Row21, hv_Column21);
							ConcatObj(ho_RegionDefectCloseRtsTb, ho_Rectangle, &ho_RegionDefectCloseRtsTb
							);

							CopyObjectModel3d(hv_lOm3TbSub, "all", &hv_CopiedObjectModel3D);
							hv_Om3dTbClose = hv_Om3dTbClose.TupleConcat(hv_CopiedObjectModel3D);
						}
					}

					UnionObjectModel3d(hv_Om3dTbClose, "points_surface", &hv_Om3dTbCloseUnion);

				}

			}


			GetObjectModel3dParams(hv_ObjectModel3DConnected, "num_points", &hv_GenParamValue);
			TupleMax(hv_GenParamValue, &hv_Max);
			TupleSort(hv_GenParamValue, &hv_Sorted);
			TupleInverse(hv_Sorted, &hv_Inverted);
			SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_fThTbSelPtsMin,
				hv_fThTbSelPtsMax, &hv_ObjectModel3DSelected);

			//筛选平面（外形 平面度等 ）
			hv_Om3dTb = HTuple();
			{
				HTuple end_val464 = (hv_ObjectModel3DSelected.TupleLength()) - 1;
				HTuple step_val464 = 1;
				for (hv_i = 0; hv_i.Continue(end_val464, step_val464); hv_i += step_val464)
				{
					//外形尺寸和车板不符合的点云块
					SmallestBoundingBoxObjectModel3d(HTuple(hv_ObjectModel3DSelected[hv_i]), "axis_aligned",
						&hv_Pose1, &hv_Length1, &hv_Length2, &hv_Length3);
					if (0 != (HTuple(int(hv_Length1 < hv_fThTbSelLMin)).TupleOr(int(hv_Length1 > hv_fThTbSelLMax))))
					{
						continue;
					}

					if (0 != (HTuple(int(hv_Length2 < hv_fThTbSelWMin)).TupleAnd(int(hv_Length2 > hv_fThTbSelWMax))))
					{
						continue;
					}


					hv_Om3dTb = hv_Om3dTb.TupleConcat(HTuple(hv_ObjectModel3DSelected[hv_i]));

				}
			}
			UnionObjectModel3d(hv_Om3dTb, "points_surface", &hv_Om3dTbUnion);


			//再次判断和铜跺接触区的间距
			hv_fThSafeDisTbNear = /*350*/spara1.fThSafeDisTbNear;


			hv_Om3dTb2 = HTuple();
			GenEmptyObj(&ho_RegionDefectTbNearRts);
			if (bJgTbClose)
			{

				{
					HTuple end_val496 = (hv_Om3dTb.TupleLength()) - 1;
					HTuple step_val496 = 1;
					for (hv_i = 0; hv_i.Continue(end_val496, step_val496); hv_i += step_val496)
					{

						hv_lOm3TbSub = HTuple(hv_Om3dTb[hv_i]);

						DistanceObjectModel3d(hv_lOm3TbSub, hv_Om3dTbCloseUnion, HTuple(), 0, HTuple(),
							HTuple());
						GetObjectModel3dParams(hv_lOm3TbSub, "&distance", &hv_Val);
						TupleMin(hv_Val, &hv_Min);


						if (0 != (int(hv_Min > hv_fThSafeDisTbNear)))
						{
							hv_Om3dTb2 = hv_Om3dTb2.TupleConcat(hv_lOm3TbSub);
						}
						else
						{

							ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_lOm3TbSub, "from_xyz_map",
								HTuple(), HTuple());
							GetDomain(ho_Xs, &ho_Domain);
							SmallestRectangle1(ho_Domain, &hv_Row11, &hv_Column11, &hv_Row21, &hv_Column21);
							GenRectangle1(&ho_Rectangle, hv_Row11, hv_Column11, hv_Row21, hv_Column21);
							ConcatObj(ho_RegionDefectTbNearRts, ho_Rectangle, &ho_RegionDefectTbNearRts
							);

						}

					}
				}

			}
			else
			{
				hv_Om3dTb2 = hv_Om3dTb;
			}
			UnionObjectModel3d(hv_Om3dTb2, "points_surface", &hv_Om3dTb2Union);
			GetObjectModel3dParams(hv_Om3dTb2, "num_points", &hv_GenParamValue);



			//**********************************************************************************
			//选取最上面的铜板
			hv_Om3dTbAdv = HTuple();
			{
				HTuple end_val539 = (hv_Om3dTb2.TupleLength()) - 1;
				HTuple step_val539 = 1;
				for (hv_i = 0; hv_i.Continue(end_val539, step_val539); hv_i += step_val539)
				{

					hv_lOm3TbSub = HTuple(hv_Om3dTb2[hv_i]);

					//去除一些上下叠板干扰
					SurfaceNormalsObjectModel3d(hv_lOm3TbSub, "mls", "mls_force_inwards", "true",
						&hv_ObjectModel3DNormals);

					GetObjectModel3dParams(hv_ObjectModel3DNormals, "point_normal_z", &hv_ValNz);
					hv_ValNzAbs = hv_ValNz.TupleAbs();
					SelectPointsObjectModel3d(hv_ObjectModel3DNormals, hv_ValNzAbs, 0.95, 1, &hv_ObjectModel3DThresholded);


					ConnectionObjectModel3d(hv_ObjectModel3DThresholded, "distance_3d", 50, &hv_ObjectModel3DConnected);

					GetObjectModel3dParams(hv_ObjectModel3DConnected, "num_points", &hv_GenParamValue);
					TupleMax(hv_GenParamValue, &hv_Max);
					SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_Max, hv_Max,
						&hv_ObjectModel3DSelected);

					ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_ObjectModel3DSelected, "from_xyz_map",
						HTuple(), HTuple());

					GetDomain(ho_Zs, &ho_Region);
					FillUp(ho_Region, &ho_RegionFillUp);
					ClosingCircle(ho_RegionFillUp, &ho_RegionClosing, 5.5);
					FillUp(ho_RegionClosing, &ho_RegionFillUp);

					ReduceObjectModel3dByView(ho_RegionFillUp, hv_lOm3TbSub, "xyz_mapping", HTuple(),
						&hv_ObjectModel3DUnion);

					ConnectionObjectModel3d(hv_ObjectModel3DUnion, "distance_3d", 50, &hv_ObjectModel3DConnected);

					GetObjectModel3dParams(hv_ObjectModel3DConnected, "num_points", &hv_GenParamValue);
					TupleMax(hv_GenParamValue, &hv_Max);
					SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_Max, hv_Max,
						&hv_ObjectModel3DSelected);

					hv_Om3dTbAdv = hv_Om3dTbAdv.TupleConcat(hv_ObjectModel3DSelected);

				}
			}

			UnionObjectModel3d(hv_Om3dTbAdv, "points_surface", &hv_Om3dTbUnion);


			//**********************************************************************************
			//*铜跺排序
			hv_Om3TbSort = HTuple();
			hv_fXList = HTuple();
			hv_Number = hv_Om3dTbAdv.TupleLength();
			{
				HTuple end_val604 = hv_Number - 1;
				HTuple step_val604 = 1;
				for (hv_i = 0; hv_i.Continue(end_val604, step_val604); hv_i += step_val604)
				{
					get_object_models_center(HTuple(hv_Om3dTbAdv[hv_i]), &hv_Center);
					hv_fXList = hv_fXList.TupleConcat(HTuple(hv_Center[0]));
				}
			}
			TupleSortIndex(hv_fXList, &hv_Indices);

			{
				HTuple end_val610 = (hv_Indices.TupleLength()) - 1;
				HTuple step_val610 = 1;
				for (hv_i = 0; hv_i.Continue(end_val610, step_val610); hv_i += step_val610)
				{
					hv_lOm3D = HTuple(hv_Om3dTbAdv[HTuple(hv_Indices[hv_i])]);
					hv_Om3TbSort = hv_Om3TbSort.TupleConcat(hv_lOm3D);
				}
			}


			//**********************************************************************************
			//相互之间的间距再次判断

			hv_indexNotSafe = HTuple();
			bool bExitSafeDisTb = 0;


			hv_Number = hv_Om3TbSort.TupleLength();
			{
				HTuple end_val624 = hv_Number - 2;
				HTuple step_val624 = 1;
				for (hv_i = 0; hv_i.Continue(end_val624, step_val624); hv_i += step_val624)
				{

					hv_lOm3D1 = HTuple(hv_Om3TbSort[hv_i]);
					hv_lOm3D2 = HTuple(hv_Om3TbSort[hv_i + 1]);
					ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_lOm3D1, "from_xyz_map", HTuple(),
						HTuple());

					GetDomain(ho_Zs, &ho_Region);
					FillUp(ho_Region, &ho_RegionFillUp);
					ClosingCircle(ho_RegionFillUp, &ho_RegionClosing, 3.5);
					OpeningCircle(ho_RegionClosing, &ho_RegionOpening, 11);
					Connection(ho_RegionOpening, &ho_ConnectedRegions);
					SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
					FillUp(ho_SelectedRegions, &ho_Region1);

					ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_lOm3D2, "from_xyz_map", HTuple(),
						HTuple());
					GetDomain(ho_Zs, &ho_Region);
					FillUp(ho_Region, &ho_RegionFillUp);
					ClosingCircle(ho_RegionFillUp, &ho_RegionClosing, 3.5);
					OpeningCircle(ho_RegionClosing, &ho_RegionOpening, 11);
					Connection(ho_RegionOpening, &ho_ConnectedRegions);
					SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
					FillUp(ho_SelectedRegions, &ho_Region2);



					ReduceObjectModel3dByView(ho_Region1, hv_lOm3D1, "xyz_mapping", HTuple(), &hv_lOm3DRmEar1);
					ReduceObjectModel3dByView(ho_Region2, hv_lOm3D2, "xyz_mapping", HTuple(), &hv_lOm3DRmEar2);


					DistanceObjectModel3d(hv_lOm3DRmEar1, hv_lOm3DRmEar2, HTuple(), 0, HTuple(),
						HTuple());

					GetObjectModel3dParams(hv_lOm3DRmEar1, "&distance", &hv_Val);
					TupleMin(hv_Val, &hv_Min);
					if (0 != (int(hv_Min < hv_fThSafeDisTbNear)))
					{
						hv_indexNotSafe = (hv_indexNotSafe.TupleConcat(hv_i)).TupleConcat(hv_i + 1);

						SmallestRectangle1(ho_Region1, &hv_Row11, &hv_Column11, &hv_Row21, &hv_Column21);
						GenRectangle1(&ho_Rectangle, hv_Row11, hv_Column11, hv_Row21, hv_Column21);
						ConcatObj(ho_RegionDefectTbNearRts, ho_Rectangle, &ho_RegionDefectTbNearRts
						);

						SmallestRectangle1(ho_Region2, &hv_Row11, &hv_Column11, &hv_Row21, &hv_Column21);
						GenRectangle1(&ho_Rectangle, hv_Row11, hv_Column11, hv_Row21, hv_Column21);
						ConcatObj(ho_RegionDefectTbNearRts, ho_Rectangle, &ho_RegionDefectTbNearRts
						);
					}

				}
			}

			//去除板子安全间距不够的板子

			Union1(ho_RegionDefectTbNearRts, &ho_RegionUnion);
			AreaCenter(ho_RegionUnion, &hv_Area, &hv_Row, &hv_Column);
			if (0 != (int(hv_Area > 0)))
			{
				bExitSafeDisTb = 1;
			}

			TupleSort(hv_indexNotSafe, &hv_Sorted);
			TupleUniq(hv_Sorted, &hv_Uniq);
			TupleRemove(hv_Om3TbSort, hv_Uniq, &hv_Om3TbSortRemoveNotSafe);


			//**********************************************************************************
			//判断外边界过小的情况

			hv_fThSafeDisToCbOuter = /*400*/spara1.fThSafeDisToCbOuter;
			int bExitSafeDisToCbOuter = 0;
			GenEmptyObj(&ho_RegionDefectOuterRts);

			ObjectModel3dToXyz(&ho_Xdb, &ho_Ydb, &ho_Zdb, hv_Om3dCbUnion, "from_xyz_map", HTuple(),
				HTuple());



			hv_Om3dTbPlus = HTuple();

			hv_Number = hv_Om3TbSortRemoveNotSafe.TupleLength();
			{
				HTuple end_val716 = hv_Number - 1;
				HTuple step_val716 = 1;
				for (hv_i = 0; hv_i.Continue(end_val716, step_val716); hv_i += step_val716)
				{
					hv_lOm3D = HTuple(hv_Om3TbSortRemoveNotSafe[hv_i]);
					ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_lOm3D, "from_xyz_map", HTuple(),
						HTuple());

					SmallestRectangle1(ho_Zs, &hv_Row12, &hv_Column12, &hv_Row22, &hv_Column22);
					GenRectangle1(&ho_Rectangle1, hv_Row12 - 200, hv_Column12 - 100, hv_Row22 + 200, hv_Column22 + 100);
					MinMaxGray(ho_Rectangle1, ho_Ydb, 0, &hv_Min1, &hv_Max1, &hv_Range);

					GetObjectModel3dParams(hv_lOm3D, "point_coord_y", &hv_GenParamValue);
					TupleMin(hv_GenParamValue, &hv_Min2);
					TupleMax(hv_GenParamValue, &hv_Max2);

					hv_aa = (hv_Min2 - hv_Min1).TupleAbs();
					hv_bb = (hv_Max2 - hv_Max1).TupleAbs();
					hv_dis = hv_aa.TupleMin2(hv_bb);
					if (0 != (int(hv_dis < hv_fThSafeDisToCbOuter)))
					{
						SmallestRectangle1(ho_Zs, &hv_Row11, &hv_Column11, &hv_Row21, &hv_Column21);
						GenRectangle1(&ho_Rectangle, hv_Row11, hv_Column11, hv_Row21, hv_Column21);
						ConcatObj(ho_RegionDefectOuterRts, ho_Rectangle, &ho_RegionDefectOuterRts);

						continue;
					}


					hv_Om3dTbPlus = hv_Om3dTbPlus.TupleConcat(hv_lOm3D);



					Union1(ho_RegionDefectOuterRts, &ho_RegionUnion);
					AreaCenter(ho_RegionUnion, &hv_Area, &hv_Row, &hv_Column);
					if (0 != (int(hv_Area > 0)))
					{
						bExitSafeDisToCbOuter = 1;
					}

				}
			}


			UnionObjectModel3d(hv_Om3dTbPlus, "points_surface", &hv_Om3dTbPlusUnion);

			GetObjectModel3dParams(hv_Om3dTbPlusUnion, "num_points", &hv_GenParamValue);
			ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_Om3dTbPlusUnion, "from_xyz_map", HTuple(), HTuple());
			GetDomain(ho_Zs, &ho_Domain);



			hv_RtsCenterPts = HTuple();
			hv_RtsConner1Pts = HTuple();
			hv_RtsConner2Pts = HTuple();
			hv_RtsConner3Pts = HTuple();
			hv_RtsConner4Pts = HTuple();
			hv_RtsEarDir = HTuple();
			hv_RtsDegs = HTuple();

			hv_RtsCenterRow = HTuple();
			hv_RtsCenterCol = HTuple();

			hv_OM3CrossCenterList = HTuple();
			hv_OM3CrossAlist = HTuple();
			hv_OM3CrossBlist = HTuple();

			GenEmptyObj(&ho_RegionTbsPlus);
			hv_NumberTb = hv_Om3dTbPlus.TupleLength();

			if (hv_NumberTb[0].I() == 0)
			{
				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 3;
				sError.strInfo = "未定位到有效铜跺";
				break;
			}


			//*循环提取每个板的中心坐标
			//****************************************************************************
			{
				HTuple end_val801 = hv_NumberTb - 1;
				HTuple step_val801 = 1;
				for (hv_i = 0; hv_i.Continue(end_val801, step_val801); hv_i += step_val801)
				{

					hv_lOm3TbSub = HTuple(hv_Om3dTbPlus[hv_i]);
					ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_lOm3TbSub, "from_xyz_map", HTuple(),
						HTuple());

					hv_fSampleTb = spara1.fSampleTb;


					//下采样拟合平面
					SampleObjectModel3d(hv_lOm3TbSub, "fast", hv_fSampleTb, HTuple(), HTuple(), &hv_SampledObjectModel3D);
					ConnectionObjectModel3d(hv_SampledObjectModel3D, "distance_3d", hv_fSampleTb * 2.0,
						&hv_ObjectModel3DConnected);

					GetObjectModel3dParams(hv_ObjectModel3DConnected, "num_points", &hv_GenParamValue);
					TupleMax(hv_GenParamValue, &hv_Max);
					SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_Max, hv_Max,
						&hv_ObjectModel3DSelected);


					FitPrimitivesObjectModel3d(hv_ObjectModel3DSelected, (HTuple("primitive_type").Append("output_xyz_mapping")),
						(HTuple("plane").Append("true")), &hv_ObjectModel3DOutID);
					GetObjectModel3dParams(hv_ObjectModel3DOutID, "primitive_parameter", &hv_TempFitParam);

					GetObjectModel3dParams(hv_ObjectModel3DOutID, "primitive_pose", &hv_GenParamPosePlane);
					GenPlaneObjectModel3d(hv_GenParamPosePlane, (((HTuple(-1).Append(-1)).Append(1)).Append(1)) * 1000,
						(((HTuple(-1).Append(1)).Append(1)).Append(-1)) * 1000, &hv_PlaneDatum);


					//确定铜跺的方向--当前只考虑y方向朝向
					GetDomain(ho_Zs, &ho_Region);
					FillUp(ho_Region, &ho_RegionFillUp);
					ClosingCircle(ho_RegionFillUp, &ho_RegionClosing, 3.5);
					OpeningCircle(ho_RegionClosing, &ho_RegionOpening, 9.5);
					Connection(ho_RegionOpening, &ho_ConnectedRegions);
					SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
					FillUp(ho_SelectedRegions, &ho_RegionFillUp);
					ReduceObjectModel3dByView(ho_RegionFillUp, hv_lOm3TbSub, "xyz_mapping", HTuple(), &hv_Om3dTbBm);
					GetObjectModel3dParams(hv_Om3dTbBm, "center", &hv_GenParamValue1);
					GetObjectModel3dParams(hv_lOm3TbSub, "center", &hv_GenParamValue2);

					if (0 != (int(HTuple(hv_GenParamValue2[1]) > HTuple(hv_GenParamValue1[1]))))
					{
						hv_EarDir = 1;
					}
					else
					{
						hv_EarDir = 3;
					}

					hv_RtsEarDir = hv_RtsEarDir.TupleConcat(hv_EarDir);

					//中心定位--由于包含了一些形态处理 可能影响到边界 重点关注边界精度影响问题

					GetDomain(ho_Zs, &ho_Region);
					FillUp(ho_Region, &ho_RegionFillUp);
					ClosingCircle(ho_RegionFillUp, &ho_RegionClosing, 3.5);
					OpeningCircle(ho_RegionClosing, &ho_RegionOpening, 9.5);
					FillUp(ho_RegionOpening, &ho_RegionFillUp);

					Connection(ho_RegionFillUp, &ho_ConnectedRegions);
					SelectShape(ho_ConnectedRegions, &ho_SelectedRegions, "area", "and", 150, 999999);
					Union1(ho_SelectedRegions, &ho_RegionUnion);

					ReduceDomain(ho_Z, ho_RegionUnion, &ho_Zs1);

					ClosingCircle(ho_RegionUnion, &ho_RegionClosing1, 3.5);

					//边界拟合
					Connection(ho_Zs1, &ho_ConnectedRegions);
					SelectShapeStd(ho_ConnectedRegions, &ho_RegionSelcted, "max_area", 70);
					FillUp(ho_RegionSelcted, &ho_RegionFillUp);
					ConcatObj(ho_RegionTbsPlus, ho_RegionFillUp, &ho_RegionTbsPlus);

					//像素中心
					AreaCenter(ho_RegionFillUp, &hv_Area, &hv_Row, &hv_Column);

					hv_RtsCenterRow.Append(hv_Row);
					hv_RtsCenterCol.Append(hv_Column);

					GenContourRegionXld(ho_RegionFillUp, &ho_RegionContours, "center");
					CountObj(ho_RegionContours, &hv_Number);
					GetContourXld(ho_RegionContours, &hv_Rows, &hv_Cols);

					GetGrayval(ho_X, hv_Rows, hv_Cols, &hv_GrayX);
					GetGrayval(ho_Y, hv_Rows, hv_Cols, &hv_GrayY);

					TupleGenConst(hv_GrayX.TupleLength(), 0, &hv_TupleZero);
					TupleNotEqualElem(hv_GrayX, hv_TupleZero, &hv_Greatereq);
					TupleFind(hv_Greatereq, 1, &hv_Indices);
					TupleSelect(hv_GrayX, hv_Indices, &hv_GrayX1);
					TupleSelect(hv_GrayY, hv_Indices, &hv_GrayY1);
					TupleGenConst(hv_GrayY1.TupleLength(), 0, &hv_TupleZero);
					TupleNotEqualElem(hv_GrayY1, hv_TupleZero, &hv_Greatereq);
					TupleFind(hv_Greatereq, 1, &hv_Indices);
					TupleSelect(hv_GrayX1, hv_Indices, &hv_GrayX2);
					TupleSelect(hv_GrayY1, hv_Indices, &hv_GrayY2);
					GenContourPolygonXld(&ho_Contour, hv_GrayY2, hv_GrayX2);

					GenCrossContourXld(&ho_Cross, hv_GrayY2, hv_GrayX2, 21, 0);

					SmoothContoursXld(ho_Contour, &ho_SmoothedContours, 21);

					FitRectangle2ContourXld(ho_SmoothedContours, "tukey", -1, 0, 0, 3, 5, &hv_CenterRow,
						&hv_CenterCol, &hv_RectPI, &hv_Length1, &hv_Length2, &hv_hv_PointerOrder);


					//fit函数适用于图像坐标系 实际坐标系需取反
					TupleDeg(-hv_RectPI, &hv_Deg);

					//根据之前判断的朝向将角度调整一下
					if (0 != (int(hv_EarDir == 1)))
					{
						if (0 != (int(hv_Deg < 0)))
						{
							hv_Deg += 180;
						}
					}
					else
					{
						if (0 != (int(hv_Deg > 0)))
						{
							hv_Deg += 180;
						}
					}


					//再转成0到360范围
					if (0 != (int(hv_Deg < 0)))
					{
						hv_Deg += 360.0;
					}


					hv_RtsDegs = hv_RtsDegs.TupleConcat(hv_Deg);

					GenRectangle2ContourXld(&ho_Rectangle, hv_CenterRow, hv_CenterCol, hv_RectPI,
						hv_Length1, hv_Length2);



					calc_rectangle2_points_x(hv_CenterRow, hv_CenterCol, hv_RectPI, hv_Length1, hv_Length2,
						&hv_pt1, &hv_pt2, &hv_pt3, &hv_pt4);

					if (0 != (int((hv_pt1.TupleLength()) == 2)))
					{
						GenCrossContourXld(&ho_Cross, ((HTuple(hv_pt1[1]).TupleConcat(HTuple(hv_pt2[1]))).TupleConcat(HTuple(hv_pt3[1]))).TupleConcat(HTuple(hv_pt4[1])),
							((HTuple(hv_pt1[0]).TupleConcat(HTuple(hv_pt2[0]))).TupleConcat(HTuple(hv_pt3[0]))).TupleConcat(HTuple(hv_pt4[0])),
							300, 0);

					}
					else
					{
						continue;
					}



					hv_pt0 = HTuple();
					hv_pt0[0] = (((HTuple(hv_pt1[0]) + HTuple(hv_pt2[0])) + HTuple(hv_pt3[0])) + HTuple(hv_pt4[0])) / 4.0;
					hv_pt0[1] = (((HTuple(hv_pt1[1]) + HTuple(hv_pt2[1])) + HTuple(hv_pt3[1])) + HTuple(hv_pt4[1])) / 4.0;
					calc_plane_point_z_x(hv_TempFitParam, HTuple(hv_pt0[0]), HTuple(hv_pt0[1]), &hv_pt0z);
					hv_RtsCenterPts = ((hv_RtsCenterPts.TupleConcat(HTuple(hv_pt0[0]))).TupleConcat(HTuple(hv_pt0[1]))).TupleConcat(hv_pt0z);

					GenCrossContourXld(&ho_CrossCenter, HTuple(hv_pt0[1]), HTuple(hv_pt0[0]), 200, 0);


					calc_plane_point_z_x(hv_TempFitParam, HTuple(hv_pt1[0]), HTuple(hv_pt1[1]), &hv_pt1z);
					calc_plane_point_z_x(hv_TempFitParam, HTuple(hv_pt2[0]), HTuple(hv_pt2[1]), &hv_pt2z);
					calc_plane_point_z_x(hv_TempFitParam, HTuple(hv_pt3[0]), HTuple(hv_pt3[1]), &hv_pt3z);
					calc_plane_point_z_x(hv_TempFitParam, HTuple(hv_pt4[0]), HTuple(hv_pt4[1]), &hv_pt4z);

					hv_RtsConner1Pts = ((hv_RtsConner1Pts.TupleConcat(HTuple(hv_pt1[0]))).TupleConcat(HTuple(hv_pt1[1]))).TupleConcat(hv_pt1z);
					hv_RtsConner2Pts = ((hv_RtsConner2Pts.TupleConcat(HTuple(hv_pt2[0]))).TupleConcat(HTuple(hv_pt2[1]))).TupleConcat(hv_pt2z);
					hv_RtsConner3Pts = ((hv_RtsConner3Pts.TupleConcat(HTuple(hv_pt3[0]))).TupleConcat(HTuple(hv_pt3[1]))).TupleConcat(hv_pt3z);
					hv_RtsConner4Pts = ((hv_RtsConner4Pts.TupleConcat(HTuple(hv_pt4[0]))).TupleConcat(HTuple(hv_pt4[1]))).TupleConcat(hv_pt4z);


					gen_cross3d_x(20, 350, ((HTuple(hv_pt0[0]).TupleConcat(HTuple(hv_pt0[1]))).TupleConcat(hv_pt0z)).TupleConcat(((HTuple(0).Append(0)).Append(0))),
						&hv_OM3CrossCenter);
					gen_cross3d_x(20, 350, ((HTuple(hv_pt1[0]).TupleConcat(HTuple(hv_pt1[1]))).TupleConcat(hv_pt1z)).TupleConcat(((HTuple(0).Append(0)).Append(0))),
						&hv_OM3CrossA1);
					gen_cross3d_x(20, 350, ((HTuple(hv_pt2[0]).TupleConcat(HTuple(hv_pt2[1]))).TupleConcat(hv_pt2z)).TupleConcat(((HTuple(0).Append(0)).Append(0))),
						&hv_OM3CrossA2);
					gen_cross3d_x(20, 350, ((HTuple(hv_pt3[0]).TupleConcat(HTuple(hv_pt3[1]))).TupleConcat(hv_pt3z)).TupleConcat(((HTuple(0).Append(0)).Append(0))),
						&hv_OM3CrossA3);
					gen_cross3d_x(20, 350, ((HTuple(hv_pt4[0]).TupleConcat(HTuple(hv_pt4[1]))).TupleConcat(hv_pt4z)).TupleConcat(((HTuple(0).Append(0)).Append(0))),
						&hv_OM3CrossA4);

					hv_OM3CrossCenterList = hv_OM3CrossCenterList.TupleConcat(hv_OM3CrossCenter);
					hv_OM3CrossAlist = (((hv_OM3CrossAlist.TupleConcat(hv_OM3CrossA1)).TupleConcat(hv_OM3CrossA2)).TupleConcat(hv_OM3CrossA3)).TupleConcat(hv_OM3CrossA4);


				}
			}


			//生成定位矩形区域
			GenEmptyObj(&ho_RectTbsPlus);
			CountObj(ho_RegionTbsPlus, &hv_Number);
			{
				HTuple end_val1022 = hv_Number;
				HTuple step_val1022 = 1;
				for (hv_I = 1; hv_I.Continue(end_val1022, step_val1022); hv_I += step_val1022)
				{
					SelectObj(ho_RegionTbsPlus, &ho_ObjectSelected, hv_I);
					SmallestRectangle1(ho_ObjectSelected, &hv_Row11, &hv_Column11, &hv_Row21, &hv_Column21);
					GenRectangle1(&ho_Rectangle, hv_Row11, hv_Column11, hv_Row21, hv_Column21);
					ConcatObj(ho_RectTbsPlus, ho_Rectangle, &ho_RectTbsPlus);

				}
			}
			Union1(ho_RectTbsPlus, &ho_RectTbsPlusUnion);


			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//结果数据

			spara2.bExistDefectSpace = bExistDefectSpace;   //空间异物检测结果

			for (size_t i = 0; i < hv_iDefectSpaceRtsListPtsSel.Length(); i++)
			{
				spara2.iDefectSpaceRtsListPtsSel.push_back(hv_iDefectSpaceRtsListPtsSel[i].I());
				spara2.fDefectSpaceRtsListXSel.push_back(hv_fDefectSpaceRtsListXSel[i].D());
				spara2.fDefectSpaceRtsListYSel.push_back(hv_fDefectSpaceRtsListYSel[i].D());
				spara2.fDefectSpaceRtsListZSel.push_back(hv_fDefectSpaceRtsListZSel[i].D());
				spara2.fDefectSpaceRtsListLSel.push_back(hv_fDefectSpaceRtsListLSel[i].D());
			}

			if (ho_RegionDefectSpaceRts.IsInitialized() && ho_RegionDefectSpaceRts.CountObj() > 0) { spara2.RegionDefectSpaceRts = ho_RegionDefectSpaceRts.Clone(); };



			spara2.bExistDefectLb = bExistDefectLb;  //围栏检测结果
			for (size_t i = 0; i < hv_iDefectLbRtsListPtsSel.Length(); i++)
			{
				spara2.iDefectLbRtsListPtsSel.push_back(hv_iDefectLbRtsListPtsSel[i].I());
				spara2.fDefectLbRtsListHeightSel.push_back(hv_fDefectLbRtsListHeightSel[i].D());
				spara2.fDefectLbRtsListLSel.push_back(hv_fDefectLbRtsListLSel[i].D());
			}
			if (ho_RegionDefectLbRts.IsInitialized() && ho_RegionDefectLbRts.CountObj() > 0) { spara2.RegionDefectLbRts = ho_RegionDefectLbRts.Clone(); };




			spara2.bExitTbClose = bJgTbClose; //存在铜跺之间有接触
			if (ho_RegionDefectCloseRtsTb.IsInitialized() && ho_RegionDefectCloseRtsTb.CountObj() > 0) { spara2.RegionDefectCloseRtsTb = ho_RegionDefectCloseRtsTb.Clone(); };


			spara2.bExitSafeDisTb = bExitSafeDisTb;  //存在铜板安全间距异常
			if (ho_RegionDefectTbNearRts.IsInitialized() && ho_RegionDefectTbNearRts.CountObj() > 0) { spara2.RegionDefectTbNearRts = ho_RegionDefectTbNearRts.Clone(); };



			spara2.bExitSafeDisToCbOuter = bExitSafeDisToCbOuter;
			if (ho_RegionDefectOuterRts.IsInitialized() && ho_RegionDefectOuterRts.CountObj() > 0) { spara2.RegionDefectOuterRts = ho_RegionDefectOuterRts.Clone(); };




			//只要有一项不满足 综合判定就为NG
			spara2.bTJG = spara2.bExistDefectSpace == false && spara2.bExitTbClose == false && spara2.bExitSafeDisTb == false && spara2.bExistDefectLb == false && spara2.bExitSafeDisToCbOuter == false;




			if (ho_RegionTbsPlus.IsInitialized() && ho_RegionTbsPlus.CountObj() > 0) { spara2.RegionTbsPlus = ho_RegionTbsPlus.Clone(); };
			if (ho_RectTbsPlus.IsInitialized() && ho_RectTbsPlus.CountObj() > 0) { spara2.RectTbsPlus = ho_RectTbsPlus.Clone(); };



			int iNum = hv_RtsCenterPts.Length() / 3;
			for (size_t i = 0; i < iNum; i++)
			{
				s_TbUpRts As;

				As.fCenterRow = hv_RtsCenterRow[i].D(); //像素中心位置  只能看个大概位置 不代表实际精度
				As.fCenterCol = hv_RtsCenterCol[i].D();
				As.fCenterX = hv_RtsCenterPts[i * 3 + 0].D();
				As.fCenterY = hv_RtsCenterPts[i * 3 + 1].D();
				As.fCenterZ = hv_RtsCenterPts[i * 3 + 2].D();
				As.fCenterDEG = hv_RtsDegs[i].D();; //角度为雷达转换坐标系下的值 且本身由于点云稀疏等原因不会很准确



				As.fCornerRow[0] = 0;
				As.fCornerCol[0] = 0;
				As.fCornerX[0] = hv_RtsConner1Pts[i * 3 + 0].D();
				As.fCornerY[0] = hv_RtsConner1Pts[i * 3 + 1].D();
				As.fCornerZ[0] = hv_RtsConner1Pts[i * 3 + 2].D();


				As.fCornerRow[1] = 0;
				As.fCornerCol[1] = 0;
				As.fCornerX[1] = hv_RtsConner2Pts[i * 3 + 0].D();
				As.fCornerY[1] = hv_RtsConner2Pts[i * 3 + 1].D();
				As.fCornerZ[1] = hv_RtsConner2Pts[i * 3 + 2].D();

				As.fCornerRow[2] = 0;
				As.fCornerCol[2] = 0;
				As.fCornerX[2] = hv_RtsConner3Pts[i * 3 + 0].D();
				As.fCornerY[2] = hv_RtsConner3Pts[i * 3 + 1].D();
				As.fCornerZ[2] = hv_RtsConner3Pts[i * 3 + 2].D();


				As.fCornerRow[3] = 0;
				As.fCornerCol[3] = 0;
				As.fCornerX[3] = hv_RtsConner4Pts[i * 3 + 0].D();
				As.fCornerY[3] = hv_RtsConner4Pts[i * 3 + 1].D();
				As.fCornerZ[3] = hv_RtsConner4Pts[i * 3 + 2].D();


				spara2.TbUpRtsList.push_back(As);
			}



			//// 3D
			//if (hv_ObjectModel3DAdv.Length() > 0)
			//{
			//	try { CopyObjectModel3d(hv_ObjectModel3DAdv, "all", &spara2.OM3ImageAdv); }
			//	catch (HalconCpp::HException& ex) { ; }

			//}

			sError.iCode = 0;
			sError.strInfo = "";

		}
		catch (HalconCpp::HException& ex)
		{
			std::string errProc = ex.ProcName(); //报错算子
			std::string errMsg = ex.ErrorMessage(); //错误提示

			sError.iCode = -1;
			sError.strInfo = "算子异常-" + errProc;
		}


		break;
	}


	//释放所有临时变量

	CountSeconds(&time_end);
	spara2.time = time_end - time_start;
	return sError;
}

bool CUnloadPlateA::CalcRtMat(std::vector <double> fListX1, std::vector <double> fListY1, std::vector <double> fListZ1,
	std::vector <double> fListX2, std::vector <double> fListY2, std::vector <double> fListZ2,
	s_PoseH& PoseT)
{

	if (fListX1.size() < 4)
	{
		return false;
	}

	if (fListX1.size() != fListX2.size())
	{
		return false;
	}


	try
	{

		HTuple hv_Px, hv_Py, hv_Pz, hv_Qx, hv_Qy, hv_Qz;

		hv_Px = HTuple();
		for (int i = 0; i < fListX1.size(); i++)
		{
			hv_Px.Append(fListX1[i]);
		}

		hv_Py = HTuple();
		for (int i = 0; i < fListY1.size(); i++)
		{
			hv_Py.Append(fListY1[i]);
		}

		hv_Pz = HTuple();
		for (int i = 0; i < fListZ1.size(); i++)
		{
			hv_Pz.Append(fListZ1[i]);
		}


		hv_Qx = HTuple();
		for (int i = 0; i < fListX2.size(); i++)
		{
			hv_Qx.Append(fListX2[i]);
		}

		hv_Qy = HTuple();
		for (int i = 0; i < fListY2.size(); i++)
		{
			hv_Qy.Append(fListY2[i]);
		}

		hv_Qz = HTuple();
		for (int i = 0; i < fListZ2.size(); i++)
		{
			hv_Qz.Append(fListZ2[i]);
		}


		HTuple hv_HomMat3D, hv_Pose;

		VectorToHomMat3d("similarity", hv_Px, hv_Py, hv_Pz, hv_Qx, hv_Qy, hv_Qz, &hv_HomMat3D);
		HomMat3dToPose(hv_HomMat3D, &hv_Pose);

		PoseT = s_PoseH(hv_Pose);


	}
	catch (HalconCpp::HException& ex)
	{
		//string str = ex.ToString();
		//CLogTxt.WriteTxt(str);

		return false;
	}

	return true;
}

//初定位计算
s_Rtnf CUnloadPlateA::CalcPreAlignRts(s_PreADPlateARtsPara sLidarRts, s_CalcPreAlignPara sCalcPreAlignPara, s_CalcPreAlignRtsPara& sCalcPreRts)
{

	s_Rtnf sError;
	sError.iCode = -1;
	sError.strInfo = "";

	sCalcPreRts.Reset();


	//判断输入
	if (sLidarRts.TbUpRtsList.size() == 0)
	{
		sError.iCode = 1;
		sError.strInfo = "无铜板数据!";
		return sError;
	}

	//if (sCalcPreAlignPara.sPoseLidar2Crane.IsAvailable() == false)
	//{
	//	sError.iCode = 1;
	//	sError.strInfo = "天车变换矩阵异常!";
	//	return sError;
	//}

	if (sCalcPreAlignPara.MatLidar2Crane[0] < 0.9)
	{
		sError.iCode = 1;
		sError.strInfo = "天车变换矩阵异常!";
		return sError;
	}



	//计算铜板在天车坐标系下的坐标

	double dX = sCalcPreAlignPara.fX - sCalcPreAlignPara.fSX; //相对偏移  注意：尽量让拍照位和基准拍照位偏移比较小 因为没有进行运动标定

	try
	{
		HTuple hv_HomMat3D;


		////采用了'rigid', 'similarity'转换方式  可以使用pose转mat3d  
		//PoseToHomMat3d(sCalcPreAlignPara.sPoseLidar2Crane.ToTuple(), &hv_HomMat3D);

		//采用affine方式 输入3D仿射矩阵
		hv_HomMat3D.Clear();
		for (size_t i = 0; i < 12; i++)
		{
			hv_HomMat3D[i] = sCalcPreAlignPara.MatLidar2Crane[i];
		}

		

		for (size_t i = 0; i < sLidarRts.TbUpRtsList.size(); i++)
		{

			s_TbUpRts As = sLidarRts.TbUpRtsList[i];

			s_PPts ppts;

			HTuple hX, hY, hZ;
			AffineTransPoint3d(hv_HomMat3D, As.fCenterX,  As.fCenterY, sCalcPreAlignPara.fLidar2Gnd - As.fCenterZ, &hX, &hY, &hZ);
			ppts.fX = hX[0].D() + dX + sCalcPreAlignPara.fOffsetTbX;
			ppts.fY = hY[0].D() + sCalcPreAlignPara.fOffsetTbY;
			ppts.fZ = hZ[0].D() + sCalcPreAlignPara.fOffsetTbZ;


			ppts.fDeg = As.fCenterDEG + sCalcPreAlignPara.fOffsetTbDeg;  //角度暂时取雷达坐标系下的角度  理论上存在误差 后续修正

			ppts.fRow = As.fCenterRow;
			ppts.fCol = As.fCenterCol;

			sCalcPreRts.fTbCenterCrane.push_back(ppts);
		}
	}
	catch (HalconCpp::HException& ex)
	{
		sCalcPreRts.Reset();

		sCalcPreRts.bTJG = false;
		//string str = ex.ToString();
		//CLogTxt.WriteTxt(str);

		sError.iCode = -1;
		sError.strInfo = "未知异常!";
		return sError;

	}







	//计算拍照位坐标
	for (size_t i = 0; i < sCalcPreRts.fTbCenterCrane.size(); i++)
	{

		s_PPts ppts;

		ppts.fX = sCalcPreRts.fTbCenterCrane[i].fX + sCalcPreAlignPara.fCameraOffsetX;
		ppts.fY = sCalcPreRts.fTbCenterCrane[i].fY + sCalcPreAlignPara.fCameraOffsetY;
		ppts.fZ = sCalcPreRts.fTbCenterCrane[i].fZ + sCalcPreAlignPara.fCameraOffsetZ;
		ppts.fDeg = sCalcPreRts.fTbCenterCrane[i].fDeg + sCalcPreAlignPara.fCameraOffsetDeg;


		//Z拍照位进行特殊处理    只要不超过最高值 让拍照位高一些
		if (ppts.fZ < sCalcPreAlignPara.fLimitCameraOffsetZMin)
		{
			ppts.fZ = sCalcPreAlignPara.fLimitCameraOffsetZMin + 50;
		}


		//天车拍照位角度需要固定到90度附近
		if (ppts.fDeg > 180)
		{
			ppts.fDeg = ppts.fDeg - 180;
		}


		sCalcPreRts.fCamaraPosCrane.push_back(ppts);

	}

	//最后对结果进行范围判断
	for (size_t i = 0; i < sCalcPreRts.fCamaraPosCrane.size(); i++)
	{
		s_PPts As = sCalcPreRts.fCamaraPosCrane[i];

		if (As.fX< sCalcPreAlignPara.fLimitCameraOffsetXMin || As.fX >sCalcPreAlignPara.fLimitCameraOffsetXMax)
		{
			sCalcPreRts.bTJG = false;

			sError.iCode = 2;
			sError.strInfo = "X计算坐标超限.";
			return sError;

		}


		if (As.fY< sCalcPreAlignPara.fLimitCameraOffsetYMin || As.fY >sCalcPreAlignPara.fLimitCameraOffsetYMax)
		{
			sCalcPreRts.bTJG = false;
			sError.iCode = 2;
			sError.strInfo = "Y计算坐标超限.";
			return sError;
		}


		if (As.fZ< sCalcPreAlignPara.fLimitCameraOffsetZMin || As.fZ >sCalcPreAlignPara.fLimitCameraOffsetZMax)
		{
			sCalcPreRts.bTJG = false;
			sError.iCode = 2;
			sError.strInfo = "Z计算坐标超限.";
			return sError;
		}


		if (As.fDeg< sCalcPreAlignPara.fLimitCameraOffsetDegMin || As.fDeg >sCalcPreAlignPara.fLimitCameraOffsetDegMax)
		{
			sCalcPreRts.bTJG = false;
			sError.iCode = 2;
			sError.strInfo = "Deg计算坐标超限.";
			return sError;
		}
	}


	sCalcPreRts.bTJG = true;

	sError.iCode = 0;
	sError.strInfo = "";
	return sError;

}

//精定位计算
s_Rtnf CUnloadPlateA::CalcAccurateAlignRts(s_AccurateADPlateARtsPara sImageRts, s_CalcAccurateAlignPara sCalcAlignPara, s_CalcAccurateAlignRtsPara& sCalcRts)
{
	s_Rtnf sError;
	sError.iCode = -1;
	sError.strInfo = "";

	sCalcRts.Reset();



	sCalcRts.TdW = sImageRts.TdW;   //铜跺的长宽高 不做补偿 直接复制
	sCalcRts.TdL = sImageRts.TdL;
	sCalcRts.TdH = sImageRts.TdH;

	sCalcRts.iTdEarDir = sImageRts.TdEarDir; //铜跺的朝向直接复制

	sCalcRts.dX = sImageRts.TdX - sCalcAlignPara.fSX; //对位偏差值
	sCalcRts.dY = sImageRts.TdY - sCalcAlignPara.fSY;
	sCalcRts.dZ = sImageRts.TdZ - sCalcAlignPara.fSZ;
	sCalcRts.dDeg = sImageRts.TdDeg - sCalcAlignPara.fSDeg;

	if (abs(sCalcRts.dX) < sCalcAlignPara.fAccurateX && abs(sCalcRts.dY) < sCalcAlignPara.fAccurateY && abs(sCalcRts.dZ) < sCalcAlignPara.fAccurateZ && abs(sCalcRts.dDeg) < sCalcAlignPara.fAccurateDeg)
	{
		sCalcRts.iStatus = 1;
	}
	else
	{
		sCalcRts.iStatus = 5;
	}


	//判断对位偏差数据是否超过限制
	if (abs(sCalcRts.dX) > sCalcAlignPara.fLimitRefMaxX || abs(sCalcRts.dY) > sCalcAlignPara.fLimitRefMaxY || abs(sCalcRts.dZ) > sCalcAlignPara.fLimitRefMaxZ || abs(sCalcRts.dDeg) > sCalcAlignPara.fLimitRefMaxDeg)
	{
		sCalcRts.iStatus = 2;

		sError.iCode = 1;
		sError.strInfo = "对位偏差数据是否超过限制.";

		return sError;
	}




	//下一个拍照位坐标 --如果时对位状态OK 需要忽略
	sCalcRts.fNextCameraX = sCalcAlignPara.fCameraX + sCalcRts.dX;
	sCalcRts.fNextCameraY = sCalcAlignPara.fCameraY + sCalcRts.dY; ;
	sCalcRts.fNextCameraZ = sCalcAlignPara.fCameraZ + sCalcRts.dZ; ;
	sCalcRts.fNextCameraDeg = sCalcAlignPara.fCameraDeg + sCalcRts.dDeg;

	//计算抓取位坐标 天车坐标系下
	sCalcRts.fGripX = sCalcAlignPara.fCameraX + sCalcRts.dX; //抓取位坐标点
	sCalcRts.fGripY = sCalcAlignPara.fCameraY + sCalcRts.dY;
	sCalcRts.fGripZ = sCalcAlignPara.fCameraZ + sImageRts.TdZ;
	sCalcRts.fGripDeg = sCalcAlignPara.fCameraDeg + sCalcRts.dDeg + 90;



	//加上对位固定补偿
	sCalcRts.fGripX = sCalcRts.fGripX + sCalcAlignPara.fOffsetTbX;
	sCalcRts.fGripY = sCalcRts.fGripY + sCalcAlignPara.fOffsetTbY;
	sCalcRts.fGripZ = sCalcRts.fGripZ + sCalcAlignPara.fOffsetTbZ;
	sCalcRts.fGripDeg = sCalcRts.fGripDeg + sCalcAlignPara.fOffsetTbDeg;


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//客户不希望直接给抓取坐标 希望给出耳朵在天车坐标系下的角度数据
	//视觉坐标系的xyz和天车坐标系是统一的  角度对应关系为DegCrane=DegCam+180
	
	if (sCalcRts.iTdEarDir==1)
	{
		//铜跺角度（相机坐标系下）90度 
		sCalcRts.fTdEarDeg = sCalcRts.fGripDeg - 90;
	}
	else
	{
		sCalcRts.fTdEarDeg = sCalcRts.fGripDeg +90;
	}

	//转天车坐标系角度
	sCalcRts.fTdEarDeg = sCalcRts.fTdEarDeg + 180;

	if (sCalcRts.fTdEarDeg >360)
	{
		sCalcRts.fTdEarDeg = sCalcRts.fTdEarDeg - 360;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////


	//对最终抓取位坐一下判断
	if (sCalcRts.fGripX< sCalcAlignPara.fLimitGripXMin || sCalcRts.fGripX >sCalcAlignPara.fLimitGripXMax)
	{
		sCalcRts.iStatus = 5;

		sError.iCode = 3;
		sError.strInfo = "X计算坐标超限.";
		return sError;

	}


	if (sCalcRts.fGripY< sCalcAlignPara.fLimitGripYMin || sCalcRts.fGripY >sCalcAlignPara.fLimitGripYMax)
	{
		sCalcRts.iStatus = 5;

		sError.iCode = 3;
		sError.strInfo = "Y计算坐标超限.";
		return sError;
	}


	if (sCalcRts.fGripZ< sCalcAlignPara.fLimitGripZMin || sCalcRts.fGripZ >sCalcAlignPara.fLimitGripZMax)
	{
		sCalcRts.iStatus = 5;
		sError.iCode = 3;
		sError.strInfo = "Z计算坐标超限.";
		return sError;
	}


	if (sCalcRts.fGripDeg< sCalcAlignPara.fLimitGripDegMin || sCalcRts.fGripDeg >sCalcAlignPara.fLimitGripDegMax)
	{
		sCalcRts.iStatus = 5;
		sError.iCode = 3;
		sError.strInfo = "Deg计算坐标超限.";
		return sError;
	}


	sError.iCode = 0;
	sError.strInfo = "";
	return sError;


}

//精定位
s_Rtnf CUnloadPlateA::OnAccurateAD(std::vector<s_Image3dS> ImgTList, s_AccurateADPlateAPara spara1, s_AccurateADPlateARtsPara& spara2)
{

	HTuple time_start = 0.0;
	HTuple time_end = 0.0;
	CountSeconds(&time_start);

	s_Rtnf sError;
	sError.iCode = -1;
	sError.strInfo = "";


	//复位结果数据
	spara2.Reset();

	// Local iconic variables
	HObject  ho_Image, ho_ImageGraya, ho_Za, ho_Xa;
	HObject  ho_Ya, ho_NXa, ho_NYa, ho_NZa, ho_GrayImagea, ho_ImageGrayb;
	HObject  ho_Zb, ho_Xb, ho_Yb, ho_NXb, ho_NYb, ho_NZb, ho_GrayImageb;
	HObject  ho_X, ho_Y, ho_Z, ho_Gray, ho_MultiChannelImage1;
	HObject  ho_NZ, ho_NX, ho_NY, ho_Domain, ho_MultiChannelImage2;
	HObject  ho_XA, ho_YA, ho_ZA, ho_XB, ho_YB, ho_ZB, ho_Region1;
	HObject  ho_Region2, ho_Region3, ho_Region4, ho_Region5, ho_Region6;
	HObject  ho_RegionUnion1, ho_RegionUnion2, ho_RegionUnion3;
	HObject  ho_RegionIntersection, ho_Xs, ho_Ys, ho_Zs, ho_RegionClosing;
	HObject  ho_RegionFillUp, ho_RegionOpening, ho_ConnectedRegions;
	HObject  ho_SelectedRegions, ho_RegionDilation, ho_RegionErosion;
	HObject  ho_RegionBmA, ho_RegionDifference, ho_RegionEarA;
	HObject  ho_Rectangle, ho_RegionUnion, ho_RegionBmARoiW;
	HObject  ho_RegionDilation1, ho_RegionBmARoiL, ho_RegionBmB;
	HObject  ho_RegionEarB, ho_RegionClosing1, ho_RegionBmBRoiW;
	HObject  ho_RegionBmBRoiL, ho_RegionBmARoiWAdv, ho_RegionBmBRoiWAdv;
	HObject  ho_Region, ho_RegionA, ho_RegionB, ho_RegionTDW;
	HObject  ho_RegionBmARoiLAdv, ho_RegionBmBRoiLAdv, ho_RegionTDL;
	HObject  ho_RegionEdgeW, ho_RegionContours, ho_Rectangle1;
	HObject  ho_Rectangle2, ho_XldSel, ho_Cross1, ho_Line1, ho_Cross2;
	HObject  ho_Line2, ho_XldBmBUpSideW, ho_XldBmAUpSideW, ho_RegionDefectGripSpaceRts1, ho_RegionDefectGripSpaceRts2;


	// Local control variables
	HTuple  hv_CamParam, hv_ImageWidth, hv_ImageHeight;
	HTuple  hv_WindowWidth, hv_WindowHeight, hv_WindowHandle;
	HTuple  hv_PathImagesFoldForPreprocess, hv_AllFiles, hv_ImageFiles;
	HTuple  hv_Index, hv_FullName, hv_Substrings, hv_ImageName;
	HTuple  hv_ImageNameNoSuffix, hv_PathFolder, hv_pathGraya;
	HTuple  hv_pathColora, hv_pathZa, hv_pathXa, hv_pathYa;
	HTuple  hv_pathNXa, hv_pathNYa, hv_pathNZa, hv_ObjectModel3Da;
	HTuple  hv_PoseOut, hv_pathGrayb, hv_pathColorb, hv_pathZb;
	HTuple  hv_pathXb, hv_pathYb, hv_pathNXb, hv_pathNYb, hv_pathNZb;
	HTuple  hv_ObjectModel3Db, hv_HomMat3D2, hv_HomMat3D1, hv_ObjectModel3DaT;
	HTuple  hv_ObjectModel3DbT, hv_CollectedAffineObjects, hv_UnionObjectModel3D;
	HTuple  hv_Pose, hv_ObjectModel3D, hv_fRoiZMin, hv_fRoiZMax;
	HTuple  hv_fRoiXMin, hv_fRoiXMax, hv_fRoiYMin, hv_fRoiYMax;
	HTuple  hv_fThSeg0Dis, hv_iThSeg0NumPtsMin, hv_iThSeg0NumPtsMax;
	HTuple  hv_fThSeg0DiameterMin, hv_fThSeg0DiameterMax, hv_ObjectModel3DRoi;
	HTuple  hv_ObjectModel3DConnected, hv_ObjectModel3DSelected;
	HTuple  hv_ObjectModel3DSelected2, hv_Rows, hv_Columns;
	HTuple  hv_NXV, hv_NYV, hv_NZV, hv_Om3DSceneA, hv_Om3DSceneB;
	HTuple  hv_fThCbNZAbsValMin, hv_fThCbNZAbsValMax, hv_fThCbNXAbsValMin;
	HTuple  hv_fThCbNXAbsValMax, hv_fThCbNYAbsValMin, hv_fThCbNYAbsValMax;
	HTuple  hv_fThSegCbDis, hv_fThCbSelLMin, hv_fThCbSelLMax;
	HTuple  hv_fThCbSelWMin, hv_fThCbSelWMax, hv_fThCbSelPtsMin;
	HTuple  hv_fThCbSelPtsMax, hv_fThCbSelZMin, hv_fThCbSelZMax;
	HTuple  hv_ValNz, hv_ValNzAbs, hv_ObjectModel3DThresholded;
	HTuple  hv_PoseOut1, hv_GenParamValue, hv_Max, hv_Om3dList;
	HTuple  hv_i, hv_Pose1, hv_Length1, hv_Length2, hv_Length3;
	HTuple  hv_Om3dCbA, hv_Om3dCbB, hv_fThTbNZAbsVal, hv_fThSegTbDis;
	HTuple  hv_fThTbSelLMin, hv_fThTbSelLMax, hv_fThTbSelWMin;
	HTuple  hv_fThTbSelWMax, hv_fThTbSelPtsMin, hv_fThTbSelPtsMax;
	HTuple  hv_fThTbSelZMin, hv_fThTbSelZMax, hv_Om3DSceneANzGood;
	HTuple  hv_Sorted, hv_Inverted, hv_Om3dTbA, hv_Om3DSceneBNzGood;
	HTuple  hv_Om3dTbB, hv_fThSafeDisTdNear, hv_fDefectTdNearSelZRefCb;
	HTuple  hv_iDefectTdNearRoiWDilation, hv_iDefectTdNearRoiLErosion;
	HTuple  bExitSafeDisTd, hv_fDefectTdNearRtsDis1, hv_fDefectTdNearRtsDis2;
	HTuple  hv_ValCbZ1, hv_Deviation, hv_ObjectModel3DReduced;
	HTuple  hv_SampledObjectModel3D1, hv_SampledObjectModel3D2;
	HTuple  hv_Val, hv_Min, hv_ValCbZ2, hv_iBmRoiWDilationW;
	HTuple  hv_iBmRoiWErosionL, hv_iBmRoiLDilationL, hv_Width;
	HTuple  hv_Height, hv_Row1, hv_Column1, hv_Row2, hv_Column2;
	HTuple  hv_Om3dBmARoiL, hv_fOffsetZSelTbRelCb, hv_Om3dBmARoiW;
	HTuple  hv_SampledObjectModel3D, hv_ObjectModel3DOutID;
	HTuple  hv_GenParamPosePlane, hv_PlaneDatum, hv_PlaneFitParam;
	HTuple  hv_ValCbARoiZ, hv_Om3dBmARoiWAdv, hv_Om3dBmBRoiW;
	HTuple  hv_ValCbBRoiZ, hv_Om3dBmBRoiWAdv, hv_ValX, hv_ValY;
	HTuple  hv_Om3dBmARoiLAdv, hv_Om3dBmBRoiL, hv_Om3dBmBRoiLAdv;
	HTuple  hv_fEdgeWLenUse, hv_fGndSelEdgeWOffset, hv_fGndSelEdgeWLen;
	HTuple  hv_fGripUp, hv_fLimitDegMin, hv_fLimitDegMax, hv_fLimitTdWMin;
	HTuple  hv_fLimitTdWMax, hv_fLimitTdLMin, hv_fLimitTdLMax;
	HTuple  hv_fLimitTdHMin, hv_fLimitTdHMax, hv_RtsX, hv_RtsY;
	HTuple  hv_RtsZ, hv_RtsDeg, hv_RtsEarDir, hv_RtsCbZ, hv_RtsTdW;
	HTuple  hv_RtsTdL, hv_RtsTdH, hv_bExitLimitSize, hv_Row11;
	HTuple  hv_Column11, hv_Row21, hv_Column21, hv_Area, hv_Row;
	HTuple  hv_Column, hv_Row12, hv_Column12, hv_Row22, hv_Column22;
	HTuple  hv_fEdgeWX1, hv_fEdgeWX2, hv_Cols, hv_Col1, hv_Col2;
	HTuple  hv_Nr, hv_Nc, hv_Dist, hv_Length, hv_ATan, hv_Deg1;
	HTuple  hv_Deg2, hv_Om3dEarA, hv_GenParamValue1, hv_GenParamValue2;
	HTuple  hv_Om3dCbANear, hv_RadioLeft, hv_RadioRight, hv_left;
	HTuple  hv_right, hv_Selected, hv_Mean, hv_Om3dCbBNear;
	HTuple  hv_OM3Cross, hv_Message;
	HTuple   hv_fThTbLayerOuter;
	HTuple  hv_fTbLayerOuterDis1, hv_fTbLayerOuterDis2, hv_DistanceMin;
	HTuple  hv_DistanceMax, hv_fDefectGripSpaceRoiXOffeset;
	HTuple  hv_fDefectGripSpaceRoiXLen, hv_fDefectGripSpaceRoiYLen;
	HTuple  hv_fDefectGripSpaceRoiZOffesetCb, hv_fDefectGripSpaceRoiZLen;
	HTuple  hv_fDefectGripSpaceThLen, hv_fDefectGripSpaceThNumPts;
	HTuple   hv_iDefectGripSpaceRtsListPtsSel;
	HTuple  hv_fDefectGripSpaceRtsListLSel, hv_ValXEdgeA, hv_PoseBox;
	HTuple  hv_ObjectModel3DBox, hv_Om3dGripRoi, hv_Diameter;
	HTuple  hv_ValXEdgeB, hv_Number;

	while (true)
	{
		try
		{


			//判断输入

			if (ImgTList.size() != 2)
			{
				sError.iCode = 1;
				sError.strInfo = "图像数据异常!";
				break;
			}

			int iCam = 0;



			if (!ImgTList[iCam].X.IsInitialized() || ImgTList[iCam].X.CountObj() == 0 || !ImgTList[iCam].Y.IsInitialized() || ImgTList[iCam].Y.CountObj() == 0 || !ImgTList[iCam].Z.IsInitialized() || ImgTList[iCam].Z.CountObj() == 0)
			{
				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 1;
				sError.strInfo = "图像为空XYZ!";
				break;
			}

			if (!ImgTList[iCam].NX.IsInitialized() || ImgTList[iCam].NX.CountObj() == 0 || !ImgTList[iCam].NY.IsInitialized() || ImgTList[iCam].NY.CountObj() == 0 || !ImgTList[iCam].NZ.IsInitialized() || ImgTList[iCam].NZ.CountObj() == 0)
			{
				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 1;
				sError.strInfo = "图像为空NXXYNZ!";
				break;
			}



			if (ImgTList[iCam].ObjectModel3D.Length() == 0)
			{
				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 1;
				sError.strInfo = "图像为空3D!";
				break;
			}

			iCam = 1;
			if (!ImgTList[iCam].X.IsInitialized() || ImgTList[iCam].X.CountObj() == 0 || !ImgTList[iCam].Y.IsInitialized() || ImgTList[iCam].Y.CountObj() == 0 || !ImgTList[iCam].Z.IsInitialized() || ImgTList[iCam].Z.CountObj() == 0)
			{
				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 1;
				sError.strInfo = "图像为空XYZ!";
				break;
			}

			if (!ImgTList[iCam].NX.IsInitialized() || ImgTList[iCam].NX.CountObj() == 0 || !ImgTList[iCam].NY.IsInitialized() || ImgTList[iCam].NY.CountObj() == 0 || !ImgTList[iCam].NZ.IsInitialized() || ImgTList[iCam].NZ.CountObj() == 0)
			{
				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 1;
				sError.strInfo = "图像为空NXXYNZ!";
				break;
			}


			if (ImgTList[iCam].ObjectModel3D.Length() == 0)
			{
				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 1;
				sError.strInfo = "图像为空3D!";
				break;
			}



			CopyObjectModel3d(ImgTList[0].ObjectModel3D, "all", &hv_Om3DSceneA);
			CopyObjectModel3d(ImgTList[1].ObjectModel3D, "all", &hv_Om3DSceneB);

			ho_NXa = ImgTList[0].NX;
			ho_NYa = ImgTList[0].NY;
			ho_NZa = ImgTList[0].NZ;


			ho_NXb = ImgTList[1].NX;
			ho_NYb = ImgTList[1].NY;
			ho_NZb = ImgTList[1].NZ;

			//算法处理



	   //***************************************************************************
	   //**算法处理
	   //***************************************************************************

			CHVisionAdvX lCHVisionAdvXObj;

			//********************************************************************************
			//step0a 提取单视野的车板面
			//********************************************************************************




			hv_fThCbNZAbsValMin = /*0.96*/spara1.fThCbNZAbsValMin;
			hv_fThCbNZAbsValMax = /*1*/spara1.fThCbNZAbsValMax;

			hv_fThCbNXAbsValMin = /*0*/spara1.fThCbNXAbsValMin;
			hv_fThCbNXAbsValMax = /*0.35*/spara1.fThCbNXAbsValMax;
			hv_fThCbNYAbsValMin = /*0*/spara1.fThCbNYAbsValMin;
			hv_fThCbNYAbsValMax = /*0.35*/spara1.fThCbNYAbsValMax;

			hv_fThSegCbDis = /*20*/spara1.fThSegCbDis;
			hv_fThCbSelLMin = /*500*/spara1.fThCbSelLMin;
			hv_fThCbSelLMax = /*5000*/spara1.fThCbSelLMax;
			hv_fThCbSelWMin = /*500*/spara1.fThCbSelWMin;
			hv_fThCbSelWMax = /*5000*/spara1.fThCbSelWMax;
			hv_fThCbSelPtsMin = /*2000*/spara1.fThCbSelPtsMin;
			hv_fThCbSelPtsMax = /*1000000*/spara1.fThCbSelPtsMax;
			hv_fThCbSelZMin = /*-2500*/spara1.fThCbSelZMin;
			hv_fThCbSelZMax = /*-1800*/spara1.fThCbSelZMax;

			//11111111111111111111111111111111111111111111111111111111111111111111111

			Threshold(ho_NZb, &ho_Region1, hv_fThCbNZAbsValMin, hv_fThCbNZAbsValMax);
			Threshold(ho_NZb, &ho_Region2, -hv_fThCbNZAbsValMax, -hv_fThCbNZAbsValMin);
			Threshold(ho_NXb, &ho_Region3, hv_fThCbNXAbsValMin, hv_fThCbNXAbsValMax);
			Threshold(ho_NXb, &ho_Region4, -hv_fThCbNXAbsValMax, -hv_fThCbNXAbsValMin);
			Threshold(ho_NYb, &ho_Region5, hv_fThCbNYAbsValMin, hv_fThCbNYAbsValMax);
			Threshold(ho_NYb, &ho_Region6, -hv_fThCbNYAbsValMax, -hv_fThCbNYAbsValMin);

			Union2(ho_Region1, ho_Region2, &ho_RegionUnion1);
			Union2(ho_Region3, ho_Region4, &ho_RegionUnion2);
			Union2(ho_Region5, ho_Region6, &ho_RegionUnion3);
			Intersection(ho_RegionUnion1, ho_RegionUnion2, &ho_RegionIntersection);
			Intersection(ho_RegionIntersection, ho_RegionUnion3, &ho_RegionIntersection);

			ReduceObjectModel3dByView(ho_RegionIntersection, hv_Om3DSceneA, "xyz_mapping",
				HTuple(), &hv_ObjectModel3DThresholded);



			//选取车板底面
			SelectPointsObjectModel3d(hv_ObjectModel3DThresholded, "point_coord_z", hv_fThCbSelZMin, hv_fThCbSelZMax, &hv_ObjectModel3D);

			ConnectionObjectModel3d(hv_ObjectModel3D, "distance_3d", hv_fThSegCbDis, &hv_ObjectModel3DConnected);

			GetObjectModel3dParams(hv_ObjectModel3DConnected, "num_points", &hv_GenParamValue);
			TupleMax(hv_GenParamValue, &hv_Max);
			SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_fThCbSelPtsMin, hv_fThCbSelPtsMax, &hv_ObjectModel3DSelected);


			hv_Om3dList = HTuple();
			{
				HTuple end_val361 = (hv_ObjectModel3DSelected.TupleLength()) - 1;
				HTuple step_val361 = 1;
				for (hv_i = 0; hv_i.Continue(end_val361, step_val361); hv_i += step_val361)
				{

					//外形尺寸和车板不符合的点云块
					SmallestBoundingBoxObjectModel3d(HTuple(hv_ObjectModel3DSelected[hv_i]), "axis_aligned",
						&hv_Pose1, &hv_Length1, &hv_Length2, &hv_Length3);
					if (0 != (HTuple(int(hv_Length1 < hv_fThCbSelLMin)).TupleOr(int(hv_Length1 > hv_fThCbSelLMax))))
					{
						continue;
					}

					if (0 != (HTuple(int(hv_Length2 < hv_fThCbSelWMin)).TupleOr(int(hv_Length2 > hv_fThCbSelWMax))))
					{
						continue;
					}


					hv_Om3dList = hv_Om3dList.TupleConcat(HTuple(hv_ObjectModel3DSelected[hv_i]));

				}
			}

			UnionObjectModel3d(hv_Om3dList, "points_surface", &hv_Om3dCbA);
			GetObjectModel3dParams(hv_Om3dCbA, "num_points", &hv_GenParamValue);



			//22222222222222222222222222222222222222222222222222222222222222222222222


			Threshold(ho_NZb, &ho_Region1, hv_fThCbNZAbsValMin, hv_fThCbNZAbsValMax);
			Threshold(ho_NZb, &ho_Region2, -hv_fThCbNZAbsValMax, -hv_fThCbNZAbsValMin);
			Threshold(ho_NXb, &ho_Region3, hv_fThCbNXAbsValMin, hv_fThCbNXAbsValMax);
			Threshold(ho_NXb, &ho_Region4, -hv_fThCbNXAbsValMax, -hv_fThCbNXAbsValMin);
			Threshold(ho_NYb, &ho_Region5, hv_fThCbNYAbsValMin, hv_fThCbNYAbsValMax);
			Threshold(ho_NYb, &ho_Region6, -hv_fThCbNYAbsValMax, -hv_fThCbNYAbsValMin);

			Union2(ho_Region1, ho_Region2, &ho_RegionUnion1);
			Union2(ho_Region3, ho_Region4, &ho_RegionUnion2);
			Union2(ho_Region5, ho_Region6, &ho_RegionUnion3);
			Intersection(ho_RegionUnion1, ho_RegionUnion2, &ho_RegionIntersection);
			Intersection(ho_RegionIntersection, ho_RegionUnion3, &ho_RegionIntersection);

			ReduceObjectModel3dByView(ho_RegionIntersection, hv_Om3DSceneB, "xyz_mapping",
				HTuple(), &hv_ObjectModel3DThresholded);



			//选取车板底面
			SelectPointsObjectModel3d(hv_ObjectModel3DThresholded, "point_coord_z", hv_fThCbSelZMin,
				hv_fThCbSelZMax, &hv_ObjectModel3D);


			ConnectionObjectModel3d(hv_ObjectModel3D, "distance_3d", hv_fThSegCbDis, &hv_ObjectModel3DConnected);
			GetObjectModel3dParams(hv_ObjectModel3DConnected, "num_points", &hv_GenParamValue);
			TupleMax(hv_GenParamValue, &hv_Max);
			SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_fThCbSelPtsMin,
				hv_fThCbSelPtsMax, &hv_ObjectModel3DSelected);

			hv_Om3dList = HTuple();
			{
				HTuple end_val432 = (hv_ObjectModel3DSelected.TupleLength()) - 1;
				HTuple step_val432 = 1;
				for (hv_i = 0; hv_i.Continue(end_val432, step_val432); hv_i += step_val432)
				{

					//外形尺寸和车板不符合的点云块
					SmallestBoundingBoxObjectModel3d(HTuple(hv_ObjectModel3DSelected[hv_i]), "axis_aligned",
						&hv_Pose1, &hv_Length1, &hv_Length2, &hv_Length3);
					if (0 != (HTuple(int(hv_Length1 < hv_fThCbSelLMin)).TupleOr(int(hv_Length1 > hv_fThCbSelLMax))))
					{
						continue;
					}

					if (0 != (HTuple(int(hv_Length2 < hv_fThCbSelWMin)).TupleOr(int(hv_Length2 > hv_fThCbSelWMax))))
					{
						continue;
					}


					hv_Om3dList = hv_Om3dList.TupleConcat(HTuple(hv_ObjectModel3DSelected[hv_i]));

				}
			}

			UnionObjectModel3d(hv_Om3dList, "points_surface", &hv_Om3dCbB);



			//********************************************************************************
			//step1a 提取上层铜板
			//********************************************************************************

			hv_fThTbNZAbsVal = /*0.90*/spara1.fThTbNZAbsVal;

			hv_fThSegTbDis = /*10*/spara1.fThSegTbDis;
			hv_fThTbSelLMin = /*800*/spara1.fThTbSelLMin;
			hv_fThTbSelLMax = /*1500*/spara1.fThTbSelLMax;
			hv_fThTbSelWMin = /*800*/spara1.fThTbSelWMin;
			hv_fThTbSelWMax = /*1500*/spara1.fThTbSelWMax;
			hv_fThTbSelPtsMin = /*50000*/spara1.fThTbSelPtsMin;
			hv_fThTbSelPtsMax = /*1000000*/spara1.fThTbSelPtsMax;
			hv_fThTbSelZMin = /*-2000*/spara1.fThTbSelZMin;
			hv_fThTbSelZMax = /*-1200*/spara1.fThTbSelZMax;


			//11111111111111111111111111111111111111111111111111111111111111111111111

			GetObjectModel3dParams(hv_Om3DSceneA, "point_normal_z", &hv_ValNz);
			hv_ValNzAbs = hv_ValNz.TupleAbs();
			SelectPointsObjectModel3d(hv_Om3DSceneA, hv_ValNzAbs, hv_fThTbNZAbsVal, 1, &hv_ObjectModel3DThresholded);

			CopyObjectModel3d(hv_ObjectModel3DThresholded, "all", &hv_Om3DSceneANzGood);


			SelectPointsObjectModel3d(hv_ObjectModel3DThresholded, "point_coord_z", hv_fThTbSelZMin,
				hv_fThTbSelZMax, &hv_ObjectModel3D);

			ConnectionObjectModel3d(hv_ObjectModel3D, "distance_3d", hv_fThSegTbDis, &hv_ObjectModel3DConnected);

			GetObjectModel3dParams(hv_ObjectModel3DConnected, "num_points", &hv_GenParamValue);
			TupleMax(hv_GenParamValue, &hv_Max);
			TupleSort(hv_GenParamValue, &hv_Sorted);
			TupleInverse(hv_Sorted, &hv_Inverted);
			SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_fThTbSelPtsMin,
				hv_fThTbSelPtsMax, &hv_ObjectModel3DSelected);



			//筛选平面（外形 平面度等 ）
			hv_Om3dList = HTuple();
			{
				HTuple end_val506 = (hv_ObjectModel3DSelected.TupleLength()) - 1;
				HTuple step_val506 = 1;
				for (hv_i = 0; hv_i.Continue(end_val506, step_val506); hv_i += step_val506)
				{

					//外形尺寸和车板不符合的点云块
					SmallestBoundingBoxObjectModel3d(HTuple(hv_ObjectModel3DSelected[hv_i]), "axis_aligned",
						&hv_Pose1, &hv_Length1, &hv_Length2, &hv_Length3);
					if (0 != (HTuple(int(hv_Length1 < hv_fThTbSelLMin)).TupleOr(int(hv_Length1 > hv_fThTbSelLMax))))
					{
						continue;
					}

					if (0 != (HTuple(int(hv_Length2 < hv_fThTbSelWMin)).TupleOr(int(hv_Length2 > hv_fThTbSelWMax))))
					{
						continue;
					}


					hv_Om3dList = hv_Om3dList.TupleConcat(HTuple(hv_ObjectModel3DSelected[hv_i]));

				}
			}

			GetObjectModel3dParams(hv_Om3dList, "num_points", &hv_GenParamValue);
			TupleMax(hv_GenParamValue, &hv_Max);
			SelectObjectModel3d(hv_Om3dList, "num_points", "and", hv_Max, hv_Max, &hv_Om3dTbA);


			//22222222222222222222222222222222222222222222222222222222222222222222222

			GetObjectModel3dParams(hv_Om3DSceneB, "point_normal_z", &hv_ValNz);
			hv_ValNzAbs = hv_ValNz.TupleAbs();
			SelectPointsObjectModel3d(hv_Om3DSceneB, hv_ValNzAbs, hv_fThTbNZAbsVal, 1, &hv_ObjectModel3DThresholded);

			CopyObjectModel3d(hv_ObjectModel3DThresholded, "all", &hv_Om3DSceneBNzGood);


			SelectPointsObjectModel3d(hv_ObjectModel3DThresholded, "point_coord_z", hv_fThTbSelZMin,
				hv_fThTbSelZMax, &hv_ObjectModel3D);

			ConnectionObjectModel3d(hv_ObjectModel3D, "distance_3d", hv_fThSegTbDis, &hv_ObjectModel3DConnected);

			GetObjectModel3dParams(hv_ObjectModel3DConnected, "num_points", &hv_GenParamValue);
			TupleMax(hv_GenParamValue, &hv_Max);
			TupleSort(hv_GenParamValue, &hv_Sorted);
			TupleInverse(hv_Sorted, &hv_Inverted);
			SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_fThTbSelPtsMin,
				hv_fThTbSelPtsMax, &hv_ObjectModel3DSelected);

			//筛选平面（外形 平面度等 ）
			hv_Om3dList = HTuple();
			{
				HTuple end_val564 = (hv_ObjectModel3DSelected.TupleLength()) - 1;
				HTuple step_val564 = 1;
				for (hv_i = 0; hv_i.Continue(end_val564, step_val564); hv_i += step_val564)
				{
					//外形尺寸和车板不符合的点云块
					SmallestBoundingBoxObjectModel3d(HTuple(hv_ObjectModel3DSelected[hv_i]), "axis_aligned",
						&hv_Pose1, &hv_Length1, &hv_Length2, &hv_Length3);
					if (0 != (HTuple(int(hv_Length1 < hv_fThTbSelLMin)).TupleOr(int(hv_Length1 > hv_fThTbSelLMax))))
					{
						continue;
					}

					if (0 != (HTuple(int(hv_Length2 < hv_fThTbSelWMin)).TupleOr(int(hv_Length2 > hv_fThTbSelWMax))))
					{
						continue;
					}


					hv_Om3dList = hv_Om3dList.TupleConcat(HTuple(hv_ObjectModel3DSelected[hv_i]));

				}
			}


			GetObjectModel3dParams(hv_Om3dList, "num_points", &hv_GenParamValue);
			TupleMax(hv_GenParamValue, &hv_Max);
			SelectObjectModel3d(hv_Om3dList, "num_points", "and", hv_Max, hv_Max, &hv_Om3dTbB);


			//********************************************************************************
			//step1b   对铜跺的板面安全距离做一个初步的检测
			//********************************************************************************


			hv_fThSafeDisTdNear = /*350*/spara1.fThSafeDisTdNear;
			hv_fDefectTdNearSelZRefCb = /*100*/spara1.fDefectTdNearSelZRefCb;
			hv_iDefectTdNearRoiWDilation = /*201*/spara1.iDefectTdNearRoiWDilation;
			hv_iDefectTdNearRoiLErosion = /*181*/spara1.iDefectTdNearRoiLErosion;


			bool bExitSafeDisTd = false;
			hv_fDefectTdNearRtsDis1 = 999999;
			hv_fDefectTdNearRtsDis2 = 999999;



			//11111111111111111111111111111111111111111111111111111111111111111111111
			//相对车底部高度做一次去除
			ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_Om3dCbA, "from_xyz_map", HTuple(),
				HTuple());
			Intensity(ho_Zs, ho_Zs, &hv_ValCbZ1, &hv_Deviation);
			SelectPointsObjectModel3d(hv_Om3DSceneANzGood, "point_coord_z", hv_ValCbZ1 + hv_fDefectTdNearSelZRefCb,
				hv_ValCbZ1 + 1000, &hv_ObjectModel3D);
			ConnectionObjectModel3d(hv_ObjectModel3D, "distance_3d", hv_fThSegTbDis, &hv_ObjectModel3DConnected);
			SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_fThTbSelPtsMin,
				hv_fThTbSelPtsMax, &hv_ObjectModel3DSelected);

			//筛选所有的车板上表面
			hv_Om3dList = HTuple();
			{
				HTuple end_val623 = (hv_ObjectModel3DSelected.TupleLength()) - 1;
				HTuple step_val623 = 1;
				for (hv_i = 0; hv_i.Continue(end_val623, step_val623); hv_i += step_val623)
				{

					//外形尺寸和车板不符合的点云块
					SmallestBoundingBoxObjectModel3d(HTuple(hv_ObjectModel3DSelected[hv_i]), "axis_aligned",
						&hv_Pose1, &hv_Length1, &hv_Length2, &hv_Length3);
					if (0 != (HTuple(int(hv_Length1 < hv_fThTbSelLMin)).TupleOr(int(hv_Length1 > hv_fThTbSelLMax))))
					{
						continue;
					}

					if (0 != (HTuple(int(hv_Length2 < 200)).TupleOr(int(hv_Length2 > hv_fThTbSelWMax))))
					{
						continue;
					}

					hv_Om3dList = hv_Om3dList.TupleConcat(HTuple(hv_ObjectModel3DSelected[hv_i]));

				}
			}

			if (0 != (int((hv_Om3dList.TupleLength()) > 2)))
			{
				hv_fDefectTdNearRtsDis1 = 0;

				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 2;
				sError.strInfo = "铜跺数异常.";
				break;

			}



			else if (0 != (int((hv_Om3dList.TupleLength()) == 2)))
			{

				try
				{



					ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, HTuple(hv_Om3dList[0]), "from_xyz_map",
						HTuple(), HTuple());
					ClosingCircle(ho_Zs, &ho_RegionClosing, 3.5);
					FillUp(ho_RegionClosing, &ho_RegionFillUp);
					OpeningRectangle1(ho_RegionFillUp, &ho_RegionOpening, 121, 1);
					Connection(ho_RegionOpening, &ho_ConnectedRegions);
					SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
					FillUp(ho_SelectedRegions, &ho_Region1);
					DilationRectangle1(ho_Region1, &ho_RegionDilation, 1, hv_iDefectTdNearRoiWDilation);
					ErosionRectangle1(ho_RegionDilation, &ho_RegionErosion, hv_iDefectTdNearRoiLErosion,
						1);
					ClosingCircle(ho_RegionErosion, &ho_RegionClosing, hv_iDefectTdNearRoiLErosion);
					FillUp(ho_RegionClosing, &ho_RegionFillUp);


					ReduceObjectModel3dByView(ho_RegionFillUp, hv_Om3DSceneA, "xyz_mapping", HTuple(),
						&hv_ObjectModel3DReduced);

					SelectPointsObjectModel3d(hv_ObjectModel3DReduced, "point_coord_z", hv_ValCbZ1 + hv_fDefectTdNearSelZRefCb,
						hv_ValCbZ1 + 1000, &hv_ObjectModel3DThresholded);

					SampleObjectModel3d(hv_ObjectModel3DThresholded, "fast", 5, HTuple(), HTuple(),
						&hv_SampledObjectModel3D1);


					ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, HTuple(hv_Om3dList[1]), "from_xyz_map",
						HTuple(), HTuple());
					ClosingCircle(ho_Zs, &ho_RegionClosing, 3.5);
					FillUp(ho_RegionClosing, &ho_RegionFillUp);
					OpeningRectangle1(ho_RegionFillUp, &ho_RegionOpening, 121, 1);
					Connection(ho_RegionOpening, &ho_ConnectedRegions);
					SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
					FillUp(ho_SelectedRegions, &ho_Region1);
					DilationRectangle1(ho_Region1, &ho_RegionDilation, 1, hv_iDefectTdNearRoiWDilation);
					ErosionRectangle1(ho_RegionDilation, &ho_RegionErosion, hv_iDefectTdNearRoiLErosion,
						1);
					ClosingCircle(ho_RegionErosion, &ho_RegionClosing, hv_iDefectTdNearRoiLErosion);
					FillUp(ho_RegionClosing, &ho_RegionFillUp);


					ReduceObjectModel3dByView(ho_RegionFillUp, hv_Om3DSceneA, "xyz_mapping", HTuple(),
						&hv_ObjectModel3DReduced);

					SelectPointsObjectModel3d(hv_ObjectModel3DReduced, "point_coord_z", hv_ValCbZ1 + hv_fDefectTdNearSelZRefCb,
						hv_ValCbZ1 + 1000, &hv_ObjectModel3DThresholded);

					SampleObjectModel3d(hv_ObjectModel3DThresholded, "fast", 5, HTuple(), HTuple(),
						&hv_SampledObjectModel3D2);



					DistanceObjectModel3d(hv_SampledObjectModel3D1, hv_SampledObjectModel3D2, HTuple(),
						0, HTuple(), HTuple());

					GetObjectModel3dParams(hv_SampledObjectModel3D1, "&distance", &hv_Val);
					TupleMin(hv_Val, &hv_Min);
					hv_fDefectTdNearRtsDis1 = hv_Min;

				}
				catch (HException& HDevExpDefaultException)
				{

				}

			}


			//2222222222222222222222222222222222222222222222222222222222222222222222222222
			//相对车底部高度做一次去除
			ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_Om3dCbB, "from_xyz_map", HTuple(),
				HTuple());
			Intensity(ho_Zs, ho_Zs, &hv_ValCbZ2, &hv_Deviation);
			SelectPointsObjectModel3d(hv_Om3DSceneBNzGood, "point_coord_z", hv_ValCbZ2 + hv_fDefectTdNearSelZRefCb,
				hv_ValCbZ2 + 1000, &hv_ObjectModel3D);
			ConnectionObjectModel3d(hv_ObjectModel3D, "distance_3d", hv_fThSegTbDis, &hv_ObjectModel3DConnected);
			SelectObjectModel3d(hv_ObjectModel3DConnected, "num_points", "and", hv_fThTbSelPtsMin,
				hv_fThTbSelPtsMax, &hv_ObjectModel3DSelected);



			//筛选所有的车板上表面
			hv_Om3dList = HTuple();
			{
				HTuple end_val712 = (hv_ObjectModel3DSelected.TupleLength()) - 1;
				HTuple step_val712 = 1;
				for (hv_i = 0; hv_i.Continue(end_val712, step_val712); hv_i += step_val712)
				{

					//外形尺寸和车板不符合的点云块
					SmallestBoundingBoxObjectModel3d(HTuple(hv_ObjectModel3DSelected[hv_i]), "axis_aligned",
						&hv_Pose1, &hv_Length1, &hv_Length2, &hv_Length3);
					if (0 != (HTuple(int(hv_Length1 < hv_fThTbSelLMin)).TupleOr(int(hv_Length1 > hv_fThTbSelLMax))))
					{
						continue;
					}

					if (0 != (HTuple(int(hv_Length2 < 200)).TupleOr(int(hv_Length2 > hv_fThTbSelWMax))))
					{
						continue;
					}

					hv_Om3dList = hv_Om3dList.TupleConcat(HTuple(hv_ObjectModel3DSelected[hv_i]));

				}
			}

			if (0 != (int((hv_Om3dList.TupleLength()) > 2)))
			{
				hv_fDefectTdNearRtsDis2 = 0;

				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 2;
				sError.strInfo = "铜跺数异常.";
				break;

			}
			else if (0 != (int((hv_Om3dList.TupleLength()) == 2)))
			{

				try
				{

					ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, HTuple(hv_Om3dList[0]), "from_xyz_map",
						HTuple(), HTuple());
					ClosingCircle(ho_Zs, &ho_RegionClosing, 3.5);
					FillUp(ho_RegionClosing, &ho_RegionFillUp);
					OpeningRectangle1(ho_RegionFillUp, &ho_RegionOpening, 1, 121);
					Connection(ho_RegionOpening, &ho_ConnectedRegions);
					SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
					FillUp(ho_SelectedRegions, &ho_Region1);
					DilationRectangle1(ho_Region1, &ho_RegionDilation, 1, hv_iDefectTdNearRoiWDilation);
					ErosionRectangle1(ho_RegionDilation, &ho_RegionErosion, hv_iDefectTdNearRoiLErosion,
						1);
					ClosingCircle(ho_RegionErosion, &ho_RegionClosing, hv_iDefectTdNearRoiLErosion);
					FillUp(ho_RegionClosing, &ho_RegionFillUp);


					ReduceObjectModel3dByView(ho_RegionFillUp, hv_Om3DSceneB, "xyz_mapping", HTuple(),
						&hv_ObjectModel3DReduced);

					SelectPointsObjectModel3d(hv_ObjectModel3DReduced, "point_coord_z", hv_ValCbZ1 + hv_fDefectTdNearSelZRefCb,
						hv_ValCbZ1 + 1000, &hv_ObjectModel3DThresholded);

					SampleObjectModel3d(hv_ObjectModel3DThresholded, "fast", 5, HTuple(), HTuple(),
						&hv_SampledObjectModel3D1);


					ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, HTuple(hv_Om3dList[1]), "from_xyz_map",
						HTuple(), HTuple());
					ClosingCircle(ho_Zs, &ho_RegionClosing, 3.5);
					FillUp(ho_RegionClosing, &ho_RegionFillUp);
					OpeningRectangle1(ho_RegionFillUp, &ho_RegionOpening, 1, 121);
					Connection(ho_RegionOpening, &ho_ConnectedRegions);
					SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
					FillUp(ho_SelectedRegions, &ho_Region1);
					DilationRectangle1(ho_Region1, &ho_RegionDilation, 1, hv_iDefectTdNearRoiWDilation);
					ErosionRectangle1(ho_RegionDilation, &ho_RegionErosion, hv_iDefectTdNearRoiLErosion,
						1);
					ClosingCircle(ho_RegionErosion, &ho_RegionClosing, hv_iDefectTdNearRoiLErosion);
					FillUp(ho_RegionClosing, &ho_RegionFillUp);


					ReduceObjectModel3dByView(ho_RegionFillUp, hv_Om3DSceneB, "xyz_mapping", HTuple(),
						&hv_ObjectModel3DReduced);

					SelectPointsObjectModel3d(hv_ObjectModel3DReduced, "point_coord_z", hv_ValCbZ1 + hv_fDefectTdNearSelZRefCb,
						hv_ValCbZ1 + 1000, &hv_ObjectModel3DThresholded);

					SampleObjectModel3d(hv_ObjectModel3DThresholded, "fast", 5, HTuple(), HTuple(),
						&hv_SampledObjectModel3D2);



					DistanceObjectModel3d(hv_SampledObjectModel3D1, hv_SampledObjectModel3D2, HTuple(),
						0, HTuple(), HTuple());

					GetObjectModel3dParams(hv_SampledObjectModel3D1, "&distance", &hv_Val);
					TupleMin(hv_Val, &hv_Min);
					hv_fDefectTdNearRtsDis2 = hv_Min;



				}
				catch (HException& HDevExpDefaultException)
				{

				}

			}

			if (0 != (HTuple(int(hv_fDefectTdNearRtsDis1 < hv_fThSafeDisTdNear)).TupleOr(int(hv_fDefectTdNearRtsDis2 < hv_fThSafeDisTdNear))))
			{

				bExitSafeDisTd = 1;

				CountSeconds(&time_end);
				spara2.time = time_end - time_start;


				spara2.bExitSafeDisTd = bExitSafeDisTd;   //存在铜跺安全距离超限
				spara2.fDefectTdNearRtsDis1 = hv_fDefectTdNearRtsDis1[0].D();  //间距1
				spara2.fDefectTdNearRtsDis2 = hv_fDefectTdNearRtsDis2[0].D();  //间距2

				sError.iCode = 3;
				sError.strInfo = "铜跺安全间距异常. 间距mm[" + std::to_string((int)spara2.fDefectTdNearRtsDis1) + "," + std::to_string((int)spara2.fDefectTdNearRtsDis2) + "]";
				break;



			}





			//********************************************************************************
			//step2a 提取上层铜跺板面和耳部
			//********************************************************************************

			hv_iBmRoiWDilationW = /*301*/spara1.iBmRoiWDilationW;
			hv_iBmRoiWErosionL = /*51*/spara1.iBmRoiWErosionL;
			hv_iBmRoiLDilationL = /*101*/spara1.iBmRoiLDilationL;


			//11111111111111111111111111111111111111111111111111111111111111111111111


			ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_Om3dTbA, "from_xyz_map", HTuple(),
				HTuple());
			ClosingRectangle1(ho_Zs, &ho_RegionClosing, 3, 3);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);

			OpeningRectangle1(ho_RegionFillUp, &ho_RegionOpening, 3, 121);
			FillUp(ho_RegionOpening, &ho_RegionFillUp);
			Connection(ho_RegionFillUp, &ho_ConnectedRegions);
			SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
			ho_RegionBmA = ho_SelectedRegions;
			FillUp(ho_Zs, &ho_RegionFillUp);
			Difference(ho_RegionFillUp, ho_SelectedRegions, &ho_RegionDifference);
			ClosingCircle(ho_RegionDifference, &ho_RegionClosing, 3.5);
			OpeningCircle(ho_RegionClosing, &ho_RegionOpening, 5);
			FillUp(ho_RegionOpening, &ho_RegionFillUp);
			Connection(ho_RegionFillUp, &ho_ConnectedRegions);
			SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
			ho_RegionEarA = ho_SelectedRegions;




			GetImageSize(ho_Xs, &hv_Width, &hv_Height);
			SmallestRectangle1(ho_RegionEarA, &hv_Row1, &hv_Column1, &hv_Row2, &hv_Column2);
			GenRectangle1(&ho_Rectangle, hv_Row1, 0, hv_Row2, hv_Width);
			Intersection(ho_Rectangle, ho_RegionBmA, &ho_RegionIntersection);
			DilationRectangle1(ho_RegionIntersection, &ho_RegionDilation, 1, hv_iBmRoiWDilationW);
			Union2(ho_RegionDilation, ho_RegionBmA, &ho_RegionUnion);
			ErosionRectangle1(ho_RegionUnion, &ho_RegionErosion, hv_iBmRoiWErosionL, 1);
			ClosingRectangle1(ho_RegionErosion, &ho_RegionClosing, 3, 1);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);
			ho_RegionBmARoiW = ho_RegionFillUp;

			DilationRectangle1(ho_RegionBmA, &ho_RegionDilation, hv_iBmRoiLDilationL, 1);
			DilationRectangle1(ho_RegionEarA, &ho_RegionDilation1, 151, 21);
			Difference(ho_RegionDilation, ho_RegionDilation1, &ho_RegionDifference);
			OpeningRectangle1(ho_RegionDifference, &ho_RegionOpening, 1, 11);
			ClosingRectangle1(ho_RegionOpening, &ho_RegionClosing, 3, 3);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);
			ho_RegionBmARoiL = ho_RegionFillUp;




			//22222222222222222222222222222222222222222222222222222222222222222222222

			ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_Om3dTbB, "from_xyz_map", HTuple(),
				HTuple());
			ClosingRectangle1(ho_Zs, &ho_RegionClosing, 3, 3);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);

			OpeningRectangle1(ho_RegionFillUp, &ho_RegionOpening, 3, 121);
			FillUp(ho_RegionOpening, &ho_RegionFillUp);
			Connection(ho_RegionFillUp, &ho_ConnectedRegions);
			SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
			ho_RegionBmB = ho_SelectedRegions;
			FillUp(ho_Zs, &ho_RegionFillUp);
			Difference(ho_RegionFillUp, ho_SelectedRegions, &ho_RegionDifference);
			ClosingCircle(ho_RegionDifference, &ho_RegionClosing, 3.5);
			OpeningCircle(ho_RegionClosing, &ho_RegionOpening, 5);
			FillUp(ho_RegionOpening, &ho_RegionFillUp);
			Connection(ho_RegionFillUp, &ho_ConnectedRegions);
			SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
			ho_RegionEarB = ho_SelectedRegions;



			GetImageSize(ho_Xs, &hv_Width, &hv_Height);
			SmallestRectangle1(ho_RegionEarB, &hv_Row1, &hv_Column1, &hv_Row2, &hv_Column2);
			GenRectangle1(&ho_Rectangle, hv_Row1, 0, hv_Row2, hv_Width);
			Intersection(ho_Rectangle, ho_RegionBmB, &ho_RegionIntersection);
			DilationRectangle1(ho_RegionIntersection, &ho_RegionDilation, 1, 301);
			Union2(ho_RegionDilation, ho_RegionBmB, &ho_RegionUnion);
			ErosionRectangle1(ho_RegionUnion, &ho_RegionErosion, 51, 1);
			ClosingRectangle1(ho_RegionErosion, &ho_RegionClosing1, 3, 1);
			FillUp(ho_RegionClosing1, &ho_RegionFillUp);
			ho_RegionBmBRoiW = ho_RegionFillUp;


			DilationRectangle1(ho_RegionBmB, &ho_RegionDilation, hv_iBmRoiLDilationL, 1);
			DilationRectangle1(ho_RegionEarB, &ho_RegionDilation1, 151, 21);
			Difference(ho_RegionDilation, ho_RegionDilation1, &ho_RegionDifference);
			OpeningRectangle1(ho_RegionDifference, &ho_RegionOpening, 1, 11);
			ClosingRectangle1(ho_RegionOpening, &ho_RegionClosing, 3, 3);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);
			ho_RegionBmBRoiL = ho_RegionFillUp;





			//***************************************************************************
			//step3 板子宽度点云数据处理(夹爪抓取方向)
			//****************************************************************************
			hv_fOffsetZSelTbRelCb =/* 30*/spara1.fOffsetZSelTbRelCb;


			//处理相机1宽度方向的点云
			ReduceObjectModel3dByView(ho_RegionBmARoiW, hv_Om3DSceneA, "xyz_mapping", HTuple(),
				&hv_Om3dBmARoiW);


			SampleObjectModel3d(hv_Om3dCbA, "xyz_mapping", 20, HTuple(), HTuple(), &hv_SampledObjectModel3D);


			FitPrimitivesObjectModel3d(hv_SampledObjectModel3D, (HTuple("primitive_type").Append("fitting_algorithm")),
				(HTuple("plane").Append("least_squares_tukey")), &hv_ObjectModel3DOutID);
			GetObjectModel3dParams(hv_ObjectModel3DOutID, "primitive_pose", &hv_GenParamPosePlane);
			GenPlaneObjectModel3d(hv_GenParamPosePlane, (((HTuple(-1).Append(-1)).Append(1)).Append(1)) * 1500,
				(((HTuple(-1).Append(1)).Append(1)).Append(-1)) * 1500, &hv_PlaneDatum);


			GetObjectModel3dParams(hv_ObjectModel3DOutID, "primitive_parameter", &hv_PlaneFitParam);
			hv_ValCbARoiZ = ((const HTuple&)hv_PlaneFitParam)[3];



			SelectPointsObjectModel3d(hv_Om3dBmARoiW, "point_coord_z", hv_ValCbARoiZ + hv_fOffsetZSelTbRelCb,
				1000, &hv_ObjectModel3DThresholded);



			ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_ObjectModel3DThresholded, "from_xyz_map",
				HTuple(), HTuple());
			GetDomain(ho_Xs, &ho_Domain);
			ClosingRectangle1(ho_Domain, &ho_RegionClosing, 1, 5);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);
			OpeningCircle(ho_RegionFillUp, &ho_RegionOpening, 11);
			ClosingCircle(ho_RegionOpening, &ho_RegionClosing, 3.5);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);
			Connection(ho_RegionFillUp, &ho_ConnectedRegions);
			SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);

			ho_RegionBmARoiWAdv = ho_SelectedRegions;




			ReduceObjectModel3dByView(ho_RegionBmARoiWAdv, hv_Om3dBmARoiW, "xyz_mapping",
				HTuple(), &hv_Om3dBmARoiWAdv);
			;

			//处理相机2宽度方向的点云
			ReduceObjectModel3dByView(ho_RegionBmBRoiW, hv_Om3DSceneB, "xyz_mapping", HTuple(),
				&hv_Om3dBmBRoiW);



			SampleObjectModel3d(hv_Om3dCbB, "xyz_mapping", 20, HTuple(), HTuple(), &hv_SampledObjectModel3D);


			FitPrimitivesObjectModel3d(hv_SampledObjectModel3D, (HTuple("primitive_type").Append("fitting_algorithm")),
				(HTuple("plane").Append("least_squares_tukey")), &hv_ObjectModel3DOutID);
			GetObjectModel3dParams(hv_ObjectModel3DOutID, "primitive_pose", &hv_GenParamPosePlane);
			GenPlaneObjectModel3d(hv_GenParamPosePlane, (((HTuple(-1).Append(-1)).Append(1)).Append(1)) * 1500,
				(((HTuple(-1).Append(1)).Append(1)).Append(-1)) * 1500, &hv_PlaneDatum);


			GetObjectModel3dParams(hv_ObjectModel3DOutID, "primitive_parameter", &hv_PlaneFitParam);
			hv_ValCbBRoiZ = ((const HTuple&)hv_PlaneFitParam)[3];


			SelectPointsObjectModel3d(hv_Om3dBmBRoiW, "point_coord_z", hv_ValCbBRoiZ + hv_fOffsetZSelTbRelCb,
				1000, &hv_ObjectModel3DThresholded);


			ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_ObjectModel3DThresholded, "from_xyz_map",
				HTuple(), HTuple());
			GetDomain(ho_Xs, &ho_Domain);
			ClosingRectangle1(ho_Domain, &ho_RegionClosing, 1, 5);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);
			OpeningCircle(ho_RegionFillUp, &ho_RegionOpening, 11);
			ClosingCircle(ho_RegionOpening, &ho_RegionClosing, 3.5);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);
			Connection(ho_RegionFillUp, &ho_ConnectedRegions);
			SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
			ho_RegionBmBRoiWAdv = ho_SelectedRegions;


			ReduceObjectModel3dByView(ho_RegionBmBRoiWAdv, hv_Om3dBmBRoiW, "xyz_mapping",
				HTuple(), &hv_Om3dBmBRoiWAdv);





			//合并宽度方向的点云


			GetObjectModel3dParams(hv_Om3dBmARoiWAdv, "point_coord_x", &hv_ValX);
			GetObjectModel3dParams(hv_Om3dBmARoiWAdv, "point_coord_y", &hv_ValY);
			GenRegionPoints(&ho_Region, hv_ValY, hv_ValX);
			ClosingCircle(ho_Region, &ho_RegionClosing, 3.5);
			FillUp(ho_RegionClosing, &ho_RegionA);


			GetObjectModel3dParams(hv_Om3dBmBRoiWAdv, "point_coord_x", &hv_ValX);
			GetObjectModel3dParams(hv_Om3dBmBRoiWAdv, "point_coord_y", &hv_ValY);
			GenRegionPoints(&ho_Region, hv_ValY, hv_ValX);
			ClosingCircle(ho_Region, &ho_RegionClosing, 3.5);
			FillUp(ho_RegionClosing, &ho_RegionB);



			Union2(ho_RegionA, ho_RegionB, &ho_RegionUnion);
			ClosingRectangle1(ho_RegionUnion, &ho_RegionClosing, 1, 11);
			OpeningRectangle1(ho_RegionClosing, &ho_RegionOpening, 101, 1);
			ClosingCircle(ho_RegionOpening, &ho_RegionClosing, 3.5);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);

			ho_RegionTDW = ho_RegionFillUp;





			//***************************************************************************
			//step4 板子L方向点云数据处理(耳朵伸出方向)
			//****************************************************************************

			//处理相机1L方向的点云
			ReduceObjectModel3dByView(ho_RegionBmARoiL, hv_Om3DSceneA, "xyz_mapping", HTuple(),
				&hv_Om3dBmARoiL);

			SelectPointsObjectModel3d(hv_Om3dBmARoiL, "point_coord_z", hv_ValCbARoiZ + 200,
				1000, &hv_ObjectModel3DThresholded);


			ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_ObjectModel3DThresholded, "from_xyz_map",
				HTuple(), HTuple());
			GetDomain(ho_Xs, &ho_Domain);
			ClosingRectangle1(ho_Domain, &ho_RegionClosing, 5, 1);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);
			OpeningCircle(ho_RegionFillUp, &ho_RegionOpening, 31);
			ClosingCircle(ho_RegionOpening, &ho_RegionClosing, 3.5);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);
			Connection(ho_RegionFillUp, &ho_ConnectedRegions);
			SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
			ho_RegionBmARoiLAdv = ho_SelectedRegions;

			ReduceObjectModel3dByView(ho_RegionBmARoiLAdv, hv_Om3dBmARoiL, "xyz_mapping",
				HTuple(), &hv_Om3dBmARoiLAdv);



			ReduceObjectModel3dByView(ho_RegionBmBRoiL, hv_Om3DSceneB, "xyz_mapping", HTuple(),
				&hv_Om3dBmBRoiL);

			SelectPointsObjectModel3d(hv_Om3dBmBRoiL, "point_coord_z", hv_ValCbARoiZ + 200,
				1000, &hv_ObjectModel3DThresholded);


			ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_ObjectModel3DThresholded, "from_xyz_map",
				HTuple(), HTuple());
			GetDomain(ho_Xs, &ho_Domain);
			ClosingRectangle1(ho_Domain, &ho_RegionClosing, 5, 1);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);
			OpeningCircle(ho_RegionFillUp, &ho_RegionOpening, 31);
			ClosingCircle(ho_RegionOpening, &ho_RegionClosing, 3.5);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);
			Connection(ho_RegionFillUp, &ho_ConnectedRegions);
			SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
			ho_RegionBmBRoiLAdv = ho_SelectedRegions;

			ReduceObjectModel3dByView(ho_RegionBmBRoiLAdv, hv_Om3dBmBRoiL, "xyz_mapping",
				HTuple(), &hv_Om3dBmBRoiLAdv);




			//合并L方向的点云

			GetObjectModel3dParams(hv_Om3dBmARoiLAdv, "point_coord_x", &hv_ValX);
			GetObjectModel3dParams(hv_Om3dBmARoiLAdv, "point_coord_y", &hv_ValY);
			GenRegionPoints(&ho_Region, hv_ValY, hv_ValX);
			ClosingCircle(ho_Region, &ho_RegionClosing, 3.5);
			FillUp(ho_RegionClosing, &ho_RegionA);


			GetObjectModel3dParams(hv_Om3dBmBRoiLAdv, "point_coord_x", &hv_ValX);
			GetObjectModel3dParams(hv_Om3dBmBRoiLAdv, "point_coord_y", &hv_ValY);
			GenRegionPoints(&ho_Region, hv_ValY, hv_ValX);
			ClosingCircle(ho_Region, &ho_RegionClosing, 3.5);
			FillUp(ho_RegionClosing, &ho_RegionB);



			Union2(ho_RegionA, ho_RegionB, &ho_RegionUnion);
			ClosingRectangle1(ho_RegionUnion, &ho_RegionClosing, 1, 11);
			OpeningRectangle1(ho_RegionClosing, &ho_RegionOpening, 101, 1);
			ClosingCircle(ho_RegionOpening, &ho_RegionClosing, 3.5);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);

			ho_RegionTDL = ho_RegionFillUp;

			//***************************************************************************
			//step5 计算各种位置数据  xyz+deg
			//****************************************************************************



			hv_fEdgeWLenUse = /*800*/spara1.fEdgeWLenUse;
			hv_fGndSelEdgeWOffset = /*300*/spara1.fGndSelEdgeWOffset;
			hv_fGndSelEdgeWLen = /*600*/spara1.fGndSelEdgeWLen;
			hv_fGripUp = /*0*/spara1.fGripUp;

			hv_fLimitDegMin = /*80*/spara1.fLimitDegMin;
			hv_fLimitDegMax = /*100*/spara1.fLimitDegMax;

			hv_fLimitTdWMin = /*900*/spara1.fLimitTdWMin;
			hv_fLimitTdWMax = /*1100*/spara1.fLimitTdWMax;
			hv_fLimitTdLMin = /*980*/spara1.fLimitTdLMin;
			hv_fLimitTdLMax = /*11200*/spara1.fLimitTdLMax;
			hv_fLimitTdHMin = /*300*/spara1.fLimitTdHMin;
			hv_fLimitTdHMax = /*600*/spara1.fLimitTdHMax;

			hv_RtsX = HTuple(0);
			hv_RtsY = HTuple(0);
			hv_RtsZ = HTuple(0);
			hv_RtsDeg = HTuple(0);
			hv_RtsEarDir = HTuple(0);
			hv_RtsCbZ = HTuple(0);

			hv_RtsTdW = HTuple(0);
			hv_RtsTdL = HTuple(0);
			hv_RtsTdH = HTuple(0);

			hv_bExitLimitSize = 0;


			//计算Y坐标

			SmallestRectangle1(ho_RegionTDL, &hv_Row11, &hv_Column11, &hv_Row21, &hv_Column21);
			GenRectangle1(&ho_Rectangle, hv_Row11, hv_Column11, hv_Row21, hv_Column21);
			AreaCenter(ho_Rectangle, &hv_Area, &hv_Row, &hv_Column);
			hv_RtsY = hv_Row;
			hv_RtsTdL = hv_Row21 - hv_Row11;

			//计算X坐标


			GenRectangle1(&ho_Rectangle, hv_RtsY - (hv_fEdgeWLenUse / 2), -1000, hv_RtsY + (hv_fEdgeWLenUse / 2),
				1000);
			Intersection(ho_Rectangle, ho_RegionTDW, &ho_RegionIntersection);
			SmallestRectangle1(ho_RegionIntersection, &hv_Row12, &hv_Column12, &hv_Row22,
				&hv_Column22);
			GenRectangle1(&ho_Rectangle, hv_Row12, hv_Column12, hv_Row22, hv_Column22);
			AreaCenter(ho_Rectangle, &hv_Area, &hv_Row, &hv_Column);
			hv_RtsX = hv_Column;
			ho_RegionEdgeW = ho_RegionIntersection;
			hv_fEdgeWX1 = hv_Column12;
			hv_fEdgeWX2 = hv_Column22;

			hv_RtsTdW = hv_Column22 - hv_Column12;

			//计算角度
			Connection(ho_RegionEdgeW, &ho_ConnectedRegions);
			SelectShape(ho_ConnectedRegions, &ho_SelectedRegions, "height", "and", 150, 99999);
			GenContourRegionXld(ho_SelectedRegions, &ho_RegionContours, "center");


			SmallestRectangle1Xld(ho_RegionContours, &hv_Row1, &hv_Column1, &hv_Row2, &hv_Column2);

			//这里现在左右不好确定 可能相机那边拍图拍反了
			GenRectangle1(&ho_Rectangle2, hv_Row1 + 10, hv_Column1 - 10, hv_Row2 - 10, hv_Column1 + 500);
			GenRectangle1(&ho_Rectangle1, hv_Row1 + 10, hv_Column2 - 500, hv_Row2 - 10, hv_Column2 + 10);



			get_region_roi_xld_x(ho_RegionContours, ho_Rectangle1, &ho_XldSel);
			GetContourXld(ho_XldSel, &hv_Rows, &hv_Cols);

			GenContourPolygonXld(&ho_XldBmAUpSideW, hv_Rows, hv_Cols);

			GenCrossContourXld(&ho_Cross1, hv_Rows, hv_Cols, 2, 0);

			GenEmptyObj(&ho_Line1);
			FitLineContourXld(ho_XldSel, "tukey", 2, 0, 5, 2, &hv_Row1, &hv_Col1, &hv_Row2,
				&hv_Col2, &hv_Nr, &hv_Nc, &hv_Dist);
			TupleLength(hv_Dist, &hv_Length);
			if (0 != (int(hv_Length < 1)))
			{

				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 4;
				sError.strInfo = "铜跺外边缘提取异常.";
				break;
			}

			GenContourPolygonXld(&ho_Line1, hv_Row1.TupleConcat(hv_Row2), hv_Col1.TupleConcat(hv_Col2));
			TupleAtan2(hv_Row2 - hv_Row1, hv_Col2 - hv_Col1, &hv_ATan);
			TupleDeg(hv_ATan, &hv_Deg1);
			if (0 != (int(hv_Deg1 < 0)))
			{
				hv_Deg1 += 180;
			}


			get_region_roi_xld_x(ho_RegionContours, ho_Rectangle2, &ho_XldSel);
			GetContourXld(ho_XldSel, &hv_Rows, &hv_Cols);
			GenContourPolygonXld(&ho_XldBmBUpSideW, hv_Rows, hv_Cols);

			GenCrossContourXld(&ho_Cross2, hv_Rows, hv_Cols, 2, 0);

			GenEmptyObj(&ho_Line2);
			FitLineContourXld(ho_XldSel, "tukey", 2, 0, 5, 2, &hv_Row1, &hv_Col1, &hv_Row2,
				&hv_Col2, &hv_Nr, &hv_Nc, &hv_Dist);
			TupleLength(hv_Dist, &hv_Length);
			if (0 != (int(hv_Length < 1)))
			{
				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 4;
				sError.strInfo = "铜跺外边缘提取异常.";
				break;
			}

			GenContourPolygonXld(&ho_Line2, hv_Row1.TupleConcat(hv_Row2), hv_Col1.TupleConcat(hv_Col2));
			TupleAtan2(hv_Row2 - hv_Row1, hv_Col2 - hv_Col1, &hv_ATan);
			TupleDeg(hv_ATan, &hv_Deg2);
			if (0 != (int(hv_Deg2 < 0)))
			{
				hv_Deg2 += 180;
			}


			hv_RtsDeg = (hv_Deg2 + hv_Deg1) / 2;

			//计算耳朵朝向

			ReduceObjectModel3dByView(ho_RegionEarA, hv_Om3DSceneA, "xyz_mapping", HTuple(), &hv_Om3dEarA);

			GetObjectModel3dParams(hv_Om3dBmBRoiW, "center", &hv_GenParamValue1);
			GetObjectModel3dParams(hv_Om3dEarA, "center", &hv_GenParamValue2);

			if (0 != (int(HTuple(hv_GenParamValue2[1]) > HTuple(hv_GenParamValue1[1]))))
			{
				hv_RtsEarDir = 1;
			}
			else
			{
				hv_RtsEarDir = 3;
			}


			//计算车板底面高度 特别重要 由于车板不同位置高度不一致  拼接也存在误差 高度应考虑夹具位置的高度

			ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_Om3dCbA, "from_xyz_map", HTuple(), HTuple());

			SelectPointsObjectModel3d(hv_Om3dCbA, (HTuple("point_coord_x").Append("point_coord_y")),
				hv_fEdgeWX2.TupleConcat(hv_RtsY - (hv_fGndSelEdgeWLen / 2)), (hv_fEdgeWX2 + hv_fGndSelEdgeWOffset).TupleConcat(hv_RtsY + (hv_fGndSelEdgeWLen / 2)),
				&hv_Om3dCbANear);


			GetObjectModel3dParams(hv_Om3dCbANear, "point_coord_z", &hv_Val);

			TupleSort(hv_Val, &hv_Sorted);
			hv_RadioLeft = 0.9;
			hv_RadioRight = 0.95;
			TupleSort(hv_Val, &hv_Sorted);
			TupleInt((hv_Sorted.TupleLength()) * hv_RadioLeft, &hv_left);
			TupleInt((hv_Sorted.TupleLength()) * hv_RadioRight, &hv_right);
			TupleSelectRange(hv_Sorted, hv_left, hv_right, &hv_Selected);
			TupleMean(hv_Selected, &hv_Mean);

			hv_RtsCbZ[0] = hv_Mean;


			ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_Om3dCbB, "from_xyz_map", HTuple(),
				HTuple());
			SelectPointsObjectModel3d(hv_Om3dCbB, (HTuple("point_coord_x").Append("point_coord_y")),
				(hv_fEdgeWX1 - hv_fGndSelEdgeWOffset).TupleConcat(hv_RtsY - (hv_fGndSelEdgeWLen / 2)),
				hv_fEdgeWX1.TupleConcat(hv_RtsY + (hv_fGndSelEdgeWLen / 2)), &hv_Om3dCbBNear);



			GetObjectModel3dParams(hv_Om3dCbBNear, "point_coord_z", &hv_Val);

			TupleSort(hv_Val, &hv_Sorted);
			hv_RadioLeft = 0.9;
			hv_RadioRight = 0.95;
			TupleSort(hv_Val, &hv_Sorted);
			TupleInt((hv_Sorted.TupleLength()) * hv_RadioLeft, &hv_left);
			TupleInt((hv_Sorted.TupleLength()) * hv_RadioRight, &hv_right);
			TupleSelectRange(hv_Sorted, hv_left, hv_right, &hv_Selected);
			TupleMean(hv_Selected, &hv_Mean);

			hv_RtsCbZ[1] = hv_Mean;
			hv_RtsZ = (hv_RtsCbZ.TupleMean()) + hv_fGripUp;


			//计算铜跺的高度

			ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_Om3dTbA, "from_xyz_map", HTuple(),
				HTuple());
			GetDomain(ho_Xs, &ho_Domain);
			ClosingCircle(ho_Domain, &ho_RegionClosing, 3.5);
			FillUp(ho_RegionClosing, &ho_RegionFillUp);
			OpeningCircle(ho_RegionFillUp, &ho_RegionOpening, 201);
			FillUp(ho_RegionOpening, &ho_RegionFillUp);
			Intersection(ho_RegionFillUp, ho_Zs, &ho_RegionIntersection);
			GetRegionPoints(ho_RegionIntersection, &hv_Rows, &hv_Columns);
			GetGrayval(ho_Zs, hv_Rows, hv_Columns, &hv_Val);
			TupleSort(hv_Val, &hv_Sorted);
			hv_RadioLeft = 0.98;
			hv_RadioRight = 0.99;
			TupleSort(hv_Val, &hv_Sorted);
			TupleInt((hv_Sorted.TupleLength()) * hv_RadioLeft, &hv_left);
			TupleInt((hv_Sorted.TupleLength()) * hv_RadioRight, &hv_right);
			TupleSelectRange(hv_Sorted, hv_left, hv_right, &hv_Selected);
			TupleMean(hv_Selected, &hv_Mean);
			hv_RtsTdH = hv_Mean - (hv_RtsCbZ.TupleMean());



			//***************************************************************************
   //step6 检测宽度方向单边凸出距离（爪子抓取区域的凸出距离 靠近耳部和尾部的凸出会不加入计算）
   //****************************************************************************
			hv_fThTbLayerOuter = /*50*/spara1.fThTbLayerOuter;

			bool bExitTbLayerOuter = false;
			hv_fTbLayerOuterDis1 = 0;
			hv_fTbLayerOuterDis2 = 0;

			//上层点云映射到底面
			GetObjectModel3dParams(hv_Om3dTbA, "point_coord_x", &hv_ValX);
			GetObjectModel3dParams(hv_Om3dTbA, "point_coord_y", &hv_ValY);
			GenRegionPoints(&ho_Region, hv_ValY, hv_ValX);
			ClosingCircle(ho_Region, &ho_RegionClosing, 3.5);
			FillUp(ho_RegionClosing, &ho_RegionA);

			GetContourXld(ho_XldBmAUpSideW, &hv_Rows, &hv_Cols);
			DistancePr(ho_RegionA, hv_Rows, hv_Cols, &hv_DistanceMin, &hv_DistanceMax);
			TupleMax(hv_DistanceMin, &hv_Max);
			hv_fTbLayerOuterDis1 = hv_Max;


			GetObjectModel3dParams(hv_Om3dTbB, "point_coord_x", &hv_ValX);
			GetObjectModel3dParams(hv_Om3dTbB, "point_coord_y", &hv_ValY);
			GenRegionPoints(&ho_Region, hv_ValY, hv_ValX);
			ClosingCircle(ho_Region, &ho_RegionClosing, 3.5);
			FillUp(ho_RegionClosing, &ho_RegionB);


			GetContourXld(ho_XldBmBUpSideW, &hv_Rows, &hv_Cols);
			DistancePr(ho_RegionB, hv_Rows, hv_Cols, &hv_DistanceMin, &hv_DistanceMax);
			TupleMax(hv_DistanceMin, &hv_Max);

			hv_fTbLayerOuterDis2 = hv_Max;


			if (0 != (HTuple(int(hv_fTbLayerOuterDis1 > hv_fThTbLayerOuter)).TupleOr(int(hv_fTbLayerOuterDis2 > hv_fThTbLayerOuter))))
			{

				bExitTbLayerOuter = 1;

				spara2.bExitTbLayerOuter = bExitTbLayerOuter; //存在层错
				spara2.fTbLayerOuterDis1 = hv_fTbLayerOuterDis1[0].D();
				spara2.fTbLayerOuterDis2 = hv_fTbLayerOuterDis2[0].D();

				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 5;
				sError.strInfo = "铜跺存在层错.Dis=[" + std::to_string((int)spara2.fTbLayerOuterDis1) + " , " + std::to_string((int)spara2.fTbLayerOuterDis2) + "]";


				break;
			}



			//***************************************************************************
			//step7 计算宽度方向单边凸出距离
			//****************************************************************************



			hv_fDefectGripSpaceRoiXOffeset = /*10*/spara1.fDefectGripSpaceRoiXOffeset;
			hv_fDefectGripSpaceRoiXLen = /*200*/spara1.fDefectGripSpaceRoiXLen;
			hv_fDefectGripSpaceRoiYLen = /*600*/spara1.fDefectGripSpaceRoiYLen;
			hv_fDefectGripSpaceRoiZOffesetCb = /*35*/spara1.fDefectGripSpaceRoiZOffesetCb;
			hv_fDefectGripSpaceRoiZLen = /*500*/spara1.fDefectGripSpaceRoiZLen;

			hv_fDefectGripSpaceThLen = /*50.00*/spara1.fDefectGripSpaceThLen;
			hv_fDefectGripSpaceThNumPts = /*10*/spara1.fDefectGripSpaceThNumPts;

			bool bExistDefectGripSpace = 0;
			hv_iDefectGripSpaceRtsListPtsSel = HTuple();
			hv_fDefectGripSpaceRtsListLSel = HTuple();
			GenEmptyObj(&ho_RegionDefectGripSpaceRts1);
			GenEmptyObj(&ho_RegionDefectGripSpaceRts2);
			//1111111111111111111111111111111111111111111111111111111111111111111111111111



			GetContourXld(ho_XldBmAUpSideW, &hv_Rows, &hv_Cols);
			hv_ValXEdgeA = hv_Cols.TupleMax();
			CreatePose((hv_ValXEdgeA + (hv_fDefectGripSpaceRoiXLen / 2)) + hv_fDefectGripSpaceRoiXOffeset,
				hv_RtsY, (HTuple(hv_RtsCbZ[0]) + hv_fDefectGripSpaceRoiZOffesetCb) + (hv_fDefectGripSpaceRoiZLen / 2),
				0, 0, 0, "Rp+T", "gba", "point", &hv_PoseBox);
			GenBoxObjectModel3d(hv_PoseBox, hv_fDefectGripSpaceRoiXLen, hv_fDefectGripSpaceRoiYLen,
				hv_fDefectGripSpaceRoiZLen, &hv_ObjectModel3DBox);


			SelectPointsObjectModel3d(hv_Om3DSceneA, ((HTuple("point_coord_z").Append("point_coord_x")).Append("point_coord_y")),
				((HTuple(hv_RtsCbZ[0]) + hv_fDefectGripSpaceRoiZOffesetCb).TupleConcat(hv_ValXEdgeA + hv_fDefectGripSpaceRoiXOffeset)).TupleConcat(hv_RtsY - (hv_fDefectGripSpaceRoiYLen / 2)),
				(((HTuple(hv_RtsCbZ[0]) + hv_fDefectGripSpaceRoiZOffesetCb) + hv_fDefectGripSpaceRoiZLen).TupleConcat((hv_ValXEdgeA + hv_fDefectGripSpaceRoiXOffeset) + hv_fDefectGripSpaceRoiXLen)).TupleConcat(hv_RtsY + (hv_fDefectGripSpaceRoiYLen / 2)),
				&hv_Om3dGripRoi);

			GetObjectModel3dParams(hv_Om3dGripRoi, "num_points", &hv_GenParamValue);



			if (0 != (int(hv_GenParamValue > hv_fDefectGripSpaceThNumPts)))
			{


				ConnectionObjectModel3d(hv_Om3dGripRoi, "distance_3d", 50, &hv_ObjectModel3DConnected);
				SelectObjectModel3d(hv_ObjectModel3DConnected, "diameter_object", "and", hv_fDefectGripSpaceThLen,
					9999999, &hv_ObjectModel3DSelected);
				if (0 != (int((hv_ObjectModel3DSelected.TupleLength()) > 0)))
				{

					UnionObjectModel3d(hv_ObjectModel3DSelected, "points_surface", &hv_UnionObjectModel3D);
					GetObjectModel3dParams(hv_UnionObjectModel3D, "num_points", &hv_GenParamValue);
					if (0 != (int(hv_GenParamValue > hv_fDefectGripSpaceThNumPts)))
					{
						MaxDiameterObjectModel3d(hv_UnionObjectModel3D, &hv_Diameter);
						hv_iDefectGripSpaceRtsListPtsSel = hv_iDefectGripSpaceRtsListPtsSel.TupleConcat(hv_GenParamValue);
						hv_fDefectGripSpaceRtsListLSel = hv_fDefectGripSpaceRtsListLSel.TupleConcat(hv_Diameter);

						ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_UnionObjectModel3D, "from_xyz_map",
							HTuple(), HTuple());
						GetDomain(ho_Zs, &ho_Domain);
						Connection(ho_Domain, &ho_ConnectedRegions);
						SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
						SmallestRectangle1(ho_SelectedRegions, &hv_Row1, &hv_Column1, &hv_Row2,
							&hv_Column2);
						GenRectangle1(&ho_Rectangle, hv_Row1, hv_Column1, hv_Row2, hv_Column2);
						ConcatObj(ho_RegionDefectGripSpaceRts1, ho_Rectangle, &ho_RegionDefectGripSpaceRts1
						);

					}
				}

			}

			//22222222222222222222222222222222222222222222222222222222222222222222222222222
			GetContourXld(ho_XldBmBUpSideW, &hv_Rows, &hv_Cols);
			hv_ValXEdgeB = hv_Cols.TupleMin();
			CreatePose((hv_ValXEdgeB - (hv_fDefectGripSpaceRoiXLen / 2)) - hv_fDefectGripSpaceRoiXOffeset,
				hv_RtsY, (HTuple(hv_RtsCbZ[1]) + hv_fDefectGripSpaceRoiZOffesetCb) + (hv_fDefectGripSpaceRoiZLen / 2),
				0, 0, 0, "Rp+T", "gba", "point", &hv_PoseBox);
			GenBoxObjectModel3d(hv_PoseBox, hv_fDefectGripSpaceRoiXLen, hv_fDefectGripSpaceRoiYLen,
				hv_fDefectGripSpaceRoiZLen, &hv_ObjectModel3DBox);


			SelectPointsObjectModel3d(hv_Om3DSceneB, ((HTuple("point_coord_z").Append("point_coord_x")).Append("point_coord_y")),
				((HTuple(hv_RtsCbZ[1]) + hv_fDefectGripSpaceRoiZOffesetCb).TupleConcat((hv_ValXEdgeB - hv_fDefectGripSpaceRoiXOffeset) - hv_fDefectGripSpaceRoiXLen)).TupleConcat(hv_RtsY - (hv_fDefectGripSpaceRoiYLen / 2)),
				(((HTuple(hv_RtsCbZ[1]) + hv_fDefectGripSpaceRoiZOffesetCb) + hv_fDefectGripSpaceRoiZLen).TupleConcat(hv_ValXEdgeB - hv_fDefectGripSpaceRoiXOffeset)).TupleConcat(hv_RtsY + (hv_fDefectGripSpaceRoiYLen / 2)),
				&hv_Om3dGripRoi);

			GetObjectModel3dParams(hv_Om3dGripRoi, "num_points", &hv_GenParamValue);

			if (0 != (int(hv_GenParamValue > hv_fDefectGripSpaceThNumPts)))
			{


				ConnectionObjectModel3d(hv_Om3dGripRoi, "distance_3d", 50, &hv_ObjectModel3DConnected);
				SelectObjectModel3d(hv_ObjectModel3DConnected, "diameter_object", "and", hv_fDefectGripSpaceThLen,
					9999999, &hv_ObjectModel3DSelected);
				if (0 != (int((hv_ObjectModel3DSelected.TupleLength()) > 0)))
				{

					UnionObjectModel3d(hv_ObjectModel3DSelected, "points_surface", &hv_UnionObjectModel3D);
					GetObjectModel3dParams(hv_UnionObjectModel3D, "num_points", &hv_GenParamValue);
					if (0 != (int(hv_GenParamValue > hv_fDefectGripSpaceThNumPts)))
					{

						MaxDiameterObjectModel3d(hv_UnionObjectModel3D, &hv_Diameter);
						hv_iDefectGripSpaceRtsListPtsSel = hv_iDefectGripSpaceRtsListPtsSel.TupleConcat(hv_GenParamValue);
						hv_fDefectGripSpaceRtsListLSel = hv_fDefectGripSpaceRtsListLSel.TupleConcat(hv_Diameter);

						ObjectModel3dToXyz(&ho_Xs, &ho_Ys, &ho_Zs, hv_UnionObjectModel3D, "from_xyz_map",
							HTuple(), HTuple());
						GetDomain(ho_Zs, &ho_Domain);
						Connection(ho_Domain, &ho_ConnectedRegions);
						SelectShapeStd(ho_ConnectedRegions, &ho_SelectedRegions, "max_area", 70);
						SmallestRectangle1(ho_SelectedRegions, &hv_Row1, &hv_Column1, &hv_Row2,
							&hv_Column2);
						GenRectangle1(&ho_Rectangle, hv_Row1, hv_Column1, hv_Row2, hv_Column2);
						ConcatObj(ho_RegionDefectGripSpaceRts2, ho_Rectangle, &ho_RegionDefectGripSpaceRts2);

					}
				}

			}


			CountObj(ho_RegionDefectGripSpaceRts1, &hv_Number);
			if (0 != (int(hv_Number > 0)))
			{
				bExistDefectGripSpace = 1;
			}

			CountObj(ho_RegionDefectGripSpaceRts2, &hv_Number);
			if (0 != (int(hv_Number > 0)))
			{
				bExistDefectGripSpace = 1;
			}

			if (bExistDefectGripSpace == true)
			{
				spara2.bExistDefectGripSpace = bExistDefectGripSpace;  //存在夹爪空间干涉
				for (size_t i = 0; i < hv_iDefectGripSpaceRtsListPtsSel.Length(); i++)
				{
					spara2.iDefectGripSpaceRtsListPtsSel.push_back(hv_iDefectGripSpaceRtsListPtsSel[i].I());
				}

				for (size_t i = 0; i < hv_fDefectGripSpaceRtsListLSel.Length(); i++)
				{
					spara2.fDefectGripSpaceRtsListLSel.push_back(hv_fDefectGripSpaceRtsListLSel[i].D());
				}

				if (ho_RegionDefectGripSpaceRts1.IsInitialized() && ho_RegionDefectGripSpaceRts1.CountObj() > 0) { spara2.RegionDefectGripSpaceRts1 = ho_RegionDefectGripSpaceRts1.Clone(); };
				if (ho_RegionDefectGripSpaceRts2.IsInitialized() && ho_RegionDefectGripSpaceRts2.CountObj() > 0) { spara2.RegionDefectGripSpaceRts2 = ho_RegionDefectGripSpaceRts2.Clone(); };


				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 6;
				sError.strInfo = "存在夹爪空间区域干涉.";


				break;

			}



			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//结果数据


			spara2.bExitSafeDisTd = bExitSafeDisTd;   //存在铜跺安全距离超限
			spara2.fDefectTdNearRtsDis1 = hv_fDefectTdNearRtsDis1[0].D();  //间距1
			spara2.fDefectTdNearRtsDis2 = hv_fDefectTdNearRtsDis2[0].D();  //间距2


			spara2.TdX = hv_RtsX[0].D();    //铜跺抓取位 XYZ+DEG
			spara2.TdY = hv_RtsY[0].D();
			spara2.TdZ = hv_RtsZ[0].D();
			spara2.TdDeg = hv_RtsDeg[0].D();
			spara2.TdEarDir = hv_RtsEarDir[0].I(); //0:0度方向 1:90度方向  2:180度方向:3：270度方向

			spara2.CbZ[0] = hv_RtsCbZ[0].D();   //车板Z
			spara2.CbZ[1] = hv_RtsCbZ[1].D();

			spara2.TdW = hv_RtsTdW[0].D();     //铜跺的长宽高
			spara2.TdL = hv_RtsTdL[0].D();
			spara2.TdH = hv_RtsTdH[0].D();

			//RegionBmARoiWAdv
			if (ho_RegionBmARoiWAdv.IsInitialized() && ho_RegionBmARoiWAdv.CountObj() > 0) { spara2.RegionTDW1 = ho_RegionBmARoiWAdv.Clone(); };
			if (ho_RegionBmBRoiWAdv.IsInitialized() && ho_RegionBmBRoiWAdv.CountObj() > 0) { spara2.RegionTDW2 = ho_RegionBmBRoiWAdv.Clone(); };

			if (ho_RegionBmARoiLAdv.IsInitialized() && ho_RegionBmARoiLAdv.CountObj() > 0) { spara2.RegionTDL1 = ho_RegionBmARoiLAdv.Clone(); };
			if (ho_RegionBmBRoiLAdv.IsInitialized() && ho_RegionBmBRoiLAdv.CountObj() > 0) { spara2.RegionTDL2 = ho_RegionBmBRoiLAdv.Clone(); };



			//范围判定及报警


			if (spara2.TdDeg< spara1.fLimitDegMin || spara2.TdDeg >spara1.fLimitDegMax)
			{
				spara2.bExitLimitSize = true;
			}

			if (spara2.TdW< spara1.fLimitTdWMin || spara2.TdW >spara1.fLimitTdWMax)
			{
				spara2.bExitLimitSize = true;
			}

			if (spara2.TdL< spara1.fLimitTdLMin || spara2.TdL >spara1.fLimitTdLMax)
			{
				spara2.bExitLimitSize = true;
			}

			if (spara2.TdH< spara1.fLimitTdHMin || spara2.TdH >spara1.fLimitTdHMax)
			{
				spara2.bExitLimitSize = true;
			}

			if (spara2.bExitLimitSize == true)
			{
				CountSeconds(&time_end);
				spara2.time = time_end - time_start;

				sError.iCode = 9;
				sError.strInfo = "铜跺尺寸超范围.W=" + std::to_string((int)spara2.TdW) + " L= " + std::to_string((int)spara2.TdL) + " H=" + std::to_string((int)spara2.TdH) + "Deg=" + std::to_string((int)spara2.TdDeg);


				break;
			}

			spara2.bExitTbLayerOuter = bExitTbLayerOuter; //存在层错
			spara2.fTbLayerOuterDis1 = hv_fTbLayerOuterDis1[0].D();
			spara2.fTbLayerOuterDis2 = hv_fTbLayerOuterDis2[0].D();

			spara2.bExistDefectGripSpace = bExistDefectGripSpace;  //存在夹爪空间缺陷
			for (size_t i = 0; i < hv_iDefectGripSpaceRtsListPtsSel.Length(); i++)
			{
				spara2.iDefectGripSpaceRtsListPtsSel.push_back(hv_iDefectGripSpaceRtsListPtsSel[i].I());
			}

			for (size_t i = 0; i < hv_fDefectGripSpaceRtsListLSel.Length(); i++)
			{
				spara2.fDefectGripSpaceRtsListLSel.push_back(hv_fDefectGripSpaceRtsListLSel[i].D());
			}

			if (ho_RegionDefectGripSpaceRts1.IsInitialized() && ho_RegionDefectGripSpaceRts1.CountObj() > 0) { spara2.RegionDefectGripSpaceRts1 = ho_RegionDefectGripSpaceRts1.Clone(); };
			if (ho_RegionDefectGripSpaceRts2.IsInitialized() && ho_RegionDefectGripSpaceRts2.CountObj() > 0) { spara2.RegionDefectGripSpaceRts2 = ho_RegionDefectGripSpaceRts2.Clone(); };



			//只要有一项不满足 综合判定就为NG
			spara2.bTJG = spara2.bExitLimitSize == false && spara2.bExitSafeDisTd == false && spara2.bExitTbLayerOuter == false && spara2.bExistDefectGripSpace == false;


			//// 3D
			//if (hv_ObjectModel3DAdv.Length() > 0)
			//{
			//	try { CopyObjectModel3d(hv_ObjectModel3DAdv, "all", &spara2.OM3ImageAdv); }
			//	catch (HalconCpp::HException& ex) { ; }

			//}

			sError.iCode = 0;
			sError.strInfo = "";

		}
		catch (HalconCpp::HException& ex)
		{
			std::string errProc = ex.ProcName(); //报错算子
			std::string errMsg = ex.ErrorMessage(); //错误提示

			sError.iCode = -1;
			sError.strInfo = "算子异常-"+ errProc;
		}


		break;
	}


	//释放所有临时变量

	CountSeconds(&time_end);
	spara2.time = time_end - time_start;
	return sError;
}


void CUnloadPlateA::get_region_roi_xld_x(HObject ho_Contours, HObject ho_Region, HObject* ho_ContoursSel)
{

	// Local iconic variables

	// Local control variables
	HTuple  hv_Rows, hv_Cols, hv_Number, hv_Index;
	HTuple  hv_Row, hv_Col, hv_DistanceMin, hv_DistanceMax;
	HTuple  hv_Indices, hv_RowSel, hv_ColSel;

	hv_Rows = HTuple();
	hv_Cols = HTuple();
	CountObj(ho_Contours, &hv_Number);
	{
		HTuple end_val3 = hv_Number;
		HTuple step_val3 = 1;
		for (hv_Index = 1; hv_Index.Continue(end_val3, step_val3); hv_Index += step_val3)
		{

			SelectObj(ho_Contours, &(*ho_ContoursSel), hv_Index);
			GetContourXld((*ho_ContoursSel), &hv_Row, &hv_Col);

			hv_Rows = hv_Rows.TupleConcat(hv_Row);
			hv_Cols = hv_Cols.TupleConcat(hv_Col);

		}
	}


	DistancePr(ho_Region, hv_Rows, hv_Cols, &hv_DistanceMin, &hv_DistanceMax);

	TupleFind(hv_DistanceMin, 0, &hv_Indices);
	TupleSelect(hv_Rows, hv_Indices, &hv_RowSel);
	TupleSelect(hv_Cols, hv_Indices, &hv_ColSel);


	GenContourPolygonXld(&(*ho_ContoursSel), hv_RowSel, hv_ColSel);


	return;
}

