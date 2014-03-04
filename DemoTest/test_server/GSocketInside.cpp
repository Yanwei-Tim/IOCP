
#include "stdafx.h"
#include <winsock2.h>
#include <mswsock.h>

#include "GLog.h"
#include "GMemory.h"
#include "GWorkerThread.h"
#include "GPerIoData.h"
#include "GPerHandleData.h"
#include "GTcpClient.h"
#include "GTcpServer.h"
#include "GThread.h"
#include "GCommProtocol.h"
#include "GProcesserThread.h"
#include "GSocketInside.h"
#include "GSocket.h"

char* GSock_GetVersion(void)
{
	return("0.8.5000.0");
}

/*********************************************************************************
                   通用函数
*********************************************************************************/
BOOL bIsStartup = FALSE;
void GSock_GetLocalNetIp(char *szIp, int nSize, BOOL bIsIntnetIp)
{
	char host_name[255];
	int i;
	struct hostent *phe;

	*szIp = 0;

	if(!bIsStartup)
	{
		bIsStartup = TRUE;
		WSADATA wsd;
		if(WSAStartup(0x0101, &wsd))
			return;
	}
	
	if(SOCKET_ERROR == gethostname(host_name, sizeof(host_name)))
		return;
	phe = gethostbyname(host_name);
	if(NULL == phe)
		return;

	if(bIsIntnetIp)
	{
		for (i = 0; 0 != phe->h_addr_list[i]; i++)
			strcpy_s(szIp, nSize, inet_ntoa(*(in_addr *)phe->h_addr_list[i]));
		
	}else
		strcpy_s(szIp, nSize, inet_ntoa(*(in_addr *)phe->h_addr_list[0]));
}

void GSock_AddrToIp(char *szIp, int nSize, DWORD dwAddr)
{
	strcpy_s(szIp, nSize, inet_ntoa(*(in_addr *)&dwAddr));
}

void* GSock_WSAGetExtensionFunctionPointer(SOCKET Socket, GUID ExFuncGuid)
{
	DWORD dwOut;
	void *pResult;
	WSAIoctl(Socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &ExFuncGuid, sizeof(GUID), &pResult, sizeof(void *), &dwOut, NULL, NULL);

	return(pResult);
}

/*********************************************************************************
                   默认事件处理
*********************************************************************************/
#if(_GSOCKET_FUNC_TCP_SERVER)
void OnGSockConnectTcpSvrDef(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
}
#endif

#if(_GSOCKET_FUNC_UDP_SERVER)
void OnGSockConnectUdpSvrDef(const DWORD pdwClientContext, const char* pBuf, const DWORD dwBytes)
{
}
#endif//#if(_GSOCKET_FUNC_UDP_SERVER)


#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_UDP_SERVER)
void OnGSockDisconnectSvrDef(const DWORD dwClientContext)
{
}

void OnGSockReceiveSvrDef(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
}

void OnGSockSendedSvrDef(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
}

void OnGSockSendErrorSvrDef(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
}

void OnGSockConnectionOverflowDef(const DWORD dwClientContext)
{
}
#endif//#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_UDP_SERVER)


#if(_GSOCKET_FUNC_TCP_CLIENT || _GSOCKET_FUNC_UDP_CLIENT)
void OnGSockConnectCltDef(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
}

void OnGSockDisconnectCltDef(const DWORD dwClientContext)
{
}

