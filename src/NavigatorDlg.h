//=============================================================================
// This source code file is a part of Wain.
// It defines the NavigatorDialog, and various helper classes such as:
// tag_class, dir_class, FileList_class.
// The implementation can be found in NavigatorDlg.cpp and Tags.cpp.
//=============================================================================
#ifndef NAVIGATORDLG_H_INC
#define NAVIGATORDLG_H_INC
#include "DialogBase.h"
#include "Wain.h"
#include "TagsType.h"
#include "NavigatorProjTree.h"

class DirInfoClass
{
public:
   DirInfoClass(const std::string &aName) : m_name(aName), m_iconIndex(-1)
   {}
   std::string m_name;
   int  m_iconIndex;
};

class FileInfoClass : public DirInfoClass
{
public:
};

inline bool operator < (const DirInfoClass &lhs, const DirInfoClass &rhs)
{
   if(lhs.m_name == "[..]")
      return true;
   if(rhs.m_name == "[..]")
      return false;
   if(lhs.m_name[0] == '[' && rhs.m_name[0] != '[')
      return true;
   if(lhs.m_name[0] != '[' && rhs.m_name[0] == '[')
      return false;

   return stricmp(lhs.m_name.c_str(), rhs.m_name.c_str()) < 0;
}

class NavigatorDialog;

class ThreadGetIconParmClass
{
public:
  unsigned int m_index;
  std::vector<std::string >m_nameList;
};

class ThreadGetIconNameListCompareClass
{
public:
   ThreadGetIconNameListCompareClass() {}
   bool operator () (const std::string &lhs, const std::string &rhs)
   {
      char l = lhs[lhs.size() - 1];
      char r = rhs[rhs.size() - 1];
      if(l == '\\' && r != '\\')
         return true;
      if(l != '\\' && r == '\\')
         return false;
      return stricmp(lhs.c_str(), rhs.c_str()) < 0;
   }
};

class ThreadGetIconConClass
{
public:
  unsigned int m_index;
  std::vector<unsigned int >m_iconList;
  HIMAGELIST m_imageList;
};

class DirClass
{
protected:
  std::vector<DirInfoClass >m_dirInfoList;
  std::string m_dirPath;
  void Clear();

  void AddEntry(const std::string &aFileName)
  {
     m_dirInfoList.push_back(DirInfoClass(aFileName));
  }
  virtual void DoSelect(int aSelection) = 0;
public:
  virtual void UpdateDir(void) = 0;
  DirClass() {}
  ~DirClass() { Clear(); }
  class NavigatorDialog *m_dlg;
  int GetIconIndex(int aNr);
  const char *GetListText(int aNr);
  void HandlePrev(void);
  bool Search(int *aIndex, int aDir, const char *aSearchText);
  virtual void OpenDirForFile(const char* _filename) = 0;
};

class HdDirClass : public DirClass
{
  #define MAX_NET_LEVEL 64
  NETRESOURCE *m_netResource[MAX_NET_LEVEL];  // Fixme
  int m_nofNetResource[MAX_NET_LEVEL];
  int m_netSel[MAX_NET_LEVEL];
  int m_netLevel;
  void SelNet(int level, int sel = 0);
  void AddNet(int level, NETRESOURCE *res);
  bool m_inNet;
  bool CheckNet(int sel);
  bool m_wasNet;
public:
  HdDirClass();
  ~HdDirClass();
  void ChangeDisk(void);
  void UpdateDir(void);
  void DoSelect(int aSelection);
  unsigned int m_threadCounter;
  void HandleGetIcon(ThreadGetIconConClass *aMsg);
  bool GetFullPath(int aSel, std::string &aStr);
  void DoPopupMenu(POINT *p);
   void OpenDirForFile(const char* _filename) override;
};

class FtpDirClass : public DirClass
{
public:
  FtpDirClass();
  void EnableFtpMode(const char *aNewPath);
  void DisableFtpMode(void);
  void UpdateDir(void);
  void DoSelect(int aSelection);
   void OpenDirForFile(const char* /* _filename */) override {}
};

