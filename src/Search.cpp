//=============================================================================
// This source code file is a part of Wain.
// It implements SearchDialog_class and ReplaceDialogClass.
// Some Search and Search&Replace functions from MainFrame is implemented
// here as well.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\Wain.h"

#include ".\..\src\WainDoc.h"
#include ".\..\src\WainView.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\WainSearch.h"
#include ".\..\src\childfrm.h"
#include ".\..\src\Project.h"
#include ".\..\src\FileSelD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4100) // unreferenced formal parameter
UINT search_message_id = RegisterWindowMessage("Wain Search Message");
UINT ReplaceMessageId = RegisterWindowMessage("Wain Replace Message");

SearchDialogClass::SearchDialogClass(CWnd *parent) : CDialog(SearchDialogClass::IDD, parent)
{
  m_fromStart = false;
  m_forward = true;
  m_first = true;
  m_ignoreCase = true;
}

SearchDialogClass::~SearchDialogClass()
{
}

IMPLEMENT_DYNAMIC(SearchDialogClass, CDialog)

BEGIN_MESSAGE_MAP(SearchDialogClass, CDialog)
  ON_BN_CLICKED(IDC_SEARCH_NEXT,      SearchNext)
  ON_BN_CLICKED(IDC_SEARCH_FORWARD,   ForwardButton)
  ON_BN_CLICKED(IDC_SEARCH_BACKWARD,  BackwardButton)
  ON_CBN_EDITCHANGE(IDC_SEARCH_COMBO, TextChanged)
END_MESSAGE_MAP();

void SearchDialogClass::OnCancel()
{
  UpdateData(TRUE);
  CComboBox *cb = (CComboBox *)GetDlgItem(IDC_SEARCH_COMBO);
  if(cb)
  {
    CString temp;
    int i = cb->GetCount();
    int j;
    if(i > MAX_NOF_SEARCH_STRINGS - 1)
      i = MAX_NOF_SEARCH_STRINGS - 1;

    wainApp.gs.m_searchStringList.clear();
    for(j = 0; j < i; j++)
    {
      cb->GetLBText(j, temp);
      if(!temp.IsEmpty())
        wainApp.gs.m_searchStringList.push_back((const char *)temp);
    }
    cb->GetWindowText(temp);
    wainApp.gs.m_searchStringList.push_back((const char *)temp);
  }
  WINDOWPLACEMENT place;
  if(GetWindowPlacement(&place))
  {
    AfxGetApp()->WriteProfileInt("Search Dialog", "Size X",      place.rcNormalPosition.left);
    AfxGetApp()->WriteProfileInt("Search Dialog", "Size Y",      place.rcNormalPosition.top);
    AfxGetApp()->WriteProfileInt("Search Dialog", "Size Height", place.rcNormalPosition.bottom - place.rcNormalPosition.top);
    AfxGetApp()->WriteProfileInt("Search Dialog", "Size Width",  place.rcNormalPosition.right - place.rcNormalPosition.left);
  }
  AfxGetApp()->WriteProfileInt("Search Dialog", "IgnoreCase", m_ignoreCase);
  AfxGetApp()->WriteProfileInt("Search Dialog", "UseRegexp", m_useRegex);

  EndDialog(IDOK);

  CDialog::OnCancel();
}

BOOL SearchDialogClass::OnInitDialog()
{
  CComboBox *cb = (CComboBox *)GetDlgItem(IDC_SEARCH_COMBO);
  if(cb)
  {
    std::list<std::string>::iterator idx = wainApp.gs.m_searchStringList.begin();
    for(; idx != wainApp.gs.m_searchStringList.end(); idx++)
      cb->AddString(idx->c_str());
    cb->SetCurSel(0);
  }
  CButton *b = (CButton *)GetDlgItem(IDC_SEARCH_FORWARD);
  if(b)
  {
    b->SetCheck(TRUE);
  }
  int x = AfxGetApp()->GetProfileInt("Search Dialog", "Size X",      0x8000);
  int y = AfxGetApp()->GetProfileInt("Search Dialog", "Size Y",      0x8000);
  int h = AfxGetApp()->GetProfileInt("Search Dialog", "Size Height", 0x8000);
  int w = AfxGetApp()->GetProfileInt("Search Dialog", "Size Width",  0x8000);
  m_ignoreCase = AfxGetApp()->GetProfileInt("Search Dialog", "IgnoreCase", TRUE);
  m_useRegex = AfxGetApp()->GetProfileInt("Search Dialog", "UseRegexp", FALSE);

  if(x != 0x8000 && y != 0x8000 && h != 0x8000 && w != 0x8000)
    MoveWindow(x, y, w, h, FALSE);
  UpdateData(FALSE);
  return FALSE;
}

