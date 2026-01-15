//=============================================================================
// This source code file is a part of Wain.
// It implements the NavigatorDialog.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\wain.h"
#include ".\..\src\wainview.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\Project.h"
#include ".\..\src\TagList.h"
#include ".\..\src\Navigatorlist.h"
#include ".\..\src\childfrm.h"
#include ".\..\src\waindoc.h"
#include ".\..\src\GlobSet.h"
#include ".\..\src\DocProp.h"
#include ".\..\src\WordListThread.h"
#pragma warning (disable : 4100) // unreferenced formal parameter

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT GetIconMsgId = RegisterWindowMessage("NavigatorDialog Get Icon");

#define COLUMN_WITDH_IMAGE  20
#define COLUMN_WITDH_NR     30
#define COLUMN_WITDH_FILE  250

#define TAG_REBUILD_TIMER    1000
#define TAG_REBUILD_INTERVAL (wainApp.gs.m_autoTagRebuildTime*60)

static const unsigned int all_buttons[] =
{
  IDB_REREAD, IDB_TAG_BUILD, IDB_TAG_SETUP, IDB_MAKE, IDB_SETUP, IDB_MANAGE, IDB_OPEN, IDB_TAG_REBUILD, IDB_COMBO, IDB_FTP, 0
};

static const struct
{
  unsigned int m_enable[4];
  BOOL m_useCombo;
}ButtonEnableStatus[] =
{
  {{IDB_REREAD, 0, 0, 0},TRUE},
  {{IDB_REREAD, IDB_FTP, 0, 0},FALSE},
  {{0, 0, 0, 0}, FALSE},
  {{IDB_TAG_BUILD, IDB_TAG_SETUP, 0, 0},FALSE},
  {{IDB_TAG_BUILD, IDB_TAG_SETUP, 0, 0},FALSE},
  {{IDB_MAKE, IDB_SETUP, IDB_MANAGE, IDB_OPEN},FALSE},
  {{IDB_TAG_REBUILD, IDB_TAG_SETUP, 0, 0},FALSE}
};

BEGIN_MESSAGE_MAP(NavigatorDialog, CDialog)
  ON_WM_CREATE()
  ON_WM_CTLCOLOR()
  ON_WM_SIZE()
  ON_WM_CLOSE()
  ON_WM_TIMER()
  ON_WM_PAINT()
  ON_WM_DRAWITEM()
  ON_WM_WINDOWPOSCHANGED()
  ON_COMMAND(IDM_PROJ_MAKE,         ProjectMake)
  ON_COMMAND(IDM_PROJ_MAKE_0,       ProjectMake0)
  ON_COMMAND(IDM_PROJ_MAKE_1,       ProjectMake1)
  ON_COMMAND(IDM_PROJ_MAKE_2,       ProjectMake2)
  ON_COMMAND(IDM_PROJ_MAKE_3,       ProjectMake3)
  ON_COMMAND(IDB_VIEW_DIR,        ViewDir)
  ON_COMMAND(IDB_VIEW_FILES,      ViewFiles)
  ON_COMMAND(IDB_VIEW_TAGS,       ViewTags)
  ON_COMMAND(IDB_VIEW_CLASS,      ViewClass)
  ON_COMMAND(IDB_VIEW_PROJ,       ViewProject)
  ON_COMMAND(IDB_VIEW_CURR,       ViewCurrentTags)
  ON_BN_CLICKED(IDB_OPEN,         ProjectOpen)
  ON_BN_CLICKED(IDB_MAKE,         ProjectMake0)
  ON_BN_CLICKED(IDB_MANAGE,       ProjectManage)
  ON_BN_CLICKED(IDB_SETUP,        ProjectSetup)
  ON_BN_CLICKED(IDB_TAG_BUILD,    MsgBuildTags)
  ON_BN_CLICKED(IDB_TAG_REBUILD,  ReBuildTags)
  ON_BN_CLICKED(IDB_TAG_SETUP,    TagSetup)
  ON_BN_CLICKED(IDB_REREAD,       ReReadDir)
  ON_BN_CLICKED(IDB_FTP,          HandleFtpConnectionection)
  ON_CBN_SELENDOK(IDB_COMBO,      ChangeDisk)
  ON_EN_CHANGE(IDB_SEARCH,        SearchChanged)
  ON_EN_SETFOCUS(IDB_SEARCH,      SearchSetFocus)
  ON_COMMAND(IDB_NEXT,            SearchNext)
  ON_COMMAND(IDB_PREV,            SearchPrev)
  ON_COMMAND(IDB_EDIT,            GotoEditor)
  ON_NOTIFY(TCN_SELCHANGE, IDC_NAVIGATOR_BAR_TAB, OnTabSelChange)
  ON_COMMAND(IDB_TAG_PEEK,  OpenAsTagPeek)

  ON_NOTIFY(TVN_GETINFOTIP, IDB_LIST_TREE, ClassTreeInfoTip)  // NOTC
  ON_NOTIFY(TVN_GETINFOTIP, IDB_LIST_TREE_1, ClassTreeInfoTip)
  ON_NOTIFY(TVN_GETINFOTIP, IDB_LIST_TREE_2, ClassTreeInfoTip)
  ON_NOTIFY(TVN_GETINFOTIP, IDB_LIST_TREE_3, ClassTreeInfoTip)
  ON_NOTIFY(TVN_GETINFOTIP, IDB_LIST_TREE_4, ClassTreeInfoTip)
  ON_NOTIFY(TVN_GETINFOTIP, IDB_LIST_TREE_5, ClassTreeInfoTip)
  ON_NOTIFY(TVN_GETINFOTIP, IDB_LIST_TREE_6, ClassTreeInfoTip)
  ON_NOTIFY(TVN_GETINFOTIP, IDB_LIST_TREE_7, ClassTreeInfoTip)
END_MESSAGE_MAP();

NavigatorDialog::NavigatorDialog(CWnd *parent) :
   CDialog(NavigatorDialog::IDD, parent),
   m_globalTags(this),
   m_currentTags(this)
{
  m_init = false;
  m_skipUpdateSearch = 0;
  m_navBarState = NavBarState::Disabled;
  m_tagImageList.Create(IDB_SMALLICONS, 16, 1, RGB(255, 255, 255));
  m_project = CreateProject(this);

  m_hdDir.m_dlg = this;
  m_ftpDir.m_dlg = this;
  m_sysImageList = 0;
  m_fileList.m_dlg = this;
  m_lastCurrentFile[0] = '\0';
  m_lastCurrentDir[0] = '\0';

  m_buttonBrush = NULL;
  m_accHandle = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD));
  m_lastSearchText = "";
  m_projectChanged = FALSE;

  m_firstActivate = TRUE;

  m_tagRebuildTimerId = 0;

  m_autoRebuildThread = NULL;
  m_autoRebuildTime = TAG_REBUILD_INTERVAL - 60; /* Auto rebuild timer will expire in one minute */
  if(m_autoRebuildTime < 0)
    m_autoRebuildTime = 0;
  m_autoTagFileTimer = 0;
  m_ftpConnection = NULL;
  m_inetConnectAttempt = 0;
  memset(m_ftpConnections, 0xFF, sizeof(m_ftpConnections));
  m_inSetFtp = FALSE;

  m_ftpWaitDialog = NULL;
}

