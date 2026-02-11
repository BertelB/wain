//=============================================================================
// This source code file is a part of Wain.
// It implements
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\wain.h"
#include ".\..\src\wainview.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\navigatorDlg.h"
#include ".\..\src\Project.h"
#include ".\..\src\TagList.h"
#include ".\..\src\Navigatorlist.h"
#include ".\..\src\waindoc.h"
#include "childfrm.h"
#include "WainUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

AddClassView::AddClassView() : m_addInfo(0), m_view(0)
{
  m_imageList.Create(IDB_SMALLICONS, 16, 1, RGB(255, 255, 255));
}

AddClassView::~AddClassView()
{
   ClearTree();
   Clear();
}

void AddClassView::SetList(AddClassInfoListClass* _addInfo, TagFileList* _fileList, AddClassInfoTreeCtrlClass* _view)
{
   Clear();
   m_addInfo = _addInfo;
   m_fileList = _fileList;
   m_view = _view;
   if(m_view)
   {
      m_view->m_addClassView = this;
      m_view->SetImageList(&m_imageList, TVSIL_NORMAL);
   }
}

void AddClassView::Clear()
{
   delete m_addInfo;
   m_addInfo = 0;
}

void AddClassView::ClearTree(void)
{
   if(!m_view || !::IsWindow(m_view->m_hWnd))
      return;

   m_view->DeleteAllItems();
};

void AddClassView::OnClose()
{
   ClearTree();
   Clear();
   m_view = 0;
}

AddClassInfoClass::~AddClassInfoClass()
{
  ListClass::size_type i;
  for(i = 0; i < m_list.size(); i++)
    delete m_list[i];
}

class AddClassInfoSortClass
{
public:
   AddClassInfoSortClass() {}
   bool operator () (const AddClassElementInfoClass *lhs, const AddClassElementInfoClass *rhs)
   {
      if(lhs->m_indexType == TagIndexType::INHERITANCE_IDX && rhs->m_indexType != TagIndexType::INHERITANCE_IDX)
         return true;
      if(lhs->m_indexType != TagIndexType::INHERITANCE_IDX && rhs->m_indexType == TagIndexType::INHERITANCE_IDX)
         return false;
      if(lhs->m_indexType != rhs->m_indexType)
         return lhs->m_indexType < rhs->m_indexType;
      return stricmp(lhs->m_tag.c_str(), rhs->m_tag.c_str()) < 0;
   }
};

void AddClassInfoClass::Sort(void)
{
   AddClassInfoSortClass Compare;
   std::sort(m_list.begin(), m_list.end(), Compare);
}

AddClassInfoListClass::AddClassInfoListClass()
{
}

AddClassInfoListClass::~AddClassInfoListClass()
{
  ListClass::size_type i;
  for(i = 0; i < m_list.size(); i++)
    delete m_list[i];
  m_list.clear();
}

AddClassElementInfoClass * AddClassInfoListClass::Add(const std::string &aClassName, const std::string &aTagName, TagIndexType _indexType, int aFileNo, int aLineNo, bool aIsStruct)
{
   AddClassInfoClass *add_class_info = FindClass(aClassName);
   if(!add_class_info)
   {
      add_class_info = new AddClassInfoClass(aClassName, aFileNo, aLineNo, aIsStruct);
      m_list.push_back(add_class_info);
   }

   return add_class_info->Add(aTagName, _indexType, aFileNo, aLineNo);
}

void AddClassInfoListClass::Remove(const std::string& aName)
{
   ListClass::iterator it;
   for(it = m_list.begin(); it != m_list.end(); ++it)
   {
      if((*it)->m_className == aName)
      {
         delete *it;
         m_list.erase(it);
         return;
      }
   }
}

void AddClassInfoListClass::SetClassInfo(const std::string &aClassName, int aFileNo, int aLineNo, bool aIsStruct)
{
  AddClassInfoClass *add_class_info = FindClass(aClassName);
  if(!add_class_info)
  {
    add_class_info = new AddClassInfoClass(aClassName, aFileNo, aLineNo, aIsStruct);
    m_list.push_back(add_class_info);
  }
  else
  {
    add_class_info->m_fileIdx = aFileNo;
    add_class_info->m_lineNo = aLineNo;
  }
}

