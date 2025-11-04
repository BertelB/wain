#ifndef CLASSVIEW_H_INC
#define CLASSVIEW_H_INC
#include "../src/TagIndexType.h"

class TagFileList;

class AddClassInfoTreeCtrlClass : public CTreeCtrl
{
public:
   bool FindClassName(std::string &aName);
   void OnInfoTip(NMTVGETINFOTIP *InfoTiP);
   void DoSelect(void);
   int DoSearch(int _direction, const char* _text, bool _reset, int _offset);
   void GetPeekParm(const char **fn, int *LineNo);
   class AddClassView *m_addClassView;
   virtual BOOL PreTranslateMessage(MSG *msg);
   AddClassInfoTreeCtrlClass(class NavigatorDialog *Parent);
   HACCEL m_accHandle;
   afx_msg void OnRButtonDown(UINT flags, CPoint point);
   afx_msg void OnLButtonDblClk(UINT flags, CPoint point);
   afx_msg void PopupMenu(void);
   afx_msg void TagPeek(void);
   afx_msg void TagSelect(void);
   afx_msg void TagExpand(void);
   afx_msg void GotoEditor(void);
   afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
   DECLARE_MESSAGE_MAP();
private:
   void DoPopUp(HTREEITEM item, POINT p);
   unsigned int m_memberIndex;
   unsigned int m_classIndex;
   class NavigatorDialog *m_dlg;
   bool UpdateIndex();
};

class AddClassElementInfoClass
{ // One funcion/variable/inherince in one class
public:
  AddClassElementInfoClass(const std::string& _tagName, TagIndexType _indexType, int _fileNo, int _lineNo) :
     m_tag(_tagName),
     m_indexType(_indexType),
     m_fileIdx(_fileNo),
     m_lineNo(_lineNo)
   {
   }
   ~AddClassElementInfoClass();
   void SetSignature(const std::string& _str);
   int  m_fileIdx;
   int  m_lineNo;
   std::string m_tag;
   std::string m_signature;
   char m_type;
   TagIndexType m_indexType;
};

class AddClassInfoClass
{ // One class
public:
  AddClassInfoClass(const std::string& _className, int _fileNr, int _lineNr, bool _isStruct) :
     m_className(_className),
     m_fileIdx(_fileNr),
     m_lineNo(_lineNr),
     m_isStruct(_isStruct)
  {

  }
  ~AddClassInfoClass();
  AddClassElementInfoClass *Add(const std::string &aTagName, TagIndexType _indexType, int aFileNo, int aLineNo)
  {
     AddClassElementInfoClass *node = new AddClassElementInfoClass(aTagName, _indexType, aFileNo, aLineNo);
     m_list.push_back(node);
     return node;
  }

  void Sort(void);
  typedef std::vector<AddClassElementInfoClass *>ListClass;
  ListClass m_list;
  std::string m_className;
  int  m_fileIdx;
  int  m_lineNo;
  bool m_isStruct;
  HTREEITEM m_treeItem;
};

class AddClassInfoListClass
{   // List of all classes
public:
   AddClassInfoListClass();
   ~AddClassInfoListClass();
   AddClassElementInfoClass *Add(const std::string &aClassName, const std::string &aTagName, TagIndexType _indexType, int aFileNo, int aLineNo, bool aIsStruct);
   void SetClassInfo(const std::string &aClassName, int aFileNo, int aLineNo, bool aIsStruct);
   AddClassInfoClass* FindClass(const std::string& _className);

   void Sort(void);
   void InitTree(CTreeCtrl *aTree);
   void GetTagList(class GetTagListClass *aGetTagList);
   void Remove(const std::string& aName);

   typedef std::vector<AddClassInfoClass *>ListClass;
   ListClass m_list;
};

class AddClassView
{
public:
   AddClassView();
   ~AddClassView();
   void SetList(AddClassInfoListClass *aAddInfo_, TagFileList *aFileList, AddClassInfoTreeCtrlClass *aView);

   void Clear();
   void ClearTree();
   void OnClose();
   AddClassInfoListClass *m_addInfo;
   TagFileList *m_fileList;
   AddClassInfoTreeCtrlClass *m_view;
   CImageList m_imageList;
private:
};

#endif