void SearchDialogClass::DoDataExchange(CDataExchange *dx)
{
  CDialog::DoDataExchange(dx);
  DDX_Check(dx, IDC_SEARCH_START, m_fromStart);
  DDX_Check(dx, IDC_SEARCH_CASE,  m_ignoreCase);
  DDX_Check(dx, IDC_SEARCH_REGEX,  m_useRegex);
}

void SearchDialogClass::SearchNext(void)
{
   UpdateData(TRUE);
   CComboBox *cb = (CComboBox *)GetDlgItem(IDC_SEARCH_COMBO);
   if(cb)
   {
      int index = cb->GetCurSel();
      int sflags = m_forward ? SEARCH_FORWARD : SEARCH_BACKWARD;
      if(m_fromStart)
      {
         sflags |= SEARCH_FROM_START;
         m_fromStart = FALSE;
         CButton *b = (CButton *)GetDlgItem(IDC_SEARCH_START);
         b->SetCheck(FALSE);
      }
      else if(m_first)
      {
         sflags |= SEARCH_FIRST;
      }
      if(m_ignoreCase)
      {
         sflags |= SEARCH_IGNORE_CASE;
      }

      if(index != CB_ERR)
      {
         cb->GetLBText(index, m_searchString);
      }
      else
      {
         cb->GetWindowText(m_searchString);
         if(m_first)
         {
            int c = cb->FindStringExact(-1, m_searchString);
            if(c != CB_ERR)
            {
               cb->DeleteString(c);
            }
            cb->InsertString(0, m_searchString);
         }
      }

      if(m_first && m_useRegex)
      {
         try
         {
            std::regex e(m_searchString);
            m_regEx = e;
         }
         catch (const std::regex_error& err)
         {
            std::string msg("Regex error: ");
            msg += err.what();
            WainMessageBox(GetMf(), msg.c_str(), IDC_MSG_OK, IDI_ERROR_ICO);
            return;
         }
      }
      if(m_useRegex)
      {
         sflags |= SEARCH_USE_REGEX;
      }

      if(!m_searchString.IsEmpty())
      {
         ::SendMessage(GetMf()->m_hWnd, search_message_id, (WPARAM )(const char *)m_searchString, (LPARAM )sflags);
      }

      m_first = false;
   }
}

void SearchDialogClass::ForwardButton(void)
{
  CButton *b = (CButton *)GetDlgItem(IDC_SEARCH_FORWARD);
  if(!b->GetCheck())
  {
    m_forward = TRUE;
    b->SetCheck(TRUE);
    b = (CButton *)GetDlgItem(IDC_SEARCH_BACKWARD);
    b->SetCheck(FALSE);
    SetDlgItemText(IDC_SEARCH_START, "From Start");
  }
}

void SearchDialogClass::BackwardButton(void)
{
  CButton *b = (CButton *)GetDlgItem(IDC_SEARCH_BACKWARD);
  if(!b->GetCheck())
  {
    m_forward = FALSE;
    b->SetCheck(TRUE);
    b = (CButton *)GetDlgItem(IDC_SEARCH_FORWARD);
    b->SetCheck(FALSE);
    SetDlgItemText(IDC_SEARCH_START, "From End");
  }
}

void SearchDialogClass::SetSearchString(const std::string &aStr)
{
  CComboBox *cb = (CComboBox *)GetDlgItem(IDC_SEARCH_COMBO);
  ASSERT(cb);
  int i = cb->FindStringExact(-1, aStr.c_str());
  if(i != CB_ERR)
    cb->SetCurSel(i);
  else if(!aStr.empty())
  {
    i = cb->InsertString(0, aStr.c_str());
    if(i >= 0)
      cb->SetCurSel(i);
  }
  else
  {
    i = cb->GetCount();
    if(i)
      cb->SetCurSel(0);
  }
  m_first = true;
}

void SearchDialogClass::TextChanged(void)
{
  m_first = true;
}


ReplaceDialogClass::ReplaceDialogClass(CWnd *parent) : DialogBaseClass(ReplaceDialogClass::IDD, parent)
{
   m_fromStart = false;
   m_forward = true;
   m_first = true;
   m_isGlobal = false;
}

