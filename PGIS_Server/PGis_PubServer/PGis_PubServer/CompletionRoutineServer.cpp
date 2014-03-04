// TCPSERVERServer.cpp: implementation of the CTCPSERVERServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompletionRoutineServer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//
CCompletionRoutineServer::CCompletionRoutineServer()
{
	m_bWorkThreadRun = FALSE;
    m_dwWorkThreadID = 0;
    m_hWorkThread = NULL;
    m_hWorkThreadEndEvt = NULL;

    m_bAcceptThreadRun = FALSE;
    m_dwAcceptThreadID = 0;
    m_hAcceptThread = NULL;
    m_hAcceptThreadEndEvt = NULL;

    m_nMalClientMum = 0;	
    m_nAppID = -1;
}

CCompletionRoutineServer::~CCompletionRoutineServer()
{
	DeleteCriticalSection(&m_csIsAcceptClient); 
	DeleteCriticalSection(&m_csDealRecv);
}

BOOL CCompletionRoutineServer::InitRoutineServer(int nMalClientMum)
{
    if(nMalClientMum <=0 || nMalClientMum > 2048)
    {
        return FALSE;
    }

    m_nMalClientMum = nMalClientMum;

    m_pListClient = new CompletionRoutineLink[m_nMalClientMum];

    memset(m_pListClient,0,sizeof(CompletionRoutineLink) * m_nMalClientMum);
    
    InitData();

    return TRUE;
}

void CCompletionRoutineServer::ReleaseRoutineServer()
{
    Close();

    if(m_pListClient)
    {
        delete[] m_pListClient;
        m_pListClient = NULL;
    }
}

/******************************************************************************
Function:    	Start;
Description��	������
Parameter:		UINT nPort:�������󶨵Ķ˿�;
				LPFTCPSERVERCALLBACK lpfTCPSERVERCallBack:�������¼�����������
				��ʽ�ο�TCPSERVERInterface.h�е�˵����
				DWORD dwInstance:�û��Զ������ݣ��ɴ���thisָ��
Return			TCPSERVER_STARTSUCCESS:�ɹ�
				����ο�TCPSERVERInterface.h��TCPSERVER_ERROR����
*******************************************************************************/
int CCompletionRoutineServer::Start(UINT nPort,const char *strLocalIP,LPFTCPSERVERCALLBACK lpfTcpServerCallBack,DWORD dwInstance,int nAppID)
{
	int nResult;

	if (m_wsaEvent[0] == WSA_INVALID_EVENT) 
	{
		m_wsaEvent[0] = WSACreateEvent();
	}
    if(m_nMalClientMum == 0)
    {
        return TCPSERVER_INIT_ERROR;
    }
    
	nResult = InitListen(nPort,strLocalIP);
	if (nResult != TCPSERVER_LISTENSUCCESS) 
	{
		return nResult;
	}
	m_lpfTcpServerCallBack = lpfTcpServerCallBack;

	nResult = CreateAcceptThread();
	if (nResult != TCPSERVER_CREATEACCEPTTHREADSUCCESS)
	{
		return nResult;
	}

	nResult = CreateRecvThread();
	if (nResult != TCPSERVER_CREATERECVTHREADSUCCESS)
	{
		return nResult;
	}
	
	m_dwUserInstance = dwInstance;

    m_nAppID = nAppID;
	return TCPSERVER_STARTSUCCESS;
}

