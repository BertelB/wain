//=============================================================================
// This source code file is a part of Wain.
// It implements TagListDialogClass as defined in TagList.h,
// and TagList() and CompletionList() from WainView.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\wain.h"
#include ".\..\src\wainview.h"
#include ".\..\src\taglist.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\childfrm.h"
#include ".\..\src\waindoc.h"
#include ".\..\src\NavigatorList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4100) // unreferenced formal parameter

ListCtrlClass::ListCtrlClass(TagListDialogClass *dlg) :
   CListCtrl(),
   m_dlg(dlg)
{
}

void ListCtrlClass::OnLButtonDblClk(UINT flags, CPoint point)
{
  m_dlg->OnOk();
}

BEGIN_MESSAGE_MAP(ListCtrlClass, CListCtrl)
  ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP();

BEGIN_MESSAGE_MAP(TagListDialogClass, CDialog)
  ON_BN_CLICKED(IDOK,         OnOk)
  ON_LBN_DBLCLK(IDC_TAG_LIST, OnOk)
  ON_BN_CLICKED(IDC_TAG_LIST_PEEK,  OnPeek)
  ON_BN_CLICKED(IDC_TAG_LIST_TYPE,  OnType)
  ON_BN_CLICKED(IDC_TAG_LIST_CLASS, OnClass)
  ON_BN_CLICKED(IDC_TAG_LIST_PEAK_CLASS, OnPeekClass)
  ON_WM_SIZE()
  ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP();

BEGIN_MESSAGE_MAP(ClassListDialogClass, CDialog)
  ON_BN_CLICKED(IDOK,         OnOk)
  ON_LBN_DBLCLK(IDC_TAG_LIST, OnOk)
  ON_BN_CLICKED(IDC_TAG_LIST_PEEK,  OnPeek)
  ON_BN_CLICKED(IDC_TAG_LIST_TYPE,  OnType)
  ON_BN_CLICKED(IDC_TAG_LIST_CLASS, OnClass)
  ON_BN_CLICKED(IDC_TAG_LIST_PEAK_CLASS, OnPeekClass)
  ON_WM_SIZE()
  ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP();

IMPLEMENT_DYNAMIC(TagListDialogClass, CDialog)
IMPLEMENT_DYNAMIC(ClassListDialogClass, CDialog)

TagListDialogClass::TagListDialogClass(CWnd *aParent, const std::string& aWord,  unsigned int aTypes) :
   CDialog(TagListDialogClass::IDD, aParent),
   m_tagWord(aWord),
   m_types(aTypes)
{
   m_parent = aParent;
   m_tagList = new GetTagListClass(aWord.c_str(), aTypes);
   m_selectedIndex = -1;
   m_listBox = new ListCtrlClass(this);
   m_oldSizeCx = -1;
   m_peekElem = 0;
}

TagListDialogClass::~TagListDialogClass()
{
  delete m_listBox;
  delete m_tagList;
}

void TagListDialogClass::OnCancel()
{
  EndDialog(IDCANCEL);
}

void TagListDialogClass::OnOk()
{
  int nof = m_listBox->GetItemCount();
  int i;
  BOOL found;
  for(i = 0; i < 4; i++)
    wainApp.gs.m_tagListColumnWidth[i] = m_listBox->GetColumnWidth(i);
  RECT cr;
  GetWindowRect(&cr);
  wainApp.gs.m_tagListCx = cr.right - cr.left;
  wainApp.gs.m_tagListCy = cr.bottom - cr.top;
  for(i = 0, found = FALSE; i < nof && !found; i += found ? 0 : 1)
  {
    if(m_listBox->GetItemState(i, LVIS_SELECTED))
      found = TRUE;
  }

  if(found)
  {
    m_selectedIndex = i;
    EndDialog(IDOK);
  }
}

