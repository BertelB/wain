#include ".\..\src\stdafx.h"
#include "NavigatorProjTree.h"
#include "WainUtil.h"
#include "WainView.h"
#include "MainFrm.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class NavTreeInfo
{
public:
   enum class EntryType
   {
      Folder, File
   };

   NavTreeInfo(const std::string& _fullName, const std::string& _shortName, EntryType _entryType, int32_t _index) :
      m_fullName(_fullName),
      m_shortName(_shortName),
      m_entryType(_entryType),
      m_index(_index)
   {
   }
   ~NavTreeInfo()
   {
   }
   std::string m_fullName;
   std::string m_shortName;
   int32_t m_index = 0;
   EntryType m_entryType;
};


static void MySplitPath(std::vector<std::string>& _pathPart, const std::string& _fn)
{
   std::string::size_type startPos = 0;
   std::string::size_type endPos;
   while ((endPos = _fn.find_first_of("\\/", startPos)) != std::string::npos)
   {
      _pathPart.push_back(_fn.substr(startPos, endPos - startPos));
      startPos = endPos + 1;
   }
   _pathPart.push_back(_fn.substr(startPos));
}

void NavigatorProjectTree::OnRButtonDown(UINT /* flags */, CPoint point)
{
   UINT ItemFlags;
   DoPopUp(HitTest(point, &ItemFlags), point);
}

void NavigatorProjectTree::DoPopUp(HTREEITEM item, POINT p)
{
   if(item)
   {
      CMenu bar;
      bar.LoadMenu(IDC_PT_POPUP);
      CMenu& popup = *bar.GetSubMenu(0);
      ASSERT(popup.m_hMenu != NULL);

      for(int pos = popup.GetMenuItemCount() - 1; pos >= 0; pos--)
         popup.DeleteMenu(pos, MF_BYPOSITION);
      popup.AppendMenu(MF_STRING, IDL_INSERT, "Add files\tInsert");
      popup.AppendMenu(MF_STRING, IDL_DELETE, "Remove from project\tDelete");
      popup.AppendMenu(MF_STRING, IDL_SELECT, "Open\tEnter");
      popup.AppendMenu(MF_STRING, IDB_EDIT,   "Jump to editor\tALT+E");
      ClientToScreen(&p);
      popup.TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
   }
}

void NavigatorProjectTree::OnInfoTip(NMTVGETINFOTIP* _infotip)
{
   NavTreeInfo* treeInfo =  (NavTreeInfo*)_infotip->lParam;
   if (treeInfo->m_entryType == NavTreeInfo::EntryType::File)
   {
      strcpy(_infotip->pszText, treeInfo->m_fullName.c_str());
   }
}

void NavigatorProjectTree::DoSelect(void)
{
   HTREEITEM selectedItem = GetSelectedItem();
   TVITEM tvItem;
   tvItem.hItem = selectedItem;
   tvItem.mask = TVIF_PARAM;
   if(GetItem(&tvItem))
   {
      NavTreeInfo* e = (NavTreeInfo*)tvItem.lParam;
      if (e->m_entryType == NavTreeInfo::EntryType::File)
      {
         SetStatusText(e->m_fullName.c_str());
         wainApp.OpenDocument(e->m_fullName.c_str());
         return;
      }
   }
   SetStatusText("No file selected");
}

int NavigatorProjectTree::DoSearch(int _direction, const char* _text, bool _reset, int /* _offset */)
{
   if (!*_text)
   {
      SetStatusText("No Text");
      return true;
   }

   HTREEITEM item = GetSelectedItem();
   if (_reset || !item)
   {
      item = GetRootItem();
   }
   if (!item)
   {
      SetStatusText("No Item");
      return false;
   }
   const NavTreeInfo* itemInfo = GetItemInfo(item);
   while (itemInfo->m_entryType == NavTreeInfo::EntryType::Folder)
   {
      item = GetChildItem(item);
      itemInfo = GetItemInfo(item);
   }
   int32_t index = itemInfo->m_index;
   if (_direction == 0 && MyStrIStr(itemInfo->m_shortName.c_str(), _text))
   {
      // The text still match, just stay where we are
      SetStatusText("Still match at index %d, dir %d", index, _direction);
      return true;
   }
   if (!_direction)
      _direction = 1;
   int32_t startIndex = index;
   index += _direction;
   for (; index >= 0 && index < m_fileList.size(); index += _direction)
   {
      const NavProjFileInfo& info = m_fileList[index];
      if (MyStrIStr(info.m_shortName.c_str(), _text))
      {
         SelectItem(info.m_hTreeItem);
         EnsureVisible(info.m_hTreeItem);
         SetStatusText("Found Forward: start %d, index %d, dir %d", startIndex, index, _direction);
         return true;
      }
   }
   if (index == int32_t(m_fileList.size()))
      index--;
   else if(index < 0)
      index = 0;
   _direction = -_direction;
   for (; index >= 0 && index < m_fileList.size(); index += _direction)
   {
      const NavProjFileInfo& info = m_fileList[index];
      if (MyStrIStr(info.m_shortName.c_str(), _text))
      {
         SelectItem(info.m_hTreeItem);
         EnsureVisible(info.m_hTreeItem);
         SetStatusText("Found Back: start %d, index %d, dir %d", startIndex, index, _direction);
         return true;
      }
   }
   SetStatusText("Nothing %s %d", _text, startIndex);
   return false;
}

