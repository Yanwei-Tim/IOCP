//
#include "StdAfx.h"
#include "PubNetControl.h"
#include "rwini.h"
#include "TcpInterface.h"
///////////////////////////////////////////////////////////////////////////////////////////
CWriteLogThread gWriteLogApp;

extern int gRecvGpsCount; //����GPS����ͳ��
extern int gSendGpsCount; //�����ط���GPS����ͳ��

int g_nSendTxtMsgInterval = 2; //�����ı�ССʱ��������ʼ��ʱ�������ļ��ж�ȡ

UINT g_dwDataAnalyseThreadID = 0;
//�ٽ���
CCriticalSection g_csPubNetRecvList;
CList<RECV_MESSAGE, RECV_MESSAGE&> g_PubNetRecvList;

extern const char *g_pcFtpDownFileName; //��Ҫ��FTP���������ļ�

///////////////////////////////////////////////////////////////////////////////////////////////
//
#define DllExport   __declspec( dllexport )

#pragma comment(lib, "cpsx0.lib")

// ����CPSX��l/m/s, �����: ��ʼ����/������/��ʼ���/�����
extern "C" DllExport int CountCpsxSchemeArry( 
                                             WORD pCpsx[60][4], 
                                             UINT l, UINT m, UINT s); 


// ����ָ��CPSX������������
extern "C" DllExport UINT GetCpsxMaxGroupIdent1(
                                                UINT iTeam, 
                                                UINT l, UINT m, UINT s);
// ----------------------------------------------------------
extern "C" DllExport int GetCpsxMaxGroupIdent(
                                              UINT *maxID, UINT iTeam, 
                                              UINT l, UINT m, UINT s);
//extern "C" DllExport UINT GetCpsxMaxGroupIdent(...);


// ����ָ��CPSX������������
extern "C" DllExport UINT GetCpsxMaxIndvIdent(
                                              UINT iTeam, 
                                              UINT l, UINT m, UINT s);


// �������� ID ��Ӧ�� �Ӻ�/�û��� ������ʾ(��"20201")
extern "C" DllExport int GetCpsxFltIdText(char cIdent[], 
                                          WORD iIdent, 
                                          UINT l, UINT m, UINT s); 

// ����������ʾ�ַ���(��"20201"), ����CPSX��EEPROM����
extern "C" DllExport WORD GetCpsxIdValueOfText1( 
                                                char *strUserID, 
                                                UINT l, UINT m, UINT s); 
// -------------------------------------------------
extern "C" DllExport int GetCpsxIdValueOfText( 
                                              WORD *pUserID, 
                                              char *strUserID, 
                                              UINT l, UINT m, UINT s); 

///////////////////////////////////////////////////////////////////////////////////////////////
//��������Э����������
struct PublicUpProtocalInfo
{
    int code;
    char *description;
};

static PublicUpProtocalInfo UpProtocalDescriptions[] =
{
    {GPS_TINMEROUT_REPORT,"��ʱ�ϴ�GPS��Ϣ"}, //0x00
    {GPS_MONITOR_RESPONSE,"��ʱ����GPS��Ϣ"},
    {GPS_BOUNDCALL_RESPONSE,"�������GPS��Ϣ"}, //0x02
    {SEND_MESSAGE_ANSWER,"������Ϣ�ظ���Ϣ"}, //0x03
    {0x04,NULL},{0x05,NULL},{0x06,NULL},{0x07,NULL},
    {0x08,NULL},{0x09,NULL},{0x0a,NULL},{0x0b,NULL},
    {0x0c,NULL},{0x0d,NULL},{0x0e,NULL},{0x0f,NULL},
    
    {TERMINAL_LOGIN_REPORT,"GPRS���ӵ�¼Э��"}, //0x10
    {TERMINAL_OPEN_REPORT,"������ϢЭ��"},
    {GPS_TERMINAL_OPEN_REPORT,"����GPS��ϢЭ��"},
    {TCPCONNECT_TICK_REPORT,"������Ϣ��ϢЭ��"},
    {PRIVATENET_LINEON_REPORT,"ר������Э��"},
    {PRIVATENET_LINEOFF_REPORT,"ר������Э��"},
    {QUERY_MY_POSITION,"�ϴ������ѯ�ҵ�λ����ϢЭ��"},
    {QUERY_DESTINATION_POSITION,"�ϴ������ѯĿ�ĵؾ�γ��Э��"}, //0x17
    {CTL_TERMINAL_MOUNTGUARD_RESPONSE,"�ն��ϰ��°�"}, //0x18
    
    {GPS_HISTORY_REPORT,"��ʷGPS����"},
    {0x1A,NULL},{0x1B,NULL},
    {0x1C,NULL},{0x1D,NULL},{0x1E,NULL},{0x1F,NULL},
    
    {CTL_PRIVATENET_LINE_RESPONSE,"Ӧ�����ר�����ߺ�ר����������"}, //0x20
    {CTL_TERMINAL_WAKEUP_RESPONSE,"Ӧ������ն˼�Ⱥң�к�ҡ��"}, //0x21
    
    {0x22,NULL},{0x23,NULL},
    {0x24,NULL},{0x25,NULL},{0x26,NULL},{0x27,NULL},
    
    {0x28,NULL},{0x29,NULL},{0x2A,NULL},{0x2B,NULL},
    {0x2C,NULL},{0x2D,NULL},{0x2E,NULL},{0x2F,NULL},
    
    {GPS_ALERTING_REPORT,"����Э��"}, //0x30
    {0x31,NULL},{0x32,NULL},{0x33,NULL},
    {0x34,NULL},{0x35,NULL},{0x36,NULL},{0x37,NULL},
    {0x38,NULL},{0x39,NULL},{0x3A,NULL},{0x3B,NULL},
    {0x3C,NULL},{0x3D,NULL},{0x3E,NULL},{0x3F,NULL},
    
    {0x40,NULL},{0x41,NULL},{0x42,NULL},{0x43,NULL},
    {0x44,NULL},{0x45,NULL},{0x46,NULL},{0x47,NULL},
    {0x48,NULL},{0x49,NULL},{0x4A,NULL},{0x4B,NULL},
    {0x4C,NULL},{0x4D,NULL},{0x4E,NULL},{0x4F,NULL},
    
    {0x50,NULL},{0x51,NULL},{0x52,NULL},{0x53,NULL},
    {0x54,NULL},{0x55,NULL},{0x56,NULL},{0x57,NULL},
    {0x58,NULL},{0x59,NULL},{0x5A,NULL},{0x5B,NULL},
    {0x5C,NULL},{0x5D,NULL},{0x5E,NULL},{0x5F,NULL},
    
    {SET_COMMUNICATION_MODEL_RESPONSE,"Ӧ������ͨѶ��ʽ����"}, //0x60
    {SET_GPRS_PARAMETER_RESPONSE,"Ӧ������GPRS����"},
    {SET_GPS_PARAMETER_RESPONSE,"Ӧ������GSM����"},
    {SET_TERMINAL_NUM_RESPONSE,"Ӧ�������ն�ר������"},
    {SET_TEMP_NUM_RESPONSE,"Ӧ�����ö�̬��ʱ�������"},
    {SET_ALERT_NUM_RESPONSE,"Ӧ������ר�����Ÿ澯����"},
    {SET_SAFETYCHECK_NUM_RESPONSE,"Ӧ������ר���������"},
    {SET_GPSCENTER_NUM_RESPONSE,"Ӧ������ר��GPS���ĺ���"},
    {SET_BACK_MODEL_RESPONSE,"Ӧ������ר������ģʽ"}, //0x68
    {0x69,NULL},{0x6A,NULL},{0x6B,NULL},
    {0x6C,NULL},{0x6D,NULL},{0x6E,NULL},{0x6F,NULL},
    
    {0x70,NULL},{0x71,NULL},{0x72,NULL},{0x73,NULL},
    {0x74,NULL},{0x75,NULL},{0x76,NULL},{0x77,NULL},
    {0x78,NULL},{0x79,NULL},{0x7A,NULL},{0x7B,NULL},
    {0x7C,NULL},{0x7D,NULL},{0x7E,NULL},{0x7F,NULL},
    
    {QUERY_COMMUNICATION_MODEL_RESPONSE,"Ӧ���ȡͨѶ��ʽ����"}, //0x80
    {QUERY_GPS_PARAMETER_RESPONSE,"Ӧ���ȡGPRS����"},
    {QUERY_GSM_PARAMETER_RESPONSE,"Ӧ���ȡGSM����"},
    {QUERY_TERMINAL_NUM_RESPONSE,"Ӧ���ȡ�ն�ר������"},
    {QUERY_TEMP_NUM_RESPONSE,"Ӧ���ȡ��̬��ʱ�������"},
    {QUERY_ALERT_NUM_RESPONSE,"Ӧ���ȡר�����Ÿ澯����"},
    {QUERY_SAFETYCHECK_NUM_RESPONSE,"Ӧ���ȡר���������"},
    {QUERY_GPSCENTER_NUM_RESPONSE,"Ӧ���ȡר��GPS���ĺ���"},
    {QUERY_BACK_MODEL_RESPONSE,"Ӧ���ȡר������ɨ��ģʽ"},
    {QUERY_MAGNETIC_BOUND_RESPONSE,"Ӧ���ȡר����ǿ����"},
    {QUERY_TERMINAL_TYPE_RESPONSE,"Ӧ���ȡ�ն˻��Ͳ���"},
    {QUERY_WAKEUP_RESPONSE,"Ӧ���ȡ�ն�ң��ҡ��״̬"},
    {QUERY_GPS_SEND_TINMEROUT,"Ӧ���ȡ�ն˹̶��ϴ�GPS��Ϣʱ����"}
    
};
//
///////////////////////////////////////////////////////////////////////////////////////////////
//
//���IP��ַ��ʽ�Ƿ���ȷ
BOOL CPubNetControl::CheckIPAddressFormat(const char *strIPAddress)
{
    int IP[4];
    int i = 0;
    for(i = 0 ; i < 4; i++)
    {
        IP[i] = -1;
    }
    sscanf(strIPAddress, "%d.%d.%d.%d",IP,IP+1,IP+2,IP+3);
    for(i = 0 ; i < 4; i++)
    {
        if(IP[i] == -1 || IP[i] > 255)
        {
            return FALSE;
        }
    }
    
    return TRUE;
}

//ȡ��У��ֵ
int CPubNetControl::GetChksum(const char *strMsg)
{
    int nChk ;
    int nLen = strlen(strMsg);
    nChk = 0;
    for(int i = 0; i < nLen; i++ )
    {
        nChk = nChk ^ strMsg[i];
    }
    
    return nChk;
}

//
//ת��������
BOOL CPubNetControl::GetCpsxIdValue(const char *strValue, char *strOut)
{
    if(strlen(strValue) != 8)
    {
        return FALSE;
    }
    
    char tempBuf[16];
    
    int nHeadValue;
    
    sscanf(strValue,"%3d%s",&nHeadValue,tempBuf);
    
    nHeadValue = nHeadValue - 200;
    if(nHeadValue < 0 || nHeadValue > 127 )
    {
        return FALSE;
    }
    
    WORD wId = 0; 
    int nTemp = GetCpsxIdValueOfText(&wId, tempBuf, m_nL, m_nM, m_nS); 
    
    if(!nTemp)
    {
        return FALSE;
    }
    sprintf(strOut,"%.2X%.4X",nHeadValue,wId);
    
    return TRUE;
}

CString CPubNetControl::GetCpsxFltId(const char *strValue)
{
    CString strRetrun = _T(" ");
    
    char tempBuf[16];
    if(strlen(strValue) != 6)
    {
        return strRetrun;
    }
    
    int nHeadValue;
    int wId = 0;
    sscanf(strValue,"%2X%x",&nHeadValue,&wId);
    if(nHeadValue < 0 || nHeadValue > 127 )
    {
        return strRetrun;
    }
    nHeadValue = nHeadValue + 200;
    
    memset(tempBuf,0,16);
    int nTemp = GetCpsxFltIdText(tempBuf, wId, m_nL, m_nM, m_nS);
    if(!nTemp)
    {
        return strRetrun;
    }
    
    wId = atoi(tempBuf); 
    strRetrun.Format("%.3d%.5d",nHeadValue,wId);
    
    return strRetrun;
}