ReplaceDialogClass::~ReplaceDialogClass()
{
}

void ReplaceDialogClass::SetGlobal(bool aIsGlobal)
{
   if(aIsGlobal)
   {
      SetWindowText("Global Search and Replace");
   }
   else
   {
      SetWindowText("Search and Replace");
   }
   m_isGlobal = aIsGlobal;
   GetDlgItem(IDC_REPLACE_NEXT)->ModifyStyle(WS_DISABLED, 0);
   GetDlgItem(IDC_REPLACE_SKIP)->ModifyStyle(WS_DISABLED, 0);
   GetDlgItem(IDC_REPLACE_ALL)->ModifyStyle(WS_DISABLED, 0);
   InvalidateRect(0, TRUE);

}

IMPLEMENT_DYNAMIC(ReplaceDialogClass, CDialog)

BEGIN_MESSAGE_MAP(ReplaceDialogClass, CDialog)
   ON_BN_CLICKED(IDC_REPLACE_NEXT,      ReplaceNext)
   ON_BN_CLICKED(IDC_REPLACE_SKIP,      ReplaceSkip)
   ON_BN_CLICKED(IDC_REPLACE_FORWARD,   ForwardButton)
   ON_BN_CLICKED(IDC_REPLACE_BACKWARD,  BackwardButton)
   ON_BN_CLICKED(IDC_REPLACE_ALL,       ReplaceAll)
   ON_CBN_EDITCHANGE(IDC_REPLACE_COMBO, TextChanged)
END_MESSAGE_MAP();

void ReplaceDialogClass::OnCancel()
{
  UpdateData(TRUE);
  CComboBox *cb = (CComboBox *)GetDlgItem(IDC_REPLACE_SEARCH_COMBO);
  CString temp;
  if(cb)
  {
    int i = cb->GetCount();
    int j;
    if(i > MAX_NOF_SEARCH_STRINGS - 1)
      i = MAX_NOF_SEARCH_STRINGS - 1;

    wainApp.gs.m_searchStringList.clear();
    for(j = 0; j < i; j++)
    {
      cb->GetLBText(j, temp);
      if(!temp.IsEmpty())
        wainApp.gs.m_searchStringList.push_back((const char *)temp);
    }
    cb->GetWindowText(temp);
    wainApp.gs.m_searchStringList.push_back((const char *)temp);
  }
  cb = (CComboBox *)GetDlgItem(IDC_REPLACE_COMBO);
  if(cb)
  {
    int i = cb->GetCount();
    int j;
    if(i > MAX_NOF_SEARCH_STRINGS - 1)
      i = MAX_NOF_SEARCH_STRINGS - 1;

    wainApp.gs.m_replaceStringList.clear();
    for(j = 0; j < i; j++)
    {
      cb->GetLBText(j, temp);
      if(!temp.IsEmpty())
        wainApp.gs.m_replaceStringList.push_back((const char *)temp);
    }
    cb->GetWindowText(temp);
    wainApp.gs.m_replaceStringList.push_back((const char *)temp);
  }
  WINDOWPLACEMENT place;
  if(GetWindowPlacement(&place))
  {
    AfxGetApp()->WriteProfileInt("Replace Dialog", "Size X",      place.rcNormalPosition.left);
    AfxGetApp()->WriteProfileInt("Replace Dialog", "Size Y",      place.rcNormalPosition.top);
    AfxGetApp()->WriteProfileInt("Replace Dialog", "Size Height", place.rcNormalPosition.bottom - place.rcNormalPosition.top);
    AfxGetApp()->WriteProfileInt("Replace Dialog", "Size Width",  place.rcNormalPosition.right - place.rcNormalPosition.left);
  }
  AfxGetApp()->WriteProfileInt("Search Dialog", "IgnoreCase", m_ignoreCase);
  AfxGetApp()->WriteProfileInt("Search Dialog", "Use regex", m_ignoreCase);

  CDialog::OnCancel();
}