NavigatorDialog::~NavigatorDialog()
{
   if(m_classViewTree)
   {
      m_addClassView.OnClose();
      delete m_classViewTree;
      m_classViewTree = 0;
   }
   if (m_projectTree)
   {
      delete m_projectTree;
   }

  m_init = false;
  delete m_project;
  if(m_buttonBrush)
  {
    m_buttonBrush->DeleteObject();
    delete m_buttonBrush;
  }
  m_project = NULL;
  if(m_autoRebuildThread)
  {
    DWORD exit_code = ~STILL_ACTIVE;
    ::GetExitCodeThread(m_autoRebuildThread->m_hThread, &exit_code);

    if(exit_code == STILL_ACTIVE)
    { // The thread is still running, raise priority and wait for it to end
      m_autoRebuildThread->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
      while(exit_code == STILL_ACTIVE)
      {
        Sleep(500);
        ::GetExitCodeThread(m_autoRebuildThread->m_hThread, &exit_code);
      }
    }
    delete m_autoRebuildThread;
  }
}

int NavigatorDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   if(CDialog::OnCreate(lpCreateStruct) == -1)
      return -1;
   m_listFont.CreateFontIndirect(&wainApp.gs.m_navigatorListLogFont);
   //Create the Tab Control
   CRect rect;
   m_tabCtrl.m_dlg = this;
   m_tabCtrl.Create(WS_VISIBLE | WS_CHILD  | TCS_OWNERDRAWFIXED, rect, this, IDC_NAVIGATOR_BAR_TAB);
   m_tabCtrl.SetFont(&wainApp.m_guiFont);

   TC_ITEM tci;
   tci.mask = TCIF_TEXT;
   tci.pszText = "&Dir";
   m_tabCtrl.InsertItem(int(NavBarState::Dir), &tci);
   tci.pszText = "FTP";
   m_tabCtrl.InsertItem(int(NavBarState::Ftp), &tci);
   tci.pszText = "&Files";
   m_tabCtrl.InsertItem(int(NavBarState::Files), &tci);
   tci.pszText = "&Tags";
   m_tabCtrl.InsertItem(int(NavBarState::Tags), &tci);
   tci.pszText = "C&lass";
   m_tabCtrl.InsertItem(int(NavBarState::Class), &tci);
   tci.pszText = "Pro&ject";
   m_tabCtrl.InsertItem(int(NavBarState::Project), &tci);
   tci.pszText = "&Current";
   m_tabCtrl.InsertItem(int(NavBarState::Curr), &tci);
   m_tabCtrl.SetCurSel(int(NavBarState::Tags));

   return 0;
}

void NavigatorDialog::OnClose(void)
{
   if(m_classViewTree)
      m_addClassView.OnClose();
   CDialog::OnClose();
}

void NavigatorDialog::OnTabSelChange(NMHDR *pNMHDR, LRESULT *pResult)
{
  NavBarState new_state = (NavBarState )m_tabCtrl.GetCurSel();
  ASSERT(new_state < NavBarState::Disabled);
  switch(new_state)
  {
    case NavBarState::Dir:
      ViewDir();
      break;
    case NavBarState::Ftp:
      ViewFtp();
      return;
    case NavBarState::Files:
      ViewFiles();
      break;
    case NavBarState::Tags:
      ViewTags();
      break;
    case NavBarState::Project:
      ViewProject();
      break;
    case NavBarState::Curr:
      ViewCurrentTags();
      break;
    case NavBarState::Class:
      ViewClass();
      break;
  }
  CEdit *search_edit = (CEdit *)GetDlgItem(IDB_SEARCH);
  if(search_edit)
    search_edit->SetWindowText("");
  InvalidateRect(NULL, TRUE);
}

void NavigatorDialog::OnPaint(void)
{
  CRect cr, mr;
  GetClientRect(&cr);
  GetMf()->GetClientRect(&mr); //!!
  if(cr.bottom > mr.bottom)
  {
    cr.bottom = mr.bottom;
    MoveWindow(&cr);
  }

  CDialog::OnPaint();
}

void NavigatorDialog::OnWindowPosChanged(WINDOWPOS *window_pos)
{
  CDialog::OnWindowPosChanged(window_pos);
}

void NavigatorDialog::DoDataExchange(CDataExchange *dx)
{
  CDialog::DoDataExchange(dx);
}

void NavigatorDialog::OnSize(UINT nType, int cx, int cy)
{
  CDialog::OnSize(nType, cx, cy);

  if(!m_init)
    return;

  CButton *b = (CButton *)GetDlgItem(IDB_SETUP);
  ASSERT(b);
  RECT br, dr;
  b->GetWindowRect(&br);
  GetWindowRect(&dr);
  int th = br.bottom - dr.top + 5;
  int eh = 20;
  CRect cr;

  GetClientRect(cr);
  CEdit *e = (CEdit *)GetDlgItem(IDB_SEARCH);
  if(e)
  {
    RECT er;
    e->GetClientRect(&er);
    // eh = er.bottom - er.top;
    er.right = cr.right - 10;
    er.left = cr.left + 10;
    er.top = th;
    er.bottom = er.top + eh;
    e->MoveWindow(&er);
  }

  if(m_tabCtrl.m_hWnd == NULL)
    return;
  GetClientRect(cr);
  cr.DeflateRect(5, 5);
  cr.bottom -= 4;
  m_tabCtrl.MoveWindow(cr);

  CRect ListTreeRect = GetListTreeRect();
  m_navigatorList->MoveWindow(ListTreeRect);
  m_classViewTree->MoveWindow(ListTreeRect);
  m_projectTree->MoveWindow(ListTreeRect);
}

BOOL NavigatorDialog::OnInitDialog(void)
{
  CDialog::OnInitDialog();

  m_init = true;
  RECT r = {5, 5, 150 + 5, 60};
  MapDialogRect(&r);
  m_navigatorList = new NavigatorList(this);
  m_navigatorList->CreateEx(WS_EX_CLIENTEDGE, NULL, "List", WS_VISIBLE | WS_CHILD | WS_TABSTOP, r, this, IDB_LIST_VIEW);

  m_classViewTree = new AddClassInfoTreeCtrlClass(this);
  m_classViewTree->Create(TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP | TVS_NOTOOLTIPS | WS_CHILD | WS_TABSTOP | TVS_SHOWSELALWAYS, r, this, IDB_LIST_TREE);
  m_classViewTree->SetFont(&m_listFont);

   m_projectTree = new NavigatorProjectTree(this);
   m_projectTree->Create(TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP | WS_CHILD | WS_TABSTOP | TVS_SHOWSELALWAYS, r, this, IDB_LIST_PROJ_TREE);
   m_projectTree->SetFont(&m_listFont);

  GetMf()->RecalcChildPos();
  int i;
  CButton *st;
  for(i = 0; all_buttons[i] != 0; i++)
  {
    st = (CButton *)GetDlgItem(all_buttons[i]);
    ASSERT(st);
    st->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);
  }
  if(TAG_REBUILD_INTERVAL)
    m_tagRebuildTimerId = SetTimer(IDB_TAG_REBUILD_TIMER, TAG_REBUILD_TIMER, NULL);
  ViewTags();
  return FALSE;
}

void NavigatorDialog::OnCancel()
{
  // Just to avoid closing the dialog when the user hits ESC
}

void NavigatorDialog::OnOK()
//  Description:
//    Messge handler, called when the user hits enter
{
   if(GetFocus() == m_navigatorList)
   {
      DoSelect(m_navigatorList->m_selected);
   }
   else if(m_classViewTree && GetFocus() == m_classViewTree)
   {
      m_classViewTree->DoSelect();
   }
   else if (m_projectTree && GetFocus() == m_projectTree)
   {
      m_projectTree->DoSelect();
   }
   else
   {
      CEdit *searchEdit = (CEdit *)GetDlgItem(IDB_SEARCH);
      if(GetFocus() == searchEdit)
      {
         if(m_navBarState == NavBarState::Class && m_classViewTree)
            ::SetFocus(m_classViewTree->m_hWnd);
         else if (m_navBarState == NavBarState::Project && m_projectTree)
            ::SetFocus(m_projectTree->m_hWnd);
         else
            ::SetFocus(m_navigatorList->m_hWnd);
      }
   }
}