BOOL CPubNetControl::GetCpsxFltId(const char *strValue, char *strOut)
{
    char tempBuf[16];
    if(strlen(strValue) != 6)
    {
        return FALSE;
    }
    int nHeadValue;
    int wId = 0;
    sscanf(strValue,"%2X%x",&nHeadValue,&wId);
    if(nHeadValue < 0 || nHeadValue > 127 )
    {
        return FALSE;
    }
    nHeadValue = nHeadValue + 200;
    
    memset(tempBuf,0,16);
    int nTemp = GetCpsxFltIdText(tempBuf, wId, m_nL, m_nM, m_nS);
    if(!nTemp)
    {
        return FALSE;
    }
    
    wId = atoi(tempBuf); 
    sprintf(strOut,"%.3d%.5d",nHeadValue,wId);
    return TRUE;
    
}

///////////////////////////////////////////////////////////////////////////////////////////
//
CPubNetControl::CPubNetControl()
{
    WSADATA wsaData;
    WORD     wVersion;
    wVersion = MAKEWORD(2,2);
    
    //����Socket��Դ
    int nRes  = WSAStartup(wVersion,&wsaData);//�汾��Ϣ
    if(nRes != 0)
    {
        WSACleanup();
        TRACE("TcpServer no Sock dll to use!!\n");	
    }
    
    if((LOBYTE(wsaData.wVersion) != 2) || (HIBYTE(wsaData.wVersion) != 2))//����WINSOCK��DLL
    {
        WSACleanup();
        TRACE("TcpServer no right Sock dll!!\n");
    }
    
    m_bThreadStart = FALSE;
    
    m_dwDataAnalyseThreadID = 0;
    m_hDataAnalyseThread = NULL;
    m_hDataAnalyseThreadEndEvt = NULL;
    
    m_dwDataDispatchThreadID = 0;
    m_hDataDispatchThread = NULL;
    m_hDataDispatchThreadEndEvt = NULL;
    
    int i;
    for(i = 0; i < MAX_PUB_PROTOCAL_DOWN; i++)
    {
        m_nAckCount[i] = 0;
    }
    
    memset(m_LocalIpAddress,0,MAX_IPADDRESS_LEN);
    
    m_SockPubNetServer.InitRoutineServer(MAX_PUBLICNET_CLIENT_CONNECT);
    
    
    m_bStartFlag = FALSE;
    
    
    InitFieldStatisticTable();
}

CPubNetControl::~CPubNetControl()
{
    m_SockPubNetServer.ReleaseRoutineServer();
    WSACleanup();
}

///////////////////////////////////////////////////////////////////////////////////////////
//�汾��Ϣ
void CPubNetControl::InitMyVersion()
{
    char constVersion[128] = {0};
    
    _snprintf(constVersion, 128-1, "hyt����GPS����ϵͳ Version:%s, Build Time:%s %s", 
        "1.2.0.6", __DATE__, __TIME__);
    
    gWriteLogApp.WriteLog(FOR_ALARM,"%s [SYS]%s \n",CTime::GetCurrentTime().Format("%H:%M:%S"),constVersion);
}

//��ʼ��Э���ֶθ���,�����ж�Э���ʽ�Ƿ���ȷ
void CPubNetControl::InitFieldStatisticTable()
{
    for(int i = 0; i < MAX_PROTOCOL_UP; i++)
    {
        m_FieldStatisticTB[i] = 0xFF;
    }

    m_FieldStatisticTB[TERMINAL_LOGIN_REPORT] = 8;  //GPRS���ӵ�-10
    m_FieldStatisticTB[TERMINAL_OPEN_REPORT] = 9;   //������ϢЭ��-11
    m_FieldStatisticTB[GPS_TERMINAL_OPEN_REPORT] = 15; //����GPS��ϢЭ��-12
    m_FieldStatisticTB[TCPCONNECT_TICK_REPORT] = 6;  //������ϢЭ��-13

    m_FieldStatisticTB[PRIVATENET_LINEON_REPORT] = 16;  //ר������Э��-14
    m_FieldStatisticTB[PRIVATENET_LINEOFF_REPORT] = 16; //ר������Э��-15

    m_FieldStatisticTB[GPS_TINMEROUT_REPORT] = 0xFF;//14; //�̶�ʱ���������ϴ�GPS��Ϣ-00  //kevin 2009-09-11

    m_FieldStatisticTB[GPS_MONITOR_RESPONSE] = 14; //Ӧ��ʱ����GPSЭ��-01
    m_FieldStatisticTB[GPS_BOUNDCALL_RESPONSE] = 14;  //Ӧ���������GPSЭ��-02
    m_FieldStatisticTB[SEND_MESSAGE_ANSWER] = 0xFF; //

    m_FieldStatisticTB[QUERY_MY_POSITION] = 15;            //�ϴ������ѯ�ҵ�λ����ϢЭ��-16
    m_FieldStatisticTB[QUERY_DESTINATION_POSITION] = 5;      //�ϴ������ѯĿ�ĵؾ�γ��Э��-17
    m_FieldStatisticTB[CTL_TERMINAL_MOUNTGUARD_RESPONSE] = 16;//Ӧ������ն��ϰ��°�-18
    
    m_FieldStatisticTB[CTL_PRIVATENET_LINE_RESPONSE] = 7; //Ӧ�����ר�����ߺ�ר����������-20
    m_FieldStatisticTB[CTL_TERMINAL_WAKEUP_RESPONSE] = 7; //Ӧ������ն˼�Ⱥң�к�ҡ��-21
    m_FieldStatisticTB[CTL_TERMINAL_MOUNTGUARD_RESPONSE] = 16; //�ն����ϰࡢ�°�-22

    m_FieldStatisticTB[GPS_ALERTING_REPORT] = 15; //����Э��-30

    m_FieldStatisticTB[SET_COMMUNICATION_MODEL_RESPONSE] = 6;//Ӧ������ͨѶ��ʽ����-60
    m_FieldStatisticTB[SET_GPRS_PARAMETER_RESPONSE] = 6;//Ӧ������GPRS����-61
    m_FieldStatisticTB[SET_GPS_PARAMETER_RESPONSE] = 6; //Ӧ������GSM����-62

    m_FieldStatisticTB[SET_TERMINAL_NUM_RESPONSE] = 6; //Ӧ�������ն�ר������-63
    m_FieldStatisticTB[SET_TEMP_NUM_RESPONSE] = 7; //6 //Ӧ�����ö�̬��ʱ�������-64 //kevin 2009-09-11
    m_FieldStatisticTB[SET_ALERT_NUM_RESPONSE] = 6;//Ӧ������ר�����Ÿ澯����-65
    m_FieldStatisticTB[SET_SAFETYCHECK_NUM_RESPONSE] = 6; //Ӧ������ר���������-66
    m_FieldStatisticTB[SET_GPSCENTER_NUM_RESPONSE] = 6;//Ӧ������ר��GPS���ĺ���-67
    m_FieldStatisticTB[SET_BACK_MODEL_RESPONSE] = 6	; //Ӧ������ר������ģʽ-68

    m_FieldStatisticTB[QUERY_COMMUNICATION_MODEL_RESPONSE] = 7;//Ӧ���ȡͨѶ��ʽ����-80
    m_FieldStatisticTB[QUERY_GPS_PARAMETER_RESPONSE] = 11; //Ӧ���ȡGPRS����-81
    m_FieldStatisticTB[QUERY_GSM_PARAMETER_RESPONSE] = 8; //Ӧ���ȡGSM����-82

    m_FieldStatisticTB[QUERY_TERMINAL_NUM_RESPONSE] = 9; //Ӧ���ȡ�ն�ר������-83
    m_FieldStatisticTB[QUERY_TEMP_NUM_RESPONSE] = 9; //Ӧ���ȡ��̬��ʱ�������-84
    m_FieldStatisticTB[QUERY_SAFETYCHECK_NUM_RESPONSE] = 9; //Ӧ���ȡר���������-86 

    m_FieldStatisticTB[QUERY_ALERT_NUM_RESPONSE] = 9;//Ӧ���ȡר�����Ÿ澯����-85
    m_FieldStatisticTB[QUERY_GPSCENTER_NUM_RESPONSE] = 9; //Ӧ���ȡר��GPS���ĺ���-87
    m_FieldStatisticTB[QUERY_BACK_MODEL_RESPONSE] = 9 ; //Ӧ���ȡר������ɨ��ģʽ-88

    m_FieldStatisticTB[QUERY_MAGNETIC_BOUND_RESPONSE] = 7; //Ӧ���ȡר����ǿ����-89
    m_FieldStatisticTB[QUERY_TERMINAL_TYPE_RESPONSE] = 10; //Ӧ���ȡ�ն˻��Ͳ���-8A
    m_FieldStatisticTB[QUERY_GPS_SEND_TINMEROUT] = 7;
    m_FieldStatisticTB[QUERY_WAKEUP_RESPONSE] = 7; //Ӧ���ȡ�ն�ң��ҡ��״̬-8B

}

//��ȡϵͳ��������
BOOL CPubNetControl::LoadSystemConfig()
{
    Crwini rwIniTemp;

    CString tempstr =_T("hytPubNetCfg.ini");

    if(!rwIniTemp.CheckIniFile(tempstr))
    {
        gWriteLogApp.WriteLog(FOR_ALARM,"%s [SYS]ϵͳ����ʧ��,�Ҳ��������ļ�\n",CTime::GetCurrentTime().Format("%H:%M:%S"));

        return FALSE;
    }

    //Ĭ������GPS�����ϴ�ʱ����
    m_nInitFixGpsDt = 60;
    m_nInitFixGpsDt = rwIniTemp.GetIntValue(_T("SystemSet"),_T("DefaultFixGpsDt"),tempstr);
    if(m_nInitFixGpsDt <= 0)
    {
        m_nInitFixGpsDt = 60;
    }

    //GSP����
    m_nMonitorGPSCount = rwIniTemp.GetIntValue(_T("SystemSet"),_T("MonitorGPSCount"),tempstr);
    if(m_nMonitorGPSCount <= 0)
    {
        m_nMonitorGPSCount = 0;
    }

    //��GPRS���ӵ�¼��GPS����ģʽ
    m_nOpenGpsModel = rwIniTemp.GetIntValue(_T("SystemSet"),_T("OpenGpsModel"),tempstr); 
    if(m_nOpenGpsModel == -1)
    {
        m_nOpenGpsModel = 2;
    }

    m_nPubNetServerPort = rwIniTemp.GetInt(_T("TCPSet"),_T("PubNetServerPort"),tempstr);
    if(m_nPubNetServerPort == 0)
    {
        m_nPubNetServerPort = 5002;
    }

    if(rwIniTemp.ReadString(_T("TCPSet"),_T("PubNetServerIP"),tempstr,m_LocalIpAddress) == 0) 
    {
        //return FALSE;
    }
    
    m_nLoginTimeOut = rwIniTemp.GetInt(_T("SystemSet"),_T("LoginTimeOut"),tempstr);
    m_nConnTickTimeOut = rwIniTemp.GetInt(_T("SystemSet"),_T("ConnTickTimeOut"),tempstr);

    m_bDispatchZeroGPS = FALSE;
    if(rwIniTemp.GetInt(_T("SystemSet"),_T("DispatchZeroGPS"),tempstr) > 0)
    {
        m_bDispatchZeroGPS = TRUE;
    }
    
    //��ȡ������ת������
    m_nL = rwIniTemp.GetInt(_T("SetCpsx"),_T("L"),tempstr);
    m_nM = rwIniTemp.GetInt(_T("SetCpsx"),_T("M"),tempstr);
    m_nS = rwIniTemp.GetInt(_T("SetCpsx"),_T("S"),tempstr);

    m_nSendTxtMsgWaitAckTimeOut = rwIniTemp.GetInt(_T("SystemSet"),_T("SendTxtMsgWaitAckTimeOut"),tempstr);

    gWriteLogApp.WriteLog(FOR_ALARM,"%s [SYS]�������ļ��ɹ� \n",CTime::GetCurrentTime().Format("%H:%M:%S"));

    //����������Ϣʱ����
    g_nSendTxtMsgInterval = rwIniTemp.GetInt(_T("SystemSet"),_T("SendTxtMsgInterval"),tempstr);
    if(g_nSendTxtMsgInterval <= 0 )
    {
        g_nSendTxtMsgInterval = 1;
    }
    
    //ftpDownLoadĿ¼
    char tempPath[MAX_PATH] ={0};
    if(rwIniTemp.ReadString(_T("FTPSet"),_T("FTPDownLoadPatch"),tempstr,tempPath) == 0) 
    {
        //û��ָ�������FTP�����������ļ������ڵ�ǰĿ¼��FTPConveyĿ¼��
        sprintf(m_strFTPDownLoadFileName,"%sFTPConvey\\%s",rwIniTemp.GetCurrPath(),g_pcFtpDownFileName);
    }
    else
    {
        if(strlen(tempPath) == 0)
        {
            sprintf(m_strFTPDownLoadFileName,"%sFTPConvey\\%s",rwIniTemp.GetCurrPath(),g_pcFtpDownFileName);
        }
        else
        {
            sprintf(m_strFTPDownLoadFileName,"%s\\%s",tempPath,g_pcFtpDownFileName);
        }
    }

    //��ϵͳ����
    return TRUE;

}

