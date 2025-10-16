#include ".\..\src\stdafx.h"
#include ".\..\src\ToolDialog.h"
#include ".\..\src\Wain.h"

#include <regex>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(ToolSetupDialogClass, CDialog)
  ON_BN_CLICKED(IDC_TOOLS_APPLY,      OnApply)
  ON_BN_CLICKED(IDC_TOOLS_NEW,        OnNew)
  ON_BN_CLICKED(IDC_TOOLS_FIND,       OnFind)
  ON_BN_CLICKED(IDC_TOOLS_DELETE,     OnDelete)
  ON_LBN_SELCHANGE(IDC_TOOLS_LIST,    OnChange)
END_MESSAGE_MAP();

IMPLEMENT_DYNAMIC(ToolSetupDialogClass, CDialog)

ToolSetupDialogClass::ToolSetupDialogClass(CWnd *parrent) : DialogBaseClass(ToolSetupDialogClass::IDD, parrent)
{
}

ToolSetupDialogClass::~ToolSetupDialogClass()
{
}

void ToolSetupDialogClass::OnOK(void)
{
  OnApply();
  EndDialog(TRUE);
}

void ToolSetupDialogClass::OnApply(void)
{
   if(m_currSel != -1)
   {
      UpdateData(TRUE);
      try
      {
         std::regex e(m_toolParm.m_regExp);
      }
      catch (const std::regex_error& e)
      {
         WainMessageBox(this, std::string("Regex error: ") + e.what(), IDC_MSG_OK, IDI_ERROR_ICO);
         return;
      }

      CListBox *lb = (CListBox *)GetDlgItem(IDC_TOOLS_LIST);
      if(m_currSel == int32_t(wainApp.gs.m_toolParm.Size()))
      {
         lb->AddString(m_toolParm.m_menuText.c_str());
         wainApp.gs.m_toolParm.Add(m_toolParm);
      }
      else
      {
         lb->DeleteString(m_currSel);
         lb->InsertString(m_currSel, m_toolParm.m_menuText.c_str());
         wainApp.gs.m_toolParm.Update(m_currSel, m_toolParm);
      }
      lb->SetCurSel(m_currSel);
   }
}

void ToolSetupDialogClass::OnCancel(void)
{
  EndDialog(FALSE);
}

void ToolSetupDialogClass::SetParm(void)
{
  UpdateData(FALSE);
}

void ToolSetupDialogClass::OnNew(void)
{
   m_currSel = wainApp.gs.m_toolParm.Size();
   m_toolParm.m_menuText = "";
   m_toolParm.m_program = "";
   m_toolParm.m_arguments = "";
   m_toolParm.m_path = "";
   m_toolParm.m_regExp = "";
   m_toolParm.m_dispModeNorm = TRUE;
   m_toolParm.m_dispModeMax  = FALSE;
   m_toolParm.m_dispModeHidden = FALSE;
   m_toolParm.m_captureStdout = TRUE;
   m_toolParm.m_captureStderr = TRUE;
   m_toolParm.m_winProgram = FALSE;
   m_toolParm.m_dosProgram = TRUE;
   m_toolParm.m_askForArguments = TRUE;
   m_toolParm.m_saveDoc = TRUE;
   m_toolParm.m_append = FALSE;
   SetParm();
}

void ToolSetupDialogClass::OnChange(void)
{
   CListBox *lb = (CListBox *)GetDlgItem(IDC_TOOLS_LIST);
   m_currSel = lb->GetCurSel();
   if(m_currSel == LB_ERR)
      m_currSel = -1;
   else
      m_toolParm = wainApp.gs.m_toolParm[m_currSel];
   SetParm();
}

void ToolSetupDialogClass::OnFind(void)
{
  MyFileDialogClass fd(FD_OPEN | FD_LAST_PATH | FD_SET_LAST_PATH, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER, NULL, NULL, "Executable (*.exe;*.com;*.cmd;*.bat)\0*.exe; *.com; *.cmd; *.bat\0All Files (*.*)\0*.*\0", this);
  if(fd.DoModal() == IDOK)
  {
    m_toolParm.m_program = fd.GetPathName();
    CEdit *e = (CEdit *)GetDlgItem(IDC_TOOLS_PROG);
    e->SetWindowText(m_toolParm.m_program.c_str());
  }
}