void NavigatorDialog::EnableButtons(NavBarState _newState)
{
   int i;
   CButton *b;
   CComboBox *cb = (CComboBox *)GetDlgItem(IDB_COMBO);
   ASSERT(cb);
   if(m_navBarState != NavBarState::Disabled)
   {
      if(ButtonEnableStatus[int(m_navBarState)].m_useCombo)
         cb->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);
      else if(ButtonEnableStatus[int(_newState)].m_useCombo)
         cb->ModifyStyle(WS_DISABLED, WS_VISIBLE, SWP_NOACTIVATE | SWP_NOZORDER);

    for(i = 0; i < 4; i++)
    {
       if(ButtonEnableStatus[int(m_navBarState)].m_enable[i])
       {
          b = (CButton *)GetDlgItem(ButtonEnableStatus[int(m_navBarState)].m_enable[i]);
          ASSERT(b);
          b->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);
        }
     }
  }
  for(i = 0; i < 4; i++)
  {
    if(ButtonEnableStatus[int(_newState)].m_enable[i])
    {
      b = (CButton *)GetDlgItem(ButtonEnableStatus[int(_newState)].m_enable[i]);
      ASSERT(b);
      b->ModifyStyle(WS_DISABLED, WS_VISIBLE, SWP_NOACTIVATE | SWP_NOZORDER);
    }
  }
  if((NavBarState )m_tabCtrl.GetCurSel() != _newState)
    m_tabCtrl.SetCurSel(int(_newState));

   if (_newState == NavBarState::Class)
   {
      m_projectTree->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);
      m_navigatorList->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);
      m_classViewTree->ModifyStyle(WS_DISABLED, WS_VISIBLE, SWP_NOACTIVATE | SWP_NOZORDER);
      m_classViewTree->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
   }
   else if (_newState == NavBarState::Project)
   {
      m_classViewTree->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);
      m_navigatorList->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);
      m_projectTree->ModifyStyle(WS_DISABLED, WS_VISIBLE, SWP_NOACTIVATE | SWP_NOZORDER);
      m_classViewTree->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
   }
   else if (_newState == NavBarState::Class)
   {
      if (m_navBarState == NavBarState::Class)
         m_classViewTree->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);
      else
         m_navigatorList->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);

      m_projectTree->ModifyStyle(WS_DISABLED, WS_VISIBLE, SWP_NOACTIVATE | SWP_NOZORDER);
      m_projectTree->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
   }
   else
   {
      m_classViewTree->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);
      m_projectTree->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);
      m_navigatorList->ModifyStyle(WS_DISABLED, WS_VISIBLE, SWP_NOACTIVATE | SWP_NOZORDER);
      m_navigatorList->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
   }
  ::RedrawWindow(m_hWnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

void NavigatorDialog::ViewDir(void)
//  Description:
//    Message handler, called when the user hits "Dir"
//    Disable/enable the appropriate controls
{
  if(m_navBarState != NavBarState::Dir)
  {
    EnableButtons(NavBarState::Dir);
    m_navBarState = NavBarState::Dir;
    m_hdDir.UpdateDir();
  }
}

void NavigatorDialog::ViewFtp(void)
//  Description:
//    Message handler, called when the user hits "FTP"
//    Disable/enable the appropriate controls
{
  if(m_navBarState != NavBarState::Ftp)
  {
    EnableButtons(NavBarState::Ftp);
    m_navBarState = NavBarState::Ftp;
    m_ftpDir.UpdateDir();
  }
}

void NavigatorDialog::ViewFiles(void)
//  Description:
//    Message handler, called when the user hits "Files"
//    Disable/enable the appropriate controls
{
  if(m_navBarState != NavBarState::Files)
  {
    EnableButtons(NavBarState::Files);
    m_navBarState = NavBarState::Files;
    m_fileList.UpdateFileView();
  }
}

void NavigatorDialog::ViewTags(void)
//  Description:
//    Message handler, called when the user hits "Tags"
//    Disable/enable the appropriate controls
{
  if(m_navBarState != NavBarState::Tags)
  {
    EnableButtons(NavBarState::Tags);
    m_navBarState = NavBarState::Tags;
    m_globalTags.UpdateTagList();
    if(m_projectChanged)
    {
      m_projectChanged = FALSE;
      BuildTags();
    }
    else if(m_globalTags.m_tagList && m_globalTags.m_tagList->GetNofTags() == 0 && !m_project->GetTagFile().empty())
    {
      DoBuildTags(NULL, FALSE);
    }
  }
}

void NavigatorDialog::ViewClass(void)
//  Description:
//    Message handler, called when the user hits "class"
//    Disable/enable the appropriate controls
{
   if(m_navBarState != NavBarState::Class)
   {
      EnableButtons(NavBarState::Class);
      m_navBarState = NavBarState::Class;
      if(m_projectChanged)
      {
         m_projectChanged = false;
         BuildTags();
      }
      else if((!m_globalTags.m_tagList || m_globalTags.m_tagList->GetNofTags() == 0) && !m_project->GetTagFile().empty() != '\0')
      {
         DoBuildTags(NULL, FALSE);
      }
   }
}

bool NavigatorDialog::ViewClass(std::string &aClassName)
{
   ViewClass();
   CEdit *searchEdit = (CEdit *)GetDlgItem(IDB_SEARCH);
   if(m_classViewTree->FindClassName(aClassName))
   {
      searchEdit->SetWindowText(aClassName.c_str());
      searchEdit->PostMessage(WM_KEYDOWN, VK_END);
      m_classViewTree->SetFocus();
      m_classViewTree->InvalidateRect(NULL, TRUE);
      return true;
   }
   SetStatusText("Did not find at match for: %s", aClassName.c_str());
   return false;
}

void NavigatorDialog::ViewProject(void)
//  Description:
//    Message handler, called when the user hits "Project"
//    Disable/enable the appropriate controls
{
   if(m_navBarState != NavBarState::Project)
   {
      EnableButtons(NavBarState::Project);
      m_navBarState = NavBarState::Project;
      m_project->UpdateFileList(false);
  }
}

void NavigatorDialog::ViewCurrentTags(void)
//  Description:
//    Message handler, called when the user hits "Current"
//    Disable/enable the appropriate controls
{
  if(m_navBarState != NavBarState::Curr)
  {
    EnableButtons(NavBarState::Curr);
    m_navBarState = NavBarState::Curr;
    ChildFrame *child_frame = (ChildFrame *)GetMf()->MDIGetActive();
    if(child_frame)
    {
      WainDoc *doc = child_frame->GetView()->GetDocument();
      if(strcmp(m_lastCurrentFile, doc->GetPathName()))
        SetFileName(doc->GetPathName(), doc->GetPropIndex());
      else
        m_currentTags.UpdateTagList();
    }
  }
}

void NavigatorDialog::ProjectManage(void)
{
   m_project->Manage();
}

void NavigatorDialog::ProjectSave(void)
//  Description:
//    Message handler, called when the user hits "Save"
{
   m_project->Save();
}

void NavigatorDialog::ReReadDir(void)
//  Description:
//    Message handler, called when the user hits "ReRead"
{
  if(m_navBarState == NavBarState::Dir)
    m_hdDir.UpdateDir();
  else if(m_navBarState == NavBarState::Ftp)
    m_ftpDir.UpdateDir();
}

void NavigatorDialog::OpenDirForFile(const char* _filename)
{
   m_hdDir.OpenDirForFile(_filename);
   ViewDir();
}

void NavigatorDialog::OpenProjectForFile(const char* _filename)
{
   m_project->OpenProjectForFile(_filename);
   ViewProject();
}

void NavigatorDialog::Close(void)
//  Description:
//    Called from OnClose in the main frame.
//    Close down the project settings
{
   if (m_projectTree)
      m_projectTree->ClearTree(0);
  if(m_project)
  {
     m_project->Close(FALSE, FALSE, TRUE);
  }
  if(m_ftpConnection)
  {
    m_ftpConnection->Close();
    delete m_ftpConnection;
    m_ftpConnection = NULL;
  }
}

void NavigatorDialog::ProjectOpen(void)
//  Description:
//    Messsage handler, called when the user hits "Open" in project state
{
   m_project->Open();
}

void NavigatorDialog::ProjectMake(void)
//  Description:
//    Message handler, called when the user hits "Make" or Project->Make from the menu
{
   m_project->Make(-1);
}
void NavigatorDialog::ProjectMake0(void)
//  Description:
//    Message handler, called when the user hits "Make" or Project->Make from the menu
{
   m_project->Make(0);
}
void NavigatorDialog::ProjectMake1(void)
//  Description:
//    Message handler, called when the user hits "Make" or Project->Make from the menu
{
   m_project->Make(1);
}
void NavigatorDialog::ProjectMake2(void)
//  Description:
//    Message handler, called when the user hits "Make" or Project->Make from the menu
{
   m_project->Make(2);
}
void NavigatorDialog::ProjectMake3(void)
//  Description:
//    Message handler, called when the user hits "Make" or Project->Make from the menu
{
   m_project->Make(3);
}

void NavigatorDialog::DoSelect(int selection)
//  Description:
//    Called to select an item in the browse list
{
  MainFrame *mf = GetMf();
  switch(m_navBarState)
  {
    case NavBarState::Dir:
      m_hdDir.DoSelect(selection);
      break;
    case NavBarState::Ftp:
      m_ftpDir.DoSelect(selection);
      break;
    case NavBarState::Files:
      const ViewListElem *elem;
      elem = mf->m_viewList.GetAbsNr(selection);
      if(elem && elem->m_nr != -1)
        mf->ActivateWin(elem->m_nr);
      break;
    case NavBarState::Project:
      {
         const std::string &Name = m_project->GetFileName(selection);
         if(!Name.empty())
            wainApp.OpenDocument(Name.c_str());
      }
      break;
    case NavBarState::Tags:
    {
      WainDoc *doc;
      const TagElemClass &tag = m_globalTags.m_tagList->Get(selection);
      if(tag.m_indexType != TagIndexType::NOF_INDEXES)
      {
        doc = wainApp.OpenDocument(m_globalTags.m_fileList->GetFullName(tag.m_fileIdx));
        if(doc)
        {
          doc->m_view->GotoLineNo(tag.m_lineNo);
        }
      }
      break;
    }
    case NavBarState::Curr:
      if(mf)
      {
        ChildFrame *child_frame = (ChildFrame *)mf->MDIGetActive();
        if(child_frame)
        {
          ::SetFocus(child_frame->GetView()->m_hWnd);
          GetMf()->SetActiveView(child_frame->GetView());
          child_frame->GetView()->GotoLineNo(m_currentTags.m_tagList->Get(selection).m_lineNo);
        }
      }
      break;
  }
}

void NavigatorDialog::JumpToTag(GetTagElemClass *aElem)
//  Description:
//    Called to open the file with the tag as specified in sel
//  Parameters:
//    Sel, the index in the global tags to jump to
{
  WainDoc *doc = wainApp.OpenDocument(aElem->m_fullName.c_str());
  if(doc)
  {
    doc->m_view->GotoLineNo(aElem->m_lineNo);
  }
  delete aElem;
}

void NavigatorDialog::ProjectSetup(void)
//  Description:
//    Message handler, called when the user hits Setup in project state
//  Parameters:
//
{
   m_project->Setup();
}

void NavigatorDialog::DoBuildTags(const char *aFile, bool aForce, bool aAutoBuild)
//  Description:
//    Called to build or read the global tag file.
//  Parameters:
//    File: The current editor file, if any else NULL
//    force: If FALSE to not build the tag file if it exist (default)
//           If TRUE, allways build the tag file
{
   // int propIndex = wainApp.gs.GetPropIndex("-", "*.c");
   // std::vector<std::string> fileList;
   // m_project->GetFileList(fileList);
   // StartWordThread(fileList, wainApp.gs.m_docProp[propIndex]->m_keyWordList, wainApp.gs.m_docProp[propIndex]->m_seps, wainApp.gs.m_docProp[propIndex]->m_ignoreCase);
   m_project->StartWordThread();

   ChildFrame *childFrame = (ChildFrame *)GetMf()->MDIGetActive();
   std::string path = aFile ? aFile : (childFrame ? (const char *)childFrame->GetDocument()->GetPathName() : (const char *)"");
  if(m_project->GetTagFile().empty())
  {
    if(!aAutoBuild)
      WainMessageBox(this, "Tags is not setup correct\r\nPlease use setup to do so", IDC_MSG_OK, IDI_WARNING_ICO);
    return;
  }
  std::string tag_f(m_project->GetTagFile());
  std::string options(m_project->GetTagOptions());
  std::string files(m_project->GetTagFiles());


  if(!path.empty())
  {
    std::string::size_type End = path.find_last_of('\\');
    if(End != std::string::npos)
      path = path.substr(0, End);
  }
  RtvStatus error;
  if((error = wainApp.ReplaceTagValues(tag_f, path)) != RtvStatus::NoError)
  {
    DisplayRtvError(this, "Unable to generate tag file name", error);
    return;
  }
  if((error = wainApp.ReplaceTagValues(options, path)) != RtvStatus::NoError)
  {
    DisplayRtvError(this, "Unable to solve options", error);
    return;
  }
  if((error = wainApp.ReplaceTagValues(files, path)) != RtvStatus::NoError)
  {
    DisplayRtvError(this, "Unable to solve files options", error);
    return;
  }

  if(!aForce && !access(tag_f.c_str(), 4))
  {
    m_globalTags.ReadTagFile(0, tag_f.c_str(), false, aAutoBuild);
  }
  else
  {
    std::string command;
    command = m_project->GetTagProgram();
    command += " ";
    if(options.find("--excmd=number") == std::string::npos)
      command += "--excmd=number ";
    command += options;
    command += " \"-f";
    command += tag_f;
    command += "\" ";
    command += files;
    m_globalTags.ReadTagFile(command.c_str(), tag_f.c_str(), TRUE, aAutoBuild);
  }
  m_projectChanged = FALSE;
}

void NavigatorDialog::MsgBuildTags(void)
//  Description:
//    Message handle, called when the user hits "Build" in tags state
{
  BuildTags(FALSE);
}

void NavigatorDialog::BuildTags(bool aAutoBuild)
{
  DoBuildTags(0, true, aAutoBuild);
}

void NavigatorDialog::FileHasBeenSaved(void)
{
  if(m_navBarState == NavBarState::Curr)
    ReBuildTags();
}

void NavigatorDialog::ReBuildTags(void)
//  Description:
//    Message handler, called when the user hits "ReBuild" in current state
{
  ChildFrame *child_frame = (ChildFrame *)GetMf()->MDIGetActive();
  if(!child_frame)
    return;
  std::string TagFile;

  TagFile = child_frame->GetDocument()->m_prop->m_tagFile;
  std::string path = (const char *)child_frame->GetDocument()->GetPathName();
  RtvStatus error;
  if((error = wainApp.ReplaceTagValues(TagFile, path)) != RtvStatus::NoError)
  {
    DisplayRtvError(this, "Unable to build tag file name", error);
    return;
  }
  std::string command;
  command = child_frame->GetDocument()->m_prop->m_tagProgram;
  command += " ";
  command += child_frame->GetDocument()->m_prop->m_tagOptions;
  if(!strstr(child_frame->GetDocument()->m_prop->m_tagOptions.c_str(), "--excmd=number"))
    command += " --excmd=number";
  command += " \"-f";
  command += TagFile;
  command += "\" \"";
  command += path;
  command += "\"";
  m_currentTags.ReadTagFile(command.c_str(), TagFile.c_str(), true, false);
}

void NavigatorDialog::GlobalTagSetup(void)
{
  TagSetupDialogClass ts(TRUE);

  ts.m_prog = m_project->GetTagProgram().c_str();
  ts.m_options = m_project->GetTagOptions().c_str();
  ts.m_files = m_project->GetTagFiles().c_str();
  ts.m_tagFile = m_project->GetTagFile().c_str();
  ts.m_track = m_project->GetTrackTags();
  if(ts.DoModal() == IDOK)
  {
    m_project->SetTagProgram(ts.m_prog);
    m_project->SetTagOptions(ts.m_options);
    m_project->SetTagFiles(ts.m_files);
    m_project->SetTagFile(ts.m_tagFile);
    m_project->SetTrackTags(ts.m_track ? true : false);
  }
}

void NavigatorDialog::TagSetup(void)
//  Description:
//    Message handler, called when the user hits "Setup" in Tags or Current state
{

  if(m_navBarState == NavBarState::Tags || m_navBarState == NavBarState::Class)
  {
    if(!m_project->GetProjectName().empty())
    {
      GlobalTagSetup();
    }
    else
      WainMessageBox(this, "You must first create/open a project before you can setup tags", IDC_MSG_OK, IDI_WARNING_ICO);
  }
  else // if NavBarState == NavBarState::Curr
  {
    TagSetupDialogClass ts(FALSE);
    ChildFrame *cf = (ChildFrame *)GetMf()->MDIGetActive();
    if(cf)
    {
      ts.m_prog    = cf->GetDocument()->m_prop->m_tagProgram;
      ts.m_options = cf->GetDocument()->m_prop->m_tagOptions;
      ts.m_files = "";
      ts.m_tagFile = cf->GetDocument()->m_prop->m_tagFile;
      if(ts.DoModal() == IDOK)
      {
        cf->GetDocument()->m_prop->m_tagProgram = ts.m_prog;
        cf->GetDocument()->m_prop->m_tagOptions = ts.m_options;
        cf->GetDocument()->m_prop->m_tagFile = ts.m_tagFile;
        cf->GetDocument()->m_prop->m_modified = TRUE;
      }
    }
  }
}

int NavigatorDialog::GetListIconNr(int item)
//  Description:
//    Called from the navigator list to get the index in the icon list which match item
//  Parameters:
//    item, the item which index is to be returned
//    Returns the index or -1 in case of failure
{
  int ret = -1;
  switch(m_navBarState)
  {
    case NavBarState::Dir:
      return m_hdDir.GetIconIndex(item);
    case NavBarState::Ftp:
      break;
    case NavBarState::Files:
      const ViewListElem *elem;
      elem = GetMf()->m_viewList.GetAbsNr(item);
      if(elem)
        ret = elem->m_iconIndex;
      break;
    case NavBarState::Tags:
      if(m_globalTags.m_tagList && item < (int )m_globalTags.m_tagList->GetNofTags())
         ret = int(m_globalTags.m_tagList->Get(item).m_indexType);
      break;
    case NavBarState::Curr:
      if(m_currentTags.m_tagList && item < (int )m_currentTags.m_tagList->GetNofTags())
        ret = int(m_currentTags.m_tagList->Get(item).m_indexType);
      break;
    case NavBarState::Project:
      ret = m_project->GetFileIcon(item);
      break;
  }
  return ret;
}

const char *NavigatorDialog::GetListText(int item, int column, BOOL short_text)
//  Description:
//    Called from the navigator list to get the text for the specified item and column
//  Parameters:
//    item, the index which text is to be retrieved.
//    column, the column for which we must return the text.
//    Returns the text or NULL in case of error
{
  static char temp[_MAX_PATH]; // Must be static since we might return a pointer to it.
  const char *ret = NULL;

  switch(m_navBarState)
  {
    case NavBarState::Dir:
      ret = m_hdDir.GetListText(item);
      break;
    case NavBarState::Ftp:
      ret = m_ftpDir.GetListText(item);
      break;
    case NavBarState::Files:
      const ViewListElem *elem;
      elem = GetMf()->m_viewList.GetAbsNr(item);
      if(elem)
      {
        if(column == 1)
        {
          sprintf(temp, "%2d", elem->m_nr);
          ret = temp;
        }
        else if(short_text)
          ret = elem->m_shortName.c_str();
        else
          ret = elem->m_name.c_str();
      }
      break;
    case NavBarState::Tags:
      if(m_globalTags.m_tagList && item < (int )m_globalTags.m_tagList->GetNofTags())
      {
        if(column == 1)
          ret = m_globalTags.m_tagList->Get(item).m_tag.c_str();
        else
          ret = m_globalTags.m_fileList->GetShortName(m_globalTags.m_tagList->Get(item).m_fileIdx);
      }
      break;
    case NavBarState::Curr:
      if(m_currentTags.m_tagList && item < (int )m_currentTags.m_tagList->GetNofTags())
      {
        if(column == 1)
          ret = m_currentTags.m_tagList->Get(item).m_tag.c_str();
        else
          ret = m_currentTags.m_fileList->GetShortName(m_currentTags.m_tagList->Get(item).m_fileIdx);
      }
      break;
    case NavBarState::Project:
      ret = m_project->GetFileName(item, short_text ? true : false).c_str();
      break;
  }
  return ret;
}

void NavigatorDialog::SetFileName(const char *file_name, int _propIndex)
//  Description:
//    Called from the main frame when the editor file has changed
//  Parameters:
//    file_name, the name of the new editor file
{
  RtvStatus error;
  if(m_navBarState == NavBarState::Curr)
  {
    if(strcmp(file_name, m_lastCurrentFile))
    {
      m_currentTags.Clear();
      strcpy(m_lastCurrentFile, file_name);
      std::string TagFile = wainApp.gs.m_docProp[_propIndex]->m_tagFile;
      if((error = wainApp.ReplaceTagValues(TagFile, std::string(file_name))) == RtvStatus::NoError)
      {
        if(!access(TagFile.c_str(), 0))
        { /* The tag_file exist */
          m_currentTags.ReadTagFile(0, TagFile.c_str(), false, false);
        }
        else
        {
          std::string command;
          command = wainApp.gs.m_docProp[_propIndex]->m_tagProgram;
          command += " ";
          command += wainApp.gs.m_docProp[_propIndex]->m_tagOptions;
          if(!strstr(wainApp.gs.m_docProp[_propIndex]->m_tagOptions.c_str(), "--excmd=number"))
            command += " --excmd=number";
          command += " \"-f";
          command += TagFile;
          command += "\" \"";
          command += file_name;
          command += "\"";
          m_currentTags.ReadTagFile(command.c_str(), TagFile.c_str(), true, false);
        }
      }
      else
      {
        DisplayRtvError(this, "Unable to build tag file name", error);
      }
    }
  }
  else if(m_navBarState == NavBarState::Files)
  {
    MainFrame *mf = GetMf();
    if(mf)
    {
      const ViewListElem *elem;
      int i;
      for(i = 0; (elem = mf->m_viewList.GetAbsNr(i)) != NULL; i++)
        if(elem->m_name == file_name)
        {
          m_navigatorList->SetSel(i);
          return;
        }
    }
  }
}

void NavigatorDialog::SetProjectFile(const char *file_name)
//  Description:
//    Called from the main frame when the user has droped a project file on Wain
//  Parameters:
//    file_name, the name of the new project
{
  m_project->SetProjectFile(file_name);
}

void NavigatorDialog::SetNewFont(void)
//  Description:
//    Called from main frame when the user has specified a new font for the navigator list
{
  m_listFont.Detach();
  m_listFont.CreateFontIndirect(&wainApp.gs.m_navigatorListLogFont);
  m_navigatorList->SetNewFont();
  if(m_classViewTree)
    m_classViewTree->SetFont(&m_listFont);
}

void NavigatorDialog::ChangeDisk(void)
//  Description:
//    Message handler, called when the user has changed disk in the disk combo
{
  m_hdDir.ChangeDisk();
}

void NavigatorDialog::HandleDelete(int sel)
//  Description:
//    Called from the borwse list when the user has hit delete
//  Parameters:
//    sel, the selected item in the navigator list
{
  if(m_navBarState == NavBarState::Project)
  {
    m_project->RemoveFile(sel);
  }
  else if(m_navBarState == NavBarState::Files)
  { // Close the selected file
    const char *s = GetListText(sel, 0, FALSE);
    if(s)
      GetMf()->RemoveFile(s);
  }
}

void NavigatorDialog::OnListSetSel(int sel)
{
  if(m_navBarState == NavBarState::Files && wainApp.gs.m_navigatorBarShortNames)
  {
    const char *s = GetListText(sel, 2, FALSE);
    if(s)
      SetStatusText(s);
  }
  else if(m_navBarState == NavBarState::Project && wainApp.gs.m_navigatorBarShortNames)
  {
    const char *s = GetListText(sel, 1, FALSE);
    if(s)
      SetStatusText(s);
  }
  else if(m_navBarState == NavBarState::Tags)
  {
    if(m_globalTags.m_tagList && sel < (int )m_globalTags.m_tagList->GetNofTags())
    {
      const char *s = m_globalTags.m_fileList->GetFullName(m_globalTags.m_tagList->Get(sel).m_fileIdx);
      if(s)
        SetStatusText(s);
    }
  }
}

void NavigatorDialog::HandleListCtrlPageUp(void)
//  Description:
//    called from the brose list when the user hits CTRL+PRIOR
{
  if(m_navBarState == NavBarState::Dir)
  {
    m_hdDir.HandlePrev();
  }
  else if(m_navBarState == NavBarState::Ftp)
  {
    m_ftpDir.HandlePrev();
  }
}

void NavigatorDialog::OnTimer(UINT _timerId)
//  Description:
//    Timer message handler
//  Parameters:
//    timer_id, the id of the expired timer
{
  if(_timerId == m_tagRebuildTimerId)
    HandleAutoRebuildTimeout();
  else if(_timerId == m_autoTagFileTimer)
    HandleAutoTagFileTimer();
  else
    SetStatusText("Unknown timer");
}

void NavigatorDialog::HandleAutoTagFileTimer(void)
{
  if(++m_autoTagFileCount >= 4)
  {
    KillTimer(m_autoTagFileTimer);
    m_autoTagFileTimer = 0;
  }

  GetTagListClass *TagList = new GetTagListClass(wainApp.m_lastTagStr, TL_ANY);

  if(!GetTagList(TagList))
  {
    delete TagList;
    return;
  }
  if(TagList->m_list.size())
  {
    GetTagElemClass *elem = new GetTagElemClass(*TagList->m_list.begin());
    OnTagPeek(elem);
  }
  delete TagList;
}

#define STAT _stat

void NavigatorDialog::HandleAutoRebuildTimeout(void)
{
  if(++m_autoRebuildTime >= TAG_REBUILD_INTERVAL)
  {
    if(m_autoRebuildThread)
    {
      DWORD exit_code = ~STILL_ACTIVE;
      ::GetExitCodeThread(m_autoRebuildThread->m_hThread, &exit_code);
      if(exit_code == STILL_ACTIVE)
        return; /* Note: Do not Reset AutoRebuildTime */
      delete m_autoRebuildThread;
      m_autoRebuildThread = NULL;
    }

    KillTimer(m_tagRebuildTimerId);
    m_tagRebuildTimerId = 0;

    const ViewListElem *celem;
    int i, j, c = 0;
    std::string TagFile;

    struct STAT f_stat, t_stat;

    ChildFrame *cf = (ChildFrame *)GetMf()->MDIGetActive();
    AutoRebuildTagVector *ReadParm = new AutoRebuildTagVector;

    j = 0;
    i = 0;
    BOOL GotError = FALSE;
    while(j < 64 && (celem = GetMf()->m_viewList.GetAbsNr(i++)) != NULL && !GotError)
    {
      TagFile = celem->m_myView->GetDocument()->m_prop->m_tagFile;
      if(!TagFile.empty())
      {
        RtvStatus error;
        if((error = wainApp.ReplaceTagValues(TagFile, celem->m_name)) != RtvStatus::NoError)
        {
          DisplayRtvError(this, "Unable to build tag file name", error);
          GotError = TRUE;
        }
        else
        {

          if(!STAT(celem->m_name.c_str(), &f_stat) && !STAT(TagFile.c_str(), &t_stat))
          {
            if(f_stat.st_mtime > t_stat.st_mtime)
            { /* The file is more recet than the tagfile, rebuild the tag file */
              if(cf && celem->m_name == (const char *)cf->GetView()->GetDocument()->GetPathName())
              { /* This elem is the active one, rebuild tags as normal */
                ReBuildTags();
                c++;
              }
              else
              {
                ReadParm->push_back(new AutoRebuildTagClass(celem->m_myView->GetDocument()->m_prop->m_tagProgram,
                                                            celem->m_myView->GetDocument()->m_prop->m_tagOptions,
                                                            TagFile,
                                                            celem->m_name));
                j++;
              }
            }
          }
        }
      }
    }
    if(j || c) // If any of the open files is changed, rebuild the global tags
      BuildTags(TRUE);

    ViewListElem *elem = NULL;
    while(j < 64 && (elem = GetMf()->m_viewList.GetRankNext(elem)) != NULL && !GotError)
    { // Build tag files for open files which does not have a tag file
      TagFile = elem->m_myView->GetDocument()->m_prop->m_tagFile;
      RtvStatus error;
      if((error = wainApp.ReplaceTagValues(TagFile, elem->m_name)) != RtvStatus::NoError)
      {
        DisplayRtvError(this, "Unable to build tag file name", error);
        GotError = TRUE;
      }
      else if(!TagFile.empty() && access(TagFile.c_str(), 0))
      {
        ReadParm->push_back(new AutoRebuildTagClass(elem->m_myView->GetDocument()->m_prop->m_tagProgram,
                                                    elem->m_myView->GetDocument()->m_prop->m_tagOptions,
                                                    TagFile,
                                                    elem->m_name));
        j++;
      }
    }
    if(j)
    {
      m_autoRebuildThread = AfxBeginThread(ThreadAutoRebuildTag, ReadParm, THREAD_PRIORITY_LOWEST, 0, CREATE_SUSPENDED);
      m_autoRebuildThread->m_bAutoDelete = FALSE;
      m_autoRebuildThread->ResumeThread();
    }
    else
      delete ReadParm;
    if(TAG_REBUILD_INTERVAL)
       m_tagRebuildTimerId = SetTimer(IDB_TAG_REBUILD_TIMER, TAG_REBUILD_TIMER, NULL);
    m_autoRebuildTime = 0;
  }
}


HBRUSH NavigatorDialog::OnCtlColor(CDC *_dc, CWnd *wnd, UINT ctrl_color)
//  Description:
//    Message handler, called to get the brush to be used for drawing controls
//  Parameters:
//
{
  if(!m_buttonBrush)
  {
    unsigned int back_color = GetSysColor(COLOR_3DFACE);
    unsigned int new_color;
    CWindowDC dc(this);
    if(dc.GetDeviceCaps(BITSPIXEL) > 8)
    {
      back_color = Brighten(back_color, 32);
      new_color = dc.GetNearestColor(back_color);
      if(new_color == CLR_INVALID)
        new_color = back_color;
    }
    else
      new_color = back_color;
    m_buttonBrush = new CBrush(new_color);
  }

  HBRUSH br = CDialog::OnCtlColor(_dc, wnd, ctrl_color);
  if(ctrl_color == CTLCOLOR_DLG || ctrl_color == CTLCOLOR_STATIC || ctrl_color == CTLCOLOR_BTN)
  {
    br = *m_buttonBrush;
  }
  return br;
}

void NavigatorDialog::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT draw_item_struct)
{
   CRect dr = draw_item_struct->rcItem;
   if(draw_item_struct->itemState & ODS_SELECTED)
   {
      dr.top -= 5;
      // dr.bottom -= 1;
   }
   else
   {
      dr.bottom += 2;
   }
   ::FillRect(draw_item_struct->hDC, dr, CBrush(::GetBkColor(draw_item_struct->hDC)));
   TCITEM item;
   char buffer[128];
   item.pszText = buffer;
   item.cchTextMax = 128;
   item.mask = TCIF_TEXT | TCIF_PARAM;
   int id = draw_item_struct->itemID;

   m_tabCtrl.GetItem(id, &item);
   dr = draw_item_struct->rcItem;
   dr.left += 4;
   dr.top += 3;
   ::DrawText(draw_item_struct->hDC, item.pszText, strlen(item.pszText), dr, 0);
}

