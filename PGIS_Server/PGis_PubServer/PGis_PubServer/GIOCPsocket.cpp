#include "stdafx.h"
#include "GIOCPsocket.h"
#include "../../IOCP/ListManager.h"

int Random(int nNumber) 
{ 
	return (int)(nNumber/(float)RAND_MAX * rand());
}


GIOCPsocket::GIOCPsocket(void)
{
	char szIp[30];
	GSock_GetLocalNetIp(szIp, 28, FALSE);
	m_strLocalIp.Format(_T("%s"), szIp);

	m_nSocketBufSize = 8192;	//���û����� ҳ���ڴ��С

	m_dwWorkerThreadNumber = GWkrThrd_GetWorkerThreadNumberDef();		//�����߳���
	m_dwConcurrentThreadNumber = GWkrThrd_GetWorkerThreadNumberDef();	//��ɶ˿�֧���߳�

	m_dwPostRecvNumber = GWkrThrd_GetWorkerThreadNumberDef();
	m_dwPostAcceptNumber = GWkrThrd_GetWorkerThreadNumberDef();

	m_dwPackSize = GIoDat_GetGBufSize() - 2 * (sizeof(SOCKADDR_IN) + 16);
	m_dwHeartbeat = 30;

	m_dwSendBytes = 0;
	m_dwRecvBytes = 0;
	m_dwByteTotal = 0;
	m_dwLong = 0;
	m_fBeat = 0;
	m_bCloseMtu = TRUE;
	m_bZeroAccept = FALSE;
	m_bZeroRecv = FALSE;
	m_bSendSelf = TRUE;
	m_bRandoDisconnect = FALSE;
	m_bShutdownSend = FALSE;

	m_dwGMemPageSize = GMem_GetPageSize();
	m_dwGMemTotal = GMem_GetTotalBytes();

	m_dwGBufSize = GIoDat_GetGBufSize();
	m_dwIoDataTotal = GIoDat_GetTotal();
	m_dwHndDataTotal = GHndDat_GetTotal();

	m_dwWorkerThreadNumber = GWkrThrd_GetWorkerThreadNumberDef();
	m_dwConcurrentThreadNumber = GWkrThrd_GetWorkerThreadNumberDef();

	m_dwAcceptOvertime = GSock_GetTimeAcceptOvertime();
	m_dwIdleOvertime = GSock_GetTimeIdleOvertime();
	m_dwMaxConnection = GSock_GetMaxNumberConnection();
	m_dwHeartbeat = GSock_GetTimeHeartbeat();
}

GIOCPsocket::~GIOCPsocket(void)
{
}


void GIOCPsocket::GetGSocketInfo(void)
{
	m_dwGMemPageSize = GMem_GetPageSize();
	m_dwGMemTotal = GMem_GetTotalBytes();

	m_dwGBufSize = GIoDat_GetGBufSize();

	m_dwIoDataTotal = GIoDat_GetTotal();
	m_dwHndDataTotal = GHndDat_GetTotal();

	m_dwWorkerThreadNumber = GWkrThrd_GetWorkerThreadNumber();
	m_dwConcurrentThreadNumber = GWkrThrd_GetConcurrentThreadNumber();

	m_dwAcceptOvertime = GSock_GetTimeAcceptOvertime();
	m_dwIdleOvertime = GSock_GetTimeIdleOvertime();
	m_dwMaxConnection = GSock_GetMaxNumberConnection();

	m_dwHeartbeat = GSock_GetTimeHeartbeat();
	m_dwTimeAutoConnect = GSock_GetTimeAutoConnect();
}

