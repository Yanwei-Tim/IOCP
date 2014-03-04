//
#include "StdAfx.h"
#include "WriteLogThread.h"

////////////////////////////////////////////////////////////////////////////////////////
//
CWriteLogThread::CWriteLogThread()
{
    m_hLogThreadEndEvt = NULL;
    m_hLogThread = NULL;
    m_dwLogThreadID = 0;
    m_bStartFlag = FALSE;
    
    m_nLogLevel = FOR_DEBUG;
    
    m_nLogFileIndex = 0;
    m_bLogListFull = FALSE;
    m_nLogListCount = 0;
    m_uBytesWritten = 0;
    
    m_uMaxFileSize = 1024 * 1024 * 64; //��־�ļ���С���ֵ 64M
    m_strCurrData = CTime::GetCurrentTime().Format("%Y:%m:%d");
    m_strLastCleanupLogFileDate = m_strCurrData;
    m_nLogFileCleanupDate = 0;
    
    memset(m_strAppLogDir,0,_MAX_PATH);

    memset(m_csLogFileName,0,64);
    
}

CWriteLogThread::~CWriteLogThread()
{
    StopLog();
    m_LogList.RemoveAll();
}

////////////////////////////////////////////////////////////////////////////////////////
//
//�����̺߳���
UINT WINAPI CWriteLogThread::WriteLogThreadProc(void *lpParameter)
{
    CWriteLogThread *pMyLogThread;
    pMyLogThread = (CWriteLogThread *)lpParameter;

    int timerid = SetTimer(NULL,NULL, 30000, 0L); //����30s�Ļ�׼��ʱ
    TRACE(_T("Into Log thread%X\n"), pMyLogThread->m_hLogThread);
    ResetEvent(pMyLogThread->m_hLogThreadEndEvt);
    MSG msg;
    BOOL bcontinue = TRUE;
    while(bcontinue)
    {
        if(GetMessage(&msg,NULL,0,0))
        {
            switch(msg.message)
            {
            case WM_THREAD_STOP:
                bcontinue = FALSE;
                break;
            case WM_LOGLIST_FULL_MSG:
                pMyLogThread->OnLogListFull();
                break;
            case WM_WRITE_LOG_MSG: 
                pMyLogThread->WriteLogFile();
                //д��־
                break;
            case WM_TIMER:
                pMyLogThread->RefreshLogFileName();
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
    SetEvent(pMyLogThread->m_hLogThreadEndEvt);

    TRACE(_T("Quit Log thread\n"));
    _endthreadex(0);
    return 0;
}

//����������־�ļ���Ŀ¼
void CWriteLogThread::CreateCurrAppLogDir()
{
    TCHAR AppCurrPath[_MAX_PATH] ={0}; 
    
    GetModuleFileName(NULL,AppCurrPath,_MAX_PATH); 

    (_tcsrchr(AppCurrPath,'\\'))[1] = 0; 

    sprintf(m_strAppLogDir,_T("%sLogFile"),AppCurrPath);

    CFileFind IsValidFile;
    if(!IsValidFile.FindFile(m_strAppLogDir))  //����Ŀ¼������
    {
        CreateDirectory(m_strAppLogDir,NULL);
    }
}

//GpsPublicNetLog
void CWriteLogThread::UpDataLogFileName()
{
    sprintf(m_LogFileName,"%s\\%s%s_%d.txt",
        m_strAppLogDir,
        CTime::GetCurrentTime().Format("%Y%m%d"),
        m_csLogFileName,
        m_nLogFileIndex);
}

//��־�ļ�����
void CWriteLogThread::CheckCleanupFileLogFile()
{
    if(m_nLogFileCleanupDate == 0)
    {
        return;
    }

     COleDateTime time1,time2;
     COleDateTimeSpan time3;
     time2 = COleDateTime::GetCurrentTime();
     int nYear,nMonth,nDate;
     
     sscanf(m_strLastCleanupLogFileDate, "%d:%d:%d",&nYear,&nMonth,&nDate);
     time1.SetDate(nYear,nMonth,nDate);
 
     time3 = time2 - time1;
     int nRunData = time3.GetDays();

    //������������m_nLogFileCleanupDate�����һ����־�ļ�����,����ǰһ�����־
    //m_nLogFileCleanupDate���Ե���SetLogFileCleanupDate()��������
    if(nRunData >= m_nLogFileCleanupDate) //
    {
        m_strLastCleanupLogFileDate = CTime::GetCurrentTime().Format("%Y:%m:%d");

        CFileFind tempFileFind;
	    CString szDir = m_strAppLogDir;
	    CString strTitle;
	    CFile file;

	    if(szDir.Right(1) != _T("\\"))
        {
		    szDir += _T("\\");
        }
	    szDir += _T("*.txt");

	    BOOL res = tempFileFind.FindFile(szDir);

        CString strYesterdayLogName;

        CTime time =CTime::GetCurrentTime() - CTimeSpan(1,0, 0,0);
        strYesterdayLogName = time.Format("%Y%m%d"); //ǰһ�������
        CString strDeleteFilePath;
	    while( res )
	    {
		    res = tempFileFind.FindNextFile();
		    if(!tempFileFind.IsDirectory() && !tempFileFind.IsDots())
		    {
			    strTitle = tempFileFind.GetFileName();
                if(strTitle.Find(strYesterdayLogName) == -1) //ֻ����ǰһ�����־�ļ�
                {
                    //ɾ��txt�ļ�
                    strDeleteFilePath.Format("%s\\%s",m_strAppLogDir,strTitle);
					TRY
					{
						CFile::Remove(strDeleteFilePath);//ɾ���ļ�
					}
					CATCH( CFileException, e )
					{
						TRACE("ɾ���ļ�ʧ��");
					}
					END_CATCH
                }
		    }
	    }
	    tempFileFind.Close();
    }
}

void CWriteLogThread::RefreshLogFileName()
{
    CString strCurrentData;
    CTime testCTime;

    strCurrentData = CTime::GetCurrentTime().Format("%Y:%m:%d");

    if(strCurrentData.Compare(m_strCurrData) != 0)
    {
        //�ж��Ƿ�Ҫ������־�ļ�������Ҫ������֮
        CheckCleanupFileLogFile();  

        m_strCurrData = strCurrentData;
        m_nLogFileIndex = 0;
        
        UpDataLogFileName();
    }
}

void CWriteLogThread::OnLogListFull()
{
    if(!m_bLogListFull)
    {
	    FILE *fhandle;
        fhandle = fopen(m_LogFileName, "a+");

	    if(fhandle == NULL)
        {
            return ;
        }
        if(fseek(fhandle, 0, SEEK_END) != 0)
        {
            fclose(fhandle);
            return ;
        }

	    char LogData[512] = {0};
	    sprintf(LogData,"%s ��־������δ�������Ϣ������\n",CTime::GetCurrentTime().Format("%H:%M:%S"));
#ifdef __SERVICE_MODE_
	    printf("%s",LogData);
#endif
        fwrite((LPCTSTR)LogData, 1, strlen(LogData), fhandle);
        fclose(fhandle);

        m_bLogListFull = TRUE;
    }
    
}

//
BOOL CWriteLogThread::WriteLogFile()
{   

    m_csLogList.Lock();

    if(m_LogList.IsEmpty())
    {
        m_LogList.RemoveAll();
        m_csLogList.Unlock();
        return FALSE;
    }

    LogMessage tmpLogMsg;
    tmpLogMsg = m_LogList.GetHead();
	m_LogList.RemoveHead();
    m_csLogList.Unlock();
    m_nLogListCount--;

    FILE *fhandle;
    fhandle = fopen(m_LogFileName, "a+");
/*
    switch(tmpLogMsg.logType)
    {
    case FOR_ALARM:
        break;
    case FOR_DEBUG: //�������ݸ���
        break;
    case FOR_TRACE: //���������̸���
        break;
    default:
        return FALSE;
        break;
    }
*/
    if(fhandle == NULL)
    {
        return FALSE;
    }
    if(fseek(fhandle, 0, SEEK_END) != 0)
    {
        fclose(fhandle);
        return FALSE;
    }

    size_t uWritten = fwrite((LPCTSTR)tmpLogMsg.LogData, 1, strlen(tmpLogMsg.LogData), fhandle);
    fclose(fhandle);
    
    m_uBytesWritten += uWritten;

    if(m_uBytesWritten > m_uMaxFileSize)
    {
        m_nLogFileIndex++;
        m_uBytesWritten = 0;
        UpDataLogFileName();

        TRACE("��־�ļ����������´���һ����־\n");
    }
    return TRUE;
}

//����ʣ��ռ��⣬�����ж����ڱ�����־�Ĵ��̿ռ��Ƿ���
//����-1,����̿ռ䲻��100M
int CWriteLogThread::GetSaveRecordDiskFreeSpace()
{
    char bufferDisk[3]={0};

    strncpy(bufferDisk,(LPCSTR)(LPCTSTR)m_strAppLogDir,2);
	bufferDisk[2] = '\0';

    char ZXBuf[256] = {0};
	
	if (GetDiskFreeSpaceEx(_T(bufferDisk),
		&m_nFreeBytesAvailableOfBuf,
		&m_nTotalNumberOfBytesOfBuf,
		&m_nTotalNumberOfFreeBytesOfBuf))
	{
		if ( (m_nFreeBytesAvailableOfBuf.QuadPart/(1024*1024))>1024 ) 
		{
            return 0;
		}
        else
		{
			sprintf(ZXBuf,_T("%I64d"),m_nFreeBytesAvailableOfBuf.QuadPart/(1024*1024));
            
            if(atoi(ZXBuf) < 100)
            {
                return -1;
            }
		}
	}

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
BOOL CWriteLogThread::StartLog(const char *pcsLogFileName,int level,int nCleanupDate)
{
    if(m_bStartFlag)
    {
        TRACE(_T("Log thread has run.\n"));
        return TRUE;
    }

    if(pcsLogFileName == NULL)
    {
        return FALSE;
    }
    
    int nLen = strlen(pcsLogFileName);
    if(nLen <= 0 || nLen >= 64)
    {
        return FALSE;
    }

    //����������־�ļ���Ŀ¼
    CreateCurrAppLogDir();

    if(m_hLogThreadEndEvt == 0)
    {
        m_hLogThreadEndEvt = CreateEvent(NULL,TRUE,TRUE,NULL);
    }
    
    m_hLogThread = (HANDLE)_beginthreadex( NULL,0,&WriteLogThreadProc,this, 0,&m_dwLogThreadID );
    if(!m_hLogThread)
    {
        TRACE(_T("Strat Log thread fail.\n"));
        return FALSE;
    }
    
    strcpy(m_csLogFileName,pcsLogFileName);

    if(level >= 0)
    {
        int nMaxLevel = FOR_DEBUG + 1;
        
        m_nLogLevel = level%nMaxLevel;
    }

    m_nLogFileCleanupDate = nCleanupDate;

    UpDataLogFileName();

    m_bStartFlag = TRUE;

    return TRUE;
}

//��ֹ�߳�
void CWriteLogThread::StopLog()
{
    if(m_bStartFlag)
    {
        m_bStartFlag = FALSE;
        //��ֹ�߳�
        PostThreadMessage(m_dwLogThreadID, WM_THREAD_STOP, 0L, 0L);
        ::WaitForSingleObject(m_hLogThreadEndEvt, INFINITE);
        
        CloseHandle(m_hLogThreadEndEvt);
        m_hLogThreadEndEvt = NULL;
        m_dwLogThreadID = 0;
    }
}

void CWriteLogThread::SetLogLevel(int level)
{
    if(level >= 0)
    {
        int nMaxLevel = FOR_DEBUG + 1;
        
        m_nLogLevel = level%nMaxLevel;
    }
}

//������־����ÿ��������һ��
void CWriteLogThread::SetLogFileCleanupDate(int nDateCount)
{
    m_nLogFileCleanupDate = nDateCount;
}

//
void CWriteLogThread::WriteLog(LOG_TYPE mode,const char *format , ... ) 
{
    if(!m_bStartFlag)
    {
        return ;
    }
    
    if(m_nLogLevel >= mode)
    {
        if( m_nLogListCount > 60 )
        {
            if( !m_bLogListFull )
            {
                PostThreadMessage(m_dwLogThreadID,WM_LOGLIST_FULL_MSG,0,0);
            }
            return;
        }
        
        if(m_bLogListFull) //
        {
            if(m_nLogListCount < 10)
            {
                m_bLogListFull = FALSE;
            }
        }
        else
        {
            LogMessage tmpLogMsg;
            memset(&tmpLogMsg,0,sizeof(LogMessage));
            tmpLogMsg.logType = mode;
            if(format != NULL)
            {
                
                va_list argp;
                va_start(argp, format);	
                
                int res = _vsnprintf(tmpLogMsg.LogData, 512, format, argp);
                if( res > 0 )
                {
#ifndef __SERVICE_MODE_
                    printf("%s",tmpLogMsg.LogData);
#endif
                    m_csLogList.Lock();
                    m_LogList.AddTail(tmpLogMsg);
                    m_csLogList.Unlock();
                    
                    m_nLogListCount++;
                    PostThreadMessage(m_dwLogThreadID,WM_WRITE_LOG_MSG,0,0); 
                }
                va_end(argp);
            }
        }
    }
}

void CWriteLogThread::WriteLog(LOG_TYPE mode,CString strLogTrace)
{
    if(!m_bStartFlag)
    {
        return ;
    }
    
    int nLen = strLogTrace.GetLength();
    if(m_nLogLevel >= mode )
    {
        if( m_nLogListCount > 60 )
        {
            if( !m_bLogListFull )
            {
                PostThreadMessage(m_dwLogThreadID,WM_LOGLIST_FULL_MSG,0,0);
            }
            return;
        }
        
        if(m_bLogListFull) //
        {
            if(m_nLogListCount < 10)
            {
                m_bLogListFull = FALSE;
            }
        }
        else
        {
            LogMessage tmpLogMsg;
            memset(&tmpLogMsg,0,sizeof(LogMessage));
            
            if( nLen < 510)
            {
                memcpy(tmpLogMsg.LogData,(char *)(LPCSTR)strLogTrace,strLogTrace.GetLength());
            }
            else
            {
                memcpy(tmpLogMsg.LogData,(char *)(LPCSTR)strLogTrace,510);
            }
#ifndef __SERVICE_MODE_        
            printf("%s",tmpLogMsg.LogData);
#endif
            tmpLogMsg.logType = mode;
            m_csLogList.Lock();
            m_LogList.AddTail(tmpLogMsg);
            m_csLogList.Unlock();
            m_nLogListCount++;
            PostThreadMessage(m_dwLogThreadID,WM_WRITE_LOG_MSG,0,0);
        }
    }
    
}