void ToolSetupDialogClass::OnDelete(void)
{
   if(m_currSel != -1 && m_currSel != int32_t(wainApp.gs.m_toolParm.Size()) && wainApp.gs.m_toolParm.Size() > 1)
   {
      wainApp.gs.m_toolParm.Remove(m_currSel);
      CListBox *lb = (CListBox *)GetDlgItem(IDC_TOOLS_LIST);
      lb->DeleteString(m_currSel);
      m_currSel = 0;
      m_toolParm = wainApp.gs.m_toolParm[m_currSel];
      lb->SetCurSel(m_currSel);
      SetParm();
   }
}

BOOL ToolSetupDialogClass::OnInitDialog(void)
{
   if(wainApp.gs.m_toolParm.Size())
   {
      m_currSel = 0;
      m_toolParm = wainApp.gs.m_toolParm[0];
      SetParm();
      size_t i;
      CListBox *lb = (CListBox *)GetDlgItem(IDC_TOOLS_LIST);
      for(i = 0; i < wainApp.gs.m_toolParm.Size(); i++)
         lb->AddString(wainApp.gs.m_toolParm[i].m_menuText.c_str());
      lb->SetCurSel(0);
      lb->SetFocus();
      return TRUE;
   }
   m_currSel = -1;
   SetParm();
   return FALSE;
}

void ToolSetupDialogClass::DoDataExchange(CDataExchange *dx)
{
   CDialog::DoDataExchange(dx);
   DdxString(dx, IDC_TOOLS_REGEX,          m_toolParm.m_regExp);
   DdxCheck(dx, IDC_TOOLS_SAVE,            m_toolParm.m_saveDoc);
   DdxCheck(dx, IDC_TOOLS_APPEND,          m_toolParm.m_append);
   DdxCheck(dx, IDC_TOOLS_ASK_ARG,         m_toolParm.m_askForArguments);
   DdxCheck(dx, IDC_TOOLS_DOS,             m_toolParm.m_dosProgram);
   DdxCheck(dx, IDC_TOOLS_WINDOWS,         m_toolParm.m_winProgram);
   DdxCheck(dx, IDC_TOOLS_RUN_NORM,        m_toolParm.m_dispModeNorm);
   DdxCheck(dx, IDC_TOOLS_RUN_MAX,         m_toolParm.m_dispModeMax);
   DdxCheck(dx, IDC_TOOLS_RUN_HIDDEN,      m_toolParm.m_dispModeHidden);
   DdxCheck(dx, IDC_TOOLS_STDOUT,          m_toolParm.m_captureStdout);
   DdxCheck(dx, IDC_TOOLS_STDERR,          m_toolParm.m_captureStderr);
   DdxString(dx,  IDC_TOOLS_PROG,            m_toolParm.m_program);
   DdxString(dx,  IDC_TOOLS_ARGS,            m_toolParm.m_arguments);
   DdxString(dx,  IDC_TOOLS_PATH,            m_toolParm.m_path);
   DdxString(dx,  IDC_TOOLS_MENU_TEXT,       m_toolParm.m_menuText);
}

IMPLEMENT_DYNAMIC(ToolArgDialogClass, CDialog)

ToolArgDialogClass::ToolArgDialogClass(CWnd *parrent) : CDialog(ToolArgDialogClass::IDD, parrent)
{
   m_append = FALSE;
   m_askToAppend = FALSE;
}

BOOL ToolArgDialogClass::OnInitDialog(void)
{
  CEdit *e;
  e = (CEdit *)GetDlgItem(IDC_TOOL_ARG_PATH);
  e->SetWindowText(m_path);
  e = (CEdit *)GetDlgItem(IDC_TOOL_ARG_ARG);
  e->SetWindowText(m_args);
  CStatic *s = (CStatic *)GetDlgItem(IDC_TOOL_ARG_PROG);
  CString pp;
  pp = "Program: ";
  pp += m_prog;
  s->SetWindowText(pp);
  CButton *b = (CButton *)GetDlgItem(IDC_TOOL_ARG_APPEND);
  ASSERT(b);
  if(!m_askToAppend)
  {
    b->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE | SWP_NOZORDER);
  }
  else
    b->SetCheck(m_append);
  e->SetFocus();
  return FALSE;
}

void ToolArgDialogClass::DoDataExchange(CDataExchange *dx)
{
  CDialog::DoDataExchange(dx);

  DDX_Text(dx, IDC_TOOL_ARG_PATH, m_path);
  DDX_Text(dx, IDC_TOOL_ARG_ARG,  m_args);
  if(m_askToAppend)
  {
    DDX_Check(dx, IDC_TOOL_ARG_APPEND, m_append);
  }
}
