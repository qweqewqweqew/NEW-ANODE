#pragma once

// 版本：V1.0.0.3

#ifndef BUTTERFLYS7_H
#define BUTTERFLYS7_H

// PLC寄存器代码
#define AreaI		0x81		//I区
#define AreaQ		0x82		//Q区
#define AreaM		0x83		//M区
#define AreaDB		0x84		//DB区、V区

// 接口函数标志
#ifndef EXPORTFUNC
#define EXPORTFUNC extern "C" __declspec(dllimport)
#endif

// 32/64位整型定义
#ifdef _WIN64
typedef unsigned __int64	PlcHandle;
#else
typedef unsigned int		PlcHandle;
#endif


//创建连接对象
//返回值：PLC对象的识别码，读写时要用到
EXPORTFUNC PlcHandle __stdcall BfS7_CreatePlc();

//销毁连接对象
//返回值：无
EXPORTFUNC void __stdcall BfS7_DestoryPlc(PlcHandle *uiPlcHandle);					//[in]  PLC对象的识别码

//连接
//返回值：0成功，非0为错误代码
EXPORTFUNC int __stdcall BfS7_ConnectPlc(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
										 char* pcIP,								//[in]  PLC的IP地址
										 int iRack,									//[in]  机架号（一般为0）
										 int iSlot,									//[in]  槽号（参考：S7-1200和S7-1500为0，S7-200和S7-200 smart为1，S7-300和S7-400为2）
										 bool bIsS7_200 = false,					//[in]  S7-200的CP243模块需要用以太网向导手动配置模块的TSAP值。此位为true时usLocalTSAP和usRemoteTSAP将生效
										 unsigned short usLocalTSAP = 0x0100,		//[in]  PC端的TSAP，即CP243模块端配置的RemoteTSAP
										 unsigned short usRemoteTSAP = 0x1000);		//[in]  CP243端的TSAP，即CP243模块端配置的LocalTSAP

//断开连接
//返回值：0成功，非0为错误代码
EXPORTFUNC int __stdcall BfS7_DisconnectPlc(PlcHandle uiPlcHandle);					//[in]  PLC对象的识别码

//读写位
//返回值：0成功，非0为错误代码
EXPORTFUNC int __stdcall BfS7_ReadBool(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
									   int iArea,									//[in]  PLC寄存器代码
									   int iDBNum,									//[in]  DB块编号，非DB寄存器的变量忽略此参数
									   int iByteNum,								//[in]  字节编号
									   int iBitNum,									//[in]	字节中的位编号（由低到高为；0~7）
									   bool *bValue);								//[out] 值

EXPORTFUNC int __stdcall BfS7_WriteBool(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
										int iArea,									//[in]  PLC寄存器代码
										int iDBNum,									//[in]  DB块编号，非DB寄存器的变量忽略此参数
										int iByteNum,								//[in]  字节编号
										int iBitNum,								//[in]	字节中的位编号（由低到高为；0~7）
										bool bValue);								//[in]  值

//读写字节
//返回值：0成功，非0为错误代码
EXPORTFUNC int __stdcall BfS7_ReadByte(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
									   int iArea,									//[in]  PLC寄存器代码
									   int iDBNum,									//[in]  DB块编号，非DB寄存器的变量忽略此参数
									   int iByteNum,								//[in]  字节编号
									   unsigned char *ucValue);						//[out] 值

EXPORTFUNC int __stdcall BfS7_WriteByte(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
										int iArea,									//[in]  PLC寄存器代码
										int iDBNum,									//[in]  DB块编号，非DB寄存器的变量忽略此参数
										int iByteNum,								//[in]  字节编号
										unsigned char ucValue);						//[in]  值

//读写字（无符号整型，2字节）
//返回值：0成功，非0为错误代码
EXPORTFUNC int __stdcall BfS7_ReadWord(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
									   int iArea,									//[in]  PLC寄存器代码
									   int iDBNum,									//[in]  DB块编号，非DB寄存器的变量忽略此参数
									   int iByteNum,								//[in]  起始字节编号
									   unsigned short *usValue);					//[out] 值

EXPORTFUNC int __stdcall BfS7_WriteWord(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
										int iArea,									//[in]  PLC寄存器代码
										int iDBNum,									//[in]  DB块编号，非DB寄存器的变量忽略此参数
										int iByteNum,								//[in]  起始字节编号
										unsigned short usValue);					//[in]  值

//读写整数（有符号整型，2字节）
//返回值：0成功，非0为错误代码
EXPORTFUNC int __stdcall BfS7_ReadInt(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
									  int iArea,									//[in]  PLC寄存器代码
									  int iDBNum,									//[in]  DB块编号，非DB寄存器的变量忽略此参数
									  int iByteNum,									//[in]  起始字节编号
									  short *sValue);								//[out] 值

EXPORTFUNC int __stdcall BfS7_WriteInt(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
									   int iArea,									//[in]  PLC寄存器代码
									   int iDBNum,									//[in]  DB块编号，非DB寄存器的变量忽略此参数
									   int iByteNum,								//[in]  起始字节编号
									   short sValue);								//[in]  值