void NavigatorDialog::DoSearch(int _from, int _direction, bool _force, bool _reset, int _offset)
//  Description:
//    Do a search in the list
//  Parameters:
//    from, the index from where to search
//    direction, 1 for Forward, -1 for reverse
//    force, TRUE if we must search allways
{
   bool result = false;
   bool found;
   if(_from < 0)
      _from = 0;

   CEdit *search_edit = (CEdit *)GetDlgItem(IDB_SEARCH);
   search_edit->GetWindowText(m_searchText);
   if(!_force && !strcmp(m_searchText, m_lastSearchText))
      return;
   int index = 0; // Dummy initialization to keep Mr. Gates happy
   switch(m_navBarState)
   {
   case NavBarState::Dir:
      index = _from;
      found = m_hdDir.Search(&index, _direction, m_searchText);
      if(found)
      {
         index -= _direction;
         result = true;
      }
      else
         result = false;
      break;
   case NavBarState::Ftp:
      index = _from;
      found = m_ftpDir.Search(&index, _direction, m_searchText);
      if(found)
      {
         index -= _direction;
         result = true;
      }
      else
         result = false;
      break;
   case NavBarState::Files:
      const ViewListElem *elem;
      elem = GetMf()->m_viewList.GetAbsNr(_from);
      if(elem)
      {
         found = false;
         index = _from;
         do
         {
            if(wainApp.gs.m_navigatorBarShortNames)
            {
              if(MyStrIStr(elem->m_shortName.c_str(), m_searchText))
                 found = true;
              else
                 index += _direction;
            }
            else
            {
               if(MyStrIStr(elem->m_name.c_str(), m_searchText))
                  found = true;
               else
                  index += _direction;
            }
         }
         while(!found && (elem = GetMf()->m_viewList.GetAbsNr(index)) != NULL);
         result = found;
      }
      else
         result = FALSE;
      break;
   case NavBarState::Tags:
      if(!m_globalTags.m_tagList)
         result = FALSE;
      else
      {
         if(m_navigatorList->m_selected >= (int )m_globalTags.m_tagList->GetNofTags())
            m_navigatorList->m_selected = 0;

         if(!_force && MyStrIStr(m_globalTags.m_tagList->Get(m_navigatorList->m_selected).m_tag.c_str(), m_searchText))
         {
            index = m_navigatorList->m_selected;
            result = TRUE;
         }
         else
         {
            CString sSearchText = m_searchText;
            sSearchText.MakeLower();
            for(index = _from, found = false; index < (int )m_globalTags.m_tagList->GetNofTags() && index >= 0 && !found; index += _direction)
               if(MyStrIStr2(m_globalTags.m_tagList->Get(index).m_tag.c_str(), sSearchText))
                  found = true;
            if(found)
            {
                index -= _direction;
                result = true;
            }
            else
               result = false;
         }
      }
      break;
   case NavBarState::Curr:
      if(!m_currentTags.m_tagList)
         result = false;
      else
      {
         if(m_navigatorList->m_selected >= (int )m_currentTags.m_tagList->GetNofTags())
            m_navigatorList->m_selected = 0;

         if(m_currentTags.m_tagList)
         {
            for(index = _from, found = false; index < (int )m_currentTags.m_tagList->GetNofTags() && index >= 0 && !found; index += _direction)
               if(MyStrIStr(m_currentTags.m_tagList->Get(index).m_tag.c_str(), m_searchText))
                  found = true;
            if(found)
            {
               index -= _direction;
               result = true;
            }
            else
               result = false;
         }
         else
            result = false;
      }
      break;
   case NavBarState::Project:
      index = _from;
      result = m_project->Search(_force ? _direction : 0, (const char *)m_searchText, _reset ? true : false, index);
      break;
   case NavBarState::Class:
      if(m_classViewTree)
         result = m_classViewTree->DoSearch(_direction, m_searchText, _reset, _offset);
      else
         result = false;
      break;
   }
   if(result)
   {
      m_lastSearchText = m_searchText;
      if(m_navBarState != NavBarState::Class)
         m_navigatorList->SetSel(index);
   }
   else
   {
      if(_reset)
         m_lastSearchText = "";
      search_edit->SetWindowText(m_lastSearchText);
      search_edit->PostMessage(WM_KEYDOWN, VK_END);
   }
}