void TagListDialogClass::OnPeek(void)
{
  int nof = m_listBox->GetItemCount();
  int i;
  BOOL found;

  for(i = 0; i < 4; i++)
    wainApp.gs.m_tagListColumnWidth[i] = m_listBox->GetColumnWidth(i);
  RECT cr;
  GetWindowRect(&cr);
  wainApp.gs.m_tagListCx = cr.right - cr.left;
  wainApp.gs.m_tagListCy = cr.bottom - cr.top;

  for(i = 0, found = FALSE; i < nof && !found; i += found ? 0 : 1)
  {
    if(m_listBox->GetItemState(i, LVIS_SELECTED))
      found = TRUE;
  }

  if(found)
  {
    m_selectedIndex = i;
    m_peekElem = new GetTagElemClass(m_tagList->m_list[m_selectedIndex]);
    EndDialog(IDCANCEL);
  }
}

void TagListDialogClass::OnType(void)
{
   bool found = false;
   uint32_t i;
   int nof = m_listBox->GetItemCount();
   for(i = 0, found = false; i < nof && !found; i += found ? 0 : 1)
   {
      if(m_listBox->GetItemState(i, LVIS_SELECTED))
      {
         found = true;
      }
   }

   if(found)
   {
      if (!m_tagList->m_list[i]->m_signature.empty())
      {
         std::string oldTagWord = m_tagWord;
         m_tagWord = m_tagList->m_list[i]->m_signature;
         std::string::size_type pos = m_tagWord.find_first_of(" \t");
         if (pos != std::string::npos)
         {
            m_tagWord = m_tagWord.substr(0, pos);
         }
         delete m_tagList;
         delete m_listBox;
         m_tagList = new GetTagListClass(m_tagWord.c_str(), m_types);
         m_listBox = new ListCtrlClass(this);
         if (DoInitDialog())
         {
            m_tagWord = oldTagWord;
            delete m_tagList;
            delete m_listBox;
            m_tagList = new GetTagListClass(m_tagWord.c_str(), m_types);
        }
      }
   }
}

void TagListDialogClass::OnClass(void)
{
   bool found = false;
   uint32_t i;
   int nof = m_listBox->GetItemCount();
   for(i = 0, found = false; i < nof && !found; i += found ? 0 : 1)
   {
      if(m_listBox->GetItemState(i, LVIS_SELECTED))
      {
         found = true;
      }
   }

   if(found)
   {
      if (!m_tagList->m_list[i]->m_signature.empty())
      {
         std::string  tagWord = m_tagList->m_list[i]->m_signature;
         std::string::size_type pos = tagWord.find_first_of(" \t");
         if (pos != std::string::npos)
         {
            tagWord = tagWord.substr(0, pos);
         }
         if (GetMf()->m_navigatorDialog.ViewClass(tagWord))
         {
            EndDialog(IDCANCEL);
            GetMf()->m_navigatorDialog.SetFocus();
            GetMf()->m_navigatorDialog.m_navigatorList->SetFocus();
         }
      }
   }
}

void TagListDialogClass::OnPeekClass(void)
{
  int nof = m_listBox->GetItemCount();
  int i;
  BOOL found;

  for(i = 0; i < 4; i++)
    wainApp.gs.m_tagListColumnWidth[i] = m_listBox->GetColumnWidth(i);
  RECT cr;
  GetWindowRect(&cr);
  wainApp.gs.m_tagListCx = cr.right - cr.left;
  wainApp.gs.m_tagListCy = cr.bottom - cr.top;

  for(i = 0, found = FALSE; i < nof && !found; i += found ? 0 : 1)
  {
    if(m_listBox->GetItemState(i, LVIS_SELECTED))
      found = TRUE;
  }

  if(found)
  {
    m_selectedIndex = i;
    m_isPeekClass = true;
    m_peekElem = new GetTagElemClass(m_tagList->m_list[m_selectedIndex]);
    EndDialog(IDCANCEL);
  }
}

void TagListDialogClass::DoDataExchange(CDataExchange *dx)
{
  CDialog::DoDataExchange(dx);
}

BOOL TagListDialogClass::OnInitDialog(void)
{
   m_imageList.Create(IDB_SMALLICONS, 16, 1, RGB(255, 255, 255));
   if (DoInitDialog())
   {
      EndDialog(IDCANCEL);
      return TRUE;
   }
   return FALSE;
}