//读写双字（无符号整型，4字节）
//返回值：0成功，非0为错误代码
EXPORTFUNC int __stdcall BfS7_ReadDWord(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
										int iArea,									//[in]  PLC寄存器代码
										int iDBNum,									//[in]  DB块编号，非DB寄存器的变量忽略此参数
										int iByteNum,								//[in]  起始字节编号
										unsigned int *uiValue);						//[out] 值

EXPORTFUNC int __stdcall BfS7_WriteDWord(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
										 int iArea,									//[in]  PLC寄存器代码
										 int iDBNum,								//[in]  DB块编号，非DB寄存器的变量忽略此参数
										 int iByteNum,								//[in]  起始字节编号
									 	 unsigned int uiValue);						//[in]  值

//读写双字（有符号整型，4字节）
//返回值：0成功，非0为错误代码
EXPORTFUNC int __stdcall BfS7_ReadDInt(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
									   int iArea,									//[in]  PLC寄存器代码
									   int iDBNum,									//[in]  DB块编号，非DB寄存器的变量忽略此参数
									   int iByteNum,								//[in]  起始字节编号
									   int *iValue);								//[out] 值

EXPORTFUNC int __stdcall BfS7_WriteDInt(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
										int iArea,									//[in]  PLC寄存器代码
										int iDBNum,									//[in]  DB块编号，非DB寄存器的变量忽略此参数
										int iByteNum,								//[in]  起始字节编号
										int iValue);								//[in]  值

//读写浮点数（4字节单精度）
//返回值：0成功，非0为错误代码
EXPORTFUNC int __stdcall BfS7_ReadFloat(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
										int iArea,									//[in]  PLC寄存器代码
										int iDBNum,									//[in]  DB块编号，非DB寄存器的变量忽略此参数
										int iByteNum,								//[in]  起始字节编号
										float *fValue);								//[out] 值

EXPORTFUNC int __stdcall BfS7_WriteFloat(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
										 int iArea,									//[in]  PLC寄存器代码
										 int iDBNum,								//[in]  DB块编号，非DB寄存器的变量忽略此参数
										 int iByteNum,								//[in]  起始字节编号
										 float fValue);								//[in]  值

//读写浮点数（4字节单精度）
//返回值：0成功，非0为错误代码
EXPORTFUNC int __stdcall BfS7_ReadString(PlcHandle uiPlcHandle,						//[in]  PLC对象的识别码
										 int iArea,									//[in]  PLC寄存器代码
										 int iDBNum,								//[in]  DB块编号，非DB寄存器的变量忽略此参数
										 int iByteNum,								//[in]  起始字节编号
										 short sStringLength,						//[in]  Buffer的长度
										 char* pcValue);							//[in]  Buffer的地址。从PLC读取的长度是Buffer的长度，将读出来的字节数组从此地址开始写

EXPORTFUNC int __stdcall BfS7_WriteString(PlcHandle uiPlcHandle,					//[in]  PLC对象的识别码
										  int iArea,								//[in]  PLC寄存器代码
										  int iDBNum,								//[in]  DB块编号，非DB寄存器的变量忽略此参数
										  int iByteNum,								//[in]  起始字节编号
										  short sStringLength,						//[in]  Buffer的长度
										  char* pcValue);							//[in]  Buffer的地址。写入到PLC的长度是Buffer的长度，需要写入的数据从此地址开始读

//按字节整块读写
//返回值：0成功，非0为错误代码
EXPORTFUNC int __stdcall BfS7_ReadBlockAsByte(PlcHandle uiPlcHandle,				//[in]  PLC对象的识别码
											  int iArea,							//[in]  PLC寄存器代码
											  int iDBNum,							//[in]  DB块编号，非DB寄存器的变量忽略此参数
											  int iByteNum,							//[in]  起始字节编号
											  int iLength,							//[in]  Buffer的长度
											  unsigned char* pucValue);				//[in]  Buffer的地址。从PLC读取的长度是Buffer的长度，将读出来的字节数组从此地址开始写

EXPORTFUNC int __stdcall BfS7_WriteBlockAsByte(PlcHandle uiPlcHandle,				//[in]  PLC对象的识别码
											   int iArea,							//[in]  PLC寄存器代码
											   int iDBNum,							//[in]  DB块编号，非DB寄存器的变量忽略此参数
											   int iByteNum,						//[in]  起始字节编号
											   int iLength,							//[in]  Buffer的长度
											   unsigned char* pucValue);			//[in]  Buffer的地址。写入到PLC的长度是Buffer的长度，需要写入的数据从此地址开始读

//根据错误代码获取提示文本
//返回值：0成功，非0为错误代码
EXPORTFUNC int __stdcall BfS7_GetErrorMsg(int iErrorCode,							//[in]  PLC对象的识别码
									      char* pcText,								//[in]  字符串起始位置的地址，字符串将从这个地址开始写
									      int iTextLen);							//[in]	字符串的长度


#endif