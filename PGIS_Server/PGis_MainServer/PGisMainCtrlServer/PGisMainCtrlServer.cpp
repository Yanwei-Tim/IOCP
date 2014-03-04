// PGisMainCtrlServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PGisMainCtrlServer.h"
#include "rwini.h"
#include <conio.h>
#include "WinServiceModule.h"
#include "MemoryPool.h"
#include "Encrypt.h"

#include "HttpInterface/HttpInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
// The one and only application object
//CWinApp theApp;
#define WEB_CMDLIST_MAXCOUNT 256

BOOL g_bQuit = FALSE;

CWriteLogThread gWriteLogApp;
CPgisMainSer gPGisMainCtrlProc;

using namespace std;

::CCriticalSection g_csRecvClientList;
CList<RECV_CLIENTDATA, RECV_CLIENTDATA&> g_RecvClientList;

::CCriticalSection g_csRecvServerList;
CList<RECV_SERVERDATA, RECV_SERVERDATA&> g_RecvServerList;

UINT g_dwWorkThreadID = 0;

UINT g_dwWebThreadID = 0;
CMemoryPool gWebServiceList;

//��¼��ǰ���ӵ�GIS�ͻ�������
int g_nRecvConntGisClientCount = 0;

///////////////////////////////////ɽ����γ/////////////////////////////////////////////////
//
// typedef enum enumTimerType
// {
// 	SH_TCP_CONN_FAIL,
// 	SH_TCP_CONN_SUCCESS,
// 	SH_TCP_WAIT_LOGIN_ACK,
// 	SH_TCP_LOGIN_SUCCESS,
// 	SH_TCP_WAIT_LOGIN_MSG, //tcp���ӳɹ���ȴ���GPRS���ӵ�¼��Ϣ��ʱ
// 	SH_WAIT_CONN_TICK,
// }SH_TCPCONN_TYPE;
// 
// typedef enum enumShanHaiConnState
// {
// 	SH_TYE_LOGIN,
// 	SH_TYPE_TICK,
// 	SH_TYPE_GPSDATA,
// 
// }SH_CONNECT_STATE;
// 
// #define MIN_SHANHAI_PACKAGE_LEN     2
// #define MAX_SHANHAI_PACKAGE_LEN     128
// 
// #define SH_MAX_GPS_MESSAGE_LEN 256

///////////////////////////////////////////////////////////////////////////////////////////////
//
void PrintfHeadInfo()
{
  printf( "=========================HITOWN PGIS���س���=========================\n\
  CMD 'C' ����!\n\
  CMD 'L' ���¶�ȡ����������־����!\n\
  CMD 'S' �鿴ͳ������!\n\
  CMD 'Ctrl+S' ���ͳ������!\n\
  CMD 'D' �鿴�����ն���Ϣ!\n\
  CMD 'Ctrl+D' �鿴�����ն�����!\n\
  CMD 'G' �鿴GIS�ͻ�������!\n\
  CMD 'Ctrl+G' �鿴GIS�ͻ�������!\n\
  CMD 'Ctrl+Q' �˳�ϵͳ!\n\
==================================================================\n");
}

int gWriteGpsConnt = 0;
int gRecvGpsCount = 0;
int g_nRecvLocationGPSCount = 0;
int g_nSendLocationGPSCount = 0;
int g_nPatrAlertCount = 0;//��Ѳ���澯����ͳ��

void CheckKey( void *dummy )
{

    int ch;
    PrintfHeadInfo();
    do
    {
        ch = _getch();
        ch = toupper( ch );
        switch( ch )
        {
        case 'C':
            system("cls");
            PrintfHeadInfo();
            break;
        case 'L':
            gPGisMainCtrlProc.SetSysLogLevel();
			break;
		case 'S':
			printf("GPͳ��,��:%d������,д��ɹ�:%d������,�����λ�÷���:%d,����λ��:%d,Ѳ���澯:%d \n",gRecvGpsCount,gWriteGpsConnt,g_nSendLocationGPSCount,g_nRecvLocationGPSCount,g_nPatrAlertCount);
			break;
		case 'S' - 64:
			gRecvGpsCount = 0;
			gWriteGpsConnt = 0;
			g_nRecvLocationGPSCount = 0;
            g_nSendLocationGPSCount = 0;
			g_nPatrAlertCount = 0;
			printf("���ͳ�����ݳɹ�\n");
			break;
		case 'D':
			gPGisMainCtrlProc.m_BusinessManage_ConToPub.ShowOnLineDeviceInfo();
			break;
		case 'D' - 64:
			gPGisMainCtrlProc.m_BusinessManage_ConToPub.ShowOnLineDeviceNum();
			break;
		case 'G':
			gPGisMainCtrlProc.m_BusinessManage_GISTerminal.ShowLoginGisClientInfo();
			break;
		case 'G' -64:
			gPGisMainCtrlProc.m_BusinessManage_GISTerminal.ShowLoginGisNum();
        default:
            break;
        }
    }
    while( ch != ('Q'- 64) );
	
    g_bQuit = TRUE;
    _endthread();
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		// TODO: code your application's behavior here.
		CString strHello;
		strHello.LoadString(IDS_HELLO);
	}

