//=============================================================================
// This source code file is a part of Wain.
// It defines WainAppClass (the application class) and GlobalSettingsClass.
//=============================================================================
#ifndef WAIN_H_INC
#define WAIN_H_INC

#include ".\..\src\resource.h"
#ifndef __AFXWIN_H__
  #error include 'stdafx.h' before including this file for PCH
#endif
#include "Config.h"
#include ".\..\src\WainUtil.h"
#include ".\..\src\GlobSet.h"
#include ".\..\src\Tags.h"

#define MAX_LINE_LEN 256

#define MAX_NOF_TOOLS 20
#define MAX_NOF_SEARCH_STRINGS 10

#define ODF_NEW_FILE         2  /* Flags to be used with OpenDocument */
#define ODF_FTP_FILE         4
#define ODF_CMD_LINE_FILE    8

class WainApp : public CWinApp
{
  CMultiDocTemplate *m_docTemplate;
  RtvStatus ReplaceTagRest(std::string &aDest, const std::string &aFile);
  bool FirstInstance(const char *aFileName, int aStartLine);
  bool m_profileRead;
public:
  bool m_firstIdle;
  LPCTSTR m_uniqueClassName;  /* Used to store global class name */
  bool    m_classRegistered;

  WainApp();
  GlobalSettingsClass gs;
  int m_lineNo;
  int m_columnNo;
  int m_markStatus;

  std::string m_tagPeekStr;
  std::string m_lastTagStr;
  std::string m_tagStr;

  std::string m_lastAutoTagFile;
  class WainView *m_lastAutoTagView;
  bool m_openAutoTagFileAsNormalDoc;
  CString m_openAsNormalDoc;
public:
  bool GetProfileBool(LPCTSTR lpszSection, LPCTSTR lpszEntry, bool nDefault)
  {
     return GetProfileInt(lpszSection, lpszEntry, nDefault) ? true : false;
  }
  CFont m_guiFont;
  GetTagElemClass *m_tagPeekElem;
  void SetTagPeek(GetTagElemClass *elem);
  HCURSOR m_viewCursor;
  class WainDoc *OpenDocument(const char *file_name, unsigned int flags = 0);
  void OpenFile(void);
  virtual BOOL InitInstance();
  virtual int ExitInstance();

  RtvStatus ReplaceTagValues(std::string &aStr, const std::string &aCurrentFile);
  virtual BOOL OnIdle(LONG count);

  afx_msg void OnAppAbout();
  afx_msg void Setup(void);
  afx_msg void KeywordSetup(void);
  afx_msg void CloseFile(void);
  afx_msg void NewFile(void);
  afx_msg void Exit(void);
  afx_msg BOOL OnOpenRecentFile(UINT nID);
  DECLARE_MESSAGE_MAP();
};

extern WainApp wainApp;
extern uint64_t GetUSec();
extern uint64_t GetMSec();
#endif
