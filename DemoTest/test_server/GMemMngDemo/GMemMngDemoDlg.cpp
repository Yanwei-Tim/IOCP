// GMemMngDemoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "GMemMngDemo.h"
#include "GMemMngDemoDlg.h"
#include "..\GMemory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

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


// CGMemMngDemoDlg �Ի���

#define	SIZE_GMEMMNG							(50 * 1024 * 1024)
#define	SIZE_GMEMMNG_PAGE						256
#define	MIN_SIZE_GMEMMNG_PAGE					128
#define	MIN_SIZE_GMEMMNG						(64 * 1024)
#define	COUNT_GMEMMNG_PAGE						((DWORD)(SIZE_GMEMMNG / SIZE_GMEMMNG_PAGE))

#define	COUNT_GMEMMNG_FREE_TYPE					8
#define	MAX_COUNT_GMEMMNG_FREE_PAGE				128

typedef struct _GMEM_BLOCK
{
_GMEM_BLOCK*		pmbNext;
_GMEM_BLOCK*		pmbPrior;
_GMEM_BLOCK*		pmbFreeNext;
_GMEM_BLOCK*		pmbFreePrior;
BOOL				bIsFree;
void*				pAddr;
DWORD				dwSize;
}GMEM_BLOCK, *PGMEM_BLOCK;

extern DWORD						dwGMemPhyPageSize;
extern ULONG_PTR					dwGMemUsedPhyPageNumber;
extern DWORD						dwGMemUsedPhyBytes;
extern PULONG_PTR					pulGMemUsedPhyPageArray;

extern CRITICAL_SECTION				csGMemCritical;
extern BOOL							bGMemIsActive;
extern DWORD						dwGMemPageSize;
extern DWORD						dwGMemPageNumber;
extern DWORD						dwGMemTotalBytes;
extern DWORD						dwGMemUsedBytes;
extern DWORD						dwGMemSysUsedBytes;

extern void*						pGMemAddrHead;
extern void*						pGMemAddrTail;
extern PGMEM_BLOCK					pmbGMemNodePool;
extern PGMEM_BLOCK					pmbGMemList;
extern PGMEM_BLOCK					*pmbGMemUsedFlag;
extern PGMEM_BLOCK					pmbGMemFreeList;

int Random(int nNumber) 
{ 
	return (int)(nNumber/(float)RAND_MAX * rand()); 
}

#define	COUNT_THREAD_ALLOC					10000

typedef struct _THREAD_INFO
{
DWORD			dwThreadID;
BOOL			bIsEnd;
BOOL			bTheEnd;
DWORD			dwOperCount;
DWORD			dwOperLong;
int				nSizeOfPix;
void*			pMems[COUNT_THREAD_ALLOC];
}THREAD_INFO, *PTHREAD_INFO;

DWORD WINAPI GMemMngThread(PTHREAD_INFO ptiThreadInfo)
{
	DWORD dwSize;
	DWORD dwTickCount;
	DWORD dwOperCount;
	int nAllocCount;

	while(!ptiThreadInfo->bIsEnd)
	{
		dwTickCount = GetTickCount();
		dwOperCount = 0;
		nAllocCount = 0;
Loop:
		dwOperCount++;
		//dwSize = ptiThreadInfo->nSizeOfPix + ptiThreadInfo->nSizeOfPix * Random(10);
		//dwSize = dwGMemPageSize + dwGMemPageSize * Random(3);
		dwSize = Random(ptiThreadInfo->nSizeOfPix * 10);
		ptiThreadInfo->pMems[nAllocCount] = GMem_Alloc(dwSize);
		if((!ptiThreadInfo->pMems[nAllocCount]) || (COUNT_THREAD_ALLOC - 1 <= nAllocCount))
		{
			ptiThreadInfo->dwOperCount = dwOperCount;
			ptiThreadInfo->dwOperLong = GetTickCount() - dwTickCount;
			if(dwOperCount <= ptiThreadInfo->dwOperLong)
				ptiThreadInfo->dwOperLong -= dwOperCount;
			else
				ptiThreadInfo->dwOperLong = 0;

			if(ptiThreadInfo->pMems[nAllocCount])
				nAllocCount++;
			while(nAllocCount)
			{
				nAllocCount--;
				GMem_Free(ptiThreadInfo->pMems[nAllocCount]);
			}			
			Sleep(1);
		}else
		if(!ptiThreadInfo->bIsEnd)
		{
			nAllocCount++;
			Sleep(1);
			goto Loop;
		}else
		{
			if(ptiThreadInfo->pMems[nAllocCount])
				nAllocCount++;
			while(nAllocCount)
			{
				nAllocCount--;
				GMem_Free(ptiThreadInfo->pMems[nAllocCount]);
			}			
		}
	}
	ptiThreadInfo->bTheEnd = TRUE;
	return(0);
}

