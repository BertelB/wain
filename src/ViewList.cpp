//=============================================================================
// This source code file is a part of wain.
// It implements ViewList_elem_class, ViewList_class and BookmarkList_dialog_class.
// And some functions from MainFrame related to these.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\Wain.h"
#include ".\..\src\MainFrm.h"
#include ".\..\src\wainview.h"
#include ".\..\src\childfrm.h"
#include ".\..\src\waindoc.h"
#include ".\..\src\tabview.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#pragma warning (disable : 4100) // unreferenced formal parameter

UINT check_req_msg_id = RegisterWindowMessage("Wain FileCheckReq");
UINT CheckStatusMsgId = RegisterWindowMessage("Wain FileCheckStatus");

ViewListElem::ViewListElem()
{
  m_rankNext = m_rankPrev = NULL;
  m_name = "Unnamed";
  m_shortName = "Unnamed";
  m_nr = -1;
  m_myView = NULL;
  m_iconIndex = -1;
  m_ignore = FALSE;
  m_reload = FALSE;
  m_skip = false;
}

ViewListElem::~ViewListElem()
{

}

void ViewListElem::SetName(const char *file_name, bool IsFtpFile)
{
  if(!IsFtpFile)
  {
    SHFILEINFO sh_FileInfo;
    (HIMAGELIST )SHGetFileInfo(file_name, 0, &sh_FileInfo, sizeof(sh_FileInfo), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
    m_iconIndex = sh_FileInfo.iIcon;
  }
  m_name = file_name;
  MySplitPath(m_name.c_str(), SP_FILE | SP_EXT, m_shortName);
  m_ftpFile = IsFtpFile;
}

ViewList::ViewList()
{
  m_list.m_rankNext = m_list.m_rankPrev = &m_list;
  m_currentView = &m_list;
  m_topView[0] = m_topView[1] = NULL;
}

ViewList::~ViewList()
{
  ViewListElem *p;
  while((p = m_list.m_rankNext) != &m_list)
     RemoveView(p->m_nr);
}

int ViewList::AddView(WainView *view, const char *file_name, bool IsFtpFile)
{
  int i = FindUniqueNr();
  ViewListElem *new_elem = new ViewListElem;
  new_elem->m_myView = view;
  new_elem->SetName(file_name, IsFtpFile);
  new_elem->m_nr = i;
  new_elem->m_position = view->GetDocument()->m_childFrame->GetPosition();

  BrowseListType::iterator idx;
  BOOL Found;
  for(idx = m_browseList.begin(), Found = FALSE; idx < m_browseList.end() && !Found; )
  {
    if(stricmp((*idx)->m_shortName.c_str(), new_elem->m_shortName.c_str()) > 0)
      Found = TRUE;
    else
      idx++;
  }
  if(idx == m_browseList.end())
    m_browseList.push_back(new_elem);
  else
    m_browseList.insert(idx, new_elem);

  new_elem->m_rankPrev = m_currentView;
  new_elem->m_rankNext = m_currentView->m_rankNext;
  m_currentView->m_rankNext->m_rankPrev = new_elem;
  m_currentView->m_rankNext = new_elem;
  m_currentView = new_elem;

  new_elem->UpdateStatus();
  return i;
}

#define STAT _stat

void ViewListElem::UpdateStatus(void)
{
   struct STAT status;
   if(!m_name.empty() && !m_myView->GetDocument()->m_newFile && !m_ftpFile && !m_ignore)
   {
      bool ro = access(m_name.c_str(), 2) ? false : true;
      if(STAT(m_name.c_str(), &status))
      {
         WainMessageBox(GetMf(), "Unable to get stat for file", IDC_MSG_OK, IDI_WARNING_ICO);
         m_ignore = true;
         return;
      }
      m_size = status.st_size;
      m_modifiedTime = status.st_mtime;
      m_readOnly = ro;
      m_ignore = m_reload = false;
      m_skip = true;
   }
   else
   {
      m_reload = false;
      m_readOnly = false;
      m_skip = true;
   }
}

void MainFrame::ReenableFileCheck(const WainView *view, bool IsFtpFile)
{
   ViewListElem *elem = NULL;
   while((elem = m_viewList.GetRankNext(elem)) != NULL)
   {
      if(elem->m_myView == view)
      {
         elem->m_ignore = FALSE;
         elem->m_reload = FALSE;
         elem->m_ftpFile = IsFtpFile;
         if(!IsFtpFile)
            elem->UpdateStatus();
         return;
      }
   }
   SetStatusText("View not found");
}

void MainFrame::CheckFileStatus(void)
{
  ViewListElem *elem = NULL;

  FileCheckListType *FileCheckList = new FileCheckListType;

  while((elem = m_viewList.GetRankNext(elem)) != NULL)
  {
    if(!elem->m_ignore &&
       !elem->m_reload &&
       !elem->m_myView->GetDocument()->m_newFile &&
       !elem->m_ftpFile &&
       !elem->m_name.empty())
    {
       FileCheckList->push_back(new FileCheckElem(elem->m_name.c_str()));
    }
  }

  if(FileCheckList->size() && m_fileCheckThread)
    m_fileCheckThread->PostThreadMessage(check_req_msg_id, (WPARAM )FileCheckList, 0);
  else
  {
     delete FileCheckList;
  }
}

void MainFrame::OnFileCheckStatus(FileCheckListType *aFileCheckList)
{
   ViewListElem *elem = NULL;
   size_t i;
   BOOL LoadAll = FALSE;

   while((elem = m_viewList.GetRankNext(elem)) != NULL)
   {
      for(i = 0; i < aFileCheckList->size(); i++)
      {
         if(!aFileCheckList->at(i)->m_fileName.empty() && aFileCheckList->at(i)->m_fileName == elem->m_name)
         {
            if(aFileCheckList->at(i)->m_error)
            {
               elem->m_ignore = TRUE;
               std::string msg = "Unable to get stat for file:\r\n";
               msg += elem->m_name;
               msg += "\r\nThe file Update checking is disabled for this file";
               WainMessageBox(this, msg.c_str(), IDC_MSG_OK, IDI_INFORMATION_ICO);
            }
            else if(elem->m_skip)
            {
               elem->UpdateStatus();
               elem->m_skip = false;
            }
            else if(elem->m_ignore)
            {
            }
            else if(aFileCheckList->at(i)->m_size != elem->m_size ||
                    aFileCheckList->at(i)->m_modifiedTime != elem->m_modifiedTime ||
                    aFileCheckList->at(i)->m_readOnly != elem->m_readOnly)
            {
               elem->m_ignore = true;  // Ignore futher check until the user has pressed YES
               std::string Msg = "The file: ";
               Msg += elem->m_name;
               Msg += "\r\nHas been changed by someone, reload?";
               WORD Status = IDC_MSG_NO;
               if(LoadAll || (Status = WainMessageBox(this, Msg.c_str(), IDC_MSG_YES | IDC_MSG_NO | IDC_MSG_CUST, IDI_QUESTION_ICO, "All")) != IDC_MSG_NO)
               {
                  if(Status == IDC_MSG_CUST)
                     LoadAll = true;
                  elem->m_myView->ReloadFile();
                  elem->UpdateStatus(); /* In case the file has been modified while the message box was displayed */
                  elem->m_ignore = false;
               }
            }
            aFileCheckList->at(i)->m_fileName = "";
         }
      }
   }
   delete aFileCheckList;
}

BOOL ViewList::RemoveView(int nr)
{
  ViewListElem *elem;
  elem = GetViewNr(nr);
  if(elem)
  {
    if(elem == m_currentView)
    {
      m_currentView = elem->m_rankPrev;
      if(m_currentView == &m_list)
        m_currentView = m_currentView->m_rankPrev;
      if(m_currentView == elem)
        m_currentView = &m_list;
    }
    ASSERT(elem->m_position < 2);
    if(m_topView[elem->m_position] == elem)
    {
      ViewListElem *p = elem->m_rankPrev;
      if(p == &m_list)
        p = p->m_rankPrev;
      while(p->m_position != elem->m_position && p != elem)
      {
        p = p->m_rankPrev;
        if(p == &m_list)
          p = p->m_rankPrev;
      }
      if(p == elem)
      { // There is no other view in this position
        m_topView[elem->m_position] = NULL;
      }
      else
        m_topView[elem->m_position] = p;
    }
    elem->m_rankNext->m_rankPrev = elem->m_rankPrev;
    elem->m_rankPrev->m_rankNext = elem->m_rankNext;
    BrowseListType::iterator i = std::find(m_browseList.begin(), m_browseList.end(), elem);
    if(i != m_browseList.end())
      m_browseList.erase(i);
    else
      SetStatusText("Failed to find elem");
    delete elem;
    return TRUE;
  }
  return FALSE;
}

void ViewList::SetViewName(int nr, const char *name, bool IsFtpFile)
{
  ViewListElem *elem = GetViewNr(nr);
  if(elem)
    elem->SetName(name, IsFtpFile);
}

ViewListElem *ViewList::GetRankNext(ViewListElem *elem)
{
  if(!elem)
  {
    elem = m_list.m_rankNext;
  }
  else
    elem = elem->m_rankNext;
  if(elem == &m_list)
    return NULL;
  return elem;
}

ViewListElem *ViewList::GetRankPrev(ViewListElem *elem)
{
  if(!elem)
  {
    elem = m_list.m_rankPrev;
  }
  else
    elem = elem->m_rankPrev;
  if(elem == &m_list)
    return NULL;
  return elem;
}

const ViewListElem *ViewList::GetAbsNr(int nr)
{
  if(nr >= int(m_browseList.size()) || nr < 0)
    return 0;
  return m_browseList[nr];
}

int ViewList::FindUniqueNr(void)
{
  int i;
  BOOL used;
  ViewListElem *elem;
  for(i = 0; i < 256; i++)
  {
    elem = NULL;
    used = FALSE;
    while((elem = GetRankNext(elem)) != NULL && !used)
      if(elem->m_nr == i)
        used = TRUE;
    if(!used)
      return i;
  }
  return -1;
}

ViewListElem *ViewList::GetViewNr(int nr)
{
  ViewListElem *elem = NULL;
  while((elem = GetRankNext(elem)) != NULL)
    if(elem->m_nr == nr)
      return elem;

  return NULL;
}

ViewListElem *ViewList::PutInRankTop(int nr)
{
  ViewListElem *elem = NULL;
  while((elem = GetRankNext(elem)) != NULL)
  {
    if(elem->m_nr == nr)
    {
      if(elem != m_currentView)
      {
        elem->m_rankNext->m_rankPrev = elem->m_rankPrev;
        elem->m_rankPrev->m_rankNext = elem->m_rankNext;
        elem->m_rankPrev = m_currentView;
        elem->m_rankNext = m_currentView->m_rankNext;
        m_currentView->m_rankNext->m_rankPrev = elem;
        m_currentView->m_rankNext = elem;
        m_currentView = elem;
      }
      ASSERT(elem->m_position < 2);
      m_topView[elem->m_position] = elem;

      return elem;
    }
  }
  return NULL;
}

BookmarkListDialogClass::BookmarkListDialogClass(CWnd *parent) : CDialog(BookmarkListDialogClass::IDD, parent)
{
   m_mf = GetMf();
}

BookmarkListDialogClass::~BookmarkListDialogClass()
{

}

BOOL BookmarkListDialogClass::OnInitDialog(void)
{
  CListBox *lb = (CListBox *)GetDlgItem(IDC_BOOKMARK_LIST);
  ASSERT(lb);
  int i, j;
  char buff[_MAX_PATH + 100];
  memset(m_map, 0, sizeof(m_map));
  for(i = 0, j = 0; i < NOF_BOOKMARKS; i++)
  {
    if(m_mf->m_bookmark[i].m_view)
    {
      const char *filename = m_mf->m_bookmark[i].m_view->GetDocument()->GetPathName();
      sprintf(buff, "%d - %s", m_mf->m_bookmark[i].m_lineNo + 1, filename);
      lb->AddString(buff);
      m_map[j++] = i;
    }
  }
  if(j)
    lb->SetCurSel(0);
  lb->SetFocus();
  return FALSE;
}

void BookmarkListDialogClass::OnOk(void)
{
  CListBox *lb = (CListBox *)GetDlgItem(IDC_BOOKMARK_LIST);
  ASSERT(lb);
  m_sel = lb->GetCurSel();
  if(m_sel != LB_ERR)
    m_sel = m_map[m_sel];
  EndDialog(IDOK);
}

IMPLEMENT_DYNAMIC(BookmarkListDialogClass, CDialog)

BEGIN_MESSAGE_MAP(BookmarkListDialogClass, CDialog)
  ON_BN_CLICKED(IDOK, OnOk)
END_MESSAGE_MAP();

void MainFrame::SetBookmark(UINT id)
{
  int b_mark = id - IDM_SET_BOOKMARK_0;
  ASSERT(b_mark >= 0 && b_mark < 10);
  ChildFrame *active_child = (ChildFrame *)MDIGetActive();
  if(active_child)
  {
    AddBookmark(active_child->GetView(), active_child->GetView()->m_columnNo, active_child->GetView()->m_lineNo, b_mark);
  }
  else
    SetStatusText("No active view");
}

void MainFrame::JumpToBookmark(UINT id)
{
  int b_mark = id - IDM_JUMP_TO_BOOKMARK_0;
  ASSERT(b_mark >= 0 && b_mark < NOF_BOOKMARKS);
  GotoBookmark(b_mark);
}

void MainFrame::AddBookmark( WainView *view, int column, int LineNo, int Bookmark_no)
{
  /* First remove the Bookmark in the tabview */
  if(m_bookmark[Bookmark_no].m_view && m_bookmark[Bookmark_no].m_view->m_tabView)
    m_bookmark[Bookmark_no].m_view->m_tabView->SetBookmark(Bookmark_no, -1);
  /* Then set the Bookmark in the new tabview */
  if(view->m_tabView)
    view->m_tabView->SetBookmark(Bookmark_no, LineNo);
  m_bookmark[Bookmark_no].m_view = view;
  m_bookmark[Bookmark_no].m_lineNo = LineNo;
  m_bookmark[Bookmark_no].m_column = column;
  ::SetStatusText("Added Bookmark: %d", Bookmark_no);
}

void MainFrame::GotoBookmark(int Bookmark_no)
{
  if(m_bookmark[Bookmark_no].m_view)
  {
    m_bookmark[Bookmark_no].m_view->GetDocument()->m_childFrame->MDIActivate();
    m_bookmark[Bookmark_no].m_view->GotoLineNo(m_bookmark[Bookmark_no].m_lineNo, m_bookmark[Bookmark_no].m_column);
    ::SetStatusText("At Bookmark: %d", Bookmark_no);
  }
  else
    ::SetStatusText("No such Bookmark");
}

void MainFrame::BookmarkList(void)
{
   ChildFrame *cf = (ChildFrame *)MDIGetActive();
   BookmarkListDialogClass bl(this);
   if(bl.DoModal() == IDOK && bl.m_sel != LB_ERR)
   {
      GotoBookmark(bl.m_sel);
   }
   else if(cf)
   {
      cf->MDIActivate();
      cf->GetView()->SetFocus();
   }
}

int MainFrame::AddView(WainView *view, const char *name, bool IsFtpFile)
/******************************************************************************
;*  Description:
;*    Return the new window's WinNr
;******************************************************************************/
{
  SetupMenu();
  int ret = m_viewList.AddView(view, name, IsFtpFile);
  m_navigatorDialog.m_fileList.UpdateFileList();
  return ret;
}

void MainFrame::SetViewName(int nr, const char *name, bool IsFtpFile)
{
  ViewListElem *elem;
  elem = m_viewList.GetViewNr(nr);
  ASSERT(elem);

  BrowseListType::iterator idx = std::find(m_viewList.m_browseList.begin(), m_viewList.m_browseList.end(), elem);
  if(idx != m_viewList.m_browseList.end())
    m_viewList.m_browseList.erase(idx);
  else
    SetStatusText("Did not find view");

  elem->SetName(name, IsFtpFile);

  BOOL Found;
  for(idx = m_viewList.m_browseList.begin(), Found = FALSE; idx < m_viewList.m_browseList.end() && !Found; )
  {
    if(stricmp((*idx)->m_shortName.c_str(), elem->m_shortName.c_str()) > 0)
      Found = TRUE;
    else
      idx++;
  }
  if(idx == m_viewList.m_browseList.end())
    m_viewList.m_browseList.push_back(elem);
  else
    m_viewList.m_browseList.insert(idx, elem);

  m_navigatorDialog.m_fileList.UpdateFileList();
}

void MainFrame::RemoveView(int nr)
{
  ViewListElem *e = m_viewList.GetViewNr(nr);

  for(int i = 0; e != NULL && i < NOF_BOOKMARKS; i++)
    if(e->m_myView == m_bookmark[i].m_view)
    {
      m_bookmark[i].m_view = NULL;
    }
  if(!m_viewList.RemoveView(nr))
    WainMessageBox(this, "View not found!", IDC_MSG_OK, IDI_ERROR_ICO);

  m_navigatorDialog.m_fileList.UpdateFileList();
  SetupMenu();
}

void MainFrame::NextWin(void)
{
   WainView *av = m_pageBarDialog.GetActiveView();
   if(av && GetFocus() == av)
   {
      m_pageBarDialog.NextView();
   }
   else
   {
      if(m_viewList.m_currentView == &m_viewList.m_list)
         return;
      m_viewList.m_currentView = m_viewList.m_currentView->m_rankNext;
      if(m_viewList.m_currentView == &m_viewList.m_list)
         m_viewList.m_currentView = m_viewList.m_currentView->m_rankNext;
      m_updateList = false;
      m_viewList.m_currentView->m_myView->GetDocument()->m_childFrame->MDIActivate();
      m_updateList = true;
   }
}

void MainFrame::PrevWin(void)
{
  WainView *av = m_pageBarDialog.GetActiveView();
  if(av && GetFocus() == av)
  {
    m_pageBarDialog.PrevView();
  }
  else
  {
    if(m_viewList.m_currentView == &m_viewList.m_list)
      return;
    m_viewList.m_currentView = m_viewList.m_currentView->m_rankPrev;
    if(m_viewList.m_currentView == &m_viewList.m_list)
      m_viewList.m_currentView = m_viewList.m_currentView->m_rankPrev;
    m_updateList = false;
    m_viewList.m_currentView->m_myView->GetDocument()->m_childFrame->MDIActivate();
    m_updateList = true;
  }
}

void MainFrame::SwapWin(void)
{
  ChildFrame *active_child = (ChildFrame *)MDIGetActive();
  if(!active_child || active_child->GetDisplayMode() != CreateModeType::CREATE_DUAL)
    return;
  ASSERT(active_child->GetPosition() < 2);
  ViewListElem *elem = m_viewList.GetViewNr(active_child->GetView()->m_winNr);
  ASSERT(m_viewList.m_topView[active_child->GetPosition()] == elem);
  ASSERT(m_viewList.m_currentView == elem);

  ViewListElem *p = elem->m_rankPrev;
  if(p == &m_viewList.m_list)
    p = p->m_rankPrev;
  while(p->m_position != elem->m_position && p != elem)
  {
    p = p->m_rankPrev;
    if(p == &m_viewList.m_list)
      p = p->m_rankPrev;
  }
  if(p != elem)
  {
    m_viewList.m_topView[elem->m_position] = p;
  }
  else
  {
    m_viewList.m_topView[elem->m_position] = NULL;
  }
  active_child->TogglePosition();
  m_viewList.m_currentView->m_position ^= 1;
  m_viewList.m_topView[elem->m_position] = elem;
  RECT r;
  CalcWinRect(&r, active_child->GetPosition());
  active_child->MoveWindow(&r);
}

void MainFrame::OtherWin(void)
{
  ChildFrame *active_child = (ChildFrame *)MDIGetActive();
  if(!active_child || active_child->GetDisplayMode() != CreateModeType::CREATE_DUAL)
    return;
  ASSERT(active_child->GetPosition() < 2);
  if(m_viewList.m_topView[active_child->GetPosition() ^ 1])
    m_viewList.m_topView[active_child->GetPosition() ^ 1]->m_myView->GetDocument()->m_childFrame->MDIActivate();
}

void MainFrame::NextWinHere(void)
{
  if(m_viewList.m_currentView == &m_viewList.m_list)
    return;
  ChildFrame *active_child = (ChildFrame *)MDIGetActive();
  if(active_child->GetDisplayMode() != CreateModeType::CREATE_DUAL)
  {
    NextWin();
    return;
  }

  ViewListElem *top_elem = m_viewList.m_topView[m_viewList.m_currentView->m_position ^ 1];
  if(top_elem == NULL)
  {
    NextWin();
    return;
  }

  // Now find the prev win != top win
  ViewListElem *elem = m_viewList.m_currentView->m_rankNext;
  if(elem == &m_viewList.m_list)
    elem = elem->m_rankNext;
  while(elem == top_elem && elem != m_viewList.m_currentView)
  {
    elem = elem->m_rankNext;
    if(elem == &m_viewList.m_list)
      elem = elem->m_rankNext;
  }
  if(elem == m_viewList.m_currentView)
    return;

  WainDoc *doc;
  doc = elem->m_myView->GetDocument();
  RECT r;
  doc->m_childFrame->SetPosition(m_viewList.m_currentView->GetPosition());
  elem->SetPosition(m_viewList.m_currentView->GetPosition());
  CalcWinRect(&r, doc->m_childFrame->GetPosition());
  m_updateList = FALSE;
  doc->m_childFrame->MoveWindow(&r);
  m_updateList = FALSE;
  doc->m_childFrame->MDIActivate();
}

void MainFrame::PrevWinHere(void)
{
  if(m_viewList.m_currentView == &m_viewList.m_list)
    return;
  ChildFrame *active_child = (ChildFrame *)MDIGetActive();
  if(active_child->GetDisplayMode() != CreateModeType::CREATE_DUAL)
  {
    NextWin();
    return;
  }

  ViewListElem *top_elem = m_viewList.m_topView[m_viewList.m_currentView->GetPosition() ^ 1];
  if(top_elem == NULL)
  {
    NextWin();
    return;
  }

  // Now find the prev win != top win
  ViewListElem *elem = m_viewList.m_currentView->m_rankPrev;
  if(elem == &m_viewList.m_list)
    elem = elem->m_rankPrev;
  while(elem == top_elem && elem != m_viewList.m_currentView)
  {
    elem = elem->m_rankPrev;
    if(elem == &m_viewList.m_list)
      elem = elem->m_rankPrev;
  }
  if(elem == m_viewList.m_currentView)
    return;

  WainDoc *doc;
  doc = elem->m_myView->GetDocument();
  RECT r;
  doc->m_childFrame->SetPosition(m_viewList.m_currentView->GetPosition());
  elem->SetPosition(m_viewList.m_currentView->GetPosition());
  CalcWinRect(&r, doc->m_childFrame->GetPosition());
  m_updateList = FALSE;
  doc->m_childFrame->MoveWindow(&r);
  m_updateList = FALSE;
  doc->m_childFrame->MDIActivate();
}

void MainFrame::GotoOtherView(void)
{
  WainView *av = m_pageBarDialog.GetActiveView();
  if(av && GetFocus() == av)
  { // We are in the page bar
    ChildFrame *cf = (ChildFrame *)MDIGetActive();
    if(cf)
      SetActiveView(cf->GetView());
  }
  else if(av)
    SetActiveView(av);
}

void MainFrame::UpdateViews(BOOL hard)
{
  if(!hard)
  {
    if(m_viewList.m_topView[0] && m_viewList.m_topView[1])
    {
      m_viewList.m_topView[0]->m_myView->InvalidateRect(NULL, TRUE);
      m_viewList.m_topView[1]->m_myView->InvalidateRect(NULL, TRUE);
    }
    else
    {
      ChildFrame *cf = (ChildFrame *)MDIGetActive();
      if(cf && cf->GetView())
        cf->GetView()->InvalidateRect(NULL, TRUE);
    }
  }
  else
  {
    ViewListElem *elem = NULL;
    while((elem = m_viewList.GetRankNext(elem)) != NULL)
    {
      elem->m_myView->GetDocument()->GetExtType(NULL, TRUE);
    }
  }
  m_pageBarDialog.UpdateViews(hard);
}

#define STAT _stat

UINT FileCheckThreadFunc(LPVOID parm)
{
   MSG msg;
   do
   {
      switch(GetMessage(&msg, NULL, 0, 0))
      {
      case 0: /* WM_QUIT */
         return 0;
      case -1:   /* Fatal error */
         return 1;
      default: /* Normal case */
         FileCheckListType *fc;
         fc = (FileCheckListType *)msg.wParam;
         if(fc)
         {
            size_t i;
            struct STAT status;

            for(i = 0; i < fc->size(); i++)
            {
               fc->at(i)->m_error = false;
               if(!STAT(fc->at(i)->m_fileName.c_str(), &status))
               {
                  bool ro = access(fc->at(i)->m_fileName.c_str(), 2) ? false : true;
                  fc->at(i)->m_modifiedTime = status.st_mtime;
                  fc->at(i)->m_readOnly = ro;
                  fc->at(i)->m_size = status.st_size;
                  fc->at(i)->m_error = false;
               }
               else
                  fc->at(i)->m_error = true;
            }
            wainApp.PostThreadMessage(CheckStatusMsgId, (WPARAM )fc, 0);
        }
        break;
     }
   }
   while(1);
}