BOOL ReplaceDialogClass::OnInitDialog()
{
  CComboBox *cb = (CComboBox *)GetDlgItem(IDC_REPLACE_SEARCH_COMBO);
  std::list<std::string>::iterator idx;
  if(cb)
  {
    for(idx = wainApp.gs.m_searchStringList.begin(); idx != wainApp.gs.m_searchStringList.end(); idx++)
      cb->AddString(idx->c_str());
    cb->SetCurSel(0);
  }
  cb = (CComboBox *)GetDlgItem(IDC_REPLACE_COMBO);
  if(cb)
  {
    for(idx = wainApp.gs.m_replaceStringList.begin(); idx != wainApp.gs.m_replaceStringList.end(); idx++)
      cb->AddString(idx->c_str());
    cb->SetCurSel(0);
  }
  CButton *b = (CButton *)GetDlgItem(IDC_REPLACE_FORWARD);
  if(b)
  {
    b->SetCheck(TRUE);
  }
  int x = AfxGetApp()->GetProfileInt("Replace Dialog", "Size X",      0x8000);
  int y = AfxGetApp()->GetProfileInt("Replace Dialog", "Size Y",      0x8000);
  // int h = AfxGetApp()->GetProfileInt("Replace Dialog", "Size Height", 0x8000);
  // int w = AfxGetApp()->GetProfileInt("Replace Dialog", "Size Width",  0x8000);
  RECT Rect;
  GetWindowRect(&Rect);
  int h = Rect.bottom - Rect.top;
  int w = Rect.right - Rect.left;
  if(x != 0x8000 && y != 0x8000 && h != 0x8000 && w != 0x8000)
    MoveWindow(x, y, w, h, FALSE);
  m_ignoreCase = wainApp.GetProfileBool("Search Dialog", "IgnoreCase", true);

  UpdateData(FALSE);
  return TRUE;
}

void ReplaceDialogClass::DoDataExchange(CDataExchange *dx)
{
  CDialog::DoDataExchange(dx);
  DdxCheck(dx, IDC_REPLACE_START, m_fromStart);
  DdxCheck(dx, IDC_REPLACE_CASE,  m_ignoreCase);
}

void ReplaceDialogClass::ReplaceNext(void)
{
  DoReplace();
}

void ReplaceDialogClass::GetSearchString()
{
  CComboBox *cb = (CComboBox *)GetDlgItem(IDC_REPLACE_SEARCH_COMBO);

  if(cb)
  {
    int index = cb->GetCurSel();
    if(index != CB_ERR)
    {
      GetListboxString(cb, index, m_searchString);
    }
    else
    {
      GetWindowString(cb, m_searchString);
      int c = cb->FindStringExact(-1, m_searchString.c_str());
      if(c != CB_ERR)
        cb->DeleteString(c);
      cb->InsertString(0, m_searchString.c_str());
    }
  }
}

void ReplaceDialogClass::GetReplaceString(void)
{
  CComboBox *cb = (CComboBox *)GetDlgItem(IDC_REPLACE_COMBO);

  if(cb)
  {
    int index = cb->GetCurSel();
    if(index != CB_ERR)
    {
      GetListboxString(cb, index, m_replaceString);
    }
    else
    {
      GetWindowString(cb, m_replaceString);
      int c = cb->FindStringExact(-1, m_replaceString.c_str());
      if(c != CB_ERR)
        cb->DeleteString(c);
      cb->InsertString(0, m_replaceString.c_str());
    }
  }
}

void ReplaceDialogClass::ReplaceAll(void)
{
   UpdateData(TRUE);
   GetSearchString();
   GetReplaceString();

   int sflags = m_forward ? SEARCH_FORWARD : SEARCH_BACKWARD;
   if(m_ignoreCase)
     sflags |= SEARCH_IGNORE_CASE;
   if(m_first)
     sflags |= SEARCH_FIRST;
   if(m_fromStart)
     sflags |= SEARCH_FROM_START;
   sflags |= REPLACE_ALL;
   const char *s[2];
   s[0] = m_searchString.c_str();
   s[1] = m_replaceString.c_str();

   ReplaceResult replaceResult = (ReplaceResult )::SendMessage(GetMf()->m_hWnd, ReplaceMessageId, (WPARAM )s, sflags);
   m_first = false;
   ClearFromStart();
   if(m_isGlobal && replaceResult == ReplaceResult::ReplaceDone)
   {
      GetDlgItem(IDC_REPLACE_NEXT)->ModifyStyle(0, WS_DISABLED);
      GetDlgItem(IDC_REPLACE_SKIP)->ModifyStyle(0, WS_DISABLED);
      GetDlgItem(IDC_REPLACE_ALL)->ModifyStyle(0, WS_DISABLED);
      InvalidateRect(0, TRUE);
   }
}

