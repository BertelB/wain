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
   bool operator () (const AddClassElementInfoClass* _lhs, const AddClassElementInfoClass* _rhs)
   {
      if(_lhs->m_indexType == TagIndexType::INHERITANCE_IDX && _rhs->m_indexType != TagIndexType::INHERITANCE_IDX)
         return true;
      if(_lhs->m_indexType != TagIndexType::INHERITANCE_IDX && _rhs->m_indexType == TagIndexType::INHERITANCE_IDX)
         return false;
      if(_lhs->m_indexType != _rhs->m_indexType)
         return _lhs->m_indexType < _rhs->m_indexType;
      return stricmp(_lhs->m_tag.c_str(), _rhs->m_tag.c_str()) < 0;
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

AddClassElementInfoClass * AddClassInfoListClass::Add(const std::string& _className, const std::string& _tagName, TagIndexType _indexType, int _fileNo, int _lineNo, bool _isStruct)
{
   AddClassInfoClass* add_class_info = FindClass(_className);
   if(!add_class_info)
   {
      add_class_info = new AddClassInfoClass(_className, _fileNo, _lineNo, _isStruct);
      m_list.push_back(add_class_info);
   }

   return add_class_info->Add(_tagName, _indexType, _fileNo, _lineNo);
}

void AddClassInfoListClass::Remove(const std::string& _name)
{
   ListClass::iterator it = std::find_if(m_list.begin(), m_list.end(), [&](auto i) { return i->m_className == _name; });
   if (it != m_list.end())
   {
      delete *it;
      m_list.erase(it);
   }
}

void AddClassInfoListClass::SetClassInfo(const std::string& _className, int _fileNo, int _lineNo, bool _isStruct)
{
   AddClassInfoClass *add_class_info = FindClass(_className);
   if(!add_class_info)
   {
      add_class_info = new AddClassInfoClass(_className, _fileNo, _lineNo, _isStruct);
      m_list.push_back(add_class_info);
   }
   else
   {
      add_class_info->m_fileIdx = _fileNo;
      add_class_info->m_lineNo = _lineNo;
   }
}

void AddClassInfoListClass::InitTree(CTreeCtrl* _tree)
{
   try
   {
      if(!_tree->m_hWnd || !IsWindow(_tree->m_hWnd))
         return;

      ListClass::size_type i;
      for(i = 0; i < m_list.size(); i++)
      {
         HTREEITEM Parent = 0;
         TVINSERTSTRUCT ParentInsert;
         ParentInsert.hParent = 0;
         ParentInsert.hInsertAfter = 0;
         ParentInsert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
         ParentInsert.item.pszText = (char *)m_list[i]->m_className.c_str();
         ParentInsert.item.iImage = int(m_list[i]->m_isStruct ? TagIndexType::STRUCT_IDX : TagIndexType::CLASS_IDX);
         ParentInsert.item.iSelectedImage = int(m_list[i]->m_isStruct ? TagIndexType::STRUCT_IDX : TagIndexType::CLASS_IDX);
         ParentInsert.item.lParam = i | 0x80000000U;
         ParentInsert.item.cChildren = !m_list[i]->m_list.empty();

         if(!_tree->m_hWnd || !IsWindow(_tree->m_hWnd))
            return;
         Parent = _tree->InsertItem(&ParentInsert);
         m_list[i]->m_treeItem = Parent;
      }
   }
   catch (...)
   {
      ::MessageBox(0, "InitTree - Catched exception", "Wain", MB_OK);
   }
}

AddClassInfoClass *AddClassInfoListClass::FindClass(const std::string& _className)
{
   ListClass::size_type i;
   for(i = 0; i < m_list.size(); i++)
      if(_className == m_list[i]->m_className)
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

void AddClassElementInfoClass::SetSignature(const std::string& _str)
{
   m_signature = _str;
}

UINT DeleteThread(LPVOID _obj)
{
   AddClassInfoTreeCtrlClass* tt = static_cast<AddClassInfoTreeCtrlClass*>(_obj);
   delete tt;
   return 0;
}

void NavigatorDialog::SetClassList(AddClassInfoListClass* _addInfo, TagFileList* _fileList, AddClassInfoTreeCtrlClass* _view)
{
   m_addClassView.SetList(_addInfo, _fileList, _view);
   AddClassInfoTreeCtrlClass *tt = m_classViewTree;
   m_classViewTree = _view;
   ASSERT(::IsWindow(m_classViewTree->m_hWnd));

   CRect tr = GetListTreeRect();
   m_classViewTree->MoveWindow(tr, FALSE);
   tt->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);
   if(m_navBarState == NavBarState::Class)
   {
     m_classViewTree->ModifyStyle(WS_DISABLED, WS_VISIBLE, SWP_NOACTIVATE | SWP_NOZORDER);
     InvalidateRect(NULL, TRUE);
   }
   auto t1 = GetUSec();
   tt->SetRedraw(FALSE);
   delete tt;
   auto t2 = GetUSec();
   SetStatusText("SetClassList delete %u", uint32_t((t2 - t1) / 1000));
   // AfxBeginThread(DeleteThread, tt, THREAD_PRIORITY_LOWEST, 0, 0);
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
  ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemExpanding)
END_MESSAGE_MAP();

void AddClassInfoTreeCtrlClass::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
   if(!m_hWnd || !IsWindow(m_hWnd))
      return;

   LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
   UINT action = pNMTreeView->action; // TVE_EXPAND or TVE_COLLAPSE
   if (action == TVE_EXPAND)
   {
      HTREEITEM hItem = pNMTreeView->itemNew.hItem;
      HTREEITEM child = GetChildItem(hItem);
      if (!child)
      {
         TVITEM tvItem;
         memset(&tvItem, 0, sizeof(tvItem));
         tvItem.hItem = hItem;
         tvItem.mask = TVIF_PARAM;
         if (GetItem(&tvItem))
         {
            uint32_t index = tvItem.lParam & 0x7FFFFFFF;
            // SetStatusText("Item TVE_EXPAND - 0x%X - %s", tvItem.lParam, m_addClassView->m_addInfo->m_list[index]->m_list[0]->m_tag.c_str());
            auto base = m_addClassView->m_addInfo->m_list[index];
            for(uint32_t j = 0; j < base->m_list.size(); j++)
            {
               TVINSERTSTRUCT insItem;
               insItem.hParent = hItem;
               insItem.hInsertAfter = 0;
               insItem.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
               insItem.item.iImage = int(base->m_list[j]->m_indexType);
               insItem.item.iSelectedImage = int(base->m_list[j]->m_indexType);
               insItem.item.pszText = (char *)base->m_list[j]->m_tag.c_str();
               insItem.item.lParam = (index << 16) | j;
               InsertItem(&insItem);
            }
         }
      }
    }
   *pResult = 0;
}