#ifdef __SERVICE_MODE_
    CWinServiceModule MyWinService;

	CString tempstr =_T("pGisMainCtrlCfg.ini");

    Crwini rwIniTemp;
    if(!rwIniTemp.CheckIniFile(tempstr))
    {
		//MessageBox(NULL, _T("ϵͳ����ʧ��_1,�Ҳ��������ļ�"), "��ʾ", MB_OK);
        //gWriteLogApp.WriteLog(FOR_ALARM,"%s [SYS] ϵͳ����ʧ��_1,�Ҳ��������ļ�\n",CTime::GetCurrentTime().Format("%H:%M:%S"));
        //Sleep(100);
		return FALSE;
    }

	char m_strTempName[512] ={0};
    if(rwIniTemp.ReadString(_T("SystemSet"),_T("ServerName"),tempstr,m_strTempName) == 0) 
    {
        sprintf(m_strTempName,"HT_GisMainCtrlServer");
        
    }
    MyWinService.SetServiceName(m_strTempName);

    if(!MyWinService.ParseStandardArgs(argv[1]))
    {
		//�������û�а�װ ��װ֮
		if(!MyWinService.IsInstalled())
		{
			MyWinService.Install();

			return nRetCode;
		}

        MyWinService.StartService();
    }
#else
	//WideCharToMultiByte();
	//MultiByteToWideChar()
    _beginthread(CheckKey,0,NULL);
    Sleep(50);
    SetTimer(NULL,0,1000,0L);
    
    gPGisMainCtrlProc.Start();
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) 
    { 
        switch(msg.message)
        {              
            //��ʱ������
        case WM_TIMER:
            if(g_bQuit)
            {
                gPGisMainCtrlProc.Stop();
                Sleep(100);
                PostMessage(NULL,WM_QUIT,0,0);
            }
            break; 
        default:
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            break;
        }
    }
#endif
	return nRetCode;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//���IP��ַ��ʽ�Ƿ���ȷ