void ReplaceDialogClass::ClearFromStart()
{
  CButton *b = (CButton *)GetDlgItem(IDC_REPLACE_START);
  ASSERT(b);
  b->SetCheck(FALSE);
  m_fromStart = false;
}

bool ReplaceDialogClass::DoReplace(void)
{
   UpdateData(TRUE);
   GetSearchString();
   GetReplaceString();
   const char *s[2];

   int sflags = m_forward ? SEARCH_FORWARD : SEARCH_BACKWARD;
   if(m_ignoreCase)
      sflags |= SEARCH_IGNORE_CASE;
   if(m_first)
      sflags |= SEARCH_FIRST;
   if(m_fromStart)
      sflags |= SEARCH_FROM_START;

   s[0] = m_searchString.c_str();
   s[1] = m_replaceString.c_str();

   ReplaceResult replaceResult = (ReplaceResult )::SendMessage(GetMf()->m_hWnd, ReplaceMessageId, (WPARAM )s, sflags);

   if(m_isGlobal && replaceResult == ReplaceResult::ReplaceDone)
   {
      GetDlgItem(IDC_REPLACE_NEXT)->ModifyStyle(0, WS_DISABLED);
      GetDlgItem(IDC_REPLACE_SKIP)->ModifyStyle(0, WS_DISABLED);
      GetDlgItem(IDC_REPLACE_ALL)->ModifyStyle(0, WS_DISABLED);
      InvalidateRect(0, TRUE);
   }

   m_first = false;
   ClearFromStart();
   return true;
}

void ReplaceDialogClass::ReplaceSkip(void)
{
  SearchNext();
}

bool ReplaceDialogClass::SearchNext(void)
{
  UpdateData(TRUE);
  int sflags = m_forward ? SEARCH_FORWARD : SEARCH_BACKWARD;
  if(m_ignoreCase)
    sflags |= SEARCH_IGNORE_CASE;
  if(m_fromStart)
    sflags |= SEARCH_FROM_START;
  if(m_first)
    sflags |= SEARCH_FIRST;
  GetSearchString();

  return ::SendMessage(GetMf()->m_hWnd, search_message_id, (WPARAM )(const char *)m_searchString.c_str(), (LPARAM )sflags) ? true : false;
}

void ReplaceDialogClass::ForwardButton(void)
{
  CButton *b = (CButton *)GetDlgItem(IDC_REPLACE_FORWARD);
  if(!b->GetCheck())
  {
    m_forward = TRUE;
    b->SetCheck(TRUE);
    b = (CButton *)GetDlgItem(IDC_REPLACE_BACKWARD);
    b->SetCheck(FALSE);
    b = (CButton *)GetDlgItem(IDC_REPLACE_START);
    b->SetWindowText("From Start");
  }
}

void ReplaceDialogClass::BackwardButton(void)
{
  CButton *b = (CButton *)GetDlgItem(IDC_REPLACE_BACKWARD);
  if(!b->GetCheck())
  {
    m_forward = FALSE;
    b->SetCheck(TRUE);
    b = (CButton *)GetDlgItem(IDC_REPLACE_FORWARD);
    b->SetCheck(FALSE);
    b = (CButton *)GetDlgItem(IDC_REPLACE_START);
    b->SetWindowText("From End");
  }
}

void ReplaceDialogClass::SetSearchString(const std::string &aStr)
{
   CComboBox *cb = (CComboBox *)GetDlgItem(IDC_REPLACE_SEARCH_COMBO);
   ASSERT(cb);
   int i = cb->FindStringExact(-1, aStr.c_str());
   if(i != CB_ERR)
     cb->SetCurSel(i);
   else if(!aStr.empty())
   {
     i = cb->InsertString(0, aStr.c_str());
     if(i >= 0)
       cb->SetCurSel(i);
   }
   else
   {
     i = cb->GetCount();
     if(i)
       cb->SetCurSel(0);
   }
   m_first = true;
}

void ReplaceDialogClass::TextChanged(void)
{
  m_first = true;
}

