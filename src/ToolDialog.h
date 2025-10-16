#ifndef _TOOL_DIALOG_H_INC
#define _TOOL_DIALOG_H_INC
#include "../src/DialogBase.h"
#include "../src/ToolParm.h"
#include "../src/Resource.h"

class ToolSetupDialogClass : public DialogBaseClass
{
  DECLARE_DYNAMIC(ToolSetupDialogClass);
  void SetParm(void);
  int32_t m_currSel;
public:
  ToolParm m_toolParm;
  ToolSetupDialogClass(CWnd *parrent = NULL);
  virtual ~ToolSetupDialogClass(void);
  virtual BOOL OnInitDialog(void);
  enum {IDD = IDD_TOOL_SETUP};
  virtual void OnCancel();
  virtual void OnOK();
protected:
  void DoDataExchange(CDataExchange *dx);
  afx_msg void OnApply(void);
  afx_msg void OnNew(void);
  afx_msg void OnChange(void);
  afx_msg void OnFind(void);
  afx_msg void OnDelete(void);
  DECLARE_MESSAGE_MAP();
};

class ToolArgDialogClass : public CDialog
{
  DECLARE_DYNAMIC(ToolArgDialogClass);
public:
  ToolArgDialogClass(CWnd *parrent = 0);
  virtual BOOL OnInitDialog(void);
  CString m_prog;
  CString m_args;
  CString m_path;
  BOOL    m_askToAppend;
  BOOL    m_append;
  enum {IDD = IDD_TOOL_ARG_BOX};
protected:
  void DoDataExchange(CDataExchange *dx);
};

#endif
