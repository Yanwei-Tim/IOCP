// GSocketDemoDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "..\HashmapManager.h"

const int MAX_DATA_PACKAGE_LEN = 256;  //GIS���ط����İ�����󳤶�
const int MAIN_DATA_PACKAGE_LEN =  8;  //GIS���ط����İ�����С����

const int PACKAGE_HEAD_DATA_STX = 0x02; //0x02 ;  //��ͷ
const int PACKAGE_TAIL_DATA_ETX = 0x03; //0x03 ;  //��β
const int MAX_RECVICE_BUFFER = 1024;
const int PACKAGE_PUB_LEN = 6;

#define MAX_RECVICE_BUFFER          /*1024+256*/4096
typedef struct _DATA_RECVBUFFER
{
	char	RecvBuf[MAX_RECVICE_BUFFER];
	int		nBufLenCount;
}DATA_RECVBUFFER,*PDATA_RECVBUFFER;

// CGSocketDemoDlg �Ի���
class CGSocketDemoDlg : public CDialog
{
// ����
public:
	CGSocketDemoDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_GSOCKETDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_edtGMemPageSize;
	DWORD m_dwGMemPageSize;
	CEdit m_edtGMemTotal;
	DWORD m_dwGMemTotal;
	CEdit m_edtGBufSize;
	DWORD m_dwGBufSize;
	CEdit m_edtIoDataTotal;
	DWORD m_dwIoDataTotal;
	CEdit m_edtHndDataTotal;
	DWORD m_dwHndDataTotal;
	CEdit m_edtPostRecvNumber;
	DWORD m_dwPostRecvNumber;
	CEdit m_edtSocketBufSize;
	int m_nSocketBufSize;
	CEdit m_edtWorkerThreadNumber;
	DWORD m_dwWorkerThreadNumber;
	CEdit m_edtConcurrentThreadNumber;
	DWORD m_dwConcurrentThreadNumber;
	CEdit m_edtAcceptOvertime;
	DWORD m_dwAcceptOvertime;
	CEdit m_edtIdleOvertime;
	DWORD m_dwIdleOvertime;
	CEdit m_edtMaxConnection;
	DWORD m_dwMaxConnection;
	CEdit m_edtPostAcceptNumber;
	DWORD m_dwPostAcceptNumber;
	CEdit m_edtSendLoop;
	DWORD m_dwSendLoop;
	CEdit m_edtListenIp;
	CString m_strListenIp;
	CEdit m_edtListenPort;
	DWORD m_dwListenPort;
	CEdit m_edtSendSpace;
	DWORD m_dwSendSpace;
	CEdit m_edtPackSize;
	DWORD m_dwPackSize;
	CEdit m_edtHeartbeat;
	DWORD m_dwHeartbeat;
	CEdit m_edtConnectNumber;
	DWORD m_dwConnectNumber;
	CEdit m_edtLocalIp;
	CString m_strLocalIp;
	CEdit m_edtLocalPort;
	DWORD m_dwLocalPort;
	CEdit m_edtRemoteIp;
	CString m_strRemoteIp;
	CEdit m_edtRemotePort;
	DWORD m_dwRemotePort;
	CEdit m_edtSendBytes;
	DWORD m_dwSendBytes;
	CEdit m_edtRecvBytes;
	DWORD m_dwRecvBytes;
	CEdit m_edtByteTotal;
	DWORD m_dwByteTotal;
	CEdit m_edtLong;
	DWORD m_dwLong;
	CEdit m_edtBeat;
	float m_fBeat;
	CButton m_bnCloseMtu;
	BOOL m_bCloseMtu;
	CButton m_bnZeroAccept;
	BOOL m_bZeroAccept;
	CButton m_bnZeroRecv;
	BOOL m_bZeroRecv;
	CButton m_bnSendSelf;
	BOOL m_bSendSelf;
	CButton m_bnRandoDisconnect;
	BOOL m_bRandoDisconnect;
	CButton m_bnTcpListen;
	CButton m_bnUdpListen;
	CButton m_bnTcpConnect;
	CButton m_bnUdpConnect;
	CButton m_bnService;

#ifdef WANGZH //2013-5-6
	CButton m_bnTcpLogin;
#endif

	BOOL m_bShutdownSend;

	void EnabledCtrls(BOOL bIsActive);
	void GetGSocketInfo(void);
	void AddClient(DWORD dwClientContext);
	void DeleteClient(DWORD dwClientContext);

	static void OnConnectSvr(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes);
	static void OnDisconnectSvr(const DWORD dwClientContext);
	static void OnReceiveSvr(const DWORD dwClientContext, const char* pBuf, const DWORD nBytes);
	static void OnReceiveErrorSvr(const DWORD dwClientContext);
	static void OnConnectClt(const DWORD dwClientContext, const char* pBuf, const DWORD dwBytes);
	static void OnDisconnectClt(const DWORD dwClientContext);
	static void OnReceiveClt(const DWORD dwClientContext, const char* pBuf, const DWORD nBytes);
	static void OnReceiveErrorClt(const DWORD dwClientContext);
	static void OnSendedSvr(const DWORD dwClientContext, const char* pBuf, const DWORD nBytes);
	static void OnSendErrorSvr(const DWORD dwClientContext, const char* pBuf, const DWORD nBytes);
	static void OnSendedClt(const DWORD dwClientContext, const char* pBuf, const DWORD nBytes);
	static void OnSendErrorClt(const DWORD dwClientContext, const char* pBuf, const DWORD nBytes);
	static void OnIdleOvertime(const DWORD dwClientContext);
	static void OnConnectionOverflow(const DWORD dwClientContext);
	static void OnHeartbeat(const DWORD dwClientContext);
	static void OnProfessionalWork(const DWORD dwClientContext);
	static void OnConnectError(const DWORD dwClientContext);

	static void OnCreateClient(const DWORD dwClientContext);
	static void OnDestroyClient(const DWORD dwClientContext);

	static void OnCreateClientInfo(const DWORD dwClientContext);
	static void OnDestroyClientInfo(const DWORD dwClientContext);

	BOOL	ProessAnalyseData(const DWORD dwClientContext, const char* RecvData, const DWORD dwBytes);
	BOOL	ProessAnalyseData(const DWORD dwClientContext, const char* RecvData, const DWORD nRecvLen, int nCmdType);
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnEnChangeGedit17();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBnService();
	afx_msg void OnBnClickedBnTcpListen();
	afx_msg void OnBnClickedBnTcpConnect();
	afx_msg void OnBnClickedButton6();
	CListCtrl m_lstInfo;
	afx_msg void OnEnChangeGedit18();
	afx_msg void OnBnClickedButton3();

#ifdef WANGZH //2013-5-6
	afx_msg void OnBnClickedBnTcpLogin();
#endif

	DWORD m_dwConnectionCount;
public:
	afx_msg void OnBnClickedBnTcpStates();
public:
	CButton m_bnTcpStates;
public:
	afx_msg void OnBnClickedBnClientAlarm();
public:
	CButton m_bnclientalarm;


	typedef CHashmapManager<DWORD,DATA_RECVBUFFER>  HashmapDataRecvBuffer;
	HashmapDataRecvBuffer m_HashmapDataRecvBuffer;
};
