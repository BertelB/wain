//=============================================================================
// This source code file is a part of Wain.
// It defines main_frame_class, ViewListElem, ViewList_class,
// MacroListEntryClass, MacroList_class and BookmarkList_dialog_class.
//=============================================================================
#ifndef MAINFRM_H_INC
#define MAINFRM_H_INC

#include ".\..\src\NavigatorDlg.h"
#include ".\..\src\dockbar.h"
#include ".\..\src\pagedlg.h"
#define MAX_NOF_ACCELERATORS 1024

extern CFont *TextWinFont;
extern UINT WainColumnTextFormat;

class SearchDialogClass;
class ReplaceDialogClass;

class ViewListElem
{
public:
  ViewListElem *m_rankNext;
  ViewListElem *m_rankPrev;
  ViewListElem();
  ~ViewListElem();
  std::string m_name;
  std::string m_shortName;
  class WainView *m_myView;
  int m_nr;
  int GetPosition() const { return m_position; }
  void SetPosition(int _position) { m_position = _position; }
  int m_position;
  int m_iconIndex;
  void SetName(const char* _fileName, bool _isFtpFile);
  void UpdateStatus(void);
  bool m_ftpFile;
  bool m_ignore;
  bool m_skip;
  bool m_reload;
  bool m_readOnly;
  time_t m_modifiedTime;
  long m_size;
};

typedef std::vector<const ViewListElem*> BrowseListType;

class ViewList
{
public:
  ViewList();
  ~ViewList();
  ViewListElem m_list;
  ViewListElem *m_currentView;
  ViewListElem *m_topView[2];
  int AddView(WainView *view, const char *file_name, bool IsFtpFile);
  BOOL RemoveView(int nr);
  void SetViewName(int nr, const char *name, bool IsFtpFile);
  ViewListElem *GetRankNext(ViewListElem *);
  ViewListElem *GetRankPrev(ViewListElem *);
  const ViewListElem *GetAbsNr(int nr);
  ViewListElem *PutInRankTop(int nr);
  int FindUniqueNr(void);
  ViewListElem *GetViewNr(int nr);

  // The List on the Browse bar, sorted by name.
  BrowseListType m_browseList;
};

struct MacroEntryInfoType
{
   int m_code;
   char *m_functionName;
   int m_info;
};

class MacroListEntryClass
{
   friend class MacroListClass;
   friend class MainFrame;
   MacroEntryInfoType m_info;
   MacroListEntryClass *m_prev;
   MacroListEntryClass *m_next;
   MacroListEntryClass();
   ~MacroListEntryClass();
};

class MacroListClass
{
public:
   MacroListEntryClass m_list;
   MacroListEntryClass *m_current;
   bool m_recording;
   MacroListClass();
   ~MacroListClass();
   void CleanUp(void);
};

struct BookmarkType
{
  WainView *m_view;
  int           m_lineNo;
  int           m_column;
};

#define NOF_BOOKMARKS 10

class JumpToTagElemClass
{
public:
   JumpToTagElemClass() {}
   JumpToTagElemClass(const char *aFileName, int aLineNo, int aColumn) :
      m_fileName(aFileName), m_lineNo(aLineNo), m_column(aColumn)
   { }
   std::string m_fileName;
   int m_lineNo;
   int m_column;

private:
};

class JumpToTagListClass
{
public:
   void Add(const char *aFileName, int aLineNo, int aColumn)
   {
      m_tagList.push_front(JumpToTagElemClass(aFileName, aLineNo, aColumn));
      if(m_tagList.size() > 50)
         m_tagList.pop_back();
   }
   bool Get(JumpToTagElemClass &aJumpToTagElem)
   {
      if(!m_tagList.empty())
      {
         aJumpToTagElem = *m_tagList.begin();
         m_tagList.pop_front();
         return true;
      }
      return false;
   }
   std::list <JumpToTagElemClass> m_tagList;
};