BOOL CheckIPAddressFormat(const char *strIPAddress)
{
    int IP[4];
    int i = 0;
    for(i = 0 ; i < 4; i++)
    {
        IP[i] = -1;
    }

    int iStrLen;
    iStrLen = strlen(strIPAddress);

    if(iStrLen > 16 || strIPAddress == NULL)
    {
        return FALSE;
    }

    _stscanf(strIPAddress, _T("%d.%d.%d.%d"),IP,IP+1,IP+2,IP+3); //sscanf
    for(i = 0 ; i < 4; i++)
    {
        if(IP[i] == -1 || IP[i] > 255)
        {
            return FALSE;
        }
    }
    //TCHAR szIpBuf[32] = {0};
    CString szIpBuf;
    szIpBuf.Format(_T("%d.%d.%d.%d"),IP[0],IP[1],IP[2],IP[3]); //sprintf

    int ilen = szIpBuf.GetLength();
    if(ilen != iStrLen)
    {
        return FALSE;
    }
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//
CPgisMainSer::CPgisMainSer()
{
    m_bSysStartFlag = FALSE;
    m_nMaxGisConnCount = -1;

    g_strConfigFile = _T("pGisMainCtrlCfg.ini");
}

CPgisMainSer::~CPgisMainSer()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////
//ת��������
BOOL CPgisMainSer::GetCpsxIdValue(const char *strValue)
{
    if(strlen(strValue) != 8)
    {
        return FALSE;
    }

	char tempBuf[16] = {0};
    
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

    return TRUE;
}

//������֤
BOOL CPgisMainSer::SysEncryptCheck()
{
    Crwini rwIniTemp;

    CString tempstr =_T("pGisMainCtrlCfg.ini");

    BOOL bRes = TRUE;
    CString strTempEncrypt1,strTempEncrypt2;
    if(rwIniTemp.ReadString(_T("SystemSet"),_T("EncryptText"),tempstr,strTempEncrypt1.GetBuffer(256)) == 0) 
    {
        bRes = FALSE ;
    }
    else
    {
        strTempEncrypt1.ReleaseBuffer();
    }

    CEncrypt tempEncryptClass;

    bRes = tempEncryptClass.CompareEncryptText(strTempEncrypt1,m_nMaxGisConnCount);
    if(!bRes)
    {
        CString strPswdTxt;
        strPswdTxt = tempEncryptClass.OutputPWSDText();

        TCHAR szFilePath[MAX_PATH] ={0};
        ::GetModuleFileName(NULL, szFilePath, MAX_PATH);

        (strrchr(szFilePath,'\\'))[1] = 0; 
        char *fn = "HYTPswd.txt";
        strcat(szFilePath,fn);

        char m_pswdInfo[512]={0};

        strcat(m_pswdInfo,strPswdTxt);

        FILE *fhandle;
        fhandle = fopen(szFilePath, "w+");
        if(fhandle == NULL)
        {
            return bRes;
        }

        if(fseek(fhandle, 0, SEEK_END) != 0)
        {
            fclose(fhandle);
            return bRes;
        }

        fwrite((LPCTSTR)m_pswdInfo, 1, strlen(m_pswdInfo), fhandle);
        fclose(fhandle);

    }
    return bRes;
}

//ת���ɿ��ַ�
BOOL CPgisMainSer::GetUnCodeTextInfo(CString strPatroAlertMessage ,char *pPatroAlertInfo)
{	
    unsigned char TempAlertInfo[MAX_LEN_TEXT_MSG] ={0};
    int size;

    int nTempLen = strPatroAlertMessage.GetLength();
    size=::MultiByteToWideChar(CP_ACP,0,strPatroAlertMessage,nTempLen,NULL,0);
    if(size==0)
    {
        return FALSE;
    }
    wchar_t *widebuff=new wchar_t[size];
    ::MultiByteToWideChar(CP_ACP,0,strPatroAlertMessage,nTempLen,widebuff,size);

    int ucodeLen = sizeof(wchar_t)*size;
    memcpy(TempAlertInfo,widebuff,ucodeLen);

    CString strTemp;
    CString strAlertInf = _T("");
    int i;
    for(i = 0 ; i < ucodeLen; i++)
    {
        strTemp.Format("%.2X",TempAlertInfo[i]);
        strAlertInf = strAlertInf + strTemp;
    }

    nTempLen = strAlertInf.GetLength();
    if(nTempLen >= MAX_LEN_TEXT_MSG)
    {
        nTempLen = MAX_LEN_TEXT_MSG - 2;
    }

    memcpy(pPatroAlertInfo,(char *)(LPCTSTR)strAlertInf,nTempLen);

    if(widebuff)
    {
        delete []widebuff;
    }
    return TRUE;

}

//�汾��Ϣ
void CPgisMainSer::InitMyVersion()
{
    char constVersion[128] = {0};

    _snprintf(constVersion, 128-1, "Hitom����GPS������� Version:%s, Build Time:%s %s", 
        "1.2.0.6", __DATE__, __TIME__);

    gWriteLogApp.WriteLog(FOR_ALARM,_T("%s [SYS] %s \n"),CTime::GetCurrentTime().Format("%H:%M:%S"),constVersion);
}

//������־ˮƽ
void CPgisMainSer::SetSysLogLevel()
{
    int nTempLogLevel;
    nTempLogLevel = rwIniTemp.GetInt(_T("SystemSet"),_T("LogLevel"),g_strConfigFile);
    if(m_nLogLevel != nTempLogLevel)
    {
        gWriteLogApp.SetLogLevel(nTempLogLevel);

        m_nLogLevel = nTempLogLevel;
    }
}

//��ȡϵͳ��������
BOOL CPgisMainSer::LoadSystemConfig()
{
    if(!rwIniTemp.CheckIniFile(g_strConfigFile))
    {
        gWriteLogApp.WriteLog(FOR_ALARM,_T("%s [SYS] ϵͳ����ʧ��,�Ҳ��������ļ�\n"),CTime::GetCurrentTime().Format("%H:%M:%S"));
        return FALSE;
    }

    //��־����
    m_nLogLevel = rwIniTemp.GetInt(_T("SystemSet"),_T("LogLevel"),g_strConfigFile);

	//ShanHai��������
	m_nShanhaiCount = rwIniTemp.GetInt(_T("SystemSet"),_T("ShanhaiCount"),g_strConfigFile);

	//��ȡ������ת������
    m_nL = rwIniTemp.GetIntX(_T("SetCpsx"),_T("L"),g_strConfigFile);
	if(m_nL < 0)
	{
		m_nL = 10;
	}
    m_nM = rwIniTemp.GetIntX(_T("SetCpsx"),_T("M"),g_strConfigFile);
	if(m_nM < 0)
	{
		m_nM = 10;
	}
    m_nS = rwIniTemp.GetIntX(_T("SetCpsx"),_T("S"),g_strConfigFile);
	if(m_nS < 0)
	{
		m_nS = 10;
	}

	CString strAlertInfo;
	if(rwIniTemp.ReadString(_T("SystemSet"),_T("PatroAlertTextInfo"),g_strConfigFile,strAlertInfo.GetBuffer(256)) == 0) 
    {
        strAlertInfo = _T("���Ѿ�������Ѳ������");
    }
	else
	{
		strAlertInfo.ReleaseBuffer();
	}
	memset(m_PatroAlertTextInfo,0,MAX_LEN_TEXT_MSG);
	GetUnCodeTextInfo(strAlertInfo,m_PatroAlertTextInfo);

    return TRUE;
}

//ϵͳ����
BOOL CPgisMainSer::Start()
{
    if(m_bSysStartFlag)
    {
        return FALSE;
    }
    //�汾��Ϣ
    InitMyVersion();

    //Load�����ļ�
    if(!LoadSystemConfig())
    {
        return FALSE;
    }

#ifdef __SYS_ENCRYPT_CHECK_
    if(!SysEncryptCheck())
    {
        gWriteLogApp.WriteLog( FOR_ALARM ,_T("%s [SYS] ϵͳ����ʧ��,�Ƿ��ļ��ܴ�!\n"),
            CTime::GetCurrentTime().Format("%H:%M:%S") );

        //MessageBox(NULL,"���ط�������ʧ��,�Ƿ��ļ��ܴ�!","����",MB_ICONERROR);
        Sleep(10);
        return FALSE;
    }
    else
    {
        gWriteLogApp.WriteLog( FOR_ALARM ,_T("%s [SYS] ������֤�ɹ�!\n"),
            CTime::GetCurrentTime().Format("%H:%M:%S") );
    }
#endif

    //������־����
    gWriteLogApp.SetLogLevel(m_nLogLevel);

    gWriteGpsConnt = 0;
    gRecvGpsCount = 0;

    g_nRecvLocationGPSCount = 0;
    g_nSendLocationGPSCount = 0;

    g_nPatrAlertCount = 0;

	if (!m_DBManage_WebServer.StartThread())
	{
		return FALSE;
	}

	if (!m_iocpsocket.Start())
	{
		return FALSE;
	}

	if (m_BusinessManage_GISTerminal.Start())
	{
		g_dwGISTerminalContext = m_BusinessManage_GISTerminal.GetContext();
	}

	if (m_BusinessManage_ConToPub.Start())
	{
		g_dwConToPubContext = m_BusinessManage_ConToPub.GetContext();
	}

	if (m_BusinessManage_ConnectToLocationServer.Start())
	{
		g_dwLocationServerContext = m_BusinessManage_ConnectToLocationServer.GetContext();
	}	

	if (m_BusinessManage_ConnectToGPSCheckServer.Start())
	{
		g_dwGPSCheckServerContext = m_BusinessManage_ConnectToGPSCheckServer.GetContext();
	}

	for (int nCount = 0;nCount < m_nShanhaiCount;++nCount)
	{
		CBusinessManage_ConnectToShanhaiServer* m_PBusinessManage_ConnectToShanhaiServer = new CBusinessManage_ConnectToShanhaiServer;
		DWORD g_dwShanhaiServerContext;
		if (m_PBusinessManage_ConnectToShanhaiServer->Start(nCount))
		{
			g_dwShanhaiServerContext = m_PBusinessManage_ConnectToShanhaiServer->GetContext();
			m_BusinessManage_ConnectToShanhaiServer_vec.push_back(m_PBusinessManage_ConnectToShanhaiServer);
		}
	}

    m_bSysStartFlag = TRUE;

    return TRUE;
}

//ϵͳֹͣ
void CPgisMainSer::Stop()
{
    if(m_bSysStartFlag)
    {
        gWriteLogApp.WriteLog(FOR_ALARM,"%s [SYS] �˳�GIS����,����GPS����:%d��,д��ok:%d��,�����λ�÷���:%d,��λ��:%d,Ѳ���澯:%d\n",
            CTime::GetCurrentTime().Format("%H:%M:%S"),
            gRecvGpsCount,
            gWriteGpsConnt,
            g_nSendLocationGPSCount,
            g_nRecvLocationGPSCount,
            g_nPatrAlertCount);

		m_iocpsocket.Stop();
		m_DBManage_WebServer.StopThread();
		
        m_bSysStartFlag = FALSE;	
    }
}