void AddClassInfoTreeCtrlClass::OnRButtonDown(UINT /* flags */, CPoint point)
{
  UINT ItemFlags;
  DoPopUp(HitTest(point, &ItemFlags), point);
}

void AddClassInfoTreeCtrlClass::OnLButtonDblClk(UINT /* flags */, CPoint /* point */)
{
   DoSelect();
}

AddClassInfoTreeCtrlClass::AddClassInfoTreeCtrlClass(class NavigatorDialog* _parent) :
   m_addClassView(0),
   m_accHandle(LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LIST_TREE))),
   m_dlg(_parent)
{
}

void AddClassInfoTreeCtrlClass::OnInfoTip(NMTVGETINFOTIP* _infoTip)
{
   LPARAM data = _infoTip->lParam;
   if(data & 0x80000000U)
   { // This is a class name
      m_classIndex = data & ~0x80000000U;
      m_memberIndex = 0xFFFFFFFFU;
   }
   else
   { // a memeber
      uint64_t ci = data >> 16;
      uint64_t mi = data & 0xFFFFU;
      if(!m_addClassView->m_addInfo->m_list[ci]->m_list[mi]->m_signature.empty())
      {
         strcpy(_infoTip->pszText, m_addClassView->m_addInfo->m_list[ci]->m_list[mi]->m_tag.c_str());
         strcat(_infoTip->pszText, m_addClassView->m_addInfo->m_list[ci]->m_list[mi]->m_signature.c_str());
      }
   }
}

bool AddClassInfoTreeCtrlClass::FindClassName(std::string& _name)
{
  AddClassInfoClass *class_info;
  if((class_info = m_addClassView->m_addInfo->FindClass(_name)) != 0)
  {
     SelectItem(class_info->m_treeItem);
     Expand(class_info->m_treeItem, TVE_EXPAND);
     return true;
  }
  return false;
}