BOOL NavigatorProjectTree::PreTranslateMessage(MSG* _msg)
{
   if(m_accHandle && TranslateAccelerator(m_hWnd, m_accHandle, _msg))
      return TRUE;

	return CTreeCtrl::PreTranslateMessage(_msg);
}

NavigatorProjectTree::NavigatorProjectTree(class NavigatorDialog* _parent)
  : m_dlg(_parent)
{
   m_accHandle = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LIST_PROJ_TREE));
}

NavigatorProjectTree::~NavigatorProjectTree()
{
}

NavTreeInfo* NavigatorProjectTree::GetItemInfo(HTREEITEM _item)
{
   TVITEM tvItem;
   memset(&tvItem, 0, sizeof(tvItem));
   tvItem.hItem = _item;
   tvItem.mask = TVIF_PARAM;
   if (GetItem(&tvItem))
   {
      return (NavTreeInfo*)tvItem.lParam;
   }
   return 0;
}

void NavigatorProjectTree::ClearTree(HTREEITEM _item)
{
   if(!::IsWindow(m_hWnd))
      return;

   if (_item == 0)
   {
      _item = GetRootItem();
   }
   HTREEITEM child = GetChildItem(_item);
   while(child)
   {
      HTREEITEM next = GetNextSiblingItem(child);
      ClearTree(child);
      child = next;
   }
   TVITEM tvItem;
   memset(&tvItem, 0, sizeof(tvItem));
   tvItem.hItem = _item;
   tvItem.mask = TVIF_PARAM;
   if (GetItem(&tvItem))
   {
      NavTreeInfo* e = (NavTreeInfo*)tvItem.lParam;
      delete e;
   }
   DeleteItem(_item);
   m_lastPath.clear();
   m_lastItem = 0;
   m_fileList.clear();
}

void NavigatorProjectTree::AddItem(const std::string& _fullName, const std::string& _fileName)
{
   std::string fn;
   MySplitPath(_fullName.c_str(), SP_FILE | SP_EXT, fn);
   m_fileList.push_back(NavProjFileInfo(_fullName, fn, 0));
   std::string dirPath;
   MySplitPath(_fileName.c_str(), SP_DRIVE | SP_DIR, dirPath);
   if (dirPath == m_lastPath)
   {
      std::string fnExt;
      MySplitPath(_fileName.c_str(), SP_FILE | SP_EXT, fnExt);
      TVINSERTSTRUCT insStruct;
      insStruct.hParent = m_lastItem;
      insStruct.item.mask = TVIF_TEXT | TVIF_PARAM;
      insStruct.hInsertAfter = 0;
      auto treeInfo = new NavTreeInfo(_fullName, fnExt, NavTreeInfo::EntryType::File, m_fileList.size() - 1);
      insStruct.item.lParam = (LPARAM)treeInfo;
      insStruct.item.pszText = (LPSTR)treeInfo->m_shortName.c_str();
      HTREEITEM newItem = InsertItem(&insStruct);
      m_fileList.back().m_hTreeItem = newItem;
      return;
   }
   m_lastPath = dirPath;

   HTREEITEM item = GetRootItem();
   HTREEITEM candidate = 0;
   int candidateIdx = -1;

   std::vector<std::string> pathPart;
   MySplitPath(pathPart, _fileName.c_str());

   bool found = false;
   int j = 0;
   while(!found && item)
   {
      CString p = GetItemText(item);
      if(j < pathPart.size() && !stricmp(pathPart[j].c_str(), p))
      {
         do
         {
            candidate = item;
            candidateIdx = j;
            item = GetChildItem(item);
            p = GetItemText(item);
            j++;
         }
         while(item && j < pathPart.size() && !stricmp(pathPart[j].c_str(), p));
      }
      else
      {
         item = GetNextSiblingItem(item);
      }
   }

   TVINSERTSTRUCT insStr;
   insStr.hParent = candidate;
   insStr.item.mask = TVIF_TEXT | TVIF_PARAM;
   insStr.hInsertAfter = 0;

   for (j = candidateIdx + 1; j < pathPart.size(); j++)
   {
      NavTreeInfo::EntryType entryType = j < pathPart.size() - 1 ? NavTreeInfo::EntryType::Folder : NavTreeInfo::EntryType::File;
      auto treeInfo = new NavTreeInfo(_fullName, pathPart[j].c_str(), entryType, m_fileList.size() - 1);
      insStr.item.lParam = (LPARAM)treeInfo;
      insStr.item.pszText = (LPSTR)treeInfo->m_shortName.c_str();
      m_lastItem = candidate;
      candidate = InsertItem(&insStr);
      if (entryType == NavTreeInfo::EntryType::File)
      {
         m_fileList.back().m_hTreeItem = candidate;
      }
      insStr.hParent = candidate;
   }
}