BOOL CPubNetControl::StartThread()
{
    if(m_bThreadStart)
    {
        TRACE(_T("System thread has run.\n"));
        return TRUE;
    }
    
    //ϵͳ�����߳�
    if(m_hDataAnalyseThreadEndEvt == 0)
    {
        m_hDataAnalyseThreadEndEvt = CreateEvent(NULL,TRUE,TRUE,NULL);
    }
    
    m_hDataAnalyseThread = (HANDLE)_beginthreadex( NULL,0,&DataAnalyseThreadProc,this, 0,&m_dwDataAnalyseThreadID );
    if(!m_hDataAnalyseThread)
    {
        TRACE(_T("Strat System thread fail.\n"));
        return FALSE;
    }
    
    if( m_hDataDispatchThreadEndEvt == 0 )
    {
        m_hDataDispatchThreadEndEvt = CreateEvent(NULL,TRUE,TRUE,NULL);
    }
    
    m_hDataDispatchThread = (HANDLE)_beginthreadex( NULL,0,&DataDispatchThreadProc,this, 0,&m_dwDataDispatchThreadID);
    if(!m_hDataDispatchThread)
    {
        return FALSE;
    }
    
    g_dwDataAnalyseThreadID = m_dwDataAnalyseThreadID;

    m_bThreadStart = TRUE;
    
    return TRUE;
}

void CPubNetControl::StopThread()
{
    if( m_bThreadStart )
    {
        m_bThreadStart = FALSE;
        //��ֹϵͳ�����߳�
        PostThreadMessage(m_dwDataAnalyseThreadID, WM_THREAD_STOP, 0L, 0L);
        ::WaitForSingleObject(m_hDataAnalyseThreadEndEvt, INFINITE);
        m_dwDataAnalyseThreadID = 0;
        
        g_dwDataAnalyseThreadID = 0;
        
        CloseHandle(m_hDataAnalyseThreadEndEvt);
        m_hDataAnalyseThreadEndEvt = NULL;
        
        //��ֹ�߳�
        PostThreadMessage(m_dwDataDispatchThreadID, WM_THREAD_STOP, 0L, 0L);
        ::WaitForSingleObject(m_hDataDispatchThreadEndEvt, INFINITE);
        m_dwDataDispatchThreadID = 0;
        
        CloseHandle(m_hDataDispatchThreadEndEvt);
        m_hDataDispatchThreadEndEvt = NULL;
        
        m_hDataAnalyseThread = NULL;
    }
}

BOOL CPubNetControl::StartPublicTcpServer()
{
    int nRes;
    if(CheckIPAddressFormat(m_LocalIpAddress))
    {
        nRes = m_SockPubNetServer.Start(m_nPubNetServerPort,m_LocalIpAddress,TcpServerPublicCallBack,(DWORD)this);
    }
    else
    {
        nRes = m_SockPubNetServer.Start(m_nPubNetServerPort,NULL, TcpServerPublicCallBack,(DWORD)this);
    }
    
    if(nRes != TCPSERVER_STARTSUCCESS)
    {
        gWriteLogApp.WriteLog(FOR_ALARM,"%s [SYS]�������������ն�Socket����ʧ��,res:%d \n",
            CTime::GetCurrentTime().Format("%H:%M:%S"),nRes);
        
        return FALSE;
    }
    
    gWriteLogApp.WriteLog(FOR_ALARM,"%s [SYS]�������������ն�Socket���ӳɹ�,�˿�:%d \n",
        CTime::GetCurrentTime().Format("%H:%M:%S"),m_nPubNetServerPort);
    
    return TRUE;
}

//ֹͣTCP Server
void CPubNetControl::StopPublicTcpServer()
{
    for(int i = 0; i < MAX_PUBLICNET_CLIENT_CONNECT; i++)
    {
        if(m_tbPubTerminalInfo[i].GetConnectState() != CONNECT_NULL )
        {
            m_tbPubTerminalInfo[i].SetConnectState(CONNECT_NULL);
            
            m_SockPubNetServer.CloseClient(i);
        }
    }
    
    m_SockPubNetServer.Close();
}

//������־
void CPubNetControl::SetLog()
{
    Crwini rwIniTemp;
    
    CString tempstr =_T("hytPubNetCfg.ini");
    
    if(rwIniTemp.CheckIniFile(tempstr))
    {
        //��־����
        int nTempV;
        nTempV = rwIniTemp.GetInt(_T("SystemSet"),_T("LogLevel"),tempstr);
        gWriteLogApp.SetLogLevel(nTempV);
        
        nTempV = rwIniTemp.GetInt(_T("SystemSet"),_T("MonitorGPSCount"),tempstr);
        gWriteLogApp.SetLogFileCleanupDate(nTempV);
    }
}

//ϵͳ����
BOOL CPubNetControl::StartHYTPubNetSystem()
{
    if(m_bStartFlag)
    {
        return TRUE;
    }

    InitMyVersion();
    //��ȡ�����ļ�
    if(!LoadSystemConfig())
    {
        gWriteLogApp.WriteLog(FOR_ALARM,"%s [SYS]��ȡ�����ļ�ʧ��\n",
            CTime::GetCurrentTime().Format("%H:%M:%S"));
        
        return FALSE;
    }
    
    SetLog();
    
    
    //�����߳�
    if(!StartThread())
    {
        return FALSE;
    }
    
    //�������������ն˵�GPRS����
    if(!StartPublicTcpServer())
    {
        StopThread();
        
        return FALSE;
    }
    
    //����FTP����ģ��
    if(!m_myFtpConveyFile.StartFTPConvey(m_dwDataAnalyseThreadID))
    {
         StopThread();
        return FALSE;
    }
    
    m_bStartFlag = TRUE;
   
    return TRUE;
}

//ϵͳֹͣ
BOOL CPubNetControl::StopHYTPubNetSystem()
{
    if(m_bStartFlag)
    {
        gWriteLogApp.WriteLog(FOR_ALARM,"%s [SYS]������������������˳�����,������GPS����:%d(��),�����ط���GPS����:%d\n",
            CTime::GetCurrentTime().Format("%H:%M:%S"),gRecvGpsCount,gSendGpsCount);
        
        StopPublicTcpServer();
        
        StopThread();

        m_myFtpConveyFile.StopFTPConvey();
        
        m_bStartFlag = FALSE;
    }

    return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////
//
//TCP�ͻ���:�������ݽ���
DWORD WINAPI CPubNetControl::TcpServerPublicCallBack(UINT uMsg,DWORD dwParam,DWORD dwInstance)
{
    char *ptr = NULL;
    int nSpaceLen = 0;
    char *pCurPoint = NULL;
    int nPackageLen = 0;
    
    CPubNetControl *pHytGpsPublicServer = (CPubNetControl *)dwInstance;
    TcpServerClient *pServerClient = (TcpServerClient *)dwParam;
    
    static char recvPublicData[MAX_RECVICE_BUFFER] = {0};
    static int nCountRecvLen = 0;
    
    static BOOL bListFull =  FALSE;
    switch(uMsg)
    {
    case TCPSERVER_EVENT_CLIENTJOIN:
        pHytGpsPublicServer->TcpClientConnected(SOCKET_SERVER_PUBLICNET_ID,
            pServerClient->nID,
            pServerClient->szIP,
            pServerClient->nPort);
        
        break;
    case TCPSERVER_EVENT_CLIENTEXIT:
        pHytGpsPublicServer->TcpClientDisconnect(SOCKET_SERVER_PUBLICNET_ID,
            pServerClient->nID,
            pServerClient->szIP,
            pServerClient->nPort);
        
        break;
    case TCPSERVER_EVENT_RECVDATA: 
        if(pServerClient != NULL && pHytGpsPublicServer != NULL)
        {
            pHytGpsPublicServer->PublicNetDataAnalyse(pServerClient->nID,
                pServerClient->pszRecvData,
                pServerClient->nRecvDataLen);
        }
        break;
    default:
        break;
    }
    
    return 0;
}

//TCP Server��������
void CPubNetControl::TcpClientConnected(int typeID,int nClientID,const char *strIP,int nPort)
{
    switch(typeID)
    {
    case SOCKET_SERVER_PUBLICNET_ID:
        gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]�����ն˽���ɹ�,IP��ַ:%s,�˿�:%d\n",
            CTime::GetCurrentTime().Format("%H:%M:%S"),
            nClientID,
            strIP,
            nPort);
        
        //����״̬Ϊ�ȴ���½
        m_tbPubTerminalInfo[nClientID].SetConnectState(CONNECT_WAIT_GPRSLOGIN,m_nLoginTimeOut);
        
        break;
    default:
        break;
    }
}

//TCP Client���ӶϿ�
void CPubNetControl::TcpClientDisconnect(int typeID,int nClientID,const char *strIP,int nPort,int nExitCode)
{
    switch(typeID)
    {
    case SOCKET_SERVER_PUBLICNET_ID:
        gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]�����ն�TCP���ӶϿ�,IP��ַ:%s,�˿�:%d\n",
            CTime::GetCurrentTime().Format("%H:%M:%S"),
            nClientID,
            strIP,nPort);
        
        if(m_tbPubTerminalInfo[nClientID].GetConnectState() == CONNECT_GPRSLOGIN_OK)
        {
            gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]�ն�GPRS���ӵ�¼ȡ��,DeviceCode:%s\n",
                CTime::GetCurrentTime().Format("%H:%M:%S"),
                nClientID,
                m_tbPubTerminalInfo[nClientID].GetDeviceNum());

            //д�ļ��������ն�����
            char tempBuf[MAX_GPS_MESSAGE_LEN] = {0};
            sprintf(tempBuf,"%s,%d,0",m_tbPubTerminalInfo[nClientID].GetDeviceNum(),m_tbPubTerminalInfo[nClientID].GetDeviceKind());
            ReportDataToGisMain(A_TYPE_TERMINAL_LINE,tempBuf,strlen(tempBuf));
        }
        
        m_tbPubTerminalInfo[nClientID].SetConnectState(CONNECT_NULL);
        
        break;
    default:
        break;
    }
}

