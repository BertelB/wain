//=============================================================================
// This source code file is a part of Wain.
// It implements WainApp_class and WainMessageBoxClass.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\Wain.h"

#include ".\..\src\MainFrm.h"
#include ".\..\src\ChildFrm.h"
#include ".\..\src\WainDoc.h"
#include ".\..\src\WainView.h"
#include ".\..\src\Project.h"
#include ".\..\src\TagList.h"
#include ".\..\src\WordListThread.h"
#include <stdio.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4100) // unreferenced formal parameter

// WainIni wainIni("wain.cfg");

BEGIN_MESSAGE_MAP(WainApp, CWinApp)
  ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
  ON_COMMAND(ID_FILE_NEW, NewFile)
  ON_COMMAND(ID_FILE_OPEN, OpenFile)
  ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
  ON_COMMAND(IDM_CLOSE_FILE,       CloseFile)
  ON_COMMAND(IDM_SETUP, Setup)
  ON_COMMAND_EX_RANGE(ID_FILE_MRU_FILE1, ID_FILE_MRU_FILE16, OnOpenRecentFile)
END_MESSAGE_MAP();

const UINT TheOtherAppMessage = RegisterWindowMessage("Wain, please open this file");
// Copy and paste from afximpl.h
BOOL AFXAPI AfxFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
BOOL AFXAPI AfxResolveShortcut(CWnd *pWnd, LPCTSTR pszShortcutFile,	LPTSTR pszPath, int cchPath);

WainApp::WainApp()
{
  m_firstIdle = TRUE;
  m_uniqueClassName = _T("Wain ...");
  m_classRegistered = FALSE;;
  m_tagStr = "";
  m_lastTagStr = "";
  m_lastAutoTagFile = "";
  m_lastAutoTagView = NULL;
  m_openAutoTagFileAsNormalDoc = FALSE;
  m_openAsNormalDoc = "";
  m_tagPeekElem = 0;
  m_guiFont.CreateFont(
    8,                        // nHeight
    0,                        // nWidth
    0,                        // nEscapement
    0,                        // nOrientation
    FW_NORMAL,                // nWeight
    FALSE,                    // bItalic
    FALSE,                    // bUnderline
    0,                        // cStrikeOut
    ANSI_CHARSET,             // nCharSet
    OUT_DEFAULT_PRECIS,       // nOutPrecision
    CLIP_DEFAULT_PRECIS,      // nClipPrecision
    DEFAULT_QUALITY,          // nQuality
    DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily
    _T("MS Sans Serif"));     // lpszFacename
}

WainApp wainApp;

LRESULT CALLBACK get_msg_proc(int code, WPARAM wParam, LPARAM lParam);
HHOOK msg_hook;
char CommandLineFile[_MAX_PATH];
int CommandLineLineNo;