class NavFileList
{
  friend class NavigatorDialog;
private:
  NavigatorDialog *m_dlg;
  void RemoveFile();
  void UpdateFileView();
public:
  void UpdateFileList();
};

class NavigatorTabCtrlClass : public CTabCtrl
{
public:
  NavigatorDialog *m_dlg;
  afx_msg BOOL OnEraseBkgnd(CDC *dc);
  DECLARE_MESSAGE_MAP();
};

#define MAX_NOF_FTP_CONS 64

class NavigatorDialog : public CDialog
{
public:
   void DoBuildTags(const char *file = 0, bool force = false, bool auto_build = false);
   void BuildTags(bool aAuto = false);

private:
   CString m_lastSearchText;
   CString m_searchText;
   void DoSearch(int _from, int _direction, bool _force, bool _reset, int offset = 0);
   unsigned int m_skipUpdateSearch;

   CWinThread *m_autoRebuildThread;
   int m_autoRebuildTime;
   void HandleAutoRebuildTimeout(void);
   void HandleAutoTagFileTimer(void);
   void EnableButtons(NavBarStateType aNewState);
   NavigatorTabCtrlClass m_tabCtrl;
   CFont m_listFont;
   RECT GetListTreeRect();
public:
   void OpenDirForFile(const char* _filename);
   void OpenProjectForFile(const char* _filename);
   void SetClassList(AddClassInfoListClass *aAddInfo, TagFileListClass *aFileList, AddClassInfoTreeCtrlClass *aView);
   AddClassViewClass m_addClassView;

   CBrush *m_buttonBrush;
   void HandleGetIcon(void *msg);
   void HandleFtpConMsg(void *ftp, unsigned int attempt);
   unsigned int m_ftpConnections[MAX_NOF_FTP_CONS];
   bool m_inSetFtp;

   unsigned int m_inetConnectAttempt;
   CInternetSession m_inetSession;
   CFtpConnection *m_ftpConnection;
   class FtpWaitDialogClass *m_ftpWaitDialog;
   void OnListSetSel(int sel);
   void FileHasBeenSaved();
   void GlobalTagSetup(void);
   void SetProjectFile(const char *file_name);
   void JumpToTag(GetTagElemClass *elem);
   BOOL m_projectChanged;
   RtvStatus MakeProjectFileList(std::string &FileName, const char *types);

   HIMAGELIST m_sysImageList;
   CImageList m_tagImageList;
   UINT m_tagRebuildTimerId;
   UINT m_autoTagFileTimer;
   int  m_autoTagFileCount;

   HdDirClass m_hdDir;
   FtpDirClass m_ftpDir;
   GlobalTagClass m_globalTags;
   CurrentTagClass m_currentTags;

   NavFileList m_fileList;
   void SetFileName(const char *file_name, int PropIndex);
   char m_lastCurrentFile[_MAX_PATH];
   char m_lastCurrentDir[_MAX_PATH];
   void DoSelect(int selection);
   class ProjectBaseClass *m_project;
   void SetNewFont(void);
   void HandleDelete(int sel);

   NavigatorDialog(CWnd *aParent = 0);
   ~NavigatorDialog();
   NavBarStateType m_navBarState;

   enum { IDD = IDD_NAVIGATOR_DIALOG };
   class NavigatorList *m_navigatorList;
   AddClassInfoTreeCtrlClass *m_classViewTree;
   NavigatorProjectTree* m_projectTree = 0;

   void Close(void); /* Called by mainframe on close */
   const char *GetListText(int item, int column, BOOL long_text);
   int GetListIconNr(int item);
   void HandleListCtrlPageUp(void);
   void Activate(void);
   void DeActivate(void);
   BOOL m_firstActivate;

