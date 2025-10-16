#ifndef GLOBSET_H_INC
#define GLOBSET_H_INC
#include "../src/ToolParm.h"
#define MAX_NOF_RECENT_PROJECTS 10

typedef std::vector<class DocPropClass*> DocPropListClass;

class GlobalSettingsClass
{
private:
  void ReadFont(LOGFONT *log_font, const char *font);
  void WriteFont(LOGFONT *log_font, const char *font);
  bool ReadExtFile(const char *file_name, class DocPropClass *dp);
  void WriteExtFile(const char *file_name, class DocPropClass *dp);
  std::string m_fileFilters;
public:
  const char *FilterStrings(void);
  GlobalSettingsClass();
  ~GlobalSettingsClass();
  bool ReadProfile();
  void WriteProfile();
  void Setup();
  void KeywordSetup(void);
  std::string m_tempPath;
  std::string m_incPath;
  CreateModeType m_createMode;
  bool m_loadFilesOnStartup;
  bool m_oneInstance;
  int m_viewNavigatorBar;
  bool m_viewStatusBar;
  bool m_viewToolBar;
  bool m_stripWhitespace;
  bool m_navigatorBarShortNames;
  int m_winMode;
  int m_winX;
  int m_winY;
  int m_winCx;
  int m_winCy;
  int m_navigatorBarWidth;
  int m_widthRatio;
  char m_projectName[_MAX_PATH];
  int  m_pageBarHeight;
  LOGFONT m_textWinLogFont;
  LOGFONT m_navigatorListLogFont;
  LOGFONT m_printerFont;
  PrintColorType m_printColor;
  int m_printScale;
  CString m_printHeader;
  CString m_printFooter;
  bool m_printLineNo;
  int  m_printMargin;
  bool m_printWrap;

  ConfigArray<ToolParm> m_toolParm;

  std::list<std::string> m_searchStringList;
  std::list<std::string> m_replaceStringList;

  bool m_cuaMouseMarking;
  char m_configPath[_MAX_PATH];

  int m_autoTagRebuildTime;

  char m_recentProject[MAX_NOF_RECENT_PROJECTS][_MAX_PATH];

  char m_lastUsedDir[_MAX_PATH];
  void SetLastUsedDir(const char *name);

  bool m_horzScrollBar;
  bool m_vertScrollBar;

  std::vector<MacroParmType> m_macroParmList;

  DocPropListClass m_docProp;

  int GetPropIndex(const char *aFile, const char *aExt);
  std::vector<PopupMenuListClass> m_popupMenu;
  std::vector<PopupMenuListClass> m_userMenu;

  int m_maxUndoBufferSize;
  bool m_flushUndoOnSave;
  bool m_stickyMark;
  bool m_visibleTabs;
  bool m_showLineNo;
  int m_tagListColumnWidth[4];
  int m_tagListCx;
  int m_tagListCy;

  int m_wordListCx;
  int m_wordListCy;
  bool m_frameCurrLine;
  bool m_debugTools;
  uint32_t m_listColorBack;
  uint32_t m_listColorText;
  uint32_t m_listColorSel;
  uint32_t m_listColorSelFocus;
  uint32_t m_listColorTextSel;

  CString m_ftpHostName;
  CString m_ftpUserName;
  CString m_ftpPassword;
  bool m_ftpSavePassword;
  bool m_useUserMenu;
  int  m_cleanupYear;
  int  m_cleanupMonth;
  int  m_cleanupDay;
  std::string m_timeFormat;
  std::string m_dateFormat;
  std::string m_timeSep;
  std::string m_userId;
  ColorType m_printingColor[NOF_KEY_INDEX];

  std::string m_projectExtension;

  enum TVCColorIndexType
  {
    TVC_NORMAL,
    TVC_CHANGED,
    TVC_CHANGED_SAVED,
    TVC_BOOKMARK,
    TVC_NUM_COLOR
  };
  ColorType m_tvcColor[TVC_NUM_COLOR];
  std::string CreateUserId();
};

#endif // GLOBSET_H_INC
