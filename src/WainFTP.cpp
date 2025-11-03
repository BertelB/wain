//=============================================================================
// This source code file is a part of Wain.
// It implements the varios functions releated to FTP.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\Wain.h"
#include ".\..\src\Wainview.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\Project.h"
#include ".\..\src\TagList.h"
#include ".\..\src\NavigatorList.h"
#include ".\..\src\childfrm.h"
#include ".\..\src\Waindoc.h"
#include ".\..\src\WainFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT __cdecl ThreadCreateFtpCon(LPVOID rp);
static DWORD ContextId = 2;

class FtpConnectionDialogClass : public DialogBaseClass
{
  DECLARE_DYNAMIC(FtpConnectionDialogClass);
public:
  FtpConnectionDialogClass(CWnd *parent = NULL);
  ~FtpConnectionDialogClass();
  CString m_hostName;
  CString m_userName;
  CString m_password;
  bool m_savePassword;
  enum {IDD = IDD_FTP_CONNECT_DIALOG};
protected:
  void DoDataExchange(CDataExchange *dx);
  virtual BOOL OnInitDialog(void);
  afx_msg void OnOK(void);
  afx_msg void HandleConnect(void);
  DECLARE_MESSAGE_MAP();
};

struct ThreadParmType
{
  CInternetSession *m_session;
  unsigned int m_attempt;
  CFtpConnection *m_ftpConnection;
  CWinThread *m_thread;
};

class FtpWaitDialogClass : public CDialog
{
  DECLARE_DYNAMIC(FtpWaitDialogClass);
  int m_count;
public:
  FtpWaitDialogClass(CWnd *parent);
  enum {IDD = IDC_FTP_WAIT_DIALOG};
  CWinThread *m_thread;
protected:
  virtual BOOL OnInitDialog(void);
  afx_msg void OnCancel(void);
  afx_msg void OnTimer(UINT timer_id);
  DECLARE_MESSAGE_MAP();
};

IMPLEMENT_DYNAMIC(FtpConnectionDialogClass, CDialog)
IMPLEMENT_DYNAMIC(FtpWaitDialogClass, CDialog)

FtpConnectionDialogClass::FtpConnectionDialogClass(CWnd *parent) : DialogBaseClass(FtpConnectionDialogClass::IDD, parent)
{
}

FtpConnectionDialogClass::~FtpConnectionDialogClass()
{

}

BOOL FtpConnectionDialogClass::OnInitDialog(void)
{
  m_savePassword = wainApp.gs.m_ftpSavePassword;
  m_hostName = wainApp.gs.m_ftpHostName;
  m_userName = wainApp.gs.m_ftpUserName;
  m_password = wainApp.gs.m_ftpPassword;
  UpdateData(FALSE);

  return CDialog::OnInitDialog();
}

void FtpConnectionDialogClass::OnOK(void)
{
  UpdateData(TRUE);
  if(!m_savePassword)
    m_password = "";

  wainApp.gs.m_ftpSavePassword = m_savePassword;
  wainApp.gs.m_ftpHostName = m_hostName;
  wainApp.gs.m_ftpUserName = m_userName;
  wainApp.gs.m_ftpPassword = m_password;

  EndDialog(IDOK);
}

void FtpConnectionDialogClass::HandleConnect(void)
{
   UpdateData(TRUE);

  wainApp.gs.m_ftpSavePassword = m_savePassword;
  wainApp.gs.m_ftpHostName = m_hostName;
  wainApp.gs.m_ftpUserName = m_userName;
  wainApp.gs.m_ftpPassword = m_password;

  EndDialog(IDC_FTP_CONNECT);
}

void FtpConnectionDialogClass::DoDataExchange(CDataExchange *dx)
{
  CDialog::DoDataExchange(dx);
  DDX_Text(dx, IDC_FTP_HOSTNAME, m_hostName);
  DDX_Text(dx, IDC_FTP_USERNAME, m_userName);
  DDX_Text(dx, IDC_FTP_PASSWORD, m_password);
  DdxCheck(dx, IDC_FTP_SAVE_PASS, m_savePassword);
}

BEGIN_MESSAGE_MAP(FtpConnectionDialogClass, CDialog)
  ON_BN_CLICKED(IDC_FTP_CONNECT, HandleConnect)
  ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP();

BEGIN_MESSAGE_MAP(FtpWaitDialogClass, CDialog)
  ON_BN_CLICKED(IDCANCEL, OnCancel)
  ON_WM_TIMER()
END_MESSAGE_MAP();

FtpWaitDialogClass::FtpWaitDialogClass(CWnd *parent) : CDialog(FtpWaitDialogClass::IDD, parent)
{
  m_count = 0;
  m_thread = NULL;
}