void AddClassInfoListClass::InitTree(CTreeCtrl *aTree)
{
   try
   {
      if(!aTree->m_hWnd || !IsWindow(aTree->m_hWnd))
         return;

      ListClass::size_type i, j;
      for(i = 0; i < m_list.size(); i++)
      {
         HTREEITEM Parent = 0;
         TVINSERTSTRUCT ParentInsert;
         ParentInsert.hParent = 0;
         ParentInsert.hInsertAfter = 0;
         ParentInsert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
         ParentInsert.item.pszText = (char *)m_list[i]->m_className.c_str();
         ParentInsert.item.iImage = int(m_list[i]->m_isStruct ? TagIndexType::STRUCT_IDX : TagIndexType::CLASS_IDX);
         ParentInsert.item.iSelectedImage = int(m_list[i]->m_isStruct ? TagIndexType::STRUCT_IDX : TagIndexType::CLASS_IDX);
         ParentInsert.item.lParam = i | 0x80000000U;
         if(!aTree->m_hWnd || !IsWindow(aTree->m_hWnd))
            return;
         Parent = aTree->InsertItem(&ParentInsert);
         m_list[i]->m_treeItem = Parent;

         for(j = 0; j < m_list[i]->m_list.size(); j++)
         {
            TVINSERTSTRUCT Insert;
            Insert.hParent = Parent;
            Insert.hInsertAfter = 0;
            Insert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
            Insert.item.iImage = int(m_list[i]->m_list[j]->m_indexType);
            Insert.item.iSelectedImage = int(m_list[i]->m_list[j]->m_indexType);
            Insert.item.pszText = (char *)m_list[i]->m_list[j]->m_tag.c_str();
            Insert.item.lParam = (i << 16) | j;
            Insert.hParent = Parent;
            if(!aTree->m_hWnd || !IsWindow(aTree->m_hWnd))
               return;
            aTree->InsertItem(&Insert);
         }
      }
   }
   catch (...)
   {
      ::MessageBox(0, "InitTree - Catched exception", "Wain", MB_OK);
   }
}

AddClassInfoClass *AddClassInfoListClass::FindClass(const std::string &aClassName)
{
   ListClass::size_type i;
   for(i = 0; i < m_list.size(); i++)
      if(aClassName == m_list[i]->m_className)
         return m_list[i];
   return 0;
}

class AddClassInfoListCompareClass
{
public:
   AddClassInfoListCompareClass(){}
   bool operator () (const AddClassInfoClass *lhs, const AddClassInfoClass *rhs)
   {
      return stricmp(lhs->m_className.c_str(), rhs->m_className.c_str()) < 0;
   }
};

void AddClassInfoListClass::Sort(void)
{
  ListClass::size_type i;
  for(i = 0; i < m_list.size(); i++)
    m_list[i]->Sort();
  AddClassInfoListCompareClass Compare;
  std::sort(m_list.begin(), m_list.end(), Compare);
}

AddClassElementInfoClass::~AddClassElementInfoClass()
{
}

void AddClassElementInfoClass::SetSignature(const std::string &aStr)
{
   m_signature = aStr;
}

void NavigatorDialog::SetClassList(AddClassInfoListClass *aAddInfo, TagFileList *aFileList, AddClassInfoTreeCtrlClass *aView)
{
   // aView->SetFont(&m_listFont);
   m_addClassView.SetList(aAddInfo, aFileList, aView);
   CTreeCtrl *tt = m_classViewTree;
   m_classViewTree = aView;
   ASSERT(::IsWindow(m_classViewTree->m_hWnd));

   CRect tr = GetListTreeRect();
   m_classViewTree->MoveWindow(tr, FALSE);
   tt->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);
   if(m_navBarState == NavBarState::Class)
   {
     m_classViewTree->ModifyStyle(WS_DISABLED, WS_VISIBLE, SWP_NOACTIVATE | SWP_NOZORDER);
     InvalidateRect(NULL, TRUE);
   }
   delete tt;
}

BEGIN_MESSAGE_MAP(AddClassInfoTreeCtrlClass, CTreeCtrl)
  ON_WM_RBUTTONDOWN()
  ON_WM_LBUTTONDBLCLK()
  ON_COMMAND(IDC_CT_VIEW_POPUP, PopupMenu)
  ON_COMMAND(IDC_CT_TAG_PEEK,  TagPeek)
  ON_COMMAND(IDC_CT_TAG_SELECT,  TagSelect)
  ON_COMMAND(IDC_CT_TAG_EXPAND,  TagExpand)
  ON_COMMAND(IDC_CT_EDIT, GotoEditor)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP();

