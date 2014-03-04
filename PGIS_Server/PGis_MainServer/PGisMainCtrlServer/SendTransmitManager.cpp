#include "StdAfx.h"
#include "SendTransmitManager.h"

extern CWriteLogThread gWriteLogApp;

CSendTransmitManager::CSendTransmitManager(void)
{
	m_pTcpServGisSocket = NULL;
}

CSendTransmitManager::~CSendTransmitManager(void)
{
}

void CSendTransmitManager::Start(DWORD dwInstanced)
{
	if (m_pTcpServGisSocket == NULL)
	{
		m_pTcpServGisSocket = (CCompletionRoutineServer*)dwInstanced;
	}
}

BOOL CSendTransmitManager::Add(DWORD socketID,CString DeptNum)
{
	CListManager<DWORD>* pTempListManager = GetCListManagerData(DeptNum);
	if (pTempListManager == NULL)
	{
		CListManager<DWORD>* pTempListManager = new CListManager<DWORD>;
		if (pTempListManager != NULL)
		{
			m_MapSendTransmitManager.AddItem(DeptNum,pTempListManager);
			pTempListManager->AddItem(socketID);
			TRACE("%s ����û���������:%s\n",CTime::GetCurrentTime().Format("%H:%M:%S"),DeptNum);
			gWriteLogApp.WriteLog(FOR_ALARM,_T("%s [CSendTransmitManager::Add] ����û���������:%s\n"),CTime::GetCurrentTime().Format(_T("%H:%M:%S")),DeptNum);
			return TRUE;
		}
		else
		{
			gWriteLogApp.WriteLog(FOR_ALARM,_T("%s [CSendTransmitManager::Add] ���� ����û���������ʧ�� \n"),CTime::GetCurrentTime().Format(_T("%H:%M:%S")));
			return FALSE;
		}
	}
	else
	{
		pTempListManager->AddItem(socketID);
		TRACE("%s ����û�ID:%d\n",CTime::GetCurrentTime().Format("%H:%M:%S"),socketID);
		gWriteLogApp.WriteLog(FOR_ALARM,_T("%s [CSendTransmitManager::Add] ����û�ID:%d \n"),CTime::GetCurrentTime().Format(_T("%H:%M:%S")),socketID);
	}

	return TRUE;
}

void CSendTransmitManager::Stop()
{
	MAPSendTransmitManager::iterator mapIter = m_MapSendTransmitManager.begin();
	CListManager<DWORD>* pTempList;
	while(mapIter != m_MapSendTransmitManager.end())
	{
		pTempList = (CListManager<DWORD>*)((*mapIter).second);
		mapIter++;
		_ASSERT(pTempList != NULL);
		pTempList->ClearAll();
		delete pTempList;
		pTempList = NULL;
	}
	m_MapSendTransmitManager.ClearAll();
}

BOOL CSendTransmitManager::SendAllData(CString DepNum,const char* pzData, int nDataLen)
{
	CListManager<DWORD>* pTransListManager = GetCListManagerData("0000");
	if (pTransListManager != NULL)
	{
		CListManager<DWORD>::iterator ListIterator = pTransListManager->begin();
		while(ListIterator != pTransListManager->end())
		{
			m_pTcpServGisSocket->Send((*ListIterator),pzData,nDataLen);
			TRACE("%s [ת������]�û�����:%s �û�����:%d SOCKETID:%d\n",CTime::GetCurrentTime().Format("%H:%M:%S"),pzData,nDataLen,(*ListIterator));
			++ListIterator;
		}
	}

	CListManager<DWORD>* pListManager = GetCListManagerData(DepNum);
	if (pListManager == NULL)
	{
		return FALSE;
	}
	CListManager<DWORD>::iterator ListIterator = pListManager->begin();
	while(ListIterator != pListManager->end())
	{
		m_pTcpServGisSocket->Send((*ListIterator),pzData,nDataLen);
		TRACE("%s �û�����:%s �û�����:%d SOCKETID:%d\n",CTime::GetCurrentTime().Format("%H:%M:%S"),pzData,nDataLen,(*ListIterator));
		++ListIterator;
	}

	return TRUE;
}

CListManager<DWORD>* CSendTransmitManager::GetCListManagerData(CString DepNum)
{
	MAPSendTransmitManager::iterator MapIterator = m_MapSendTransmitManager.find(DepNum);
	CListManager<DWORD>* pListManager = NULL;
	if (MapIterator != m_MapSendTransmitManager.end())
	{
		pListManager = (CListManager<DWORD>*)((*MapIterator).second);
	}

	return pListManager;
}

BOOL CSendTransmitManager::ViewListData(CString DepNum)
{
	CListManager<DWORD>* pListManager = GetCListManagerData(DepNum);
	if (pListManager == NULL)
	{
		return FALSE;
	}
	CListManager<DWORD>::iterator ListIterator = pListManager->begin();
	UINT coutsize = pListManager->GetSize();
	while(ListIterator != pListManager->end())
	{
		gWriteLogApp.WriteLog( FOR_ALARM,"%s �û�����:%s �û�SOCKETID:%d �û�����:%d\n",CTime::GetCurrentTime().Format("%H:%M:%S"),DepNum,(*ListIterator),coutsize);
		++ListIterator;
	}
	return TRUE;
}