BOOL WainApp::InitInstance()
{
  AfxEnableControlContainer();
  CoInitialize(NULL);
  OleInitialize(0);
  m_lineNo = 1;
  m_columnNo = 1;
  m_profileRead = FALSE;
#ifdef _AFXDLL
  Enable3dControls();      // Call this when using MFC in a shared DLL
#else
  Enable3dControlsStatic();  // Call this when linking to MFC statically
#endif

  SetRegistryKey(_T("Wain Application"));

  // CCommandLineInfo cmdInfo;
  // ParseCommandLine(cmdInfo);
  // if(cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew && !strlen(cmdInfo.m_strFileName))
  //  cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
  // int once = GetProfileInt("Settings", "OneInst", 1);
  // if(once && !FirstInstance(cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen ? (const char *)cmdInfo.m_strFileName : (const char *)""))
  //  return FALSE;
   int Once = GetProfileInt("Settings", "OneInst", 1);
   int i;
   const char *StartFileName = 0;
   int StartLine = -1;
   for(i =  1; i < __argc; i++)
   {
      if(__argv[i][0] == '-' || __argv[i][0] == '/')
      {
         switch(__argv[i][1])
         {
         case 's':
         case 'S':
            Once = false;
            break;
         case 'n':
         case 'N':
            if(__argv[i][2])
            {
               StartLine = strtol(&__argv[i][2], 0, 10);
            }
            else if(i < __argc - 1)
            {
               i++;
               StartLine = strtol(__argv[i], 0, 10);
            }
            break;
         }
      }
      else
      {
         StartFileName = __argv[i];
      }
   }
   if(Once && !FirstInstance(StartFileName, StartLine))
     return FALSE;

  m_viewCursor = LoadCursor(IDR_VIEW_CURSOR);
  gs.ReadProfile();
  m_profileRead = TRUE;
  setlocale( LC_ALL, "" );
  LoadStdProfileSettings(10);  // Load standard INI file options (including MRU)

  WNDCLASS win_class;
  memset(&win_class, 0, sizeof(WNDCLASS));   // start with NULL defaults

  win_class.style = CS_DBLCLKS | /* CS_HREDRAW | CS_VREDRAW | */ CS_SAVEBITS;
  win_class.lpfnWndProc = ::DefWindowProc;
  win_class.hInstance = AfxGetInstanceHandle();
  win_class.hIcon = LoadIcon(IDR_MAINFRAME);
  win_class.hCursor = LoadCursor(IDC_ARROW);
  win_class.hbrBackground = (HBRUSH )::GetStockObject(WHITE_BRUSH);
  win_class.lpszMenuName = NULL;

  // specify our own class name for using FindWindow later
  win_class.lpszClassName = m_uniqueClassName;

  // register new class and exit if it fails
  if(!AfxRegisterClass(&win_class))
    return FALSE;
  m_classRegistered = TRUE;

  // Register the application's document templates.  Document templates
  //  serve as the connection between documents, frame windows and views.

  m_docTemplate = new CMultiDocTemplate(IDR_WAINTYPE, RUNTIME_CLASS(WainDoc), RUNTIME_CLASS(ChildFrame), RUNTIME_CLASS(WainView));
  AddDocTemplate(m_docTemplate);
  char OldProj[_MAX_PATH] = "";
  // create main MDI Frame window
   if(StartFileName && StartFileName[0])
   { /* If I'm opend withe a .yp or a .wpj file set it as project file */
      char ext[_MAX_EXT];
      MySplitPath(StartFileName, SP_EXT, ext);
      if(!stricmp(ext, gs.m_projectExtension.c_str()))
      {
         strcpy(OldProj, gs.m_projectName);
         strcpy(gs.m_projectName, StartFileName);
         StartFileName = 0;
      }
   }
  MainFrame *pMainFrame = new MainFrame;

  if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
    return FALSE;
  m_pMainWnd = pMainFrame;
   if(OldProj[0])
     GetMf()->HandleRecentProjects(OldProj);

   if(StartFileName && StartFileName[0])
   {
      strcpy(CommandLineFile, StartFileName);
      CreateLongFileName(CommandLineFile);
      CommandLineLineNo = StartLine;
   }
   else
      CommandLineFile[0] = 0;

  // The main window has been initialized, so show and Update it.
  if(gs.m_winMode == 2)
    pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
  else
    pMainFrame->ShowWindow(m_nCmdShow);

  pMainFrame->UpdateWindow();
  if(gs.m_viewNavigatorBar)
    pMainFrame->ViewNavigatorBar();
  pMainFrame->DragAcceptFiles(TRUE);

  msg_hook = SetWindowsHookEx(WH_GETMESSAGE, get_msg_proc, NULL, m_nThreadID);
  return TRUE;
}

void WainApp::Setup(void)
{
  gs.Setup();
}

int WainApp::ExitInstance()
{
  if(m_profileRead)
    gs.WriteProfile();
  m_profileRead = FALSE;
  if(m_classRegistered)
    ::UnregisterClass(m_uniqueClassName, AfxGetInstanceHandle());

  return CWinApp::ExitInstance();
}

class CAboutDlg : public CDialog
{
public:
  CAboutDlg();

  enum { IDD = IDD_ABOUTBOX };
protected:
  BOOL OnInitDialog(void);
  virtual void DoDataExchange(CDataExchange *dx);
protected:
  DECLARE_MESSAGE_MAP();
};

BOOL CAboutDlg::OnInitDialog(void)
{
   char FileName[1024];
   GetModuleFileName(0, FileName, sizeof(FileName));
   #define STAT _stat

   struct STAT status;
   if(!STAT(FileName, &status))
   {
      sprintf(FileName, "Released: %s", ctime(&status.st_mtime));
      SetDlgItemText(IDC_ABOUT_TIME, FileName);
   }
   else
   {
      SetDlgItemText(IDC_ABOUT_TIME, "---");
   }
   return TRUE;
}

