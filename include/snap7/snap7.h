/*=============================================================================|
|  PROJECT SNAP7                                                         1.4.2 |
|==============================================================================|
|  Copyright (C) 2013, 2014 Davide Nardella                                    |
|  All rights reserved.                                                        |
|==============================================================================|
|  SNAP7 is free software: you can redistribute it and/or modify               |
|  it under the terms of the Lesser GNU General Public License as published by |
|  the Free Software Foundation, either version 3 of the License, or           |
|  (at your option) any later version.                                         |
|                                                                              |
|  It means that you can distribute your commercial software linked to SNAP7   |
|  without the requirement to distribute the source code of your application   |
|  and without the requirement that your application be itself distributed     |
|  under LGPL.                                                                 |
|                                                                              |
|  SNAP7 is distributed in the hope that it will be useful,                    |
|  but WITHOUT ANY WARRANTY; without even the implied warranty of              |
|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               |
|  Lesser GNU General Public License for more details.                         |
|                                                                              |
|  You should have received a copy of the GNU General Public License and a     |
|  copy of Lesser GNU General Public License along with Snap7.                 |
|  If not, see  http://www.gnu.org/licenses/                                   |
|=============================================================================*/
#ifndef snap7_h
#define snap7_h
//------------------------------------------------------------------------------
// Platform detection
//------------------------------------------------------------------------------
#if defined (_WIN32)||defined(_WIN64)||defined(__WINDOWS__)
# define OS_WINDOWS
#endif

#ifdef OS_WINDOWS
 #define S7API __stdcall
#else
 #define S7API
#endif

//------------------------------------------------------------------------------
// C++ compatibility
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Legacy types
//------------------------------------------------------------------------------
typedef unsigned char   byte;
typedef unsigned short  word;
typedef unsigned int    longword;
typedef byte            *pbyte;
typedef word            *pword;

//******************************************************************************
//                                   PARAMS LIST
//******************************************************************************
// Parameter ID
const int p_u16_LocalPort        = 1;
const int p_u16_RemotePort       = 2;
const int p_i32_PingTimeout      = 3;
const int p_i32_SendTimeout      = 4;
const int p_i32_RecvTimeout      = 5;
const int p_i32_WorkInterval     = 6;
const int p_u16_SrcRef           = 7;
const int p_u16_DstRef           = 8;
const int p_u16_SrcTSap          = 9;
const int p_i32_PDURequest       = 10;
const int p_i32_MaxClients       = 11;
const int p_i32_BSendTimeout     = 12;
const int p_i32_BRecvTimeout     = 13;
const int p_u32_RecoveryTime     = 14;
const int p_u32_KeepAliveTime    = 15;

// Area ID
const byte S7AreaPE   =  0x81;
const byte S7AreaPA   =  0x82;
const byte S7AreaMK   =  0x83;
const byte S7AreaDB   =  0x84;
const byte S7AreaCT   =  0x1C;
const byte S7AreaTM   =  0x1D;

// Word Length ID
const int S7WLBit     = 0x01;
const int S7WLByte    = 0x02;
const int S7WLChar    = 0x03;
const int S7WLWord    = 0x04;
const int S7WLInt     = 0x05;
const int S7WLDWord   = 0x06;
const int S7WLDInt    = 0x07;
const int S7WLReal    = 0x08;
const int S7WLCounter = 0x1C;
const int S7WLTimer   = 0x1D;

// Client Connection Type
const word CONNTYPE_PG         = 0x0001;  // Connect to the PLC as a PG
const word CONNTYPE_OP         = 0x0002;  // Connect to the PLC as an OP
const word CONNTYPE_BASIC      = 0x0003;  // Basic connection

