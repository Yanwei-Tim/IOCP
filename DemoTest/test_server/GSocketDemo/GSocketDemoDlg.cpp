// GSocketDemoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"

#include "GSockInfo.h"
#include "GSocketDemo.h"
#include "GSocketDemoDlg.h"

#include "..\GLog.h"
#include "..\GMemory.h"
#include "..\GSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef struct _DATA_INFO
{
DWORD			dwSendBytes;
DWORD			dwRecvBytes;
DWORD			dwLong;
float			fSpeed;
DWORD			dwDataError;
DWORD			dwTickCountSend;
}DATA_INFO, *PDATA_INFO;

DATA_INFO DataInfo;

typedef struct _DATA_HEAD
{
DWORD			dwSender;
DWORD			dwSize;
DWORD			dwTickCount;
}DATA_HEAD, *PDATA_HEAD;

typedef struct _CLIENT_DATA_INFO
{
	DWORD	dwNo;
	BOOL	sendLogin;
}CLIENT_DATA_INFO,*PCLIENT_DATA_INFO;

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

int Random(int nNumber) 
{ 
	return (int)(nNumber/(float)RAND_MAX * rand());
}

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CGSocketDemoDlg �Ի���
#ifdef WANGZH //2013-5-6
int GetChksum(const char *strMsg)
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

BOOL OnPostLoginClient(CGSocketDemoDlg* pDlg, const DWORD nIndex, const DWORD dwClientContext)
{
	char tempStr[128] = {0};
	char sendBuf[256] = {0};
	if (GHndDat_GetState(dwClientContext) == GHND_STATE_CONNECTED)
	{
//		sprintf(tempStr,"%d,2A,123456,H6,00,,",PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->dwNo);
//		int nChksum = GetChksum(tempStr);

		sprintf(tempStr,"%d,H5,00,NULL",PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->dwNo);
		sprintf(sendBuf,"%s%.4X00%s#","*HITOM",strlen(tempStr),tempStr);
		GCommProt_PostSendBuf((DWORD)dwClientContext, sendBuf, strlen(sendBuf));
	}

	return(TRUE);
}

BOOL OnPostStatesClient(CGSocketDemoDlg* pDlg, const DWORD nIndex, const DWORD dwClientContext)
{
	char tempStr[128] = {0};
	char sendBuf[256] = {0};
	if (PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->sendLogin  && (GHndDat_GetState(dwClientContext) == GHND_STATE_CONNECTED))
	{
//		sprintf(tempStr,"%d,30,123424,231543,123.124,324.123,04,\u5DE1\u903B\u4EFB\u52A1\u4E2D,00,,",PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->dwNo);
//		int nChksum = GetChksum(tempStr);
//		sprintf(sendBuf,"%s$%X",tempStr,nChksum);

		sprintf(tempStr,"%d,H5,00,130528,060554,2230.0035,11354.9463,00,02,\u6C42\u52A9",PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->dwNo);
		sprintf(sendBuf,"%s%.4X11%s#","*HITOM",strlen(tempStr),tempStr);
		GCommProt_PostSendBuf((DWORD)dwClientContext, sendBuf, strlen(sendBuf));
	}

	return(TRUE);
}

BOOL OnPostAlemaltClient(CGSocketDemoDlg* pDlg, const DWORD nIndex, const DWORD dwClientContext)
{
	char tempStr[128] = {0};
	char sendBuf[256] = {0};
	if (PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->sendLogin  && (GHndDat_GetState(dwClientContext) == GHND_STATE_CONNECTED))
	{
//		sprintf(tempStr,"%d,3E,000011,N,3725.3203,E,00005.0456,A,0.00,030,130502,00,01,1233,",PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->dwNo);
// 		int nChksum = GetChksum(tempStr);
// 		sprintf(sendBuf,"%s$%X",tempStr,nChksum);

		sprintf(tempStr,"%d,H5,00,171332,N,2229.9931,E,11354.9026,A,0.00,030,130529,01|460|03|0|0|9202|2|13844|0|0",PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->dwNo);
		sprintf(sendBuf,"%s%.4X10%s#","*HITOM",strlen(tempStr),tempStr);
		GCommProt_PostSendBuf((DWORD)dwClientContext, sendBuf, strlen(sendBuf));
	}

	return(TRUE);
}

BOOL OnHeartDataClient(CGSocketDemoDlg* pDlg, const DWORD nIndex, const DWORD dwClientContext)
{
	char tempStr[128] = {0};
	char sendBuf[256] = {0};
	if (PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->sendLogin  && (GHndDat_GetState(dwClientContext) == GHND_STATE_CONNECTED))
	{
// 		sprintf(tempStr,"%d,2D,,,,",PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->dwNo);
// 		int nChksum = GetChksum(tempStr);
// 		sprintf(sendBuf,"%s$%X",tempStr,nChksum);

		sprintf(tempStr,"%d,H5,00,NULL",PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->dwNo);
		sprintf(sendBuf,"%s%.4X02%s#","*HITOM",strlen(tempStr),tempStr);
		GCommProt_PostSendBuf((DWORD)dwClientContext, sendBuf, strlen(sendBuf));
	}

	return(TRUE);
}

BOOL OnPostGPRSDataClient(CGSocketDemoDlg* pDlg, const DWORD nIndex, const DWORD dwClientContext)
{
	char tempStr[128] = {0};
	char sendBuf[256] = {0};
	if (PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->sendLogin  && (GHndDat_GetState(dwClientContext) == GHND_STATE_CONNECTED))
	{
// 		sprintf(tempStr,"%d,21,000011,N,3725.3203,E,00005.0456,A,0.00,030,130502,00,11,",PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->dwNo);
// 		int nChksum = GetChksum(tempStr);
// 		sprintf(sendBuf,"%s$%X",tempStr,nChksum);
// 		GCommProt_PostSendBuf((DWORD)dwClientContext, sendBuf, strlen(sendBuf));

		sprintf(tempStr,"%d,H5,00,163826,N,2230.0180,E,11354.9148,V,0.00,030,130609,01|460|03|0|0|9204|2|13844|1640487|323943",PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->dwNo);
		sprintf(sendBuf,"%s%.4X10%s#","*HITOM",strlen(tempStr),tempStr);
		GCommProt_PostSendBuf((DWORD)dwClientContext, sendBuf, strlen(sendBuf));
	}

	return(TRUE);
}
#endif

BOOL OnTraversalClient(CGSocketDemoDlg* pDlg, const DWORD nIndex, const DWORD dwClientContext)	//ҵ���
{
	PDATA_INFO pDataInfo = (PDATA_INFO)GHndDat_GetData(dwClientContext);
	DWORD dwTickCount = GetTickCount();
	if(pDataInfo && (GHndDat_GetState(dwClientContext) == GHND_STATE_CONNECTED) && (dwTickCount - pDataInfo->dwTickCountSend > pDlg->m_dwSendSpace))
	{
		pDataInfo->dwTickCountSend = dwTickCount;
		PGIO_BUF pBuf = GIoDat_AllocGBuf();
		if(pBuf)
		{
			PDATA_INFO pDataInfo = (PDATA_INFO)GHndDat_GetData(dwClientContext);
			PDATA_HEAD(pBuf)->dwSender = DWORD(pDataInfo);
			PDATA_HEAD(pBuf)->dwTickCount = GetTickCount();
			PDATA_HEAD(pBuf)->dwSize = pDlg->m_dwPackSize;
			
			DWORD i;
			for(i = sizeof(DATA_HEAD); i < pDlg->m_dwPackSize; i++)
			{
				*(char *)(pBuf + i) = (char)i;
			}
			
			GCommProt_PostSendGBuf(dwClientContext, pBuf, pDlg->m_dwPackSize);
		}
	}
	return(TRUE);
}

