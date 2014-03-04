/*********************************************************************
* Copyright (c) 2008
* All rights reserved.
* 
* �ļ����ƣ�rwini.h
* �ļ���ʶ��
* ժ    Ҫ����ini�ļ���д����
* ��ʷ�޸ļ�¼��
*   ����            ����            �汾            ����
*
*********************************************************************/

#if !defined(AFX_RWINI_H__7D3FE520_41BD_41E6_A155_5DE0DE3B4625__INCLUDED_)
#define AFX_RWINI_H__7D3FE520_41BD_41E6_A155_5DE0DE3B4625__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Crwini  
{
public:
	void WriteInt(LPCTSTR appname,LPCTSTR keyname, int i,LPCTSTR fn);
	int ReadString(LPCTSTR appname, LPCTSTR keyname, LPCTSTR fn, LPTSTR strContent);
	int GetInt(LPCTSTR appname, LPCTSTR kn, LPCTSTR fn);
	int GetIntX(LPCTSTR appname, LPCTSTR keyname, LPCTSTR fn);
    int GetIntValue(LPCTSTR appname, LPCTSTR keyname, LPCTSTR fn);
	bool WriteString(LPCTSTR appname,LPCTSTR keyname,LPCTSTR s,LPCTSTR fn);
    BOOL CheckIniFile(LPCTSTR fileName);
	Crwini();
	virtual ~Crwini();

    char * GetCurrPath();
private:
    char m_FilePath[MAX_PATH]; 

};

#endif
//!defined(AFX_RWINI_H__7D3FE520_41BD_41E6_A155_5DE0DE3B4625__INCLUDED_)