BOOL GIOCPsocket::OnInitSocket()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(GSock_IsActive())
	{
		//KillTimer(10001);		
		m_bShutdownSend = TRUE;
		GSock_StopService();
		return FALSE;
	}
	else
	{		
		if(m_dwPackSize > GIoDat_GetGBufSize() - 2 * (sizeof(SOCKADDR_IN) + 16))
			m_dwPackSize = GIoDat_GetGBufSize() - 2 * (sizeof(SOCKADDR_IN) + 16);

		GMem_SetPageSize(m_dwGMemPageSize);	//�����ڴ�ҳ��С ����С����С�ڴ�ҳ���Զ�����
		GMem_SetTotalBytes(m_dwGMemTotal);	//�����ڴ��С ������С����С�ڴ棬�Զ�����

		GIoDat_SetGBufSize(m_dwGBufSize);
		GIoDat_SetTotal(m_dwIoDataTotal);

		GHndDat_SetTotal(m_dwHndDataTotal);

		GSock_SetMtuNodelay(m_bCloseMtu);
		GSock_SetZeroByteRecv(m_bZeroRecv);			//�Ƿ����0����
		GSock_SetZeroByteAccept(m_bZeroAccept);		//�Ƿ�֧��0����ACCEPT

		GSock_SetSocketSendBufSize(m_nSocketBufSize);	//���û�����
		GSock_SetSocketRecvBufSize(m_nSocketBufSize);	//���û�����

		GWkrThrd_SetWorkerThreadNumber(m_dwWorkerThreadNumber);			//�����߳���
		GWkrThrd_SetConcurrentThreadNumber(m_dwConcurrentThreadNumber);	//��ɶ˿�֧���߳���

		GSock_SetTimeIdleOvertime(m_dwIdleOvertime);		//CLIENT SERVER ���ӳ�ʱ
		//----------------------------server--------------------------------------
		GSock_SetMaxNumberConnection(m_dwMaxConnection);	
		GSock_SetNumberPostAccept(m_dwPostAcceptNumber);	//SERVER ���Ͷ��AcceptEx������
		GSock_SetNumberPostRecv(m_dwPostRecvNumber);		//���� CLIENT OnConnected  SERVER OnAccept Ͷ�� WSRECV IODATA ���� 		
		GSock_SetTimeAcceptOvertime(m_dwAcceptOvertime);	

		//-----------------------------client-------------------------------------
		GSock_SetTimeHeartbeat(m_dwHeartbeat);				//��������ʱ�䣬�����ʱ������ͻ��˷�����������
		GSock_SetTimeAutoConnect(m_dwTimeAutoConnect);		//Client ����	

		GSock_SetOnConnectProcTcpSvr(&OnConnectSvr);
		GSock_SetOnDisconnectProcSvr(&OnDisconnectSvr);
		GSock_SetOnReceiveProcSvr(&OnReceiveSvr);
		GSock_SetOnSendedProcSvr(&OnSendedSvr);
		GSock_SetOnSendErrorProcSvr(&OnSendErrorSvr);
		GSock_SetOnIdleOvertimeProc(&OnIdleOvertimeSvr);
		GSock_SetOnConnectionOverflowProc(&OnConnectionOverflowSvr);
		GSock_SetUnknownErrorProc(&OnUnknownErrorSvr);
		GSock_SetOnCreateClientSvrProc(&OnCreateClientSvr);
		GSock_SetOnDestroyClientSvrProc(&OnDestroyClientSvr);

		GSock_SetOnConnectProcClt(&OnConnectClt);
		GSock_SetOnConnectErrorProc(&OnConnectErrorClt);
		GSock_SetOnDisconnectProcClt(&OnDisconnectClt);
		GSock_SetOnReceiveProcClt(&OnReceiveClt);
		GSock_SetOnSendedProcClt(&OnSendedClt);
		GSock_SetOnSendErrorProcClt(&OnSendErrorClt);
		GSock_SetOnHeartbeatProc(&OnHeartbeatClt);
		GSock_SetOnCreateClientCltProc(&OnCreateClientClt);
		GSock_SetOnDestroyClientCltProc(&OnDestroyClientClt);

		GSock_StartService();
		if(!GSock_IsActive())
		{
			TRACE("GSock_StartService ERROR");
			return FALSE;
		}

		TRACE("GSock_StartService SUCEESE");
		return TRUE;
	}
}

void GIOCPsocket::OnUninitSocket()
{
	m_bShutdownSend = TRUE;
	GSock_StopService();
}