DWORD WINAPI ThreadSend(CGSocketDemoDlg* pDlg)
{
	while(!pDlg->m_bShutdownSend)
	{
		Sleep(1000);
		GTcpClt_TraversalClient((DWORD)pDlg, 0, GTcpClt_GetClientCount(), (PFN_ON_GSOCK_TRAVERSAL)&OnTraversalClient);
	}
	pDlg->m_bShutdownSend = FALSE;
	return(0);
}

DWORD WINAPI ThreadDisconnect(CGSocketDemoDlg* pDlg)
{
	while(1)
	{
		Sleep(1000);
		if(pDlg->m_bRandoDisconnect)
		{
			int nIndex = pDlg->m_lstInfo.GetItemCount();
			if(nIndex)
			{
				nIndex = Random(nIndex * 2);
				if(nIndex < pDlg->m_lstInfo.GetItemCount())
				{
					GTcpClt_DisconnectClient((DWORD)(pDlg->m_lstInfo.GetItemData(nIndex)));
				}
			}
		}
	}
	return(0);
}


CGSocketDemoDlg::CGSocketDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGSocketDemoDlg::IDD, pParent)
	, m_dwGMemPageSize(0)
	, m_bCloseMtu(FALSE)
	, m_dwConnectionCount(0)
{
	char szIp[30];

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_dwPostRecvNumber = GWkrThrd_GetWorkerThreadNumberDef();
	m_nSocketBufSize = 8192;
	m_dwWorkerThreadNumber = GWkrThrd_GetWorkerThreadNumberDef();
	m_dwConcurrentThreadNumber = GWkrThrd_GetWorkerThreadNumberDef();
	m_dwPostAcceptNumber = GWkrThrd_GetWorkerThreadNumberDef();
	m_dwSendLoop = 0;
	GSock_GetLocalNetIp(szIp, 28, FALSE);
	m_strListenIp.Format(_T("%s"), szIp);
	m_dwListenPort = 10000;
	m_dwSendSpace = 100;
	m_dwPackSize = GIoDat_GetGBufSize() - 2 * (sizeof(SOCKADDR_IN) + 16);
	m_dwHeartbeat = 30;
	m_dwConnectNumber = 1;
	m_strLocalIp.Format(_T("%s"), szIp);
	m_dwLocalPort = 20030;
	m_strRemoteIp.Format(_T("%s"), szIp);
	m_dwRemotePort = 10002;
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

	ZeroMemory(&DataInfo, sizeof(DATA_INFO));
	DataInfo.dwLong = GetTickCount();
}

void CGSocketDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GEDIT_1, m_edtGMemPageSize);
	DDX_Text(pDX, IDC_GEDIT_1, m_dwGMemPageSize);
	DDX_Control(pDX, IDC_GEDIT_2, m_edtGMemTotal);
	DDX_Text(pDX, IDC_GEDIT_2, m_dwGMemTotal);
	DDX_Control(pDX, IDC_GEDIT_3, m_edtGBufSize);
	DDX_Text(pDX, IDC_GEDIT_3, m_dwGBufSize);
	DDX_Control(pDX, IDC_GEDIT_4, m_edtIoDataTotal);
	DDX_Text(pDX, IDC_GEDIT_4, m_dwIoDataTotal);
	DDX_Control(pDX, IDC_GEDIT_5, m_edtHndDataTotal);
	DDX_Text(pDX, IDC_GEDIT_5, m_dwHndDataTotal);
	DDX_Control(pDX, IDC_GEDIT_6, m_edtPostRecvNumber);
	DDX_Text(pDX, IDC_GEDIT_6, m_dwPostRecvNumber);
	DDX_Control(pDX, IDC_GEDIT_7, m_edtSocketBufSize);
	DDX_Text(pDX, IDC_GEDIT_7, m_nSocketBufSize);
	DDX_Control(pDX, IDC_GEDIT_8, m_edtWorkerThreadNumber);
	DDX_Text(pDX, IDC_GEDIT_8, m_dwWorkerThreadNumber);
	DDX_Control(pDX, IDC_GEDIT_9, m_edtConcurrentThreadNumber);
	DDX_Text(pDX, IDC_GEDIT_9, m_dwConcurrentThreadNumber);
	DDX_Control(pDX, IDC_GEDIT_10, m_edtAcceptOvertime);
	DDX_Text(pDX, IDC_GEDIT_10, m_dwAcceptOvertime);
	DDX_Control(pDX, IDC_GEDIT_11, m_edtIdleOvertime);
	DDX_Text(pDX, IDC_GEDIT_11, m_dwIdleOvertime);
	DDX_Control(pDX, IDC_GEDIT_12, m_edtMaxConnection);
	DDX_Text(pDX, IDC_GEDIT_12, m_dwMaxConnection);
	DDX_Control(pDX, IDC_GEDIT_13, m_edtPostAcceptNumber);
	DDX_Text(pDX, IDC_GEDIT_13, m_dwPostAcceptNumber);
	DDX_Control(pDX, IDC_GEDIT_14, m_edtSendLoop);
	DDX_Text(pDX, IDC_GEDIT_14, m_dwSendLoop);
	DDX_Control(pDX, IDC_GEDIT_15, m_edtListenIp);
	DDX_Text(pDX, IDC_GEDIT_15, m_strListenIp);
	DDX_Control(pDX, IDC_GEDIT_16, m_edtListenPort);
	DDX_Text(pDX, IDC_GEDIT_16, m_dwListenPort);
	DDX_Control(pDX, IDC_GEDIT_17, m_edtSendSpace);
	DDX_Text(pDX, IDC_GEDIT_17, m_dwSendSpace);
	DDX_Control(pDX, IDC_GEDIT_18, m_edtPackSize);
	DDX_Text(pDX, IDC_GEDIT_18, m_dwPackSize);
	DDX_Control(pDX, IDC_GEDIT_19, m_edtHeartbeat);
	DDX_Text(pDX, IDC_GEDIT_19, m_dwHeartbeat);
	DDX_Control(pDX, IDC_GEDIT_20, m_edtConnectNumber);
	DDX_Text(pDX, IDC_GEDIT_20, m_dwConnectNumber);
	DDX_Control(pDX, IDC_GEDIT_21, m_edtLocalIp);
	DDX_Text(pDX, IDC_GEDIT_21, m_strLocalIp);
	DDX_Control(pDX, IDC_GEDIT_22, m_edtLocalPort);
	DDX_Text(pDX, IDC_GEDIT_22, m_dwLocalPort);
	DDX_Control(pDX, IDC_GEDIT_23, m_edtRemoteIp);
	DDX_Text(pDX, IDC_GEDIT_23, m_strRemoteIp);
	DDX_Control(pDX, IDC_GEDIT_24, m_edtRemotePort);
	DDX_Text(pDX, IDC_GEDIT_24, m_dwRemotePort);
	DDX_Control(pDX, IDC_GEDIT_25, m_edtSendBytes);
	DDX_Text(pDX, IDC_GEDIT_25, m_dwSendBytes);
	DDX_Control(pDX, IDC_GEDIT_26, m_edtRecvBytes);
	DDX_Text(pDX, IDC_GEDIT_26, m_dwRecvBytes);
	DDX_Control(pDX, IDC_GEDIT_27, m_edtByteTotal);
	DDX_Text(pDX, IDC_GEDIT_27, m_dwByteTotal);
	DDX_Control(pDX, IDC_GEDIT_28, m_edtLong);
	DDX_Text(pDX, IDC_GEDIT_28, m_dwLong);
	DDX_Control(pDX, IDC_GEDIT_29, m_edtBeat);
	DDX_Text(pDX, IDC_GEDIT_29, m_fBeat);

	DDX_Control(pDX, IDC_CHECK1, m_bnCloseMtu);
	DDX_Check(pDX, IDC_CHECK1, m_bCloseMtu);
	DDX_Control(pDX, IDC_CHECK2, m_bnZeroAccept);
	DDX_Check(pDX, IDC_CHECK2, m_bZeroAccept);
	DDX_Control(pDX, IDC_CHECK3, m_bnZeroRecv);
	DDX_Check(pDX, IDC_CHECK3, m_bZeroRecv);
	DDX_Control(pDX, IDC_CHECK4, m_bnSendSelf);
	DDX_Check(pDX, IDC_CHECK4, m_bSendSelf);
	DDX_Control(pDX, IDC_CHECK5, m_bnRandoDisconnect);
	DDX_Check(pDX, IDC_CHECK5, m_bRandoDisconnect);

	DDX_Control(pDX, IDC_BN_TCP_LISTEN, m_bnTcpListen);
	DDX_Control(pDX, IDC_BN_UDP_LISTEN, m_bnUdpListen);
	DDX_Control(pDX, IDC_BN_TCP_CONNECT, m_bnTcpConnect);
	DDX_Control(pDX, IDC_BN_UDP_CONNECT, m_bnUdpConnect);