// Error codes
const int errNegotiatingPDU    = 0x00100000;
const int errCliInvalidParams  = 0x00200000;
const int errCliJobPending     = 0x00300000;
const int errCliTooManyItems   = 0x00400000;
const int errCliInvalidWordLen = 0x00500000;
const int errCliPartialDataWritten = 0x00600000;
const int errCliSizeOverPDU    = 0x00700000;
const int errCliInvalidPlcAnswer = 0x00800000;
const int errCliAddressOutOfRange = 0x00900000;
const int errCliInvalidTransportSize = 0x00A00000;
const int errCliWriteDataSizeMismatch = 0x00B00000;
const int errCliItemNotAvailable = 0x00C00000;
const int errCliInvalidValue   = 0x00D00000;
const int errCliCannotStartPLC = 0x00E00000;
const int errCliAlreadyRun     = 0x00F00000;
const int errCliCannotStopPLC  = 0x01000000;
const int errCliCannotCopyRamToRom = 0x01100000;
const int errCliCannotCompress = 0x01200000;
const int errCliAlreadyStop    = 0x01300000;
const int errCliFunNotAvailable = 0x01400000;
const int errCliUploadSequenceFailed = 0x01500000;
const int errCliInvalidDataSizeRecvd = 0x01600000;
const int errCliInvalidBlockType = 0x01700000;
const int errCliInvalidBlockNumber = 0x01800000;
const int errCliInvalidBlockSize = 0x01900000;
const int errCliDownloadSequenceFailed = 0x01A00000;
const int errCliInsertRefused  = 0x01B00000;
const int errCliDeleteRefused  = 0x01C00000;
const int errCliNeedPassword   = 0x01D00000;
const int errCliInvalidPassword = 0x01E00000;
const int errCliNoPasswordToSetOrClear = 0x01F00000;
const int errCliJobTimeout     = 0x02000000;
const int errCliPartialDataRead = 0x02100000;
const int errCliBufferTooSmall = 0x02200000;
const int errCliFunctionRefused = 0x02300000;
const int errCliDestroying     = 0x02400000;
const int errCliInvalidParamNumber = 0x02500000;
const int errCliCannotChangeParam  = 0x02600000;

//******************************************************************************
//                                  CLIENT
//******************************************************************************

// Create/Destroy
typedef void *S7Object;

S7Object S7API Cli_Create();
void S7API Cli_Destroy(S7Object *Client);

// Connection/Disconnection
int S7API Cli_ConnectTo(S7Object Client, const char *Address, int Rack, int Slot);
int S7API Cli_SetConnectionParams(S7Object Client, const char *Address, word LocalTSAP, word RemoteTSAP);
int S7API Cli_SetConnectionType(S7Object Client, word ConnectionType);
int S7API Cli_Connect(S7Object Client);
int S7API Cli_Disconnect(S7Object Client);

// Parameter management
int S7API Cli_GetParam(S7Object Client, int ParamNumber, void *pValue);
int S7API Cli_SetParam(S7Object Client, int ParamNumber, void *pValue);

// Data I/O main functions
int S7API Cli_ReadArea(S7Object Client, int Area, int DBNumber, int Start, int Amount, int WordLen, void *pData);
int S7API Cli_WriteArea(S7Object Client, int Area, int DBNumber, int Start, int Amount, int WordLen, void *pData);
int S7API Cli_DBRead(S7Object Client, int DBNumber, int Start, int Size, void *pData);
int S7API Cli_DBWrite(S7Object Client, int DBNumber, int Start, int Size, void *pData);
int S7API Cli_MBRead(S7Object Client, int Start, int Size, void *pData);
int S7API Cli_MBWrite(S7Object Client, int Start, int Size, void *pData);
int S7API Cli_EBRead(S7Object Client, int Start, int Size, void *pData);
int S7API Cli_EBWrite(S7Object Client, int Start, int Size, void *pData);
int S7API Cli_ABRead(S7Object Client, int Start, int Size, void *pData);
int S7API Cli_ABWrite(S7Object Client, int Start, int Size, void *pData);
int S7API Cli_TMRead(S7Object Client, int Start, int Amount, void *pData);
int S7API Cli_TMWrite(S7Object Client, int Start, int Amount, void *pData);
int S7API Cli_CTRead(S7Object Client, int Start, int Amount, void *pData);
int S7API Cli_CTWrite(S7Object Client, int Start, int Amount, void *pData);