BOOL TagListDialogClass::DoInitDialog()
{
  size_t i;
  if(!GetMf()->m_navigatorDialog.GetTagList(m_tagList))
  {
    SetStatusText("No tags, use the navigator-dialog->tags to setup tags");
    EndDialog(IDCANCEL);
    return TRUE;
  }
  else if(m_types != TL_ANY && m_tagList->m_list.size() == 1)
  {
    m_selectedIndex = 0;
    return TRUE;
  }
  else
  {
    CRect cr;
    GetClientRect(&cr);
    cr.DeflateRect(5, 5);
    cr.bottom -= 40;
    m_listBox->Create(LBS_NOTIFY | LVS_REPORT | WS_VISIBLE | WS_CHILD | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_TABSTOP, cr, this, IDC_TAG_LIST);
    m_listBox->SetExtendedStyle(m_listBox->GetExtendedStyle() | LVS_EX_FULLROWSELECT);
    m_listBox->ModifyStyleEx(0, WS_EX_CLIENTEDGE, FALSE);
    m_listBox->SetImageList(&m_imageList, LVSIL_SMALL);

    m_listBox->InsertColumn(0, "Name");
    m_listBox->InsertColumn(1, "Signature");
    m_listBox->InsertColumn(2, "File");
    m_listBox->InsertColumn(3, "FullPath");
    for(i = 0; i < 4; i++)
      m_listBox->SetColumnWidth(i, wainApp.gs.m_tagListColumnWidth[i]);

    for(i = 0; i < m_tagList->m_list.size(); i++)
    {
      m_listBox->InsertItem(i, m_tagList->m_list[i]->m_tag.c_str(), int(m_tagList->m_list[i]->m_indexType));
      if(m_tagList->m_list[i]->m_signature.size())
        m_listBox->SetItemText(i, 1, m_tagList->m_list[i]->m_signature.c_str());
      m_listBox->SetItemText(i, 2, m_tagList->m_list[i]->m_shortName.c_str());
      m_listBox->SetItemText(i, 3, m_tagList->m_list[i]->m_fullName.c_str());
    }
    if(!i)
    {
      SetStatusText("No matching tag found");
      return TRUE;
    }

    RECT pr;
    GetMf()->GetWindowRect(&pr);

    MoveWindow(pr.left + (pr.right - pr.left)/2 - wainApp.gs.m_tagListCx/2, pr.top + (pr.bottom - pr.top)/2 - wainApp.gs.m_tagListCy/2,
               wainApp.gs.m_tagListCx, wainApp.gs.m_tagListCy);
    m_listBox->SetFocus();
    m_listBox->SetItemState(0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    GetClientRect(&cr);
    m_oldSizeCx = cr.right;
    SetSizes(cr.right, cr.bottom);
    return FALSE;
  }
}

void TagListDialogClass::OnSize(UINT type, int cx, int cy)
{
  SetSizes(cx, cy);
}

void TagListDialogClass::SetSizes(int cx, int cy)
{
  CButton *b = (CButton *)GetDlgItem(IDOK);
  if(!b)
    return;
  b->MoveWindow(10, cy - 30, 70, 22);
  b = (CButton *)GetDlgItem(IDCANCEL);
  ASSERT(b);
  b->MoveWindow(90, cy - 30, 70, 22);
  b = (CButton *)GetDlgItem(IDC_TAG_LIST_PEEK);
  ASSERT(b);
  b->MoveWindow(170, cy - 30, 70, 22);
  b = (CButton *)GetDlgItem(IDC_TAG_LIST_TYPE);
  ASSERT(b);
  b->MoveWindow(250, cy - 30, 70, 22);
  b = (CButton *)GetDlgItem(IDC_TAG_LIST_CLASS);
  ASSERT(b);
  b->MoveWindow(330, cy - 30, 70, 22);
  b = (CButton *)GetDlgItem(IDC_TAG_LIST_PEAK_CLASS);
  ASSERT(b);
  b->MoveWindow(410, cy - 30, 70, 22);

  if(m_listBox)
    m_listBox->MoveWindow(10, 10, cx - 20, cy - 50);
  if(cx != m_oldSizeCx && m_oldSizeCx > 0 && abs(cx - m_oldSizeCx) > 10)
  { /* We need to adjust the column widths */
    int i;

    for(i = 0; i < 4; i++)
    {
      wainApp.gs.m_tagListColumnWidth[i] = (m_listBox->GetColumnWidth(i)*cx + m_oldSizeCx/2)/m_oldSizeCx;
      if(wainApp.gs.m_tagListColumnWidth[i] < 20)
        wainApp.gs.m_tagListColumnWidth[i] = 20;
      else if(wainApp.gs.m_tagListColumnWidth[i] > cx*5/6)
        wainApp.gs.m_tagListColumnWidth[i] = cx*5/6;
      m_listBox->SetColumnWidth(i, wainApp.gs.m_tagListColumnWidth[i]);
    }
    m_oldSizeCx = cx;
  }
}

void TagListDialogClass::OnGetMinMaxInfo(MINMAXINFO FAR *min_max_info)
{
  CDialog::OnGetMinMaxInfo(min_max_info);
  min_max_info->ptMinTrackSize.x = 260;
  min_max_info->ptMinTrackSize.y = 120;
}

void WainView::TagList(void)
{
  TagListXxx(TL_ANY);
}

int operator << (int _n, TagIndexType _type)
{
   return _n << uint32_t(_type);
}

void WainView::TagListClass()
{
  TagListXxx(1 << TagIndexType::CLASS_IDX);
}

void WainView::TagListDefine()
{
  TagListXxx(1 << TagIndexType::DEFINE_IDX);
}

void WainView::TagListType()
{
  TagListXxx(1 << TagIndexType::TYPE_IDX);
}

void WainView::TagListStruct()
{
  TagListXxx(1 << TagIndexType::STRUCT_IDX);
}

void WainView::TagListFunction()
{
  TagListXxx(1 << TagIndexType::FUNCTION_IDX);
}

void WainView::TagListPrototype()
{
  TagListXxx(1 << TagIndexType::PROTOTYPE_IDX);
}

void WainView::TagListEnumName()
{
  TagListXxx(1 << TagIndexType::ENUM_NAME_IDX);
}

void WainView::TagListEnum()
{
  TagListXxx(1 << TagIndexType::ENUM_IDX);
}

void WainView::TagListVar()
{
  TagListXxx(1 << TagIndexType::VAR_IDX);
}

void WainView::TagListExtern()
{
  TagListXxx(1 << TagIndexType::EXTERN_IDX);
}

void WainView::TagListMember()
{
  TagListXxx(1 << TagIndexType::MEMBER_IDX);
}

void WainView::TagListXxx(unsigned int flags)
//  Description:
//    Pops up a dialog with the tags matching the word under the cursor.
//    TagListDialogClass does the works.
//    If he hits Ok Wain will jump to the selected tag, and an Undo entry will be added to the
//    mainframes jump to tag Undo list
//  Parameters:
{
  std::string temp;
  WainDoc *doc = GetDocument();
  if(doc->GetCurrentWord(temp, m_columnNo, m_currentTextLine))
  {
    GetMf()->m_lastTag = temp;
    TagListDialogClass tl(this, temp, flags);
    int res = tl.DoModal();
    if(res == IDOK || res == ID_JUST_ONE)
    {
      if(doc->GetPathName().IsEmpty())
        SetStatusText("Unable to add Undo jump to tag entry");
      else
        GetMf()->AddJumpToTag(doc->GetPathName(), m_lineNo, m_columnNo);
      GetTagElemClass *elem = new GetTagElemClass(tl.m_tagList->m_list[tl.m_selectedIndex]);
      GetMf()->m_navigatorDialog.JumpToTag(elem);
    }
    else if(tl.m_peekElem)
    {
       if (tl.m_isPeekClass)
       {
          TagElemClass tag;
          if (GetMf()->m_navigatorDialog.m_globalTags.m_tagList->FindClass(tag, tl.m_peekElem->m_signature.c_str()))
          {
             GetTagElemClass* getTag = new GetTagElemClass(GetMf()->m_navigatorDialog.m_globalTags.m_fileList->GetFullName(tag.m_fileIdx),
                                                           GetMf()->m_navigatorDialog.m_globalTags.m_fileList->GetShortName(tag.m_fileIdx),
                                                           tag.m_lineNo,
                                                           tag.m_tag.c_str(),
                                                           tag.m_indexType,
                                                           tag.m_signature.c_str());
             GetMf()->AddAutoTagList(getTag->m_fullName.c_str(), getTag->m_lineNo, 0);
             wainApp.SetTagPeek(getTag);
          }
          delete tl.m_peekElem;
          tl.m_peekElem = 0;
       }
       else
       {
         GetMf()->AddAutoTagList(tl.m_peekElem->m_fullName.c_str(), tl.m_peekElem->m_lineNo, 0);
         wainApp.SetTagPeek(tl.m_peekElem);
       }
    }
  }
}

void MainFrame::RedoTag()
{
   if(m_lastTag.empty())
      return;
   TagListDialogClass tl(this, m_lastTag, TL_ANY);
   int res = tl.DoModal();

   if(res == IDOK || res == ID_JUST_ONE)
   {
     WainView *View = (WainView *)GetActiveView();

     if(!View || !View->GetDocument() || View->GetDocument()->GetPathName().IsEmpty())
        SetStatusText("Unable to add Undo jump to tag entry");
     else
        AddJumpToTag(View->GetDocument()->GetPathName(), View->m_lineNo, View->m_columnNo);
     GetTagElemClass *elem = new GetTagElemClass(tl.m_tagList->m_list[tl.m_selectedIndex]);
     m_navigatorDialog.JumpToTag(elem);
   }
   else if(tl.m_peekElem)
   {
      GetMf()->AddAutoTagList(tl.m_peekElem->m_fullName.c_str(), tl.m_peekElem->m_lineNo, 0);
      wainApp.SetTagPeek(tl.m_peekElem);
   }
}

void WainView::GetPopupPos(POINT *p, int NofItems) const
{
  RECT cr;
  GetClientRect(&cr);
  p->x = m_columnNo*m_charWidth;
  int y = m_lineNo*m_lineHeight - m_yOffset;
  p->y = y - NofItems*GetSystemMetrics(SM_CYMENUSIZE);
  if(p->y < 0)
  {
    y++;
    if((cr.bottom - y)/GetSystemMetrics(SM_CYMENUSIZE) > NofItems)
      p->y = y + m_lineHeight;
    else
      p->y = 100;
  }
  ClientToScreen(p);
}

void WainView::CompletionList(void)
{
  if(GetDocument()->GetReadOnly())
  {
    SetStatusText("WordCompletion does not work with read-only files!");
    return;
  }
  std::string word;
  if(!GetDocument()->GetCurrentWord(word, m_columnNo, m_currentTextLine))
  {
    SetStatusText("The cursor is not over a word");
    return;
  }
  CMenu bar;
  if(bar.LoadMenu(IDR_COMP_POPUP_MENU))
  {
    CMenu &popup = *bar.GetSubMenu(0);
    ASSERT(popup.m_hMenu != NULL);
    int pos, n;

    // First, delete all items
    for(pos = popup.GetMenuItemCount()-1; pos >= 0; pos--)
      popup.DeleteMenu(pos, MF_BYPOSITION);

    int num = GetMf()->m_navigatorDialog.GetMatchTags(m_completionString, word);

    for(n = 0; n < TL_MAX_NOF_TAGS && n < num; n++)
    {
      popup.AppendMenu(MF_STRING, IDM_COMP_ITEM0 + n, m_completionString[n].c_str());
    }
    if(num)
    {
      POINT p;
      GetPopupPos(&p, num);
      popup.TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
    }
    else
      SetStatusText("No match found");
  }
}

void WainView::WordCompletion(void)
//  Description:
//    Message handler, build a list of words from words in the current document matching the current word,
//    the user can then select one, which will the replace the current word.
{
  if(GetDocument()->GetReadOnly())
  {
    SetStatusText("WordCompletion does not work with read-only files!");
    return;
  }
  std::string word;
  if(!GetDocument()->GetCurrentWord(word, m_columnNo, m_currentTextLine))
  {
    SetStatusText("The cursor is not over a word");
    return;
  }
  CMenu bar;
  if(bar.LoadMenu(IDR_COMP_POPUP_MENU))
  {
    CMenu &popup = *bar.GetSubMenu(0);
    ASSERT(popup.m_hMenu != NULL);
    int pos, n;

    // First, delete all items
    for(pos = popup.GetMenuItemCount() - 1; pos >= 0; pos--)
      popup.DeleteMenu(pos, MF_BYPOSITION);
    WainDoc *doc = GetDocument();

    TxtLine *l = doc->GetLineNo(0);
    int y = 0;
    int x = 0;
    int j;
    std::string temp;
    BOOL found;
    int flags = SEARCH_FROM_START | SEARCH_IGNORE_CASE;
    for(n = 0; l && n < TL_MAX_NOF_TAGS; )
    {
      l = doc->FindString(l, word.c_str(), &x, &y, flags);
      flags &= ~SEARCH_FROM_START;
      if(l)
      {
        temp = "";
        doc->GetCurrentWord(temp, x, l);
        for(j = 0, found = FALSE; j < n; j++)
         if(temp == m_completionString[j])
           found = TRUE;
        if(!found && temp != word)
        {
          m_completionString[n] = temp;
          popup.AppendMenu(MF_STRING, IDM_COMP_ITEM0 + n, m_completionString[n].c_str());
          n++;
        }
      }
    }
    if(n)
    {
      POINT p;
      GetPopupPos(&p, n);
      popup.TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
    }
    else
      SetStatusText("No match found");
  }
}

void WainView::CompletionFunc(UINT id)
// Called when the user select an item in the completion list.
// Replaces the current word with the selected.
{
  std::string temp;
  GetDocument()->GetCurrentWord(temp, m_columnNo, m_currentTextLine);
  int off = GetDocument()->RemoveWordCurrent(m_columnNo, m_currentTextLine);
  if(off != NOT_A_OFFSET)
  {
    UndoCutTextEntryType *c = new UndoCutTextEntryType;
    c->m_text = new char [temp.size() + 1];
    strcpy(c->m_text, temp.c_str());
    m_columnNo += off;
    c->m_x = m_columnNo;
    c->m_y = m_lineNo;
    m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, off, 0, 0, c);
    m_undoList.AddInsertEntry(m_completionString[id - IDM_COMP_ITEM0].size(), 0, m_columnNo, m_lineNo, m_completionString[id - IDM_COMP_ITEM0].c_str());

    m_currentTextLine->InsertAt(m_columnNo, m_completionString[id - IDM_COMP_ITEM0].c_str(), m_completionString[id - IDM_COMP_ITEM0].size());
    m_columnNo += m_completionString[id - IDM_COMP_ITEM0].size();
    RemoveCursor();
    PutText(NULL, m_currentTextLine, m_lineNo);
    SetCursor();
  }
}

UINT ThreadReadTagFile(LPVOID rp);

void ReadTagsForFile(const std::string& _fileName)
{
   ReadTagParmClass *readParm = new ReadTagParmClass;
   readParm->m_priority = IDLE_PRIORITY_CLASS;
   char command[1024];
   std::string fnExt;
   MySplitPath(_fileName.c_str(), SP_FILE | SP_EXT, fnExt);
   sprintf(command, "c:\\bkb\\uctags\\ctags.exe -u --c-kinds=+px-n --fields=+Si --excmd=number \"-fc:\\bkb\\wain\\temp\\%s.atag\" \"%s\"", fnExt.c_str(), _fileName.c_str());
   readParm->m_command = command;
   sprintf(command, "c:\\bkb\\wain\\temp\\%s.atag", fnExt.c_str());
   readParm->m_fileName = command;
   readParm->m_nr = 0;
   readParm->m_viewNr = 0;
   readParm->m_readTagsMsgId = IDB_AUTO_TAG_DONE;
   AfxBeginThread(ThreadReadTagFile, (LPVOID)readParm, THREAD_PRIORITY_BELOW_NORMAL);
}