void CPubNetControl::PublicNetDataAnalyse(int nClientID,const char *recvData, int nRecvLen)
{
    if(nClientID < MAX_PUBLICNET_CLIENT_CONNECT)
    {
        m_tbPubTerminalInfo[nClientID].DataParseProcess(nClientID,recvData,nRecvLen);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
//
//���ݷ����߳�
UINT WINAPI CPubNetControl::DataAnalyseThreadProc(void *lpParameter)
{
    CPubNetControl *pHytPublicSys = NULL;
    pHytPublicSys = (CPubNetControl *)lpParameter;
    
    TRACE(_T("Into System thread%X\n"), pHytPublicSys->m_hDataAnalyseThread);
    ResetEvent(pHytPublicSys->m_hDataAnalyseThreadEndEvt);
    int timerid = SetTimer(NULL,0xf0, 1000, 0L); //����1s�Ļ�׼��ʱ
    MSG msg;
    BOOL bcontinue = TRUE;
    while(bcontinue)
    {
        if(GetMessage(&msg,NULL,0,0) )
        {
            switch(msg.message)
            {
            case WM_THREAD_STOP:
                bcontinue = FALSE;
                break;
            case WM_TIMER:
                pHytPublicSys->OnCheckTerminalTickOverTimer();
                break;
            case WM_RECVICE_PUBGPS_MESSAGE:
                pHytPublicSys->OnAnalysePublicServerRecvData();
                break;
#ifndef WANGZH	//2013-04-19
            case WM_FTP_DOWNLOAD_OVER_MESSAGE: //�����ļ����֪ͨ
                pHytPublicSys->OnReadMainFtpConveyFile();
                break;
#endif
            default:
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                break;
            }
        }
    }
    
    if(timerid != 0)
    {
        KillTimer(NULL, timerid);
    }
    
    SetEvent(pHytPublicSys->m_hDataAnalyseThreadEndEvt);
    TRACE(_T("Quit System thread\n"));
    _endthreadex(0);
    
    return 0;
}

//���ն˵�¼��Ϣ��ʱ��,ÿ��1����һ��
void CPubNetControl::OnCheckTerminalTickOverTimer()
{
    for(int i = 0 ; i < MAX_PUBLICNET_CLIENT_CONNECT; i++)
    {
        int nRes = m_tbPubTerminalInfo[i].CheckTimerOut();
        switch(nRes)
        {
        case 1:
            gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]�ȴ����ն˵�¼��Ϣ��ʱ\n",
                CTime::GetCurrentTime().Format("%H:%M:%S"),i);
            
            m_SockPubNetServer.CloseClient(i); //�ر��ն˵�TCP����
            break;
        case 2:
            gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]�ȴ��ն�������Ϣ��ʱ,DeviceCode:%s\n",
                CTime::GetCurrentTime().Format("%H:%M:%S"),i,m_tbPubTerminalInfo[i].GetDeviceNum());
            
            m_SockPubNetServer.CloseClient(i); //�ر��ն˵�TCP����
            break;
        default:
            break;
            
        }
    }
}

//�������չ�����������ݰ�
BOOL CPubNetControl::OnAnalysePublicServerRecvData()
{
    g_csPubNetRecvList.Lock();
    if(g_PubNetRecvList.IsEmpty())
    {
        g_csPubNetRecvList.Unlock();
        return FALSE;
    }
    
    RECV_MESSAGE RecvPubMsg;
    RecvPubMsg = g_PubNetRecvList.GetHead();
    g_PubNetRecvList.RemoveHead();
    g_csPubNetRecvList.Unlock();

    char *pData[MAX_PUBLICNET_DATE_TYPE];
    char tempBuf[MAX_PUBLIC_PACKAGE_LEN] = {0};
    char dispatchBuf[MAX_PUBLIC_PACKAGE_LEN] = {0};

    //��ȡ��ǰʱ��
    CString strCurrTime = CTime::GetCurrentTime().Format("%H:%M:%S");
    int i;
    for(i = 0; i < MAX_PUBLICNET_DATE_TYPE; i++)
    {
        pData[i] = NULL;
    }

    memcpy(tempBuf,RecvPubMsg.MessagePacket,RecvPubMsg.length);
    char *pBuf = tempBuf;
    i= 0 ;
    int nCount = 0;
    while((pData[i]=strtok(pBuf,",")) != NULL)
    {
        i++;
        nCount++;
        pBuf = NULL; 
    }

    int nCMD = 0;
    sscanf(pData[2], "%2x",&nCMD);

    CString strLogTrace;
    if( nCMD < MAX_PROTOCOL_UP )
    {
        if(m_FieldStatisticTB[nCMD] != nCount && m_FieldStatisticTB[nCMD] != 0xFF)
        {
            strLogTrace.Format("%s [ID:%d]���������,%s\n",
                strCurrTime,
                RecvPubMsg.nClientID,
                RecvPubMsg.MessagePacket);

            gWriteLogApp.WriteLog(FOR_ALARM,strLogTrace);
            
            return FALSE;
        }
    }
    
    if(UpProtocalDescriptions[nCMD].description != NULL)
    {
        if( UpProtocalDescriptions[nCMD].code == nCMD )
        {
            strLogTrace.Format("%s [ID:%d]<����>%s:%s\n",strCurrTime,
                RecvPubMsg.nClientID,
                UpProtocalDescriptions[nCMD].description,
                RecvPubMsg.MessagePacket);
			
			gWriteLogApp.WriteLog(FOR_TRACE,strLogTrace);
        }
        else
        {
            strLogTrace.Format("%s [ID:%d]<����>��������:%s\n",strCurrTime,
                RecvPubMsg.nClientID,
                RecvPubMsg.MessagePacket);

            gWriteLogApp.WriteLog(FOR_TRACE,strLogTrace);
        }
    }

    switch(nCMD)
    {
    /*********************************����Ӧ��Э��******************************************/
    case TERMINAL_LOGIN_REPORT: //GPRS���ӵ�
    {
        //�����ע����Ϣ����GIS���س���,��������Ϣ��¼��Ӧ�Ķ���
        int nRes = TerminalLoginProccess( RecvPubMsg.nClientID,
            pData[1], //SIM����
            pData[3], //��Ⱥ�ն�ID
            pData[4]  //�����ͺ�
            );

        if(nRes > 0)
        {
            //���ն˻ص�¼ȷ��
            DispatchAckToTerminal(RecvPubMsg.nClientID,ACK_TERMINAL_LOGIN,pData[5]);
            DispatchSetFixGpsDtCmdToTerminal(RecvPubMsg.nClientID);
            
            if(nRes == 1) //
            {
                //��GIS���ط���������Ϣ
                char tempSendBuf[MAX_GPS_MESSAGE_LEN] = {0};
                //д�ļ��������ն�����
                sprintf(tempSendBuf,"%s,%d,1",pData[1],m_tbPubTerminalInfo[RecvPubMsg.nClientID].GetDeviceKind());
                ReportDataToGisMain(A_TYPE_TERMINAL_LINE,tempSendBuf,strlen(tempSendBuf));
            }
        }
        break;
    }
	case TCPCONNECT_TICK_REPORT:  //������ϢЭ��-13
    {
		//����Ӧ������
        DispatchAckToTerminal(RecvPubMsg.nClientID,ACK_TCPCONNECT_TICK,pData[3]);
        //��������������鶨ʱ
        if(m_tbPubTerminalInfo[RecvPubMsg.nClientID].GetConnectState() == CONNECT_GPRSLOGIN_OK)
        {
            m_tbPubTerminalInfo[RecvPubMsg.nClientID].SetConnectState(CONNECT_GPRSLOGIN_OK,m_nConnTickTimeOut);
        }
		break;
    }
    case PRIVATENET_LINEON_REPORT:  //ר������Э��-14
    case PRIVATENET_LINEOFF_REPORT: //ר������Э��-15
    {
        int nAckType;
        GPS_DATA_TYPE nGpsType;
        if(nCMD == PRIVATENET_LINEON_REPORT)
        {
            nGpsType = GPS_DATA_PRIVATENET_LINEON;
            nAckType = ACK_PRIVATENET_LINEON;
        }
        else
        {
            nGpsType = GPS_DATA_PRIVATENET_LINEOFF;
            nAckType = ACK_PRIVATENET_LINEOFF;
        }

        //���ն�Ӧ�� pData[13] ΪACK�ֶ�
        DispatchAckToTerminal(RecvPubMsg.nClientID,nAckType,pData[13]);

        //ר��������GPS����
//         PublicGpsDataProcess(nGpsType,RecvPubMsg.nClientID,
// 			    pData[1], //SIM����
// 			    pData[3], //ʱ��
// 			    pData[4], //N/S γ�ȱ�־
// 			    pData[5], //γ��
// 			    pData[6], //E/W ���ȱ�־
// 			    pData[7], //����
// 			    pData[8], //��λ��־ �Ƿ���Ч
// 			    pData[9],  //�ٶ�
//              pData[10],  //����
//              pData[11],  //����
//              pData[14]  //״̬
// 			);
        
        break;
    }
    case GPS_TINMEROUT_REPORT:  //�̶�ʱ���������ϴ�GPS��Ϣ-00
    { 
        char strTempState[9] = {0};
        if(nCount == 14) //��Э��
        {
            if(m_tbPubTerminalInfo[RecvPubMsg.nClientID].GetConnectState() != CONNECT_GPRSLOGIN_OK)
            {
                gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]����,�ն�δ�Ǽ�,DeviceCode:%s\n",strCurrTime,RecvPubMsg.nClientID,pData[1]);
                return FALSE;
		    } 
        }
        else if(nCount == 13)
        {
            if(m_tbPubTerminalInfo[RecvPubMsg.nClientID].GetConnectState() != CONNECT_GPRSLOGIN_OK)
            {
                if(TerminalLoginProccess( RecvPubMsg.nClientID,
                    pData[1], //SIM����
                    pData[3], //��Ⱥ�ն�ID
                    pData[4],  //�����ͺ�
                    TRUE //����Э��
                    ) == 1)
                {
                    //
                    //д�ļ����ն�����
                    //��GIS���ط���������Ϣ
                    char tempSendBuf[MAX_GPS_MESSAGE_LEN] = {0};
                    sprintf(tempSendBuf,"%s,%d,1",pData[1],m_tbPubTerminalInfo[RecvPubMsg.nClientID].GetDeviceKind());
                    ReportDataToGisMain(A_TYPE_TERMINAL_LINE,tempSendBuf,strlen(tempSendBuf));
                }
            }
        }
        else
        {
            gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]����Ķ�ʱ�ϴ�GPS��Ϣ,�ֶθ�������\n",strCurrTime,RecvPubMsg.nClientID);
            return FALSE;
        }
        
        memcpy(strTempState,pData[12],8);
		if(!PublicGpsDataProcess(GPS_DATA_TINMEROUT,RecvPubMsg.nClientID,
			    pData[1], //SIM����
			    pData[3], //ʱ��
			    pData[4], //N/S γ�ȱ�־
			    pData[5], //γ��
			    pData[6], //E/W ���ȱ�־
			    pData[7], //����
			    pData[8], //��λ��־ �Ƿ���Ч
			    pData[9],  //�ٶ�
                pData[10],  //����
                pData[11],  //����
                strTempState,  //״̬
                RecvPubMsg.MessagePacket
			))
        {
            gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]����(��ʱ�ϴ�GPS��Ϣ),�ֶθ�ʽ��\n",strCurrTime,RecvPubMsg.nClientID);
			
            return FALSE;
        }
		break;
    }
    case GPS_HISTORY_REPORT: //��ʷGPS����
        //if(!PubNetHistoryGpsDataProcess(pData[1], pData[3],pData[4]))
        //{
        //    gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]����(��ʷGPS��Ϣ),�ֶθ�ʽ��\n",strCurrTime,RecvPubMsg.nClientID);
        //    return FALSE;
        //}
        break;
	case GPS_MONITOR_RESPONSE:  //��ʱ����GPSЭ��-01
    {
        if(m_tbPubTerminalInfo[RecvPubMsg.nClientID].GetConnectState() != CONNECT_GPRSLOGIN_OK)
        {
            gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]����,�ն�δ�Ǽ�,DeviceCode:%s\n",strCurrTime,RecvPubMsg.nClientID,pData[1]);
            return FALSE;
		}
        if(!PublicGpsDataProcess(GPS_DATA_MONITOR,RecvPubMsg.nClientID,
                pData[1], //SIM����
			    pData[3], //ʱ��
			    pData[4], //N/S γ�ȱ�־
			    pData[5], //γ��
			    pData[6], //E/W ���ȱ�־
			    pData[7], //����
			    pData[8], //��λ��־ �Ƿ���Ч
			    pData[9],  //�ٶ�
                pData[10],  //����
                pData[11],  //����
                pData[12],  //״̬
                RecvPubMsg.MessagePacket
            ) )
        {
            gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]����(��ʱ����GPS��Ϣ),�ֶθ�ʽ��\n",strCurrTime,RecvPubMsg.nClientID);
            return FALSE;
        }
        break;
    }
    case GPS_BOUNDCALL_RESPONSE:  //Ӧ���������GPSЭ��-02
    {
        if(m_tbPubTerminalInfo[RecvPubMsg.nClientID].GetConnectState() != CONNECT_GPRSLOGIN_OK)
        {
            gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]����(�������GPS��Ϣ),�ն�δ�Ǽ�,DeviceCode:%s\n",strCurrTime,RecvPubMsg.nClientID,pData[1]);
            return FALSE;
		}
        break;
    }
    case SEND_MESSAGE_ANSWER:
    {
        int nMsgAck = 0;
        sscanf(pData[3], "%4x",&nMsgAck);
        SendMessageAnswerOkProcess(RecvPubMsg.nClientID,pData[1],nMsgAck);
        break;
    }
    case QUERY_MY_POSITION://�ϴ������ѯ���ҵ�λ�á���ϢЭ��-16
    {
        //д�ļ�,�����ҵ�λ��
        break;
    }
    case QUERY_DESTINATION_POSITION: //�ϴ������ѯĿ�ĵؾ�γ��Э��-17
    {
        //д�ļ�,�ϴ������ѯĿ�ĵؾ�γ��Э��
        break;
    }
    case CTL_TERMINAL_MOUNTGUARD_RESPONSE:  //�ն��ϰ��°�-18
    {
        int nTmpGuardState = atoi(pData[3]);
        
        //Ӧ���ϰ��°�
        DispatchAckToTerminal(RecvPubMsg.nClientID,ACK_TERMINAL_MOUNTGUARD,pData[13]);
                
        GPS_DATA_TYPE nGpsType;
        if(nTmpGuardState == 1)
        {
            nGpsType = GPS_DATA_MY_ONDUTY;
        }
        else
        {
            nGpsType = GPS_DATA_MY_OFFDUTY;
        }

        //�ϰ��°�GPS����
//         PublicGpsDataProcess(nGpsType,RecvPubMsg.nClientID,
// 			    pData[1], //SIM����
// 			    pData[3], //ʱ��
// 			    pData[4], //N/S γ�ȱ�־
// 			    pData[5], //γ��
// 			    pData[6], //E/W ���ȱ�־
// 			    pData[7], //����
// 			    pData[8], //��λ��־ �Ƿ���Ч
// 			    pData[9],  //�ٶ�
//                 pData[10],  //����
//                 pData[11],  //����
//                 pData[14]  //״̬
// 			);
        break;
    }
    /*****************************����Ӧ���������Э��**************************************/

    /***********************************���б���Э��****************************************/

    /*********************************����Ӧ�����ò���Э��**********************************/

    /*********************************����Ӧ���ȡ����Э��**********************************/

    /***************************************************************************************/
    default:
        //gWriteLogApp.WriteLog(FOR_ALARM,"%s ����ʶ�������\n",strCurrTime);
        strLogTrace.Format("%s [ID:%d]����ʶ�������,%s\n",
                strCurrTime,
                RecvPubMsg.nClientID,
                RecvPubMsg.MessagePacket);

        gWriteLogApp.WriteLog(FOR_ALARM,strLogTrace);

        break;
    }

    return TRUE;
}