// Multi variables functions
int S7API Cli_ReadMultiVars(S7Object Client, void *Item, int ItemsCount);
int S7API Cli_WriteMultiVars(S7Object Client, void *Item, int ItemsCount);

// Directory functions
int S7API Cli_ListBlocks(S7Object Client, void *pUsrData);
int S7API Cli_GetAgBlockInfo(S7Object Client, int BlockType, int BlockNum, void *pUsrData);
int S7API Cli_GetPgBlockInfo(S7Object Client, void *pBlock, void *pUsrData, int Size);
int S7API Cli_ListBlocksOfType(S7Object Client, int BlockType, void *pUsrData, int *ItemsCount);

// Block oriented functions
int S7API Cli_Upload(S7Object Client, int BlockType, int BlockNum, void *pUsrData, int *Size);
int S7API Cli_FullUpload(S7Object Client, int BlockType, int BlockNum, void *pUsrData, int *Size);
int S7API Cli_Download(S7Object Client, int BlockNum, void *pUsrData, int Size);
int S7API Cli_Delete(S7Object Client, int BlockType, int BlockNum);
int S7API Cli_DBGet(S7Object Client, int DBNumber, void *pUsrData, int *Size);
int S7API Cli_DBFill(S7Object Client, int DBNumber, int FillChar);

// Date/Time functions
int S7API Cli_GetPlcDateTime(S7Object Client, void *DateTime);
int S7API Cli_SetPlcDateTime(S7Object Client, void *DateTime);
int S7API Cli_SetPlcSystemDateTime(S7Object Client);

// System info functions
int S7API Cli_GetOrderCode(S7Object Client, void *pUsrData);
int S7API Cli_GetCpuInfo(S7Object Client, void *pUsrData);
int S7API Cli_GetCpInfo(S7Object Client, void *pUsrData);
int S7API Cli_ReadSZL(S7Object Client, int ID, int Index, void *pUsrData, int *Size);
int S7API Cli_ReadSZLList(S7Object Client, void *pUsrData, int *ItemsCount);

// PLC control functions
int S7API Cli_PlcHotStart(S7Object Client);
int S7API Cli_PlcColdStart(S7Object Client);
int S7API Cli_PlcStop(S7Object Client);
int S7API Cli_CopyRamToRom(S7Object Client, int Timeout);
int S7API Cli_Compress(S7Object Client, int Timeout);
int S7API Cli_GetPlcStatus(S7Object Client, int *Status);

// Security functions
int S7API Cli_SetSessionPassword(S7Object Client, char *Password);
int S7API Cli_ClearSessionPassword(S7Object Client);
int S7API Cli_GetProtection(S7Object Client, void *pUsrData);

// Properties
int S7API Cli_GetConnected(S7Object Client, int *Connected);
int S7API Cli_GetPduLength(S7Object Client, int *Requested, int *Negotiated);
int S7API Cli_GetCpuInfo(S7Object Client, void *pUsrData);
int S7API Cli_GetExecTime(S7Object Client, int *Time);
int S7API Cli_GetLastError(S7Object Client, int *LastError);
int S7API Cli_ErrorText(int Error, char *Text, int TextLen);

// Helper functions to access data
int S7API Cli_GetBitAt(void *pData, int Pos, int Bit);
byte S7API Cli_GetByteAt(void *pData, int Pos);
word S7API Cli_GetWordAt(void *pData, int Pos);
longword S7API Cli_GetDWordAt(void *pData, int Pos);
float S7API Cli_GetRealAt(void *pData, int Pos);

void S7API Cli_SetBitAt(void *pData, int Pos, int Bit, int Value);
void S7API Cli_SetByteAt(void *pData, int Pos, byte Value);
void S7API Cli_SetWordAt(void *pData, int Pos, word Value);
void S7API Cli_SetDWordAt(void *pData, int Pos, longword Value);
void S7API Cli_SetRealAt(void *pData, int Pos, float Value);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // snap7_h


