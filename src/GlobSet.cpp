//=============================================================================
// This source code file is a part of Wain.
// It implements GlobalSettingsClass as defined in yate.h.
// And SetupDialogClass.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\wain.h"

#include ".\..\src\MainFrm.h"
#include ".\..\src\ChildFrm.h"
#include ".\..\src\WainDoc.h"
#include ".\..\src\WainView.h"
#include ".\..\src\DocProp.h"
#include ".\..\src\keySetup.h"
#include ".\..\src\Project.h"
#include ".\..\src\Navigatorlist.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4100) // Unreferenced formal parameter

static void cipher_password(CString &word);
static void decipher_password(CString &word);

class SetupDialogClass : public DialogBaseClass
{
public:
   enum { IDD = IDD_SETUP};
   SetupDialogClass();
   ~SetupDialogClass();
   CreateModeType m_createMode;
   bool m_createDual;
   bool m_createDefault;
   bool m_createMax;
   bool m_loadFilesOnStartup;
   bool m_oneInstance;
   bool m_classicNavigatorBar;
   bool m_stripWhitespace;
   bool m_firstUpdate;
   bool m_horzScrollBar;
   bool m_vertScrollBar;
   bool m_stickyMark;
   bool m_visibleTabs;
   int  m_autoRebuildTime;
   std::string m_tempPath;
   std::string m_incPath;
   std::string m_maxSizeStr;
   std::string m_timeFormat;
   std::string m_dateFormat;
   std::string m_timeSep;
   std::string m_userId;
   std::string m_projectExtension;
   bool m_cuaMouseMarking;
   bool m_columnMouseMarking;
   bool m_flushUndoOnSave;
   bool m_showLineNo;
   bool m_navigatorBarShortNames;
   bool m_frameCurrLline;
   bool m_debugTools;
   bool m_useUserMenu;
protected:
   virtual void DoDataExchange(CDataExchange *dx);
   virtual BOOL OnInitDialog();
   afx_msg void BrowseIPath(void);
   afx_msg void BrowseTPath(void);
   afx_msg void LevelChanged(void);
   afx_msg void FlushSave(void);
   afx_msg void UseMaxSize(void);
   virtual void OnCancel(void);
   virtual void OnOK(void);
   DECLARE_MESSAGE_MAP();
};

BEGIN_MESSAGE_MAP(SetupDialogClass, CDialog)
   ON_BN_CLICKED(ID_SETUP_BROWSE_TPATH, BrowseTPath)
   ON_BN_CLICKED(ID_SETUP_BROWSE_IPATH, BrowseIPath)
   ON_EN_CHANGE(IDC_AUTO_REBUILD_CTRL, LevelChanged)
   ON_BN_CLICKED(IDC_FLUSH_SAVE, FlushSave)
   ON_BN_CLICKED(IDC_MAX_ENTRIS, UseMaxSize)
END_MESSAGE_MAP();


void SetupDialogClass::BrowseIPath(void)
{
   BROWSEINFO bi;
   char temp[_MAX_PATH];
   bi.hwndOwner = m_hWnd;
   bi.pidlRoot = NULL;
   bi.pszDisplayName = temp;
   bi.lpszTitle = "Select include path";
   bi.ulFlags = 0;
   bi.lpfn = NULL;
   bi.lParam = 0;
   bi.iImage = 0;
   LPITEMIDLIST item_list = SHBrowseForFolder(&bi);
   if(item_list)
   {
      SHGetPathFromIDList(item_list, temp);
      m_incPath = temp;
      SetDlgItemText(IDC_SETUP_IPATH, m_incPath.c_str());
   }
}

void SetupDialogClass::BrowseTPath(void)
{
  BROWSEINFO bi;
  char temp[_MAX_PATH];
  bi.hwndOwner = m_hWnd;
  bi.pidlRoot = NULL;
  bi.pszDisplayName = temp;
  bi.lpszTitle = "Select temp path";
  bi.ulFlags = 0;
  bi.lpfn = NULL;
  bi.lParam = 0;
  bi.iImage = 0;
  LPITEMIDLIST item_list = SHBrowseForFolder(&bi);
  if(item_list)
  {
    SHGetPathFromIDList(item_list, temp);
    m_tempPath = temp;
    SetDlgItemText(IDC_SETUP_TPATH, m_tempPath.c_str());
  }
}

void SetupDialogClass::LevelChanged(void)
{
  if(m_firstUpdate)
  {
    m_firstUpdate = FALSE;
    return;
  }
  CEdit *e = (CEdit *)GetDlgItem(IDC_AUTO_REBUILD_CTRL);
  char text[20];
  e->GetWindowText(text, 20);
  m_autoRebuildTime = (int )strtol(text, NULL, 10);
  if(m_autoRebuildTime > 120)
  {
    m_autoRebuildTime = 120;
    sprintf(text, "%d", m_autoRebuildTime);
    e->SetWindowText(text);
  }
  else if(m_autoRebuildTime < 0)
  {
    m_autoRebuildTime = 0;
    sprintf(text, "%d", m_autoRebuildTime);
    e->SetWindowText(text);
  }
}

SetupDialogClass::SetupDialogClass() : DialogBaseClass(SetupDialogClass::IDD)
{
  m_createDefault = m_createMax = m_createDual = FALSE;
  m_firstUpdate = TRUE;
}

SetupDialogClass::~SetupDialogClass()
{

}

BOOL SetupDialogClass::OnInitDialog(void)
{
  UpdateData(FALSE);
  CSpinButtonCtrl *sb = (CSpinButtonCtrl *)GetDlgItem(IDC_AUTO_REBUILD_SPIN);
  sb->SetRange(0, 120);
  CEdit *e = (CEdit *)GetDlgItem(IDC_AUTO_REBUILD_CTRL);
  char text[256];
  if(m_autoRebuildTime > 120)
    m_autoRebuildTime = 120;
  else if(m_autoRebuildTime < 0)
    m_autoRebuildTime = 0;

  sprintf(text, "%d", m_autoRebuildTime);
  e->SetWindowText(text);

  if(m_flushUndoOnSave)
    FlushSave();
  else
    UseMaxSize();

  return TRUE;
}

void SetupDialogClass::FlushSave(void)
{
  m_flushUndoOnSave = TRUE;
  CButton *b = (CButton *)GetDlgItem(IDC_FLUSH_SAVE);
  ASSERT(b);
  b->SetCheck(TRUE);
  b = (CButton *)GetDlgItem(IDC_MAX_ENTRIS);
  ASSERT(b);
  b->SetCheck(FALSE);
  CEdit *e = (CEdit *)GetDlgItem(IDC_MAX_ENTRIS_EDIT);
  ASSERT(e);
  e->SetReadOnly(TRUE);
}

void SetupDialogClass::UseMaxSize(void)
{
  m_flushUndoOnSave = FALSE;
  CButton *b = (CButton *)GetDlgItem(IDC_FLUSH_SAVE);
  ASSERT(b);
  b->SetCheck(FALSE);
  b = (CButton *)GetDlgItem(IDC_MAX_ENTRIS);
  ASSERT(b);
  b->SetCheck(TRUE);
  CEdit *e = (CEdit *)GetDlgItem(IDC_MAX_ENTRIS_EDIT);
  ASSERT(e);
  e->SetReadOnly(FALSE);
}