CGMemMngDemoDlg::CGMemMngDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGMemMngDemoDlg::IDD, pParent)
	, m_nMemSize(50 * 1024 * 1024)
	, m_nTimer(1000)
	, m_bTimerMade(FALSE)
	, m_nMemCount(0)
	, m_strLong(_T(""))
	, m_nPageSize(0)
	, m_nPageCount(0)
	, m_nSysPageSize(0)
	, m_nPPageCount(0)
	, m_nPMemSize(0)
	, m_nUsedSize(0)
	, m_nPhyUse(0)
	, m_pMemList(NULL)
	, m_bRanFree(FALSE)
	, m_bRanAlloc(FALSE)
	, m_fPhyUsedR(0)
	, m_fMemUsedR(0)
	, m_nOperCount(0)
	, m_fPreOper(0)
	, m_bFreeAll(FALSE)
	, m_nThreadCount(4)
{
	pThreadInfos = NULL;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nSysPageSize = GMem_GetPhyPageSize();
	m_nPhyUse = GMem_GetAvailPhyBytes(NULL);
}

void CGMemMngDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, m_edtMemSize);
	DDX_Control(pDX, IDC_EDIT3, m_edtTimer);
	DDX_Text(pDX, IDC_EDIT2, m_nMemSize);
	DDX_Text(pDX, IDC_EDIT3, m_nTimer);
	DDX_Control(pDX, IDC_CHECK2, m_btnTimerMade);
	DDX_Check(pDX, IDC_CHECK2, m_bTimerMade);
	DDX_Control(pDX, IDC_BUTTON1, m_btnCreate);
	DDX_Control(pDX, IDC_BUTTON2, m_bnMade);
	DDX_Text(pDX, IDC_EDIT4, m_strLong);
	DDX_Text(pDX, IDC_EDIT5, m_nPageSize);
	DDX_Text(pDX, IDC_EDIT9, m_nPageCount);
	DDX_Text(pDX, IDC_EDIT6, m_nSysPageSize);
	DDX_Text(pDX, IDC_EDIT7, m_nPPageCount);
	DDX_Text(pDX, IDC_EDIT8, m_nPMemSize);
	DDX_Text(pDX, IDC_EDIT10, m_nUsedSize);
	DDX_Control(pDX, IDC_BUTTON3, m_bnFreeAll);
	DDX_Control(pDX, IDC_EDIT5, m_edtPageSize);
	DDX_Text(pDX, IDC_EDIT11, m_nPhyUse);
	DDX_Check(pDX, IDC_CHECK3, m_bRanAlloc);
	DDX_Check(pDX, IDC_CHECK4, m_bRanFree);
	DDX_Text(pDX, IDC_EDIT13, m_fPhyUsedR);
	DDX_Text(pDX, IDC_EDIT12, m_fMemUsedR);
	DDX_Control(pDX, IDC_BUTTON4, m_bnAlloc);
	DDX_Control(pDX, IDC_BUTTON5, m_bnFree);
	DDX_Text(pDX, IDC_EDIT15, m_nOperCount);
	DDX_Text(pDX, IDC_EDIT14, m_fPreOper);
	DDX_Check(pDX, IDC_CHECK5, m_bFreeAll);
	DDX_Control(pDX, IDC_GLIST2, m_lstThread);
	DDX_Text(pDX, IDC_EDIT16, m_nThreadCount);
	DDX_Control(pDX, IDC_BUTTON6, m_bnThread);
	DDX_Control(pDX, IDC_EDIT16, m_edtThreadCount);
}