BOOL WainApp::OnIdle(LONG lCount)
{
  if(m_firstIdle)
  {
    int fv = 0;

    CreateModeType cm = gs.m_createMode;
    gs.m_createMode = CreateModeType::CREATE_DEFAULT;

    if(gs.m_projectName[0])
      fv = GetMf()->m_navigatorDialog.m_project->SetProjectFile(gs.m_projectName);
    if(gs.m_loadFilesOnStartup)
    {
      int nof = GetProfileInt("Last Files", "Nof files", 0);
      if(nof)
      {
        for(int i = 0; i < nof; i++)
        {
          CString s;
          char n_str[20];
          sprintf(n_str, "Nr: %d", i);
          s = GetProfileString("Last Files", n_str, "");
          OpenDocument(s);
        }
      }
    }

    gs.m_createMode = cm;
    if(gs.m_createMode == CreateModeType::CREATE_DUAL)
      GetMf()->RecalcChildPos();
    else if(gs.m_createMode == CreateModeType::CREATE_MAX && GetMf()->m_viewList.m_currentView && GetMf()->m_viewList.m_currentView->m_myView)
      GetMf()->m_viewList.m_currentView->m_myView->MdiMax();

     m_firstIdle = FALSE;
     if(CommandLineFile[0])
     {
        WainDoc* Doc = OpenDocument(CommandLineFile, ODF_CMD_LINE_FILE);
        if(CommandLineLineNo != -1 && Doc)
           Doc->m_view->GotoLineNo(CommandLineLineNo);
      }
      else
         GetMf()->ActivateWin(fv >= 0 ? fv : 0); // To make sure that the first opened doc is viewed
    GetMf()->SetupMenu(TRUE);
  }
  else
  {
    char str[60];
    sprintf(str, "L: %5d C: %5d", m_lineNo, m_columnNo);
    GetMf()->m_statusBar.SetPaneText(STATUS_LINE, str);
    if(m_markStatus & 0x8000)
    {
      m_markStatus &= ~0x8000;
      if(m_markStatus)
        GetMf()->m_statusBar.SetPaneText(STATUS_MARK, "MARK");
      else
        GetMf()->m_statusBar.SetPaneText(STATUS_MARK, "          ");
    }

    if(m_lastTagStr != m_tagStr)
    {
      m_lastTagStr = m_tagStr;
      GetMf()->m_navigatorDialog.OnAutoTag();
    }
    if(m_tagPeekElem)
    {
      GetMf()->m_navigatorDialog.OnTagPeek(m_tagPeekElem);
      m_tagPeekElem = 0;
    }
    if(m_openAutoTagFileAsNormalDoc)
    {
      if(!m_lastAutoTagFile.empty())
      {
        int LineNo = 0;
        if(m_lastAutoTagView)
          LineNo = m_lastAutoTagView->m_lineNo;
        GetMf()->CloseDebugFile(m_lastAutoTagFile.c_str(), true, false);
        WainDoc *doc = OpenDocument(m_lastAutoTagFile.c_str());
        if(doc && LineNo != 0)
          doc->m_view->GotoLineNo(LineNo);
      }
      m_openAutoTagFileAsNormalDoc = FALSE;
    }
    if(m_openAsNormalDoc != "")
    {
      GetMf()->CloseDebugFile(m_openAsNormalDoc, true, false);
      OpenDocument(m_openAsNormalDoc);
      m_openAsNormalDoc = "";
    }
  }
  if(GetMf()->m_doCheckFileStatus)
  {
    GetMf()->CheckFileStatus();
    GetMf()->m_doCheckFileStatus = FALSE;
  }
  return CWinApp::OnIdle(lCount);
}

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange *dx)
{
  CDialog::DoDataExchange(dx);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP();

// App command to run the dialog
void WainApp::OnAppAbout()
{
  CAboutDlg aboutDlg;
  aboutDlg.DoModal();
}

#define SEP '$'
RtvStatus WainApp::ReplaceTagValues(std::string &aStr, const std::string &aCurrentFile)
{
   std::string::size_type pos;
   std::string ResStr;

   RtvStatus Status = RtvStatus::rtv_no_error;

   while((pos = aStr.find(SEP)) != std::string::npos && Status == RtvStatus::rtv_no_error)
   {
      ResStr += aStr.substr(0, pos);
      aStr = aStr.substr(pos + 1);
      std::string::size_type pos2 = aStr.find(SEP);

      if(pos2 == std::string::npos)
      {
         Status = RtvStatus::rtv_missing_sep;
      }
      else
      {
         std::string TagStr = aStr.substr(0, pos2);
         aStr = aStr.substr(pos2 + 1);
         if(TagStr.empty())
         {
            ResStr += "$";
         }
         else switch(TagStr[0])
         {
           case 'w':
           case 'W':
             if(TagStr.size() != 1)
             {
                Status = RtvStatus::rtv_missing_sep;
             }
             else
             {
                std::string Word;
                GetMf()->GetCurrentWord(Word);
                ResStr += Word;
             }
             break;
           case 't':
           case 'T':
             if(TagStr.size() != 1)
             {
                Status = RtvStatus::rtv_missing_sep;
             }
             else if(gs.m_tempPath.empty() || !IsDir(gs.m_tempPath.c_str()))
             {
               Status = RtvStatus::rtv_no_temp_path;
             }
             else
             {
                ResStr += gs.m_tempPath;
             }
             break;
           case 'F':
           case 'f':
             if(aCurrentFile.empty())
             {
               Status = RtvStatus::rtv_no_curr_file;
             }
             else if(TagStr.size() == 1)
             {
                ResStr += aCurrentFile;
             }
             else
             {
                 TagStr = TagStr.substr(1);
                 Status = ReplaceTagRest(TagStr, aCurrentFile);
                 if(Status == RtvStatus::rtv_no_error)
                    ResStr += TagStr;
             }
             break;
           case 'M':
           case 'm':
           {
             const std::string &make_file = GetMf()->m_navigatorDialog.m_project->GetMakeFile();
             if(make_file.empty())
             {
                Status = RtvStatus::rtv_no_make_file;
             }
             else if(TagStr.size() == 1)
             {
                ResStr += make_file;
             }
             else
             {
                 TagStr = TagStr.substr(1);
                 Status = ReplaceTagRest(TagStr, std::string(make_file));
                 if(Status == RtvStatus::rtv_no_error)
                    ResStr += TagStr;
             }
             break;
           }
           case 'P':
           case 'p':
           {
             const std::string &pn = GetMf()->m_navigatorDialog.m_project->GetProjectName();
             if(pn.empty())
             {
                Status = RtvStatus::rtv_no_project;
             }
             else if(TagStr.size() == 1)
             {
                ResStr += pn;
             }
             else
             {
                 TagStr = TagStr.substr(1);
                 Status = ReplaceTagRest(TagStr, std::string(pn));
                 if(Status == RtvStatus::rtv_no_error)
                    ResStr += TagStr;
             }
             break;
           }
           case 'X':
           case 'x':
           {
             std::string ProjFile;
             const char *s;
             s = &TagStr[1];
             if((Status = GetMf()->m_navigatorDialog.MakeProjectFileList(ProjFile, s)) == RtvStatus::rtv_no_error)
             {
                ResStr += ProjFile;
             }
             break;
           }
           case 'i':
           case 'I':
             if(TagStr.size() != 1)
             {
               Status = RtvStatus::rtv_missing_sep;
             }
             else
             {
                ResStr += gs.m_incPath;
                if(!gs.m_incPath.empty() && gs.m_incPath[gs.m_incPath.size() - 1] != '\\')
                   ResStr  += "\\";
             }
             break;
           case 'e':
           case 'E':
           {
             const std::string &pi = GetMf()->m_navigatorDialog.m_project->GetIncPath();
             if(pi.empty())
             {
               Status = RtvStatus::rtv_no_project_inc_path;
             }
             else if(TagStr.size() == 1)
             {
                Status = RtvStatus::rtv_missing_sep;
             }
             else
             {
                ResStr += pi;
                if(pi.substr(pi.size() -1) != "\\")
                   ResStr += "\\";
             }
             break;
           }
           default:
              Status = RtvStatus::rtv_unknown_tag;
              break;
         }
    }
  }

  ResStr += aStr;
  aStr = ResStr;
  return Status;
}

RtvStatus WainApp::ReplaceTagRest(std::string &aDest, const std::string &aFile)
{
  unsigned int flags = 0;
  const char *t;

  for(t = aDest.c_str(); *t != 0; t++)
  {
    if(*t == 'p' || *t == 'P')
      flags |= SP_DRIVE | SP_DIR;
    else if(*t == 'f' || *t == 'F')
      flags |= SP_FILE;
    else if(*t == 'e' || *t == 'E')
      flags |= SP_EXT;
    else /* Unexpected flag */
      return RtvStatus::rtv_ill_formed_tag;
  }
  if(!flags)
     return RtvStatus::rtv_ill_formed_tag;

  MySplitPath(aFile.c_str(), flags, aDest);

  return RtvStatus::rtv_no_error;
}

void CreateLongPathName(char *path)
{
  char temp[MAX_PATH], *s, dest[MAX_PATH], temp2[MAX_PATH], temp3[MAX_PATH], temp4[MAX_PATH], temp5[MAX_PATH];
  CFileFind finder;

  strcpy(temp, path);
  s = strtok(temp, "\\");
  dest[0] = 0;
  temp5[0] = 0;
  BOOL working, got_it;

  while(s)
  {
    strcat(temp5, s);
    got_it = FALSE;
    strcpy(temp2, dest);
    strcpy(temp4, s);
    if(strchr(s, '~'))
    {
      strcat(temp2, "\\*.*");
      working = finder.FindFile(temp2);

      while(working && !got_it)
      {
        working = finder.FindNextFile();
        if(finder.IsDirectory() && !finder.IsDots())
        {
          strcpy(temp3, finder.GetFilePath());
          GetShortPathName(temp3, temp4, MAX_PATH);

          if(!stricmp(temp4, temp5))
          {
            got_it = TRUE;
            strcpy(dest, temp3);
            strcat(dest, "\\");
          }
        }
      }
      finder.Close();
    }
    if(!got_it)
    {
      strcat(dest, temp4);
      strcat(dest, "\\");
    }
    strcat(temp5, "\\");
    s = strtok(NULL, "\\");
  }
  strcpy(path, dest);
}

void CreateLongFileName(char *short_path)
{
  char ext[MAX_PATH];
  char path[MAX_PATH];
  char temp[MAX_PATH];
  char temp2[MAX_PATH];
  MySplitPath(short_path, SP_DIR | SP_DRIVE, path);
  MySplitPath(short_path, SP_EXT, ext);
  if(strlen(path) && path[strlen(path) - 1] != '\\')
    strcat(path, "\\");

  CreateLongPathName(path);

  sprintf(temp, "%s*%s", path, ext);
  CFileFind finder;
  BOOL working = finder.FindFile(temp);
  while(working)
  {
    working = finder.FindNextFile();
    GetShortPathName(finder.GetFilePath(), temp2, MAX_PATH);
    if(!stricmp(short_path, temp2))
    {
      char temp3[MAX_PATH];
      MySplitPath(finder.GetFilePath(), SP_FILE | SP_EXT, temp3);
      sprintf(short_path, "%s%s", path, temp3);
      finder.Close();
      return;
    }
  }
  finder.Close();
  char temp4[MAX_PATH];
  MySplitPath(short_path, SP_FILE | SP_EXT, temp4);
  sprintf(short_path, "%s%s", path, temp4);
}

bool WainApp::FirstInstance(const char *aFileName, int aStartLine)
{
   // Check for a previous instance of this application by searching for a
   // window with our specific pre-registered class name. If one is found,
   // then Activate it.

   CWnd *pWndChild;
   MainFrame *pWndPrev;
   // determine if another window with our class name exists...
   if((pWndPrev = (MainFrame *)CWnd::FindWindow(m_uniqueClassName, NULL)) != NULL)
   {
      pWndChild = pWndPrev->GetLastActivePopup(); // if so, does it have any popups?
      if(pWndPrev->IsIconic())
         pWndPrev->ShowWindow( SW_RESTORE );  // If iconic, restore the main window
      // bring the main window or it's popup to the foreground
      // and we are Done activating the previous one.
      pWndChild->SetForegroundWindow();
      if(aFileName && aFileName[0])
      {
         char FullPath[_MAX_PATH];
         AfxFullPath(FullPath, aFileName);
         // BOOL do_wait = MyStrIStr(full_path, "temp\\$wc") ? TRUE : FALSE; /* Windows Commander Hack */
         if(strchr(FullPath, '~'))
            CreateLongFileName(FullPath);
         ATOM temp1 = GlobalAddAtom(FullPath);
         ATOM temp2 = GlobalAddAtom(ToString(aStartLine).c_str());
         ::PostMessage(pWndPrev->m_hWnd, TheOtherAppMessage, temp1, temp2);
      }
      return false;
   }
   return true; // first instance; proceed as normal
}

void WainApp::CloseFile(void)
{
  GetMf()->CloseFile();
}

void WainApp::NewFile(void)
{
  static unsigned int NewCounter;
  char FileName[32];
  sprintf(FileName, "Default%d.new", NewCounter++);
  OpenDocument(FileName, ODF_NEW_FILE);
}

void  WainApp::OpenFile(void)
{
  MyFileDialogClass fd(FD_OPEN | FD_LAST_PATH | FD_DOC_FILTER | FD_SET_LAST_PATH, OFN_HIDEREADONLY | OFN_NOCHANGEDIR);
  if(fd.DoModal() == IDOK)
  {
    OpenDocument(fd.GetPathName());
  }
}

void WainApp::Exit(void)
{
  CWinApp::OnAppExit();
}

extern void DeallocateTagsRead(WPARAM w);

LRESULT CALLBACK get_msg_proc(int code, WPARAM wParam, LPARAM lParam)
{
  if(code == HC_ACTION)
  {
    MSG *msg = (MSG *)lParam;
    MainFrame *mf = GetMf();
    if(mf && ::IsWindow(mf->m_hWnd))
    {
      if(msg->message == IDB_GLOB_TAG_DONE)
      {
        if(wParam == PM_REMOVE)
        {
          if(wainApp.m_pMainWnd)
            mf->m_navigatorDialog.GlobalTagReadFunc((ReadTagClass *)msg->wParam);
          else if(msg->wParam)
            DeallocateTagsRead(msg->wParam);
          return 0;
        }
      }
      else if(msg->message == IDB_TAG_DONE)
      {
        if(wParam == PM_REMOVE)
        {
          if(wainApp.m_pMainWnd)
            mf->m_navigatorDialog.TagReadFunc((ReadTagClass *)msg->wParam);
          else if(msg->wParam)
            DeallocateTagsRead(msg->wParam);
          return 0;
        }
      }
      else if (msg->message == IDB_AUTO_TAG_DONE)
      {
        if(wParam == PM_REMOVE)
        {
          if(msg->wParam)
             DeallocateTagsRead(msg->wParam);
          return 0;
        }

      }
      else if(msg->message == IDC_FTP_WAIT_THREAD_MSG)
      {
        mf->m_navigatorDialog.HandleFtpConMsg((void *)msg->wParam, (unsigned int )msg->lParam);
      }
      else if (msg->message == IDB_WORD_THREAD_DONE)
      {
         if (wParam == PM_REMOVE)
         {
            WordThreadParam* parm = (WordThreadParam*)msg->wParam;
            if (!parm->m_replace)
            {
               delete mf->m_navigatorDialog.m_project->m_wordParam[parm->m_propIndex];
               mf->m_navigatorDialog.m_project->m_wordParam[parm->m_propIndex] = parm;
            }
            else
            {
               mf->m_navigatorDialog.m_project->ReplaceWordParm(parm);
               delete parm;
            }
            SetStatusText("Word parameters has been read");
         }
      }
      else if(msg->message == GetIconMsgId && wParam == PM_REMOVE)
      {
        if(msg->wParam)
        {
          mf->m_navigatorDialog.HandleGetIcon((void *)msg->wParam);
          msg->wParam = NULL;
        }
      }
      else if(msg->message == CheckStatusMsgId && wParam == PM_REMOVE)
        mf->OnFileCheckStatus((FileCheckListType *)msg->wParam);
    }
  }
  return CallNextHookEx(msg_hook, code, wParam, lParam);
}

class WainMessageBoxClass : public CDialog
{
public:
  WainMessageBoxClass(CWnd *parent = NULL);
  enum {IDD = IDD_MSG_BOX};
  CString m_caption;
  CString m_msg;
  CString m_custButtonText;
  WORD m_flags;
  WORD m_ico;
protected:
  virtual BOOL OnInitDialog(void);
  virtual void OnCancel(void);
protected:
  afx_msg void OkHit(void);
  afx_msg void CancelHit(void);
  afx_msg void YesHit(void);
  afx_msg void NoHit(void);
  afx_msg void CustomHit(void);
  DECLARE_MESSAGE_MAP();
};

WainMessageBoxClass::WainMessageBoxClass(CWnd *parent) : CDialog(WainMessageBoxClass::IDD, parent)
{
}

static WORD b_flags[] =
{
  IDC_MSG_OK,
  IDC_MSG_CANCEL,
  IDC_MSG_YES,
  IDC_MSG_NO,
  IDC_MSG_CUST
};

BOOL WainMessageBoxClass::OnInitDialog(void)
{
  SetWindowText(m_caption);
  char text[2048];
  strcpy(text, m_msg);
  CStatic *t = (CStatic *)GetDlgItem(IDC_MSG_TEXT);
  ASSERT(t);
  CDC *dc = t->GetDC();
  CFont *font = GetFont();
  CFont *old_font = dc->SelectObject(font);
  int len = 0;
  int y = 10;
  CSize p;

  char *s = strtok(text, "\r\n");

  while(s)
  {
    p = dc->GetOutputTextExtent(s, strlen(s));
    if(p.cx > len)
      len = p.cx;
    y += p.cy;
    s = strtok(NULL, "\r\n");
  }
  dc->SelectObject(old_font);
  ReleaseDC(dc);
  ASSERT(m_flags != 0);
  int i, j;
  CButton *b;
  BOOL focus_set = FALSE;
  for(i = 0, j = 0; i < sizeof(b_flags)/sizeof(b_flags[0]); i++)
  {
    b = (CButton *)GetDlgItem(b_flags[i]);
    if((m_flags & b_flags[i]) == b_flags[i])
    {
      RECT br = CRect(5 + j*60, y + p.cy, 50 + j*60, y + p.cy + 24);
      b->MoveWindow(&br);
      j++;
      if(!focus_set)
      {
        b->SetFocus();
        focus_set = TRUE;
      }
    }
    else
    {
      b->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE);
    }
    b->InvalidateRect(NULL, TRUE);
  }
  if(m_flags & IDC_MSG_CUST)
    SetDlgItemText(IDC_MSG_CUST, m_custButtonText);
  if((m_flags & IDC_MSG_OK) == IDC_MSG_OK)
  {
    SetDefID(IDC_MSG_OK);
  }
  else if((m_flags & IDC_MSG_YES) == IDC_MSG_YES)
  {
    SetDefID(IDC_MSG_YES);
  }
  if(len < j*60 + 10)
    len = j*60 + 10;
  if(m_ico)
    len += 64;
  len += 25;
  WINDOWPLACEMENT place;
  t->GetWindowPlacement(&place);
  place.rcNormalPosition.right = len + 25;
  place.rcNormalPosition.left = 5;
  place.rcNormalPosition.bottom = y;
  t->MoveWindow(&place.rcNormalPosition);
  #if 0
  GetWindowPlacement(&place);
  place.rcNormalPosition.right = len + place.rcNormalPosition.left;
  place.rcNormalPosition.bottom = y + 80 + place.rcNormalPosition.top;
  MoveWindow(&place.rcNormalPosition);
  #endif
  {
     RECT R;
     GetMf()->GetWindowRect(&R);
     RECT N;
     int midY = (R.bottom - R.top)/2 + R.top;
     int midX = (R.right - R.left)/2 + R.left;
     N.top = midY - (y + 80)/2;
     N. bottom = midY + (y + 80)/2;
     N.left = midX - len/2;
     N.right = midX + len/2;
     MoveWindow(&N);
  }
  SetDlgItemText(IDC_MSG_TEXT, m_msg);
  /* First disable all icons */
  CStatic *icon = (CStatic *)GetDlgItem(IDI_QUESTION_ICO);
  ASSERT(icon);
  icon->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE);
  icon = (CStatic *)GetDlgItem(IDI_WARNING_ICO);
  ASSERT(icon);
  icon->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE);
  icon = (CStatic *)GetDlgItem(IDI_INFORMATION_ICO);
  ASSERT(icon);
  icon->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE);
  icon = (CStatic *)GetDlgItem(IDI_ERROR_ICO);
  ASSERT(icon);
  icon->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE);
  /* Then check if one of them is to be visible, if so enable it & move it */
  if(m_ico)
  {
    t->GetWindowPlacement(&place);
    icon = (CStatic *)GetDlgItem(m_ico);
    ASSERT(m_ico);
    icon->ModifyStyle(WS_DISABLED, WS_VISIBLE, SWP_NOACTIVATE);
    icon->MoveWindow(place.rcNormalPosition.right - 75, place.rcNormalPosition.top + 5, 32, 32);
  }

  return FALSE;
}