void SetupDialogClass::DoDataExchange(CDataExchange *dx)
{
  CDialog::DoDataExchange(dx);

  DdxString(dx, IDC_SETUP_TPATH, m_tempPath);
  DdxString(dx, IDC_SETUP_IPATH, m_incPath);
  DdxString(dx, IDC_MAX_ENTRIS_EDIT, m_maxSizeStr);

  DdxCheck(dx, IDC_STRIP_WS, m_stripWhitespace);
  DdxCheck(dx, IDC_ONE_APP, m_oneInstance);
  DdxCheck(dx, IDC_LOAD_F, m_loadFilesOnStartup);

  DdxCheck(dx, IDC_CREATE_DUAL, m_createDual);
  DdxCheck(dx, IDC_CREATE_MAX,  m_createMax);
  DdxCheck(dx, IDC_CREATE_DEF,  m_createDefault);
  DdxCheck(dx, IDC_MARKING_COLUMN, m_columnMouseMarking);
  DdxCheck(dx, IDC_MARKING_CUA, m_cuaMouseMarking);
  DdxCheck(dx, IDC_HORZ_SCROLLBAR, m_horzScrollBar);
  DdxCheck(dx, IDC_VERT_SCROLLBAR, m_vertScrollBar);
  DdxCheck(dx, IDC_STICKY_MARK, m_stickyMark);
  DdxCheck(dx, IDC_VISIBLE_TABS, m_visibleTabs);
  DdxCheck(dx, IDC_SHOW_LINE_NO, m_showLineNo);
  DdxCheck(dx, IDC_SHORT_NAMES,  m_navigatorBarShortNames);
  DdxCheck(dx, IDC_FRAME_LINE,   m_frameCurrLline);
  DdxCheck(dx, IDC_DEBUG_TOOLS,  m_debugTools);
  DdxCheck(dx, IDC_USE_USER_MENU,m_useUserMenu);

  DdxString(dx, IDC_SETUP_TIME_FORMAT, m_timeFormat);
  DdxString(dx, IDC_SETUP_DATE_FORMAT, m_dateFormat);
  DdxString(dx, IDC_SETUP_TIME_SEP, m_timeSep);
  DdxString(dx, IDC_SETUP_USER_ID,  m_userId);
  DdxString(dx, IDC_SETUP_PROJ_EXT, m_projectExtension);
}

void SetupDialogClass::OnCancel(void)
{
  EndDialog(IDCANCEL);
}

void SetupDialogClass::OnOK(void)
{
  UpdateData(TRUE);
  if(m_tempPath.empty())
  {
    WainMessageBox(this, "Temp path is empty", IDC_MSG_OK, IDI_ERROR_ICO);
    return;
  }
  std::string Temp = m_tempPath;
  RemoveSlash(Temp);

  if(!IsDir(Temp.c_str()))
  {
    WainMessageBox(this, "Temp path is not a directory", IDC_MSG_OK, IDI_ERROR_ICO);
    return;
  }
  EndDialog(IDOK);
}

GlobalSettingsClass::GlobalSettingsClass()
{

}

GlobalSettingsClass::~GlobalSettingsClass()
{
}

void GlobalSettingsClass::Setup(void)
{
  SetupDialogClass setup_dialog;

  setup_dialog.m_loadFilesOnStartup = m_loadFilesOnStartup;
  setup_dialog.m_oneInstance = m_oneInstance;

  setup_dialog.m_stripWhitespace = m_stripWhitespace;
  setup_dialog.m_incPath = m_incPath;
  setup_dialog.m_tempPath = m_tempPath;
  setup_dialog.m_autoRebuildTime = m_autoTagRebuildTime;
  setup_dialog.m_horzScrollBar = m_horzScrollBar;
  setup_dialog.m_vertScrollBar = m_vertScrollBar;
  setup_dialog.m_flushUndoOnSave = m_flushUndoOnSave;
  setup_dialog.m_maxSizeStr = ToString(m_maxUndoBufferSize);
  setup_dialog.m_stickyMark = m_stickyMark;
  setup_dialog.m_visibleTabs = m_visibleTabs;
  setup_dialog.m_showLineNo = m_showLineNo;
  setup_dialog.m_navigatorBarShortNames = m_navigatorBarShortNames;
  setup_dialog.m_frameCurrLline = m_frameCurrLine;
  setup_dialog.m_debugTools = m_debugTools;
  setup_dialog.m_useUserMenu = m_useUserMenu;
  setup_dialog.m_timeFormat = m_timeFormat;
  setup_dialog.m_dateFormat = m_dateFormat;
  setup_dialog.m_timeSep    = m_timeSep;
  setup_dialog.m_userId =     m_userId;
  setup_dialog.m_projectExtension = m_projectExtension;

  if(m_createMode == CreateModeType::CREATE_DUAL)
    setup_dialog.m_createDual = true;
  else if(m_createMode == CreateModeType::CREATE_MAX)
    setup_dialog.m_createMax = true;
  else
    setup_dialog.m_createDefault = true;
  if(m_cuaMouseMarking)
  {
    setup_dialog.m_cuaMouseMarking = TRUE;
    setup_dialog.m_columnMouseMarking = FALSE;
  }
  else
  {
    setup_dialog.m_cuaMouseMarking = FALSE;
    setup_dialog.m_columnMouseMarking = TRUE;
  }

  if(setup_dialog.DoModal() == IDOK)
  {
    if(setup_dialog.m_createDual)
      m_createMode = CreateModeType::CREATE_DUAL;
    else if(setup_dialog.m_createMax)
      m_createMode = CreateModeType::CREATE_MAX;
    else
      m_createMode = CreateModeType::CREATE_DEFAULT;
    m_loadFilesOnStartup = setup_dialog.m_loadFilesOnStartup;
    if(m_oneInstance != setup_dialog.m_oneInstance)
    {
      m_oneInstance = setup_dialog.m_oneInstance;
      wainApp.WriteProfileInt("Settings", "OneInst", m_oneInstance);
    }
    m_incPath = setup_dialog.m_incPath;

    m_tempPath = setup_dialog.m_tempPath;
    AddSlash(m_tempPath);

    m_stripWhitespace = setup_dialog.m_stripWhitespace;
    m_autoTagRebuildTime = setup_dialog.m_autoRebuildTime;
    m_cuaMouseMarking = setup_dialog.m_cuaMouseMarking;

    m_horzScrollBar = setup_dialog.m_horzScrollBar;
    m_vertScrollBar = setup_dialog.m_vertScrollBar;
    MainFrame *mf = GetMf();
    ViewListElem *elem = NULL;
    m_stickyMark = setup_dialog.m_stickyMark;
    m_visibleTabs = setup_dialog.m_visibleTabs;
    while((elem = mf->m_viewList.GetRankNext(elem)) != NULL)
      elem->m_myView->SetScrollInfo();
    m_flushUndoOnSave = setup_dialog.m_flushUndoOnSave;
    m_maxUndoBufferSize = FromString<int>(setup_dialog.m_maxSizeStr);
    if(m_maxUndoBufferSize < 0)
      m_maxUndoBufferSize = 0;
    if(m_showLineNo != setup_dialog.m_showLineNo)
    {
      m_showLineNo = setup_dialog.m_showLineNo;
      GetMf()->ShowLineNo(m_showLineNo ? true : false);
    }
    if(m_navigatorBarShortNames != setup_dialog.m_navigatorBarShortNames)
    {
      m_navigatorBarShortNames = setup_dialog.m_navigatorBarShortNames;
      GetMf()->m_navigatorDialog.m_project->SortFileList();
      GetMf()->m_navigatorDialog.m_navigatorList->ReDraw();
    }
    m_frameCurrLine = setup_dialog.m_frameCurrLline;
    m_debugTools = setup_dialog.m_debugTools;
    if(m_useUserMenu != setup_dialog.m_useUserMenu)
    {
      m_useUserMenu = setup_dialog.m_useUserMenu;
      mf->SetupMenu(TRUE);
    }
    mf->UpdateViews();
    m_timeFormat = setup_dialog.m_timeFormat;
    m_dateFormat = setup_dialog.m_dateFormat;
    m_timeSep    = setup_dialog.m_timeSep;
    m_userId     = setup_dialog.m_userId;

    // Make sure project extension starts with one .
    const char *s = setup_dialog.m_projectExtension.c_str();
    while(*s == '.')
      s++;
    m_projectExtension = ".";
    m_projectExtension += s;
  }
}