#ifdef WANGZH //2013-5-6
	DDX_Control(pDX, IDC_BN_TCP_LOGIN, m_bnTcpLogin);
#endif

	DDX_Control(pDX, IDC_BN_SERVICE, m_bnService);
	DDX_Control(pDX, IDC_GLIST3, m_lstInfo);
	DDX_Text(pDX, IDC_GEDIT_WAIT_ACCEPT3, m_dwConnectionCount);
	DDX_Control(pDX, IDC_BN_TCP_STATES, m_bnTcpStates);
	DDX_Control(pDX, IDC_BN_CLIENT_Alarm, m_bnclientalarm);
}

BEGIN_MESSAGE_MAP(CGSocketDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CGSocketDemoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CGSocketDemoDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON2, &CGSocketDemoDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_CHECK4, &CGSocketDemoDlg::OnBnClickedCheck4)
	ON_BN_CLICKED(IDC_CHECK5, &CGSocketDemoDlg::OnBnClickedCheck4)
	ON_EN_CHANGE(IDC_GEDIT_17, &CGSocketDemoDlg::OnEnChangeGedit17)
	ON_EN_CHANGE(IDC_GEDIT_14, &CGSocketDemoDlg::OnBnClickedCheck4)
	ON_BN_CLICKED(IDC_GEDIT_18, &CGSocketDemoDlg::OnBnClickedCheck4)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BN_SERVICE, &CGSocketDemoDlg::OnBnClickedBnService)
	ON_BN_CLICKED(IDC_BN_TCP_LISTEN, &CGSocketDemoDlg::OnBnClickedBnTcpListen)
	ON_BN_CLICKED(IDC_BN_TCP_CONNECT, &CGSocketDemoDlg::OnBnClickedBnTcpConnect)
	ON_BN_CLICKED(IDC_BUTTON6, &CGSocketDemoDlg::OnBnClickedButton6)
	ON_EN_CHANGE(IDC_GEDIT_18, &CGSocketDemoDlg::OnEnChangeGedit18)
	ON_BN_CLICKED(IDC_BUTTON3, &CGSocketDemoDlg::OnBnClickedButton3)

#ifdef WANGZH //2013-5-6
	ON_BN_CLICKED(IDC_BN_TCP_LOGIN, &CGSocketDemoDlg::OnBnClickedBnTcpLogin)
#endif

	ON_BN_CLICKED(IDC_BN_TCP_STATES, &CGSocketDemoDlg::OnBnClickedBnTcpStates)
	ON_BN_CLICKED(IDC_BN_CLIENT_Alarm, &CGSocketDemoDlg::OnBnClickedBnClientAlarm)
END_MESSAGE_MAP()


// CGSocketDemoDlg ��Ϣ�������

void SetListCtrlStyleDemo(CListCtrl* pLstCtrl)
{
	//���ԭ�з��
	DWORD dwStyle = GetWindowLong(pLstCtrl->m_hWnd, GWL_STYLE); 
	dwStyle &= ~(LVS_TYPEMASK);
	dwStyle &= ~(LVS_EDITLABELS);

	//�����·��
    SetWindowLong(pLstCtrl->m_hWnd, GWL_STYLE, dwStyle | LVS_REPORT | LVS_NOLABELWRAP | LVS_SHOWSELALWAYS);

	//������չ���
	DWORD styles = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	ListView_SetExtendedListViewStyleEx(pLstCtrl->m_hWnd, styles, styles );
}

BOOL CGSocketDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	SetListCtrlStyleDemo(&m_lstInfo);

	LV_COLUMN col;

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("��");
	col.cx = 25;
	m_lstInfo.InsertColumn(0, &col);

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("����");
	col.cx = 60;
	m_lstInfo.InsertColumn(1, &col);

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("״̬");
	col.cx = 50;
	m_lstInfo.InsertColumn(2, &col);

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("Զ��");
	col.cx = 50;
	m_lstInfo.InsertColumn(3, &col);

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("����");
	col.cx = 70;
	m_lstInfo.InsertColumn(4, &col);

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("����");
	col.cx = 70;
	m_lstInfo.InsertColumn(5, &col);

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("�շ�");
	col.cx = 70;
	m_lstInfo.InsertColumn(6, &col);

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("˫�̺�ʱ");
	col.cx = 70;
	m_lstInfo.InsertColumn(7, &col);

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("˫������");
	col.cx = 70;
	m_lstInfo.InsertColumn(8, &col);

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("��½����");
	col.cx = 60;
	m_lstInfo.InsertColumn(9, &col);

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("���ݷ��ʹ���");
	col.cx = 60;
	m_lstInfo.InsertColumn(10, &col);

	srand((ULONG)(time(NULL)));

	SetTimer(10000, 1000, NULL);
	SetTimer(10001, 15000, NULL);

	DWORD dwThreadId;
	CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ThreadDisconnect, this, 0, &dwThreadId));

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CGSocketDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CGSocketDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CGSocketDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CGSocketDemoDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnOK();
}