BOOL GIOCPsocket::OnStartIOCPSocket(DWORD dwMaxConnection_SVR,		//�����������Ĭ�Ͻ����޸��������������
									DWORD dwAcceptOvertime_SVR,		//���ݽ��ܳ�ʱ��ACCEPT�󣬱����һ��������������������ݣ����򱻶Ͽ�	
									DWORD dwIdleOvertime_SVR,		//���ܳ�ʱ���ͻ������Ӻ󣬱��������ʱ������������������ݣ����򱻶Ͽ�
									DWORD dwHeartbeat_CLR,			//�ͻ��˷���������
									DWORD dwTimeAutoConnect_CLR,	//�ͻ����Զ�����
									DWORD dwIoDataBufSize_SOCKET,	//��������Buffer
									int	  nSocketBufSize_SOCKET)
{
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);	
	DWORD dwThreadNum = SystemInfo.dwNumberOfProcessors*2 + 2;	//Ϊ�����߳�����

	m_dwMaxConnection = dwMaxConnection_SVR;
	m_dwAcceptOvertime = dwAcceptOvertime_SVR;
	m_dwIdleOvertime = dwIdleOvertime_SVR;
	m_dwHeartbeat = dwHeartbeat_CLR;
	m_dwTimeAutoConnect = dwTimeAutoConnect_CLR;
	m_dwGBufSize = dwIoDataBufSize_SOCKET;
	m_dwIoDataTotal = ((dwThreadNum * dwMaxConnection_SVR * 1.1) > DWORD(-1)) ? DWORD(-1):(dwThreadNum * dwMaxConnection_SVR * 1.1);
	m_dwHndDataTotal = dwMaxConnection_SVR + ((dwMaxConnection_SVR*0.1>1000)?dwMaxConnection_SVR*0.1:1000);
	m_dwGMemTotal = GHndDat_GetSize() * m_dwHndDataTotal * 1.2 + m_dwGBufSize * m_dwIoDataTotal * 1.5;
	m_nSocketBufSize = nSocketBufSize_SOCKET;

	return OnInitSocket();
}

BOOL GIOCPsocket::GTcpSvr_OpenListen(DWORD &dwClientContext, CString strListenIp,DWORD dwListenPort,DWORD dwProtocol,DWORD dwInstaned,
								PFN_ON_DATA_EVENT_BUSINESS 	pfnOnGSockConnectTcpSvr,
								PFN_ON_DATA_EVENT_BUSINESS	pfnOnGSockReceiveSvr,
								PFN_ON_DATA_EVENT_BUSINESS	pfnOnGSockSendedSvr,
								PFN_ON_EVENT_BUSINESS		pfnOnGSockDisconnectSvr,
								PFN_ON_DATA_EVENT_BUSINESS	pfnOnGSockSendErrorSvr,
								PFN_ON_EVENT_BUSINESS	pfnOnGSockConnectionOverflow,
								PFN_ON_EVENT_BUSINESS	pfnOnGSockIdleOvertime,
								PFN_ON_EVENT_BUSINESS	pfnOnGSockUnknownError,
								PFN_ON_EVENT_BUSINESS	pfnOnGSockCreateClientSvr,
								PFN_ON_EVENT_BUSINESS	pfnOnGSockDestroyClientSvr)
{
	if ("" == strListenIp)
	{
		strListenIp = m_strLocalIp;
	}

	DWORD dwListenerId = 0;

	switch(dwProtocol)
	{
	case 0:
		dwListenerId = GTcpSvr_CreateListen((char*)strListenIp.GetBuffer(), dwListenPort, this);
		break;
	}
	
	if(dwListenerId)
	{
		dwClientContext = dwListenerId;
		GPFNDATA_SVR temp_GPfnData_SVR;
		temp_GPfnData_SVR.dwInstaned = dwInstaned;
		temp_GPfnData_SVR.pfnOnGSockConnectTcpSvr = pfnOnGSockConnectTcpSvr;
		temp_GPfnData_SVR.pfnOnGSockDisconnectSvr = pfnOnGSockDisconnectSvr;
		temp_GPfnData_SVR.pfnOnGSockReceiveSvr = pfnOnGSockReceiveSvr;
		temp_GPfnData_SVR.pfnOnGSockSendedSvr = pfnOnGSockSendedSvr;
		temp_GPfnData_SVR.pfnOnGSockSendErrorSvr = pfnOnGSockSendErrorSvr;
		temp_GPfnData_SVR.pfnOnGSockConnectionOverflow = pfnOnGSockConnectionOverflow;
		temp_GPfnData_SVR.pfnOnGSockIdleOvertime = pfnOnGSockIdleOvertime;
		temp_GPfnData_SVR.pfnOnGSockUnknownError = pfnOnGSockUnknownError;
		temp_GPfnData_SVR.pfnOnGSockCreateClientSvr = pfnOnGSockCreateClientSvr;
		temp_GPfnData_SVR.pfnOnGSockDestroyClientSvr = pfnOnGSockDestroyClientSvr;
		m_GPfnData_SVR_HashManager.AddItem(dwListenerId,temp_GPfnData_SVR);
		return (TRUE);
	}

	return (FALSE);
}