bool IsDir(const char *pathname)
//  Description:
//    Function used to check if pathname is a dir
//  Parameters:
//    pathname: The name to check
{
  if(pathname[0] && pathname[strlen(pathname) - 1] == '\\')
  {
    char temp[MAX_PATH];
    strcpy(temp, pathname);
    temp[strlen(temp) - 1] = 0;

    struct _stat buffer;
    if(_stat(temp, &buffer))  /* _stat() return 0 on ok */
      return false;
    if(buffer.st_mode & _S_IFDIR)
      return true;
    return false;
  }

  struct _stat buffer;
  if(_stat(pathname, &buffer))  /* _stat() return 0 on ok */
    return false;
  if(buffer.st_mode & _S_IFDIR)
    return true;
  return false;
}

bool GlobalSettingsClass::ReadProfile()
{
  bool first_time = false;
  int i = wainApp.GetProfileInt("Settings", "DispMode", -1);
  if(i == -1)
    first_time = true;
  if(i != -1 && i <= int(CreateModeType::CREATE_DUAL))
    m_createMode = (CreateModeType )i;
  else
    m_createMode = CreateModeType::CREATE_DEFAULT;

  i = wainApp.GetProfileInt("Settings", "OneInst", -1);
  if(i != -1)
    m_oneInstance = i ? TRUE : FALSE;
  else
    m_oneInstance = TRUE;

  i = wainApp.GetProfileInt("Settings", "LoadFiles", -1);
  if(i != -1)
    m_loadFilesOnStartup = i ? TRUE : FALSE;
  else
    m_loadFilesOnStartup = TRUE;

  m_frameCurrLine = wainApp.GetProfileBool("Settings", "FrameCurrLine", true);
  m_debugTools = wainApp.GetProfileBool("Settings", "Debug Tools", true);
  m_autoTagRebuildTime = wainApp.GetProfileInt("Settings", "Tag Auto Rebuild Time", 10);
  m_cuaMouseMarking = wainApp.GetProfileBool("Settings", "Cua Mouse Mraking", false);
  m_viewNavigatorBar = wainApp.GetProfileInt("Settings", "ViewBrowseBar", AFX_IDW_DOCKBAR_LEFT);
  m_viewToolBar = wainApp.GetProfileBool("Settings", "ViewToolBar", true);
  m_viewStatusBar = wainApp.GetProfileBool("Settings", "ViewStatusBar", true);
  m_stripWhitespace = wainApp.GetProfileBool("Settings", "StripWhitespace", true);

  m_horzScrollBar = wainApp.GetProfileBool("Settings", "Horz Scroll Bar", true);
  m_vertScrollBar = wainApp.GetProfileBool("Settings", "Vert Scroll Bar", true);

  CString str;
  str = wainApp.GetProfileString("Settings", "IncPath", "");
  m_incPath = str;

  m_winMode = wainApp.GetProfileInt("Win Size", "Mode", 0);
  m_winX = wainApp.GetProfileInt("Win Size", "X", 0);
  m_winY = wainApp.GetProfileInt("Win Size", "Y", 0);
  m_winCx = wainApp.GetProfileInt("Win Size", "CX", 200);
  m_winCy = wainApp.GetProfileInt("Win Size", "CY", 200);

  m_navigatorBarWidth = wainApp.GetProfileInt("Browse Bar", "Width", 200);
  m_widthRatio = wainApp.GetProfileInt("Browse Bar", "Width Ratio", 50);
  m_navigatorBarShortNames = wainApp.GetProfileBool("Browse Bar", "Short Names", false);
  m_pageBarHeight = wainApp.GetProfileInt("Settings", "PageBarHeight", 100);
  ReadFont(&m_textWinLogFont, "Def font");
  ReadFont(&m_navigatorListLogFont, "Browse bar font");
  ReadFont(&m_printerFont, "PrinterFont");

  str = wainApp.GetProfileString("Project", "Name", "");
  strcpy(m_projectName, str);

  char file_name[_MAX_PATH];
  strcpy(file_name, wainApp.m_pszHelpFilePath);
  char path[_MAX_PATH];
  MySplitPath(file_name, SP_DRIVE | SP_DIR, path);
  char *s = &path[strlen(path) - 1];
  if(*s == '\\')
    s--;
  while(*s != '\\' && *s && s != path)
    s--;
  *s = '\0';
  strcat(path, "\\Config\\");
  strcat(m_configPath, path);

  str = wainApp.GetProfileString("Settings", "TempPath", "");
  m_tempPath = str;
  if(m_tempPath.empty())
  {
    m_tempPath = m_configPath;
    RemoveLastPath(m_tempPath);
    m_tempPath += "\\temp";
    if(!IsDir(m_tempPath.c_str()))
    {
      if(!CreateDirectory(m_tempPath.c_str(), NULL))
        m_tempPath = "";
      else
        m_tempPath += "\\";
    }
    else
        m_tempPath += "\\";
  }

  sprintf(file_name, "%sDefault.ext", m_configPath);
  DocPropClass *dp = new DocPropClass;
  if(!ReadExtFile(file_name, dp))
  {
    WainMessageBox(GetMf(), "Failed to find Default.ext", IDC_MSG_OK, IDI_ERROR_ICO);
  }
  else
  {
    m_docProp.push_back(dp);
    dp = 0;
  }

  sprintf(file_name, "%s*.ext", m_configPath);
  struct _finddata_t find_data;
  long handle = _findfirst(file_name, &find_data);
  char path_name[_MAX_PATH];
  if(handle != -1)
  {
    sprintf(path_name, "%s%s", m_configPath, find_data.name);
    if(!dp)
       dp = new DocPropClass;
    if(ReadExtFile(path_name, dp))
    {
       m_docProp.push_back(dp);
       dp = 0;
    }
    while(_findnext(handle, &find_data) == 0)
    {
      if(stricmp(find_data.name, "default.ext"))
      {
        sprintf(path_name, "%s%s", m_configPath, find_data.name);
        if(!dp)
           dp = new DocPropClass;
        if(ReadExtFile(path_name, dp))
        {
           m_docProp.push_back(dp);
           dp = 0;
        }
      }
    }
  }

  CString tstr;
  bool Done;
  for(i = 0, Done = false; i < MAX_NOF_SEARCH_STRINGS && !Done; i++)
  {
    tstr.Format("String %d", i);
    tstr = wainApp.GetProfileString("Search", tstr, "");
    if(tstr.IsEmpty())
      Done = true;
    else
      m_searchStringList.push_back((const char *)tstr);
  }
  for(i = 0, Done = false; i < MAX_NOF_SEARCH_STRINGS && !Done; i++)
  {
    tstr.Format("String %d", i);
    tstr = wainApp.GetProfileString("Replace", tstr, "");
    if(tstr.IsEmpty())
      Done = true;
    else
      m_replaceStringList.push_back((const char *)tstr);
  }

  for(i = 0; i < MAX_NOF_RECENT_PROJECTS; i++)
  {
    tstr.Format("File %d", i);
    str = wainApp.GetProfileString("Recent Projects", tstr, "");
    strcpy(m_recentProject[i], str);
  }
  str = wainApp.GetProfileString("Settings", "Last Dir", "");
  if(str.IsEmpty())
  {
    _getcwd(m_lastUsedDir, _MAX_PATH);
  }
  else
    strcpy(m_lastUsedDir, str);
  FILE *f;

  sprintf(file_name, "%smacro.set", m_configPath);
  if((f = fopen(file_name, "rt")) != NULL)
  {
    char temp[1024];
    // char *s;
    while(fgets(temp, 1024, f) && m_macroParmList.size() < MAX_NUM_MACRO)
    {
      if((s = strtok(temp, "\"")) != NULL)
      {
         MacroParmType MacroParm;
         MacroParm.m_fileName = s;
         if((s = strtok(NULL, "\n")) != NULL)
         {
           while(*s == ' ')
             s++;
           MacroParm.m_menuText = s;
           m_macroParmList.push_back(MacroParm);
         }
      }
    }
    fclose(f);
  }
  str = wainApp.GetProfileString("PopupMenu", "Items", "");
  if(str.IsEmpty())
  {
    m_popupMenu.push_back(PopupMenuListClass("", IDM_FILE_OPEN));
    m_popupMenu.push_back(PopupMenuListClass("", IDM_CLOSE_FILE));
    m_popupMenu.push_back(PopupMenuListClass("", IDM_POPUP_MENU_SETUP));
  }
  else
  {
    std::string Temp(str);
    std::string W;
    for(i = 0; !(W = StrTok(Temp, " ")).empty() && i < MAX_NOF_POPUP_MENU_ITEMS; i++)
    {
      m_popupMenu.push_back(PopupMenuListClass("", FromString<WORD>(W)));
    }
  }
  str = wainApp.GetProfileString("UserMenu", "Items", "");
  if(str.IsEmpty())
  {
    m_userMenu.push_back(PopupMenuListClass("", IDV_GOTO_LINE_NO));
    m_userMenu.push_back(PopupMenuListClass("", IDV_INC_SEARCH));
    m_userMenu.push_back(PopupMenuListClass("", IDV_SEARCH_NEXT));
  }
  else
  {
    std::string Temp(str);
    std::string W;
    for(i = 0; !(W = StrTok(Temp, " ")).empty() && i < MAX_NOF_POPUP_MENU_ITEMS; i++)
    {
      m_userMenu.push_back(PopupMenuListClass("",  FromString<WORD>(W)));
    }
  }
  size_t idx;
  for(idx = 0; idx < m_userMenu.size(); idx++)
  {
    const char* s2 = MsgId2FuncName(m_userMenu[idx].m_id);
    if(s2)
      m_userMenu[idx].m_text = s2;
    else
      m_userMenu[idx].m_text = "---";
  }
  m_useUserMenu = wainApp.GetProfileBool("UserMenu", "Display", true);

  m_maxUndoBufferSize = wainApp.GetProfileInt("Settings", "MaxUndoBufferSize", 200);
  m_flushUndoOnSave = wainApp.GetProfileBool("Settings", "FlushUndoOnSave", true);
  m_stickyMark = wainApp.GetProfileBool("Settings", "Sticky Mark", true);
  m_visibleTabs = wainApp.GetProfileBool("Settings", "Visible Tabs", false);
  m_showLineNo = wainApp.GetProfileBool("Settings", "Show Line No", false);

  m_tagListColumnWidth[0] = wainApp.GetProfileInt("Settings", "Column Width 0", 130);
  m_tagListColumnWidth[1] = wainApp.GetProfileInt("Settings", "Column Width 1", 100);
  m_tagListColumnWidth[2] = wainApp.GetProfileInt("Settings", "Column Width 2",  45);
  m_tagListColumnWidth[3] = wainApp.GetProfileInt("Settings", "Column Width 3", 150);
  for(i = 0; i < 4; i++)
    if(m_tagListColumnWidth[i] < 20)
      m_tagListColumnWidth[i] = 20;
  m_tagListCx = wainApp.GetProfileInt("Settings", "TaglistCX", 575);
  m_tagListCy = wainApp.GetProfileInt("Settings", "TaglistCY", 200);

  m_wordListCx = wainApp.GetProfileInt("Settings", "WordlistCX", 575);
  m_wordListCy = wainApp.GetProfileInt("Settings", "WordlistCY", 200);

  m_listColorBack = wainApp.GetProfileInt("BrowseList", "ColorBack", RGB(255, 255, 255));
  m_listColorText = wainApp.GetProfileInt("BrowseList", "ColorText", 0);
  m_listColorTextSel = wainApp.GetProfileInt("BrowseList", "ColorTextSel", RGB(255, 255, 255));
  m_listColorSel = wainApp.GetProfileInt("BrowseList", "ColorSel",  RGB(160, 160, 160));
  m_listColorSelFocus = wainApp.GetProfileInt("BrowseList", "ColorSelFocus",  RGB(10, 36, 106));

  m_ftpHostName = wainApp.GetProfileString("FTP", "HostName", "");
  m_ftpUserName = wainApp.GetProfileString("FTP", "UserName", "");
  m_ftpSavePassword = wainApp.GetProfileBool("FTP", "SavePassWord", false);
  if(m_ftpSavePassword)
  {
    m_ftpPassword = wainApp.GetProfileString("FTP", "PassWord", "");
    decipher_password(m_ftpPassword);
  }
  else
    m_ftpPassword = "";

  m_printColor = (PrintColorType )wainApp.GetProfileInt("Print", "Black&White", PC_DOCUMENT);
  m_printScale = wainApp.GetProfileInt("Print", "Scale", 100);
  m_printHeader = wainApp.GetProfileString("Print", "Head", "\\t\\F");
  m_printFooter = wainApp.GetProfileString("Print", "Foot", "\\D-\\T\\t\\t\\P/\\C");
  m_printLineNo = wainApp.GetProfileBool("Print", "LineNr", false);
  m_printMargin = wainApp.GetProfileInt("Print", "Margin", 10);
  m_printWrap = wainApp.GetProfileBool("Print", "Wrap", true);

  m_cleanupYear  = wainApp.GetProfileInt("CleanUp", "Year", 2002);
  m_cleanupMonth = wainApp.GetProfileInt("CleanUp", "Month", 2);
  m_cleanupDay   = wainApp.GetProfileInt("CleanUp", "Day",  2);

  m_timeFormat = wainApp.GetProfileString("Format", "Time", "%X");
  m_dateFormat = wainApp.GetProfileString("Format", "Date", "%x");
  m_timeSep    = wainApp.GetProfileString("Format", "TimeSep", "-");
  m_userId =     wainApp.GetProfileString("Format",  "UserId", "Unknown");

  int ci;
  char col_str[256];
  for(ci = 0; ci < NOF_KEY_INDEX; ci++)
  {
    sprintf(col_str, "Color %d ForeG", ci);
    m_printingColor[ci].m_textColor = wainApp.GetProfileInt("Print", col_str, 0);
    sprintf(col_str, "Color %d backG", ci);
    m_printingColor[ci].m_backColor = wainApp.GetProfileInt("Print", col_str, RGB(255, 255, 255));
  }
  m_projectExtension = wainApp.GetProfileString("Settings", "ProjectExtension", ".wpj");

  m_tvcColor[TVC_NORMAL].m_textColor = wainApp.GetProfileInt("TabView", "NormText", RGB(0, 0, 0));
  m_tvcColor[TVC_NORMAL].m_backColor = wainApp.GetProfileInt("TabView", "NormBack", RGB(255, 255, 255));
  m_tvcColor[TVC_CHANGED].m_textColor = wainApp.GetProfileInt("TabView", "ChangedText", RGB(0, 0, 0));
  m_tvcColor[TVC_CHANGED].m_backColor = wainApp.GetProfileInt("TabView", "ChangedBack", RGB(255, 255, 0));
  m_tvcColor[TVC_CHANGED_SAVED].m_textColor = wainApp.GetProfileInt("TabView", "SavedText", RGB(0, 0, 0));
  m_tvcColor[TVC_CHANGED_SAVED].m_backColor = wainApp.GetProfileInt("TabView", "SavedBack", RGB(240, 255, 240));
  m_tvcColor[TVC_BOOKMARK].m_textColor = wainApp.GetProfileInt("TabView", "BookmarkText", RGB(80, 80, 0));
  m_tvcColor[TVC_BOOKMARK].m_backColor = wainApp.GetProfileInt("TabView", "BookmarkBack", RGB(255, 160, 255));

  return first_time;
}

