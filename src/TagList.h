//=============================================================================
// This source code file is a part of Wain.
// It defines TagList_dialog_class.
//=============================================================================
#ifndef TAGLIST_H_INC
#define TAGLIST_H_INC

#define TL_ANY          0xFFFFFFFFUL
#define ID_JUST_ONE     4


class TagListDialogClass : public CDialog
{
  DECLARE_DYNAMIC(TagListDialogClass);
  CWnd *m_parent;

  void SetSizes(int aCx, int aCy);
  int m_oldSizeCx;

  CImageList m_imageList;
public:
  class GetTagElemClass* m_peekElem;
  bool m_isPeekClass = false;
  unsigned int m_types;
  int m_selectedIndex;
  std::string m_tagWord;

  CListCtrl *m_listBox;
  TagListDialogClass(CWnd* _parent, const std::string& _word, unsigned int _types);
  virtual ~TagListDialogClass(void);
  virtual BOOL OnInitDialog(void);
  BOOL DoInitDialog();
  enum {IDD = IDD_TAG_LIST_DIALOG};
  virtual void OnCancel();
  virtual void OnOk();
  class GetTagListClass *m_tagList;
protected:
  void DoDataExchange(CDataExchange *dx);
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnPeek(void);
  afx_msg void OnType(void);
  afx_msg void OnClass(void);
  afx_msg void OnPeekClass(void);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR *min_max_info);
  DECLARE_MESSAGE_MAP();
};

class ClassListDialogClass : public CDialog
{
  DECLARE_DYNAMIC(ClassListDialogClass);
  CWnd *m_parent;

  void SetSizes(int _cx, int _cy);
  int m_oldSizeCx;
  CImageList m_imageList;
public:
  // class GetTagElemClass* m_peekElem;
  // bool m_isPeekClass = false;
  // unsigned int m_types;
  // int m_selectedIndex;
  // std::string m_tagWord;

  // CListCtrl *m_listBox;
  ClassListDialogClass(CWnd* _parent, const std::string& _word);
  virtual ~ClassListDialogClass(void);
  virtual BOOL OnInitDialog(void);
  BOOL DoInitDialog();
  enum {IDD = IDD_CLASS_LIST_DIALOG};
  virtual void OnCancel();
  virtual void OnOk();
  // class GetTagListClass *m_tagList;
protected:
  std::string m_word;
  void DoDataExchange(CDataExchange *dx);
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnPeek(void);
  afx_msg void OnType(void);
  afx_msg void OnClass(void);
  afx_msg void OnPeekClass(void);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* _minMaxInfo);
  DECLARE_MESSAGE_MAP();
};


class ListCtrlClass : public CListCtrl
{
  TagListDialogClass *m_dlg;
public:
  ListCtrlClass(TagListDialogClass *dlg);
  afx_msg void OnLButtonDblClk(UINT flags, CPoint point);
  DECLARE_MESSAGE_MAP();
};

#endif // TAGLIST_H_INC