BOOL GIOCPsocket::GTcpSvr_Connect(DWORD &dwClientContext, CString strRemoteIp,DWORD dwRemotePort,CString strLocalIp,DWORD dwProtocol,DWORD dwInstaned,
								PFN_ON_DATA_EVENT_BUSINESS		pfnOnGSockConnectClt,
								PFN_ON_DATA_EVENT_BUSINESS		pfnOnGSockReceiveClt,
								PFN_ON_DATA_EVENT_BUSINESS		pfnOnGSockSendedClt,
								PFN_ON_DATA_EVENT_BUSINESS		pfnOnGSockSendErrorClt,
								PFN_ON_EVENT_BUSINESS			pfnOnGSockConnectError,
								PFN_ON_EVENT_BUSINESS			pfnOnGSockDisconnectClt,
								PFN_ON_EVENT_BUSINESS			pfnOnGSockHeartbeat,
								PFN_ON_EVENT_BUSINESS			pfnOnGSockCreateClientClt,
								PFN_ON_EVENT_BUSINESS			pfnOnGSockDestroyClientClt)
{
	if ("" == strLocalIp)
	{
		strLocalIp = m_strLocalIp;
	}

	DWORD dwClientId;

	switch(dwProtocol)
	{
	case 0:
		dwClientId = GTcpClt_CreateClient((char *)strRemoteIp.GetBuffer(), dwRemotePort, (char *)strLocalIp.GetBuffer(), this);
		break;
	}
	
	if(dwClientId)
	{
		dwClientContext = dwClientId;
		GPFNDATA_CLT temp_GPfnData_CLT;
		temp_GPfnData_CLT.dwInstaned = dwInstaned;
		temp_GPfnData_CLT.pfnOnGSockConnectClt = pfnOnGSockConnectClt;
		temp_GPfnData_CLT.pfnOnGSockReceiveClt = pfnOnGSockReceiveClt;
		temp_GPfnData_CLT.pfnOnGSockSendedClt = pfnOnGSockSendedClt;
		temp_GPfnData_CLT.pfnOnGSockSendErrorClt = pfnOnGSockSendErrorClt;
		temp_GPfnData_CLT.pfnOnGSockConnectError = pfnOnGSockConnectError;
		temp_GPfnData_CLT.pfnOnGSockDisconnectClt = pfnOnGSockDisconnectClt;
		temp_GPfnData_CLT.pfnOnGSockHeartbeat = pfnOnGSockHeartbeat;
		temp_GPfnData_CLT.pfnOnGSockCreateClientClt = pfnOnGSockCreateClientClt;
		temp_GPfnData_CLT.pfnOnGSockDestroyClientClt = pfnOnGSockDestroyClientClt;
		m_GPfnData_CLT_HashManager.AddItem(dwClientId,temp_GPfnData_CLT);
		return (TRUE);
	}

	return (FALSE);

}

/******************************************************************************************************************/
/*						Server Callback Funcation
/******************************************************************************************************************/