void MainFrame::Search(void)
//  Description:
//    Message handler, brings up the search dialog, which will do the work
{
  WainView *av = GetActiveView();
  if(av)
  {
    std::string word;
    av->GetCurrentWord(word);
    if(!m_searchDialog)
    {
      m_searchDialog = new SearchDialogClass();
      if(m_searchDialog && m_searchDialog->Create(IDD_SEARCH_DIALOG, this))
      {
         m_searchDialog->SetSearchString(word);
         m_searchDialog->ShowWindow(SW_SHOW);
      }
    }
    else if(m_searchDialog->IsWindowVisible())
    {
      m_searchDialog->SetSearchString(word);
      ::SetFocus(m_searchDialog->m_hWnd);
    }
    else
    {
      m_searchDialog->SetSearchString(word);
      m_searchDialog->ShowWindow(SW_SHOW);
    }
  }
  else
    SetStatusText("No document to search in");
}

void MainFrame::Replace(void)
//  Description:
//    Message handler, brings up the search dialog, which will do the work
{
  ChildFrame *cf = (ChildFrame *)MDIGetActive();
  if(cf)
  {
    std::string word;
    cf->m_view->GetCurrentWord(word);
    if(!m_replaceDialog)
    {
      m_replaceDialog = new ReplaceDialogClass();
      if(m_replaceDialog && m_replaceDialog->Create(IDD_REPLACE_DIALOG, this))
      {
        m_replaceDialog->SetSearchString(word);
        m_replaceDialog->ShowWindow(SW_SHOW);
      }
    }
    else if(m_replaceDialog->IsWindowVisible())
    {
      m_replaceDialog->SetSearchString(word);
      ::SetFocus(m_replaceDialog->m_hWnd);
    }
    else
    {
      m_replaceDialog->SetSearchString(word);
      m_replaceDialog->ShowWindow(SW_SHOW);
      ::SetFocus(m_replaceDialog->m_hWnd);
    }
     m_replaceDialog->SetGlobal(false);
  }
  else
     SetStatusText("No document to replace in");
   m_replaceIdx = -1;

   m_firstProjectReplace = true;
   m_firstGlobalReplace = true;
}

void MainFrame::ProjectReplace()
{
   m_globalSearchFiles.clear();

   ChildFrame *cf = (ChildFrame *)MDIGetActive();
   if(!m_replaceDialog)
   {
      m_replaceDialog = new ReplaceDialogClass();
      m_replaceDialog->Create(IDD_REPLACE_DIALOG, this);
   }
   m_replaceIdx = 0;
   std::string word = "";
   if(cf)
   {
      cf->m_view->GetCurrentWord(word);
      m_replaceDialog->SetSearchString(word);
   }
   m_replaceDialog->SetGlobal(true);
   m_replaceDialog->ShowWindow(SW_SHOW);
   ::SetFocus(m_replaceDialog->m_hWnd);
   m_firstProjectReplace = true;
   m_firstGlobalReplace = false;
}

void MainFrame::GlobalReplace()
{
   m_globalSearchFiles.clear();
   FileSelectDialogClass FileSelectDialog(this);
   FileSelectDialog.m_caption = "Global Search And Replace";

   if(FileSelectDialog.DoModal())
   {
      size_t n;
      for(n = 0; n < FileSelectDialog.m_fileList.size(); n++)
      {
         m_globalSearchFiles.push_back(FileSelectDialog.m_fileList[n]);
      }
      if(!m_globalSearchFiles.empty())
      {
         ChildFrame *cf = (ChildFrame *)MDIGetActive();
         if(!m_replaceDialog)
         {
            m_replaceDialog = new ReplaceDialogClass();
            m_replaceDialog->Create(IDD_REPLACE_DIALOG, this);
         }
         m_replaceIdx = 0;
         std::string word = "";
         if(cf)
         {
            cf->m_view->GetCurrentWord(word);
            m_replaceDialog->SetSearchString(word);
         }
         m_replaceDialog->SetGlobal(true);
         m_replaceDialog->ShowWindow(SW_SHOW);
         ::SetFocus(m_replaceDialog->m_hWnd);
         m_firstGlobalReplace = true;
         m_firstProjectReplace = true;
         m_replaceIdx = 0;
      }
   }
}


LRESULT MainFrame::SearchFunc(WPARAM wparm, LPARAM lparm)
{
  WainView *av = GetActiveView();
  if(av)
  {
    return av->SearchFunc((const char *)wparm, (unsigned int )lparm);
  }
  SetStatusText("No document to search in");
  return FALSE;
}