bool GlobalSettingsClass::ReadExtFile(const char *aFileName, DocPropClass *aDocProp)
{
  FILE *f = fopen(aFileName, "rt");
  if(!f)
    return FALSE;
  char line[1024];
  BOOL Done = FALSE;

  int i;
  int state;
  aDocProp->m_tabPos.clear();

  aDocProp->m_fileName = aFileName;
  aDocProp->m_tabSize = 2;
  aDocProp->m_indent = 2;
  aDocProp->m_numberStr = "";
  memset(&aDocProp->m_color, 0, sizeof(aDocProp->m_color));
  aDocProp->m_color[MATCH_IDX].m_textColor = 0xFFFFFF;
  aDocProp->m_color[MATCH_IDX].m_backColor = 0xFFC0A0;

  state = 0;
  char *end;
  while(fgets(line, 1024, f) && !feof(f) && !Done)
  {
    if((end = strchr(line, '\n')) != NULL)
      *end = '\0';
    else
      end = &line[strlen(line) - 1];

    if(line[0] == '[')
    {
      char *nstr = strtok(line, "[]\n ");
      if(!stricmp(nstr, "Keyword"))
      {
        nstr = strtok(NULL, " ");
        if(nstr)
        {
          state = (int )strtol(nstr, NULL, 0);
          if(state < 0 || state > 4)
          {
            WainMessageBox(GetMf(), "Wrong keyword in keyword file", IDC_MSG_OK, IDI_WARNING_ICO);
            Done = TRUE;
          }
        }
        else
        {
          WainMessageBox(GetMf(), "Wrong keyword in keyword file", IDC_MSG_OK, IDI_WARNING_ICO);
          Done = TRUE;
        }
      }
      else if(!stricmp(nstr, "ExtensionFile"))
      {
        state = -1;
      }
      else if(!stricmp(nstr, "Color"))
      {
        state = -2;
      }
      else if(!stricmp(nstr, "Tags"))
      {
        state = -3;
      }
      else if(!stricmp(nstr, "Templates"))
      {
        state = -4;
      }
      else
      {
        WainMessageBox(GetMf(), "Wrong keyword in keyword file", IDC_MSG_OK, IDI_WARNING_ICO);
        Done = TRUE;
      }
    }
    else if(state == -1)
    {
      char *x = strtok(line, ": ");
      if(x)
      {
        char *y;
        if(stricmp(x, "TabPos") && stricmp(x, "SEPS"))
          y = strtok(NULL, " ");
        else
          y = line + strlen(x) + 2;
        if(y)
        {
          if(!stricmp(x, "EXTENSIONS"))
          {
            aDocProp->m_extensions = y;
          }
          else if(!stricmp(x, "SEPS"))
          {
            std::string T(y);
            ConvToTab(T);
            aDocProp->m_seps = T;
            if((aDocProp->m_seps.find(' ')) == std::string::npos)
               aDocProp->m_seps += " ";
          }
          else if(!stricmp(x, "STRING"))
          {
            aDocProp->m_stringDelim = y;
          }
          else if(!stricmp(x, "CHAR"))
          {
            aDocProp->m_chDelim = y;
          }
          else if(!stricmp(x, "BRACES"))
          {
            aDocProp->m_braces = y;
          }
          else if(!stricmp(x, "BLOCKBEGIN"))
          {
            aDocProp->m_blockBegin = y;
          }
          else if(!stricmp(x, "BLOCKEND"))
          {
            aDocProp->m_blockEnd = y;
          }
          else if(!stricmp(x, "LINECOMMENT"))
          {
            aDocProp->m_lineComment = y;
          }
          else if(!stricmp(x, "COMMENTBEGIN"))
          {
            aDocProp->m_commentBegin = y;
          }
          else if(!stricmp(x, "COMMENTEND"))
          {
            aDocProp->m_commentEnd = y;
          }
          else if(!stricmp(x, "COMMENTWHOLEWORD"))
          {
            if(*y == '0')
              aDocProp->m_commentWholeWord = false;
            else
              aDocProp->m_commentWholeWord = true;
          }
          else if(!stricmp(x, "COMMENTFIRSTWORD"))
          {
            if(*y == '0')
              aDocProp->m_commentFirstWord = false;
            else
              aDocProp->m_commentFirstWord = true;
          }
          else if(!stricmp(x, "PREPROCESSOR"))
          {
            aDocProp->m_preProcessor = y;
          }
          else if(!stricmp(x, "IndentAfter"))
          {
            do
            {
              aDocProp->m_indentAfter.push_back(y);
            }
            while((y = strtok(0, ", ")) != 0);
          }
          else if(!stricmp(x, "IndentUnless"))
          {
            do
            {
              aDocProp->m_indentUnless.push_back(y);
            }
            while((y = strtok(0, ", ")) != 0);
          }
          else if(!stricmp(x, "Unindent"))
          {
            do
            {
              aDocProp->m_unindent.push_back(y);
            }
            while((y = strtok(0, ", ")) != 0);
          }
          else if(!stricmp(x, "IGNORECASE"))
          {
            if(*y == '0')
              aDocProp->m_ignoreCase = false;
            else
              aDocProp->m_ignoreCase = true;
          }
          else if(!stricmp(x, "EXTENSIONTYPE"))
          {
            aDocProp->m_extensionType = y;
          }
          else if(!stricmp(x, "LITERAL"))
          {
            aDocProp->m_literal = y;
          }
          else if(!stricmp(x, "LINECONTINUATION"))
          {
            aDocProp->m_lineCon = y;
          }
          else if(!stricmp(x, "TabSize"))
          {
            aDocProp->m_tabSize = strtol(y, NULL, 0);
            aDocProp->m_indent = aDocProp->m_tabSize;
          }
          else if(!stricmp(x, "Indent"))
          {
            aDocProp->m_indent = strtol(y, NULL, 0);
          }
          else if(!stricmp(x, "TabPos"))
          {
             aDocProp->m_tabPos.FromString(y);
          }
          else if(!stricmp(x, "PreProcWords"))
          {
            do
            {
              aDocProp->m_preProcWord.push_back(y);
            }
            while((y = strtok(NULL, " ")) != 0);
          }
          else if(!stricmp(x, "Numbers"))
          {
            aDocProp->m_numberStr = y;
            try
            {
               aDocProp->m_numberRegEx = aDocProp->m_numberStr.c_str();
            }
            catch (const std::regex_error& err)
            {
              CString msg = "The number regexp string did not compile\r\n";
              msg += "The error is: ";
              msg += err.what();
              WainMessageBox(GetMf(), msg, IDC_MSG_OK, IDI_ERROR_ICO);
            }
          }
          else if(!stricmp(x, "BlockBeginWords"))
          {
             aDocProp->m_blockBeginWords = SplitStringList(y, ';');
          }
          else if(!stricmp(x, "BlockEndWords"))
          {
             aDocProp->m_blockEndWords = SplitStringList(y, ';');
          }
          else
          {
            WainMessageBox(GetMf(), "Wrong word in ext file", IDC_MSG_OK, IDI_WARNING_ICO);
            Done = TRUE;
          }
        }
      }
    }
    else if(state == -2)
    {
      char *x = strtok(line, ": ");
      if(x)
      {
        #define TRUNC(c) (c & 0x0F)
        char *y = strtok(NULL, " ");
        unsigned long c = strtol(y, NULL, 0);
        if(y)
        {
          char temp[128];
          BOOL found = FALSE;
          int k;
          for(k = 0; k < NOF_KEY_INDEX && !found; k++)
          {
            sprintf(temp, "%sText", KeyIndexStr[k]);
            if(!stricmp(temp, x))
            {
              aDocProp->m_color[k].m_textColor = c;
              found = TRUE;
            }
          }
          for(k = 0; k < NOF_KEY_INDEX && !found; k++)
          {
            sprintf(temp, "%sBackground", KeyIndexStr[k]);
            if(!stricmp(temp, x))
            {
              aDocProp->m_color[k].m_backColor = c;
              found = TRUE;
            }
          }
        }
      }
    }
    else if(state == -3)
    {
      char *x = strtok(line, ": ");
      if(x)
      {
        char *y = strtok(NULL, " ");
        if(y)
        {
          if(!stricmp(x, "Program"))
            aDocProp->m_tagProgram = y;
          else if(!stricmp(x, "Options"))
            aDocProp->m_tagOptions = y;
          else if(!stricmp(x, "File"))
            aDocProp->m_tagFile = y;
          else if(!stricmp(x, "UseTagColor"))
          {
            aDocProp->m_useTagColor = *y != '0' ? true : false;
          }
          else
            WainMessageBox(GetMf(), "Wrong keyword in ext file", IDC_MSG_OK, IDI_WARNING_ICO);
        }
      }
    }
    else if(state == -4)
    {
      if(line[0] == '!' && line[1] == '*' && line[2] == '!')
      { // A new template starts
        TemplateListClass *Template = new TemplateListClass;
        Template->m_name = &line[3];
        aDocProp->m_templateList.push_back(Template);
      }
      else
      {
        ASSERT(!aDocProp->m_templateList.empty());
        aDocProp->m_templateList[aDocProp->m_templateList.size() - 1]->m_expansion += line;
      }
    }
    else
    {
      end--;
      while(end > line && *end == ' ')
      {
        *end = '\0';
        end--;
      }
      if(*end != ' ' && strlen(line))
      {
        aDocProp->m_keyWordList[state].push_back(line);
      }
    }
  }
  fclose(f);
  if(Done)
  { // Only in case of an error
  }
  else
  {
    StringIgnoreCaseCompareClass StringIgnoreCaseCompare;
    for(i = 0; i < 5; i++)
    {
      if(aDocProp->m_ignoreCase)
        std::sort(aDocProp->m_keyWordList[i].begin(), aDocProp->m_keyWordList[i].end(), StringIgnoreCaseCompare);
      else
        std::sort(aDocProp->m_keyWordList[i].begin(), aDocProp->m_keyWordList[i].end());
    }
  }
  aDocProp->m_modified = false;
  return Done ? false : true;
}