/******************************************************************************
Function:    	Send;
Description��	�������ݣ�
Parameter��		int nSockID:Socket�������� 
				const char *pszData:Ҫ���͵�����
				int nDataLen:Ҫ�������ݵĳ���
Return:			TCPSERVER_ERROR_NOTRIGHTSOCKID,
				TCPSERVER_ERROR_NOTRIGHTSOCKID,
				TCPSERVER_ERROR_SENDDATA,
				TCPSERVER_ERROR_SEND:�ο�TCPSERVERInterface.h��TCPSERVER_ERROR����
				>=0:���ͳɹ����ݵ�ʵ�ʳ���
*******************************************************************************/
int CCompletionRoutineServer::Send(int nSockID, 
							   const char *pszData, 
							   int nDataLen)
{
	if ((nSockID < 0) || (nSockID >m_nMalClientMum - 1))
	{
		return TCPSERVER_ERROR_NOTRIGHTSOCKID;
	}
	
	if (m_pListClient[nSockID].sock == INVALID_SOCKET)
	{
		return TCPSERVER_ERROR_NOTRIGHTSOCKID;
	}
	
	if ((pszData == NULL) || (nDataLen <= 0))
	{
		return TCPSERVER_ERROR_SENDDATA;
	}

	int nSendResult;
	nSendResult = send(m_pListClient[nSockID].sock,pszData,nDataLen,0);
	if (nSendResult == SOCKET_ERROR)
	{
        //return TCPSERVER_ERROR_SEND;
		if(WSAGetLastError() != WSAEWOULDBLOCK)
		{
			return TCPSERVER_ERROR_SEND;
		}
		else
		{
			Sleep(1);
			return 0;
		}
	}
	
	return nSendResult;
}