BOOL FtpWaitDialogClass::OnInitDialog(void)
{
  ThreadParmType *thread_parm  = new ThreadParmType;
  thread_parm->m_session = &GetMf()->m_navigatorDialog.m_inetSession;
  thread_parm->m_attempt = GetMf()->m_navigatorDialog.m_inetConnectAttempt;
  GetMf()->m_navigatorDialog.m_ftpConnections[GetMf()->m_navigatorDialog.m_inetConnectAttempt] = 0;

  thread_parm->m_ftpConnection = NULL;
  m_thread = AfxBeginThread(ThreadCreateFtpCon, thread_parm, THREAD_PRIORITY_LOWEST, 0, CREATE_SUSPENDED);
  m_thread->m_bAutoDelete = TRUE;
  GetMf()->m_navigatorDialog.m_inSetFtp = TRUE;
  thread_parm->m_thread = m_thread;
  m_thread->ResumeThread();

  SetTimer(IDC_FTP_WAIT_TIMER, 250, NULL);
  return CDialog::OnInitDialog();
}

void FtpWaitDialogClass::OnCancel(void)
{
  SetStatusText("Cancel");
  KillTimer(IDC_FTP_WAIT_TIMER);
  EndDialog(IDCANCEL);
}

void FtpWaitDialogClass::OnTimer(UINT id)
{
  ASSERT(id == IDC_FTP_WAIT_TIMER);
  if(m_count++ == 4)
  {
     CString msg;
     GetDlgItemText(IDC_FTP_WAIT_STATIC, msg);
     msg += ".";
     SetDlgItemText(IDC_FTP_WAIT_STATIC, msg);
     m_count = 0;
  }
}

void NavigatorDialog::HandleFtpConMsg(void *ftp, unsigned int attempt)
{
  ASSERT(attempt < MAX_NOF_FTP_CONS);
  ThreadParmType *parm = (ThreadParmType *)ftp;

  if(!m_ftpConnections[attempt])
  {
    if(attempt == m_inetConnectAttempt && m_inSetFtp)
    {
      m_ftpConnection = parm->m_ftpConnection;
    }
    else if(parm->m_ftpConnection)
      delete parm->m_ftpConnection;

    delete parm;
    if(m_ftpWaitDialog)
      m_ftpWaitDialog->EndDialog(IDOK);
    m_ftpConnections[attempt] = 1;
  }
}

void NavigatorDialog::HandleFtpConnectionection(void)
{
  if(!m_ftpConnection)
  {
    FtpConnectionDialogClass FtpConSetup(this);
    if(FtpConSetup.DoModal() == IDC_FTP_CONNECT)
    {
      m_ftpWaitDialog = new FtpWaitDialogClass(this);
      m_ftpWaitDialog->DoModal();
      delete m_ftpWaitDialog;
      m_ftpWaitDialog = NULL;
      m_inetConnectAttempt++;
      if(m_inetConnectAttempt >= MAX_NOF_FTP_CONS)
        m_inetConnectAttempt = 0;
      /* Ftpcon have been setup when we receive the message from the thread  */
      m_inSetFtp = FALSE;
      if(m_ftpConnection)
      {
        SetDlgItemText(IDB_FTP, "Disconnect");
        CString path = "/";
        m_ftpConnection->GetCurrentDirectory(path);
        m_ftpDir.EnableFtpMode(path);
      }
      else
        SetStatusText("Failed to make a FTP connection");
    }
  }
  else
  {
    m_ftpConnection->Close();
    delete m_ftpConnection;
    m_ftpConnection = NULL;
    SetDlgItemText(IDB_FTP, "Connect");
    m_ftpDir.DisableFtpMode();
  }
}

//---- Implementation of file_class
FileClass::~FileClass()
{

}

InetFileClass::InetFileClass(const char *file_name, unsigned int access_mode)
{
  m_inetFile = 0;
  if(!GetMf()->m_navigatorDialog.m_ftpConnection)
  {
    WainMessageBox(GetMf(), "No FTP connection", IDC_MSG_OK, IDI_WARNING_ICO);
    m_gotError = TRUE;
    return;
  }
  try
  {
    m_inetFile = GetMf()->m_navigatorDialog.m_ftpConnection->OpenFile(file_name, access_mode, FTP_TRANSFER_TYPE_BINARY, ContextId++);
  }
  catch (CInternetException *ex)
  {
    TCHAR str[1024];
    ex->GetErrorMessage(str, 1024);
    CString msg = "Received an internet exception:\r\n";
    msg += str;
    WainMessageBox(GetMf(), msg, IDC_MSG_OK, IDI_WARNING_ICO);
    ex->Delete();
    m_gotError = TRUE;
    return;
  }
  if(m_inetFile)
  {
    if(access_mode == GENERIC_READ)
      m_inetFile->SetReadBufferSize(8192);
    else
      m_inetFile->SetWriteBufferSize(8192);
  }
  else
    m_gotError = TRUE;
}