void CGSocketDemoDlg::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	KillTimer(10000);
	KillTimer(10001);
	BOOL bIsSend = GTcpClt_GetClientCount() > 0;
	m_bShutdownSend = TRUE;		
	if(bIsSend)
		while(m_bShutdownSend)
			Sleep(1);
	if(GSock_IsActive())
	{
		GInfo_ClearList();
		GSock_StopService();
	}
	OnCancel();
}

void CGSocketDemoDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GMem_Create();
	GInfo_Show(this);
}

void CGSocketDemoDlg::OnBnClickedCheck4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
}

void CGSocketDemoDlg::OnEnChangeGedit17()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ�������������
	// ���͸�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	/*if(GTcpClt_GetClientCount())
	{
		UpdateData(TRUE);
		KillTimer(10001);
		SetTimer(10001, m_dwSendSpace, NULL);
	}*/
}

void CGSocketDemoDlg::EnabledCtrls(BOOL bIsActive)
{
	m_edtGMemPageSize.EnableWindow(!bIsActive);
	m_edtGMemTotal.EnableWindow(!bIsActive);
	m_edtGBufSize.EnableWindow(!bIsActive);
	m_edtIoDataTotal.EnableWindow(!bIsActive);
	m_edtHndDataTotal.EnableWindow(!bIsActive);
	m_edtPostRecvNumber.EnableWindow(!bIsActive);
	m_edtSocketBufSize.EnableWindow(!bIsActive);
	m_edtWorkerThreadNumber.EnableWindow(!bIsActive);
	m_edtConcurrentThreadNumber.EnableWindow(!bIsActive);
	m_edtAcceptOvertime.EnableWindow(!bIsActive);
	m_edtIdleOvertime.EnableWindow(!bIsActive);
	m_edtMaxConnection.EnableWindow(!bIsActive);
	m_edtPostAcceptNumber.EnableWindow(!bIsActive);
	m_edtHeartbeat.EnableWindow(!bIsActive);

	m_bnCloseMtu.EnableWindow(!bIsActive);
	m_bnZeroAccept.EnableWindow(!bIsActive);
	m_bnZeroRecv.EnableWindow(!bIsActive);
	m_bnTcpListen.EnableWindow(bIsActive);
	m_bnUdpListen.EnableWindow(bIsActive);
	m_bnTcpConnect.EnableWindow(bIsActive);
	m_bnUdpConnect.EnableWindow(bIsActive);

#ifdef WANGZH //2013-5-6
	m_bnTcpLogin.EnableWindow(bIsActive);
	m_bnTcpStates.EnableWindow(bIsActive);
	m_bnclientalarm.EnableWindow(bIsActive);
#endif
}


void CGSocketDemoDlg::GetGSocketInfo(void)
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
	m_dwHeartbeat = 30;
	UpdateData(FALSE);
}

void CGSocketDemoDlg::OnBnClickedBnService()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
		TRACE("%d",this);
	if(GSock_IsActive())
	{
		//KillTimer(10001);		
		m_bShutdownSend = TRUE;
		m_lstInfo.DeleteAllItems();
		GInfo_ClearList();
		GSock_StopService();
		m_dwListenPort = 9002;
		UpdateData(FALSE);
		m_bnService.SetWindowText(_T("��ʼ����"));
		m_bnTcpListen.SetWindowText(_T("TCP����(��δ����)"));
		m_bnUdpListen.SetWindowText(_T("UDP����"));
		OnBnClickedButton6();
		EnabledCtrls(FALSE);
	}else
	{		
		UpdateData(TRUE);
		if(m_dwPackSize > GIoDat_GetGBufSize() - 2 * (sizeof(SOCKADDR_IN) + 16))
			m_dwPackSize = GIoDat_GetGBufSize() - 2 * (sizeof(SOCKADDR_IN) + 16);

		GMem_SetPageSize(m_dwGMemPageSize);
		GMem_SetTotalBytes(m_dwGMemTotal);
		GIoDat_SetGBufSize(m_dwGBufSize);
		GIoDat_SetTotal(m_dwIoDataTotal);
		GHndDat_SetTotal(m_dwHndDataTotal);
		GSock_SetNumberPostRecv(m_dwPostRecvNumber);
		GSock_SetMtuNodelay(m_bCloseMtu);
		GSock_SetZeroByteRecv(m_bZeroRecv);
		GSock_SetZeroByteAccept(m_bZeroAccept);
		GSock_SetSocketSendBufSize(m_nSocketBufSize);
		GSock_SetSocketRecvBufSize(m_nSocketBufSize);
		GWkrThrd_SetWorkerThreadNumber(m_dwWorkerThreadNumber);
		GWkrThrd_SetConcurrentThreadNumber(m_dwConcurrentThreadNumber);
		GSock_SetTimeAcceptOvertime(m_dwAcceptOvertime);
		GSock_SetTimeIdleOvertime(m_dwIdleOvertime);
		GSock_SetMaxNumberConnection(m_dwMaxConnection);
		GSock_SetNumberPostAccept(m_dwPostAcceptNumber);

		GSock_SetTimeHeartbeat(m_dwHeartbeat);

		GSock_SetOnConnectProcTcpSvr(&OnConnectSvr);
		GSock_SetOnDisconnectProcSvr(&OnDisconnectSvr);
		GSock_SetOnReceiveProcSvr(&OnReceiveSvr);
		GSock_SetOnSendedProcSvr(&OnSendedSvr);
		GSock_SetOnSendErrorProcSvr(&OnSendErrorSvr);
		GSock_SetOnIdleOvertimeProc(&OnIdleOvertime);
		GSock_SetOnConnectionOverflowProc(&OnConnectionOverflow);

//-----------------------------client-------------------------------------
		GSock_SetOnConnectProcClt(&OnConnectClt);
		GSock_SetOnConnectErrorProc(&OnConnectError);
		GSock_SetOnDisconnectProcClt(&OnDisconnectClt);

		GSock_SetOnCreateClientProc(&OnCreateClient);
		GSock_SetOnDestroyClientProc(&OnDestroyClient);

		GSock_SetOnReceiveProcClt(&OnReceiveClt);
		GSock_SetOnSendedProcClt(&OnSendedClt);
		GSock_SetOnSendErrorProcClt(&OnSendErrorClt);
		GSock_SetOnHeartbeatProc(&OnHeartbeat);
		GSock_SetOnProfessionalWorkProc(&OnProfessionalWork);

		GSock_StartService();
		if(GSock_IsActive())
		{
			m_bnService.SetWindowText(_T("ֹͣ����"));
			EnabledCtrls(TRUE);
		}else
			MessageBox(_T("GSocket����ʧ�ܣ��������־�ļ���GLog.ini"));
		GetGSocketInfo();
	}
}