void NavigatorDialog::SearchChanged(void)
//  Description:
//    Message handler, called when the text in the search edit is changed
//  Parameters:
//
{
   if(!m_skipUpdateSearch)
      DoSearch(0, 1, FALSE, FALSE);
   else
      m_skipUpdateSearch--;
}

void NavigatorDialog::SearchSetFocus(void)
//  Description:
//    Message handler, called when the search edit box gets focus
{
   DoSearch(0, 1, TRUE, TRUE);
}

BOOL NavigatorDialog::PreTranslateMessage(MSG *msg)
{
  if(m_accHandle && TranslateAccelerator(m_hWnd, m_accHandle, msg))
    return TRUE;
  if(msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN)
  {
    if(msg->wParam >= VK_F1 && msg->wParam <= VK_F12)
    {
      return FALSE; /* We do not use these, but the main_frame does, so let it have them */
    }
  }
  return CDialog::PreTranslateMessage(msg);
}

void NavigatorDialog::AddSearchChar(char ch)
{
   CEdit *e = (CEdit *)GetDlgItem(IDB_SEARCH);
   ASSERT(e);
   CString str;
   e->GetWindowText(str);
   if(ch == -1)
   {
      if(!str.IsEmpty())
      {
         str.Delete(str.GetLength() - 1);
      }
   }
   else
   {
      str += ch;
   }
   e->SetWindowText(str);
}