void AddClassInfoTreeCtrlClass::OnRButtonDown(UINT /* flags */, CPoint point)
{
  UINT ItemFlags;
  DoPopUp(HitTest(point, &ItemFlags), point);
}

void AddClassInfoTreeCtrlClass::OnLButtonDblClk(UINT /* flags */, CPoint /* point */)
{
  DoSelect();
}

AddClassInfoTreeCtrlClass::AddClassInfoTreeCtrlClass(class NavigatorDialog *Parent)
{
   m_addClassView = 0;
   m_accHandle = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LIST_TREE));
   m_dlg = Parent;
   // SetFont(&m_dlg->GetListFont());
}

void AddClassInfoTreeCtrlClass::OnInfoTip(NMTVGETINFOTIP *InfoTip)
{
  DWORD data =  InfoTip->lParam;
  if(data & 0x80000000U)
  { // This is a class name
    m_classIndex = data & ~0x80000000U;
    m_memberIndex = 0xFFFFFFFFU;
  }
  else
  { // a memeber
    unsigned int ci = data >> 16;
    unsigned int mi = data & 0xFFFFU;
    if(!m_addClassView->m_addInfo->m_list[ci]->m_list[mi]->m_signature.empty())
    {
      strcpy(InfoTip->pszText, m_addClassView->m_addInfo->m_list[ci]->m_list[mi]->m_tag.c_str());
      strcat(InfoTip->pszText, m_addClassView->m_addInfo->m_list[ci]->m_list[mi]->m_signature.c_str());
    }
  }
}

bool AddClassInfoTreeCtrlClass::FindClassName(std::string &aName)
{
  AddClassInfoClass *class_info;
  if((class_info = m_addClassView->m_addInfo->FindClass(aName)) != 0)
  {
     SelectItem(class_info->m_treeItem);
     Expand(class_info->m_treeItem, TVE_EXPAND);
     return true;
  }
  return false;
}

BOOL AddClassInfoTreeCtrlClass::PreTranslateMessage(MSG *msg)
{
  if(m_accHandle && TranslateAccelerator(m_hWnd, m_accHandle, msg))
    return TRUE;

  if(msg->message == WM_CHAR)
  {
    if(msg->wParam >= 32 && msg->wParam <= 127)
    {
      OnChar(msg->wParam, LOWORD(msg->lParam), HIWORD(msg->lParam));
      return TRUE;
    }
  }
  return CTreeCtrl::PreTranslateMessage(msg);
}

void AddClassInfoTreeCtrlClass::PopupMenu(void)
{
  HTREEITEM item = GetSelectedItem();
  if(item)
  {
    POINT p;
    RECT r;
    GetItemRect(item, &r, TRUE);
    p.x = r.right;
    p.y = r.top;
    DoPopUp(item, p);
  }
}

void AddClassInfoTreeCtrlClass::DoPopUp(HTREEITEM item, POINT p)
{
  if(item && m_addClassView)
  {
    CMenu bar;
    bar.LoadMenu(IDC_CT_POPUP);
    CMenu &popup = *bar.GetSubMenu(0);
    ASSERT(popup.m_hMenu != NULL);

    int pos;
    for(pos = popup.GetMenuItemCount() - 1; pos >= 0; pos--)
      popup.DeleteMenu(pos, MF_BYPOSITION);

    DWORD data = GetItemData(item);
    if(data & 0x80000000U)
    { // This is a class name
      m_classIndex = data & ~0x80000000U;
      m_memberIndex = 0xFFFFFFFFU;
      popup.AppendMenu(MF_STRING, IDC_CT_TAG_PEEK,   "Peek");
      popup.AppendMenu(MF_STRING, IDC_CT_TAG_SELECT, "Select");
    }
    else
    { // a memeber
      m_classIndex = data >> 16;
      m_memberIndex = data & 0xFFFFU;
      popup.AppendMenu(MF_STRING, IDC_CT_TAG_PEEK,   "Peek");
      popup.AppendMenu(MF_STRING, IDC_CT_TAG_SELECT, "Select");
      if(m_addClassView->m_addInfo->m_list[m_classIndex]->m_list[m_memberIndex]->m_indexType == TagIndexType::INHERITANCE_IDX)
        popup.AppendMenu(MF_STRING, IDC_CT_TAG_EXPAND, "Expand");
    }

    ClientToScreen(&p);
    popup.TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
  }
}