StdioFileClass::StdioFileClass(const char *file_name, unsigned int access_mode)
{
  m_file = fopen(file_name, access_mode == GENERIC_READ ? "rb" : "wb");
  if(!m_file)
    m_gotError = TRUE;
}

DebugFileClass::DebugFileClass(const char *file_name, unsigned int access_mode)
{
  SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
  m_fileHandle = CreateFile(file_name, access_mode, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
  if(!m_fileHandle)
    m_gotError = TRUE;
}

StdioFileClass::~StdioFileClass()
{
  if(m_file)
  {
    fclose(m_file);
    m_file = NULL;
  }
}

InetFileClass::~InetFileClass()
{
  if(!m_inetFile)
    return;
  if(GetMf()->m_navigatorDialog.m_ftpConnection)
    m_inetFile->Close();
  delete m_inetFile;
  m_inetFile = NULL;
}

DebugFileClass::~DebugFileClass()
{
  if(m_fileHandle)
    CloseHandle(m_fileHandle);

  m_fileHandle = 0;
}

GetStrLineEndType StdioFileClass::Read(char *ch)
{
   if(m_file)
   {
      int i;
      switch(i = fgetc(m_file))
      {
      case 0x0A:
         *ch = 0;
         return CR_LINE_END;
      case 0x0D:
         if((i = fgetc(m_file)) != 0x0A)
           ungetc(i, m_file);
         *ch = 0;
         return CR_LF_LINE_END;
      case 0x00:
         return Read(ch);
      case EOF:
         *ch = 0;
         return EOF_LINE_END;
      }
      *ch = (char )i;
      return NORMAL_LINE_END;
   }
   m_gotError = TRUE;
   return EOF_LINE_END;
}

GetStrLineEndType DebugFileClass::Read(char *ch)
{
   while (m_fileHandle)
   {
      DWORD num_read = 0;
      if(m_leftOverChar == 0 && !ReadFile(m_fileHandle, ch, 1, &num_read, 0))
      {
         CString error_msg;
         char *msg_buf;
         FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL,
                       GetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                       (char *)&msg_buf, 0, NULL);
         error_msg += "Windows reports the following error:\r\n";
         error_msg += msg_buf;
         WainMessageBox(GetMf(), error_msg, IDC_MSG_OK, IDI_WARNING_ICO);
         LocalFree(msg_buf);
         m_gotError = TRUE;
         *ch = 0;
         return EOF_LINE_END;
      }
      if (m_leftOverChar)
      {
         *ch = m_leftOverChar;
         m_leftOverChar = 0;
         return NORMAL_LINE_END;
      }
      if(num_read == 0)
      {
         *ch  = 0;
         return EOF_LINE_END;
      }
      switch(*ch)
      {
      case 0x0A:
         *ch = 0;
         return CR_LINE_END;
      case 0x0D:
         ReadFile(m_fileHandle, ch, 1, &num_read, 0);
         if (*ch != 0x0A)
         {
            m_leftOverChar = *ch;
         }
         *ch = 0;
         return CR_LF_LINE_END;
      case 0x00:
         break;
      default:
         return NORMAL_LINE_END;
      }
   }
   m_gotError = TRUE;
   return EOF_LINE_END;
}

GetStrLineEndType InetFileClass::Read(char *ch)
{

   if(m_inetFile)
   {
      try
      {
         if(m_inetFile->Read(ch, 1))
         {
            switch(*ch)
            {
            case 0x0A:
               *ch = 0;
               return CR_LINE_END;
            case 0x0D:
               m_inetFile->Read(ch, 1);
               *ch = 0;
               return CR_LF_LINE_END;
            case 0x00:
               *ch = ' ';
			   return NORMAL_LINE_END;
            default:
               return NORMAL_LINE_END;
            }
         }
         else
         {
            *ch = 0;
            return EOF_LINE_END;
         }
      }
      catch (CInternetException *ex)
      {
         TCHAR str[1024];
         ex->GetErrorMessage(str, 1024);
         CString msg = "Received an internet exception:\r\n";
         msg += str;
         WainMessageBox(GetMf(), msg, IDC_MSG_OK, IDI_WARNING_ICO);
         ex->Delete();
         *ch = 0;
         m_gotError = TRUE;
         return EOF_LINE_END;
      }
   }
   m_gotError = TRUE;
   return EOF_LINE_END;
}

bool StdioFileClass::Write(const char *s)
{
  size_t len = strlen(s);
  if(!len)
    return TRUE;
  if(m_file)
  {
    return fwrite(s, 1, len, m_file) < len ? FALSE : TRUE;
  }
  m_gotError = TRUE;
  return FALSE;
}

