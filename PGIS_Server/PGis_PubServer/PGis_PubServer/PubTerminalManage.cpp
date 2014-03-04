#include "StdAfx.h"
#include "PubTerminalManage.h"

extern int g_nSendTxtMsgInterval;
extern CWriteLogThread gWriteLogApp;
extern UINT g_dwDataAnalyseThreadID;

#define MAX_PUBNET_RECVICE_BUF          1024+256
extern CCriticalSection g_csPubNetRecvList;
extern CList<RECV_MESSAGE, RECV_MESSAGE&> g_PubNetRecvList;
///////////////////////////////////////////////////////////////////////////////////////////
//
CPubTerminalManage::CPubTerminalManage()
{
    m_nTimeOutValue = 0;

    m_nConnectState = CONNECT_NULL;
    m_bOldProtocol = FALSE;

	memset(m_csDeviceType,0,MAX_DEVICE_CODE_LEN);
    memset(m_csDeviceCode,0,MAX_DEVICE_CODE_LEN);
    memset(m_csDeptCode,0,MAX_DEVICE_CODE_LEN);

    m_pRecvBuf = NULL;
    m_pRecvBuf = new  char[MAX_PUBNET_RECVICE_BUF];
    m_nRecvLen  = 0;
	
	m_nSendMsgTxtTimeVal = 0;
	m_SendMsgWaitAck.bStart = FALSE;
    //����Ϣ���·�
    m_bSendGroupSetWaitAckStart = FALSE;
}

CPubTerminalManage::~CPubTerminalManage()
{
    if(m_pRecvBuf)
    {
        delete[] m_pRecvBuf;
        m_pRecvBuf = NULL;
    }
}

//��������״̬
void CPubTerminalManage::SetConnectState(CONNECT_STATE connectState,int nTimeOutValue)
{
    m_nConnectState = connectState;
    m_nNowTimerVal = 0;
    m_nTimeOutValue = nTimeOutValue;

    if(m_nConnectState == CONNECT_NULL)
    {
        ResetTerminalInfo();
    }
}

//��ȡ����״̬
int CPubTerminalManage::GetConnectState()
{
    return m_nConnectState;
}

//��ʱ�����,ÿ1s����һ��
int CPubTerminalManage::CheckTimerOut()
{
    if(m_nTimeOutValue == 0)
    {
        //����ⳬʱ
        return 0; 
    }
    
    switch(m_nConnectState)
    {
    case CONNECT_NULL:
        break;
    case CONNECT_WAIT_GPRSLOGIN:
        if(++m_nNowTimerVal >= m_nTimeOutValue)
        {
            m_nNowTimerVal = 0;

            return 1; //�ȴ���½��ʱ
        }
        break;
    case CONNECT_GPRSLOGIN_OK:
        if(++m_nNowTimerVal >= m_nTimeOutValue)
        {
            m_nNowTimerVal = 0; 

            return 2; //������ʱ
        }
        break;
    default:
        break;
    }

    return 0;
}

#ifdef WANGZH_H6 //2013-5-3
char * CPubTerminalManage::GetDeptNum()
{
	return m_csDeptCode;
}

void CPubTerminalManage::SetDeptNum(const char *csDeptNum)
{
	strcpy(m_csDeptCode,csDeptNum);
}
#endif

//��ȡ�ն��豸����
char * CPubTerminalManage::GetDeviceNum()
{
    return m_csDeviceCode;
}

//�����ն��豸����
void CPubTerminalManage::SetDeviceNum(const char *csDeviceNum)
{
    strcpy(m_csDeviceCode,csDeviceNum);
}

//��ȡ�ն�����
char * CPubTerminalManage::GetDeviceKind()
{
    return m_csDeviceType;
}

void CPubTerminalManage::SetDeviceKind(const char *csDeviceType)
{
    strcpy(m_csDeviceType,csDeviceType);
}