void GlobalSettingsClass::WriteExtFile(const char *aFileName, DocPropClass *aDocProp)
{
  FILE *f = fopen(aDocProp->m_fileName.c_str(), "wt");
  if(!f)
    return;
  std::string temp;
  size_t i;
  fprintf(f, "[ExtensionFile]\n");
  fprintf(f, "EXTENSIONTYPE: %s\n", aDocProp->m_extensionType.c_str());
  fprintf(f, "EXTENSIONS: %s\n", aDocProp->m_extensions.c_str());
  ConvFromTab(temp, aDocProp->m_seps);
  fprintf(f, "SEPS: %s\n", temp.c_str());
  fprintf(f, "STRING: %s\n", aDocProp->m_stringDelim.c_str());
  fprintf(f, "CHAR: %s\n", aDocProp->m_chDelim.c_str());
  fprintf(f, "LITERAL: %s\n", aDocProp->m_literal.c_str());
  fprintf(f, "LINECONTINUATION: %s\n", aDocProp->m_lineCon.c_str());
  fprintf(f, "BRACES: %s\n", aDocProp->m_braces.c_str());
  fprintf(f, "BLOCKBEGIN: %s\n", aDocProp->m_blockBegin.c_str());
  fprintf(f, "BLOCKEND: %s\n", aDocProp->m_blockEnd.c_str());
  fprintf(f, "LINECOMMENT: %s\n", aDocProp->m_lineComment.c_str());
  fprintf(f, "COMMENTBEGIN: %s\n", aDocProp->m_commentBegin.c_str());
  fprintf(f, "COMMENTEND: %s\n", aDocProp->m_commentEnd.c_str());
  fprintf(f, "COMMENTWHOLEWORD: %d\n", aDocProp->m_commentWholeWord ? 1 : 0);
  fprintf(f, "COMMENTFIRSTWORD: %d\n", aDocProp->m_commentFirstWord ? 1 : 0);
  fprintf(f, "PREPROCESSOR: %s\n", aDocProp->m_preProcessor.c_str());
  fprintf(f, "BLOCKBEGINWORDS: %s\n", ConcatStringList(aDocProp->m_blockBeginWords, ';').c_str());
  fprintf(f, "BLOCKENDWORDS: %s\n", ConcatStringList(aDocProp->m_blockEndWords, ';').c_str());
  fprintf(f, "IGNORECASE: %d\n", aDocProp->m_ignoreCase ? 1 : 0);
  fprintf(f, "IndentAfter:");
  for(i = 0; i < aDocProp->m_indentAfter.size(); i++)
     fprintf(f, " %s", aDocProp->m_indentAfter[i].c_str());
  fprintf(f, "\n");
  fprintf(f, "IndentUnless:");
  for(i = 0; i < aDocProp->m_indentUnless.size(); i++)
     fprintf(f, " %s", aDocProp->m_indentUnless[i].c_str());
  fprintf(f, "\n");
  fprintf(f, "UnIndent:");
  for(i = 0; i < aDocProp->m_unindent.size(); i++)
     fprintf(f, " %s", aDocProp->m_unindent[i].c_str());
  fprintf(f, "\n");

  fprintf(f, "TabSize: %d\n", aDocProp->m_tabSize);
  fprintf(f, "Indent: %d\n", aDocProp->m_indent); /* Must be written AFTER tab_size, for compability */
  fprintf(f, "Numbers: %s\n", aDocProp->m_numberStr.c_str());
  temp = aDocProp->m_tabPos.ToString();
  fprintf(f, "TabPos: %s\n", temp.c_str());
  fprintf(f, "PreProcWords:");
  for(i = 0; i < aDocProp->m_preProcWord.size(); i++)
    fprintf(f, " %s", aDocProp->m_preProcWord[i].c_str());
  fprintf(f, "\n");
  fprintf(f, "[COLOR]\n");

  int k;

  for(k = 0; k < NOF_KEY_INDEX; k++)
  {
    fprintf(f, "%sText: 0x%06X\n", KeyIndexStr[k], aDocProp->m_color[k].m_textColor);
    fprintf(f, "%sBackGround: 0x%06X\n", KeyIndexStr[k], aDocProp->m_color[k].m_backColor);
  }
  fprintf(f, "[TAGS]\n");
  fprintf(f, "Program: %s\n", aDocProp->m_tagProgram.c_str());
  fprintf(f, "Options: %s\n", aDocProp->m_tagOptions.c_str());
  fprintf(f, "File: %s\n", aDocProp->m_tagFile.c_str());
  fprintf(f, "UseTagColor: %d\n", aDocProp->m_useTagColor ? 1 : 0);
  fprintf(f, "[Templates]\n");

  for(i = 0; i < aDocProp->m_templateList.size(); i++)
  {
    fprintf(f, "!*!%s\n", aDocProp->m_templateList[i]->m_name.c_str());
    fprintf(f, "%s\n", aDocProp->m_templateList[i]->m_expansion.c_str());
  }

  for(i = 0; i < 5; i++)
  {
    fprintf(f, "[Keyword %d]\n", i);
    for(size_t j = 0; j < aDocProp->m_keyWordList[i].size(); j++)
      fprintf(f, "%s\n", aDocProp->m_keyWordList[i][j].c_str());
  }
  aDocProp->m_modified = false;
  fclose(f);
}