void AddClassInfoTreeCtrlClass::TagPeek(void)
{
   m_dlg->OpenAsTagPeek();  // Will call GetPeekParm() to get the parameters
}

void AddClassInfoTreeCtrlClass::GetPeekParm(const char **fn, int *LineNo)
{
   if(!m_dlg->m_globalTags.m_fileList)
   {
      *fn = "";
      *LineNo = 1;
      return;
   }

   if(!UpdateIndex())
   {
      *fn = "";
      *LineNo = 1;
      return;
   }

   if(m_memberIndex == 0xFFFFFFFFU)
   { // The class itself
      *fn = m_dlg->m_globalTags.m_fileList->GetFullName(m_addClassView->m_addInfo->m_list[m_classIndex]->m_fileIdx);
      *LineNo = m_addClassView->m_addInfo->m_list[m_classIndex]->m_lineNo;
   }
   else
   { // A member or iheritance_idx
      *fn = m_dlg->m_globalTags.m_fileList->GetFullName(m_addClassView->m_addInfo->m_list[m_classIndex]->m_list[m_memberIndex]->m_fileIdx);
      *LineNo = m_addClassView->m_addInfo->m_list[m_classIndex]->m_list[m_memberIndex]->m_lineNo;
   }
}

void AddClassInfoTreeCtrlClass::TagSelect(void)
{
   if(!m_dlg->m_globalTags.m_fileList)
      return;
   if(!UpdateIndex())
      return;

   if(m_memberIndex == 0xFFFFFFFFU)
   { // The class itself
      WainDoc *doc;
      doc = wainApp.OpenDocument(m_dlg->m_globalTags.m_fileList->GetFullName(m_addClassView->m_addInfo->m_list[m_classIndex]->m_fileIdx));
      if(doc)
      {
         doc->m_view->GotoLineNo(m_addClassView->m_addInfo->m_list[m_classIndex]->m_lineNo);
      }
   }
   else
   { // A member or iheritance_idx
      WainDoc *doc;
      doc = wainApp.OpenDocument(m_dlg->m_globalTags.m_fileList->GetFullName(m_addClassView->m_addInfo->m_list[m_classIndex]->m_list[m_memberIndex]->m_fileIdx));
      if(doc)
      {
         doc->m_view->GotoLineNo(m_addClassView->m_addInfo->m_list[m_classIndex]->m_list[m_memberIndex]->m_lineNo);
      }
   }
}

void AddClassInfoTreeCtrlClass::DoSelect(void)
{
   HTREEITEM item = GetSelectedItem();
   if(item)
   {
      DWORD data = GetItemData(item);
      if(data & 0x80000000U)
      { // This is a class name
         m_classIndex = data & ~0x80000000U;
         m_memberIndex = 0xFFFFFFFFU;
         TagSelect();
      }
      else
      { // a memeber
         m_classIndex = data >> 16;
         m_memberIndex = data & 0xFFFFU;
         if(m_addClassView->m_addInfo->m_list[m_classIndex]->m_list[m_memberIndex]->m_indexType == TagIndexType::INHERITANCE_IDX)
            TagExpand();
         else
            TagSelect();
      }
   }
}

void AddClassInfoTreeCtrlClass::TagExpand(void)
{
  const std::string class_name = m_addClassView->m_addInfo->m_list[m_classIndex]->m_list[m_memberIndex]->m_tag.c_str();
  AddClassInfoClass *add_class_info = m_addClassView->m_addInfo->FindClass(class_name);
  if(add_class_info)
  {
    SelectItem(add_class_info->m_treeItem);
    Expand(add_class_info->m_treeItem, TVE_EXPAND);
  }
}

void AddClassInfoTreeCtrlClass::GotoEditor(void)
{
  ChildFrame *cf = (ChildFrame *)GetMf()->MDIGetActive();
  if(cf)
  {
    cf->GetView()->SetFocus();
  }
}