/******************************************************************************
Function:    	InitListen;
Description��	��ʼ��������
Parameter��		UINT nPort:�����Ķ˿�
Return:			TCPSERVER_LISTENSUCCESS:�����ɹ�;
				����ο�TCPSERVERInterface.h��TCPSERVER_ERROR����
*******************************************************************************/
int CCompletionRoutineServer::InitListen(UINT nPort,const char *strLocalIP)
{
	int nRet;
	unsigned long ulUnLock = 1;
	
	if (nPort <0 || nPort > 65535)
	{
		TRACE("TCPSERVER Port Error!!\n");
		return TCPSERVER_ERROR_PORTEXCEEDRANGE;
	}
	
	m_TcpListen.sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (m_TcpListen.sock == INVALID_SOCKET)
	{
		TRACE("TCPSERVER ListenSocket create fail!!\n");
		return TCPSERVER_ERROR_CREATELISTNESOCK;
	}
	
	nRet = ioctlsocket(m_TcpListen.sock,FIONBIO,&ulUnLock);
	if (nRet == SOCKET_ERROR)
	{
		TRACE("TCPSERVER to put the socket into nonblocking mode Fail!!\n");		
		return TCPSERVER_ERROR_NONBLOCKING;
	}
	
	m_TcpListen.Addr.sin_port = htons(nPort);
	m_TcpListen.Addr.sin_family = AF_INET;
    if(strLocalIP != NULL)
    {
        m_TcpListen.Addr.sin_addr.s_addr = inet_addr(strLocalIP);
    }
    else
    {
        m_TcpListen.Addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
	
	nRet = bind(m_TcpListen.sock, (LPSOCKADDR)&(m_TcpListen.Addr), sizeof(m_TcpListen.Addr));
	if (nRet == SOCKET_ERROR)
	{
		
		TRACE("TCPSERVER Bind fail!!\n");
		return TCPSERVER_ERROR_BIND;
	}
	
	nRet = listen(m_TcpListen.sock,100);
	if (nRet == SOCKET_ERROR)
	{
		TRACE("TCPSERVER Listen fail!!\n");		
		return TCPSERVER_ERROR_LISTEN;
	}	

	
	return TCPSERVER_LISTENSUCCESS;
}

/******************************************************************************
Function:    	CreateAcceptThread;
Description��	�����ͻ��������̣߳�����ͻ������ӣ�
Parameter��		��;
Return:			TCPSERVER_CREATEACCEPTTHREADSUCCESS:Accept �̴߳����ɹ�;
				TCPSERVER_ERROR_CREATEACCEPTTHREADFAIL:Accepte �̴߳���ʧ��;
*******************************************************************************/
int CCompletionRoutineServer::CreateAcceptThread()
{
    //�����ͻ��������߳�
    m_bAcceptThreadRun = TRUE;

    if(m_hAcceptThreadEndEvt == 0)
    {
        m_hAcceptThreadEndEvt = CreateEvent(NULL,TRUE,TRUE,NULL);
    }

    m_hAcceptThread = (HANDLE)_beginthreadex( NULL,0,&AcceptThreadDispatch,this, 0,&m_dwAcceptThreadID );
    if(!m_hAcceptThread)
    {
        TRACE("TCPSERVER AcceptThread Create Fail!!\n");

        m_bAcceptThreadRun = FALSE;

        return TCPSERVER_ERROR_CREATEACCEPTTHREADFAIL;
    }
   
    TRACE("TCPSERVER RecvThread Create succeed!!\n");
	
	return TCPSERVER_CREATEACCEPTTHREADSUCCESS;
}

/******************************************************************************
Function:    	AcceptThreadDispatch;
Description��	��Accept���䵽��غ������д���
Parameter��		LPVOID pParam:����thisָ��;
Return:			������;
*******************************************************************************/
UINT CCompletionRoutineServer::AcceptThreadDispatch(LPVOID pParam)
{
	if (pParam != NULL)//��Ϊ��������Ϊ�գ����Բ�������
	{
		CCompletionRoutineServer * pTcpOverlappedSocket = (CCompletionRoutineServer *)pParam;
		pTcpOverlappedSocket ->AcceptThreadWork();
		
	}
	return 0;
}


/******************************************************************************
Function:    	AcceptThreadWork;
Description��	Accept ������̣�
Parameter��		��;
Return:			��;
*******************************************************************************/
void CCompletionRoutineServer::AcceptThreadWork()
{
	CompletionRoutineLink Client;
	
	int nClientAddrLen = sizeof(Client.Addr);
	int nSockID = TCPSERVER_NONE_CANUSESOCK;
	
    ResetEvent(m_hAcceptThreadEndEvt);
    
	while (m_bAcceptThreadRun)
	{
		Client.sock = accept(m_TcpListen.sock, (struct sockaddr*)&Client.Addr, &nClientAddrLen);
		if (Client.sock == INVALID_SOCKET)
		{
			Sleep(10);//����Ϊ�����������߳��л����л�
			continue;
		}
		
#ifdef _DEBUG
//		char szMessage[256];
//		memset(szMessage,0,sizeof(szMessage));
//		sprintf(szMessage,"TCPSERVER Connection from %s\n",inet_ntoa(Client.Addr.sin_addr));
//		TRACE("%s",szMessage);
#endif
		
		nSockID = GetUsableSock();
		
#ifdef _DEBUG
//		memset(szMessage,0,sizeof(szMessage));
//		sprintf(szMessage,"TCPSERVER Malloc Socket ID is:%d\n",nSockID);
//		TRACE(szMessage);
#endif

		if (nSockID != TCPSERVER_NONE_CANUSESOCK)//���鲻��Խ��,��GetUsableSocket���п���
		{
			m_pListClient[nSockID].sock = Client.sock;	
			m_pListClient[nSockID].Addr = Client.Addr;
			
			ZeroMemory(&m_pListClient[nSockID].ol,sizeof(WSAOVERLAPPED));
			
			if (m_pListClient[nSockID].wsaBuff.buf == NULL)//˵���ռ仹û�з���
			{
				m_pListClient[nSockID].wsaBuff.buf = new char[TCPSERVER_RECVBUFLEN];
				if (m_pListClient[nSockID].wsaBuff.buf == NULL)//��Դ�������
				{
					closesocket(m_pListClient[nSockID].sock);
					m_pListClient[nSockID].sock = INVALID_SOCKET;
					TRACE("TCPSERVER The Memory is not enough\n");					
					continue;
				}
			}
			
			TcpServerCallBack(TCPSERVER_EVENT_CLIENTJOIN,
							inet_ntoa(Client.Addr.sin_addr),
							 nSockID,
							 ntohs(Client.Addr.sin_port),
							 NULL,
							 0);
			
			memset(m_pListClient[nSockID].wsaBuff.buf,0,TCPSERVER_RECVBUFLEN);
			m_pListClient[nSockID].wsaBuff.len = TCPSERVER_RECVBUFLEN;
			m_pListClient[nSockID].dwFlags = 0;
			m_pListClient[nSockID].nIndex = nSockID;
			
			m_pListClient[nSockID].ol.Offset = nSockID;//����socketʱʹ��
			m_pListClient[nSockID].ol.OffsetHigh = (DWORD)this;//�ص�������ʹ��
			m_pListClient[nSockID].pAppThis = (DWORD)this;//�ص�������ʹ��

			EnterCriticalSection(&m_csDealRecv);
			m_vecAcceptClientTable.push_back(nSockID);
			LeaveCriticalSection(&m_csDealRecv);


			EnterCriticalSection(&m_csIsAcceptClient);
			m_bIsAcceptClient = TRUE;
			LeaveCriticalSection(&m_csIsAcceptClient);
			
		}
		else
		{
            printf("û���㹻��Socket��Դ\n");

			//û���㹻��Socket��Դ,ǿ�ƶϿ��ͻ�������
			closesocket(Client.sock);
			Client.sock = INVALID_SOCKET;
			
			TRACE("TcpSever sock is not enough\n");
		}
				
	}

    SetEvent(m_hAcceptThreadEndEvt);
    
}

/******************************************************************************
Function:    	GetUsableSock;
Description��	ȡ�ÿ��õ�Socket;
Parameter��		��;
Return:			NONE_CANUSESOCK:�޿��õ�Sock;
				!=NONE_CANUSESOCK:m_Client���������;
*******************************************************************************/
int CCompletionRoutineServer::GetUsableSock()
{
	static int nCurrSockNum = 0;//��ǰSocket�ı�ţ�
	int i = 0;
	for( i = nCurrSockNum ; i < m_nMalClientMum ; i ++)
	{
		if (m_pListClient[i].sock == INVALID_SOCKET)
		{
			nCurrSockNum = i + 1;

			return i;
		}
	}

	for(i = 0 ; i <= nCurrSockNum - 1; i++)
	{
		if (m_pListClient[i].sock == INVALID_SOCKET)
		{
			nCurrSockNum = i + 1;
			return i;
		}
	}
	
	return TCPSERVER_NONE_CANUSESOCK;
}

/******************************************************************************
Function:    	CreateRecvThread;
Description��	�����ͻ������ݽ����̣߳�
Parameter��		��;
Return:			TCPSERVER_CREATERECVTHREADSUCCESS:Recv �̴߳����ɹ�;
				TCPSERVER_ERROR_CREATERECVTHREADFAIL:Accepte �̴߳���ʧ��;
*******************************************************************************/
int CCompletionRoutineServer::CreateRecvThread()
{

    //ϵͳ�����߳�
    m_bWorkThreadRun = TRUE;

    if(m_hWorkThreadEndEvt == 0)
    {
        m_hWorkThreadEndEvt = CreateEvent(NULL,TRUE,TRUE,NULL);

    }

    m_hWorkThread = (HANDLE)_beginthreadex( NULL,0,&RecvThreadDispatch,this, 0,&m_dwWorkThreadID );
    if(!m_hWorkThread)
    {
        TRACE(_T("TCPSERVER RecvThread Create Fail\n"));

        m_bWorkThreadRun = FALSE;

        return TCPSERVER_ERROR_CREATERECVTHREADFAIL;
    }
   
    TRACE("TCPSERVER RecvThread Create succeed!!\n");
	
	return TCPSERVER_CREATERECVTHREADSUCCESS;

}

/******************************************************************************
Function:    	RecvThreadDispatch;
Description��	��Recv���䵽��غ������д���
Parameter��		LPVOID pParam:����thisָ��;
Return:			������;
*******************************************************************************/
UINT WINAPI CCompletionRoutineServer::RecvThreadDispatch(LPVOID pParam)
{
	if (pParam != NULL)//��Ϊ��������Ϊ�գ����Բ�������
	{
		CCompletionRoutineServer * pTcpOverlappedSocket = (CCompletionRoutineServer *)pParam;
		pTcpOverlappedSocket ->RecvThreadWork();
	}
	return 0;
}

/******************************************************************************
Function:    	RecvThreadWork;
Description��	recv ������̣�
Parameter��		��;
Return:			��;
*******************************************************************************/
void CCompletionRoutineServer::RecvThreadWork()
{
	BOOL bIsAcceptClient;
	vector<int>::iterator it;
	int nSockID;

    ResetEvent(m_hWorkThreadEndEvt);

	while(m_bWorkThreadRun) 
	{
		EnterCriticalSection(&m_csIsAcceptClient);
		bIsAcceptClient = m_bIsAcceptClient;
		LeaveCriticalSection(&m_csIsAcceptClient);
		if (bIsAcceptClient)
		{
			EnterCriticalSection(&m_csDealRecv);
			for (it = m_vecAcceptClientTable.begin(); it != m_vecAcceptClientTable.end(); )
			{
				nSockID = *it;
				if(WSARecv(m_pListClient[nSockID].sock ,
					&m_pListClient[nSockID].wsaBuff,1,
					&m_pListClient[nSockID].dwBytes,
					&m_pListClient[nSockID].dwFlags,
					&m_pListClient[nSockID].ol, 
					CompletionROUTINE) == SOCKET_ERROR)
				{ 
					// ����WSA_IO_PENDING�������������ʾIO�������ڽ��У������������
                    int nError = WSAGetLastError();
					if( nError != WSA_IO_PENDING)
					{
						closesocket(m_pListClient[nSockID].sock);   
						m_pListClient[nSockID].sock = INVALID_SOCKET;   
						TcpServerCallBack(TCPSERVER_EVENT_CLIENTEXIT,
							inet_ntoa(m_pListClient[nSockID].Addr.sin_addr),
							nSockID,
							ntohs(m_pListClient[nSockID].Addr.sin_port),
							NULL,0);

						TRACE("TcpSerer The Accept is WSA_IO_PENDING\n");					
						
					}
				}
				it = m_vecAcceptClientTable.erase(it);
			}
			LeaveCriticalSection(&m_csDealRecv);
			
			EnterCriticalSection(&m_csIsAcceptClient);
			m_bIsAcceptClient = FALSE;
			LeaveCriticalSection(&m_csIsAcceptClient);
		}

		DWORD dwIndex = WSAWaitForMultipleEvents(1,m_wsaEvent,FALSE,10,TRUE);
		if(dwIndex == WSA_WAIT_IO_COMPLETION)
		{
			TRACE("WSA_WAIT_IO_COMPLETION\n");

			//ResetEvent(m_wsaEvent);
			//break;
		}
		else
		{
			if (dwIndex == WSA_WAIT_FAILED)
			{
				TRACE("WSA_WAIT_IO_COMPLETION Error   1111111\n");
			}
		}

		Sleep(5);
		
	}
    SetEvent(m_hWorkThreadEndEvt);
    
}

/******************************************************************************
Function:    	CloseClient;
Description��	��ָ���ͻ��˶Ͽ����ӣ�
Parameter��		int nSockID:Ҫ�رտͻ���Socket ID��
Return:			TCPSERVER_CLOSECLIENTSUCCESS:�ɹ�;
				TCPSERVER_ERROR_NOTRIGHTSOCKID:ʧ��,�ο�TCPSERVERInterface.h
				��TCPSERVER_ERROR����
*******************************************************************************/
int CCompletionRoutineServer::CloseClient(int nSockID)
{
	if ((nSockID < 0) || (nSockID >m_nMalClientMum - 1))
	{
		return TCPSERVER_ERROR_NOTRIGHTSOCKID;
	}	

	if (m_pListClient[nSockID].sock != INVALID_SOCKET) 
	{
		//kevin tang 2009-6-9
		shutdown(m_pListClient[nSockID].sock, SD_BOTH); //�⽫ǿ�����еĴ���ͽ���ʧ��

		closesocket(m_pListClient[nSockID].sock);
		m_pListClient[nSockID].sock = INVALID_SOCKET;
	}
	else
	{
		return TCPSERVER_ERROR_NOTRIGHTSOCKID;
	}
	return TCPSERVER_CLOSECLIENTSUCCESS;
}

/******************************************************************************
Function:    	Close;
Description��	ֹͣ��ǰ���й����߳�,�Ͽ��ͻ�������,�ͷ���Դ��
Parameter��		��;
Return:			��;
*******************************************************************************/
void CCompletionRoutineServer::Close()
{
    m_bWorkThreadRun = FALSE;
    //�ȴ��߳̽���
    if(WaitForSingleObject(m_hWorkThreadEndEvt,2000) == WAIT_TIMEOUT)
    {
        TerminateThread(m_hWorkThread,0);
        TRACE(_T("Work�߳�ǿ����ֹ \n"));
    }

    CloseHandle(m_hWorkThreadEndEvt);
    m_hWorkThreadEndEvt = NULL;
    CloseHandle(m_hWorkThread);
    m_hWorkThread = NULL;
            
	//���������߳�
    m_bAcceptThreadRun = FALSE;
    if(WaitForSingleObject(m_hAcceptThreadEndEvt,2000) == WAIT_TIMEOUT)
    {
        TerminateThread(m_hAcceptThread,0);
        TRACE(_T("Accept�߳�ǿ����ֹ \n"));
    }

    CloseHandle(m_hAcceptThreadEndEvt);
    m_hAcceptThreadEndEvt = NULL;
    CloseHandle(m_hAcceptThread);
    m_hAcceptThread = NULL;
	
	for(int i = 0 ; i < m_nMalClientMum  ; i++)
	{
		if (m_pListClient[i].wsaBuff.buf != NULL) 
		{
			delete[] m_pListClient[i].wsaBuff.buf;
			m_pListClient[i].wsaBuff.buf = NULL;
		}
		
		if(m_pListClient[i].sock != INVALID_SOCKET)
		{
			shutdown(m_pListClient[i].sock, SD_BOTH); //�⽫ǿ�����еĴ���ͽ���ʧ��
			closesocket(m_pListClient[i].sock);
			m_pListClient[i].sock = INVALID_SOCKET;
		}
	}

	if (m_wsaEvent[0] != WSA_INVALID_EVENT)
	{
		WSACloseEvent(m_wsaEvent[0]);
		m_wsaEvent[0] = WSA_INVALID_EVENT;
	}

	closesocket(m_TcpListen.sock);
	m_TcpListen.sock = INVALID_SOCKET;

	m_lpfTcpServerCallBack = NULL;
}

/******************************************************************************
Function:    	TCPSERVERCallBack;
Description:	���ûص�������
Parameter:		const UINT uMsg:�ص���Ϣ
				const char *pszIP:�ͻ�IP,��192.168.21.112
				const int nID:�ͻ�ID
				const int nPort:�ͻ��˿�
Return:			��;
*******************************************************************************/
void CCompletionRoutineServer::TcpServerCallBack(UINT uMsg,
											 const char *pszIP, 
											 int nID, 
											 int nPort,
                                             char *pszData,
											 //const char *pszData,
											 int nDataLen)
{
	TcpServerClient ClientInfo;
    ClientInfo.nAppID = m_nAppID;
	ClientInfo.nID = nID;
	ClientInfo.nPort = nPort;
	memset(ClientInfo.szIP,0,sizeof(ClientInfo.szIP));
	memcpy(ClientInfo.szIP,pszIP,strlen(pszIP));

    //û��Ҫ��һ���ڴ�COPY kevin
	//char szData[TCPSERVER_RECVBUFLEN];
	//if (pszData != NULL)
	//{
	//	memset(szData,0,sizeof(szData));
	//	memcpy(szData,pszData,nDataLen);
	//	ClientInfo.pszRecvData = szData;
	//}
	//else
	//{
	//	ClientInfo.pszRecvData = NULL;
	//}

    if(pszData != NULL)
    {
        ClientInfo.pszRecvData = pszData;
        ClientInfo.nRecvDataLen = nDataLen;
    }
    else
    {
        ClientInfo.pszRecvData = NULL;
        ClientInfo.nRecvDataLen = 0;
    }
	
	if (m_lpfTcpServerCallBack != NULL)
	{
		m_lpfTcpServerCallBack(uMsg,(DWORD)(&ClientInfo),m_dwUserInstance);
	}
}

/******************************************************************************
Function:    	InitData;
Description��	��ʼ������Ҫ�����ݣ�
Parameter��		��
Return:			��;
*******************************************************************************/
void CCompletionRoutineServer::InitData()
{
	m_lpfTcpServerCallBack = NULL;
	m_TcpListen.sock = INVALID_SOCKET;
    
    m_hWorkThread = NULL;
    m_hWorkThreadEndEvt = NULL;

    m_hAcceptThread = NULL;
    m_hAcceptThreadEndEvt = NULL;
	
	for(int i = 0 ; i < m_nMalClientMum ; i++)
	{
		m_pListClient[i].sock = INVALID_SOCKET;
		m_pListClient[i].wsaBuff.buf = NULL;
	}
	
	m_wsaEvent[0] = WSA_INVALID_EVENT;//ֻ��Ҫһ��

	InitializeCriticalSection(&m_csIsAcceptClient); 
	InitializeCriticalSection(&m_csDealRecv);
	m_bIsAcceptClient = FALSE;
}


void CALLBACK CCompletionRoutineServer::CompletionROUTINE(
								IN DWORD dwError, 
								IN DWORD cbTransferred, 
								IN LPWSAOVERLAPPED lpOverlapped, 
								IN DWORD dwFlags
								)
{
	
	//LPPER_IO_OPERATION_DATA lpPerIOData = (LPPER_IO_OPERATION_DATA )lpOverlapped;
	if (lpOverlapped != NULL)
	{
		// Connection was closed by client
        
		CompletionRoutineLink *ptemp = (CompletionRoutineLink *)lpOverlapped;
		CCompletionRoutineServer * pTcpOverlappedSocket = (CCompletionRoutineServer *)ptemp->pAppThis;
		
		if(pTcpOverlappedSocket !=NULL)
		{
			//if(cbTransferred > 0) // add by kevin 2010-8-3
			{
				pTcpOverlappedSocket ->DealRecv(dwError,ptemp->nIndex,cbTransferred);
			}
		}
		
	}

	/*if (lpOverlapped != NULL)
	{
		if (lpOverlapped->OffsetHigh != 0)
		{
			CCompletionRoutineServer * pTcpOverlappedSocket = (CCompletionRoutineServer *)lpOverlapped->OffsetHigh;
			pTcpOverlappedSocket ->DealRecv(dwError,lpOverlapped->Offset,cbTransferred);
		}
		else
		{
			int nError = GetLastError();

			printf("afdsadfasdff   dfsdf   error:%d\n",nError);
		}
	}*/

}

void CCompletionRoutineServer::DealRecv(DWORD dwError,int nSockID,DWORD cbTransferred)
{
	if(nSockID < 0 || nSockID > m_nMalClientMum)
	{
		return ;
	}

	if (dwError != 0|| cbTransferred == 0)
	{
		TRACE("CompletionROUTINE Error\n");
        shutdown(m_pListClient[nSockID].sock, SD_BOTH); //�⽫ǿ�����еĴ���ͽ���ʧ��

		closesocket(m_pListClient[nSockID].sock);   
		m_pListClient[nSockID].sock = INVALID_SOCKET;   
		TcpServerCallBack(TCPSERVER_EVENT_CLIENTEXIT,
			inet_ntoa(m_pListClient[nSockID].Addr.sin_addr),
			nSockID,
			ntohs(m_pListClient[nSockID].Addr.sin_port),
			NULL,
            0);
		
		return;
	}

	TcpServerCallBack(TCPSERVER_EVENT_RECVDATA,
						inet_ntoa(m_pListClient[nSockID].Addr.sin_addr),
						nSockID,
						ntohs(m_pListClient[nSockID].Addr.sin_port),
						m_pListClient[nSockID].wsaBuff.buf,cbTransferred
						);

	//memset(m_pListClient[nSockID].wsaBuff.buf,0,TCPSERVER_RECVBUFLEN);

	m_pListClient[nSockID].wsaBuff.len = TCPSERVER_RECVBUFLEN;
	m_pListClient[nSockID].dwFlags = 0;
	if(WSARecv(m_pListClient[nSockID].sock ,&m_pListClient[nSockID].wsaBuff,1,
		&m_pListClient[nSockID].dwBytes,&m_pListClient[nSockID].dwFlags,
		&m_pListClient[nSockID].ol, CompletionROUTINE) == SOCKET_ERROR)
	{ 
		int nErrorCode = WSAGetLastError() ;
		if(nErrorCode != WSA_IO_PENDING)    
		{
            shutdown(m_pListClient[nSockID].sock, SD_BOTH); //�⽫ǿ�����еĴ���ͽ���ʧ��

			closesocket(m_pListClient[nSockID].sock);   
			m_pListClient[nSockID].sock = INVALID_SOCKET;   

			TcpServerCallBack(TCPSERVER_EVENT_CLIENTEXIT,
				inet_ntoa(m_pListClient[nSockID].Addr.sin_addr),
				nSockID,
				ntohs(m_pListClient[nSockID].Addr.sin_port),
				NULL,0);
			
			TRACE("TcpSerer The Accept is WSA_IO_PENDING\n");					
			
		}
	}
}