void CPubNetControl::DispatchAckToTerminal(int nClientID,int AckType,const char *AckValue)
{
    int nAckValue = 0;
    sscanf(AckValue,"%X",&nAckValue);
    
    PostThreadMessage(m_dwDataDispatchThreadID,
        WM_SEND_ACK_TERMINAL_MESSAGE,
        MAKELPARAM((WORD)nClientID, (WORD)AckType),
        (WORD)nAckValue);
}

//�������ö�ʱ�ϴ�ʱ��
void CPubNetControl::DispatchSetFixGpsDtCmdToTerminal(int nClientID)
{
    if(m_nOpenGpsModel == OPEN_GPS_GPRSLOGIN)
    {
        PostThreadMessage(m_dwDataDispatchThreadID,
            WM_SET_TERMINAL_GPSINTERVAL_MESSAGE,
            (WORD)nClientID,
            (WORD)m_nInitFixGpsDt);
    }
    else if(m_nOpenGpsModel == OPEN_GRPS_SOON )
    {
        PostThreadMessage(m_dwDataDispatchThreadID,
            WM_SET_TERMINAL_GPSMONITOR_MESSAGE,
            MAKELPARAM((WORD)nClientID, (WORD)m_nInitFixGpsDt),
            (WORD)m_nMonitorGPSCount);
    }
}

//GPRS���ӵ�¼��Ϣ������ϵͳ�еǼ��ն���Ϣ����¼��Ӧ�Ķ��У�
int CPubNetControl::TerminalLoginProccess( int nClientID,
                                     const char *strDeviceCode, //DeviceCode
                                     const char * strTrunkNum, //��ȺID
                                     const char *strModel, //�����ͺ�
                                     BOOL isOldProtocol)
{  
    int nRes = 0;
    //�ж������Ƿ��Լ����Ǽ�
    if(m_tbPubTerminalInfo[nClientID].GetConnectState() == CONNECT_GPRSLOGIN_OK)
    {
        gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]�ն�GPRS�����Ѿ��Ǽ�,DeviceCode:%s \n",
            CTime::GetCurrentTime().Format("%H:%M:%S"),
            nClientID,
            m_tbPubTerminalInfo[nClientID].GetDeviceNum());
        
        return nRes;
    }
    
    nRes = 1;
    //��ѯ�ú����Ƿ��Ѿ���¼
    for(int i = 0; i < MAX_PUBLICNET_CLIENT_CONNECT; i++)
    {
        
        if(m_tbPubTerminalInfo[i].GetConnectState() == CONNECT_GPRSLOGIN_OK)
        {
            if( (strcmp(m_tbPubTerminalInfo[i].GetDeviceNum(),strDeviceCode) == 0) &&
                nClientID != i)
            {
                
                gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]����ͬDeviceCode���ն˵�¼��ǿ�ƶϿ�֮ǰ������,DeviceCode:%s\n",
                    CTime::GetCurrentTime().Format("%H:%M:%S"),i,strDeviceCode);
                
                m_tbPubTerminalInfo[i].SetConnectState(CONNECT_NULL);

                //�ر�TCP����
                m_SockPubNetServer.CloseClient(i);
                
                
                nRes = 2;
                break;
            }
        }
    }
    
    //�ն��ͺ�
    int nTempKind ;
    if( strcmp(strModel,"TC890GM") == 0 )
    {
        nTempKind = KIND_TC890GM;
    }
    else if( strcmp(strModel,"TM800") == 0 )
    {
        nTempKind = KIND_TM800;
    }
    else
    {
        nTempKind = KIND_UNKNOWN;
    }
    
    m_tbPubTerminalInfo[nClientID].SetDeviceNum(strDeviceCode);
    m_tbPubTerminalInfo[nClientID].SetDeviceKind(nTempKind);
    
    m_tbPubTerminalInfo[nClientID].SetConnectState(CONNECT_GPRSLOGIN_OK,m_nConnTickTimeOut);
    
    m_tbPubTerminalInfo[nClientID].m_bOldProtocol = isOldProtocol;
    
    return nRes;
}

//�յ�����������Ϣ�ɹ�Ӧ������� add by kevin 2009-12-23
void CPubNetControl::SendMessageAnswerOkProcess(int nPubDeviceID,const char *pcDeviceNum,int nAckValue)
{
    if(!m_tbPubTerminalInfo[nPubDeviceID].SendMessageAnswerOkCheck(nAckValue))
    {
        return;
    }

    CString strTemp;
    
    int nSendMsgID = m_tbPubTerminalInfo[nPubDeviceID].GetSendMsgID();
    if(nSendMsgID <= 0)
    {
        return;
    }
    
    strTemp.Format("%d",nSendMsgID);
    
    char TempMsgPacket[MAX_GPS_MESSAGE_LEN] = {0};

    int nTempLen;
    sprintf(TempMsgPacket,"%s,03,%d", //03��ʾ�·�����������Ϣ
        pcDeviceNum,
        nSendMsgID);
    nTempLen = strlen(TempMsgPacket);
    //д�ļ��������ر��淢��������ϢӦ��,MsgID
    ReportDataToGisMain(A_TYPE_APPCMD_TERMINAL,TempMsgPacket,nTempLen);

    //change by kevin 2010-5-26
    m_tbPubTerminalInfo[nPubDeviceID].StopSendMessageWaitAnswer();
}

//����GPS����
BOOL CPubNetControl::PublicGpsDataProcess(GPS_DATA_TYPE gpsDataType,
                          int nClientID,
                          const char *SIMNum, //SIM����
                          const char * strTimer, //ʱ��
                          const char *nsFlag,   //N/S
                          const char *latitude, //γ��
                          const char *ewFlag,  //E/W
                          const char *longitude, //����
                          const char *avFlag,  //�Ƿ���Ч
                          const char *speed,   //�ٶ�
                          const char *Direction,   //����
                          const char *Date,   //����;
                          const char *state,
                          const char *strAllData
    )
{
    //���ݼ��
    if(strlen(strTimer) != 6)
    {
        return FALSE;
    }
    
    if(strlen(avFlag) != 1 || strlen(nsFlag) != 1 || strlen(ewFlag) != 1)
    {
        return FALSE;
    }   
    if(latitude[4] != '.' || strlen(latitude) != 9)
    {
        return FALSE;
    }
    if(longitude[5] != '.' || strlen(longitude) != 10)
    {
        return FALSE;
    }
    
    if(!m_bDispatchZeroGPS)
    {
        float nlatitude = atol(latitude);
        if(nlatitude < 1 )
        {
            return TRUE;
        }
    }

    char tempBuf[MAX_GPS_MESSAGE_LEN] = {0};
    sprintf(tempBuf,"%s,%.2X,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",SIMNum,gpsDataType,strTimer,
        nsFlag,latitude,ewFlag,longitude,
        avFlag,speed,Direction,Date,
        state);
    
    gRecvGpsCount++;
    
    ReportDataToGisMain(A_TYPE_TERMINAL_GPS,tempBuf,strlen(tempBuf));

    return TRUE;
}

//������д�ļ�,Ȼ���ϴ���FTP
BOOL CPubNetControl::ReportDataToGisMain(int nType,const char *DataBuf,int nBufLen)
{
    if(DataBuf == NULL)
    {
        return FALSE;
    }
    char TempMsgPacket[MAX_GPS_MESSAGE_LEN] = {0};
    BOOL bRes = TRUE;
    int nTempLen;
    switch(nType)
    {
    case A_TYPE_TERMINAL_GPS: //�ն�GPS����
        nTempLen = nBufLen + 6;
        sprintf(TempMsgPacket,"%.4X%.2X%s\n",nTempLen,A_TYPE_TERMINAL_GPS,DataBuf);
        break;
    case A_TYPE_TERMINAL_LINE: //�ն�������
        nTempLen = nBufLen + 6;
        sprintf(TempMsgPacket,"%.4X%.2X%s\n",nTempLen,A_TYPE_TERMINAL_LINE,DataBuf);
        break;
    case A_TYPE_APPCMD_TERMINAL:
        nTempLen = nBufLen + 6;
        sprintf(TempMsgPacket,"%.4X%.2X%s\n",nTempLen,A_TYPE_APPCMD_TERMINAL,DataBuf);
        break;
    default:
        bRes = FALSE;
        break;
    }
    
    if(bRes)
    {
        //д�ļ�
		m_myFtpConveyFile.WriteData(0,TempMsgPacket);
    }
    return bRes;
}

