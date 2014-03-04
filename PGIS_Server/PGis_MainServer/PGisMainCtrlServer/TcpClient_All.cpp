// TcpClient.cpp: implementation of the Crwini class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "TcpClient.h"
#include "MemoryPool.h"

#ifdef WANGZH_ALLP //2013-4-23
#include "PGisMainCtrlServer.h"
extern CPgisMainCtrlProc gPGisMainCtrlProc;
#endif

extern CWriteLogThread gWriteLogApp;
extern ::CCriticalSection g_csRecvClientList;
extern CList<RECV_CLIENTDATA, RECV_CLIENTDATA&> g_RecvClientList;//?
extern UINT g_dwWorkThreadID;

extern UINT g_dwWebThreadID;
extern CMemoryPool gWebServiceList;
extern int gRecvGpsCount;
//////////////////////////////////////////////////////////////////////
//
CTcpClient_All::CTcpClient_All()
{
    Reset();
}

//
CTcpClient_All::~CTcpClient_All()
{
    
}

//////////////////////////////////////////////////////////////////////
//
BOOL CTcpClient_All::AnalyseRecvData(const char *recvData ,int nRecvLen,int nAppID)
{
    if(m_nBufLenCount + nRecvLen > MAX_RECVICE_BUFFER)
    {
        memset(m_RecvBuf,0,MAX_RECVICE_BUFFER);

        return FALSE;
    }
    
    memcpy(m_RecvBuf+m_nBufLenCount,recvData,nRecvLen);
    m_nBufLenCount = m_nBufLenCount + nRecvLen;
    
    //�ְ�
    int nSpaceLen = m_nBufLenCount;
    char *pCurPoint = m_RecvBuf;
    int i;
    int nPackageLen = 0;
    while(nSpaceLen >= MAIN_DATA_PACKAGE_LEN)
    {
        if( pCurPoint[0] != PACKAGE_HEAD_DATA_STX ) //����ͷ
        {
			gWriteLogApp.WriteLog(FOR_ALARM,_T("%s [CTcpClient][ID:%d,APP:%d]��ͷ��:len:%d,recv:%s\n"),
                    CTime::GetCurrentTime().Format(_T("%H:%M:%S")),m_nSocketID,nAppID,nSpaceLen,recvData);

            nSpaceLen = 0;

            break;
        }
        //���Ұ�β
        int nPoint = 0;
        for( i = 0 ; i < nSpaceLen; i++ )
        {
            if( pCurPoint[i] == PACKAGE_TAIL_DATA_ETX)
            {
                nPoint = i;
                break;
            }
        }
        if( nPoint > 0 )
        {
            //һ�������İ�
            nPackageLen = nPoint + 1;
            //
            if( nPackageLen < MAX_DATA_PACKAGE_LEN )
            {
				pCurPoint[nPackageLen -1] = NULL;
				TcpClientDataProcess(pCurPoint+1,nPackageLen-2,nAppID);
            }
            else
            {
                gWriteLogApp.WriteLog( FOR_ALARM,_T("%s :�����ȳ�����������ֵ,len:%d.\n"),
                    CTime::GetCurrentTime().Format("%H:%M:%S"),
					nPackageLen);
            }
            //ʣ�µ����ݼ����ְ�
            pCurPoint = pCurPoint + nPackageLen;
            nSpaceLen = nSpaceLen - nPackageLen;
        }
        else
        {
            if( nSpaceLen > MAX_DATA_PACKAGE_LEN )
            {
                gWriteLogApp.WriteLog( FOR_ALARM,_T("%s ���������ݰ�,len:%d.\n"),
                        CTime::GetCurrentTime().Format("%H:%M:%S"),nSpaceLen );
                
                nSpaceLen = 0;
            }
            break;
        }
        
    }//endwhile
    
    //�Ƿ���û�д��������
    if( nSpaceLen > 0 )
    {
        m_nBufLenCount = nSpaceLen;
        if( pCurPoint != m_RecvBuf )
        {
            memcpy(m_RecvBuf,pCurPoint,m_nBufLenCount);
            memset(m_RecvBuf+m_nBufLenCount,0,MAX_RECVICE_BUFFER-m_nBufLenCount);
        }
    }
    else
    {
        m_nBufLenCount = 0;
        memset(m_RecvBuf,0,MAX_RECVICE_BUFFER);
    }

    return TRUE;
}