class BookmarkListDialogClass : public CDialog
{
  DECLARE_DYNAMIC(BookmarkListDialogClass);
public:
  BookmarkListDialogClass(CWnd *parent = NULL);
  ~BookmarkListDialogClass(void);
  enum {IDD = IDD_BOOKMARK_LIST_DIALOG};
  MainFrame *m_mf;
  int m_sel;
  int m_map[NOF_BOOKMARKS];
protected:
  virtual BOOL OnInitDialog(void);
  afx_msg void OnOk(void);
  DECLARE_MESSAGE_MAP();
};

class FileCheckElem
{
public:
   FileCheckElem(const char *aFileName)
    : m_fileName(aFileName),
      m_error(false),
      m_readOnly(false),
      m_size(0)
   {
   }
   std::string m_fileName;
   bool m_error;
   bool m_readOnly;
   time_t m_modifiedTime;
   long m_size;
};

class FileCheckListType : public std::vector<FileCheckElem *>
{
public:
   ~FileCheckListType()
   {
      std::vector<FileCheckElem *>::iterator idx;
      for(idx = begin(); idx != end(); ++idx)
        delete *idx;
      clear();
   }
};

enum DebugFileType
{
  NormalDebugFile,
  TagPeekDebugFile,
  ToolDebugFile
};

class MainFrame : public CMDIFrameWnd
{
  DECLARE_DYNAMIC(MainFrame);
  SearchDialogClass *m_searchDialog;
  ReplaceDialogClass *m_replaceDialog;
  CWinThread *m_cleanupThread;
  CWinThread *m_fileCheckThread;
  int m_replaceIdx;
  std::vector<std::string> m_globalSearchFiles;
  bool m_firstProjectReplace;
  bool m_firstGlobalReplace;
public:
  void ReenableFileCheck(const WainView *view, bool IsFtpFile);
  friend class BookmarkListDialogClass;
  friend class ViewListElem;
  friend class WainApp;
  void AddMacroEntry(int code, int add_info = 0);
  BOOL MacroRecordToggle(void);
  BOOL MacroRecordingOn(void);
  BOOL GetMacroEntry(MacroEntryInfoType *info, BOOL first);
  MacroListClass m_macroList;
  BOOL m_updateList;
  class ToolClass *m_tools;
  void PutInTopOfStack(int pos, int WinNr);
  void SetFileName(const char *file_name, int PropIndex);
  void RemoveFile(const char *name, BOOL do_close = FALSE);
  MainFrame();
  ViewList m_viewList;
  WainDoc *ActiveFile(const char *s);
  void HandleRecentProjects(const char *new_proj);
  WainDoc *OpenDebugFile(const char *_fileName, DebugFileType _fileType, bool _setFocus, const std::string& _toolPath, bool _isProject, bool _isMake, uint32_t _nr, const char* _viewName);
  void FileHasBeenSaved();
  WainView *GetActiveView(void);
  std::string m_lastWord;
  uint32_t m_lastSelectedWord = 0;
public:
  void AddBookmark(WainView *view, int column, int LineNo, int Bookmark_no);
  void GotoBookmark(int Bookmark_no);
  void AddJumpToTag(const char *pathname, int line, int column);
  void AddAutoTagList(const char *pathname, int line, int column);

  void MakePropSheet(int aCurrSel);
  void MakePageBarVisible(WainView *view);
  void UpdateViews(BOOL hard = FALSE);
  void SetupMenu(BOOL force = FALSE);

  void DoContextMenu(CPoint pt, const char *fn);

public:
  void MyGetClientRect(RECT *rect);
private:

  BOOL m_isInit;
  BookmarkType m_bookmark[NOF_BOOKMARKS];

  JumpToTagListClass m_jumpToTagList;
  JumpToTagListClass m_autoTagList;

  CString m_keyboardSetupFile;

  UINT m_statusBarTimer;
  void CalcWinRect(RECT *rect, int part);
  // void OnUpdateFrameTitle(BOOL aAddToTitle);

  std::string m_assocPath;
  std::vector<std::string >m_assocFileList;