   virtual BOOL PreTranslateMessage(MSG *msg);
   void AddSearchChar(char ch);
protected:
   virtual void DoDataExchange(CDataExchange *dx);

protected:
   bool m_init;
   HACCEL m_accHandle;
   afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnPaint(void);
   afx_msg void ProjectSave(void);
   afx_msg void ProjectOpen(void);
   afx_msg void ProjectManage(void);
   afx_msg void ProjectMake(void);
   afx_msg void ProjectMake0(void);
   afx_msg void ProjectMake1(void);
   afx_msg void ProjectMake2(void);
   afx_msg void ProjectMake3(void);
   afx_msg void ProjectSetup(void);
   afx_msg void OnClose(void);
   afx_msg void MsgBuildTags();
   afx_msg void ReBuildTags(void);
   afx_msg void TagSetup(void);
   afx_msg void HandleFtpConnectionection(void);
   afx_msg void ChangeDisk(void);
   afx_msg void OnTimer(UINT timer_id);
   afx_msg void OnWindowPosChanged(WINDOWPOS *window_pos);
   virtual BOOL OnInitDialog();
   virtual void OnOK();
   virtual void OnCancel();
public:
   void GlobalTagReadFunc(ReadTagClass *aReadClass);
   void TagReadFunc(ReadTagClass *aReadClass);
   bool ViewClass(std::string &aClassName);
   int GetMatchTags(std::string aList[], std::string &aWord);

   afx_msg void OpenWith(void);
   afx_msg void ViewDir(void);
   afx_msg void ViewFiles(void);
   afx_msg void ViewClass(void);
   afx_msg void ViewTags(void);
   afx_msg void ViewProject(void);
   afx_msg void ViewCurrentTags(void);
   afx_msg void ViewFtp(void);
   afx_msg void SearchChanged(void);
   afx_msg void SearchSetFocus(void);
   afx_msg void SearchNext(void);
   afx_msg void SearchPrev(void);
   afx_msg void GotoEditor(void);
   afx_msg void ReReadDir(void);
   afx_msg void OpenAsTagPeek(void);

   bool GetTagList(GetTagListClass *aList);
   void OnTagPeek(GetTagElemClass *aElem);
   void OnAutoTag();

   afx_msg HBRUSH OnCtlColor(CDC *dc, CWnd *wnd, UINT ctl_color);
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnTabSelChange(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void ClassTreeInfoTip(NMHDR *aNotifyStruct, LRESULT *aResult);
   afx_msg void ProjTreeInfoTip(NMHDR *aNotifyStruct, LRESULT *aResult);
   DECLARE_MESSAGE_MAP();
};

class TagSetupDialogClass : public DialogBaseClass
{
  DECLARE_DYNAMIC(TagSetupDialogClass );
  BOOL m_glob;
public:
  std::string m_prog;
  std::string m_options;
  std::string m_files;
  std::string m_tagFile;
  BOOL m_track;
  TagSetupDialogClass (BOOL glob, CWnd *pParent = NULL);
  virtual ~TagSetupDialogClass (void);
  enum {IDD = IDD_TAG_SETUP_DIALOG};
protected:
  virtual BOOL OnInitDialog();
  void DoDataExchange(CDataExchange *dx);
  afx_msg void LookupTagProg(void);
  DECLARE_MESSAGE_MAP();
};

class TagSetupPropClass : public PropertyPageBaseClass
{
  DECLARE_DYNAMIC(TagSetupPropClass);
  DocPropClass *m_prop;
public:
  std::string m_prog;
  std::string m_options;
  std::string m_tagFile;
  TagSetupPropClass(DocPropClass *doc_prop);
  virtual ~TagSetupPropClass(void);
  enum {IDD = IDD_TAG_SETUP_PROP};
protected:
  virtual BOOL OnInitDialog();
  virtual BOOL OnKillActive(void);
  void DoDataExchange(CDataExchange *dx);
  afx_msg void LookupTagProg(void);
  DECLARE_MESSAGE_MAP();
};

extern UINT GlobalTagReadMsgId;
extern UINT GetIconMsgId;

extern UINT ThreadAutoRebuildTag(LPVOID parm);
extern UINT ThreadGetIcon(LPVOID parm);

#endif // #ifndef NAVIGATORDLG_H_INC