void CGSocketDemoDlg::OnBnClickedBnTcpListen()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	DWORD dwListenerId = GTcpSvr_CreateListen((char*)m_strListenIp.GetBuffer(), m_dwListenPort, this);
	if(dwListenerId)
	{
		m_dwListenPort++;
		UpdateData(FALSE);
		GInfo_OnAddListener(dwListenerId);
		CString str;
		str.Format(_T("TCP����(%d)"), GTcpSvr_GetListenerCount());
		m_bnTcpListen.SetWindowText(str);
	}else
		MessageBox(_T("��������ʧ�ܣ��������־�ļ���GLog.ini"));
}

void CGSocketDemoDlg::OnConnectSvr(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
	GInfo_OnConnect(dwClientContext);
	if(dwBytes)
		OnReceiveSvr(dwClientContext, pBuf, dwBytes);
}

void CGSocketDemoDlg::OnDisconnectSvr(const DWORD dwClientContext)
{
	PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->sendLogin = FALSE;
	GInfo_OnDisconnect(dwClientContext);
}

void CGSocketDemoDlg::OnReceiveSvr(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
	if(!((CGSocketDemoDlg*)GHndDat_GetOwner((DWORD)GHndDat_GetOwner(dwClientContext)))->m_bSendSelf)
		return;

	PDATA_INFO pDataInfo = (PDATA_INFO)GHndDat_GetData(dwClientContext);
	if(!pDataInfo)
		return;

// 	if (!ProessAnalyseData(dwClientContext,pBuf,dwBytes))
// 	{
// 
// 	}
// 	
// 	int i = ((CGSocketDemoDlg*)GHndDat_GetOwner((DWORD)GHndDat_GetOwner(dwClientContext)))->m_dwSendLoop;
// 	while(i)
// 		i--;
// 
// 	GCommProt_PostSendBuf(dwClientContext, (char*)pBuf, dwBytes);
	TRACE("OnReceiveSvr\n");
}

void CGSocketDemoDlg::OnReceiveErrorSvr(const DWORD dwClientContext)
{
}

void CGSocketDemoDlg::OnSendedSvr(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
}

void CGSocketDemoDlg::OnSendErrorSvr(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
}

void CGSocketDemoDlg::OnIdleOvertime(const DWORD dwClientContext)
{
}

void CGSocketDemoDlg::OnConnectionOverflow(const DWORD dwClientContext)
{
}
//--------------------------------------------------------------------------------------------------------------
void CGSocketDemoDlg::OnHeartbeat(const DWORD dwClientContext)
{
// 	if (!((CGSocketDemoDlg*)GHndDat_GetOwner(dwClientContext))->m_bShutdownSend)
// 	{
		OnHeartDataClient(((CGSocketDemoDlg*)GHndDat_GetOwner(dwClientContext)),0,dwClientContext);
//	}
}

void CGSocketDemoDlg::OnProfessionalWork(const DWORD dwClientContext)
{
	BOOL result = ((CGSocketDemoDlg*)GHndDat_GetOwner(dwClientContext))->m_bShutdownSend;
	if ((!((CGSocketDemoDlg*)GHndDat_GetOwner(dwClientContext))->m_bShutdownSend))
	{
		//GTcpClt_TraversalClient((DWORD)GHndDat_GetOwner(dwClientContext), 0, GTcpClt_GetClientCount(), (PFN_ON_GSOCK_TRAVERSAL)&OnProfessionalWorkClient);
		OnPostLoginClient(((CGSocketDemoDlg*)GHndDat_GetOwner(dwClientContext)),0,dwClientContext);
	}
}

void CGSocketDemoDlg::OnConnectError(const DWORD dwClientContext)
{
	PDATA_INFO pDataInfo = (PDATA_INFO)GHndDat_GetData(dwClientContext);
	if(!pDataInfo)
		return;
	InterlockedIncrement((volatile LONG*)&pDataInfo->dwDataError);
}

void CGSocketDemoDlg::OnCreateClient(const DWORD dwClientContext)
{
	PDATA_INFO pDataInfo = new DATA_INFO;
	ZeroMemory(pDataInfo, sizeof(DATA_INFO));
	GHndDat_SetData(dwClientContext, pDataInfo);
	GInfo_OnConnect(dwClientContext);

	OnCreateClientInfo(dwClientContext);
}

void CGSocketDemoDlg::OnDestroyClient(const DWORD dwClientContext)
{
	GInfo_OnDisconnect(dwClientContext);
	PDATA_INFO pDataInfo = (PDATA_INFO)GHndDat_GetData(dwClientContext);
	GHndDat_SetData(dwClientContext, NULL);
	delete pDataInfo;

	OnDestroyClientInfo(dwClientContext);
}

DWORD userinfo = 1500000;
void CGSocketDemoDlg::OnCreateClientInfo(const DWORD dwClientContext)
{
	PCLIENT_DATA_INFO pDataInfo = new CLIENT_DATA_INFO;
	ZeroMemory(pDataInfo, sizeof(CLIENT_DATA_INFO));
	InterlockedExchangeAdd((volatile LONG*)&userinfo, 1);
	pDataInfo->dwNo = userinfo;
	pDataInfo->sendLogin = FALSE;
	GHndDat_SetDataInfo(dwClientContext, pDataInfo);
}

void CGSocketDemoDlg::OnDestroyClientInfo(const DWORD dwClientContext)
{
	PCLIENT_DATA_INFO pDataInfo = (PCLIENT_DATA_INFO)GHndDat_GetDataInfo(dwClientContext);
	GHndDat_SetDataInfo(dwClientContext, NULL);
	delete pDataInfo;
}

void CGSocketDemoDlg::OnConnectClt(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
	OnPostLoginClient(((CGSocketDemoDlg*)GHndDat_GetOwner((DWORD)GHndDat_GetOwner(dwClientContext))),0,dwClientContext);
}

void CGSocketDemoDlg::OnDisconnectClt(const DWORD dwClientContext)
{
	PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->sendLogin = FALSE;
	TRACE("Server Disconnect \n");	
}