void NavigatorDialog::SearchNext(void)
//  Description:
//    Message handler, called to find the next match
{
   DoSearch(m_navigatorList->m_selected + 1, 1, TRUE, FALSE, 1);
}

void NavigatorDialog::SearchPrev(void)
//  Description:
//    Message handler, called to find the previous match
{
   DoSearch(m_navigatorList->m_selected - 1, -1, TRUE, FALSE, -1);
}

void NavigatorDialog::GotoEditor(void)
//  Description:
//    Message handler, called to set focus to the active document
{
  ChildFrame *cf = (ChildFrame *)GetMf()->MDIGetActive();
  if(cf)
  {
    cf->GetView()->SetFocus();
  }
}

RtvStatus NavigatorDialog::MakeProjectFileList(std::string &FileName, const char *types)
//  Description:
//    Called to make a list of the files in the current project. (Tag value $X$)
//  Parameters:
//    file_name, the name of the file in which the filenames must be stored.
//    Return TRUE on success else FALSE
{
  return m_project->MakeProjectFileList(FileName, types);
}

void NavigatorDialog::Activate(void)
//  Description:
//    Called from the main frame when the application is Activated
{
  if(m_firstActivate)
  {
    m_firstActivate = FALSE;
  }
  else
  {
    if(!m_tagRebuildTimerId && TAG_REBUILD_INTERVAL)
      m_tagRebuildTimerId = SetTimer(IDB_TAG_REBUILD_TIMER, TAG_REBUILD_TIMER, NULL);
  }
}

