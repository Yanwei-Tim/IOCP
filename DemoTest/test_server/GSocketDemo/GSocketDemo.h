// GSocketDemo.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CGSocketDemoApp:
// �йش����ʵ�֣������ GSocketDemo.cpp
//

class CGSocketDemoApp : public CWinApp
{
public:
	CGSocketDemoApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};
 
extern CGSocketDemoApp theApp;