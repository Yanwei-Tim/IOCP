// GMemMngDemo.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CGMemMngDemoApp:
// �йش����ʵ�֣������ GMemMngDemo.cpp
//

class CGMemMngDemoApp : public CWinApp
{
public:
	CGMemMngDemoApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CGMemMngDemoApp theApp;