void NavigatorDialog::DeActivate(void)
//  Description:
//    Called from the mainframe when the application is DeActivated
{
  if(!m_firstActivate)
  {
    if(m_tagRebuildTimerId)
    {
      KillTimer(m_tagRebuildTimerId);
      m_tagRebuildTimerId = 0;
    }
  }
}

int NavigatorDialog::GetMatchTags(std::string aList[], std::string &aWord)
//  Description:
//    Message handler, gets a list of tags matching a word
//  Parameters:
{
  if(m_globalTags.m_tagList)
  {
    int i, n, j;
    for(i = 0, n = 0; i < (int )m_globalTags.m_tagList->GetNofTags() && n < TL_MAX_NOF_TAGS; i++)
    {
      if(MyStrIStr(m_globalTags.m_tagList->Get(i).m_tag.c_str(), aWord.c_str()))
      {
        bool found;
        for(j = 0, found = false; j < n; j++)
          if(aList[j] == m_globalTags.m_tagList->Get(i).m_tag)
            found = true;
        if(!found)
          aList[n++] = m_globalTags.m_tagList->Get(i).m_tag;
      }
    }
    return n;
  }
  return 0;
}

bool NavigatorDialog::GetTagList(GetTagListClass *aList)
{
   m_globalTags.GetTagList(aList, true);
  if(m_classViewTree && m_classViewTree->m_addClassView && m_classViewTree->m_addClassView->m_addInfo)
    m_classViewTree->m_addClassView->m_addInfo->GetTagList(aList);
   return m_globalTags.GetTagList(aList, false);
}