//GIS�ͻ��� ���ݳ�����֤ 
BOOL CTcpClient_All::TcpClientDataProcess(const char *recvData, int nRecvLen,int nAppID)
{
	if(nRecvLen < PACKAGE_LEN)
    {
        return FALSE;
    }

	int nTempLen = 0;
	int nCmdType;
    sscanf(recvData, "%4x%2x",&nTempLen,&nCmdType);
	nTempLen = nTempLen - 2;	//ȥ��ͷ��β
	if(nTempLen != nRecvLen)
	{
		gWriteLogApp.WriteLog( FOR_ALARM,_T("%s ���ݳ��Ȳ�һ��,data:%s\n"),
			CTime::GetCurrentTime().Format("%H:%M:%S"),recvData );

		return FALSE;
	}

	nRecvLen = nRecvLen - PACKAGE_LEN;	//���� ����ͷ
	CString strTrace;
	CString strCurrTime;
	strCurrTime = CTime::GetCurrentTime().Format("%H:%M:%S");

    if(nAppID == SOCKET_GISCLIENT_TYPEID)
    {
        DispatchGisClientRecvData(nCmdType,recvData+PACKAGE_LEN,nRecvLen); 
    }
// #ifdef WANGZH_ALLP //2013-4-23
// 	else if (nAppID == SOCKET_PUBSERVER_TYPEID)
// 	{
// 		gPGisMainCtrlProc.DispatchPubNetClientRecvData(nCmdType,recvData+PACKAGE_LEN,nRecvLen); 	
// 	}
// #endif
	else
    {
        return FALSE;
    }

	return TRUE;
}