void GlobalSettingsClass::ReadFont(LOGFONT *log_font, const char *font)
{
  memset(log_font, 0, sizeof(*log_font));

  int def_int = -32767;
  int size = (int )wainApp.GetProfileInt(font, "Size", def_int);
  if(size == def_int || size == 0)
    size = -12;
  log_font->lfHeight = size;

  CString def_str = "Test";
  CString text_item = wainApp.GetProfileString(font, "Face", def_str);
  if(text_item == def_str)
    text_item = _T("Courier New");
  lstrcpy(log_font->lfFaceName, text_item);

  size = (int )wainApp.GetProfileInt(font, "OutPrececision", def_int);
  if(size == def_int || size == 0)
    size = OUT_TT_PRECIS;
  log_font->lfOutPrecision = (unsigned char )size;

  size = (int )wainApp.GetProfileInt(font, "ClipPrececision", def_int);
  if(size == def_int || size == 0)
    size = CLIP_DEFAULT_PRECIS;
  log_font->lfClipPrecision = (unsigned char )size;

  size = (int )wainApp.GetProfileInt(font, "Quality", def_int);
  if(size == def_int || size == 0)
    size = PROOF_QUALITY;
  log_font->lfQuality = (unsigned char )size;

  size = (int )wainApp.GetProfileInt(font, "Pitch", def_int);
  if(size == def_int || size == 0)
    size = FF_MODERN;
  log_font->lfPitchAndFamily = (unsigned char )size;

  size = (int )wainApp.GetProfileInt(font, "Weight", def_int);
  if(size == def_int || size == 0)
    size = 0;
  log_font->lfWeight = size;

  size = (int )wainApp.GetProfileInt(font, "Italic", def_int);
  if(size == def_int || size == 0)
    size = 0;
  log_font->lfItalic = (unsigned char )size;
}