bool InetFileClass::Write(const char *s)
{
  size_t len = strlen(s);
  if(!m_inetFile)
  {
    m_gotError = true;
    return false;
  }
  try
  {
    m_inetFile->Write(s, len);
  }
  catch (CInternetException *ex)
  {
    TCHAR str[1024];
    ex->GetErrorMessage(str, 1024);
    CString msg = "Received an internet exception:\r\n";
    msg += str;
    WainMessageBox(GetMf(), msg, IDC_MSG_OK, IDI_WARNING_ICO);
    ex->Delete();
    m_gotError = true;
    return false;
  }
  return true;
}

bool DebugFileClass::Write(const char *)
{ // debug files does not write
   m_gotError = true;
   return false;
}

bool WainDoc::IsFileReadOnly(const char *file_name) const
{
  if(m_isFtpFile)
  {
    bool is_ro = true;
    if(GetMf()->m_navigatorDialog.m_ftpConnection)
    {
      CFtpFileFind finder(GetMf()->m_navigatorDialog.m_ftpConnection, ContextId++);
      if(finder.FindFile(file_name))
      {
        finder.FindNextFile();
        if(finder.IsReadOnly())
          is_ro = true;
        else
          is_ro = false;
      }
      finder.Close();
    }
    return is_ro;
  }
  return access(file_name, 2) ? true : false;
}


#if 0
// The standard BorlandC version has a BUG
UINT CInternetFile::Read(LPVOID lpBuf, UINT nCount)
{
  ASSERT_VALID(this);
  ASSERT(AfxIsValidAddress(lpBuf, nCount));
  ASSERT(m_hFile != NULL);
  ASSERT(m_bReadMode);

  DWORD dwBytes;

  if (!m_bReadMode || m_hFile == NULL)
    AfxThrowInternetException(m_dwContext, ERROR_INVALID_HANDLE);

  if (m_pbReadBuffer == NULL)
  {
    if(!InternetReadFile(m_hFile, (LPVOID) lpBuf, nCount, &dwBytes))
      AfxThrowInternetException(m_dwContext);
    return dwBytes;
  }

  LPBYTE lpbBuf = (LPBYTE )lpBuf;

  // if the requested size is bigger than our buffer,
  // then handle it directly

  if (nCount >= m_nReadBufferSize)
  {
    DWORD dwMoved = max((long )0, (long)m_nReadBufferBytes - (long)m_nReadBufferPos);
    memcpy(lpBuf, m_pbReadBuffer + m_nReadBufferPos, dwMoved);
    m_nReadBufferPos = m_nReadBufferSize;
    if(!InternetReadFile(m_hFile, lpbBuf+dwMoved, nCount-dwMoved, &dwBytes))
      AfxThrowInternetException(m_dwContext);
    dwBytes += dwMoved;
  }
  else
  {
    if (m_nReadBufferPos + nCount >= m_nReadBufferBytes)
    {
      #if 1
      DWORD dwMoved = max((long )0, (long )((long)m_nReadBufferBytes - (long)m_nReadBufferPos));
      #else
      int dwMoved = m_nReadBufferPos > m_nReadBufferBytes ? 0 : m_nReadBufferBytes - m_nReadBufferPos;
      #endif
      memcpy(lpbBuf, m_pbReadBuffer + m_nReadBufferPos, dwMoved);

      DWORD dwRead;
      if (!InternetReadFile(m_hFile, m_pbReadBuffer, m_nReadBufferSize, &dwRead))
        AfxThrowInternetException(m_dwContext);
      m_nReadBufferBytes = dwRead;

      dwRead = min(nCount - dwMoved, m_nReadBufferBytes);
      memcpy(lpbBuf + dwMoved, m_pbReadBuffer, dwRead);
      m_nReadBufferPos = dwRead;
      dwBytes = dwMoved + dwRead;
    }
    else
    {
      memcpy(lpbBuf, m_pbReadBuffer + m_nReadBufferPos, nCount);
      m_nReadBufferPos += nCount;
      dwBytes = nCount;
    }
  }

  return dwBytes;
}
#endif

UINT __cdecl ThreadCreateFtpCon(LPVOID rp)
{
  ThreadParmType *parm = (ThreadParmType *)rp;
  try
  {
     parm->m_ftpConnection = parm->m_session->GetFtpConnection(wainApp.gs.m_ftpHostName,
                                                           wainApp.gs.m_ftpUserName,
                                                           wainApp.gs.m_ftpPassword);
  }
  catch (CInternetException *ex)
  {
    TCHAR str[1024];
    ex->GetErrorMessage(str, 1024);
    ex->Delete();
    parm->m_ftpConnection = NULL;
  }
  wainApp.PostThreadMessage(IDC_FTP_WAIT_THREAD_MSG, (WPARAM )parm, parm->m_attempt);
  return 0;
}