void GIOCPsocket::OnConnectSvr(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
	GPFNDATA_SVR temp_gpfndata_svr;
	ZeroMemory(&temp_gpfndata_svr,sizeof(GPFNDATA_SVR));
	((GIOCPsocket*)GHndDat_GetOwner((DWORD)GHndDat_GetOwner(dwClientContext)))->m_GPfnData_SVR_HashManager.FindItem((DWORD)GHndDat_GetOwner(dwClientContext),temp_gpfndata_svr);
	temp_gpfndata_svr.pfnOnGSockConnectTcpSvr(dwClientContext,pBuf,dwBytes,temp_gpfndata_svr.dwInstaned);
}

void GIOCPsocket::OnDisconnectSvr(const DWORD dwClientContext)
{
	GPFNDATA_SVR temp_gpfndata_svr;
	ZeroMemory(&temp_gpfndata_svr,sizeof(GPFNDATA_SVR));
	((GIOCPsocket*)GHndDat_GetOwner((DWORD)GHndDat_GetOwner(dwClientContext)))->m_GPfnData_SVR_HashManager.FindItem((DWORD)GHndDat_GetOwner(dwClientContext),temp_gpfndata_svr);
	temp_gpfndata_svr.pfnOnGSockDisconnectSvr(dwClientContext,temp_gpfndata_svr.dwInstaned);
}

void GIOCPsocket::OnReceiveSvr(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
	GPFNDATA_SVR temp_gpfndata_svr;
	ZeroMemory(&temp_gpfndata_svr,sizeof(GPFNDATA_SVR));
	((GIOCPsocket*)GHndDat_GetOwner((DWORD)GHndDat_GetOwner(dwClientContext)))->m_GPfnData_SVR_HashManager.FindItem((DWORD)GHndDat_GetOwner(dwClientContext),temp_gpfndata_svr);
	temp_gpfndata_svr.pfnOnGSockReceiveSvr(dwClientContext,pBuf,dwBytes,temp_gpfndata_svr.dwInstaned);
}

void GIOCPsocket::OnReceiveErrorSvr(const DWORD dwClientContext)
{

}

void GIOCPsocket::OnSendedSvr(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
	GPFNDATA_SVR temp_gpfndata_svr;
	ZeroMemory(&temp_gpfndata_svr,sizeof(GPFNDATA_SVR));
	((GIOCPsocket*)GHndDat_GetOwner((DWORD)GHndDat_GetOwner(dwClientContext)))->m_GPfnData_SVR_HashManager.FindItem((DWORD)GHndDat_GetOwner(dwClientContext),temp_gpfndata_svr);
	temp_gpfndata_svr.pfnOnGSockSendedSvr(dwClientContext,pBuf,dwBytes,temp_gpfndata_svr.dwInstaned);
}

void GIOCPsocket::OnSendErrorSvr(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
	GPFNDATA_SVR temp_gpfndata_svr;
	ZeroMemory(&temp_gpfndata_svr,sizeof(GPFNDATA_SVR));
	((GIOCPsocket*)GHndDat_GetOwner((DWORD)GHndDat_GetOwner(dwClientContext)))->m_GPfnData_SVR_HashManager.FindItem((DWORD)GHndDat_GetOwner(dwClientContext),temp_gpfndata_svr);
	temp_gpfndata_svr.pfnOnGSockSendErrorSvr(dwClientContext,pBuf,dwBytes,temp_gpfndata_svr.dwInstaned);
}

void GIOCPsocket::OnIdleOvertimeSvr(const DWORD dwClientContext)
{
	GPFNDATA_SVR temp_gpfndata_svr;
	ZeroMemory(&temp_gpfndata_svr,sizeof(GPFNDATA_SVR));
	((GIOCPsocket*)GHndDat_GetOwner((DWORD)GHndDat_GetOwner(dwClientContext)))->m_GPfnData_SVR_HashManager.FindItem((DWORD)GHndDat_GetOwner(dwClientContext),temp_gpfndata_svr);
	temp_gpfndata_svr.pfnOnGSockIdleOvertime(dwClientContext,temp_gpfndata_svr.dwInstaned);
}