//����FTP�����������ļ��������صĽ�����Ϣ DataAnalyseThreadProc
BOOL CPubNetControl::OnReadMainFtpConveyFile()
{
    CFileFind IsValidFile;
    if(!IsValidFile.FindFile(m_strFTPDownLoadFileName)) 
    {
        //�ļ�������
        return FALSE;
    }
    
    FILE *fptr;
    if((fptr = fopen(m_strFTPDownLoadFileName, "rb")) == NULL)
    {    
        //���ļ�ʧ��
        return FALSE;
    }
    
    unsigned char rdDataBuf[MAX_GPS_MESSAGE_LEN*2] = {0};
    int nReadLen = 0;
    BOOL bLoop = TRUE;
    int nSeekCount = 0;
    while(bLoop)
    {
        if(fseek(fptr,nSeekCount, SEEK_SET) == 0)
        {
            nReadLen = fread(rdDataBuf,1,MAX_GPS_MESSAGE_LEN, fptr);
            //�Զ�ȡ�����ݽ��з���
            if(nReadLen > 0)
            {
                int nCurrPoint = 0;
                char strGetNumber[MAX_GPS_MESSAGE_LEN] = {0}; 
                int nNumLen;
                CString strAddNum;
                
                for(int i = 0; i < nReadLen; i++)
                {
                    if(rdDataBuf[i] == '\r' || rdDataBuf[i] == '\n') 
                    {
                        memset(strGetNumber,0,32);
                        nNumLen = i - nCurrPoint;
                        memcpy(strGetNumber,rdDataBuf+nCurrPoint,nNumLen);
                        nCurrPoint = i+1;
                        strAddNum.Format(_T("%s"),strGetNumber);
                        if(strAddNum.GetLength() > 4)
                        {
                            FtpMainDataAnalyse(strGetNumber);         
                        }
                    }
                }
                nSeekCount = nSeekCount + nCurrPoint;
                if(nSeekCount == 0) //����MAX_GPS_MESSAGE_LENû���ҵ�һ�н���
                {
                    nSeekCount = MAX_GPS_MESSAGE_LEN;
                }
            }
            
            if( nReadLen < MAX_GPS_MESSAGE_LEN)
            {
                //�ļ������˳�
                fclose(fptr);  
                fptr = NULL;
                bLoop = FALSE;
            }
        }
    }
    
    if(fptr)
    {
        fclose(fptr); 
    }
    
	TRY
	{
		//ɾ���ļ�
		CFile::Remove(m_strFTPDownLoadFileName);
	}
	CATCH( CFileException, e )
	{
		TRACE("ɾ���ļ�ʧ��");
	}
	END_CATCH

    
    return TRUE;
}

typedef enum enumGisMainAppCmd
{
    aAPP_CMD_SetFixGpsDt = 0, //���ù̶��ϴ�GPS��Ϣʱ����
    aAPP_CMD_GpsTrace = 1, //��ʱ��������
    aAPP_CMD_AreaGpsCall = 2,//�������
    aAPP_CMD_SendMessage = 3,
    app_MAX = 4,
}GISMAIN_APP_CMD;

static int g_AppCmdTB[app_MAX] = {SET_GPS_TINMEROUT,SET_GPS_MONITOR,SET_GPS_BOUNDCALL,APP_SEND_MESSAGE};

//�������ļ����������ݰ�
BOOL CPubNetControl::FtpMainDataAnalyse(const char *pstrMainDataPacket)
{
    if(pstrMainDataPacket == NULL)
    {
        return FALSE;
    }

    char tempBuf[MAX_GPS_MESSAGE_LEN] = {0};
    char *pData[20];
    int i;
    for(i = 0; i < 20; i++)
    {
        pData[i] = NULL;
    }

    int nDataLen = strlen(pstrMainDataPacket);
    memcpy(tempBuf,pstrMainDataPacket,nDataLen);
    char *pBuf = tempBuf;
    i = 0;
    int nCount = 0;
    while((pData[i]=strtok(pBuf,",")) != NULL)
    {
        i++;
        nCount++;
        pBuf = NULL; 
    }
    if(nCount < 1)
    {
        return FALSE;
    }
    
    int nCMD = 0;
    int nPackageLen;
    if(strlen(pData[0]) < 6)
    {
        return FALSE;
    }
    sscanf(pData[0], "%4x%2x",&nPackageLen,&nCMD);

    if(nPackageLen != nDataLen)
    {
        //Э���ʽ����
        return FALSE;
    }
    int nMoveLen = 6;
    int nTempValue;
    switch(nCMD)
    {
    case A_TYPE_LOGIN: //�����������д���������
        MainStartSuccessResponse();
        break;
    case A_TYPE_APPCMD_TERMINAL: //�·�Ӧ��������ն�,�̶��ϴ�,��ʱ����,������Ϣ
    {
        int nClientID = GetClientID(pData[0]+6); 
        nTempValue = atoi(pData[1]);
        int nMoveLen = strlen(pData[0]) + strlen(pData[1]) + 2;
        if(nClientID >= 0 && nTempValue < app_MAX)
        {
            SendDataToPubNetTerminal(nClientID,
                g_AppCmdTB[nTempValue],
                pstrMainDataPacket+nMoveLen, //Ϊ��ȥ���ն˺��롢����Ϣ���͵�����
                nDataLen - nMoveLen);
        }
        break;
    }
    default:
        break;
    }

    return TRUE;

}