BOOL AddClassInfoTreeCtrlClass::PreTranslateMessage(MSG* _msg)
{
   if(m_accHandle && TranslateAccelerator(m_hWnd, m_accHandle, _msg))
   {
      return TRUE;
   }

   if(_msg->message == WM_CHAR)
   {
      if(_msg->wParam >= 32 && _msg->wParam <= 127)
      {
         OnChar(static_cast<UINT>(_msg->wParam), LOWORD(_msg->lParam), HIWORD(_msg->lParam));
         return TRUE;
     }
   }
   return CTreeCtrl::PreTranslateMessage(_msg);
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

void AddClassInfoTreeCtrlClass::DoPopUp(HTREEITEM _item, POINT _p)
{
   if(_item && m_addClassView)
   {
      CMenu bar;
      bar.LoadMenu(IDC_CT_POPUP);
      CMenu &popup = *bar.GetSubMenu(0);
      ASSERT(popup.m_hMenu != NULL);

      int pos;
      for (pos = popup.GetMenuItemCount() - 1; pos >= 0; pos--)
         popup.DeleteMenu(pos, MF_BYPOSITION);

      DWORD_PTR data = GetItemData(_item);
      if(data & 0x80000000U)
      { // This is a class name
         m_classIndex = data & ~0x80000000U;
         m_memberIndex = 0xFFFFFFFFU;
         popup.AppendMenu(MF_STRING, IDC_CT_TAG_PEEK,   "Peek");
         popup.AppendMenu(MF_STRING, IDC_CT_TAG_SELECT, "Select");
      }
      else
      { // a memeber
         m_classIndex = static_cast<uint32_t>(data >> 16);
         m_memberIndex = data & 0xFFFFU;
         popup.AppendMenu(MF_STRING, IDC_CT_TAG_PEEK,   "Peek");
         popup.AppendMenu(MF_STRING, IDC_CT_TAG_SELECT, "Select");
         if(m_addClassView->m_addInfo->m_list[m_classIndex]->m_list[m_memberIndex]->m_indexType == TagIndexType::INHERITANCE_IDX)
            popup.AppendMenu(MF_STRING, IDC_CT_TAG_EXPAND, "Expand");
      }
      ClientToScreen(&_p);
      popup.TrackPopupMenu(TPM_LEFTALIGN, _p.x, _p.y, this);
   }
}

void AddClassInfoTreeCtrlClass::TagPeek(void)
{
   m_dlg->OpenAsTagPeek();  // Will call GetPeekParm() to get the parameters
}

void AddClassInfoTreeCtrlClass::GetPeekParm(const char** _fn, int* _lineNo)
{
   if(!m_dlg->m_globalTags.m_fileList)
   {
      *_fn = "";
      *_lineNo = 1;
      return;
   }

   if(!UpdateIndex())
   {
      *_fn = "";
      *_lineNo = 1;
      return;
   }

   if(m_memberIndex == 0xFFFFFFFFU)
   { // The class itself
      *_fn = m_dlg->m_globalTags.m_fileList->GetFullName(m_addClassView->m_addInfo->m_list[m_classIndex]->m_fileIdx);
      *_lineNo = m_addClassView->m_addInfo->m_list[m_classIndex]->m_lineNo;
   }
   else
   { // A member or iheritance_idx
      *_fn = m_dlg->m_globalTags.m_fileList->GetFullName(m_addClassView->m_addInfo->m_list[m_classIndex]->m_list[m_memberIndex]->m_fileIdx);
      *_lineNo = m_addClassView->m_addInfo->m_list[m_classIndex]->m_list[m_memberIndex]->m_lineNo;
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
      DWORD_PTR data = GetItemData(item);
      if(data & 0x80000000U)
      { // This is a class name
         m_classIndex = data & ~0x80000000U;
         m_memberIndex = 0xFFFFFFFFU;
         TagSelect();
      }
      else
      { // a memeber
         m_classIndex = static_cast<uint32_t>(data >> 16);
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
   AddClassInfoClass* addClassInfo = m_addClassView->m_addInfo->FindClass(class_name);
   if(addClassInfo)
   {
      SelectItem(addClassInfo->m_treeItem);
      Expand(addClassInfo->m_treeItem, TVE_EXPAND);
   }
}

void AddClassInfoTreeCtrlClass::GotoEditor(void)
{
   ChildFrame* cf = (ChildFrame *)GetMf()->MDIGetActive();
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
    DWORD_PTR data = GetItemData(item);
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
   DWORD_PTR data = GetItemData(item);
   if(data & 0x80000000U)
   { // This is a class name
      m_classIndex = data & ~0x80000000U;
      m_memberIndex = 0xFFFFFFFFU;
   }
   else
   { // a memeber
      m_classIndex = static_cast<uint32_t>( data >> 16);
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
