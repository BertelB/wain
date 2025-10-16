#ifndef FILESELD_H_INC
#define FILESELD_H_INC

class FileSelectDialogClass : public CDialog
{
public:
  FileSelectDialogClass(CWnd *aParent = NULL);
  ~FileSelectDialogClass();

  enum {IDD = IDD_PROJ_MANAGE_DIALOG};
  typedef std::vector<std::string >FileListClass;
  FileListClass m_fileList;
  CString m_caption;

private:
  void ViewDir(void);
  std::string m_dirPath;

  typedef std::vector<std::string >DirListClass;
  DirListClass m_dirList;
  void DirListSelect(void);
  bool m_changed;
  std::string m_filter;
  void SetSizes(int aX, int aY);
  void AddPath(const std::string &aPath);
  void AddSub(HTREEITEM aPrev, int aFirst, int aLast, int aOffset);
  CTreeCtrl m_tree;
  void AddItem(const std::string &aFileName); /* Add an item to the tree ctrl */
  void ClearTree(void);
  void RemoveSub(HTREEITEM aItem);
  void ExpandTree(HTREEITEM aItem, int Level);
  void SortTree(HTREEITEM aItem);

  HTREEITEM m_lastItem;
  std::string m_lastPath;
protected:
  virtual void DoDataExchange(CDataExchange *aDx);
  virtual BOOL OnInitDialog(void);
  virtual void OnOK(void);
  virtual void OnCancel(void);
protected:
  afx_msg void Add(void);
  afx_msg void Remove(void);
  afx_msg void Home(void);
  afx_msg void ChangeDisk(void);
  afx_msg void ChangeFilter(void);
  afx_msg void OnSize(UINT aType, int aX, int aY);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO *aMinMaxInfo);
  DECLARE_MESSAGE_MAP();
};

#endif