void GIOCPsocket::OnConnectionOverflowSvr(const DWORD dwClientContext)
{
	GPFNDATA_SVR temp_gpfndata_svr;
	ZeroMemory(&temp_gpfndata_svr,sizeof(GPFNDATA_SVR));
	((GIOCPsocket*)GHndDat_GetOwner((DWORD)GHndDat_GetOwner(dwClientContext)))->m_GPfnData_SVR_HashManager.FindItem((DWORD)GHndDat_GetOwner(dwClientContext),temp_gpfndata_svr);
	temp_gpfndata_svr.pfnOnGSockConnectionOverflow(dwClientContext,temp_gpfndata_svr.dwInstaned);
}

void GIOCPsocket::OnUnknownErrorSvr(const DWORD dwClientContext)
{
	GPFNDATA_SVR temp_gpfndata_svr;
	ZeroMemory(&temp_gpfndata_svr,sizeof(GPFNDATA_SVR));
	((GIOCPsocket*)GHndDat_GetOwner((DWORD)GHndDat_GetOwner(dwClientContext)))->m_GPfnData_SVR_HashManager.FindItem((DWORD)GHndDat_GetOwner(dwClientContext),temp_gpfndata_svr);
	temp_gpfndata_svr.pfnOnGSockUnknownError(dwClientContext,temp_gpfndata_svr.dwInstaned);
}

void GIOCPsocket::OnCreateClientSvr(const DWORD dwClientContext)
{
	GPFNDATA_SVR temp_gpfndata_svr;
	ZeroMemory(&temp_gpfndata_svr,sizeof(GPFNDATA_SVR));
	((GIOCPsocket*)GHndDat_GetOwner((DWORD)GHndDat_GetOwner(dwClientContext)))->m_GPfnData_SVR_HashManager.FindItem((DWORD)GHndDat_GetOwner(dwClientContext),temp_gpfndata_svr);
	temp_gpfndata_svr.pfnOnGSockCreateClientSvr(dwClientContext,temp_gpfndata_svr.dwInstaned);
}

void GIOCPsocket::OnDestroyClientSvr(const DWORD dwClientContext)
{
	GPFNDATA_SVR temp_gpfndata_svr;
	ZeroMemory(&temp_gpfndata_svr,sizeof(GPFNDATA_SVR));
	((GIOCPsocket*)GHndDat_GetOwner((DWORD)GHndDat_GetOwner(dwClientContext)))->m_GPfnData_SVR_HashManager.FindItem((DWORD)GHndDat_GetOwner(dwClientContext),temp_gpfndata_svr);
	temp_gpfndata_svr.pfnOnGSockDestroyClientSvr(dwClientContext,temp_gpfndata_svr.dwInstaned);
}

/***************************************************************************************************************************/
/*					Client Callback Funcation
/***************************************************************************************************************************/
void GIOCPsocket::OnHeartbeatClt(const DWORD dwClientContext)
{
	GPFNDATA_CLT temp_gpfndata_clt;
	ZeroMemory(&temp_gpfndata_clt,sizeof(GPFNDATA_CLT));
	((GIOCPsocket*)GHndDat_GetOwner(dwClientContext))->m_GPfnData_CLT_HashManager.FindItem(dwClientContext,temp_gpfndata_clt);
	temp_gpfndata_clt.pfnOnGSockHeartbeat(dwClientContext,temp_gpfndata_clt.dwInstaned);
}

void GIOCPsocket::OnConnectClt(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
	GPFNDATA_CLT temp_gpfndata_clt;
	ZeroMemory(&temp_gpfndata_clt,sizeof(GPFNDATA_CLT));
	((GIOCPsocket*)GHndDat_GetOwner(dwClientContext))->m_GPfnData_CLT_HashManager.FindItem(dwClientContext,temp_gpfndata_clt);
	temp_gpfndata_clt.pfnOnGSockConnectClt(dwClientContext,pBuf,dwBytes,temp_gpfndata_clt.dwInstaned);
}

