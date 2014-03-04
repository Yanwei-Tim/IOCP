// GisMainClient.cpp: implementation of the Crwini class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "GisMainClient.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//
CGisMainClient::CGisMainClient()
{
	m_strLoginName = _T("");
	m_strDeptNum = _T("");
}

CGisMainClient::~CGisMainClient()
{

}

//////////////////////////////////////////////////////////////////////
//
CString CGisMainClient::GetLoginName()
{
	return m_strLoginName;
}

CString CGisMainClient::GetLoginMac()
{
	return m_strLoginMac;
}

void CGisMainClient::ResetLoginInfo()
{
	m_strLoginName = "";
	m_strLoginMac = "";
	m_strDeptNum = "";
}

CString CGisMainClient::GetDeptNum()
{
	return m_strDeptNum;
}

void CGisMainClient::SetLoginInfo(const char *strLoginName,const char *strDeptNum,const char *strMac)
{
	if(strLoginName != NULL)
	{
		m_strLoginName.Format("%s",strLoginName);
	}

	if(strDeptNum != NULL)
	{
		m_strDeptNum.Format("%s",strDeptNum);
	}

	if(strMac != NULL)
	{
		m_strLoginMac.Format("%s",strMac);
		m_bNewMode = TRUE;
	}
	else
	{
		m_strLoginMac = "";
		m_bNewMode = FALSE;
	}
}

//ͨ��web Service��ѯ���ݿ�,��ȡ���ڸ�GIS�ͻ��˹�����ն���Ϣ
BOOL CGisMainClient::LoadTerminalInfo()
{
    return TRUE;
}

//��ѯ�ն˺����Ƿ����
BOOL CGisMainClient::FindTerminalNumber(const char *strNumber)
{
    POSITION pos = NULL;
    if( m_TerminalInfoList.IsEmpty() )
    {
        return FALSE;
    }

    pos=m_TerminalInfoList.GetHeadPosition();
    while(pos) 
    { 
        //GetNext()���ص���posָ��ĵ�ǰ�ڵ㣬�����Ƿ�����һ���ڵ�,�ڷ��ص�ǰ�ڵ��ͬʱ��
        //pos��GetNext()�޸ĳ�ָ����һ���ڵ�
        TERMINALInfo &tempTermainalInfo = m_TerminalInfoList.GetNext(pos);  

        if( strcmp(tempTermainalInfo.strNumber,strNumber) == 0 )
        {
            return TRUE;
        }
    }
   
    return FALSE;
}

BOOL CGisMainClient::CheckSendState(CString strDeptNum)
{
	if(GetConnectState() == TCP_LOGIN_SUCCESS)
	{
		if(strDeptNum != "")
		{
			int nLen = m_strDeptNum.GetLength();
			if(nLen > strDeptNum.GetLength())
			{
				return FALSE;
			}
			strDeptNum = strDeptNum.Mid(0,nLen);
			int nRes = strDeptNum.Compare(m_strDeptNum);
			if(nRes == 0)
			{
				return TRUE;
			}
		}
		else
		{
			return TRUE;
		}
	}

	return FALSE;
}