void NavigatorDialog::TagReadFunc(ReadTagClass *aReadTag)
{
  m_currentTags.HandleTagsRead(aReadTag);
}

void NavigatorDialog::GlobalTagReadFunc(ReadTagClass *aReadTag)
{
  m_globalTags.HandleTagsRead(aReadTag);
}

void NavigatorDialog::OnTagPeek(GetTagElemClass *aElem)
{
  WainView *view = (WainView *)GetMf()->GetActiveView();
  bool WasAuto = false;

  if(view  && (view == wainApp.m_lastAutoTagView || view->GetDocument()->m_isDebugFile))
    WasAuto = true;

  GetMf()->CloseDebugFile(wainApp.m_lastAutoTagFile.c_str(), false, false);
  WainDoc *Doc = GetMf()->OpenDebugFile(aElem->m_fullName.c_str(), TagPeekDebugFile, true, "", false, false, 0, 0);
  if(Doc)
  {
    Doc->m_view->GotoLineNo(aElem->m_lineNo);
    wainApp.m_lastAutoTagView = Doc->m_view;
  }
  wainApp.m_lastAutoTagFile = aElem->m_fullName;
  delete aElem;

  if(!WasAuto && view)
    view->SetFocus();

  if(m_autoTagFileTimer)
  {
    KillTimer(m_autoTagFileTimer);
    m_autoTagFileTimer = 0;
  }
}

void NavigatorDialog::OnAutoTag()
{
  if(m_project->GetTrackTags())
  {
    if(!m_autoTagFileTimer)
      m_autoTagFileTimer = SetTimer(IDB_AUTO_TAG_FILE_TIMER, 250, NULL);
    m_autoTagFileCount = 0;
  }
  else if(m_autoTagFileTimer)
  {
    KillTimer(m_autoTagFileTimer);
    m_autoTagFileTimer = 0;
  }
}

void NavigatorDialog::HandleGetIcon(void *aMsg)
{
  m_hdDir.HandleGetIcon((ThreadGetIconConClass *)aMsg);
  m_project->HandleGetIcon((ThreadGetIconConClass *)aMsg);
  ThreadGetIconConClass *con = (ThreadGetIconConClass *)aMsg;

  if(m_navBarState == NavBarState::Dir || m_navBarState == NavBarState::Project)
    m_navigatorList->UpdateIcons(m_sysImageList);

  delete con;
}

void NavigatorDialog::OpenAsTagPeek(void)
{
  int LineNo = -1;
  const char *fn = NULL;

  if(m_navBarState == NavBarState::Tags)
  {
    fn = m_globalTags.m_fileList->GetFullName(m_globalTags.m_tagList->Get(m_navigatorList->m_selected).m_fileIdx);
    LineNo = m_globalTags.m_tagList->Get(m_navigatorList->m_selected).m_lineNo;
  }
  else if(m_navBarState == NavBarState::Curr)
  {
    fn = m_currentTags.m_fileList->GetFullName(m_currentTags.m_tagList->Get(m_navigatorList->m_selected).m_fileIdx);
    LineNo = m_currentTags.m_tagList->Get(m_navigatorList->m_selected).m_lineNo;
  }
  else if(m_navBarState == NavBarState::Class)
  {
    m_classViewTree->GetPeekParm(&fn, &LineNo);
  }
  if(fn && LineNo != -1)
  {
    if(wainApp.m_lastAutoTagView && wainApp.m_lastAutoTagView->GetDocument()->GetPathName() == fn)
    {
      wainApp.m_lastAutoTagView->GotoLineNo(LineNo);
      GetMf()->MakePageBarVisible(wainApp.m_lastAutoTagView);
    }
    else
    {
      GetMf()->CloseDebugFile(wainApp.m_lastAutoTagFile.c_str(), false, false);
      WainDoc *doc = GetMf()->OpenDebugFile(fn, TagPeekDebugFile, true, "", false, false, 0, 0);
      if(doc)
      {
        doc->m_view->GotoLineNo(LineNo);
        wainApp.m_lastAutoTagView = doc->m_view;
      }
      wainApp.m_lastAutoTagFile = fn;
    }
    GetMf()->AddAutoTagList(fn, LineNo, 0);
  }
}

void NavigatorDialog::ClassTreeInfoTip(NMHDR *aNotifyStruct, LRESULT *aResult)
{
    m_classViewTree->OnInfoTip((NMTVGETINFOTIP *)aNotifyStruct);
    *aResult = 0;
}

void NavigatorDialog::ProjTreeInfoTip(NMHDR *aNotifyStruct, LRESULT *aResult)
{
   if (m_projectTree)
   {
      m_projectTree->OnInfoTip((NMTVGETINFOTIP *)aNotifyStruct);
   }
   *aResult = 0;
}

RECT NavigatorDialog::GetListTreeRect()
{
   CRect cr;

   GetClientRect(cr);
   cr.DeflateRect(10, 10);
   cr.bottom -= 4;
   cr.top = 20 + 70;

   return cr;
}