void GIOCPsocket::OnConnectErrorClt(const DWORD dwClientContext)
{
	GPFNDATA_CLT temp_gpfndata_clt;
	ZeroMemory(&temp_gpfndata_clt,sizeof(GPFNDATA_CLT));
	((GIOCPsocket*)GHndDat_GetOwner(dwClientContext))->m_GPfnData_CLT_HashManager.FindItem(dwClientContext,temp_gpfndata_clt);
	temp_gpfndata_clt.pfnOnGSockConnectError(dwClientContext,temp_gpfndata_clt.dwInstaned);	
}

void GIOCPsocket::OnDisconnectClt(const DWORD dwClientContext)
{
	GPFNDATA_CLT temp_gpfndata_clt;
	ZeroMemory(&temp_gpfndata_clt,sizeof(GPFNDATA_CLT));
	((GIOCPsocket*)GHndDat_GetOwner(dwClientContext))->m_GPfnData_CLT_HashManager.FindItem(dwClientContext,temp_gpfndata_clt);
	temp_gpfndata_clt.pfnOnGSockDisconnectClt(dwClientContext,temp_gpfndata_clt.dwInstaned);
}

void GIOCPsocket::OnReceiveClt(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
	GPFNDATA_CLT temp_gpfndata_clt;
	ZeroMemory(&temp_gpfndata_clt,sizeof(GPFNDATA_CLT));
	((GIOCPsocket*)GHndDat_GetOwner(dwClientContext))->m_GPfnData_CLT_HashManager.FindItem(dwClientContext,temp_gpfndata_clt);
	temp_gpfndata_clt.pfnOnGSockReceiveClt(dwClientContext,pBuf,dwBytes,temp_gpfndata_clt.dwInstaned);
}

void GIOCPsocket::OnReceiveErrorClt(const DWORD dwClientContext)
{
	
}

void GIOCPsocket::OnSendedClt(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
	GPFNDATA_CLT temp_gpfndata_clt;
	ZeroMemory(&temp_gpfndata_clt,sizeof(GPFNDATA_CLT));
	((GIOCPsocket*)GHndDat_GetOwner(dwClientContext))->m_GPfnData_CLT_HashManager.FindItem(dwClientContext,temp_gpfndata_clt);
	temp_gpfndata_clt.pfnOnGSockSendedClt(dwClientContext,pBuf,dwBytes,temp_gpfndata_clt.dwInstaned);
}

void GIOCPsocket::OnSendErrorClt(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
	GPFNDATA_CLT temp_gpfndata_clt;
	ZeroMemory(&temp_gpfndata_clt,sizeof(GPFNDATA_CLT));
	((GIOCPsocket*)GHndDat_GetOwner(dwClientContext))->m_GPfnData_CLT_HashManager.FindItem(dwClientContext,temp_gpfndata_clt);
	temp_gpfndata_clt.pfnOnGSockSendErrorClt(dwClientContext,pBuf,dwBytes,temp_gpfndata_clt.dwInstaned);
}

void GIOCPsocket::OnCreateClientClt(const DWORD dwClientContext)
{
// 	GPFNDATA_CLT temp_gpfndata_clt;
// 	ZeroMemory(&temp_gpfndata_clt,sizeof(GPFNDATA_CLT));
// 	((GIOCPsocket*)GHndDat_GetOwner(dwClientContext))->m_GPfnData_CLT_HashManager.FindItem(dwClientContext,temp_gpfndata_clt);
// 	temp_gpfndata_clt.pfnOnGSockCreateClientClt(dwClientContext,temp_gpfndata_clt.dwInstaned);
}

void GIOCPsocket::OnDestroyClientClt(const DWORD dwClientContext)
{
// 	GPFNDATA_CLT temp_gpfndata_clt;
// 	ZeroMemory(&temp_gpfndata_clt,sizeof(GPFNDATA_CLT));
// 	((GIOCPsocket*)GHndDat_GetOwner(dwClientContext))->m_GPfnData_CLT_HashManager.FindItem(dwClientContext,temp_gpfndata_clt);
// 	temp_gpfndata_clt.pfnOnGSockDestroyClientClt(dwClientContext,temp_gpfndata_clt.dwInstaned);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------