void GlobalSettingsClass::WriteFont(LOGFONT *aLogFont, const char *aFont)
{
  CString item = "Size";
  wainApp.WriteProfileInt(aFont, item, aLogFont->lfHeight);
  item = "Face";
  wainApp.WriteProfileString(aFont, item, aLogFont->lfFaceName);
  item = "OutPrececision";
  wainApp.WriteProfileInt(aFont, item, aLogFont->lfOutPrecision);
  item = "ClipPrececision";
  wainApp.WriteProfileInt(aFont, item, aLogFont->lfClipPrecision);
  item = "Quality";
  wainApp.WriteProfileInt(aFont, item, aLogFont->lfQuality);
  item = "Pitch";
  wainApp.WriteProfileInt(aFont, item, aLogFont->lfPitchAndFamily);
  item = "Weight";
  wainApp.WriteProfileInt(aFont, item, aLogFont->lfWeight);
  item = "Italic";
  wainApp.WriteProfileInt(aFont, item, aLogFont->lfItalic);
}


void GlobalSettingsClass::WriteProfile()
{
   wainApp.WriteProfileInt("Settings", "DispMode", int(m_createMode));
   wainApp.WriteProfileInt("Settings", "OneInst", m_oneInstance);
   wainApp.WriteProfileInt("Settings", "LoadFiles", m_loadFilesOnStartup);
   wainApp.WriteProfileInt("Settings", "StripWhitespace", m_stripWhitespace);
   wainApp.WriteProfileInt("Settings", "ViewBrowseBar", m_viewNavigatorBar);
   wainApp.WriteProfileInt("Settings", "ViewToolBar", m_viewToolBar);
   wainApp.WriteProfileInt("Settings", "ViewStatusBar", m_viewStatusBar);

   wainApp.WriteProfileString("Settings", "TempPath", m_tempPath.c_str());
   wainApp.WriteProfileString("Settings", "IncPath", m_incPath.c_str());
   wainApp.WriteProfileInt("Settings", "Tag Auto Rebuild Time", m_autoTagRebuildTime);
   wainApp.WriteProfileInt("Settings", "Cua Mouse Mraking", m_cuaMouseMarking);
   wainApp.WriteProfileInt("Settings", "Horz Scroll Bar", m_horzScrollBar);
   wainApp.WriteProfileInt("Settings", "Vert Scroll Bar", m_vertScrollBar);
   wainApp.WriteProfileInt("Settings", "FrameCurrLine", m_frameCurrLine);
   wainApp.WriteProfileInt("Settings", "Debug Tools", m_debugTools);
   wainApp.WriteProfileInt("Win Size", "Mode", m_winMode);
   wainApp.WriteProfileInt("Win Size", "X", m_winX);
   wainApp.WriteProfileInt("Win Size", "Y", m_winY);
   wainApp.WriteProfileInt("Win Size", "CX", m_winCx);
   wainApp.WriteProfileInt("Win Size", "CY", m_winCy);
   wainApp.WriteProfileInt("Browse Bar", "Width", m_navigatorBarWidth);
   wainApp.WriteProfileInt("Browse Bar", "Width Ratio", m_widthRatio);
   wainApp.WriteProfileInt("Browse Bar", "Short Names", m_navigatorBarShortNames);
   wainApp.WriteProfileInt("Settings", "PageBarHeight", m_pageBarHeight);

   WriteFont(&m_textWinLogFont, "Def font");
   WriteFont(&m_navigatorListLogFont, "Browse bar font");
   WriteFont(&m_printerFont, "PrinterFont");
   wainApp.WriteProfileString("Project", "Name", m_projectName);
   size_t k;
   CString tstr;

   for(k = 0; k < m_docProp.size(); k++)
   {
     if(m_docProp[k]->m_modified)
       WriteExtFile(m_docProp[k]->m_fileName.c_str(), m_docProp[k]);
   }
   std::list<std::string>::iterator idx;
   int i;
   for(idx = m_searchStringList.begin(), i = 0; idx != m_searchStringList.end() && i < MAX_NOF_SEARCH_STRINGS; idx++, i++)
   {
     tstr.Format("String %d", i);
     wainApp.WriteProfileString("Search", tstr, idx->c_str());
   }
   for(idx = m_replaceStringList.begin(), i = 0; idx != m_replaceStringList.end() && i < MAX_NOF_SEARCH_STRINGS; idx++, i++)
   {
     tstr.Format("String %d", i);
     wainApp.WriteProfileString("replace", tstr, idx->c_str());
   }

   for(i = 0; i < MAX_NOF_RECENT_PROJECTS; i++)
   {
     tstr.Format("File %d", i);
     wainApp.WriteProfileString("Recent Projects", tstr, m_recentProject[i]);
   }
   wainApp.WriteProfileString("Settings", "Last Dir", m_lastUsedDir);

   if(!m_popupMenu.empty())
   {
      std::stringstream ss;
      for(uint32_t z = 0; z < m_popupMenu.size(); z++)
        ss << " " << m_popupMenu[z].m_id;
      wainApp.WriteProfileString("PopupMenu", "Items", ss.str().c_str());
   }
   else
      wainApp.WriteProfileString("PopupMenu", "Items", "");

   if(!m_userMenu.empty())
   {
      std::stringstream ss;
      for(uint32_t z = 0; z < m_userMenu.size(); z++)
         ss << " " << m_userMenu[z].m_id;
      wainApp.WriteProfileString("UserMenu", "Items", ss.str().c_str());
   }
   else
      wainApp.WriteProfileString("UserMenu", "Items", "");

   wainApp.WriteProfileInt("Settings", "MaxUndoBufferSize", m_maxUndoBufferSize);
   wainApp.WriteProfileInt("Settings", "FlushUndoOnSave", m_flushUndoOnSave);
   wainApp.WriteProfileInt("Settings", "Sticky Mark", m_stickyMark);
   wainApp.WriteProfileInt("Settings", "Visible Tabs", m_visibleTabs);
   wainApp.WriteProfileInt("Settings", "Show Line No", m_showLineNo);

   for(size_t h = 0; h < m_docProp.size(); h++)
   {
      delete m_docProp[h];
   }
   m_docProp.clear();
   wainApp.WriteProfileInt("Settings", "Column Width 0", m_tagListColumnWidth[0]);
   wainApp.WriteProfileInt("Settings", "Column Width 1", m_tagListColumnWidth[1]);
   wainApp.WriteProfileInt("Settings", "Column Width 2", m_tagListColumnWidth[2]);
   wainApp.WriteProfileInt("Settings", "Column Width 3", m_tagListColumnWidth[3]);
   wainApp.WriteProfileInt("Settings", "TaglistCX", m_tagListCx);
   wainApp.WriteProfileInt("Settings", "TaglistCY", m_tagListCy);

   wainApp.WriteProfileInt("Settings", "WordlistCX", m_wordListCx);
   wainApp.WriteProfileInt("Settings", "WordlistCY", m_wordListCy);

   wainApp.WriteProfileInt("BrowseList", "ColorBack", m_listColorBack);
   wainApp.WriteProfileInt("BrowseList", "ColorText", m_listColorText);
   wainApp.WriteProfileInt("BrowseList", "ColorTextSel", m_listColorTextSel);
   wainApp.WriteProfileInt("BrowseList", "ColorSel",  m_listColorSel);
   wainApp.WriteProfileInt("BrowseList", "ColorSelFocus", m_listColorSelFocus);

   wainApp.WriteProfileString("FTP", "HostName", m_ftpHostName);
   wainApp.WriteProfileString("FTP", "UserName", m_ftpUserName);
   wainApp.WriteProfileInt("FTP", "SavePassWord", m_ftpSavePassword);
   if(m_ftpSavePassword)
   {
     cipher_password(m_ftpPassword);
     wainApp.WriteProfileString("FTP", "PassWord", m_ftpPassword);
   }
   else
     wainApp.WriteProfileString("FTP", "PassWord", "");

   wainApp.WriteProfileInt("Print", "Scale", m_printScale);
   wainApp.WriteProfileInt("Print", "Black&White", m_printColor);
   wainApp.WriteProfileString("Print", "Head", m_printHeader);
   wainApp.WriteProfileString("Print", "Foot", m_printFooter);
   wainApp.WriteProfileInt("Print", "LineNr", m_printLineNo);
   wainApp.WriteProfileInt("Print", "Margin", m_printMargin);
   wainApp.WriteProfileInt("Print", "Wrap", m_printWrap);

   wainApp.WriteProfileInt("UserMenu", "Display", m_useUserMenu);

   wainApp.WriteProfileInt("CleanUp", "Year", m_cleanupYear);
   wainApp.WriteProfileInt("CleanUp", "Month", m_cleanupMonth);
   wainApp.WriteProfileInt("CleanUp", "Day", m_cleanupDay);

   wainApp.WriteProfileString("Format", "Time", m_timeFormat.c_str());
   wainApp.WriteProfileString("Format", "Date", m_dateFormat.c_str());
   wainApp.WriteProfileString("Format", "TimeSep", m_timeSep.c_str());
   wainApp.WriteProfileString("Format", "UserId", m_userId.c_str());

   int ci;
   char col_str[256];
   for(ci = 0; ci < NOF_KEY_INDEX; ci++)
   {
     sprintf(col_str, "Color %d ForeG", ci);
     wainApp.WriteProfileInt("Print", col_str, m_printingColor[ci].m_textColor);
     sprintf(col_str, "Color %d backG", ci);
     wainApp.WriteProfileInt("Print", col_str, m_printingColor[ci].m_backColor);
   }
   wainApp.WriteProfileString("Settings", "ProjectExtension", m_projectExtension.c_str());

   wainApp.WriteProfileInt("TabView", "NormText",     m_tvcColor[TVC_NORMAL].m_textColor);
   wainApp.WriteProfileInt("TabView", "NormBack",     m_tvcColor[TVC_NORMAL].m_backColor);
   wainApp.WriteProfileInt("TabView", "ChangedText",  m_tvcColor[TVC_CHANGED].m_textColor);
   wainApp.WriteProfileInt("TabView", "ChangedBack",  m_tvcColor[TVC_CHANGED].m_backColor);
   wainApp.WriteProfileInt("TabView", "SavedText",    m_tvcColor[TVC_CHANGED_SAVED].m_textColor);
   wainApp.WriteProfileInt("TabView", "SavedBack",    m_tvcColor[TVC_CHANGED_SAVED].m_backColor);
   wainApp.WriteProfileInt("TabView", "BookmarkText", m_tvcColor[TVC_BOOKMARK].m_textColor);
   wainApp.WriteProfileInt("TabView", "BookmarkBack", m_tvcColor[TVC_BOOKMARK].m_backColor);
}