//���ļ��������������ɹ���Ϣ����,�����е������ն�д�ļ�
void CPubNetControl::MainStartSuccessResponse()
{
    int i = 0; 
    char TempBuf[MAX_GPS_MESSAGE_LEN] = {0};
    for(i = 0; i < MAX_PUBLICNET_CLIENT_CONNECT; i++)
    {
        if(m_tbPubTerminalInfo[i].GetConnectState() == CONNECT_GPRSLOGIN_OK)
        {
            sprintf(TempBuf,"%s,%d,1",m_tbPubTerminalInfo[i].GetDeviceNum(),m_tbPubTerminalInfo[i].GetDeviceKind());
            
            ReportDataToGisMain(A_TYPE_TERMINAL_LINE,TempBuf,strlen(TempBuf));
            
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
//
//ͨ��SIM���Ų�ѯ����ID
int CPubNetControl::GetClientID(const char *strDeviceCode)
{
    if(strDeviceCode == NULL)
    {
        return -1;
    }
    
    if(strlen(strDeviceCode) == 0)
    {
        return -1;
    }
    
    //��ѯ�ú����Ƿ��Ѿ���¼
    for(int i = 0; i < MAX_PUBLICNET_CLIENT_CONNECT; i++)
    {
        if( m_tbPubTerminalInfo[i].GetConnectState() == CONNECT_GPRSLOGIN_OK &&
            (strncmp(m_tbPubTerminalInfo[i].GetDeviceNum(),strDeviceCode,strlen(strDeviceCode)) == 0) )
        {
            return i;
        }
    }
    
    return -1;
}


//�������ݸ������ն˵Ĺ�������
BOOL CPubNetControl::SendDataToPubNetTerminal(int nClientID,int nType,const char *DataBuf,int nBufLen)
{
    char tempStr[MAX_GPS_MESSAGE_LEN] = {0};
    
    CString StrTrace;
    CString strCurrTime;

    CString strSIMNum = _T(" ");
    BOOL bErrorFlag = FALSE; 
    int nTempValue;
    if(nClientID >= 0 && nClientID < MAX_PUBLICNET_CLIENT_CONNECT)
    {
        
        strSIMNum.Format(_T("%s"),m_tbPubTerminalInfo[nClientID].GetDeviceNum());
    }
    else
    {
        return FALSE;
    }
    
    strCurrTime = CTime::GetCurrentTime().Format("%H:%M:%S");
    if(m_tbPubTerminalInfo[nClientID].m_bOldProtocol)
    {
        gWriteLogApp.WriteLog(FOR_TRACE,"%s [ID:%d]<����>�ն�Ϊ�ϵ�Э��汾,��֧����������,DeviceCode:%s\n",
            strCurrTime,
            nClientID,strSIMNum);

        return FALSE;
    }

    int nChksum;
    char MessagePacket[MAX_GPS_MESSAGE_LEN] = {0};
    switch(nType)
    {
    /*************************�����·�Ӧ��������Ϣ******************************/    
    case SET_GPS_TINMEROUT://���ù̶��ϴ�GPSʱ��������-00
    {
        int nTimeFix = atoi(DataBuf);
        
        StrTrace = _T("���ù̶��ϴ�GPSʱ����");
        sprintf(tempStr,"HYT,%.2X,%.4d,",nType,nTimeFix);
        
        break;
    }  
    case SET_GPS_MONITOR://�����·���ʱ����ָ��-01
    {
        int nCount = 0;
        int nTimeSlot = 0;
        sscanf(DataBuf,"%d,%d",&nTimeSlot,&nCount);
        
        sprintf(tempStr,"HYT,%.2X,%.4d,%.5d,",nType,nTimeSlot,nCount);
        StrTrace = _T("��ʱ��������");

        break;
    }  
    case SET_GPS_BOUNDCALL: //���������Ϣ-02
    {
        StrTrace = _T("�����������");
        sprintf(tempStr,"HYT,%.2X,%s,",nType,DataBuf);

        break;
    }
    case APP_SEND_MESSAGE: //�·�������
    {
        StrTrace = _T("�·�������");
        char tmpMsg[MAX_GPS_MESSAGE_LEN] = {0};
        memcpy(tmpMsg,DataBuf,nBufLen);
        
        strtok(tmpMsg,",");
        char *ptmpMsg = tmpMsg;
        char *pStrMsgID = strtok(NULL,",");
        if(ptmpMsg == NULL || pStrMsgID == NULL)
        {
            return FALSE;
        }
        int nMsgID = atoi(pStrMsgID);

        if(g_nSendTxtMsgInterval > 0 && nMsgID > 0)
        {
            //������Ϣ��ʱ����
            m_tbPubTerminalInfo[nClientID].AddMsgTxtToBuffer(nMsgID,ptmpMsg);

            return TRUE;
        }
        else
        {
            //������Ϣ��������
            _snprintf(tempStr,MAX_GPS_MESSAGE_LEN-6,"HYT,%.2X,%s,%.4X,",nType,ptmpMsg,m_nAckCount[nType]);
            if(nMsgID > 0)
            {
                //�����ط�����
                m_tbPubTerminalInfo[nClientID].StartSendMessageWaitAnswer( m_nAckCount[nType],
                    nMsgID,
                    ptmpMsg,
                    m_nSendTxtMsgWaitAckTimeOut);
            }
        }
        break;
    }
    /*************************�����·�����������Ϣ******************************/
    case CTL_PRIVATENET_LINE://����ר�����ߺ�ר����������-20
    {
        nTempValue = atoi(DataBuf);
        if(nTempValue > 0)
        {
            nTempValue = 1;
        }
        
        StrTrace = _T("����ר�����ߺ�ר������");
        sprintf(tempStr,"HYT,%.2X,%d,%.4X,",nType,nTempValue,m_nAckCount[nType]);

        break;
    }
    case CTL_TERMINAL_WAKEUP://�����ն˼�Ⱥң�к�ҡ��-21
    {
        nTempValue = atoi(DataBuf);
        if(nTempValue > 0)
        {
            nTempValue = 1;
        }
        StrTrace = _T("�����ն˼�Ⱥң�к�ҡ��");
        sprintf(tempStr,"HYT,%.2X,%d,%.4X,",nType,nTempValue,m_nAckCount[nType]);
        break;
    }
    /*************************�����·�Ӧ�𱨾���Ϣ******************************/
    case ACK_GPS_ALERTING://Ӧ�𱨾����������-30
    {
        StrTrace = _T("Ӧ�𱨾����������");
        nTempValue = atoi(DataBuf);
        sprintf(tempStr,"HYT,%.2X,%.2X,%.4X,",nType,nTempValue,m_nAckCount[nType]);
        break;
    }
    /*************************�����·�Ӧ�𱨾���Ϣ******************************/
    case CFG_COMMUNICATION_MODEL_CMD://����ͨѶ��ʽ����-60
    {
        nTempValue = atoi(DataBuf);
        if(nTempValue > 0)
        {
            nTempValue = 1;
        }
        StrTrace = _T("����ͨѶ��ʽ����");
        sprintf(tempStr,"HYT,%.2X,%d,%.4X,",nType,nTempValue,m_nAckCount[nType]);
        break;
    }
    /*************************�����·�Ӧ��Ӧ����Ϣ******************************/
    

    /*************************�����·���������**********************************/
    case CFG_GPRS_PARAMETER_CMD://����GPRS����-61
    {
        StrTrace = _T("����GPRS����");
        sprintf(tempStr,"HYT,%.2X,%s,%.4X,",nType,DataBuf,m_nAckCount[nType]); 
        break;
    }
    case CFG_GSM_PARAMETER_CMD://����GSM����-62
    {
        StrTrace = _T("����GSM����");
        sprintf(tempStr,"HYT,%.2X,%s,%.4X,",nType,DataBuf,m_nAckCount[nType]); 
        break;
    }
    case CFG_TERMINAL_NUM_CMD://�����ն�ר������-63
    {    
        StrTrace = _T("�����ն�ר������");
        sprintf(tempStr,"HYT,%.2X,1,1,%s,%.4X,",nType,DataBuf,m_nAckCount[nType]);
        break;
    }
    case CFG_TEMPGROUP_NUM_CMD://���ö�̬��ʱ�������-64
    {
        char tempCps[16] = {0};
        char tempCrateFlag[2] = {0};
        tempCrateFlag[0] = DataBuf[0];
        int nCreatFlag = atoi(tempCrateFlag);
        if( !GetCpsxIdValue(DataBuf+2,tempCps))
        {
            gWriteLogApp.WriteLog(FOR_ALARM,"%s <����>����ĺ����ʽ,���ö�̬��ʱ�������,DeviceCode:%s,cmdData:%s\n",strCurrTime,strSIMNum,DataBuf);
            return FALSE;
        }
        StrTrace = _T("���ö�̬��ʱ�������");
        sprintf(tempStr,"HYT,%.2X,%d,%s,%.4X,",nType,nCreatFlag,tempCps,m_nAckCount[nType]); 
        break;
    }
    case CFG_ALERT_NUM_CMD://����ר�����Ÿ澯����-65
    {
        StrTrace = _T("����ר�����Ÿ澯����");
        sprintf(tempStr,"HYT,%.2X,1,1,%s,%.4X,",nType,DataBuf,m_nAckCount[nType]); 
        break;
    }
    case CFG_SAFETYCHECK_NUM_CMD://����ר���������-66
    {
        StrTrace = _T("����ר���������");
        sprintf(tempStr,"HYT,%.2X,1,1,%s,%.4X,",nType,DataBuf,m_nAckCount[nType]); 
        break;
    }
    case CFG_GPSCENTER_NUM_CMD://����ר��GPS���ĺ���-67
    {
        StrTrace = _T("����ר��GPS���ĺ���");
        sprintf(tempStr,"HYT,%.2X,1,1,%s,%.4X,",nType,DataBuf,m_nAckCount[nType]); 
        break;
    }
    case CFG_BACK_MODEL_CMD://����ר������ģʽ-68
    {
        StrTrace = _T("����ר������ģʽ");
        sprintf(tempStr,"HYT,%.2X,%s,%.4X,",nType,DataBuf,m_nAckCount[nType]); 
        break;
    }

    /************************�����·���ȡ��������********************************/
    case QUERY_COMMUNICATION_MODEL_CMD://��ȡͨѶ��ʽ����-80
    {
        sprintf(tempStr,"HYT,%.2X,%.4X,",nType,m_nAckCount[nType]);
        StrTrace = _T("��ȡͨѶ��ʽ����");
        break;
    }
    case QUERY_GPS_PARAMETER_CMD://��ȡGPRS����-81
    {
        sprintf(tempStr,"HYT,%.2X,%.4X,",nType,m_nAckCount[nType]);
        StrTrace = _T("��ȡGPRS����");
        break;
    }
    case QUERY_GSM_PARAMETER_CMD://��ȡGSM����-82
    {
        sprintf(tempStr,"HYT,%.2X,%.4X,",nType,m_nAckCount[nType]);
        StrTrace = _T("��ȡGSM����");
        break;
    }
    case QUERY_TERMINAL_NUM_CMD://��ȡ�ն�ר������-83
    {
        sprintf(tempStr,"HYT,%.2X,%.4X,",nType,m_nAckCount[nType]);
        StrTrace = _T("��ȡ�ն�ר������");
        break;
    }
    case QUERY_TEMPGROUP_NUM_CMD://��ȡ��̬��ʱ�������-84
    {
        sprintf(tempStr,"HYT,%.2X,%.4X,",nType,m_nAckCount[nType]);
        StrTrace = _T("��ȡ��̬��ʱ�������");
        break;
    }
    case QUERY_ALERT_NUM_CMD://��ȡר�����Ÿ澯����-85
    {
        sprintf(tempStr,"HYT,%.2X,%.4X,",nType,m_nAckCount[nType]);
        StrTrace = _T("��ȡר�����Ÿ澯����");
        break;
    }
    case QUERY_SAFETYCHECK_NUM_CMD://��ȡר���������-86
    {
        sprintf(tempStr,"HYT,%.2X,%.4X,",nType,m_nAckCount[nType]);
        StrTrace = _T("��ȡ�ն˰�ȫ������");
        break;
    } 
    case QUERY_GPSCENTER_NUM_CMD://��ȡר��GPS���ĺ���-87
    {
        sprintf(tempStr,"HYT,%.2X,%.4X,",nType,m_nAckCount[nType]);
        StrTrace = _T("��ȡר��GPS���ĺ���");
        break;
    }
    case QUERY_BACK_MODEL_CMD://��ȡר������ɨ��ģʽ-88
    {
        sprintf(tempStr,"HYT,%.2X,%.4X,",nType,m_nAckCount[nType]);
        StrTrace = _T("��ȡר������ɨ��ģʽ");
        break;
    }
    case QUERY_MAGNETIC_BOUND_CMD://��ȡר����ǿ����-89
    {
        sprintf(tempStr,"HYT,%.2X,%.4X,",nType,m_nAckCount[nType]);
        StrTrace = _T("��ȡר����ǿ����");
        break;
    }
    case QUERY_TERMINAL_TYPE_CMD://��ȡ�ն˻��Ͳ���-8A
    {
        sprintf(tempStr,"HYT,%.2X,%.4X,",nType,m_nAckCount[nType]);
        StrTrace = _T("��ȡ�ն˻��Ͳ���");
        break;
    }
    case QUERY_WAKEUP_CMD ://��ȡ�ն�ң��ҡ��״̬-8B
    {
        sprintf(tempStr,"HYT,%.2X,%.4X,",nType,m_nAckCount[nType]);
        StrTrace = _T("��ȡ�ն�ң��ҡ��״̬");
        break;
    }
    case QUERY_GPSSEND_INTERVAL_CMD: //��ȡGPS�̶��ϴ�ʱ��������
    {
        sprintf(tempStr,"HYT,%.2X,%.4X,",nType,m_nAckCount[nType]);
        StrTrace = _T("��ȡGPS�̶��ϴ�ʱ��������");
        break;
    }
    default:
        return FALSE;
    }

    nChksum = GetChksum(tempStr);
    sprintf(MessagePacket,"*%s$%X#",tempStr,nChksum);

    m_nAckCount[nType]++;
    if(m_nAckCount[nType] >= 0xFFFF)
    {
        m_nAckCount[nType] = 0;
    }

    if(nType != SET_GPS_BOUNDCALL )
    {
        //��������
        if( m_SockPubNetServer.Send(nClientID,
            MessagePacket,
            strlen(MessagePacket)) == TCPSERVER_ERROR_SEND)
        {            
            gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]<����>�������ݸ��ն�ʧ��,�ر�TCP����,DeviceCode:%s\n",
                strCurrTime,
                nClientID,
                strSIMNum);

            m_SockPubNetServer.CloseClient(nClientID);

            return FALSE;
        }

        gWriteLogApp.WriteLog(FOR_TRACE,"%s [ID:%d]<����>%s,DeviceCode:%s,����:%s\n",
            strCurrTime,
            nClientID,
            StrTrace,strSIMNum,MessagePacket);
    }
    
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
UINT WINAPI CPubNetControl::DataDispatchThreadProc(void *lpParameter)
{
    CPubNetControl *pHytPublicSys = NULL;
    pHytPublicSys = (CPubNetControl *)lpParameter;
    
    //TRACE(_T("Into System thread%X\n"), pHytPublicSys->m_hSysThread);
    ResetEvent(pHytPublicSys->m_hDataDispatchThreadEndEvt);
    int timerid = SetTimer( NULL,NULL, 1000, 0L ); //����1s�Ļ�׼��ʱ
    MSG msg;
    BOOL bcontinue = TRUE;
    while(bcontinue)
    {
        if( GetMessage(&msg,NULL,0,0) )
        {
            switch(msg.message)
            {
            case WM_THREAD_STOP:
                bcontinue = FALSE;
                break;
            case WM_TIMER:
                pHytPublicSys->OnCheckSendMessageWaitAnswerTimerOut();
                pHytPublicSys->OnCheckGetSendMsgTxtTimerOut();
                break;
            case WM_SET_TERMINAL_GPSINTERVAL_MESSAGE:
                pHytPublicSys->OnSetTerminalGpsIntervalMessage(msg.wParam,msg.lParam);
                break;
            case WM_SET_TERMINAL_GPSMONITOR_MESSAGE:
                pHytPublicSys->OnSetTerminalGpsMonitorMessage(msg.wParam,msg.lParam);
                break;
            case WM_SEND_ACK_TERMINAL_MESSAGE:
                pHytPublicSys->OnAnswerAckToTerminal(msg.wParam,msg.lParam);
                break;
            case WM_DISPATCH_DATA_MESSAGE:
                pHytPublicSys->OnDispatchDataMessage();
                break;
            default:
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                break;
            }
        }
    }
    
    if(timerid != 0)
    {
        KillTimer(NULL, timerid);
    }
    
    SetEvent(pHytPublicSys->m_hDataDispatchThreadEndEvt);
    TRACE(_T("Quit Read DataBase thread\n"));
    _endthreadex(0);
    
    return 0;
}

//���ù̶��ϴ�ʱ����
void CPubNetControl::OnSetTerminalGpsIntervalMessage(WPARAM wParam, LPARAM lparam)
{
    char tempStr[8] ={0};
    sprintf(tempStr,"%d",lparam);
    SendDataToPubNetTerminal((int)wParam,SET_GPS_TINMEROUT ,tempStr,strlen(tempStr));
}

//���ü�ʱ����
void CPubNetControl::OnSetTerminalGpsMonitorMessage(WPARAM wParam, LPARAM lparam)
{
    char tempStr[64] ={0};
    int nClientID = LOWORD(wParam);  
    int nTimeSlot = HIWORD(wParam);
    int nSendCount = (int)lparam;
    
    sprintf(tempStr,"%d,%d",nTimeSlot,nSendCount);

    SendDataToPubNetTerminal(nClientID,SET_GPS_MONITOR ,tempStr,strlen(tempStr));
    
}

//��ACK���ݷ����ն�
BOOL CPubNetControl::OnAnswerAckToTerminal(WPARAM wParam, LPARAM lparam)
{
    int nClientID = LOWORD(wParam);  
    int AckType = HIWORD(wParam);
    
    int nAckValue = (int)lparam;
    
    char tempStr[128] = {0};
    char sendBuf[256] = {0};
    CString StrTrace;
    switch(AckType)
    {
    case ACK_TERMINAL_LOGIN: //Ӧ���ն�GPRS��¼��-10
        sprintf(tempStr,"HYT,10,%.4X,",nAckValue);
        StrTrace = _T("Ӧ���ն�GPRS��¼");
        break;
    case ACK_TERMINAL_OPEN: //Ӧ���ն˿�����Ϣ-11
        sprintf(tempStr,"HYT,11,%.4X,",nAckValue);
        StrTrace = _T("Ӧ���ն˿���");
        break;
    case ACK_GPS_TERMINAL_OPEN: //Ӧ���ն˿���GPS��Ϣ-12
        sprintf(tempStr,"HYT,12,%.4X,",nAckValue);
        StrTrace = _T("Ӧ���ն˿���GPS");
        break;
    case ACK_TCPCONNECT_TICK: //Ӧ������
        sprintf(tempStr,"HYT,13,%.4X,",nAckValue);
        StrTrace = _T("Ӧ������");
        break;
    case ACK_PRIVATENET_LINEON: //���ն�Ӧ��ר��������ȷ��
        sprintf(tempStr,"HYT,14,%.4X,",nAckValue);
        StrTrace = _T("Ӧ��ר������ȷ��");
        break;
    case ACK_PRIVATENET_LINEOFF: //���ն�Ӧ��ר��������ȷ��
        sprintf(tempStr,"HYT,15,%.4X,",nAckValue);
        StrTrace = _T("Ӧ��ר��������ȷ��");
        break;
    case ACK_GPS_ALERTING: //���ն�Ӧ�𱨾�ȷ��
        sprintf(tempStr,"HYT,30,00,%.4X,",nAckValue);
        StrTrace = _T("Ӧ�𱨾�ȷ��");
        break;
    case ACK_TERMINAL_MOUNTGUARD: //Ӧ���ϰ��°�
        sprintf(tempStr,"HYT,18,00,%.4X,",nAckValue);
        StrTrace = _T("Ӧ���ϰ��°�");
        break;
    default:
        return FALSE;
        break;
    }
    
    CString strCurrTime;
    strCurrTime = CTime::GetCurrentTime().Format("%H:%M:%S");
    //ȡЧ���
    int nChksum = GetChksum(tempStr);
    sprintf(sendBuf,"*%s$%X#",tempStr,nChksum);
    
    //��������
    if( m_SockPubNetServer.Send(nClientID,
        sendBuf,
        strlen(sendBuf)) == TCPSERVER_ERROR_SEND )
    {
        
        gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]<����>�������ݸ��ն�ʧ��,�ر�TCP����,DeviceCode:%s\n",
            strCurrTime,
            nClientID,
            m_tbPubTerminalInfo[nClientID].GetDeviceNum());
        
        m_SockPubNetServer.CloseClient(nClientID);
        
        return FALSE;
    }
    
    //if(AckType != ACK_TCPCONNECT_TICK)
    {
        gWriteLogApp.WriteLog(FOR_TRACE,"%s [ID:%d]<����>%s,DeviceCode:%s,data:%s\n",
            strCurrTime,
            nClientID,StrTrace,
            m_tbPubTerminalInfo[nClientID].GetDeviceNum(),
            sendBuf);
    }
    
    return TRUE;
    
}

