#ifndef NAVIGATOR_PROJ_TREE_H_INC
#define NAVIGATOR_PROJ_TREE_H_INC
#include ".\..\src\stdafx.h"
#include ".\..\src\Project.h"

class NavProjFileInfo
{
public:
   NavProjFileInfo()
   {
   }
   NavProjFileInfo(const std::string _name, const std::string _shortName, int _iconIndex)
    : m_name(_name),
      m_shortName(_shortName),
      m_iconIndex(_iconIndex)
   {
   }
   std::string m_name;
   std::string m_shortName;
   int m_iconIndex = 0;
   HTREEITEM m_hTreeItem = 0;
};

class NavigatorProjectTree : public CTreeCtrl
{
public:
   void OnInfoTip(NMTVGETINFOTIP *InfoTiP);
   void DoSelect(void);
   int DoSearch(int _direction, const char* _text, bool _reset, int _offset);
   void GetPeekParm(const char **fn, int *LineNo);
   virtual BOOL PreTranslateMessage(MSG *msg);
   NavigatorProjectTree(class NavigatorDialog* _parent);
   ~NavigatorProjectTree();
   void ClearTree(HTREEITEM _item);
   void AddItem(const std::string& _fullName, const std::string& _fileName);
   void EndUpdate();
   void Select(const char* _fileName);
private:
   class NavTreeInfo* GetItemInfo(HTREEITEM _item);
   void ExpandTree(HTREEITEM item, uint32_t _level);
   HACCEL m_accHandle;
   std::string m_lastPath;
   std::vector<NavProjFileInfo> m_fileList;

   HTREEITEM m_lastItem = 0;
   afx_msg void OnRButtonDown(UINT flags, CPoint point);
   afx_msg void OnLButtonDblClk(UINT _flags, CPoint _point);
   afx_msg void HandleDelete(void);
   afx_msg void HandleInsert(void);
   afx_msg void GotoEditor(void);
   afx_msg void Select(void);
   afx_msg void PopupMenu(void);
   afx_msg void OnTreeGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
  void DoPopUp(HTREEITEM item, POINT p);
  class NavigatorDialog *m_dlg;

  DECLARE_MESSAGE_MAP();
};

#endif
