// GMemMngDemoDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CGMemMngDemoDlg �Ի���
class CGMemMngDemoDlg : public CDialog
{
// ����
public:
	CGMemMngDemoDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_GMEMMNGDEMO_DIALOG };

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
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CEdit m_edtMemSize;
	CEdit m_edtTimer;
	int m_nMemSize;
	int m_nTimer;
	CButton m_btnTimerMade;
	BOOL m_bTimerMade;
	BOOL m_bRanAlloc;
	BOOL m_bRanFree;
	int m_nWidth;
	float m_fBL;
	int nSizeOfPix;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButton1();
	CButton m_btnCreate;
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedButton2();
	CButton m_bnMade;
	void DrawMemInfo(void);
	CString m_strLong;
	int m_nPageSize;
	int m_nPageCount;
	int m_nSysPageSize;
	int m_nPPageCount;
	int m_nPMemSize;
	int m_nUsedSize;
	afx_msg void OnBnClickedButton3();
	CButton m_bnFreeAll;
	CEdit m_edtPageSize;
	int m_nPhyUse;
	afx_msg void OnBnClickedCheck4();
	float m_fPhyUsedR;
	float m_fMemUsedR;
	ULONG* m_pMemList;
	int m_nMemCount;
	void CheckAddr(void);
	void DeleteMem(int nIndex);
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnEnChangeEdit5();
	afx_msg void OnEnChangeEdit2();
	CButton m_bnAlloc;
	CButton m_bnFree;
	int m_nOperCount;
	float m_fPreOper;
	BOOL m_bFreeAll;
	afx_msg void OnBnClickedCheck5();
	CListCtrl m_lstThread;
	int m_nThreadCount;
	CButton m_bnThread;
	afx_msg void OnEnChangeEdit16();
	CEdit m_edtThreadCount;
	void* pThreadInfos;
	afx_msg void OnBnClickedButton6();
};