BEGIN_MESSAGE_MAP(CGMemMngDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CGMemMngDemoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CGMemMngDemoDlg::OnBnClickedCancel)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CGMemMngDemoDlg::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_EDIT3, &CGMemMngDemoDlg::OnEnChangeEdit3)
	ON_BN_CLICKED(IDC_CHECK2, &CGMemMngDemoDlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, &CGMemMngDemoDlg::OnBnClickedCheck3)
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BUTTON2, &CGMemMngDemoDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CGMemMngDemoDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_CHECK4, &CGMemMngDemoDlg::OnBnClickedCheck4)
	ON_BN_CLICKED(IDC_BUTTON4, &CGMemMngDemoDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CGMemMngDemoDlg::OnBnClickedButton5)
	ON_EN_CHANGE(IDC_EDIT5, &CGMemMngDemoDlg::OnEnChangeEdit5)
	ON_EN_CHANGE(IDC_EDIT2, &CGMemMngDemoDlg::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_CHECK5, &CGMemMngDemoDlg::OnBnClickedCheck5)
	ON_EN_CHANGE(IDC_EDIT16, &CGMemMngDemoDlg::OnEnChangeEdit16)
	ON_BN_CLICKED(IDC_BUTTON6, &CGMemMngDemoDlg::OnBnClickedButton6)
END_MESSAGE_MAP()


// CGMemMngDemoDlg ��Ϣ�������

void SetListCtrlStyle(CListCtrl* pLstCtrl)
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

BOOL CGMemMngDemoDlg::OnInitDialog()
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
	m_bnMade.EnableWindow(FALSE);
	m_bnFreeAll.EnableWindow(FALSE);
	m_bnFree.EnableWindow(FALSE);
	m_bnAlloc.EnableWindow(FALSE);
	m_bnThread.EnableWindow(FALSE);
	srand((ULONG)(time(NULL)));
	SetListCtrlStyle(&m_lstThread);

	LV_COLUMN col;

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("��");
	col.cx = 25;
	m_lstThread.InsertColumn(0, &col);

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("����");
	col.cx = 50;
	m_lstThread.InsertColumn(1, &col);

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("��ʱ");
	col.cx = 40;
	m_lstThread.InsertColumn(2, &col);

	col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	col.fmt = LVCFMT_LEFT;
	col.iSubItem = 0;
	col.pszText = _T("ƽ��");
	col.cx = 50;
	m_lstThread.InsertColumn(3, &col);

	SetTimer(100, m_nTimer, NULL);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CGMemMngDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CGMemMngDemoDlg::OnPaint()
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
		DrawMemInfo();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CGMemMngDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CGMemMngDemoDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnOK();
}

void CGMemMngDemoDlg::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnCancel();
	if(GMem_IsActive())
		OnBnClickedButton1();
}

void CGMemMngDemoDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(GMem_IsActive())
	{
		if(pThreadInfos)
			OnBnClickedButton6();

		m_bnMade.EnableWindow(FALSE);
		//KillTimer(100);
		m_btnCreate.SetWindowText(_T("����"));
		m_edtPageSize.EnableWindow(TRUE);
		m_edtMemSize.EnableWindow(TRUE);
		m_bnFreeAll.EnableWindow(FALSE);
		m_bnFree.EnableWindow(FALSE);
		m_bnAlloc.EnableWindow(FALSE);
		m_bnThread.EnableWindow(FALSE);
		GMem_Destroy();
		m_nPMemSize = GMem_GetUsedPhyBytes();
		m_nPPageCount = GMem_GetUsedPhyPageNumber();
		m_nUsedSize = GMem_GetUsedBytes();
		DWORD dwTotalPhys;
		m_nPhyUse = GMem_GetAvailPhyBytes(&dwTotalPhys);
		m_fPhyUsedR	= (float)m_nPhyUse / (float)dwTotalPhys * 100;
		UpdateData(FALSE);
	}else
	{
		if(0 == m_edtPageSize.GetWindowTextLength())
		{
			MessageBox(_T("������GMemҳ��С��"));
			return;
		}
		if(0 == m_edtMemSize.GetWindowTextLength())
		{
			MessageBox(_T("������GMem�ڴ��С��"));
			return;
		}
		UpdateData(TRUE);
		m_nPageSize = GMem_SetPageSize(m_nPageSize);
		m_nMemSize = GMem_SetTotalBytes(m_nMemSize);
		GMem_Create();
		m_nPPageCount = GMem_GetUsedPhyPageNumber();
		m_nPMemSize = GMem_GetUsedPhyBytes();
		m_nPageSize = GMem_GetPageSize();
		m_nPageCount = GMem_GetPageNumber();
		m_nMemSize = GMem_GetTotalBytes();
		DWORD dwTotalPhys;
		m_nPhyUse = GMem_GetAvailPhyBytes(&dwTotalPhys);
		m_fPhyUsedR	= (float)m_nPhyUse / (float)dwTotalPhys * 100;
		m_nUsedSize = GMem_GetUsedBytes();
		UpdateData(FALSE);
		if(GMem_IsActive())
		{			
			CRect rect;
			GetClientRect(&rect);
			rect.left += 16;
			rect.right -= 16;
			m_nWidth = rect.right - rect.left;
			nSizeOfPix = (int)(dwGMemTotalBytes / m_nWidth);
			m_fBL = ((float)m_nWidth) / dwGMemTotalBytes;

			if(m_pMemList)
				delete m_pMemList;
			m_pMemList = new ULONG[dwGMemPageNumber];
			m_nMemCount = 0;
			m_bnMade.EnableWindow(TRUE);
			m_bnFreeAll.EnableWindow(TRUE);
			m_bnFree.EnableWindow(TRUE);
			m_bnAlloc.EnableWindow(TRUE);
			m_bnThread.EnableWindow(TRUE);
			m_edtMemSize.EnableWindow(FALSE);
			m_edtPageSize.EnableWindow(FALSE);
			m_btnCreate.SetWindowText(_T("�ݻ�"));
			Invalidate(FALSE);
		}else
			MessageBox(_T("GMemMng����ʧ�ܣ���ȷ�����㹻�ڴ棬�����һ���û�Ȩ�����ú���������Windows���������->������->���ذ�ȫ����->�û�Ȩ��ָ��->�ڴ�������ҳ"), _T("������ʾ"));
	}
}

void CGMemMngDemoDlg::OnEnChangeEdit5()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ�������������
	// ���͸�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if(0 == m_edtPageSize.GetWindowTextLength())
	{
//		MessageBox(_T("������GMemҳ��С��"));
		return;
	}
	UpdateData(TRUE);
}

void CGMemMngDemoDlg::OnEnChangeEdit2()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ�������������
	// ���͸�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if(0 == m_edtMemSize.GetWindowTextLength())
	{
//		MessageBox(_T("������GMem�ڴ��С��"));
		return;
	}
	UpdateData(TRUE);
}

void CGMemMngDemoDlg::OnEnChangeEdit3()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ�������������
	// ���͸�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if(0 == m_edtTimer.GetWindowTextLength())
		return;
	UpdateData(TRUE);
	KillTimer(100);
	SetTimer(100, m_nTimer, NULL);
}

void CGMemMngDemoDlg::OnBnClickedCheck2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
}

void CGMemMngDemoDlg::OnBnClickedCheck3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
}

void CGMemMngDemoDlg::OnBnClickedCheck4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
}

void CGMemMngDemoDlg::OnBnClickedCheck5()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
}

void CGMemMngDemoDlg::OnEnChangeEdit16()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ�������������
	// ���͸�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if(0 == m_edtThreadCount.GetWindowTextLength())
	{
		//MessageBox(_T("������GMem�ڴ��С��"));
		return;
	}
	UpdateData(TRUE);
}

void CGMemMngDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if(GMem_IsActive())
	{
		if(m_bFreeAll)
			OnBnClickedButton3();
		if(m_bTimerMade)
			OnBnClickedButton2();
		if(m_bRanAlloc)
			OnBnClickedButton4();
		if(m_bRanFree)
			OnBnClickedButton5();
	}

	m_nUsedSize = GMem_GetUsedBytes();
	m_fMemUsedR	= (float)m_nUsedSize / (float)m_nMemSize * 100;
	DWORD dwTotalPhys;
	m_nPhyUse = GMem_GetAvailPhyBytes(&dwTotalPhys);
	m_fPhyUsedR	= (float)m_nPhyUse / (float)dwTotalPhys * 100;
	UpdateData(FALSE);

	int i;
	LV_ITEM lvitem;
	CString str;

	if(pThreadInfos)
	{
		for(i = 0; i < m_nThreadCount; i++)
		{
			lvitem.state = 0;
			lvitem.stateMask = 0;

			lvitem.mask = LVIF_TEXT;
			lvitem.iItem = i;
			lvitem.iSubItem = 1;
			str.Format(_T("%d"), PTHREAD_INFO(pThreadInfos)[i].dwOperCount);
			lvitem.pszText = (LPTSTR)(LPCTSTR)str;
			m_lstThread.SetItem(&lvitem);

			lvitem.mask = LVIF_TEXT;
			lvitem.iItem = i;
			lvitem.iSubItem = 2;
			str.Format(_T("%d"), PTHREAD_INFO(pThreadInfos)[i].dwOperLong);
			lvitem.pszText = (LPTSTR)(LPCTSTR)str;
			m_lstThread.SetItem(&lvitem);

			lvitem.mask = LVIF_TEXT;
			lvitem.iItem = i;
			lvitem.iSubItem = 3;
			if(PTHREAD_INFO(pThreadInfos)[i].dwOperCount)
				str.Format(_T("%f"), (float)PTHREAD_INFO(pThreadInfos)[i].dwOperLong / PTHREAD_INFO(pThreadInfos)[i].dwOperCount * 1000);
			else
				str.Format(_T("%f"), 0);
			lvitem.pszText = (LPTSTR)(LPCTSTR)str;
			m_lstThread.SetItem(&lvitem);
		}
		Invalidate(FALSE);
	}

	CDialog::OnTimer(nIDEvent);
}

void CGMemMngDemoDlg::DrawMemInfo(void)
{
	CClientDC dc(this);
	CRect rect;
	GetClientRect(&rect);
	rect.left += 15;
	rect.top += 15;
	rect.bottom = rect.top + 30;
	rect.right -= 15;
	CBrush brushRed(RGB(255, 0, 0)), *brushSave;

	brushSave = dc.SelectObject(&brushRed);
	dc.Rectangle(rect.left, rect.top, rect.right, rect.bottom);
	dc.SelectObject(&brushSave);

	rect.left += 1;
	rect.top += 1;
	rect.bottom -= 1;
	rect.right -= 1;
	int Left = rect.left;
	CBrush brushGreen(RGB(0, 255, 0));
	CPen penGreen(PS_SOLID, 1, RGB(0, 255, 0)), *penSave;
	brushSave = dc.SelectObject(&brushGreen);
	penSave = dc.SelectObject(&penGreen);
	dc.SetDCPenColor(RGB(0, 255, 0));

	if(bGMemIsActive)
	{
		EnterCriticalSection(&csGMemCritical);
		PGMEM_BLOCK pBlock;
		pBlock = pmbGMemFreeList;
		while(pBlock)
		{
			rect.left = 16 + (int)(m_fBL * ((int)pBlock->pAddr - (int)pGMemAddrHead));
			rect.right = rect.left + (int)(m_fBL * pBlock->dwSize) + 1;
			dc.Rectangle(rect.left, rect.top, rect.right, rect.bottom);
			pBlock = pBlock->pmbFreeNext;
		}
		LeaveCriticalSection(&csGMemCritical);
	}

	dc.SelectObject(&brushSave);
	dc.SelectObject(&penSave);
}

void CGMemMngDemoDlg::CheckAddr(void)
{
	PGMEM_BLOCK pBlock;
	pBlock = pmbGMemList;
	while(pBlock->pmbNext)
	{
		if((DWORD)(pBlock->pAddr) + pBlock->dwSize != (DWORD)(pBlock->pmbNext->pAddr))
			return;
		pBlock = pBlock->pmbNext;
	}
	if((DWORD)(pBlock->pAddr) + pBlock->dwSize != (DWORD)pGMemAddrTail)
		pBlock = NULL;
}

void CGMemMngDemoDlg::DeleteMem(int nIndex)
{
	while(nIndex < m_nMemCount - 1)
	{
		m_pMemList[nIndex] = m_pMemList[nIndex + 1];
		nIndex++;
	}
	m_nMemCount--;
}

BOOL CGMemMngDemoDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	return CDialog::OnEraseBkgnd(pDC);
}

