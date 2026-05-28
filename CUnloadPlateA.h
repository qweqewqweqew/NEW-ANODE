#pragma once

#include "DataX.h"

class CUnloadPlateA
{

public:
	CUnloadPlateA();
	virtual ~CUnloadPlateA();


	//�ṹ�����ͼ��Ԥ����
	s_Rtnf OnPreProcess(s_Image3dS sImage3dS, s_PreProcess3DSPara spara1, s_PreProcess3DSResultPara& spara2);

	//�״�ͼ��Ԥ����
	s_Rtnf OnPreProcess(s_Lidar3d imageT, s_PreProcessLidar3dPara spara1, s_PreProcessLidar3dRts& spara2);

	//����λͭ��
	s_Rtnf OnPreAD(s_Lidar3d Image3d, s_PreADPlateAPara spara1, s_PreADPlateARtsPara& spara2);


	//����λ����
	s_Rtnf CalcPreAlignRts(s_PreADPlateARtsPara sLidarRts, s_CalcPreAlignPara sCalcPreAlignPara, s_CalcPreAlignRtsPara& sCalcPreRts);



	//ͨ����������任����
	bool CalcRtMat(std::vector <double> fListX1, std::vector <double> fListY1, std::vector <double> fListZ1,
		std::vector <double> fListX2, std::vector <double> fListY2, std::vector <double> fListZ2,
		s_PoseH& PoseT);


	//����λͭ��
	s_Rtnf OnAccurateAD(std::vector<s_Image3dS> ImgTList, s_AccurateADPlateAPara spara1, s_AccurateADPlateARtsPara& spara2);


	//���㾫��λ���
	s_Rtnf CalcAccurateAlignRts(s_AccurateADPlateARtsPara sImageRts, s_CalcAccurateAlignPara sCalcPreAlignPara, s_CalcAccurateAlignRtsPara& sCalcRts);

private:
	void get_object_models_center(HTuple hv_ObjectModel3DID, HTuple* hv_Center);

	void calc_rectangle2_points_x(HTuple hv_rowCenter, HTuple hv_columnCenter, HTuple hv_phi,
		HTuple hv_length1, HTuple hv_length2, HTuple* hv_pt1, HTuple* hv_pt2, HTuple* hv_pt3,
		HTuple* hv_pt4);
	void calc_plane_point_z_x(HTuple hv_PlanePara, HTuple hv_x, HTuple hv_y,
		HTuple* hv_z);

	void gen_cross3d_x(HTuple hv_CrossThickness, HTuple hv_CrossLength, HTuple hv_CrossPoseT,
		HTuple* hv_OM3Cross);

	void get_box_corner_x(HTuple hv_Length11, HTuple hv_Length21, HTuple hv_Length31,
		HTuple hv_PoseBox, HTuple* hv_CornersWorldX, HTuple* hv_CornersWorldY, HTuple* hv_CornersWorldZ);

	void get_region_roi_xld_x(HObject ho_Contours, HObject ho_Region, HObject* ho_ContoursSel);
};