void GlobalSettingsClass::SetLastUsedDir(const char *name)
{
  char path[_MAX_PATH];
  MySplitPath(name, SP_DRIVE | SP_DIR, path);
  if(strlen(path) && path[strlen(path) - 1] != '\\')
    strcat(path, "\\");
  strcpy(m_lastUsedDir, path);
}

void ConvFromTab(std::string &aDst, const std::string &aSrc)
{
   std::string::size_type idx;
   aDst = "";
   for(idx = 0; idx < aSrc.size(); idx++)
   {
      if(aSrc[idx] == '\t')
         aDst += "\\t";
      else
         aDst += aSrc[idx];
   }
}

void ConvToTab(std::string &aStr)
{
   std::string::size_type idx;
   idx = aStr.find("\\t");
   if(idx != std::string::npos)
   {
       aStr[idx] = '\t';
       aStr.erase(idx + 1, 1);
   }
}

int GlobalSettingsClass::GetPropIndex(const char* _file, const char* _ext)
{
   if(*_ext == '.')
      _ext++;

   for(size_t i = 1; i < m_docProp.size(); i++)
   {
      StrSplitterClass Splitter(m_docProp[i]->m_extensions);
      std::string Part;
      while(Splitter.Get(Part, ";"))
      {
         std::string::size_type pos = Part.rfind('.');
         if(pos != std::string::npos)
         {
            if(!StrICmp(Part.substr(pos + 1), std::string(_ext)))
               return i;
         }
         if(!StrICmp(Part.substr(0, pos), std::string(_file)))
            return i;
      }
   }
   return 0;
}

const char *GlobalSettingsClass::FilterStrings(void)
//  Description:
//    Create a string to be used as filter in CFileDialog, based on the document types
//  Parameters:
//    Return the string.
{
   m_fileFilters = "";

   for(size_t i = 1; i < m_docProp.size(); i++)
   {
      m_fileFilters += m_docProp[i]->m_extensionType;
      m_fileFilters += " (";
      m_fileFilters += m_docProp[i]->m_extensions;
      m_fileFilters += ")";
      m_fileFilters.append(std::string::size_type(1), '\0');
      m_fileFilters += m_docProp[i]->m_extensions;
      m_fileFilters.append(std::string::size_type(1), '\0');
   }
   m_fileFilters += m_docProp[0]->m_extensionType;
   m_fileFilters += " (";
   m_fileFilters += m_docProp[0]->m_extensions;
   m_fileFilters += ")";
   m_fileFilters.append(std::string::size_type(1), '\0');
   m_fileFilters += m_docProp[0]->m_extensions;
   m_fileFilters.append(std::string::size_type(1), '\0');
   return m_fileFilters.c_str();
}

static void cipher_password(CString &word)
{
  char buf[256];
  strcpy(buf, word);
  int i;
  for(i = 0; buf[i]; i++)
  {
    if(i & 1)
      buf[i] = (char )(((buf[i] & 0xE0) >> 4) | ((buf[i] & 0x1E) << 3) | (buf[i] & 1));
    else
      buf[i] = (char )(((buf[i] & 0xC0) >> 5) | ((buf[i] & 0x3E) << 2) | (buf[i] & 1));
  }
  word = buf;
}

static void decipher_password(CString &word)
{
  char buf[256];
  strcpy(buf, word);
  int i;
  for(i = 0; buf[i]; i++)
  {
    if(i & 1)
      buf[i] = (char )(((buf[i] & 0x0E) << 4) | ((buf[i] & 0xF0) >> 3) | (buf[i] & 1));
    else
      buf[i] = (char )(((buf[i] & 0x06) << 5) | ((buf[i] & 0xF8) >> 2) | (buf[i] & 1));
  }
  word = buf;
}

std::string GlobalSettingsClass::CreateUserId()
{
   const char *ui = m_userId.c_str();
   if(wainApp.gs.m_userId[0] == '%' && wainApp.gs.m_userId[wainApp.gs.m_userId.size() - 1] == '%')
   {
      std::string t = wainApp.gs.m_userId.substr(1, wainApp.gs.m_userId.size() - 2);
      ui = getenv(t.c_str());
      if(ui == NULL)
         return "Unknown";
   }
   return ui;
}