//GIS�ͻ��˷��͹���������  
void CTcpClient_All::DispatchGisClientRecvData(int CmdType,const char *RecvData, int nLen)
{
	CString strCurrTime = CTime::GetCurrentTime().Format("%H:%M:%S");
	CString strTrace;
	switch(CmdType)
	{
	case C_TYPE_TICK :
		//strTrace.Format(_T("%s [GIS][ID:%d]����GIS�ͻ���������:%s��IP:%s\n"),strCurrTime,
		//	m_nSocketID,
		//	RecvData,
		//	GetConnectIP());
		//gWriteLogApp.WriteLog(FOR_ALARM,strTrace);
		break;
	case C_TYPE_LOGIN:
		strTrace.Format(_T("%s [GIS][ID:%d]����GIS�ͻ��˵�¼����:%s\n"),strCurrTime,
			m_nSocketID,
			RecvData);
		gWriteLogApp.WriteLog(FOR_TRACE_1,strTrace);
		break;
	case C_TYPE_SYNC_REPORT: //ͬ��
		strTrace.Format(_T("%s [GIS][ID:%d]����GIS�ͻ���ͬ������:%s\n"),strCurrTime,
			m_nSocketID,
			RecvData);
		gWriteLogApp.WriteLog(FOR_TRACE_1,strTrace);
		break;
	case C_TYPE_APPCMD_TERMINAL: //�·�Ӧ��������ն�Э��
		strTrace.Format(_T("%s [GIS][ID:%d]����GIS�ͻ���Ӧ����������:%s\n"),strCurrTime,
			m_nSocketID,
			RecvData);
		gWriteLogApp.WriteLog(FOR_TRACE_1,strTrace);
		break;
	case C_TYPE_CTRLCMD_TERMINAL://�·���������Э��
		strTrace.Format(_T("%s [GIS][ID:%d]����GIS�ͻ��˷�������������:%s\n"),strCurrTime,
			m_nSocketID,
			RecvData);
		gWriteLogApp.WriteLog(FOR_TRACE_1,strTrace);
		break;
	case C_TYPE_TERMINAL_ALERT_ACK: //�·�Ӧ��ͽ���ն˱���Э��
		strTrace.Format(_T("%s [GIS][ID:%d]����GIS�ͻ��˷�Ӧ��ͽ���ն˱�������:%s\n"),strCurrTime,
			m_nSocketID,
			RecvData);
		gWriteLogApp.WriteLog(FOR_TRACE_1,strTrace);
		break;

// #ifdef WANGZH_H6 //2013-5-2
// 	case C_H6_TYPE_TERMINAL_ALERT_ACK:
// 		break;
// 	case C_H6_TYPE_TERMINAL_STATE_ACK:
// 		break;
// 	case C_H6_TYPE_TERMINAL_WAKEUP:
// 		break;
// #endif
// 	default:
// 		gWriteLogApp.WriteLog(FOR_ALARM,_T("%s [GIS][ID:%d]��������,�޷�ʶ�������,type:%d,data:%s\n"),
// 			CTime::GetCurrentTime().Format("%H:%M:%S"),
// 			m_nSocketID,
// 			CmdType,
// 			RecvData);

		return ;
	}

	if(g_dwWorkThreadID > 0)
	{
		RECV_CLIENTDATA tempRecvMessage;
		memset(tempRecvMessage.MessagePacket,0,MAX_DATA_PACKAGE_LEN);

		tempRecvMessage.nAppID = SOCKET_GISCLIENT_TYPEID;
		tempRecvMessage.nSockID = m_nSocketID;
		tempRecvMessage.nCmdType = CmdType;

		memcpy(tempRecvMessage.MessagePacket,RecvData,nLen);
		tempRecvMessage.length = nLen; 
		g_csRecvClientList.Lock();
		g_RecvClientList.AddHead(tempRecvMessage);
		//�����ݽ��������߳�������
		if(!PostThreadMessage(g_dwWorkThreadID,WM_RECVICE_CLIENT_MESSAGE,0,0))
		{
			Sleep(1);
			PostThreadMessage(g_dwWorkThreadID,WM_RECVICE_CLIENT_MESSAGE,0,0);
		}
		g_csRecvClientList.Unlock();

	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CTcpClient_All::StartTcpTickTimer(int nConnState ,int nTimerVal)
{
    m_bStartTimerFlag = FALSE;
    m_nNowTimerVal = 0;
    m_nConnState = nConnState;
    if(nTimerVal > 0)
    {
        m_nTimerVal = nTimerVal;
        m_bStartTimerFlag = TRUE;
    }
}

void CTcpClient_All::SetSocketConn(int nSocketID,const char *strIp)
{
	m_nSocketID = nSocketID;
	if(strIp != NULL)
	{
		_snprintf(strIpAddress, MAX_IPADDRESS_LEN-1, "%s",strIp);
	}
}

void CTcpClient_All::Reset()
{
    m_bStartTimerFlag = FALSE;
    m_nSocketID = -1;
    m_nNowTimerVal = 0;
    m_nConnState = TCP_DISCONNECT;
	m_nGISClientType = 0;

	memset(strIpAddress,0,MAX_IPADDRESS_LEN);

}
void CTcpClient_All::StopTcpTickTimer()
{
    m_bStartTimerFlag = FALSE;
}

int CTcpClient_All::CheckTimerOver()
{
    if(!m_bStartTimerFlag)
    {
        return 0;
    }

    switch(m_nConnState)
    {
    case TCP_LOGIN_WAIT: //�ͻ���TCP���ӳɹ���ȴ���¼��
        if( ++m_nNowTimerVal >= m_nTimerVal )
        {
            m_bStartTimerFlag = FALSE;
            m_nNowTimerVal = 0;

            return 1;
        }
        break;
    case TCP_LOGIN_SUCCESS: //������ֻ��ҪӦ��
        if( ++m_nNowTimerVal > m_nTimerVal )
        {
            m_bStartTimerFlag = FALSE;
            m_nNowTimerVal = 0;

            return 2;
        }
        break;
    default:
        break;
    }

    return 0;
}