void NavigatorProjectTree::EndUpdate()
{
   sort(m_fileList.begin(), m_fileList.end(), [](const NavProjFileInfo& _a, const NavProjFileInfo&  _b)
   {
      return _a.m_shortName < _b.m_shortName;
   });
   for (uint32_t i = 0; i < m_fileList.size(); i++)
   {
      TVITEM tvItem;
      tvItem.hItem = m_fileList[i].m_hTreeItem;
      tvItem.mask = TVIF_PARAM;
      if(GetItem(&tvItem))
      {
         NavTreeInfo* e = (NavTreeInfo*)tvItem.lParam;
         if (e->m_entryType == NavTreeInfo::EntryType::File)
         {
            e->m_index = i;
         }
      }
   }
   ExpandTree(GetRootItem(), 0);
}

void NavigatorProjectTree::Select(const char* _fileName)
{
   for (auto fi : m_fileList)
   {
      if (fi.m_name == _fileName)
      {
         SelectItem(fi.m_hTreeItem);
         EnsureVisible(fi.m_hTreeItem);
         return;
      }
   }
}

void NavigatorProjectTree::ExpandTree(HTREEITEM _item, uint32_t _level)
{
   while(_item && _level < 3)
   {
      TVITEM tvItem;
      tvItem.hItem = _item;
      tvItem.mask = TVIF_PARAM;
      if(GetItem(&tvItem))
      {
         Expand(_item, TVE_EXPAND);
         ExpandTree(GetChildItem(_item), _level + 1);
      }
      _item = GetNextSiblingItem(_item);
   }
}

void NavigatorProjectTree::OnLButtonDblClk(UINT /* _flags */, CPoint /* _point */)
{
   DoSelect();
}

void NavigatorProjectTree::HandleDelete(void)
{
   HTREEITEM selectedItem = GetSelectedItem();
   TVITEM tvItem;
   tvItem.hItem = selectedItem;
   tvItem.mask = TVIF_PARAM;
   if(GetItem(&tvItem))
   {
      NavTreeInfo* e = (NavTreeInfo*)tvItem.lParam;
      if (e->m_entryType == NavTreeInfo::EntryType::File)
      {
         SetStatusText("About to delete: %s", e->m_fullName.c_str());
         m_dlg->m_project->RemoveFile(e->m_index);
         return;
      }
   }
   SetStatusText("Nothing to delete");
}

void NavigatorProjectTree::HandleInsert(void)
{
   SetStatusText("Handle Insert");
   m_dlg->m_project->Add();
}

void NavigatorProjectTree::GotoEditor(void)
{
   ChildFrame *cf = (ChildFrame *)GetMf()->MDIGetActive();
   if(cf)
   {
      cf->GetView()->SetFocus();
   }
}

void NavigatorProjectTree::Select(void)
{
   DoSelect();
}

void NavigatorProjectTree::PopupMenu(void)
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

void NavigatorProjectTree::OnTreeGetInfoTip(NMHDR *pNMHDR, LRESULT *_result)
{
   OnInfoTip((NMTVGETINFOTIP *)pNMHDR);
   *_result = 0;
}

BEGIN_MESSAGE_MAP(NavigatorProjectTree, CTreeCtrl)
   ON_WM_RBUTTONDOWN()
   ON_WM_LBUTTONDBLCLK()
   ON_COMMAND(IDL_DELETE, HandleDelete)
   ON_COMMAND(IDL_INSERT, HandleInsert)
   ON_COMMAND(IDB_EDIT,   GotoEditor)
   ON_COMMAND(IDL_SELECT, Select)
   ON_COMMAND(IDC_PT_VIEW_POPUP, PopupMenu)
   ON_NOTIFY_REFLECT(TVN_GETINFOTIP, OnTreeGetInfoTip)
   ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP();

void NavigatorProjectTree::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    NMTVCUSTOMDRAW *pcd = (NMTVCUSTOMDRAW   *)pNMHDR;
    switch ( pcd->nmcd.dwDrawStage )
    {
    case CDDS_PREPAINT:
        *pResult = CDRF_NOTIFYITEMDRAW;
        break;

    case CDDS_ITEMPREPAINT :
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