void WainMessageBoxClass::OkHit(void)
{
  EndDialog(IDC_MSG_OK);
}

void WainMessageBoxClass::YesHit(void)
{
  EndDialog(IDC_MSG_YES);
}

void WainMessageBoxClass::NoHit(void)
{
  EndDialog(IDC_MSG_NO);
}

void WainMessageBoxClass::CancelHit(void)
{
  EndDialog(IDC_MSG_CANCEL);
}

void WainMessageBoxClass::CustomHit(void)
{
   EndDialog(IDC_MSG_CUST);
}

void WainMessageBoxClass::OnCancel(void)
{
  if((m_flags & IDC_MSG_CANCEL) == IDC_MSG_CANCEL)
    EndDialog(IDC_MSG_CANCEL);
}

BEGIN_MESSAGE_MAP(WainMessageBoxClass, CDialog)
  ON_BN_CLICKED(IDC_MSG_OK,     OkHit)
  ON_BN_CLICKED(IDC_MSG_YES,    YesHit)
  ON_BN_CLICKED(IDC_MSG_NO,     NoHit)
  ON_BN_CLICKED(IDC_MSG_CANCEL, CancelHit)
  ON_BN_CLICKED(IDC_MSG_CUST,   CustomHit)
END_MESSAGE_MAP();


WORD WainMessageBox(CWnd *parent, const std::string& _str, WORD flags, WORD icon, const char *CustBText)
{
   return WainMessageBox(parent, _str.c_str(), flags, icon, CustBText);
}

WORD WainMessageBox(CWnd *parent, const char *msg, WORD flags, WORD icon, const char *CustBText)
{
  WainMessageBoxClass mb(parent);
  mb.m_caption = "Wain";
  mb.m_msg = msg;
  mb.m_flags = flags;
  mb.m_ico = icon;
  if(flags & IDC_MSG_CUST && CustBText)
  {
    mb.m_custButtonText = CustBText;
  }
  return (WORD )mb.DoModal();
}

WainDoc *WainApp::OpenDocument(const char *file_name, unsigned int flags)
{
  CWaitCursor wait;
  char path[_MAX_PATH];
  char temp_path[_MAX_PATH];
  char link_name[_MAX_PATH];

  /* first get the real filename */
  ASSERT(strlen(file_name) < _MAX_PATH);
  strcpy(temp_path, file_name[0] == '\"' ? &file_name[0] : file_name);
  char *s = strchr(temp_path, '\"');
  if(s)
    *s = '\0';

  if(flags == 0)
  {
    AfxFullPath(path, temp_path);
    if(AfxResolveShortcut(AfxGetMainWnd(), path, link_name, _MAX_PATH))
      strcpy(path, link_name);
  }
  else
    strcpy(path, temp_path);

  WainDoc *doc;
  doc = GetMf()->ActiveFile(path);
  if(doc)
  { /* The file is open, so ActiveFile() has Activated the view, so simply return the doc */
    return doc;
  }

  doc = (WainDoc *)WainDoc::CreateObject();
  doc->m_newFile = flags & ODF_NEW_FILE ? TRUE : FALSE;
  doc->m_isFtpFile = flags & ODF_FTP_FILE ? TRUE : FALSE;
  ChildFrame *frame = (ChildFrame *)ChildFrame::CreateObject();
  CCreateContext context;
  memset(&context, 0, sizeof(context));
  context.m_pCurrentDoc = doc;
  context.m_pNewViewClass = RUNTIME_CLASS(WainView);
  context.m_pNewDocTemplate = m_docTemplate;
  doc->m_childFrame = frame;
  frame->m_doc = doc;
  frame->LoadFrame(IDR_WAINTYPE, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, AfxGetMainWnd(), &context);

  doc->m_isDebugFile = FALSE;
  if(!doc->OnOpenDocument(path))
  {
    if(flags & ODF_CMD_LINE_FILE)
    {
      CString msg = "Failed to open document\r\n";
      msg += path;
      msg += "\r\nDo you want to create it?";
      if(WainMessageBox(GetMf(), msg, IDC_MSG_YES | IDC_MSG_NO, IDI_QUESTION_ICO) == IDC_MSG_NO)
      {
        frame->DestroyWindow();
        return NULL;

      }
      doc->m_newFile = FALSE;
      doc->SetPathName(path);
      *doc += "";
      doc->SaveFile();
    }
    else
    {
      CString msg = "Failed to open document\r\n";
      msg += path;
      WainMessageBox(GetMf(), msg, IDC_MSG_OK, IDI_WARNING_ICO);
      frame->DestroyWindow();
      return NULL;
    }
  }
  doc->SetPathName(path, doc->m_newFile ? FALSE : TRUE);

  frame->InitialUpdateFrame(doc, TRUE);
  ASSERT(doc->m_view);
  GetMf()->SetActiveView(doc->m_view);
  return doc;
}

BOOL WainApp::OnOpenRecentFile(UINT id)
{
  ASSERT_VALID(this);
  ASSERT(m_pRecentFileList != NULL);

  ASSERT(id >= ID_FILE_MRU_FILE1);
  ASSERT(id < ID_FILE_MRU_FILE1 + (UINT )m_pRecentFileList->GetSize());
  int index = id - ID_FILE_MRU_FILE1;
  ASSERT((*m_pRecentFileList)[index].GetLength() != 0);

  if(!OpenDocument((*m_pRecentFileList)[index]))
    m_pRecentFileList->Remove(index);

  return TRUE;
}

char *MySplitPath(const char *full_name, unsigned int what, char *dest)
{
  char drive[MAX_PATH];
  char dir[MAX_PATH];
  char file[MAX_PATH];
  char ext[MAX_PATH];

  _splitpath(full_name, drive, dir, file, ext);

  dest[0] = 0;

  if(what & SP_DRIVE)
    strcat(dest, drive);

  if(what & SP_DIR)
    strcat(dest, dir);

  if(what & SP_FILE)
    strcat(dest, file);

  if(what & SP_EXT)
    strcat(dest, ext);

  return dest;
}

const char *MySplitPath(const char *full_name, unsigned int what, CString &dest)
{
  char drive[MAX_PATH];
  char dir[MAX_PATH];
  char file[MAX_PATH];
  char ext[MAX_PATH];

  _splitpath(full_name, drive, dir, file, ext);

  dest = "";

  if(what & SP_DRIVE)
    dest += drive;

  if(what & SP_DIR)
    dest += dir;

  if(what & SP_FILE)
    dest += file;

  if(what & SP_EXT)
    dest += ext;

  return dest;
}

const char *MySplitPath(const char *full_name, unsigned int what, std::string &dest)
{
  char drive[MAX_PATH];
  char dir[MAX_PATH];
  char file[MAX_PATH];
  char ext[MAX_PATH];

  _splitpath(full_name, drive, dir, file, ext);

  dest = "";

  if(what & SP_DRIVE)
    dest += drive;

  if(what & SP_DIR)
    dest += dir;

  if(what & SP_FILE)
    dest += file;

  if(what & SP_EXT)
    dest += ext;

  return dest.c_str();
}


MyFileDialogClass::MyFileDialogClass(unsigned int flags, unsigned int std_flags, const char *def_ext, const char *file_name, const char *filter, CWnd *parent) :
                      CFileDialog(flags & FD_OPEN ? TRUE : FALSE, def_ext, file_name, std_flags, NULL, parent)
{
   ASSERT(flags & (FD_SAVE | FD_OPEN));
   m_flags = flags;
   if(flags & FD_DOC_FILTER)
      m_ofn.lpstrFilter = wainApp.gs.FilterStrings();
   else
      m_ofn.lpstrFilter = filter;
   if(flags & FD_LAST_PATH)
      m_ofn.lpstrInitialDir = wainApp.gs.m_lastUsedDir;
   else if(flags & FD_CONFIG_PATH)
      m_ofn.lpstrInitialDir = wainApp.gs.m_configPath;
}

int MyFileDialogClass::DoModal(void)
{
  int res = CFileDialog::DoModal();
  if(res == IDCANCEL)
  {
    if(CommDlgExtendedError() == CDERR_STRUCTSIZE)
    {
      res = CFileDialog::DoModal();
    }
  }
  if(res != IDCANCEL && (m_flags & FD_SET_LAST_PATH))
  {
    if(m_flags & FD_MULTI_FILES)
    {
      POSITION pos = GetStartPosition();
      CString s = "";
      if(pos)
        wainApp.gs.SetLastUsedDir(GetNextPathName(pos));
    }
    else
      wainApp.gs.SetLastUsedDir(GetPathName());
  }
  return res;
}


bool ColorType::operator == (ColorType other)
{
  if(m_textColor != other.m_textColor)
    return FALSE;
  if(m_backColor != other.m_backColor)
    return FALSE;

  return TRUE;
}

const char * const KeyIndexStr[] =
{
  "Mark",
  "Text",
  "Match",
  "Seperators",
  "PreProcessor",
  "Comments",
  "Strings",
  "Numbers",
  "Tags",
  "Words",
  "Keyword0",
  "Keyword1",
  "Keyword2",
  "Keyword3",
  "Keyword4"
};

void DisplayRtvError(CWnd *aParent, const char *aMsg, RtvStatus aError, bool aFatal)
{
   static const char* const rtv_error_str[] =
   {
      "No Error",
      "Missing Seperator ($)",
      "No temp path specified",
      "No files open",
      "Invalid formed tag",
      "No project makefile",
      "No project open",
      "No files in project or none selected",
      "Empty include path",
      "Unknown tag",
      "String To Long"
   };

   CString message = aMsg;
   if(wainApp.gs.m_debugTools || aFatal)
   {
      message += "\r\nBecause of the following error:\r\n";
      message += rtv_error_str[uint32_t(aError)];
      WainMessageBox(aParent, message, IDC_MSG_OK, IDI_WARNING_ICO);
   }
   else
   {
      message += ": ";
      message += rtv_error_str[uint32_t(aError)];
      SetStatusText(message);
   }
}

void WainApp::SetTagPeek(GetTagElemClass *elem)
{
  if(m_tagPeekElem)
    delete m_tagPeekElem;

  m_tagPeekElem = elem;
}

bool PathExist(const char* path)
{
   DWORD dwAttrib = GetFileAttributes(path);
//#define INVALID_FILE_ATTRIBUTES (-1)
   return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