void CGMemMngDemoDlg::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	DWORD dwTickCount = GetTickCount();

	DWORD dwSize;

	m_nOperCount = 0;
	while(pmbGMemFreeList)
	{
		dwSize = nSizeOfPix + nSizeOfPix * Random(3);
		//dwSize = dwGMemPageSize + dwGMemPageSize * Random(3);
		m_nOperCount++;
		m_pMemList[m_nMemCount] = (ULONG)GMem_Alloc(dwSize);
		if(m_pMemList[m_nMemCount])
		{
			m_nMemCount++;
			CheckAddr();
		}else
			break;
	}

	int i;
	for(i = 0; i < m_nMemCount / 2; i++)
	{
		OnBnClickedButton5();
		m_nOperCount++;
	}

	dwTickCount = GetTickCount() - dwTickCount;
	m_strLong.Format(_T("%d"), dwTickCount);
	m_fPreOper = (float)dwTickCount / m_nOperCount * 1000;

	Invalidate(FALSE);
	UpdateData(FALSE);
}

void CGMemMngDemoDlg::OnBnClickedButton3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int i;

	DWORD dwTickCount = GetTickCount();
	for(i = 0; i < m_nMemCount; i++)
		GMem_Free((void *)(m_pMemList[i]));
	dwTickCount = GetTickCount() - dwTickCount;
	m_strLong.Format(_T("%d"), dwTickCount);
	m_fPreOper = (float)dwTickCount / m_nMemCount * 1000;
	m_nMemCount = 0;
	Invalidate(FALSE);
}
void CGMemMngDemoDlg::OnBnClickedButton4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DWORD dwSize;

	dwSize = nSizeOfPix + nSizeOfPix * Random(10);
	m_pMemList[m_nMemCount] = (ULONG)GMem_Alloc(dwSize);
	if(m_pMemList[m_nMemCount])
	{
		m_nMemCount++;
		Invalidate(FALSE);
		CheckAddr();
	}
}

void CGMemMngDemoDlg::OnBnClickedButton5()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(m_nMemCount)
	{
		int nIndex;

		nIndex = Random(m_nMemCount);
		GMem_Free((void *)(m_pMemList[nIndex]));
		DeleteMem(nIndex);
		Invalidate(FALSE);
		CheckAddr();
	}
}


void CGMemMngDemoDlg::OnBnClickedButton6()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int i;

	if(pThreadInfos)
	{
		for(i = 0; i < m_nThreadCount; i++)
			PTHREAD_INFO(pThreadInfos)[i].bIsEnd = TRUE;
		for(i = 0; i < m_nThreadCount; i++)
			while(!PTHREAD_INFO(pThreadInfos)[i].bTheEnd)
				Sleep(100);
		m_lstThread.DeleteAllItems();
		GMem_Free(pThreadInfos);
		pThreadInfos = NULL;
		m_edtThreadCount.EnableWindow(TRUE);
		m_bnThread.SetWindowText(_T("�Ӷ���Դ"));
	}else
	{
		pThreadInfos = GMem_Alloc(sizeof(THREAD_INFO) * m_nThreadCount);
		if(!pThreadInfos)
		{
			MessageBox(_T("GMem�ڴ治�㣬�޷�������Դ�Ӷᣬ������ȫ���ͷš���"));
			return;
		}
		m_bnThread.SetWindowText(_T("ֹͣ�Ӷ�"));
		m_edtThreadCount.EnableWindow(FALSE);		
		ZeroMemory(pThreadInfos, sizeof(THREAD_INFO) * m_nThreadCount);

		LV_ITEM lvitem;
		CString str;
		for(i = 0; i < m_nThreadCount; i++)
		{		
			lvitem.state = 0;
			lvitem.stateMask = 0;

			lvitem.mask = LVIF_TEXT;
			lvitem.iItem = i;
			lvitem.iSubItem = 0;
			str.Format(_T("%d"), i);
			lvitem.pszText = (LPTSTR)(LPCTSTR)str;
			m_lstThread.InsertItem(&lvitem);
			PTHREAD_INFO(pThreadInfos)[i].nSizeOfPix = nSizeOfPix;

			CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&GMemMngThread, &(PTHREAD_INFO(pThreadInfos)[i]), 0, &(PTHREAD_INFO(pThreadInfos)[i].dwThreadID)));
		}
	}
}