//
BOOL CPubNetControl::OnDispatchDataMessage()
{
    m_csDispatchList.Lock();
    if(m_DispatchList.IsEmpty())
    {
        m_csDispatchList.Unlock();
        return FALSE;
    }

    DISPATCH_MESSAGE DispatchMsg;

    DispatchMsg = m_DispatchList.GetHead();
    m_DispatchList.RemoveHead();
    m_csDispatchList.Unlock();
    
    char tempBuf[MAX_GPS_MESSAGE_LEN] = {0};
    
    if(DispatchMsg.aPPID == 1) //����GIS����
    {
        return FALSE;
    }
   
    //�����ն�
    memcpy(tempBuf,DispatchMsg.MessagePacket,MAX_GPS_MESSAGE_LEN);
    char *pBuf = tempBuf;
    int i= 0 ;
    char *pData[16];
    for(i = 0; i < 16; i++)
    {
        pData[i] = NULL;
    }
    
    int nCount = 0;
    i = 0;
    while((pData[i]=strtok(pBuf,",")) != NULL)
    {
        i++;
        nCount++;
        if(nCount >= 16)
        {
            return FALSE;
        }
        pBuf = NULL; 
    }

    if(nCount < 2)
    {
        return FALSE;
    }

    /*//i = 0;
    while((pData[i]=strtok(pBuf,",")) != NULL)
    {
        //i++;
        //if(i == 2)
        //{
        //    break;
        //}
        pBuf = NULL; 
    }*/

    int nClientID = GetClientID(pData[0]); 
    if(nClientID == -1)
    {
        CString strCurrTime;
        strCurrTime = CTime::GetCurrentTime().Format("%H:%M:%S");
        gWriteLogApp.WriteLog(FOR_ALARM,"%s <����>�ն˻�û����ϵͳ�Ǽ�,����:%s:Э������:%d,����:%s\n",strCurrTime,
            pData[0],DispatchMsg.TypeID,DispatchMsg.MessagePacket);

        return FALSE; 
    }
    int nID = 0;
    sscanf(pData[1], "%2x",&nID);
    int nProtocalType = -1;
    //int nProtocalType = GetPubDownProtocalType(DispatchMsg.TypeID,nID);
    if(nProtocalType == -1)
    {
        return FALSE;
    }

    int nMoveLen = 0;
    nMoveLen = strlen(pData[0]) + strlen(pData[1]) + 1; //ȥ���ն˺���,��Ϣ����
    if(nMoveLen >= DispatchMsg.length)
    {
        //�·���������Э�顢�·�Ӧ��ͽ���ն˱���Э�顢�·���ȡ�������ն�Э��
        //��ѯ��
        if(nProtocalType == CTL_TERMINAL_WAKEUP || nProtocalType == CTL_PRIVATENET_LINE)
        {
            char TempStr[4] ={0};
            nID = nID %2;  
            sprintf(tempBuf,"%d",nID);

            tempBuf[1] = NULL; //�ַ�����β
            SendDataToPubNetTerminal(nClientID,
                nProtocalType,
                tempBuf,1);
        }
        else
        {
            SendDataToPubNetTerminal(nClientID,nProtocalType,NULL,0);
        }
    }
    else
    {
        //�·�Ӧ��������ն�Э�顢�·����ò������ն�Э��
        nMoveLen = nMoveLen + 1;
        memcpy(tempBuf,DispatchMsg.MessagePacket+nMoveLen,DispatchMsg.length - nMoveLen );
        tempBuf[DispatchMsg.length - nMoveLen] = NULL; //�ַ�����β

        SendDataToPubNetTerminal(nClientID,
            nProtocalType,
            tempBuf, //Ϊ��ȥ���ն˺��롢����Ϣ���͵�����
            DispatchMsg.length - nMoveLen);
    }

    return TRUE;
}

//================================================================
//����Ƿ�����Ҫ�ط���������Ϣ
void CPubNetControl::OnCheckSendMessageWaitAnswerTimerOut()
{
    int i = 0;
    if(m_nSendTxtMsgWaitAckTimeOut <= 0)
    {
        return ;
    }
    
    for(i = 0; i< MAX_PUBLICNET_CLIENT_CONNECT; i++)
    {
        int nRes = m_tbPubTerminalInfo[i].CheckResendMsgInfoTimerOut();
        if(nRes == 1)
        {
            ResendMessgToPubDevice(i,
                m_tbPubTerminalInfo[i].GetMessageInfo(),
                m_tbPubTerminalInfo[i].GetMsgAckValue());
        }
        else if( nRes == 2)
        {
            m_tbPubTerminalInfo[i].StopSendMessageWaitAnswer();
            
            gWriteLogApp.WriteLog(FOR_ALARM,"%s <GIS>���ն˷���������Ϣ���ξ�δ�յ�����������Ϣ�ظ�,%s\n",
                CTime::GetCurrentTime().Format("%H:%M:%S"),m_tbPubTerminalInfo[i].GetDeviceNum());
            
        }
    }
}

//�ط�������Ϣ
BOOL CPubNetControl::ResendMessgToPubDevice(int nPubDeviceID,const char *pcSendMsgInfo,int nAckVaule)
{
    char tempStr[MAX_GPS_MESSAGE_LEN] = {0};
    char MessagePacket[MAX_GPS_MESSAGE_LEN] = {0};
    if(pcSendMsgInfo == NULL)
    {
        return FALSE;
    }
    
    if(strlen(pcSendMsgInfo) == 0)
    {
        return FALSE;
    }
    
    _snprintf(tempStr,MAX_GPS_MESSAGE_LEN-6,"HYT,%.2X,%s,%.4X,",APP_SEND_MESSAGE,pcSendMsgInfo,nAckVaule);
    
    int nChksum = GetChksum(tempStr);
    sprintf(MessagePacket,"*%s$%X#",tempStr,nChksum);
    
    //��������
    if( m_SockPubNetServer.Send(nPubDeviceID,
        MessagePacket,
        strlen(MessagePacket)) == TCPSERVER_ERROR_SEND)
    {            
        gWriteLogApp.WriteLog(FOR_ALARM,"%s [ID:%d]<����>�ط�������Ϣʧ��,�ر�TCP����,DeviceCode:%s\n",
            CTime::GetCurrentTime().Format("%H:%M:%S"),
            nPubDeviceID,
            m_tbPubTerminalInfo[nPubDeviceID].GetDeviceNum());
        
        m_SockPubNetServer.CloseClient(nPubDeviceID);
        
        return FALSE;
    }
    
    gWriteLogApp.WriteLog(FOR_TRACE,"%s [ID:%d]<����>�ط�������Ϣ,DeviceCode:%s,����:%s\n",
        CTime::GetCurrentTime().Format("%H:%M:%S"),
        nPubDeviceID,
        m_tbPubTerminalInfo[nPubDeviceID].GetDeviceNum(),
        MessagePacket);
    
    return TRUE;
    
}

//
void CPubNetControl::OnCheckGetSendMsgTxtTimerOut()
{
    int i = 0;
    char csSendMsgTxt[MAX_GPS_MESSAGE_LEN] ={0};
    int nMsgID = 0;
    int nAckValue = m_nAckCount[APP_SEND_MESSAGE];
    
    char MessagePacket[MAX_GPS_MESSAGE_LEN] = {0};
    char tempStr[MAX_GPS_MESSAGE_LEN] = {0};
    
    for(i = 0 ; i < MAX_PUBLICNET_CLIENT_CONNECT; i++)
    {
        if(m_tbPubTerminalInfo[i].GetMsgTxtTimerCheck(csSendMsgTxt,nMsgID))
        {
            _snprintf(tempStr,MAX_GPS_MESSAGE_LEN-6,"HYT,%.2X,%s,%.4X,",APP_SEND_MESSAGE,csSendMsgTxt,nAckValue);
            
            int nChksum = GetChksum(tempStr);
            sprintf(MessagePacket,"*%s$%X#",tempStr,nChksum);
            
            //��������
            if( m_SockPubNetServer.Send(i,
                MessagePacket,
                strlen(MessagePacket)) == TCPSERVER_ERROR_SEND)
            {
                gWriteLogApp.WriteLog(FOR_TRACE,"%s [ID:%d]<����>����������Ϣʧ��,�ر�TCP����,DeviceCode:%s,����:%s\n",
                    CTime::GetCurrentTime().Format("%H:%M:%S"),
                    i,
                    m_tbPubTerminalInfo[i].GetDeviceNum(),
                    MessagePacket);
                
                m_SockPubNetServer.CloseClient(i);
            }
            else
            {
                gWriteLogApp.WriteLog(FOR_TRACE,"%s [ID:%d]<����>����������Ϣ,DeviceCode:%s,����:%s\n",
                    CTime::GetCurrentTime().Format("%H:%M:%S"),
                    i,
                    m_tbPubTerminalInfo[i].GetDeviceNum(),
                    MessagePacket);
                
                //�����ط���ʱ
                m_tbPubTerminalInfo[i].StartSendMessageWaitAnswer( nAckValue,
                    nMsgID,
                    csSendMsgTxt,
                    m_nSendTxtMsgWaitAckTimeOut);
            }
            
            memset(MessagePacket,0,MAX_GPS_MESSAGE_LEN);
            
            m_nAckCount[APP_SEND_MESSAGE]++;
            if(m_nAckCount[APP_SEND_MESSAGE] >= 0xFFFF)
            {
                m_nAckCount[APP_SEND_MESSAGE] = 0;
            }
            
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
//FTP�ϴ�����


