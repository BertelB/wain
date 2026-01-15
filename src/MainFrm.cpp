//=============================================================================
// This source code file is a part of Wain.
// It implements most of main_frame_class.
// Some functions from main_frame_class related to macros are implemented in Macro.cpp.
// Some functions from main_frame_class related to Bookmarks are implemented in ViewList.cpp.
// Some functions from main_frame_class related to Tools are implemented in Tools.cpp
// Some functions from main_frame_class related to Search and Search&replace are implemented in Search.cpp
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\resource.h"
#include ".\..\src\Wain.h"
#include ".\..\src\MainFrm.h"
#include ".\..\src\Project.h"
#include ".\..\src\KeySetup.h"
#include ".\..\src\WainSearch.h"
#include ".\..\src\DocProp.h"
#include ".\..\src\wainview.h"
#include ".\..\src\Tools.h"
#include ".\..\src\Navigatorlist.h"
#include ".\..\src\childfrm.h"
#include ".\..\src\waindoc.h"
#include ".\..\src\tabview.h"
#include ".\..\src\WordListDialog.h"
#include ".\..\src\TagList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#pragma warning (disable : 4100) // unreferenced formal parameter

CFont *TextWinFont;
UINT WainColumnTextFormat = ::RegisterClipboardFormat("WAIN column text format");
IMPLEMENT_DYNAMIC(MainFrame, CMDIFrameWnd)
UINT blob = RegisterClipboardFormat(CFSTR_SHELLIDLIST);
UINT blob2 = RegisterClipboardFormat(CFSTR_FILECONTENTS);
UINT CleanupThreadFunc(LPVOID rp);

BEGIN_MESSAGE_MAP(MainFrame, CMDIFrameWnd)
  ON_WM_WINDOWPOSCHANGED()
  ON_WM_CREATE()
  ON_WM_SIZE()
  ON_WM_CLOSE()
  ON_WM_TIMER()
  ON_WM_ACTIVATE()
  ON_WM_ACTIVATEAPP()
  ON_WM_DROPFILES()
  ON_COMMAND(ID_HELP_FINDER,               Help)
  ON_COMMAND(ID_HELP,                      Help)
  ON_COMMAND(ID_CONTEXT_HELP,              CMDIFrameWnd::OnContextHelp)
  ON_COMMAND(ID_DEFAULT_HELP,              CMDIFrameWnd::OnHelpFinder)
  ON_COMMAND_EX(ID_VIEW_STATUS_BAR,        OnBarCheck)
  ON_COMMAND_EX(ID_VIEW_TOOLBAR,           OnBarCheck)
  ON_COMMAND(IDM_SET_FONT,                 SelectFont)
  ON_COMMAND(IDM_SET_NAVIGATOR_FONT,       SelectNavigatorListFont)
  ON_COMMAND(IDM_VIEW_NAVIGATOR_BAR,       ViewNavigatorBar)
  ON_COMMAND(IDM_SEL_NAVIGATOR_BAR,        SelectNavigatorBar)
  ON_COMMAND(IDM_SEL_NAVIGATOR_LIST,       SelectNavigatorList)
  ON_COMMAND(IDM_NEXT_WIN,                 NextWin)
  ON_COMMAND(IDM_NEXT_WIN_HERE,            NextWinHere)
  ON_COMMAND(IDM_PREV_WIN_HERE,            PrevWinHere)
  ON_COMMAND(IDM_PREV_WIN,                 PrevWin)
  ON_COMMAND(IDM_SWAP_WIN,                 SwapWin)
  ON_COMMAND(IDM_OTHER_WIN,                OtherWin)
  ON_COMMAND(IDM_VIEW_DIR,                 ViewDir)
  ON_COMMAND(IDM_VIEW_TAGS,                ViewTags)
  ON_COMMAND(IDM_VIEW_CLASS,               ViewClass)
  ON_COMMAND(IDM_VIEW_PROJ,                ViewProject)
  ON_COMMAND(IDM_VIEW_CURR,                ViewCurrentTags)
  ON_COMMAND(IDM_VIEW_FILES,               ViewFiles)
  ON_COMMAND(IDM_TOOL_SETUP,               ToolSetup)
  ON_COMMAND(IDM_MACRO_SAVE,               SaveMacro)
  ON_COMMAND(IDM_MACRO_LOAD,               LoadMacro)
  ON_COMMAND_RANGE(IDM_RUN_TOOL_0, IDM_RUN_TOOL_19, RunTool)
  ON_COMMAND_RANGE(IDM_RUN_MACRO_0, IDM_RUN_MACRO_LAST, RunMacro)
  ON_COMMAND_RANGE(IDM_ASSOC_ITEM0, IDM_ASSOC_ITEM9, DoAssocList)
  ON_COMMAND(IDM_DOC_PROP_SETUP,           DocumentPropertiesSetup)
  ON_COMMAND(IDM_PROJ_SETUP,               ProjectSetup)
  ON_COMMAND(IDM_PROJ_MAKE,                ProjectMake)
  ON_COMMAND(IDM_PROJ_MAKE_0,              ProjectMake0)
  ON_COMMAND(IDM_PROJ_MAKE_1,              ProjectMake1)
  ON_COMMAND(IDM_PROJ_MAKE_2,              ProjectMake2)
  ON_COMMAND(IDM_PROJ_MAKE_2,              ProjectMake3)
  // ON_COMMAND(IDM_PROJ_MAKE_ALL,            ProjectMakeAll)
  ON_COMMAND(IDM_PROJ_EXE,                 ProjectExecute)
  ON_COMMAND(IDM_PROJ_EXE_1,                ProjectExecute1)
  ON_COMMAND(IDM_PROJ_EXE_2,                ProjectExecute2)
  ON_COMMAND(IDM_PROJ_EXE_3,                ProjectExecute3)
  ON_COMMAND(IDM_PROJ_EXE_4,                ProjectExecute4)
  ON_COMMAND(IDM_PROJ_OPEN,                ProjectOpen)
  ON_COMMAND(IDM_PROJ_SAVE,                ProjectSave)
  ON_COMMAND(IDM_PROJ_ADD_CUR_FILE,        ProjectAddCurrentFile)
  ON_COMMAND(IDM_PROJ_MANAGE,              ProjectManage)
  ON_COMMAND(IDM_KEY_SETUP,                KeyboardSetup)
  ON_COMMAND(IDM_CLOSE_FILE,               CloseFile)
  ON_COMMAND(IDM_FILE_OPEN,                OpenFile)
  ON_COMMAND(IDM_FILE_NEW,                 NewFile)
  ON_COMMAND(IDM_APP_EXIT,                 Exit)
  ON_COMMAND(IDM_USER_MENU_SETUP,          UserMenuSetup)
  ON_COMMAND_RANGE(IDM_SET_BOOKMARK_0, IDM_SET_BOOKMARK_9, SetBookmark)
  ON_COMMAND_RANGE(IDM_JUMP_TO_BOOKMARK_0, IDM_JUMP_TO_BOOKMARK_9, JumpToBookmark)
  ON_COMMAND(IDM_BOOKMARK_LIST, BookmarkList)
  ON_COMMAND(IDM_UNDO_JUMP_TO_TAG, UndoJumpToTag)
  ON_COMMAND(IDM_REDO_LAST_TAG,            RedoTag)

  ON_REGISTERED_MESSAGE(TheOtherAppMessage, DoOpenFile)
  ON_COMMAND_RANGE(IDM_RECENT_PROJ0, IDM_RECENT_PROJ9, OpenRecentProject)
  ON_COMMAND(IDM_MACRO_SETUP, MacroSetup)
  ON_COMMAND(IDM_SEARCH,      Search)
  ON_COMMAND(IDM_REPLACE,     Replace)
  ON_COMMAND(IDM_PROJECT_REPLACE,     ProjectReplace)
  ON_COMMAND(IDM_GLOBAL_REPLACE,      GlobalReplace)
  ON_COMMAND(IDM_GOTO_OTHER_VIEW,     GotoOtherView)
  ON_COMMAND(IDM_TOOL_OUTPUT_NEXT,    ToolOutputNext)
  ON_COMMAND(IDM_TOOL_OUTPUT_PREV,    ToolOutputPrev)
  ON_COMMAND(IDM_ASSOC_LIST,          AssocList)
  ON_COMMAND(IDM_POPUP_MENU_SETUP,    PopupMenuSetup)
  ON_COMMAND(IDM_CURR_DOC_PROP_SETUP, DocPropSetup)
  ON_REGISTERED_MESSAGE(search_message_id, SearchFunc)
  ON_REGISTERED_MESSAGE(ReplaceMessageId, GlobalReplaceFunc)
  // The line below is to catch the "More Windows ..." message
  // Instead of using the silly tiny standard box, simply go to NavigatorBar in files mode
  ON_COMMAND(AFX_IDM_FIRST_MDICHILD + 9, ViewFiles)
  ON_COMMAND(IDM_SAVE_ALL_DOC,           DoSaveAllDocs)
  ON_COMMAND(IDM_LAST_AUTO_TAG,          JumpLastAutoTag)
  ON_WM_MOUSEWHEEL()
  ON_COMMAND(IDL_COLOR_SETUP,            ColorSetup)

  ON_WM_MENUSELECT()
  ON_WM_INITMENUPOPUP()
  ON_WM_DRAWITEM()
  ON_WM_MEASUREITEM()
  ON_COMMAND(IDM_TAB_VIEW_COLOR, OnColor)
  ON_COMMAND(IDM_SEND_TOOL_CMD, SendToolCmd)
END_MESSAGE_MAP();

// These MUST match StatusIndexType in mainfrm.h
static UINT indicators[] =
{
  ID_SEPARATOR,           // status line indicator
  ID_SB_STATUS_TEXT,
  ID_INDICATOR_CAPS,
  ID_SB_READ_ONLY,
  ID_SB_SCROLL_LOCK,
  ID_SB_MACRO_REC,
  ID_SB_INS_STATUS,
  ID_SB_MARK_STATUS,
  ID_SB_SPECIAL_MODE,
  ID_SB_MOD_STATUS,
  ID_SB_CRLF_STATUS,
  ID_SB_LINE_STATUS
};

MainFrame::MainFrame()
{
   m_navigatorBar.m_enabled = FALSE;
   m_navigatorBar.m_init = FALSE;

   int i;
   TextWinFont = new CFont();
   TextWinFont->CreateFontIndirect(&wainApp.gs.m_textWinLogFont);

   m_statusBarTimer = 0;
   m_tools = new ToolClass(this);

   for(i = 0; i < NOF_BOOKMARKS; i++)
      m_bookmark[i].m_view = 0;
   m_updateList = TRUE;
   m_nofAccEntrys = 0;
   m_accHandle = 0;
   m_isInit = FALSE;
   m_doCheckFileStatus = FALSE;
   m_fileCheckTimer = 0;
   m_searchDialog = NULL;
   m_replaceDialog = NULL;
   m_cleanupThread = NULL;
   m_fileCheckThread = NULL;
   m_isContextMenu = FALSE;
   m_firstProjectReplace = false;
   m_firstGlobalReplace = false;
}

MainFrame::~MainFrame()
{
  delete TextWinFont;
  delete m_tools;
  if(m_accHandle)
    DestroyAcceleratorTable(m_accHandle);

  if(m_searchDialog)
    delete m_searchDialog;
  if(m_replaceDialog)
    delete m_replaceDialog;
  if(m_cleanupThread)
  {
    DWORD exit_code = ~STILL_ACTIVE;
    ::GetExitCodeThread(m_cleanupThread, &exit_code);
    if(exit_code == STILL_ACTIVE)
      m_cleanupThread->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);

    time_t time_now;
    time(&time_now);
    struct tm *tm = gmtime(&time_now);

    wainApp.gs.m_cleanupYear = tm->tm_year;
    wainApp.gs.m_cleanupMonth = tm->tm_mon;
    wainApp.gs.m_cleanupDay = tm->tm_mday;

    while(exit_code == STILL_ACTIVE)
    {
      Sleep(100);
      ::GetExitCodeThread(m_cleanupThread, &exit_code);
    }

    delete m_cleanupThread;
  }
  if(m_fileCheckThread)
    m_fileCheckThread->PostThreadMessage(WM_QUIT, 0, 0);
}

void MainFrame::OnClose(void)
{
  if(m_navigatorBar.m_init)
  {
    ((NavigatorDialog *)m_navigatorBar.m_myDialog)->Close();
    if(m_navigatorBar.m_enabled)
    {
      if(m_navigatorBar.m_dockBarID == AFX_IDW_DOCKBAR_RIGHT)
        wainApp.gs.m_viewNavigatorBar = AFX_IDW_DOCKBAR_RIGHT;
      else
        wainApp.gs.m_viewNavigatorBar = AFX_IDW_DOCKBAR_LEFT;
    }
    else
      wainApp.gs.m_viewNavigatorBar = 0;
  }
  if(m_pageBar.m_init)
    m_pageBarDialog.OnClose();
  WINDOWPLACEMENT place;
  if(GetWindowPlacement(&place))
  {
    if(place.showCmd == SW_SHOWMAXIMIZED)
    {
      wainApp.gs.m_winMode = 2;
    }
    else if(place.showCmd == SW_SHOWNORMAL)
    {
      wainApp.gs.m_winMode = 1;
      wainApp.gs.m_winX = place.rcNormalPosition.left;
      wainApp.gs.m_winY = place.rcNormalPosition.top;
      wainApp.gs.m_winCx = place.rcNormalPosition.right - place.rcNormalPosition.left;
      wainApp.gs.m_winCy = place.rcNormalPosition.bottom - place.rcNormalPosition.top;
    }
  }
  ViewListElem *elem = NULL;
  int i;
  char n_str[20];
  ChildFrame *acf = (ChildFrame *)MDIGetActive();
  if(acf && acf->GetView() && acf->GetView()->GetDocument()->m_isDebugFile)
    acf = NULL;
  for(i = acf ? 1 : 0; ((elem = m_viewList.GetRankNext(elem)) != NULL) && i < 50;)
  {
    if(!elem->m_myView->GetDocument()->m_isDebugFile)
    {
      if(elem->m_myView->GetDocument()->GetModified())
      {
        std::string Msg = "The file: ";
        Msg += elem->m_name;
        Msg += "\r\nHas been changed, do you want to save it?";
        if(WainMessageBox(this, Msg.c_str(), IDC_MSG_YES | IDC_MSG_NO, IDI_QUESTION_ICO) == IDC_MSG_YES)
        {
          elem->m_myView->GetDocument()->SaveFile();
        }
        elem->m_myView->GetDocument()->SetModified(FALSE);
      }
      if(!elem->m_myView->GetDocument()->m_newFile && !elem->m_ftpFile)
      {
        if(acf && acf->GetView() == elem->m_myView)
        {
          sprintf(n_str, "Nr: %d", 0);
          wainApp.WriteProfileString("Last Files", n_str, elem->m_name.c_str());
          acf = NULL;
        }
        else
        {
          sprintf(n_str, "Nr: %d", i++);
          wainApp.WriteProfileString("Last Files", n_str, elem->m_name.c_str());
        }
      }
    }
  }
  if(acf)
  {
    sprintf(n_str, "Nr: %d", i - 1);
    CString last = wainApp.GetProfileString("Last Files", n_str, "");
    sprintf(n_str, "Nr: %d", 0);
    wainApp.WriteProfileString("Last Files", n_str, last);
    i--;
  }
  wainApp.WriteProfileInt("Last files", "Nof files", i);
  CMDIFrameWnd::OnClose();
}

int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
     return -1;

   DWORD flags;
   if(wainApp.gs.m_viewToolBar)
     flags = WS_CHILD | CBRS_TOP | WS_VISIBLE;
   else
     flags = WS_CHILD | CBRS_TOP;

   if (!m_toolBar.Create(this, flags) || !m_toolBar.LoadToolBar(IDR_MAINFRAME))
   {
     TRACE0("Failed to create toolbar\n");
     return -1;      // fail to create
   }
   if(wainApp.gs.m_viewStatusBar)
     flags = WS_CHILD | CBRS_BOTTOM | WS_VISIBLE;
   else
     flags = WS_CHILD | CBRS_BOTTOM;
   if (!m_statusBar.Create(this, flags) || !m_statusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
   {
     TRACE0("Failed to create status bar\n");
     return -1;      // fail to create
   }
   EnableDocking(CBRS_ALIGN_ANY);

 //  StatusBar.SetPaneInfo(0, ID_SEPARATOR, SBPS_NOBORDERS, 250);
   m_statusBar.SetPaneInfo(0, ID_SEPARATOR, 0, 250);
   m_statusBar.SetPaneInfo(int(StatusIndexType::DUMMY2), ID_SB_STATUS_TEXT, SBPS_STRETCH, 200);

   m_toolBar.SetBarStyle(m_toolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
   m_toolBar.EnableDocking(CBRS_ALIGN_ANY);
   DockControlBar(&m_toolBar);

   CString dummy = "PageBar";
   m_pageBar.Create(this, &m_pageBarDialog, dummy, IDD_PAGE_BAR_DIALOG, WS_CHILD | WS_VISIBLE | CBRS_BORDER_TOP);
   // PageBar.SetBarStyle(PageBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_ALIGN_BOTTOM | CBRS_SIZE_DYNAMIC);
   m_pageBar.SetBarStyle(m_pageBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_ALIGN_BOTTOM | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP);
   // PageBar.EnableDocking(CBRS_ALIGN_BOTTOM);
   // DockControlBar(&PageBar, AFX_IDW_DOCKBAR_BOTTOM);
   ShowControlBar(&m_pageBar, FALSE, FALSE);
   m_pageBar.SetWindowText("Pages");

   SetMarkStatus(FALSE);
   m_statusBar.SetPaneText(int(StatusIndexType::REC), "            ");
   m_keyboardSetupFile = wainApp.GetProfileString("Settings", "KeyFile", "");
   SetSpecialModeStatus(NULL);
   m_tools->ReadToolFile();
   if(m_keyboardSetupFile.IsEmpty() || access(m_keyboardSetupFile, 4))
   {
     m_keyboardSetupFile = wainApp.gs.m_configPath;
     m_keyboardSetupFile += "keysetup.key";
   }
   if(ReadKeySetupFile(m_keyboardSetupFile, (ACCEL *)&m_accEntry, &m_nofAccEntrys))
     m_accHandle = CreateAcceleratorTable((ACCEL *)&m_accEntry, m_nofAccEntrys);
   SetupMenu(TRUE);
   m_isInit = TRUE;
   MakePopupMenuStrings(wainApp.gs.m_popupMenu);

   time_t time_now;
   time(&time_now);
   struct tm Tm;
   memset(&Tm, 0, sizeof(Tm));
   Tm.tm_year = wainApp.gs.m_cleanupYear;
   Tm.tm_mon = wainApp.gs.m_cleanupMonth;
   Tm.tm_mday = wainApp.gs.m_cleanupDay;
   time_t LastTime = mktime(&Tm);

   double diff = difftime(time_now, LastTime);

   if(diff > 24*60*60)
   { /* More than 1 day since last check, so run the temp path cleanup thread */
      char *path = strdup(wainApp.gs.m_tempPath.c_str());
      m_cleanupThread = AfxBeginThread(CleanupThreadFunc, path, THREAD_PRIORITY_LOWEST, 0, CREATE_SUSPENDED);
      m_cleanupThread->m_bAutoDelete = FALSE;
      m_cleanupThread->ResumeThread();
   }

   m_fileCheckThread = AfxBeginThread(FileCheckThreadFunc, NULL, THREAD_PRIORITY_NORMAL, 0, 0);
   return 0;
}

BOOL MainFrame::PreCreateWindow(CREATESTRUCT &cs)
{
  cs.lpszClass = wainApp.m_uniqueClassName;
  if(wainApp.gs.m_winMode == 2)
    cs.style |= WS_MAXIMIZE;
  else if(wainApp.gs.m_winMode == 1)
  {
    cs.x = wainApp.gs.m_winX;
    cs.y = wainApp.gs.m_winY;
    cs.cx = wainApp.gs.m_winCx;
    cs.cy = wainApp.gs.m_winCy;
  }
  return CMDIFrameWnd::PreCreateWindow(cs);
}

BOOL MainFrame::PreTranslateMessage(MSG *msg)
{
  if(IsWindow(m_pageBar.m_hWnd))
  {
    WainView *av = m_pageBarDialog.GetActiveView();
    if(av && GetFocus() == av)
      if(av->PreTranslateMessage(msg))
        return TRUE;
  }

  if(m_accHandle && TranslateAccelerator(m_hWnd, m_accHandle, msg))
    return TRUE;

  return CMDIFrameWnd::PreTranslateMessage(msg);
}

BOOL MainFrame::OnCommand(WPARAM wparm, LPARAM lparm)
{
  // If there is a view open in the pagebar and the view has focus, we have to route the command to it.
  WainView *av = m_pageBarDialog.GetActiveView();
  if(av && GetFocus() == av)
    if(AfxCallWndProc(av, av->m_hWnd, WM_COMMAND, wparm, lparm) != 0)
      return TRUE; // handled by the view
  return CMDIFrameWnd::OnCommand(wparm, lparm);
}

void MainFrame::OnActivate(UINT state, CWnd *wnd_other, BOOL minimized)
{
  CMDIFrameWnd::OnActivate(state, wnd_other, minimized);
}

void MainFrame::OnActivateApp(BOOL active, DWORD task)
{
  if(active)
  {
    m_fileCheckTimer = SetTimer(ID_FILE_CHECK_TIMER, 2000, NULL);
    m_doCheckFileStatus = TRUE;
    if(m_navigatorBar.m_enabled)
      m_navigatorDialog.Activate();
    if(m_tools)
      m_tools->Activate();
  }
  else
  {
    KillTimer(m_fileCheckTimer);
    m_fileCheckTimer = 0;
    m_navigatorDialog.DeActivate();
    if(m_tools)
      m_tools->DeActivate();
  }
}

void MainFrame::SelectFont(void)
{
  CClientDC dc(NULL);

  LOGFONT lf = wainApp.gs.m_textWinLogFont;
  CFontDialog dlg(&lf);
  dlg.m_cf.Flags |= CF_ANSIONLY | CF_FORCEFONTEXIST | CF_FIXEDPITCHONLY;
  dlg.m_cf.Flags &= ~CF_EFFECTS;
  if (dlg.DoModal() == IDOK)
  {
    wainApp.gs.m_textWinLogFont = lf;
    int i = 0;

    TextWinFont->Detach();
    TextWinFont->CreateFontIndirect(&wainApp.gs.m_textWinLogFont);

    ViewListElem *elem = NULL;
    while((elem = m_viewList.GetRankNext(elem)) != NULL)
    {
      m_updateList = FALSE;
      elem->m_myView->UpdateFont();
    }
    WainView *av;
    while((av = ((PageBarDialogClass *)m_pageBar.m_myDialog)->GetView(i++)) != NULL)
      av->UpdateFont();
  }
  m_updateList = TRUE;
}

void MainFrame::SelectNavigatorListFont(void)
{
  CClientDC dc(NULL);

  LOGFONT lf = wainApp.gs.m_navigatorListLogFont;
  // lf.lfHeight = -::MulDiv(-lf.lfHeight, dc.GetDeviceCaps(LOGPIXELSY), 72);
  CFontDialog dlg(&lf);
  dlg.m_cf.Flags |= CF_ANSIONLY | CF_FORCEFONTEXIST;
  dlg.m_cf.Flags &= ~CF_EFFECTS;
  if (dlg.DoModal() == IDOK)
  {
    // lf.lfHeight = -::MulDiv(-lf.lfHeight, 72, dc.GetDeviceCaps(LOGPIXELSY));
    wainApp.gs.m_navigatorListLogFont = lf;
    m_navigatorDialog.SetNewFont();
  }
}

void MainFrame::OnSize(UINT type, int cx, int cy)
{
  if(type != SIZE_MINIMIZED)
    RecalcLayout();
  RecalcChildPos();
  RECT cr;
  GetClientRect(&cr);
  if(m_navigatorBar.m_init && m_navigatorBar.m_enabled)
  {
    RECT br;
    m_navigatorBar.GetWindowRect(&br);
    m_navigatorBar.SetWindowPos(NULL, 0, 0, br.right - br.left, cr.bottom - cr.top, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
  }
}

void MainFrame::OnWindowPosChanged(WINDOWPOS FAR *window_pos)
{
  CMDIFrameWnd::OnWindowPosChanged(window_pos);
}

void MainFrame::SetLineStatus(class WainView *aView, int aLineNo, int aColumn)
{
   if(GetActiveView() == aView)
   {
      wainApp.m_lineNo = aLineNo + 1;
      wainApp.m_columnNo = aColumn + 1;
   }
}

void MainFrame::SetMarkStatus(bool on)
{
   wainApp.m_markStatus = on ? 0x8001 : 0x8000;
}

void MainFrame::SetCrLfStatus(BOOL unix)
{
  if(unix)
    m_statusBar.SetPaneText(int(StatusIndexType::CRLF), "Unix");
  else
    m_statusBar.SetPaneText(int(StatusIndexType::CRLF), "DOS");
}

void MainFrame::SetReadOnlyStatus(bool aReadOnly)
{
   m_statusBar.SetPaneText(int(StatusIndexType::READONLY), aReadOnly ? "RO" : "");
}

void MainFrame::SetInsertStatus(bool on)
{
  if(on)
    m_statusBar.SetPaneText(int(StatusIndexType::INS), "INS");
  else
    m_statusBar.SetPaneText(int(StatusIndexType::INS), "OVR");
}

void MainFrame::SetSpecialModeStatus(const char *str)
{
  if(str)
    m_statusBar.SetPaneText(int(StatusIndexType::SPECIAL_MODE), str);
  else
    m_statusBar.SetPaneText(int(StatusIndexType::SPECIAL_MODE), "");
}

void MainFrame::SetModifiedStatus(WainView *aView, bool aOn)
{
   if(aView != GetActiveView())
      return;
   if(aOn)
      m_statusBar.SetPaneText(int(StatusIndexType::MODIFIED), "MOD");
   else
      m_statusBar.SetPaneText(int(StatusIndexType::MODIFIED), "        ");
}

void MainFrame::SetMacroRecStatus(BOOL on)
{
   m_statusBar.SetPaneText(int(StatusIndexType::REC), on ? "REC": "             ");
}

void MainFrame::SetScrollLock_status(BOOL on)
{
  m_statusBar.SetPaneText(int(StatusIndexType::SCROLLLOCK), on ? "ScrollLock": "                    ");
}

void MainFrame::SetStatusText(const char *text)
{
  if(IsWindow(m_statusBar.m_hWnd))
  {
    m_statusBar.SetPaneText(int(StatusIndexType::DUMMY2), text);
    if(m_statusBarTimer)
      KillTimer(m_statusBarTimer);
    m_statusBarTimer = SetTimer(IDM_STATUS_BAR_TIMER, 10000, NULL);
  }
}

void SetStatusText(const char *msg, ...)
{
  char text[1024];
  va_list args;
  va_start(args, msg);
  vsprintf(text, msg, args);
  va_end(args);
  MainFrame *mf = GetMf();
  if(mf && mf->m_hWnd)
    mf->SetStatusText(text);
}

void MainFrame::RecalcChildPos(bool aForce)
{
   if(wainApp.gs.m_createMode == CreateModeType::CREATE_DUAL || aForce)
   {
      RECT rect;
      ChildFrame *frame;
      ViewListElem *elem = NULL;
      if(aForce)
      {
         wainApp.gs.m_createMode = CreateModeType::CREATE_DUAL;
         while((elem = m_viewList.GetRankNext(elem)) != NULL)
         {
            frame = elem->m_myView->GetDocument()->m_childFrame;
            if(frame->GetDisplayMode() == CreateModeType::CREATE_MAX)
               frame->ShowWindow(SW_RESTORE);
            frame->SetDisplayMode(CreateModeType::CREATE_DUAL);
         }
         elem = NULL;
      }
      while((elem = m_viewList.GetRankNext(elem)) != NULL)
      {
         WainDoc *doc = elem->m_myView->GetDocument();
         if(!doc)
         {
            WainMessageBox(this, "No doc", IDC_MSG_OK, IDI_ERROR_ICO);
            return;
         }
         frame = doc->m_childFrame;
         if(!frame)
         {
            WainMessageBox(this, "No frame", IDC_MSG_OK, IDI_ERROR_ICO);
            return;
         }
         CalcWinRect(&rect, frame->GetPosition());
         frame->MoveWindow(&rect);
      }
   }
}

void MainFrame::ViewNavigatorBar(void)
{
  if(!m_navigatorBar.m_enabled)
  {
    m_navigatorBar.m_enabled = TRUE;
    m_navigatorBar.m_init = TRUE;
    if(::IsWindow(m_navigatorBar))
    {
      ShowControlBar(&m_navigatorBar, TRUE, FALSE);
    }
    else
    {
      CString dummy;
      dummy = "Navigator Bar";
       if(!m_navigatorBar.Create(this, &m_navigatorDialog, dummy, IDD_NAVIGATOR_DIALOG))
      {
        TRACE0("Failed to create dialogbar\n");
        return;      // fail to create
      }
      m_navigatorBar.SetBarStyle(m_navigatorBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
      // NavigatorBar.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
      // if(wainApp.gs.ViewNavigatorBar == AFX_IDW_DOCKBAR_RIGHT)
      //   DockControlBar(&NavigatorBar, AFX_IDW_DOCKBAR_RIGHT);
      // else
      //   DockControlBar(&NavigatorBar, AFX_IDW_DOCKBAR_LEFT);
    }
    CMenu *menu = GetMenu();
    if(menu)
      menu->CheckMenuItem(IDM_VIEW_NAVIGATOR_BAR, MF_CHECKED);
  }
  else
  {
    m_navigatorBar.m_enabled = FALSE;
    ShowControlBar(&m_navigatorBar, FALSE, FALSE);
    CMenu *menu = GetMenu();
    if(menu)
      menu->CheckMenuItem(IDM_VIEW_NAVIGATOR_BAR, MF_UNCHECKED);
  }
  RecalcChildPos();
}

void MainFrame::MyGetClientRect(RECT *rect)
/* Calculate the area avalable for documents, that is, client area - what is needed for toolbars */
{
   RECT cr;
   int bar_height = 0;
   CMDIFrameWnd::GetClientRect(&cr);
   rect->top = 0;
   rect->left = 0;
   RECT sr = {0, 0, 0, 0}, tr = {0, 0, 0, 0}, pr = {0, 0, 0, 0}, ctr = {0, 0, 0, 0};

   if(::IsWindow(m_statusBar) && m_statusBar.IsWindowEnabled() && m_statusBar.IsWindowVisible())
      m_statusBar.GetWindowRect(&sr);

   if(::IsWindow(m_toolBar) && m_toolBar.IsWindowEnabled() && !m_toolBar.IsFloating() && m_toolBar.IsWindowVisible())
   {
      m_toolBar.GetWindowRect(&tr);
      bar_height = -2;
   }
   if(::IsWindow(m_pageBar) && m_pageBar.IsWindowEnabled() && !m_pageBar.IsFloating() && m_pageBar.IsWindowVisible())
   {
      m_pageBar.GetWindowRect(&pr);
      pr.bottom -= 2;
   }
   bar_height += (tr.bottom - tr.top) + (sr.bottom - sr.top) + (pr.bottom - pr.top) + (ctr.bottom - ctr.top);
   if(::IsWindow(m_navigatorBar) && m_navigatorBar.IsWindowEnabled() && !m_navigatorBar.IsFloating() && m_navigatorBar.m_enabled)
   {
      RECT dr;
      m_navigatorBar.GetClientRect(&dr);
      if(m_navigatorBar.IsHorz())
      {
        rect->right = cr.right;
        rect->bottom = (cr.bottom - dr.bottom - bar_height);
      }
      else
      {
        rect->right = cr.right - dr.right - 4;
        rect->bottom = (cr.bottom - bar_height);
      }
   }
   else
   {
      rect->right = cr.right;
      rect->bottom = (cr.bottom - bar_height);
   }
   int border_size = GetSystemMetrics(SM_CYEDGE);
   rect->right -= 2*border_size;
   border_size = GetSystemMetrics(SM_CXEDGE);
   rect->bottom -= 2*border_size;
}

void MainFrame::CalcWinRect(RECT *aRect, int aPart)
{
   MyGetClientRect(aRect);
   aRect->bottom /= 2;
   if(aPart)
   {
      aRect->top = aRect->bottom;
      aRect->bottom *= 2;
   }
}

int MainFrame::GetNewWinRect(RECT *rect)
{
  int pos = 0;
  if(m_viewList.m_currentView != &m_viewList.m_list)
  {
    pos = m_viewList.m_currentView->m_position ? 0 : 1;
  }
  CalcWinRect(rect, pos);
  return pos;
}

// void MainFrame::OnUpdateFrameTitle(BOOL aAddToTitle)
// {
//   SetWindowText("Wain");
//}

void MainFrame::ActivateWin(int nr)
{
  if(nr >= 0)
  {
    ViewListElem *elem;
    elem = m_viewList.GetViewNr(nr);
    if(elem && elem->m_myView && elem->m_myView->GetDocument() && elem->m_myView->GetDocument()->m_childFrame)
    {
      elem->m_myView->GetDocument()->m_childFrame->MDIActivate();
      SetActiveView(elem->m_myView);
      elem->m_myView->SetFocus();
    }
  }
}

void MainFrame::SelectNavigatorBar(void)
{
  if(m_navigatorBar.m_enabled)
  {
    ::SetActiveWindow(m_navigatorDialog.m_hWnd);
    ::SetFocus(m_navigatorDialog.m_hWnd);
  }
}

void MainFrame::SelectNavigatorList(void)
{
   if(m_navigatorBar.m_enabled)
   {
    if(m_navigatorDialog.m_navBarState == NavBarState::Class)
    {
      if(m_navigatorDialog.m_classViewTree)
      {
        ::SetActiveWindow(m_navigatorDialog.m_classViewTree->m_hWnd);
        ::SetFocus(m_navigatorDialog.m_classViewTree->m_hWnd);
      }
    }
    else
    {
      ::SetActiveWindow(m_navigatorDialog.m_navigatorList->m_hWnd);

      ::SetFocus(m_navigatorDialog.m_navigatorList->m_hWnd);
   }
}
}


void MainFrame::ToolOutputNext(void)
{
  WainView *av = m_pageBarDialog.GetActiveView();
  if(av)
  {
    av->ToolOutputNext();
  }
}

void MainFrame::ToolOutputPrev(void)
{
  WainView *av = m_pageBarDialog.GetActiveView();
  if(av)
  {
    av->ToolOutputPrev();
  }
}

void MainFrame::PutInTopOfStack(int pos, int WinNr)
{
  if(m_updateList)
    m_viewList.PutInRankTop(WinNr);
  else
  {
    ViewListElem *elem = m_viewList.GetViewNr(WinNr);
    ASSERT(elem);
    ASSERT(elem->m_position < 2);
    m_viewList.m_topView[elem->m_position] = elem;
    m_viewList.m_currentView = elem;
  }
  m_updateList = TRUE;
  m_navigatorDialog.m_fileList.UpdateFileList();
}

void MainFrame::SetFileName(const char *file_name, int PropIndex)
{
  if(m_navigatorBar.m_enabled && m_navigatorBar.m_init)
  {
    m_navigatorDialog.SetFileName(file_name, PropIndex);
  }
}

void MainFrame::ViewDir(void)
{
  if(!m_navigatorBar.m_enabled)
    ViewNavigatorBar();
  m_navigatorDialog.ViewDir();
  m_navigatorDialog.m_navigatorList->SetFocus();
}

void MainFrame::OpenDirForFile(const char* _filename)
{
   if(!m_navigatorBar.m_enabled)
      ViewNavigatorBar();
   m_navigatorDialog.OpenDirForFile(_filename);
   m_navigatorDialog.ViewDir();
   m_navigatorDialog.m_navigatorList->SetFocus();
}

void MainFrame::OpenProjectForFile(const char* _filename)
{
   if(!m_navigatorBar.m_enabled)
      ViewNavigatorBar();
   m_navigatorDialog.OpenProjectForFile(_filename);
   m_navigatorDialog.ViewProject();
   m_navigatorDialog.m_projectTree->SetFocus();
}

void MainFrame::ViewFiles(void)
{
  if(!m_navigatorBar.m_enabled)
    ViewNavigatorBar();
  m_navigatorDialog.ViewFiles();
  m_navigatorDialog.m_navigatorList->SetFocus();
}

void MainFrame::ViewTags(void)
{
  if(!m_navigatorBar.m_enabled)
    ViewNavigatorBar();
  m_navigatorDialog.ViewTags();
  m_navigatorDialog.m_navigatorList->SetFocus();
}

void MainFrame::ViewProject(void)
{
  if(!m_navigatorBar.m_enabled)
    ViewNavigatorBar();
  m_navigatorDialog.ViewProject();
  m_navigatorDialog.m_navigatorList->SetFocus();
}

void MainFrame::ViewCurrentTags(void)
{
  if(!m_navigatorBar.m_enabled)
    ViewNavigatorBar();
  m_navigatorDialog.ViewCurrentTags();
  m_navigatorDialog.m_navigatorList->SetFocus();
}

void MainFrame::ViewClass(void)
{
  std::string Word = "";
  GetCurrentWord(Word);

  if(!m_navigatorBar.m_enabled)
    ViewNavigatorBar();

  m_navigatorDialog.ViewClass(Word);
}

void MainFrame::OnTimer(UINT aTimerId)
{
  if(aTimerId == m_tools->m_timerId)
  {
    m_tools->OnTimer();
  }
  else if(aTimerId == m_statusBarTimer)
  {
    KillTimer(m_statusBarTimer);
    m_statusBarTimer = 0;
    m_statusBar.SetPaneText(int(StatusIndexType::DUMMY2), "");
  }
  else if(aTimerId == m_fileCheckTimer)
  { // DoCheckFileStatus is polled in OnIdle(), which will call CheckFileStatus()
    m_doCheckFileStatus = TRUE;
  }
  else
  {
    char msg[256];
    sprintf(msg, "Wrong timer: %d", aTimerId);
    WainMessageBox(this, msg, IDC_MSG_OK, IDI_ERROR_ICO);
  }
}

void MainFrame::DocumentPropertiesSetup(void)
{
  LangSelectDialogClass ls(this);
  ls.DoModal();
  ViewListElem *elem = NULL;
  while((elem = m_viewList.GetRankNext(elem)) != NULL)
  {
    elem->m_myView->GetDocument()->GetExtType(elem->m_myView->GetDocument()->GetPathName(), TRUE);
  }
}

LRESULT MainFrame::DoOpenFile(WPARAM aFileNameAtom, LPARAM aFileLineAtom)
{
   char Str[MAX_PATH];
   if(GlobalGetAtomName((ATOM )aFileNameAtom, Str, MAX_PATH))
   {
      char ext[MAX_PATH];
      MySplitPath(Str, SP_EXT, ext);
      if(!stricmp(ext, wainApp.gs.m_projectExtension.c_str()))
      {
         m_navigatorDialog.m_project->SetProjectFile(Str);
      }
      else
      {
         CString dest;
         WainDoc *doc;
         if(m_navigatorDialog.m_project->CheckFileName(dest, Str))
         {
            doc = wainApp.OpenDocument(dest);
         }
         else
         {
            doc = wainApp.OpenDocument(Str);
         }
         GlobalDeleteAtom((ATOM )aFileNameAtom);
         if(GlobalGetAtomName((ATOM )aFileLineAtom, Str, MAX_PATH))
         {
            int LineNo = strtol(Str, 0, 10);
            if(doc && LineNo != -1)
            {
               doc->m_view->GotoLineNo(LineNo - 1);
            }
            GlobalDeleteAtom((ATOM )aFileLineAtom);
         }
      }
   }
   else
      ::MessageBox(m_hWnd, "Wrong ATOM", "Error", MB_OK);
   return 0;
}

void MainFrame::GetCurrentWord(std::string &aWord)
{
  WainView *av = GetActiveView();
  if(av)
  {
    av->GetCurrentWord(aWord);
    return;
  }
  aWord = "";
}

void MainFrame::GetCurrentLine(std::string& _line)
{
   WainView *av = GetActiveView();
   if(av)
   {
      av->GetCurrentLine(_line);
      return;
   }
   _line = "";
}

void MainFrame::ProjectSetup(void)
{
  m_navigatorDialog.m_project->Setup();
}

void MainFrame::ProjectMake(void)
{
   m_navigatorDialog.m_project->Make(-1);
}

void MainFrame::ProjectMake0(void)
{
   m_navigatorDialog.m_project->Make(0);
}

void MainFrame::ProjectMake1(void)
{
   m_navigatorDialog.m_project->Make(1);
}

void MainFrame::ProjectMake2(void)
{
   m_navigatorDialog.m_project->Make(2);
}

void MainFrame::ProjectMake3(void)
{
   m_navigatorDialog.m_project->Make(3);
}

void MainFrame::ProjectMakeAll(void)
{
  // NavigatorDialog.Project->MakeAll();
}

void MainFrame::ProjectExecute(void)
{
  m_navigatorDialog.m_project->Execute(-1);
}
void MainFrame::ProjectExecute1(void)
{
  m_navigatorDialog.m_project->Execute(0);
}
void MainFrame::ProjectExecute2(void)
{
  m_navigatorDialog.m_project->Execute(1);
}
void MainFrame::ProjectExecute3(void)
{
  m_navigatorDialog.m_project->Execute(2);
}
void MainFrame::ProjectExecute4(void)
{
  m_navigatorDialog.m_project->Execute(3);
}

void MainFrame::ProjectOpen(void)
{
  m_navigatorDialog.m_project->Open();
}

void MainFrame::ProjectSave(void)
{
  m_navigatorDialog.m_project->Save();
}

void MainFrame::ProjectAddCurrentFile(void)
{
   ChildFrame *cf = (ChildFrame *)MDIGetActive();
   if(cf)
   {
      m_navigatorDialog.m_project->AddFile((const char *)cf->GetDocument()->GetPathName());
   }
}

void MainFrame::ProjectManage(void)
{
  m_navigatorDialog.m_project->Manage();
}

void MainFrame::DoSaveAllDocs(void)
{
  SaveAllDocs();
}

void MainFrame::SaveAllDocs(void)
{
  ViewListElem *elem = NULL;
  while((elem = m_viewList.GetRankNext(elem)) != NULL)
  {
    m_updateList = FALSE;
    elem->m_myView->GetDocument()->SaveFile(FALSE);
  }
  m_updateList = TRUE;
}

void MainFrame::RemoveFile(const char *name, BOOL do_close)
// Close the file with name.
// If it is a db-view and do_close is TRUE and there is no more db-view's close the pagebar.
{
  WainDoc *doc;
  ViewListElem *elem = NULL;
  while((elem = m_viewList.GetRankNext(elem)) != NULL)
  {
    if(elem->m_name == name)
    {
      SetActiveView(NULL);
      if((doc = elem->m_myView->GetDocument()) != NULL)
      {
        doc->m_childFrame->MDIDestroy();
      }
      ChildFrame *cf = (ChildFrame *)MDIGetActive();
      if(cf)
        SetFileName(cf->GetDocument()->GetPathName(), cf->GetDocument()->GetPropIndex());
      return;
    }
  }

  SetActiveView(NULL);
  ChildFrame *cf = (ChildFrame *)MDIGetActive();
  if(cf)
  {
    SetActiveView(cf->GetView());
  }

  // It was not one of the MDI childs, so try on the page bar.
  if(m_pageBarDialog.CloseDocument(name))
  {
    WainView *av = m_pageBarDialog.GetActiveView();
    if(av)
    {
      if(IsChild(av))
        SetActiveView(av);
    }
    else if(do_close)
    {
      ShowControlBar(&m_pageBar, FALSE, FALSE);
    }
  }
}

bool MainFrame::CloseDebugFile(const char *aFileName, bool aCloseBar, bool aCanCancel)
{
  if(!m_tools->OnCloseFile(aFileName, aCanCancel))
     return false;

  m_navigatorDialog.m_project->OnCloseDebugFile(aFileName);

  SetActiveView(NULL);
  if(m_pageBarDialog.CloseDocument(aFileName))
  {
    WainView *av = m_pageBarDialog.GetActiveView();
    if(!av && aCloseBar)
      ShowControlBar(&m_pageBar, FALSE, FALSE);
    SetActiveView(NULL);
    ChildFrame *cf = (ChildFrame *)MDIGetActive();
    if(cf)
    {
      SetActiveView(cf->GetView());
    }
  }
  return true;
}

void MainFrame::MakePageBarVisible(WainView *view)
{
  if(!m_pageBar.m_enabled)
    ShowControlBar(&m_pageBar, TRUE, FALSE);
  ((PageBarDialogClass *)m_pageBar.m_myDialog)->SetActiveView(view);
}

void MainFrame::CloseFile(void)
{
   WainView *av = m_pageBarDialog.GetActiveView();
   if(av && GetFocus() == av)
   {
      SetActiveView(NULL);
      CString FileName = av->GetDocument()->GetPathName();
      if(!m_tools->OnCloseFile(FileName, true))
         return;

      m_pageBarDialog.CloseDocument(FileName);
      av = m_pageBarDialog.GetActiveView();
      if(!av)
      {
         ChildFrame *active_child = (ChildFrame *)MDIGetActive();
         if(active_child)
            SetActiveView(active_child->GetView());
         else
            SetActiveView(NULL);
         ShowControlBar(&m_pageBar, FALSE, FALSE);
         RecalcChildPos();
      }
   }
   else
   {
      CloseNormalDoc((ChildFrame *)MDIGetActive());
   }
}

void MainFrame::CloseNormalDoc(ChildFrame *frame)
{
  WainView *av = m_pageBarDialog.GetActiveView();
  if(frame)
  {
    SetActiveView(NULL);
    frame->GetDocument()->DoCloseFile();
    frame = (ChildFrame *)MDIGetActive();
    if(frame)
      SetActiveView(frame->GetView());
    else if(av)
      SetActiveView(av);
  }
}

int MainFrame::TryCloseFile(const char *file_name)
{
  ViewListElem *elem = NULL;
  while((elem = m_viewList.GetRankNext(elem)) != NULL)
  {
    if(!stricmp(file_name, elem->m_myView->GetDocument()->GetPathName()))
    {
      int LineNo = elem->m_myView->m_lineNo;
      SetActiveView(NULL);
      elem->m_myView->GetDocument()->DoCloseFile();
      ChildFrame *frame = (ChildFrame *)MDIGetActive();
      if(frame)
        SetActiveView(frame->GetView());
      else
      {
        WainView *av = ((PageBarDialogClass *)m_pageBar.m_myDialog)->GetActiveView();
        if(av)
          SetActiveView(av);
      }
      return LineNo;
    }
  }
  return -1;
}

WainDoc *MainFrame::ActiveFile(const char *s)
{
  WainDoc *doc;
  ViewListElem *elem = 0;

  while((elem = m_viewList.GetRankNext(elem)) != 0)
  {
    doc = elem->m_myView->GetDocument();
    if(!stricmp(doc->GetPathName(), s) && doc->m_childFrame)
    {
      MDIActivate(doc->m_childFrame);
      elem->m_myView->SetFocus();
      SetActiveView(doc->m_view);
      return doc;
    }
  }
  return 0;
}

void MainFrame::OpenFile(void)
{
  wainApp.OpenFile();
}

void MainFrame::NewFile(void)
{
  wainApp.NewFile();
}

void MainFrame::Exit(void)
{
  wainApp.Exit();
}

void MainFrame::SetupMenu(BOOL force)
{
  static BOOL Update_main = FALSE, Update_view = FALSE;
  static int user_menu_main = -1;
  static int user_menu_view = -1;

  if(force)
  {
    RemovePopupMenuAccelerator(wainApp.gs.m_popupMenu);
    AddPopupMenuAccelerator(wainApp.gs.m_popupMenu);
    Update_main = Update_view = TRUE;
  }

  if(!Update_main && !Update_view)
    return;

  CMenu *top_menu = GetMenu();
  ASSERT(top_menu);

  CMenu *recent_proj_menu;
  BOOL found;
  BOOL is_view_menu = FALSE;
  int pos;
  // First check to see if this is the menu for the mainframe or the view
  for(found = FALSE, pos = top_menu->GetMenuItemCount() - 1; pos >= 0 && !found; pos--)
  {
    CMenu *sub_menu = top_menu->GetSubMenu(pos);
    if(sub_menu && sub_menu->GetMenuItemID(0) == IDV_EDIT_UNDO) // Undo must be the first menu item in a sub menu
    {
      found = TRUE; // It is the view menu
      is_view_menu = TRUE;
    }
  }
  if(found && !Update_view)  // It is the menu for the view, and it does not need a Update
    return;
  if(!found && !Update_main) // It is the menu for main, it does not need a Update
    return;
  if(found)
    Update_view = FALSE;
  else
    Update_main = FALSE;

  char text[64];
  char *s;
  int sub_pos;
  int n;
  BOOL got_user_menu = FALSE;
  for(pos = top_menu->GetMenuItemCount() - 1; pos >= 0; pos--)
  {
    CMenu *sub_menu = top_menu->GetSubMenu(pos);
    BOOL skip = FALSE;
    if(sub_menu)
    {
      if(sub_menu->GetMenuItemID(0) == IDM_TOOL_SETUP) // IDM_TOOL_SETUP is assumed to be nr 0
      {
        // First, delete all items but IDM_TOOL_SETUP at position 0
        for(sub_pos = sub_menu->GetMenuItemCount() - 1; sub_pos > 0; sub_pos--)
          sub_menu->DeleteMenu(sub_pos, MF_BYPOSITION);

        // Then, add a seperator and an item for each available tool
        sub_menu->AppendMenu(MF_SEPARATOR);
        for(size_t i = 0; i < wainApp.gs.m_toolParm.Size(); i++)
          sub_menu->AppendMenu(MF_STRING, IDM_RUN_TOOL_0 + i, wainApp.gs.m_toolParm[i].m_menuText.c_str());
      }
      else if(sub_menu->GetMenuItemID(0) == IDM_MACRO_SETUP)    // IDM_MACRO_SETUP, is assumed to be nr 0
      {
        // First, delete the macros
        for(sub_pos = sub_menu->GetMenuItemCount() - 1; sub_pos > 5; sub_pos--)
          sub_menu->DeleteMenu(sub_pos, MF_BYPOSITION);

        // Then, add the real macro names
        for(size_t i = 0; i < wainApp.gs.m_macroParmList.size(); i++)
          sub_menu->AppendMenu(MF_STRING, IDM_RUN_MACRO_0 + i, wainApp.gs.m_macroParmList[i].m_menuText.c_str());
      }
      else if(sub_menu->GetMenuItemID(0) == IDM_USER_MENU_SETUP) // IDM_USER_MENU_SETUP is assumed to be nr 0
      {
        got_user_menu = TRUE;
        if(is_view_menu)
          user_menu_view = pos;
        else
          user_menu_main = pos;
        if(wainApp.gs.m_useUserMenu)
        {
          // first delete all items but IDM_USER_MENU_SETUP at pos 0
          for(sub_pos = sub_menu->GetMenuItemCount() - 1; sub_pos > 0; sub_pos--)
            sub_menu->DeleteMenu(sub_pos, MF_BYPOSITION);
          // Then, add a seperator and an item for each user item
          sub_menu->AppendMenu(MF_SEPARATOR);
          for(size_t i = 0; i < wainApp.gs.m_userMenu.size(); i++)
            sub_menu->AppendMenu(MF_STRING, wainApp.gs.m_userMenu[i].m_id, MsgId2FuncName(wainApp.gs.m_userMenu[i].m_id));
        }
        else
        {
          skip = TRUE;
          top_menu->DeleteMenu(pos, MF_BYPOSITION);
        }
      }

      for(sub_pos = skip ? 0 : sub_menu->GetMenuItemCount() - 1; sub_pos >= 0 && !skip; sub_pos--)
      {
        if(sub_menu->GetMenuItemID(sub_pos) != 0) // For seperators the id is 0
        {
          sub_menu->GetMenuString(sub_pos, text, 64, MF_BYPOSITION);

          if((s = strchr(text, '\t')) != NULL)
            *s = '\0'; // Remove the accelerator text

          if((recent_proj_menu = sub_menu->GetSubMenu(sub_pos)) != NULL)
          { // The only sub menu who has a sub menu is at the moment "Recent Projects"
            for(int x = 0; x < MAX_NOF_RECENT_PROJECTS; x++)
            {
              if(strlen(wainApp.gs.m_recentProject[x]))
              {
                recent_proj_menu->ModifyMenu(x, MF_BYPOSITION, recent_proj_menu->GetMenuItemID(x), wainApp.gs.m_recentProject[x]);
                recent_proj_menu->EnableMenuItem(recent_proj_menu->GetMenuItemID(x), MF_ENABLED);
              }
              else
              {
                recent_proj_menu->ModifyMenu(x, MF_BYPOSITION, recent_proj_menu->GetMenuItemID(x), "-");
                recent_proj_menu->EnableMenuItem(recent_proj_menu->GetMenuItemID(x), MF_DISABLED);
              }
            }
          }
          for(n = 0, found = FALSE; n < m_nofAccEntrys && !found; n++)
          {
            if((WORD )sub_menu->GetMenuItemID(sub_pos) == m_accEntry[n].cmd)
            {
              strcat(text, "\t");
              KeyState2MenuString(&text[strlen(text)], m_accEntry[n].key, m_accEntry[n].fVirt);
              found = TRUE;
            }
          }
          sub_menu->ModifyMenu(sub_pos, MF_BYPOSITION, sub_menu->GetMenuItemID(sub_pos), text);
        }
      }
    }
  }
  if(!got_user_menu && wainApp.gs.m_useUserMenu)
  {
    CMenu *ins_menu = new CMenu();
    ins_menu->CreatePopupMenu();
    top_menu->InsertMenu(is_view_menu ? user_menu_view : user_menu_main, MF_POPUP | MF_BYPOSITION | MF_STRING, (unsigned int)ins_menu->m_hMenu, "User");
    CMenu *sub_menu = top_menu->GetSubMenu(is_view_menu ? user_menu_view : user_menu_main);
    if(sub_menu)
      sub_menu->AppendMenu(MF_STRING, IDM_USER_MENU_SETUP, "Setup");
    DrawMenuBar();
    SetupMenu(TRUE);
  }

  top_menu->CheckMenuItem(IDM_VIEW_NAVIGATOR_BAR, m_navigatorBar.m_enabled ? MF_CHECKED : MF_UNCHECKED);
  DrawMenuBar();
}

BOOL MainFrame::OnBarCheck(UINT nID)
{
  CControlBar *pBar = GetControlBar(nID);
  if (pBar != NULL)
  {
    DWORD flag = pBar->GetStyle() & WS_VISIBLE;
    bool state = flag ? false : true;
    if(nID == ID_VIEW_TOOLBAR)
      wainApp.gs.m_viewToolBar = state;
    else if(nID == ID_VIEW_STATUS_BAR)
      wainApp.gs.m_viewStatusBar = state;
  }
  return CMDIFrameWnd::OnBarCheck(nID);
}

void MainFrame::AddJumpToTag(const char *pathname, int line, int column)
{
   m_jumpToTagList.Add(pathname, line, column);
}

void MainFrame::AddAutoTagList(const char *pathname, int line, int column)
{
   m_autoTagList.Add(pathname, line, column);
}

void MainFrame::UndoJumpToTag(void)
{
   JumpToTagElemClass JumpToTagElem;
   if(m_jumpToTagList.Get(JumpToTagElem))
   {
      WainDoc *doc = wainApp.OpenDocument(JumpToTagElem.m_fileName.c_str());
      if(doc)
      {
         doc->m_view->GotoLineNo(JumpToTagElem.m_lineNo, JumpToTagElem.m_column);
      }
      else
      {
         SetStatusText("Unable to Undo jump to tag");
      }
   }
   else
   {
      SetStatusText("No jump to tag to Undo");
   }
}

void MainFrame::JumpLastAutoTag(void)
{
   JumpToTagElemClass JumpToTagElem;
   if(m_autoTagList.Get(JumpToTagElem))
   {
      CloseDebugFile(wainApp.m_lastAutoTagFile.c_str(), false, false);
      WainDoc *doc = OpenDebugFile(JumpToTagElem.m_fileName.c_str(), TagPeekDebugFile, false, "", false, false, 0, 0);
      if(doc)
      {
         doc->m_view->GotoLineNo(JumpToTagElem.m_lineNo, JumpToTagElem.m_column);
         wainApp.m_lastAutoTagView = doc->m_view;
         wainApp.m_lastAutoTagFile = JumpToTagElem.m_fileName.c_str();
      }
      else
      {
        SetStatusText("Unable to Undo jump to tag");
      }
   }
   else
      SetStatusText("No more auto tag to jump to");
}

MainFrame *GetMf()
{
  return (MainFrame *)AfxGetMainWnd();
}

void MainFrame::OnDropFiles(HDROP drop_info)
{
  SetActiveWindow();      // Activate us first !
  UINT nof_files = ::DragQueryFile(drop_info, (UINT)-1, NULL, 0);

  char file_name[_MAX_PATH];
  char ext[_MAX_EXT];
  for (UINT file = 0; file < nof_files; file++)
  {
    ::DragQueryFile(drop_info, file, file_name, _MAX_PATH);
    CreateLongFileName(file_name);
    MySplitPath(file_name, SP_EXT, ext);
    if(!stricmp(ext, wainApp.gs.m_projectExtension.c_str()))
      m_navigatorDialog.SetProjectFile(file_name);
    else
      wainApp.OpenDocument(file_name);
  }
  ::DragFinish(drop_info);
}

void MainFrame::HandleRecentProjects(const char *new_proj)
{
  ASSERT(new_proj);
  if(!strlen(new_proj))
    return;
  int i, x;
  // First check if the project name is in the list
  for(i = 0; i < MAX_NOF_RECENT_PROJECTS && stricmp(wainApp.gs.m_recentProject[i], new_proj); i++)
  { /* Nothing */ }

  // if it is the first in the list do nothing
  if(!i)
    return;

  // Copy the recent project one down
  if(i >= MAX_NOF_RECENT_PROJECTS)
    i = MAX_NOF_RECENT_PROJECTS - 1;
  for(x = i; x; x--)
    strcpy(wainApp.gs.m_recentProject[x], wainApp.gs.m_recentProject[x - 1]);

  // Make the new project the first entry
  strcpy(wainApp.gs.m_recentProject[0], new_proj);
  if(m_isInit)
  {
    SetupMenu(TRUE);
  }
}

void MainFrame::OpenRecentProject(UINT id)
{
  ASSERT(id >= IDM_RECENT_PROJ0 && id <= IDM_RECENT_PROJ9);
  int index = id - IDM_RECENT_PROJ0;
  if(strlen(wainApp.gs.m_recentProject[index]))
  {
    char temp[_MAX_PATH];
    /* Our project will call handle_RecentProjects(), which might modify wainApp.gs.RecentProject[index], */
    /* so we will have to pass a copy to set_proj_file */
    strcpy(temp, wainApp.gs.m_recentProject[index]);
    m_navigatorDialog.SetProjectFile(temp);
  }
}

void MainFrame::Help(void)
{
  char temp1[_MAX_PATH];
  strcpy(temp1, wainApp.gs.m_configPath);
  char *p = &temp1[strlen(temp1) - 1];
  if(*p == '\\')
    p--;
  while(*p != '\\' && p != temp1)
    p--;
  p++;
  strcpy(p, "Readme\\readme.html");

  char temp2[_MAX_PATH];
  char command[2048];

  temp2[0] = 0;
  command[0] = 0;
  if((int )FindExecutable(temp1, temp2, command) > 32)
  {
    ::ShellExecute(m_hWnd, "Open", temp1, command, NULL, SW_SHOWNORMAL);
  }
  else
    SetStatusText("Unable to load HelpFile");
}

WainDoc *MainFrame::OpenDebugFile(const char *file_name, DebugFileType FileType, bool set_focus, const std::string& _toolPath, bool _isProject, bool _isMake, uint32_t _nr, const char* _viewName)
{
  WainView *av;
  WainDoc *ad;

  if(!m_pageBar.m_enabled)
  {
    ShowControlBar(&m_pageBar, TRUE, FALSE);
    RecalcChildPos();
  }
  if((ad = m_pageBarDialog.FindDocument(file_name)) != 0)
  {
    ad->Update();
    av = ad->m_view;
  }
  else
  {
    CWnd *old_active_view = GetActiveView();
    av = m_pageBarDialog.OpenDocument(file_name, FileType, _toolPath, _isProject, _isMake, _nr, _viewName);
    av->MoveDocEnd();
    av->MoveHome();
    if(!set_focus && old_active_view)
      old_active_view->SetFocus();
  }

  if(set_focus && av && IsChild(av))
    SetActiveView(av);

  if(av)
    return av->GetDocument();

  return 0;
}

#define _AfxGetDlgCtrlID(hWnd) ((UINT)(WORD)::GetDlgCtrlID(hWnd))

void MainFrame::RepositionBars(UINT nIDFirst, UINT nIDLast, UINT nIDLeftOver, UINT nFlags, LPRECT lpRectParam, LPCRECT lpRectClient, BOOL bStretch)
{
  ASSERT(nFlags == 0 || nFlags == reposQuery || nFlags == reposExtra);

  // walk kids in order, control bars get the resize notification which allow them to shrink the client area
  // remaining size goes to the 'nIDLeftOver' pane
  // NOTE: nIDFirst->nIDLast are usually 0->0xffff

  AFX_SIZEPARENTPARAMS layout;
  HWND hWndLeftOver = NULL;

  layout.bStretch = bStretch;
  layout.sizeTotal.cx = layout.sizeTotal.cy = 0;
  if (lpRectClient != NULL)
    layout.rect = *lpRectClient;    // starting rect comes from parameter
  else
    GetClientRect(&layout.rect);    // starting rect comes from client rect

  if (nFlags != reposQuery)
    layout.hDWP = ::BeginDeferWindowPos(8); // reasonable guess
  else
    layout.hDWP = NULL; // not actually doing layout

  HWND windows_handle[128];
  int winc = 0, i;
  for(i = 0; i < 128; i++)
    windows_handle[i] = 0;

  for(HWND hWndChild = ::GetTopWindow(m_hWnd); hWndChild != NULL; hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
  {
    UINT nIDC = _AfxGetDlgCtrlID(hWndChild);
    CWnd *pWnd = CWnd::FromHandlePermanent(hWndChild);
    if (nIDC == nIDLeftOver)
      hWndLeftOver = hWndChild;
    else if (nIDC >= nIDFirst && nIDC <= nIDLast && pWnd != NULL)
      windows_handle[winc++] = hWndChild;

  }
  // First send to the status bar
  for(i = 0; i < winc; i++)
  {
    if(windows_handle[i] && windows_handle[i] == m_statusBar.m_hWnd)
    {
      ::SendMessage(windows_handle[i], WM_SIZEPARENT, 0, (LPARAM )&layout);
      windows_handle[i] = 0;
    }
  }
  // Then send to any other, this will make the placement of the navigatorbar "correct"
  for(i = winc - 1; i >= 0; i--)
    if(windows_handle[i])
      ::SendMessage(windows_handle[i], WM_SIZEPARENT, 0, (LPARAM)&layout);
  // if just getting the available rectangle, return it now...
  if (nFlags == reposQuery)
  {
    ASSERT(lpRectParam != NULL);
    if (bStretch)
      ::CopyRect(lpRectParam, &layout.rect);
    else
    {
      lpRectParam->left = lpRectParam->top = 0;
      lpRectParam->right = layout.sizeTotal.cx;
      lpRectParam->bottom = layout.sizeTotal.cy;
    }
    return;
  }

  // the rest is the client size of the left-over pane
  if (nIDLeftOver != 0 && hWndLeftOver != NULL)
  {
    CWnd *pLeftOver = CWnd::FromHandle(hWndLeftOver);
    // allow extra space as specified by lpRectBorder
    if (nFlags == reposExtra)
    {
      ASSERT(lpRectParam != NULL);
      layout.rect.left += lpRectParam->left;
      layout.rect.top += lpRectParam->top;
      layout.rect.right -= lpRectParam->right;
      layout.rect.bottom -= lpRectParam->bottom;
    }
    // reposition the window
    pLeftOver->CalcWindowRect(&layout.rect);
    AfxRepositionWindow(&layout, hWndLeftOver, &layout.rect);
  }

  // move and resize all the windows at once!
  if (layout.hDWP == NULL || !::EndDeferWindowPos(layout.hDWP))
    TRACE0("Warning: DeferWindowPos failed - low system resources.\n");
}

void MainFrame::RecalcLayout(BOOL bNotify)
{
  if (m_bInRecalcLayout)
    return;

  m_bInRecalcLayout = TRUE;
  m_nIdleFlags &= ~(idleLayout|idleNotify);

  // reposition all the child windows (regardless of ID)
  if (GetStyle() & FWS_SNAPTOBARS)
  {
    CRect rect(0, 0, 32767, 32767);
    RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery, &rect, &rect, FALSE);
    RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposExtra, &m_rectBorder, &rect, TRUE);
    CalcWindowRect(&rect);
    SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
  }
  else
    RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposExtra, &m_rectBorder);
  m_bInRecalcLayout = FALSE;
}

void MainFrame::AssocList(void)
{
  CMenu bar;
  ChildFrame *cf = (ChildFrame *)MDIGetActive();
  if(cf && bar.LoadMenu(IDR_ASSOC_LIST))
  {
    CMenu &popup = *bar.GetSubMenu(0);
    ASSERT(popup.m_hMenu != NULL);
    int pos;

    // First, delete all items
    for(pos = popup.GetMenuItemCount() - 1; pos >= 0; pos--)
      popup.DeleteMenu(pos, MF_BYPOSITION);
    char file[_MAX_PATH], org_name[_MAX_PATH];
    MySplitPath(cf->GetDocument()->GetPathName(), SP_DRIVE | SP_DIR | SP_FILE, file);
    MySplitPath(cf->GetDocument()->GetPathName(), SP_FILE | SP_EXT, org_name);
    MySplitPath(cf->GetDocument()->GetPathName(), SP_DRIVE | SP_DIR, m_assocPath);
    strcat(file, ".*");
    struct _finddata_t fileinfo;
    long handle = _findfirst(file, &fileinfo);
    if(handle == -1)
    {
      SetStatusText("No associated files found");
      return;
    }
    m_assocFileList.clear();
    do
    {
      if(stricmp(org_name, fileinfo.name))
      {
        popup.AppendMenu(MF_STRING, IDM_ASSOC_ITEM0 + m_assocFileList.size(), fileinfo.name);
        m_assocFileList.push_back(fileinfo.name);
      }
    }
    while(!_findnext(handle, &fileinfo) && m_assocFileList.size() < 10);
    if(m_assocFileList.empty())
    {
      SetStatusText("No associated files found");
      return;
    }
    POINT p;
    cf->GetView()->GetPopupPos(&p, m_assocFileList.size());
    popup.TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
  }
}

void MainFrame::DoAssocList(UINT aId)
{
   if(aId < IDM_ASSOC_ITEM0 || aId > IDM_ASSOC_ITEM9 ||  aId - IDM_ASSOC_ITEM0 >= m_assocFileList.size())
   {
       return;
   }
   aId -= IDM_ASSOC_ITEM0;
   std::string str = m_assocPath + m_assocFileList[aId];
   wainApp.OpenDocument(str.c_str());
}

void MainFrame::FileHasBeenSaved()
{
  m_navigatorDialog.FileHasBeenSaved();
}

void MainFrame::ProjectChanged(const char *file_name)
{
}

void MainFrame::ShowLineNo(bool show)
{
  ViewListElem *elem = NULL;

  while((elem = m_viewList.GetRankNext(elem)) != NULL)
  {
    elem->m_myView->GetDocument()->m_childFrame->ShowLineNo(show);
    if(show)
    { /* Update the Bookmarks in the tab-views */
      for(int i = 0; i < NOF_BOOKMARKS; i++)
      {
        if(m_bookmark[i].m_view == elem->m_myView && m_bookmark[i].m_view->m_tabView)
          m_bookmark[i].m_view->m_tabView->SetBookmark(i, m_bookmark[i].m_lineNo);
      }
    }
  }
}

BOOL MainFrame::OnMouseWheel(UINT flags, short delta, CPoint pt)
{
  return TRUE;
}

WainView *MainFrame::GetOtherView(WainView *m_view)
{
  if(wainApp.gs.m_createMode == CreateModeType::CREATE_DUAL)
  {
    ChildFrame *cf = m_view->GetDocument()->m_childFrame;
    ASSERT(cf);
    if(m_viewList.m_topView[cf->GetPosition() ^ 1])
      return m_viewList.m_topView[cf->GetPosition() ^ 1]->m_myView;
  }
  return NULL;
}

void MainFrame::SetScrollLock(void)
{
  ViewListElem *elem = NULL;
  while((elem = m_viewList.GetRankNext(elem)) != NULL)
  {
    elem->m_myView->DisableScrollLock();
  }
}

WainView *MainFrame::GetActiveView(void)
{
  WainView *view = (WainView *)CMDIFrameWnd::GetActiveView();
  if(IsKindOf(RUNTIME_CLASS(WainView)))
    return view;
  view = m_pageBarDialog.GetActiveView();
  if(view && GetFocus() == view)
    return view;
  ChildFrame *cf = (ChildFrame *)MDIGetActive();
  if(cf && cf->GetView())
    return cf->GetView();

  return NULL; /* Bad luck */
}

void MainFrame::ColorSetup(void)
{
  m_navigatorDialog.m_navigatorList->ColorSetup();
}

UINT CleanupThreadFunc(LPVOID rp)
{
  char full_path[MAX_PATH];
  strcpy(full_path, (char *)rp);

  strcat(full_path, "*.*");
  CFileFind finder;
  BOOL working = finder.FindFile(full_path);
  CTime Old = CTime::GetCurrentTime() - CTimeSpan(30, 0, 0, 0);

   while(working)
   {
      working = finder.FindNextFile();
      if(!finder.IsDirectory())
      {
         CTime FileTime;
         finder.GetLastWriteTime(FileTime);
         if(FileTime < Old)
         { /* The file is more than 30 days old; remove it */
            ::remove(finder.GetFilePath());
         }
      }
   }
   finder.Close();
   free(rp);
   return 0;
}

//-----------------------------------------------------------------------------
// global variables used for passing data to the subclassing wndProc

#define MIN_SHELL_ID 1
#define MAX_SHELL_ID 30000

LPITEMIDLIST GetNextItem (LPITEMIDLIST pidl)
{
  USHORT nLen;

  if ((nLen = pidl->mkid.cb) == 0)
    return NULL;

  return (LPITEMIDLIST) (((LPBYTE) pidl) + nLen);
}

UINT GetItemCount (LPITEMIDLIST pidl)
{
  UINT nCount;

  nCount = 0;
  while(pidl->mkid.cb != 0)
  {
    pidl = GetNextItem(pidl);
    nCount++;
  }
  return nCount;
}

LPITEMIDLIST DuplicateItem(LPMALLOC pMalloc, LPITEMIDLIST pidl)
{
  USHORT nLen;
  LPITEMIDLIST pidlNew;

  nLen = pidl->mkid.cb;
  if (nLen == 0)
    return NULL;

  pidlNew = (LPITEMIDLIST) pMalloc->Alloc(nLen + sizeof (USHORT));
  if (pidlNew == NULL)
    return NULL;

  CopyMemory (pidlNew, pidl, nLen);
  *((USHORT *)(((LPBYTE )pidlNew) + nLen)) = 0;

  return pidlNew;
}

HRESULT GetSHContextMenu(LPSHELLFOLDER psfFolder, LPCITEMIDLIST localPidl, void **ppCM, int *pcmType)
{
  *ppCM = NULL;
  LPCONTEXTMENU pICv1 = NULL; // plain version
  // try to obtain the lowest possible IContextMenu
  HRESULT hr = psfFolder->GetUIObjectOf(GetMf()->m_hWnd, 1, &localPidl, IID_IContextMenu, NULL, (void**)&pICv1);

  if(pICv1 && hr == NOERROR)
  { // try to obtain a higher level pointer, first 3 then 2

    if(pICv1->QueryInterface(IID_IContextMenu3, ppCM) == NOERROR)
    {
      *pcmType = 3;
      pICv1->Release(); // free initial "v1.0" interface
    }
    else if(pICv1->QueryInterface(IID_IContextMenu2, ppCM) == NOERROR)
    {
      *pcmType = 2;
      pICv1->Release(); // free initial "v1.0" interface
    }
    else
    { // no higher version supported
      *pcmType = 1;
      *ppCM = pICv1;
    }
  }
  return *ppCM ? NOERROR : E_NOINTERFACE;
}

HRESULT ProcessCMCommand(LPCONTEXTMENU pCM, UINT idCmdOffset, const char *fn)
{
  CMINVOKECOMMANDINFOEX ici;
  memset(&ici, 0, sizeof(ici));
  ici.cbSize = sizeof(ici);
  ici.lpVerb = MAKEINTRESOURCE(idCmdOffset);
  ici.nShow = SW_SHOWNORMAL;
  HRESULT hr = pCM->InvokeCommand((CMINVOKECOMMANDINFO *)&ici);
  if(hr != NOERROR)
  {
    char text[1024];
    sprintf(text, "ProcessCommand Error: %08X", (unsigned int )hr);
    WainMessageBox(GetMf(), text, IDC_MSG_OK, IDI_WARNING_ICO);
  }
  return hr;
}

void MainFrame::DoContextMenu(CPoint pt, const char *fn)
{
  int cmType; // "version" # of context menu
  CMenu Menu;
  LPSHELLFOLDER shell_folder, psfNextFolder;
  LPITEMIDLIST pidlMain, pidlItem, pidlNextItem, *ppidl;
  ULONG ulCount;
  static TCHAR tchPath[MAX_PATH];
  static WCHAR wchPath[MAX_PATH];
  UINT nCount;
  LPMALLOC pMalloc;

  if (!SUCCEEDED (SHGetMalloc(&pMalloc)))
    return;

  if(!SUCCEEDED(SHGetDesktopFolder(&shell_folder)))
  {
    pMalloc->Release();
    return;
  }
  GetFullPathName(fn, sizeof (tchPath) / sizeof (TCHAR), tchPath, NULL);

  if (IsTextUnicode (tchPath, lstrlen (tchPath), NULL))
    lstrcpy((char *) wchPath, tchPath);
  else
    MultiByteToWideChar(CP_ACP, 0, fn, -1, wchPath, sizeof (wchPath) / sizeof (WCHAR));

  shell_folder->ParseDisplayName(m_hWnd, 0, wchPath, &ulCount, &pidlMain, 0);

  if(pidlMain && (nCount = GetItemCount(pidlMain)) > 0)
  { // nCount must be > 0
    //
    // Initialize psfFolder with a pointer to the IShellFolder
    // interface of the folder that contains the item whose context
    // menu we're after, and initialize pidlItem with a pointer to
    // the item's item ID. If nCount > 1, this requires us to walk
    // the list of item IDs stored in pidlMain and bind to each
    // subfolder referenced in the list.
    //
    pidlItem = pidlMain;

    while (--nCount)
    {
      if((pidlNextItem = DuplicateItem (pMalloc, pidlItem)) == 0)
      {
        pMalloc->Free(pidlMain);
        shell_folder->Release();
        pMalloc->Release();
        return;
      }

      if (!SUCCEEDED(shell_folder->BindToObject(pidlNextItem, NULL, IID_IShellFolder, (void **)&psfNextFolder)))
      {
        pMalloc->Free(pidlNextItem);
        pMalloc->Free(pidlMain);
        shell_folder->Release();
        pMalloc->Release();
        return;
      }

      shell_folder->Release();
      shell_folder = psfNextFolder;

      pMalloc->Free (pidlNextItem);
      pMalloc->Release(); // ???
      pidlItem = GetNextItem (pidlItem);
    }

    ppidl = &pidlItem;

  }
  else
  { /* Something Wrong */
    pMalloc->Release();
    shell_folder->Release();
    return;
  }
  //-----------------

  // m_psfFolder: active IShellFolder; m_localPidl: selected item
  HRESULT hr = GetSHContextMenu(shell_folder, *ppidl, (void **)&m_contextMenu, &cmType);

  if(hr != NOERROR || m_contextMenu == NULL)
  { /* Bad luck */
    pMalloc->Release();
    shell_folder->Release();
    return;
  }
  // fill the menu with the standard shell items
  Menu.CreatePopupMenu();
  m_contextMenu->QueryContextMenu(Menu, 0, MIN_SHELL_ID, MAX_SHELL_ID, CMF_NORMAL);
  // insert a single item of our own for demonstration
  // menu.InsertMenu(0, MF_BYPOSITION | MF_STRING, ID_APP_ABOUT, "&Custom");


  if(cmType > 1)
  {
    m_contextMenu2 = (LPCONTEXTMENU2 )m_contextMenu; // cast ok for ICMv3
    m_isContextMenu = TRUE;
  }

  // show the menu and get the command
  UINT cmdID = Menu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN, pt.x, pt.y, this);
  m_isContextMenu = FALSE;

  if(cmdID >= MIN_SHELL_ID && cmdID <= MAX_SHELL_ID)
    ProcessCMCommand(m_contextMenu, cmdID - MIN_SHELL_ID, fn);
  else if(cmdID)
    PostMessage(WM_COMMAND, cmdID);

  m_contextMenu2 = NULL; // prevents accidental use

  /* TODO, We should release the m_contextMenu. But it makes the program fail in case of Paste */
  if(m_contextMenu)
  {
    m_contextMenu->Release();
    m_contextMenu = 0;
  }
  shell_folder->Release();
  pMalloc->Release();
}

void MainFrame::OnMenuSelect(UINT ItemID, UINT Flags, HMENU hSysMenu)
{
  if(!m_isContextMenu || (MF_POPUP & Flags) || ItemID < MIN_SHELL_ID || ItemID > MAX_SHELL_ID)
  {
    CMDIFrameWnd::OnMenuSelect(ItemID, Flags, hSysMenu);
    return;
  }
  char Buf[256] = "";

  m_contextMenu2->GetCommandString(ItemID - MIN_SHELL_ID, GCS_HELPTEXT, NULL, Buf, sizeof(Buf)/sizeof(Buf[0]));
  SetMessageText(Buf);
}

void MainFrame::OnInitMenuPopup(CMenu *PopupMenu, UINT Index, BOOL SysMenu)
{
  if(!m_isContextMenu)
  {
    CMDIFrameWnd::OnInitMenuPopup(PopupMenu, Index, SysMenu);
    return;
  }

  m_contextMenu2->HandleMenuMsg(WM_INITMENUPOPUP, (WPARAM )PopupMenu->m_hMenu, Index | (SysMenu << 16));
}

void MainFrame::OnDrawItem(int IDCtl, LPDRAWITEMSTRUCT DrawItemStruct)
{
  if(!m_isContextMenu || IDCtl)
  {
    CMDIFrameWnd::OnDrawItem(IDCtl, DrawItemStruct);
    return;
  }

  m_contextMenu2->HandleMenuMsg(WM_DRAWITEM, IDCtl, (LPARAM )DrawItemStruct);
}

void MainFrame::OnMeasureItem(int IDCtl, LPMEASUREITEMSTRUCT MeasureItemStruct)
{
  if(!m_isContextMenu || IDCtl)
  {
    CMDIFrameWnd::OnMeasureItem(IDCtl, MeasureItemStruct);
    return;
  }

  m_contextMenu2->HandleMenuMsg(WM_MEASUREITEM, IDCtl, (LPARAM )MeasureItemStruct);
}

#ifdef _DEBUG
void MainFrame::AssertValid() const
{
  CMDIFrameWnd::AssertValid();
}

void MainFrame::Dump(CDumpContext& dc) const
{
  CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