void OnGSockReceiveCltDef(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{

}

void OnGSockSendedCltDef(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{

}

void OnGSockSendErrorCltDef(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
}
#endif//#if(_GSOCKET_FUNC_TCP_CLIENT || _GSOCKET_FUNC_UDP_CLIENT)


#if(_GSOCKET_FUNC_TCP_CLIENT)
void OnGSockConnectErrorDef(const DWORD dwClientContext)
{
}
void OnGSockCreateClientDef(const DWORD dwClientContext)
{
}

void OnGSockDestroyClientDef(const DWORD dwClientContext)
{
}

void OnGSockCreateClientInfoDef(const DWORD dwClientContext)
{
}

void OnGSockDestroyClientInfoDef(const DWORD dwClientContext)
{
}
#endif


void OnGSockIdleOvertimeDef(const DWORD dwClientContext)
{
}

void OnGSockHeartbeatDef(const DWORD dwClientContext)
{
}

void OnGSockProfessionalWorkDef(const DWORD dwClientContext)
{
}

/*********************************************************************************
                   变量定义和初始化
*********************************************************************************/
#if(_GSOCKET_FUNC_TCP_SERVER)
PFN_ON_GHND_DATA_EVENT					pfnOnGSockConnectTcpSvr = OnGSockConnectTcpSvrDef;
#endif

#if(_GSOCKET_FUNC_UDP_SERVER)
PFN_ON_GHND_DATA_EVENT					pfnOnGSockConnectUdpSvr = OnGSockConnectUdpSvrDef;
#endif

#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_UDP_SERVER)
PFN_ON_GHND_EVENT					pfnOnGSockDisconnectSvr = OnGSockDisconnectSvrDef;
PFN_ON_GHND_DATA_EVENT				pfnOnGSockReceiveSvr = OnGSockReceiveSvrDef;
PFN_ON_GHND_DATA_EVENT				pfnOnGSockSendedSvr = OnGSockSendedSvrDef;
PFN_ON_GHND_DATA_EVENT				pfnOnGSockSendErrorSvr = OnGSockSendErrorSvrDef;
PFN_ON_GHND_EVENT					pfnOnGSockConnectionOverflow = OnGSockConnectionOverflowDef;
#endif

#if(_GSOCKET_FUNC_TCP_CLIENT || _GSOCKET_FUNC_UDP_CLIENT)
PFN_ON_GHND_DATA_EVENT				pfnOnGSockConnectClt = OnGSockConnectCltDef;
PFN_ON_GHND_EVENT					pfnOnGSockDisconnectClt = OnGSockDisconnectCltDef;
PFN_ON_GHND_DATA_EVENT				pfnOnGSockReceiveClt = OnGSockReceiveCltDef;
PFN_ON_GHND_DATA_EVENT				pfnOnGSockSendedClt = OnGSockSendedCltDef;
PFN_ON_GHND_DATA_EVENT				pfnOnGSockSendErrorClt = OnGSockSendErrorCltDef;
#endif

#if(_GSOCKET_FUNC_TCP_CLIENT)
PFN_ON_GHND_EVENT					pfnOnGSockConnectError = OnGSockConnectErrorDef;

PFN_ON_GHND_EVENT					pfnOnGSockCreateClient = OnGSockCreateClientDef;
PFN_ON_GHND_EVENT					pfnOnGSockDestroyClient = OnGSockDestroyClientDef;

PFN_ON_GHND_EVENT					pfnOnGSockCreateClientInfo = OnGSockCreateClientInfoDef;
PFN_ON_GHND_EVENT					pfnOnGSockDestroyClientInfo = OnGSockDestroyClientInfoDef;
#endif

PFN_ON_GHND_EVENT					pfnOnGSockIdleOvertime = OnGSockIdleOvertimeDef;
PFN_ON_GHND_EVENT					pfnOnGSockHeartbeat = OnGSockHeartbeatDef;
PFN_ON_GHND_EVENT					pfnOnGSockProfessionalWork= OnGSockProfessionalWorkDef;

#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_UDP_SERVER)
DWORD						dwGSockMaxNumberConnection = 40000/2;
#endif

#if(_GSOCKET_FUNC_TCP_SERVER)
DWORD						dwGSockNumberPostAccept = 0;
DWORD						dwGSockTimeAcceptOvertime = 5;
BOOL						bGSockIsZeroByteAccept = FALSE;
DWORD						dwGSockAcceptBytes = 0;
#endif

#if(_GSOCKET_FUNC_TCP_CLIENT)
DWORD						dwGSockTimeAutoConnect = 5 * 1000;
#endif
DWORD						dwGSockNumberPostRecv = 0;
DWORD						dwGSockTimeHeartbeat = 30 * 1000;
DWORD						dwGSockTimeLoginOvertime = 10 * 1000;
DWORD						dwGSockTimeProfessionalWork = 30 * 1000;
DWORD						dwGSockTimeIdleOvertime = 60 * 1000;

BOOL						bGSockIsZeroByteRecv = FALSE;
DWORD						dwGSockRecvBytes = 0;

BOOL						bGSockIsActive = FALSE;
BOOL						bGSock_SocketOptNodelay = TRUE;
int							nGSock_SocketOptSendBuf = -1;
int							nGSock_SocketOptRecvBuf = -1;

LPFN_ACCEPTEX				pfnGSockAcceptEx = NULL;
LPFN_GETACCEPTEXSOCKADDRS	pfnGSockGetAcceptExSockAddrs = NULL;
LPFN_CONNECTEX				pfnGSockConnectEx = NULL;
LPFN_DISCONNECTEX			pfnGSockDisconnectEx = NULL;
LPFN_TRANSMITFILE			pfnGSockTransmitFile = NULL;
LPFN_TRANSMITPACKETS		pfnGSockTransmitPackets = NULL;

/*********************************************************************************
                变量设置与获取
*********************************************************************************/
#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_UDP_SERVER)
DWORD GSock_GetMaxNumberConnection(void)
{
	return(dwGSockMaxNumberConnection );
}

void GSock_SetMaxNumberConnection(DWORD dwNumber)
{
	if(bGSockIsActive || (!dwNumber))
		return;
	dwGSockMaxNumberConnection = dwNumber;
}
#endif//#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_UDP_SERVER)

#if(_GSOCKET_FUNC_TCP_SERVER)
DWORD GSock_GetTimeAcceptOvertime(void)
{
	return(dwGSockTimeAcceptOvertime);
}

void GSock_SetTimeAcceptOvertime(DWORD dwTime)
{
	if(bGSockIsActive || (!dwTime))
		return;
	dwGSockTimeAcceptOvertime = dwTime;
}

DWORD GSock_GetNumberPostAccept(void)
{
	return(dwGSockNumberPostAccept);
}

void GSock_SetNumberPostAccept(DWORD dwNumber)
{
	if(bGSockIsActive || (!dwNumber))
		return;
	dwGSockNumberPostAccept = dwNumber;
}

BOOL GSock_IsZeroByteAccept(void)
{
	return(bGSockIsZeroByteAccept);
}

void GSock_SetZeroByteAccept(BOOL bIsZeroBuffer)
{
	if(bGSockIsActive)
		return;
	bGSockIsZeroByteAccept = bIsZeroBuffer;
	if(bGSockIsZeroByteAccept)
		dwGSockAcceptBytes = 0;
	else
		dwGSockAcceptBytes = GIoDat_GetGBufSize() - ((sizeof(SOCKADDR_IN) + 16) * 2);
}
#endif//#if(_GSOCKET_FUNC_TCP_SERVER)

DWORD GSock_GetTimeIdleOvertime(void)
{
	return(dwGSockTimeIdleOvertime / 1000);
}

void GSock_SetTimeIdleOvertime(DWORD dwTime)
{
	if(bGSockIsActive)
		return;
	dwGSockTimeIdleOvertime = dwTime * 1000;
}

DWORD GSock_GetTimeHeartbeat(void)
{
	return(dwGSockTimeHeartbeat / 1000);
}

void GSock_SetTimeHeartbeat(DWORD dwTime)
{
	if(bGSockIsActive)
		return;
	dwGSockTimeHeartbeat = dwTime * 1000;
}

DWORD GSock_GetTimeProfessionalWork(void)
{
	return(dwGSockTimeProfessionalWork / 1000);
}

void GSock_SetTimeProfessionalWork(DWORD dwTime)
{
	if(bGSockIsActive)
		return;
	dwGSockTimeProfessionalWork = dwTime * 1000;
}

DWORD GSock_GetNumberPostRecv(void)
{
	return(dwGSockNumberPostRecv);
}

void GSock_SetNumberPostRecv(DWORD dwNumber)
{
	if(bGSockIsActive || (!dwNumber))
		return;
	dwGSockNumberPostRecv = dwNumber;
}

BOOL GSock_IsZeroByteRecv(void)
{
	return(bGSockIsZeroByteRecv);
}

void GSock_SetZeroByteRecv(BOOL bIsZeroBuffer)
{
	if(bGSockIsActive)
		return;
	bGSockIsZeroByteRecv = bIsZeroBuffer;
	if(bGSockIsZeroByteRecv)
		dwGSockRecvBytes = 0;
	else
		dwGSockRecvBytes = GIoDat_GetGBufSize();
}

BOOL GSock_IsActive(void)
{
	return(bGSockIsActive);
}

void GSock_SetSocketSendBufSize(int nSize)
{
	if(bGSockIsActive)
	{
		return;
	}
	nGSock_SocketOptSendBuf = nSize;
}

void GSock_SetSocketRecvBufSize(int nSize)
{
	if(bGSockIsActive)
	{
		return;
	}
	nGSock_SocketOptRecvBuf = nSize;
}

void GSock_SetMtuNodelay(BOOL bNodelay)
{
	if(bGSockIsActive)
	{
		return;
	}
	bGSock_SocketOptNodelay = bNodelay;
}

/*********************************************************************************
                事件处理函数设置
*********************************************************************************/
#if(_GSOCKET_FUNC_TCP_SERVER)
void GSock_SetOnConnectProcTcpSvr(PFN_ON_GHND_DATA_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockConnectTcpSvr = pfnOnProc;
}
#endif

#if(_GSOCKET_FUNC_UDP_SERVER)
void GSock_SetOnConnectProcUdpSvr(PFN_ON_GHND_DATA_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockConnectUdpSvr = pfnOnProc;
}
#endif

#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_UDP_SERVER)
void GSock_SetOnDisconnectProcSvr(PFN_ON_GHND_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockDisconnectSvr = pfnOnProc;
}

void GSock_SetOnReceiveProcSvr(PFN_ON_GHND_DATA_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockReceiveSvr = pfnOnProc;
}

void GSock_SetOnSendedProcSvr(PFN_ON_GHND_DATA_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockSendedSvr = pfnOnProc;
}

void GSock_SetOnSendErrorProcSvr(PFN_ON_GHND_DATA_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockSendErrorSvr = pfnOnProc;
}

void GSock_SetOnConnectionOverflowProc(PFN_ON_GHND_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockConnectionOverflow = pfnOnProc;
}
#endif//#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_UDP_SERVER)

#if(_GSOCKET_FUNC_TCP_CLIENT || _GSOCKET_FUNC_UDP_CLIENT)
void GSock_SetOnConnectProcClt(PFN_ON_GHND_DATA_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockConnectClt = pfnOnProc;
}

void GSock_SetOnDisconnectProcClt(PFN_ON_GHND_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockDisconnectClt = pfnOnProc;
}

void GSock_SetOnReceiveProcClt(PFN_ON_GHND_DATA_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockReceiveClt = pfnOnProc;
}

void GSock_SetOnSendedProcClt(PFN_ON_GHND_DATA_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockSendedClt = pfnOnProc;
}

void GSock_SetOnSendErrorProcClt(PFN_ON_GHND_DATA_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockSendErrorClt = pfnOnProc;
}
#endif//#if(_GSOCKET_FUNC_TCP_CLIENT || _GSOCKET_FUNC_UDP_CLIENT)

#if(_GSOCKET_FUNC_TCP_CLIENT)
void GSock_SetOnConnectErrorProc(PFN_ON_GHND_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockConnectError = pfnOnProc;
}

void GSock_SetOnCreateClientProc(PFN_ON_GHND_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockCreateClient = pfnOnProc;
}

void GSock_SetOnDestroyClientProc(PFN_ON_GHND_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockDestroyClient = pfnOnProc;
}
#endif//#if(_GSOCKET_FUNC_TCP_CLIENT)

void GSock_SetOnIdleOvertimeProc(PFN_ON_GHND_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockIdleOvertime = pfnOnProc;
}

void GSock_SetOnHeartbeatProc(PFN_ON_GHND_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockHeartbeat = pfnOnProc;
}

void GSock_SetOnProfessionalWorkProc(PFN_ON_GHND_EVENT pfnOnProc)
{
	if(pfnOnProc)
		pfnOnGSockProfessionalWork = pfnOnProc;
}
/*********************************************************************************
                   Socket操作
*********************************************************************************/
DWORD GSock_GetClientCount(void)
{
	DWORD Result = 0;
	#if(_GSOCKET_FUNC_TCP_SERVER)
	Result += GTcpSvr_GetClientCount();
	#endif
	#if(_GSOCKET_FUNC_TCP_CLIENT)
	Result += GTcpClt_GetClientCount();
	#endif
	#if(_GSOCKET_FUNC_UDP_SERVER)
	Result += GUdps_GetClientCount();
	#endif
	#if(_GSOCKET_FUNC_UDP_CLIENT)
	Result += GUdpc_GetClientCount();
	#endif

	return(Result);
}

void GSock_CloseClient(DWORD dwClientContext)
{
	if(GHND_TYPE_TCP_SVR_CLIENT == PGHND_DATA(dwClientContext)->htType)
			GSock_CloseClient(dwClientContext);
}

#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_TCP_CLIENT)
BOOL GSock_InitTcpHndData(PGHND_DATA pHndData)
{
	ULONG nNoBlock = 1;

	pHndData->Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(INVALID_SOCKET == pHndData->Socket)
	{
		GLog_Write("GSock_InitTcpHndData：创建Socket句柄失败");
		return(FALSE);
	}

	if((SOCKET_ERROR == ioctlsocket(pHndData->Socket, (LONG)FIONBIO, &nNoBlock)) ||
		(SOCKET_ERROR == setsockopt(pHndData->Socket, IPPROTO_TCP, TCP_NODELAY, (char *)(&bGSock_SocketOptNodelay), sizeof(bGSock_SocketOptNodelay))) ||		//不使用Nagle算法，不会将小包进行拼接成大包再进行发送，直接将小包发送出去，会使得小包时候用户体验非常好。
		(NULL == CreateIoCompletionPort((HANDLE)pHndData->Socket, hGWkrThrdCompletionPort, (DWORD)pHndData, 0)))
	{
		closesocket(pHndData->Socket);
		return(FALSE);
	}
	if(-1 < nGSock_SocketOptSendBuf)
		setsockopt(pHndData->Socket, SOL_SOCKET, SO_SNDBUF, (const char *)&nGSock_SocketOptSendBuf, sizeof(nGSock_SocketOptSendBuf));
	if(-1 < nGSock_SocketOptRecvBuf)
		setsockopt(pHndData->Socket, SOL_SOCKET, SO_RCVBUF, (const char *)&nGSock_SocketOptRecvBuf, sizeof(nGSock_SocketOptRecvBuf));

	BOOL bDontLinger = FALSE;
	setsockopt(pHndData->Socket, SOL_SOCKET, SO_DONTLINGER, (const char *) &bDontLinger, sizeof(BOOL));	//close socket to linger
	LINGER Linger;
	Linger.l_onoff = 1;
	Linger.l_linger = 0;
	setsockopt(pHndData->Socket, SOL_SOCKET, SO_LINGER, (const char *) &Linger, sizeof(LINGER));

	#if(_REUSED_SOCKET)
	BOOL bReused = TRUE;
	setsockopt(pHndData->Socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &bReused, sizeof(BOOL));	//Port Reuse
	#endif

	GUID Guid1 = WSAID_ACCEPTEX;
		GUID Guid2 = WSAID_DISCONNECTEX;
		GUID Guid3 = WSAID_GETACCEPTEXSOCKADDRS;
		GUID Guid4 = WSAID_CONNECTEX;
		GUID Guid5 = WSAID_TRANSMITFILE;
		GUID Guid6 = WSAID_TRANSMITPACKETS;

	if(!pfnGSockAcceptEx)
	{
		
		pfnGSockAcceptEx = (LPFN_ACCEPTEX)GSock_WSAGetExtensionFunctionPointer(pHndData->Socket, Guid1);
		pfnGSockDisconnectEx = (LPFN_DISCONNECTEX)GSock_WSAGetExtensionFunctionPointer(pHndData->Socket, Guid2);
		pfnGSockGetAcceptExSockAddrs = (LPFN_GETACCEPTEXSOCKADDRS)GSock_WSAGetExtensionFunctionPointer(pHndData->Socket, Guid3);
		pfnGSockConnectEx = (LPFN_CONNECTEX)GSock_WSAGetExtensionFunctionPointer(pHndData->Socket, Guid4);
		pfnGSockTransmitFile = (LPFN_TRANSMITFILE)GSock_WSAGetExtensionFunctionPointer(pHndData->Socket, Guid5);
		pfnGSockTransmitPackets = (LPFN_TRANSMITPACKETS)GSock_WSAGetExtensionFunctionPointer(pHndData->Socket, Guid6);
	}

	/*DWORD dwBytes;
	WSAIoctl(pHndData->Socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
                   &Guid1,
                    sizeof(GUID),
                   &pHndData->pfnExFunc1,
                    sizeof(pHndData->pfnExFunc1),
                   &dwBytes,
                    NULL,
                    NULL
                    );
	WSAIoctl(pHndData->Socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
                   &Guid2,
                    sizeof(GUID),
                   &pHndData->pfnExFunc2,
                    sizeof(pHndData->pfnExFunc2),
                   &dwBytes,
                    NULL,
                    NULL
                    );*/

	return(TRUE);
}

void GSock_UninitTcpHndData(PGHND_DATA pHndData)
{
	shutdown(pHndData->Socket, SD_BOTH);
	closesocket(pHndData->Socket);
}
#endif//#if(_GSOCKET_FUNC_TCP_SERVER || _GSOCKET_FUNC_TCP_CLIENT)

#if(_GSOCKET_FUNC_UDP_SERVER || _GSOCKET_FUNC_UDP_CLIENT)
BOOL GSock_InitUdpHndData(PGHND_DATA pHndData)
{
	return(TRUE);
}

void GSock_UninitUdpHndData(PGHND_DATA pHndData)
{
	closesocket(pHndData->Socket);
}
#endif//#if(_GSOCKET_FUNC_UDP_SERVER || _GSOCKET_FUNC_UDP_CLIENT)

/*********************************************************************************
                  GSocket服务开始结束
*********************************************************************************/
void GSock_StartService(void)
{
	if(bGSockIsActive)
		return;

	if(GHndDat_GetTotal() < dwGSockMaxNumberConnection + dwGSockMaxNumberConnection * 0.1)
		GHndDat_SetTotal(dwGSockMaxNumberConnection + DWORD(dwGSockMaxNumberConnection * 0.1));

	#if(_GSOCKET_FUNC_TCP_SERVER)
	if(!dwGSockNumberPostAccept)
		dwGSockNumberPostAccept = GWkrThrd_GetWorkerThreadNumber();
	if(!dwGSockNumberPostRecv)
		dwGSockNumberPostRecv = GWkrThrd_GetWorkerThreadNumber();
	if(bGSockIsZeroByteAccept)
		dwGSockAcceptBytes = 0;
	else
		dwGSockAcceptBytes = GIoDat_GetGBufSize() - ((sizeof(SOCKADDR_IN) + 16) * 2);
	#endif

	if(!dwGSockNumberPostRecv)
		dwGSockNumberPostRecv = GWkrThrd_GetWorkerThreadNumber();
	if(bGSockIsZeroByteRecv)
		dwGSockRecvBytes = 0;
	else
		dwGSockRecvBytes = GIoDat_GetGBufSize();

	WSADATA wsd;
	if(WSAStartup(0x0101, &wsd))
		return;

	GCommProt_Init();
	GThrd_Create();//Create Thread Pool

	GMem_Create();
	if(!GMem_IsActive())
	{
		GLog_Write("GSock_StartService：GMem创建失败，请确保有足够内存，或进行用户权限设置后重新启动Windows：控制面板->管理工具->本地安全策略->用户权利指派->内存中锁定页");
		return;
	}	

	GIoDat_Create();
	if(!bGIoDataIsActive)
	{			
		GMem_Destroy();
		return;
	}

	#if(_USE_PROCESS_THREAD)
	GProcThrd_Create();
	if(!bGProcThrdThreadIsActive)
	{
		GIoDat_Destroy();
		GMem_Destroy();
		return;
	}
	#endif

	GWkrThrd_Create();
	if(!bGWkrThrdIsActive)
	{
		GIoDat_Destroy();
		#if(_USE_PROCESS_THREAD)
		GProcThrd_Destroy();
		#endif
		GMem_Destroy();
		return;
	}

	#if(_REUSED_SOCKET)
	GHndDat_Create(GSock_InitTcpHndData);//所有HndData初始化并完成端口绑定SOCKET（初始化）
	#else
	GHndDat_Create(NULL);
	#endif
	extern BOOL bGHndDataIsActive;
	if(!bGHndDataIsActive)
	{
		
		GWkrThrd_Destroy();
		#if(_USE_PROCESS_THREAD)
		GProcThrd_Destroy();
		#endif
		GIoDat_Destroy();
		GMem_Destroy();
		return;
	}
	
	#if(_GSOCKET_FUNC_TCP_SERVER)
	GTcpSvr_Create();
	if(!bGTcpSvrIsActive)
	{
		GWkrThrd_Destroy();
		#if(_USE_PROCESS_THREAD)
		GProcThrd_Destroy();
		#endif
		GIoDat_Destroy();
		#if(_REUSED_SOCKET)
		GHndDat_Destroy(GSock_UninitTcpHndData);
		#else
		GHndDat_Destroy(NULL);
		#endif
		GMem_Destroy();
		return;
	}
	#endif

	#if(_GSOCKET_FUNC_TCP_CLIENT)
	GTcpClt_Create();
	if(!bGTcpCltIsActive)
	{
		GWkrThrd_Destroy();
		#if(_USE_PROCESS_THREAD)
		GProcThrd_Destroy();
		#endif
		GIoDat_Destroy();
		#if(_REUSED_SOCKET)
		GHndDat_Destroy(GSock_UninitTcpHndData);
		#else
		GHndDat_Destroy(NULL);
		#endif
		GTcpSvr_Destroy();
		GMem_Destroy();
		return;
	}
	#endif

	bGSockIsActive = TRUE;
}

void GSock_StopService(void)
{

	if(!bGSockIsActive)
		return;

	bGSockIsActive = FALSE;
	bGHndDataIsActive = FALSE;
	bGIoDataIsActive = FALSE;
	bGWkrThrdIsActive = FALSE;
	#if(_GSOCKET_FUNC_TCP_SERVER)
	bGTcpSvrIsActive = FALSE;
	#endif
	#if(_GSOCKET_FUNC_TCP_CLIENT)
	bGTcpCltIsActive = FALSE;
	#endif
	#if(_GSOCKET_FUNC_UDP_SERVER)
	bGUdpsIsActive = FALSE;
	#endif
	#if(_GSOCKET_FUNC_UDP_CLIENT)
	bGUdpcIsActive = FALSE;
	#endif	

	#if(_GSOCKET_FUNC_UDP_CLIENT)
	//...
	#endif

	#if(_GSOCKET_FUNC_TCP_CLIENT)
	GTcpClt_EndThread();
	GTcpClt_CloseClients();
	GTcpClt_FreeClients();
	#endif

	#if(_GSOCKET_FUNC_UDP_SERVER)
	//...
	#endif

	#if(_GSOCKET_FUNC_TCP_SERVER)
	GTcpSvr_CloseListeners();
	GTcpSvr_EndThread();
	GTcpSvr_CloseListenerEvents();
	GTcpSvr_CloseClients();
	GLog_Write("GSock_StopService：等待TcpServer客户完全断开");
	while(GTcpSvr_GetClientCount())
		Sleep(50);
	GTcpSvr_Destroy();
	#endif

	GLog_Write("GSock_StopService：等待句柄完全释放");
	while(GHndDat_GetUsed())
		Sleep(50);
	GLog_Write("GSock_StopService：等待IO数据完全释放");
	while(GIoDat_GetUsed())
		Sleep(50);

	GLog_Write("GSock_StopService：摧毁Iocp模块");
	GWkrThrd_Destroy();
	GLog_Write("GSock_StopService：摧毁Process模块");
	#if(_USE_PROCESS_THREAD)
	GProcThrd_Destroy();
	#endif
	GLog_Write("GSock_StopService：摧毁IoData模块");
	GIoDat_Destroy();
	GLog_Write("GSock_StopService：摧毁HndData模块");
	#if(_REUSED_SOCKET)
	GHndDat_Destroy(GSock_UninitTcpHndData);
	#else
	GHndDat_Destroy(NULL);
	#endif
	GLog_Write("GSock_StopService：摧毁GMem模块");
	GMem_Destroy();

	GThrd_Destroy();

	//关闭已经打开的底层服务
	WSACleanup();
}