void CPubTerminalManage::ResetTerminalInfo()
{
    m_nConnectState = CONNECT_NULL;
    m_bOldProtocol = FALSE;
	memset(m_csDeviceType,0,MAX_DEVICE_CODE_LEN);
    memset(m_csDeviceCode,0,MAX_DEVICE_CODE_LEN);
    memset(m_csDeptCode,0,MAX_DEVICE_CODE_LEN);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
int CPubTerminalManage::CheckResendTempGroupSetTimerout()
{
    if(m_nConnectState == CONNECT_GPRSLOGIN_OK &&
        m_bSendGroupSetWaitAckStart)
    {
        if(++m_nGroupSetTimerVal >= m_nGroupSetTimeOutValue)
        {
            m_nGroupSetTimerVal = 0;
            m_nGroupSetSendCount++;
            if(m_nGroupSetSendCount > 3)
            {
                return 2;
            }
            else
            {
                return 1;
            }
        }
    }
    
    return 0;
}

//������̬������Ϣ�ظ���ʱ
BOOL CPubTerminalManage::StartGroupSetWaitAnswer(int nAckValue,const char *pcGroupNumber,BOOL bSetFlag,int nTimeOutValue)
{
    if(nTimeOutValue <= 0)
    {
        return FALSE;
    }
    int nLen = strlen(pcGroupNumber);
    if(nLen >= 16)
    {
        return FALSE;
    }

    m_bSendGroupSetWaitAckStart = TRUE;
    m_nGroupSetTimerVal = 0;
    
    m_nGroupSetTimeOutValue = nTimeOutValue;
    m_nGroupSetSendCount = 1;
    
    m_nGroupSetAckValue = nAckValue;
    
    memset(m_csGroupSetNum,0,16);
    
    
    memcpy(m_csGroupSetNum,pcGroupNumber,nLen);
    
    m_bGroupSetFlag = bSetFlag;
    
    return TRUE;
}

void CPubTerminalManage::StopGroupSetWaitAnswer()
{
    m_bSendGroupSetWaitAckStart = FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
int CPubTerminalManage::CheckResendMsgInfoTimerOut()
{
    if(m_nConnectState == CONNECT_GPRSLOGIN_OK && 
        m_SendMsgWaitAck.bStart)
    {
        if(++m_SendMsgWaitAck.nTimerVal >= m_SendMsgWaitAck.nTimeOutValue)
        {
            m_SendMsgWaitAck.nTimerVal = 0;
            
            
            m_SendMsgWaitAck.nSendCount++;
            if(m_SendMsgWaitAck.nSendCount > 3)
            {
                return 2;
            }
            else
            {
                return 1;
            }
        }
    }

    return 0;
}

//������Ϣ�ط�
BOOL CPubTerminalManage::StartSendMessageWaitAnswer(int nAckValue,int nSendMsgID,const char *pcSendMsgInfo,int nTimeOutValue)
{
    if(nTimeOutValue <= 0)
    {
        return FALSE;
    }
    m_SendMsgWaitAck.bStart = TRUE;
    m_SendMsgWaitAck.nTimerVal = 0;

    m_SendMsgWaitAck.nTimeOutValue = nTimeOutValue;
    m_SendMsgWaitAck.nSendCount = 1;
    m_SendMsgWaitAck.nSendMsgID = nSendMsgID;
    m_SendMsgWaitAck.nAckValue = nAckValue;
    memset(m_SendMsgWaitAck.MessageInfo,0,256);

    int nLen = strlen(pcSendMsgInfo);
    memcpy(m_SendMsgWaitAck.MessageInfo,pcSendMsgInfo,nLen);
    
    return TRUE;
}

//
void CPubTerminalManage::StopSendMessageWaitAnswer()
{
    m_SendMsgWaitAck.bStart = FALSE;
}

int CPubTerminalManage::GetSendMsgID()
{
    return m_SendMsgWaitAck.nSendMsgID;
}

const char * CPubTerminalManage::GetMessageInfo()
{
    return m_SendMsgWaitAck.MessageInfo;
}

int CPubTerminalManage::GetMsgAckValue()
{
    return m_SendMsgWaitAck.nAckValue;
}

BOOL CPubTerminalManage::SendMessageAnswerOkCheck(int nAckValue)
{
    if(!m_SendMsgWaitAck.bStart)
    {
        return FALSE;
    }

    if(m_SendMsgWaitAck.nSendMsgID <= 0)
    {
        return FALSE;
    }

    if(m_SendMsgWaitAck.nAckValue != nAckValue)
    {
        return FALSE;
    }

    return TRUE;
}

void CPubTerminalManage::AddMsgTxtToBuffer(int nMsgID, const char * pcsMsgTxt)
{
    if(m_nConnectState != CONNECT_GPRSLOGIN_OK)
    {
        return;
    }

    if(pcsMsgTxt == NULL)
    {
        return;
    }

    if(strlen(pcsMsgTxt) == 0)
    {
        return;
    }

    MessageTxtInfo tempTxtInfo;
    tempTxtInfo.nMsgID = nMsgID;
    memset(tempTxtInfo.TxtInfo,0,256);
    strcpy(tempTxtInfo.TxtInfo,pcsMsgTxt);

    m_MsgBufferList.AddTail(tempTxtInfo);
    
}

BOOL CPubTerminalManage::GetMsgTxtTimerCheck(char * pcsMsgTxtBuf,int &nMsgID)
{
    if(m_nConnectState != CONNECT_GPRSLOGIN_OK)
    {
        return FALSE;
    }

    if(++m_nSendMsgTxtTimeVal >= g_nSendTxtMsgInterval)
    {
        m_nSendMsgTxtTimeVal = 0;
        if(m_MsgBufferList.IsEmpty())
        {
            return FALSE;
        }

        MessageTxtInfo &tempTxtInfo = m_MsgBufferList.GetHead();

        strcpy(pcsMsgTxtBuf,tempTxtInfo.TxtInfo);

        nMsgID = tempTxtInfo.nMsgID;

        m_MsgBufferList.RemoveHead();

        return TRUE;
    }

    return FALSE;
    
}


#define MAIN_PUBNET_PACKAGE_LEN         4
#define MAX_PUBNET_PACKAGE_LEN          256
//
void CPubTerminalManage::DataParseProcess(int nSockID,const  char * recvData ,int nRecvLen) //change to unsigned char
{
    int nCountLen; 
    nCountLen = nRecvLen+m_nRecvLen;
    if(nCountLen > MAX_PUBNET_RECVICE_BUF)
    {
        m_nRecvLen = 0;
        return ;
    }
    memcpy(m_pRecvBuf+m_nRecvLen,recvData,nRecvLen);
    m_nRecvLen = nCountLen;
    int nPackageLen = 0;
    char *ptr = NULL;
    //�ְ�
    int nSpaceLen = m_nRecvLen;
    char *pCurPoint = m_pRecvBuf;
    while(nSpaceLen >= MAIN_PUBNET_PACKAGE_LEN)
    {                
        //pCurPoint = strstr(pCurPoint,m_strHead);
        //����ͷ
     char  *m_strHead =  "*HYT";
	
		unsigned short m_shortHead=0xAA;

/*
if(pCurPoint[0]!=m_shortHead)
{
	gWriteLogApp.WriteLog(FOR_ALARM,"%s [PublicNet]��ͷ��,û���ҵ�%d\n",
		CTime::GetCurrentTime().Format("%H:%M:%S"),
		m_shortHead);
	
	nSpaceLen = 0;
            break;
}
       
 */
        if(strncmp(pCurPoint,m_strHead,MAIN_PUBNET_PACKAGE_LEN) != 0)
        {
            gWriteLogApp.WriteLog(FOR_ALARM,"%s [PublicNet]��ͷ��,û���ҵ�%s\n",
                CTime::GetCurrentTime().Format("%H:%M:%S"),
                m_strHead);

                nSpaceLen = 0;
            break;
        }
      
        //���Ұ�β
        char m_cTailFlag = '#';
        ptr = const_cast<char*>(strchr((const char *)pCurPoint,m_cTailFlag));
	    /*	char m_chSize[4];
	    m_chSize[0]=pCurPoint[6];
	    m_chSize[1]=pCurPoint[7];
	    m_chSize[2]=pCurPoint[8];
	    m_chSize[3]=pCurPoint[9];
    	int m_iSize=*(int *)m_chSize;

  */
        if(ptr)
        {
            //��Ҫ����һ�ΰ�ͷ
            //һ�������İ�
            nPackageLen = ptr - (const char *)pCurPoint + 1; //(const char *) change
            if(nPackageLen < MAX_PUBNET_PACKAGE_LEN)
            {
                
                if(g_dwDataAnalyseThreadID > 0)
                {
                    //������������
                    RECV_MESSAGE tempRecvMessage;
                    memset(&tempRecvMessage.MessagePacket,0,MAX_GPS_MESSAGE_LEN);
                    tempRecvMessage.nClientID = nSockID;
                    tempRecvMessage.length = nPackageLen;
                    memcpy(tempRecvMessage.MessagePacket,pCurPoint,nPackageLen);
                    
                    g_csPubNetRecvList.Lock();
                    g_PubNetRecvList.AddTail(tempRecvMessage);
                    g_csPubNetRecvList.Unlock();
                    PostThreadMessage(g_dwDataAnalyseThreadID,WM_RECVICE_PUBGPS_MESSAGE,0,0);
                }
                
                
            }
            else
            {
                gWriteLogApp.WriteLog(FOR_ALARM,"%s [PubServer]�����ȳ�����������ֵ:len:%d\n",
                    CTime::GetCurrentTime().Format("%H:%M:%S"),nPackageLen);
            }

            //ʣ�µ����ݼ����ְ�
            pCurPoint = ( char *)ptr + 1;
            nSpaceLen = nSpaceLen - nPackageLen;
        }
        else
        {
            printf("[PubServer]:û���ҵ���β.\n");
            if(nSpaceLen > MAX_PUBNET_PACKAGE_LEN)
            {
                gWriteLogApp.WriteLog(FOR_ALARM,"%s [PubServer]�����ȳ�����������ֵ:len:%d\n",
                    CTime::GetCurrentTime().Format("%H:%M:%S"),nPackageLen);

                    nSpaceLen = 0;
            }
            break;
        }
    }//endwhile

    //�Ƿ���û�д��������
    if(nSpaceLen > 0)
    {
        m_nRecvLen = nSpaceLen;
        if(pCurPoint != m_pRecvBuf)
        {
            memcpy(m_pRecvBuf,pCurPoint,m_nRecvLen);
            m_pRecvBuf[m_nRecvLen] = 0;
        }
    }
    else
    {
        m_nRecvLen = 0;
    }


}