void CGSocketDemoDlg::OnReceiveClt(const DWORD dwClientContext, const char* pRecvData, const DWORD nRecvLen)
{
	TRACE(_T("[THREAD:%d] �������Ϊ %s %d \n"),GetCurrentThreadId(),pRecvData,nRecvLen);
	CGSocketDemoDlg* temp_this = ((CGSocketDemoDlg*)GHndDat_GetOwner(dwClientContext));
	TRACE("%d",temp_this);
	DATA_RECVBUFFER temprecvbufferinfo;
	ZeroMemory(&temprecvbufferinfo,sizeof(DATA_RECVBUFFER));
	DWORD dwThreadId = GetCurrentThreadId();
	if (temp_this->m_HashmapDataRecvBuffer.IsEmpty() || !temp_this->m_HashmapDataRecvBuffer.FindItem(dwThreadId/*GetCurrentThreadId()*/,temprecvbufferinfo))
	{
		temprecvbufferinfo.nBufLenCount = 0;
		temp_this->m_HashmapDataRecvBuffer.AddItem(dwThreadId,temprecvbufferinfo);
	}

	if(temprecvbufferinfo.nBufLenCount + nRecvLen > MAX_RECVICE_BUFFER)
	{
		memset(temprecvbufferinfo.RecvBuf,0,MAX_RECVICE_BUFFER);
		TRACE(_T("[THREAD:%d][ID:%d] ERROR �������ݰ�����Ϊ %s %d \n"),GetCurrentThreadId(),dwClientContext,temprecvbufferinfo.RecvBuf,temprecvbufferinfo.nBufLenCount);		
		return /*FALSE*/;
	}

	memcpy(temprecvbufferinfo.RecvBuf+temprecvbufferinfo.nBufLenCount,pRecvData,nRecvLen);
	temprecvbufferinfo.nBufLenCount = temprecvbufferinfo.nBufLenCount + nRecvLen;
	int nSpaceLen = temprecvbufferinfo.nBufLenCount;
	char *pCurPoint = temprecvbufferinfo.RecvBuf;
	TRACE(_T("[THREAD:%d] ��Ҫ��������ݰ���Ϊ%d %s %d \n"),GetCurrentThreadId(),dwClientContext,temprecvbufferinfo.RecvBuf,temprecvbufferinfo.nBufLenCount);

	int nPackageLen = 0;
// 	char *m_strHead =  Cmdtypelist[0].HeadData;
// 	unsigned int temp_nHeadDataLen = strlen(m_strHead);
// 	char *m_strTail =  Cmdtypelist[0].TailData;
// 	unsigned int temp_nTailDataLen = strlen(m_strTail);
	char *m_strHead =  "*HITOM"/*m_Cmdtypelist[m_nAppID].HeadData*/;
	unsigned int temp_nHeadDataLen = 6/*strlen(m_strHead)*/;
	char *m_strTail =  "#"/*m_Cmdtypelist[m_nAppID].TailData*/;
	unsigned int temp_nTailDataLen = 1/*strlen(m_strTail)*/;
	char *ptempCurPoint = pCurPoint;
	while(nSpaceLen >= MAIN_DATA_PACKAGE_LEN)
	{
		if(strncmp(pCurPoint,m_strHead,temp_nHeadDataLen) != 0) //����ͷ
		{
			TRACE(_T("��ͷ��:len:%d,recv:%s\n"),nSpaceLen,pRecvData);
			nSpaceLen = 0;

			break;
		}
		//���Ұ�β
		int nPoint = 0;
		for(int i = 0 ; i < nSpaceLen; i++ )
		{
			if(strncmp(ptempCurPoint,m_strTail,temp_nTailDataLen) != 0) //����ͷ
			{
				++ptempCurPoint;
			}
			else
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
				pCurPoint[nPackageLen-1] = NULL;
				temp_this->ProessAnalyseData(dwClientContext,pCurPoint+temp_nHeadDataLen,nPackageLen-temp_nHeadDataLen-temp_nTailDataLen);
				TRACE(_T("[THREAD:%d] ��������Ϊ%d %s %d \n"),GetCurrentThreadId(),dwClientContext,pCurPoint+temp_nHeadDataLen,nPackageLen-temp_nHeadDataLen-temp_nTailDataLen);
			}
			else
			{
				TRACE(_T("�����ȳ�����������ֵ:len:%d,recv:%s\n"),nSpaceLen,pRecvData);
			}
			//ʣ�µ����ݼ����ְ�
			pCurPoint = pCurPoint + nPackageLen;
			nSpaceLen = nSpaceLen - nPackageLen;
		}
		else
		{
			if( nSpaceLen > MAX_DATA_PACKAGE_LEN )
			{
				TRACE(_T("���������ݰ�:len:%d,recv:%s\n"),nSpaceLen,pRecvData);
				nSpaceLen = 0;
			}
			break;
		}

	}//endwhile

	//�Ƿ���û�д��������
	if( nSpaceLen > 0 )
	{
		temprecvbufferinfo.nBufLenCount = nSpaceLen;
		if( pCurPoint != temprecvbufferinfo.RecvBuf )
		{
			TRACE(_T("[THREAD:%d] û�д�������ݰ���Ϊ%d %s %d \n"),GetCurrentThreadId(),dwClientContext,temprecvbufferinfo.RecvBuf,temprecvbufferinfo.nBufLenCount);
			memcpy(temprecvbufferinfo.RecvBuf,pCurPoint,temprecvbufferinfo.nBufLenCount);
			memset(temprecvbufferinfo.RecvBuf+temprecvbufferinfo.nBufLenCount,0,MAX_RECVICE_BUFFER-temprecvbufferinfo.nBufLenCount);
		}
	}
	else
	{
		temprecvbufferinfo.nBufLenCount = 0;
		memset(temprecvbufferinfo.RecvBuf,0,MAX_RECVICE_BUFFER);
	}
	temp_this->m_HashmapDataRecvBuffer.AddItem(dwThreadId,temprecvbufferinfo);
	return /*TRUE*/;



// 	PDATA_INFO pDataInfo = (PDATA_INFO)GHndDat_GetData(dwClientContext);
// 	if(!pDataInfo)
// 		return;
// 	BOOL bIsError = FALSE;
// 
// 	InterlockedExchangeAdd((volatile LONG*)&pDataInfo->dwRecvBytes, dwBytes);
// 	InterlockedExchangeAdd((volatile LONG*)&DataInfo.dwRecvBytes, dwBytes);

// 	if((PDATA_HEAD(pBuf)->dwSender != (DWORD)pDataInfo) || (PDATA_HEAD(pBuf)->dwSize != dwBytes))
// 	{
// 		InterlockedIncrement((volatile LONG*)&pDataInfo->dwDataError);
// 		bIsError = TRUE;
// 	}else
// 	{
// 		DWORD i;
// 		for(i = sizeof(DATA_HEAD); i < dwBytes; i++)
// 			if(*(char *)(pBuf + i) != (char)i)
// 			{
// 				InterlockedIncrement((volatile LONG*)&pDataInfo->dwDataError);
// 				bIsError = TRUE;
// 				break;
// 			}
// 	}
	
	
// 	if (!ProessAnalyseData(dwClientContext,pBuf,dwBytes))
// 	{
// 		bIsError = TRUE;
// 	}

// 	if(!bIsError)
// 	{
// 		pDataInfo->dwLong = GetTickCount() - ((PDATA_HEAD)pBuf)->dwTickCount;
// 		
// 		if(pDataInfo->dwLong)
// 			pDataInfo->fSpeed = float(PDATA_HEAD(pBuf)->dwSize) * 2 / pDataInfo->dwLong;
// 		else
// 			pDataInfo->fSpeed = float(PDATA_HEAD(pBuf)->dwSize) * 2;
// 	}
}


BOOL CGSocketDemoDlg::ProessAnalyseData(const DWORD dwClientContext, const char* RecvData, const DWORD nRecvLen)
{
	char tempBuf[MAX_DATA_PACKAGE_LEN] = {0};
	strncpy(tempBuf,RecvData,nRecvLen);

	if(tempBuf == NULL)
	{
		return FALSE;
	}

	if(nRecvLen < PACKAGE_PUB_LEN)
	{
		return FALSE;
	}

	int nTempLen = 0;
	int nCmdType = 0;
	sscanf(tempBuf, "%4x%2x",&nTempLen,&nCmdType);	//|-4-|datalen |-2-|cmdtype

	if(nTempLen != nRecvLen -6)
	{
		return FALSE;
	}

	ProessAnalyseData(dwClientContext,tempBuf+6,nTempLen,nCmdType);

	return TRUE;
}