LRESULT MainFrame::GlobalReplaceFunc(WPARAM wparm, LPARAM lparm)
{
   if(m_replaceIdx >= 0)
   {
      if(m_firstProjectReplace)
      {
         CWaitCursor WaitCursor;
         size_t Idx;
         const char **Str = (const char **)wparm;
         for(Idx = 0; Idx < m_navigatorDialog.m_project->GetNumFiles(); Idx++)
         {
            std::string Name = m_navigatorDialog.m_project->GetFileName(Idx, false);
            FILE *File = fopen(Name.c_str(), "rt");
            if(File)
            {
               char Line[1024];
               bool Found = false;
               while(!Found && fgets(Line, sizeof(Line), File))
               {
                  if(lparm & SEARCH_IGNORE_CASE)
                  {
                     if(MyStrIStr(Line, Str[0]))
                     {
                        m_globalSearchFiles.push_back(Name);
                        Found = true;
                     }
                  }
                  else
                  {
                     if(strstr(Line, Str[0]))
                     {
                        m_globalSearchFiles.push_back(Name);
                        Found = true;
                     }
                  }
               }
               fclose(File);
            }
         }
         m_firstProjectReplace = false;
         if(m_globalSearchFiles.empty())
         {
            SetStatusText("Search string not found");
            return LRESULT(ReplaceDialogClass::ReplaceResult::ReplaceDone);
         }
         lparm |= SEARCH_FROM_START;
      }
      else if(m_firstGlobalReplace)
      {
         CWaitCursor WaitCursor;
         size_t Idx;
         const char **Str = (const char **)wparm;
         for(Idx = 0; Idx < m_globalSearchFiles.size(); )
         {
            FILE *File = fopen(m_globalSearchFiles[Idx].c_str(), "rt");
            if(File)
            {
               char Line[1024];
               bool Found = false;
               while(!Found && fgets(Line, sizeof(Line), File))
               {
                  if(lparm & SEARCH_IGNORE_CASE)
                  {
                     if(MyStrIStr(Line, Str[0]))
                     {
                        Found = true;
                     }
                  }
                  else
                  {
                     if(strstr(Line, Str[0]))
                     {
                        Found = true;
                     }
                  }
               }
               fclose(File);
               if(Found)
               {
                  Idx++;
               }
               else
               {
                  m_globalSearchFiles.erase(m_globalSearchFiles.begin() + Idx);
               }
            }
         }
         m_firstGlobalReplace = false;
         if(m_globalSearchFiles.empty())
         {
            SetStatusText("Search string not found");
            return LRESULT(ReplaceDialogClass::ReplaceResult::ReplaceDone);
         }
         lparm |= SEARCH_FROM_START;
      }

      WainDoc *Doc;
      bool Found = true;
      do
      {
         Doc = 0;
         ChildFrame *cf;
         const std::string& FileName = m_globalSearchFiles[m_replaceIdx];

         if((cf = (ChildFrame *)MDIGetActive()) != 0)
         {
            if(FileName == (const char *)cf->m_doc->GetPathName())
               Doc = cf->m_doc;
         }

         if(!FileName.empty())
         {
            if(!Doc)
            {
               Doc = wainApp.OpenDocument(FileName.c_str());
            }
            if(Doc)
            {
               Found = Doc->m_view->ReplaceFunc((const char **)wparm, (unsigned int )lparm) ? true : false;
               if(!Found)
               {
                  m_replaceIdx++;
                  if(m_replaceIdx >= m_globalSearchFiles.size())
                  {
                     SetStatusText("Global search Done");
                     m_globalSearchFiles.clear();
                     m_replaceIdx = -1;
                     Doc = 0;
                  }
                  else
                  {
                     lparm |= SEARCH_FROM_START;
                  }
               }
               else
               {
                  return LRESULT(ReplaceDialogClass::ReplaceResult::ReplaceContinue);
               }
            }

         }
      }
      while(Doc && !Found);
      return LRESULT(ReplaceDialogClass::ReplaceResult::ReplaceDone);
   }
   else
   {
      ChildFrame *cf = (ChildFrame *)MDIGetActive();
      if(cf)
      {
         return LRESULT(cf->m_view->ReplaceFunc((const char **)wparm, (unsigned int )lparm) ? ReplaceDialogClass::ReplaceResult::ReplaceContinue : ReplaceDialogClass::ReplaceResult::ReplaceDone);
      }
      else
      {
         SetStatusText("No document to search in");
      }
   }
   return LRESULT(ReplaceDialogClass::ReplaceResult::ReplaceDone);
}