  BOOL m_isContextMenu;
public:
  void ShowLineNo(bool show);
  void SaveAllDocs(void);
  void ProjectChanged(const char *file_name);
  void CheckFileStatus(void);
  BOOL m_doCheckFileStatus;
  UINT m_fileCheckTimer;
  void SetStatusText(const char *text);
  void OnFileCheckStatus(FileCheckListType *fc);

  LRESULT DoOpenFile(WPARAM aFileNameAtom, LPARAM aFileLineAtom);
  int AddView(WainView *view, const char *name, bool IsFtpFile);
  void RemoveView(int nr);
  void ActivateWin(int nr);
  void SetViewName(int nr, const char *name, bool IsFtpFile);
  virtual BOOL PreCreateWindow(CREATESTRUCT &cs);
  void SetScrollLock(void);

  WainView *GetOtherView(WainView *my_view);
  void GetCurrentWord(std::string &aWord);
  int GetNewWinRect(RECT *rect);
  void RecalcChildPos(bool force = false);
  virtual ~MainFrame();
  void SetLineStatus(class WainView *aView, int aLineNo, int aColumn);
  void SetInsertStatus(bool on);
  void SetSpecialModeStatus(const char *str);
  void SetMarkStatus(bool on);
  void SetModifiedStatus(WainView *aView, bool aOn);
  void SetMacroRecStatus(BOOL on);
  void SetCrLfStatus(BOOL to_unix);
  void SetReadOnlyStatus(bool aReadOnly);
  void SetScrollLock_status(BOOL on);
  void OpenDirForFile(const char* _filename);
  void OpenProjectForFile(const char* _filename);

  BOOL DoLoadMacro(const char *file_name);
  int TryCloseFile(const char *file_name);
  void CloseNormalDoc(class ChildFrame *frame);
  bool CloseDebugFile(const char *aFileName, bool aCloseBar, bool aCanCancel);
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif
  NavigatorBarClass m_navigatorBar;
  virtual BOOL PreTranslateMessage(MSG *msg);

  CStatusBar           m_statusBar;
  CToolBar             m_toolBar;
  PageBarDialogClass   m_pageBarDialog;
  NavigatorDialog m_navigatorDialog;
  PageBarClass         m_pageBar;
  size_t m_nofAccEntrys;
  ACCEL m_accEntry[MAX_NOF_ACCELERATORS];
  void OnToolTimer(class ProcessClass *aProcess, bool aDone);
  std::string m_lastTag;
protected:
  HACCEL m_accHandle;
  virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
  void RepositionBars(UINT nIDFirst, UINT nIDLast, UINT nIDLeftOver, UINT nFlag = CWnd::reposDefault, LPRECT lpRectParam = NULL, LPCRECT lpRectClient = NULL, BOOL bStretch = TRUE);
  virtual void RecalcLayout(BOOL bNotify = TRUE);
protected:
  afx_msg void OnActivateApp(BOOL active, DWORD task);
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void SelectFont(void);
  afx_msg void SelectNavigatorListFont(void);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnClose(void);
  afx_msg void SelectNavigatorBar(void);
  afx_msg void SelectNavigatorList(void);
  afx_msg void ViewNavigatorBar(void);
  afx_msg void NextWin(void);
  afx_msg void NextWinHere(void);
  afx_msg void PrevWinHere(void);
  afx_msg void PrevWin(void);
  afx_msg void SwapWin(void);
  afx_msg void ViewDir(void);
  afx_msg void ViewFiles(void);
  afx_msg void ViewTags(void);
  afx_msg void ViewProject(void);
  afx_msg void ViewCurrentTags(void);
  afx_msg void ViewClass(void);
  afx_msg void OnTimer(UINT timer_id);
  afx_msg void OnActivate(UINT state, CWnd *wnd_other, BOOL minimized);

  afx_msg void RedoTag(void);