int AddClassInfoTreeCtrlClass::DoSearch(int _direction, const char* _text, bool _reset, int _offset)
{
  HTREEITEM item = GetSelectedItem();
  if(!*_text || !m_addClassView)
    return true;
  if(item && !_reset)
  {
    DWORD data = GetItemData(item);
    if(data & 0x80000000U)
    { // This is a class name
      m_classIndex = data & ~0x80000000U;
    }
    else
    { // a member
      m_classIndex = data >> 16;
    }
    if(_offset > 0)
    {
      m_classIndex += _offset;
      if(m_classIndex >= m_addClassView->m_addInfo->m_list.size())
        return false;
    }
    else if(_offset  < 0)
    {
      if(!m_classIndex)
        return false;
      m_classIndex += _offset;
    }
  }
  else
    m_classIndex = 0;

  if(_reset)
    m_classIndex = 0;

  bool HasReversed = false;

  while(1)
  {
    if(MyStrIStr(m_addClassView->m_addInfo->m_list[m_classIndex]->m_className.c_str(), _text))
    {
      SelectItem(m_addClassView->m_addInfo->m_list[m_classIndex]->m_treeItem);
      EnsureVisible(m_addClassView->m_addInfo->m_list[m_classIndex]->m_treeItem);
      return TRUE;
    }
    if(_direction > 0)
    {
      m_classIndex++;
      if(m_classIndex >= m_addClassView->m_addInfo->m_list.size())
      {
        if(!HasReversed)
        {
          _direction = -1;
          HasReversed = TRUE;
          m_classIndex--;
        }
        else
          return FALSE;
      }
    }
    else
    {
      if(!m_classIndex)
      {
        if(!HasReversed)
        {
          _direction = 1;
          HasReversed = TRUE;
        }
        else
          return FALSE;
      }
      else
        m_classIndex--;
    }
  }
}

void AddClassInfoListClass::GetTagList(GetTagListClass *aGetTagList)
{
   unsigned int j;
   ListClass::size_type i;
   std::string temp;
   NavigatorDialog *dlg = &GetMf()->m_navigatorDialog;
   if(!dlg->m_globalTags.m_fileList)
      return;

   for(i = 0; i < m_list.size(); i++)
   {
      // The class name itself will be added by GlobalTags
      // TODO types
      for(j = 0; j < m_list[i]->m_list.size(); j++)
      {
         if(aGetTagList->m_word == m_list[i]->m_list[j]->m_tag)
         {
            temp = m_list[i]->m_className;
            temp += "::";
            temp += m_list[i]->m_list[j]->m_tag;

            aGetTagList->Add(dlg->m_globalTags.m_fileList->GetFullName(m_list[i]->m_list[j]->m_fileIdx),
                             dlg->m_globalTags.m_fileList->GetShortName(m_list[i]->m_list[j]->m_fileIdx),
                             m_list[i]->m_list[j]->m_lineNo,
                             temp.c_str(),
                             m_list[i]->m_list[j]->m_indexType,
                             m_list[i]->m_list[j]->m_signature.c_str());
         }
      }
   }
}

bool AddClassInfoTreeCtrlClass::UpdateIndex()
{
   HTREEITEM item = GetSelectedItem();
   if(!item)
      return false;
   DWORD data = GetItemData(item);
   if(data & 0x80000000U)
   { // This is a class name
      m_classIndex = data & ~0x80000000U;
      m_memberIndex = 0xFFFFFFFFU;
   }
   else
   { // a memeber
      m_classIndex = data >> 16;
      m_memberIndex = data & 0xFFFFU;
   }
   return true;
}

void AddClassInfoTreeCtrlClass::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMTVCUSTOMDRAW *pcd = (NMTVCUSTOMDRAW   *)pNMHDR;
    switch ( pcd->nmcd.dwDrawStage )
    {
    case CDDS_PREPAINT:
        *pResult = CDRF_NOTIFYITEMDRAW;
        break;
    case CDDS_ITEMPREPAINT:
        {
            HTREEITEM hItem = (HTREEITEM)pcd->nmcd.dwItemSpec;
            HTREEITEM selectedItem = GetSelectedItem();
            if (hItem == selectedItem)
            {
               pcd->clrText = wainApp.gs.m_listColorTextSel;
               if (GetFocus() == this)
                  pcd->clrTextBk = wainApp.gs.m_listColorSelFocus;
               else
                  pcd->clrTextBk = wainApp.gs.m_listColorSel;
            }
            else
            {
               pcd->clrText = wainApp.gs.m_listColorText;
               pcd->clrTextBk = wainApp.gs.m_listColorBack;
            }
            *pResult = CDRF_DODEFAULT;// do not set *pResult = CDRF_SKIPDEFAULT
        }
        break;
    }
}

//--- EOF