BOOL CGSocketDemoDlg::ProessAnalyseData(const DWORD dwClientContext, const char* RecvData, const DWORD dwRecvLen, int nCmdType)
{

	#define H6_TYPE_LOGIN 0x00

	if(NULL == RecvData)
	{
		return FALSE;
	}

	char tempBuf[MAX_DATA_PACKAGE_LEN] = {0};
	strncpy(tempBuf,RecvData,dwRecvLen);
	char *pBuf = tempBuf;

	char *pData[2] = {0};
	strtok(pBuf,",");
	pData[0] = pBuf;
	pData[1] = strtok(NULL,",");

	char tempSendBuf[MAX_DATA_PACKAGE_LEN] = {0};

	switch(nCmdType)
	{
	case H6_TYPE_LOGIN:
		PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->sendLogin = TRUE;
		break;
	default:
		break;
	}

	return TRUE;
}


void CGSocketDemoDlg::OnReceiveErrorClt(const DWORD dwClientContext)
{
	PDATA_INFO pDataInfo = (PDATA_INFO)GHndDat_GetData(dwClientContext);
	if(!pDataInfo)
		return;
	InterlockedIncrement((volatile LONG*)&pDataInfo->dwDataError);
	PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->sendLogin = FALSE;
}

void CGSocketDemoDlg::OnSendedClt(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
	PDATA_INFO pDataInfo = (PDATA_INFO)GHndDat_GetData(dwClientContext);
	if(!pDataInfo)
		return;
	InterlockedExchangeAdd((volatile LONG*)&pDataInfo->dwSendBytes, dwBytes);
	InterlockedExchangeAdd((volatile LONG*)&DataInfo.dwSendBytes, dwBytes);
}

void CGSocketDemoDlg::OnSendErrorClt(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes)
{
	PDATA_INFO pDataInfo = (PDATA_INFO)GHndDat_GetData(dwClientContext);
	if(!pDataInfo)
		return;
	InterlockedIncrement((volatile LONG*)&pDataInfo->dwDataError);
	PCLIENT_DATA_INFO(GHndDat_GetDataInfo(dwClientContext))->sendLogin = FALSE;
	TRACE("[OnSendErrorClt] PBUF:%s BYTE:%d\n",pBuf,dwBytes);
}

void CGSocketDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (10001 == nIDEvent)
	{
		if ((!m_bShutdownSend))
		{
			GTcpClt_TraversalClient((DWORD)this, 0, GTcpClt_GetClientCount(), (PFN_ON_GSOCK_TRAVERSAL)&OnPostGPRSDataClient);
		}		
	}

	if(10000 == nIDEvent)
	{
		m_dwSendBytes = DataInfo.dwSendBytes;
		m_dwRecvBytes = DataInfo.dwRecvBytes;
		m_dwByteTotal = m_dwSendBytes + m_dwRecvBytes;
		m_dwLong = GetTickCount() - DataInfo.dwLong;
		m_fBeat = (float)m_dwByteTotal / (float)m_dwLong;
		CString str;

		str.Format(_T("%d"), m_dwSendBytes);
		m_edtSendBytes.SetWindowText(str);
		str.Format(_T("%d"), m_dwRecvBytes);
		m_edtRecvBytes.SetWindowText(str);
		str.Format(_T("%d"), m_dwByteTotal);
		m_edtByteTotal.SetWindowText(str);
		str.Format(_T("%d"), m_dwLong);
		m_edtLong.SetWindowText(str);
		str.Format(_T("%f"), m_fBeat);
		m_edtBeat.SetWindowText(str);

		LV_ITEM lvitem;
		int i, nEnd;

		i = m_lstInfo.GetTopIndex();
		nEnd = i + 20;
		if(nEnd > m_lstInfo.GetItemCount())
			nEnd = m_lstInfo.GetItemCount();
		lvitem.state = 0;
		lvitem.stateMask = 0;
		lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
		while(i < nEnd)
		{
			lvitem.iItem = i;
			lvitem.iSubItem = 2;
			DWORD dwClientContext = (DWORD)m_lstInfo.GetItemData(i);
			switch(GHndDat_GetState(dwClientContext))
			{
// 				case GHND_STATE_LOGIN:
// 				{
// 					lvitem.pszText = _T("��½");
// 					break;
// 				}
				case GHND_STATE_CONNECTED:
				{
					lvitem.pszText = _T("����");
					break;
				}
				case GHND_STATE_CONNECTING:
				{
					lvitem.pszText = _T("������");
					break;
				}
				default:
					lvitem.pszText = _T("����");
			}
			m_lstInfo.SetItem(&lvitem);

			PDATA_INFO pDataInfo = (PDATA_INFO)GHndDat_GetData(dwClientContext);

			lvitem.iSubItem = 4;
			str.Format(_T("%d"), pDataInfo->dwSendBytes);
			lvitem.pszText = (LPTSTR)(LPCTSTR)str;
			m_lstInfo.SetItem(&lvitem);

			lvitem.iSubItem = 5;
			str.Format(_T("%d"), pDataInfo->dwRecvBytes);
			lvitem.pszText = (LPTSTR)(LPCTSTR)str;
			m_lstInfo.SetItem(&lvitem);

			lvitem.iSubItem = 6;
			str.Format(_T("%d"), pDataInfo->dwRecvBytes + pDataInfo->dwSendBytes);
			lvitem.pszText = (LPTSTR)(LPCTSTR)str;
			m_lstInfo.SetItem(&lvitem);

			lvitem.iSubItem = 7;
			str.Format(_T("%d"), pDataInfo->dwLong);
			lvitem.pszText = (LPTSTR)(LPCTSTR)str;
			m_lstInfo.SetItem(&lvitem);

			lvitem.iSubItem = 8;
			str.Format(_T("%f"), pDataInfo->fSpeed);
			lvitem.pszText = (LPTSTR)(LPCTSTR)str;

			m_lstInfo.SetItem(&lvitem);

			lvitem.iSubItem = 9;
			str.Format(_T("%d"), pDataInfo->dwDataError);
			lvitem.pszText = (LPTSTR)(LPCTSTR)str;
			m_lstInfo.SetItem(&lvitem);
			i++;
		}

		/*if(m_bRandoDisconnect)
		{
			int nIndex = m_lstInfo.GetItemCount();
			if(nIndex)
			{
				nIndex = Random(nIndex * 2);
				if(nIndex < m_lstInfo.GetItemCount())
				{
					GTcpClt_DisconnectClient((DWORD)m_lstInfo.GetItemData(nIndex));
				}
			}
		}*/

		return;
	}
	CDialog::OnTimer(nIDEvent);
}