  afx_msg LRESULT SearchFunc(WPARAM wparm, LPARAM lparam);
  afx_msg LRESULT GlobalReplaceFunc(WPARAM wparm, LPARAM lparam);
  afx_msg void ToolSetup(void);
  afx_msg void RunTool(UINT id);
  afx_msg void RunMacro(UINT id);
  afx_msg void MacroSetup(void);
  afx_msg void OtherWin(void);
  afx_msg void SaveMacro(void);
  afx_msg void LoadMacro(void);
  afx_msg void DocumentPropertiesSetup(void);
  afx_msg void KeyboardSetup(void);
  afx_msg void ProjectSetup(void);
  afx_msg void ProjectMake(void);
  afx_msg void ProjectMake0(void);
  afx_msg void ProjectMake1(void);
  afx_msg void ProjectMake2(void);
  afx_msg void ProjectMake3(void);
  afx_msg void ProjectMakeAll(void);
  afx_msg void ProjectExecute(void);
  afx_msg void ProjectExecute1(void);
  afx_msg void ProjectExecute2(void);
  afx_msg void ProjectExecute3(void);
  afx_msg void ProjectExecute4(void);
  afx_msg void ProjectOpen(void);
  afx_msg void ProjectSave(void);
  afx_msg void ProjectAddCurrentFile(void);
  afx_msg void ProjectManage(void);
  afx_msg void CloseFile(void);
  afx_msg void OpenFile(void);
  afx_msg void NewFile(void);
  afx_msg void Exit(void);
  afx_msg void SetBookmark(UINT id);
  afx_msg void JumpToBookmark(UINT id);
  afx_msg void BookmarkList(void);
  afx_msg void UndoJumpToTag(void);
  afx_msg BOOL OnBarCheck(UINT nID);
  afx_msg void OnDropFiles(HDROP drop_info);
  afx_msg void OnWindowPosChanged(WINDOWPOS FAR *window_pos);
  afx_msg void OpenRecentProject(UINT id);
  afx_msg void Help(void);
  afx_msg void Search(void);
  afx_msg void Replace(void);
  afx_msg void ProjectReplace();
  afx_msg void GlobalReplace();
  afx_msg void GotoOtherView(void);
  afx_msg void ToolOutputNext(void);
  afx_msg void ToolOutputPrev(void);
  afx_msg void AssocList(void);
  afx_msg void DoAssocList(UINT id);
  afx_msg void PopupMenuSetup(void);
  afx_msg void DoSaveAllDocs(void);
  afx_msg void DocPropSetup(void);
  afx_msg void JumpLastAutoTag(void);
  afx_msg BOOL OnMouseWheel(UINT flags, short delta, CPoint pt);
  afx_msg void ColorSetup(void);
  afx_msg void UserMenuSetup(void);

  afx_msg void OnMenuSelect(UINT ItemID, UINT Flags, HMENU hSysMenu);
  afx_msg void OnInitMenuPopup(CMenu *PopupMenu, UINT Index, BOOL SysMenu);
  afx_msg void OnDrawItem(int IDCtl, LPDRAWITEMSTRUCT DrawItemStruct);
  afx_msg void OnMeasureItem(int IDCtl, LPMEASUREITEMSTRUCT MeasureItemStruct );

  afx_msg void OnColor(void);
  afx_msg void SendToolCmd();
  DECLARE_MESSAGE_MAP();
};

// This enum must match indicators[] as defined in mainfrm.cpp
enum StatusIndexType
{
   STATUS_DUMMY1,
   STATUS_DUMMY2,
   STATUS_CAPS,
   STATUS_READONLY,
   STATUS_SCROLLLOCK,
   STATUS_REC,
   STATUS_INS,
   STATUS_MARK,
   STATUS_SPECIAL_MODE,
   STATUS_MODIFIED,
   STATUS_CRLF,
   STATUS_LINE
};

extern const char *MyStrIStr(const char *s1, const char *s2);
extern const char *MyStrIStr2(const char *s1, const char *s2);
extern void SetStatusText(const char *msg, ...);
extern MainFrame *GetMf();
extern bool IsFile(const char *filename);
extern bool IsDir(const char *pathname);
extern UINT CheckStatusMsgId;
extern UINT FileCheckThreadFunc(LPVOID parm);

#endif // MAINFRM_H_INC