void CGSocketDemoDlg::DeleteClient(DWORD dwClientContext)
{
	int i, nIndex = -1;

	for(i = 0; i <= m_lstInfo.GetItemCount() - 1; i++)
	{
		if((DWORD)m_lstInfo.GetItemData(i) == dwClientContext)
		{
			nIndex = i;
			m_lstInfo.DeleteItem(nIndex);
			break;
		}
	}

	if(-1 == nIndex)
		return;

	LV_ITEM lvitem;
	CString str;

	lvitem.state = 0;
	lvitem.stateMask = 0;
	lvitem.mask = LVIF_TEXT | LVIF_IMAGE;

	for(i = nIndex; i <= m_lstInfo.GetItemCount() - 1; i++)
	{
		lvitem.iItem = i;
		lvitem.iSubItem = 0;
		str.Format(_T("%d"), i + 1);
		lvitem.pszText = (LPTSTR)(LPCTSTR)str;
		m_lstInfo.SetItem(&lvitem);
	}
	m_dwConnectionCount = m_lstInfo.GetItemCount();
	UpdateData(FALSE);
}

void CGSocketDemoDlg::AddClient(DWORD dwClientContext)
{
	LV_ITEM lvitem;
	CString str;
	char szIp[30];
	int nIndex;

	nIndex = m_lstInfo.GetItemCount();
	if(0 > nIndex)
		nIndex = 0;
	
	lvitem.state = 0;
	lvitem.stateMask = 0;

	lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
	lvitem.iItem = nIndex;
	lvitem.iSubItem = 0;
	str.Format(_T("%d"), nIndex + 1);
	lvitem.pszText = (LPTSTR)(LPCTSTR)str;
	m_lstInfo.InsertItem(&lvitem);

	lvitem.iItem = nIndex;
	lvitem.iSubItem = 1;
	GHND_TYPE HndType = GHndDat_GetType(dwClientContext);
	switch(HndType)
	{
		case GHND_TYPE_TCP_SVR_CLIENT:
			lvitem.pszText = _T("TCP����");
		break;
		case GHND_TYPE_TCP_CLT_CLIENT:
			lvitem.pszText = _T("TCP�ͻ�");
		break;
		case GHND_TYPE_UDP_SVR_CLIENT:
			lvitem.pszText = _T("UDP����");
		break;
		default:
			lvitem.pszText = _T("UDP�ͻ�");
	}
	m_lstInfo.SetItem(&lvitem);

	lvitem.iItem = nIndex;
	lvitem.iSubItem = 2;
	switch(GHndDat_GetState(dwClientContext))
	{
// 		case GHND_STATE_LOGIN:
// 		{
// 			lvitem.pszText = _T("��½");
// 			break;
// 		}
		case GHND_STATE_CONNECTED:
		{
			lvitem.pszText = _T("����");
			break;
		}
		case GHND_STATE_CONNECTING:
		{
			lvitem.pszText = _T("������");
			break;
		}
		default:
			lvitem.pszText = _T("����");
	}
	m_lstInfo.SetItem(&lvitem);

	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = nIndex;
	lvitem.iSubItem = 3;
	GSock_AddrToIp(szIp, 28, GHndDat_GetAddr(dwClientContext));
	str.Format(_T("%s:%d"), szIp, GHndDat_GetPort(dwClientContext));
	lvitem.pszText = (LPTSTR)(LPCTSTR)str;
	m_lstInfo.SetItem(&lvitem);

	lvitem.iItem = nIndex;
	lvitem.iSubItem = 4;
	lvitem.pszText = _T("0");
	m_lstInfo.SetItem(&lvitem);

	lvitem.iItem = nIndex;
	lvitem.iSubItem = 5;
	lvitem.pszText = _T("0");
	m_lstInfo.SetItem(&lvitem);

	lvitem.iItem = nIndex;
	lvitem.iSubItem = 6;
	lvitem.pszText = _T("0");
	m_lstInfo.SetItem(&lvitem);

	lvitem.iItem = nIndex;
	lvitem.iSubItem = 7;
	lvitem.pszText = _T("0");
	m_lstInfo.SetItem(&lvitem);

	lvitem.iItem = nIndex;
	lvitem.iSubItem = 8;
	lvitem.pszText = _T("0");
	m_lstInfo.SetItem(&lvitem);

	lvitem.iItem = nIndex;
	lvitem.iSubItem = 9;
	lvitem.pszText = _T("0");
	m_lstInfo.SetItem(&lvitem);

	m_lstInfo.SetItemData(nIndex, (DWORD_PTR)dwClientContext);

	m_dwConnectionCount = m_lstInfo.GetItemCount();
	UpdateData(FALSE);
}

void CGSocketDemoDlg::OnBnClickedBnTcpConnect()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DWORD i, dwClient;

	UpdateData(TRUE);
	if(!GTcpClt_GetClientCount())
	{
		DataInfo.dwLong = GetTickCount();
		//SetTimer(10001, m_dwSendSpace, NULL);
		DWORD dwThreadId;
		m_bShutdownSend = FALSE;
		//CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ThreadSend, this, 0, &dwThreadId)); wangzh
	}
	if(m_dwPackSize > GIoDat_GetGBufSize() - 2 * (sizeof(SOCKADDR_IN) + 16))
		m_dwPackSize = GIoDat_GetGBufSize() - 2 * (sizeof(SOCKADDR_IN) + 16);

	UpdateData(TRUE);
	for(i = 0; i < m_dwConnectNumber; i++)
	{
		dwClient = GTcpClt_CreateClient((char *)m_strRemoteIp.GetBuffer(), m_dwRemotePort, (char *)m_strLocalIp.GetBuffer(), this);
		if(dwClient)
			AddClient(dwClient);
		else
			break;
	}
}

void CGSocketDemoDlg::OnBnClickedButton6()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ZeroMemory(&DataInfo, sizeof(DATA_INFO));
	DataInfo.dwLong = GetTickCount();
}

void CGSocketDemoDlg::OnEnChangeGedit18()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ�������������
	// ���͸�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	if(m_dwPackSize > GIoDat_GetGBufSize() - 2 * (sizeof(SOCKADDR_IN) + 16))
		m_dwPackSize = GIoDat_GetGBufSize() - 2 * (sizeof(SOCKADDR_IN) + 16);
}

void CGSocketDemoDlg::OnBnClickedButton3()
{
	if(0 < m_lstInfo.GetItemCount())
	{
		GTcpClt_DisconnectClient((DWORD)m_lstInfo.GetItemData(0));
	}
}

//-----------------------------------------------------------------------------------------------------------------------

void CGSocketDemoDlg::OnBnClickedBnTcpStates()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if ((!m_bShutdownSend))
	{
		GTcpClt_TraversalClient((DWORD)this, 0, GTcpClt_GetClientCount(), (PFN_ON_GSOCK_TRAVERSAL)&OnPostStatesClient);
	}
}

void CGSocketDemoDlg::OnBnClickedBnTcpLogin()
{
	if ((!m_bShutdownSend))
	{
		GTcpClt_TraversalClient((DWORD)this, 0, GTcpClt_GetClientCount(), (PFN_ON_GSOCK_TRAVERSAL)&OnPostLoginClient);
	}
}

void CGSocketDemoDlg::OnBnClickedBnClientAlarm()
{
	if ((!m_bShutdownSend))
	{
		GTcpClt_TraversalClient((DWORD)this, 0, GTcpClt_GetClientCount(), (PFN_ON_GSOCK_TRAVERSAL)&OnPostAlemaltClient);
	}
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}
