#include ".\..\src\stdafx.h"
#include "Wain.h"
#include "wainview.h"
#include "mainfrm.h"
#include "NavigatorDlg.h"
#include "Project.h"
#include "childfrm.h"
#include "NavigatorList.h"
#include "waindoc.h"
#include "Tools.h"
#include "FileSelD.h"
#include "ProjectSetup.h"
#include ".\..\src\SimpleDialog.h"
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

ProjectMakeExecParamDialog::ProjectMakeExecParamDialog(CWnd *parent, ProjectExecParam& _param) :
   DialogBaseClass(ProjectMakeExecParamDialog::IDD, parent),
   m_param(_param)
{
}

ProjectMakeExecParamDialog::~ProjectMakeExecParamDialog()
{
}

BOOL ProjectMakeExecParamDialog::OnInitDialog(void)
{
   return DialogBaseClass::OnInitDialog();
}

void ProjectMakeExecParamDialog::OnOk(void)
{
   UpdateData(TRUE);
   EndDialog(IDOK);
}

IMPLEMENT_DYNAMIC(ProjectMakeExecParamDialog, CDialog)

BEGIN_MESSAGE_MAP(ProjectMakeExecParamDialog, CDialog)
  ON_BN_CLICKED(IDOK, OnOk)
  ON_BN_CLICKED(IDC_PROJ_EXE_PROG_BROWSE, ProgBrowse)
  ON_BN_CLICKED(IDC_PROJ_EXE_PATH_BROWSE, PathBrowse)
END_MESSAGE_MAP();

void ProjectListCtrl::OnLButtonDblClk(UINT /* flags */, CPoint /* point */)
{
   POSITION pos = GetFirstSelectedItemPosition();
   if (pos != NULL)
   {
      int nItem = GetNextSelectedItem(pos);
      ProjectMakeExecParamDialog dialog(this, m_param[nItem]);
      if(dialog.DoModal() == IDOK)
      {
         m_param[nItem] = dialog.GetParam();
         SetItemText(nItem, 0, m_param[nItem].m_prog.c_str());
         SetItemText(nItem, 1, m_param[nItem].m_args.c_str());
         SetItemText(nItem, 2, m_param[nItem].m_path.c_str());
         SetItemText(nItem, 3, m_param[nItem].m_name.c_str());
         SetItemText(nItem, 4, m_param[nItem].m_regexStr.c_str());
         SetItemText(nItem, 5, m_param[nItem].m_pathFind.c_str());
         SetItemText(nItem, 6, m_param[nItem].m_pathReplace.c_str());
      }
   }
}

void ProjectMakeExecParamDialog::ProgBrowse(void)
{
   MyFileDialogClass fd(FD_OPEN | FD_LAST_PATH | FD_SET_LAST_PATH, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY, NULL, NULL, "Executeables (*.exe, *.com, *.bat, *.cmd) \0*.exe;*.com;*.cmd;*.bat\0All files (*.*)\0*.*\0\0");
   if(fd.DoModal() == IDOK)
   {
      m_param.m_prog = fd.GetPathName();
      SetDlgItemText(IDC_PROJ_EXE_PROG, m_param.m_prog.c_str());
  }
}

void ProjectMakeExecParamDialog::PathBrowse(void)
{
   BROWSEINFO bi;
   char temp[_MAX_PATH];
   bi.hwndOwner = m_hWnd;
   bi.pidlRoot = NULL;
   bi.pszDisplayName = temp;
   bi.lpszTitle = "Select project execute path";
   bi.ulFlags = 0;
   bi.lpfn = NULL;
   bi.lParam = 0;
   bi.iImage = 0;
   LPITEMIDLIST item_list = SHBrowseForFolder(&bi);
   if(item_list)
   {
      SHGetPathFromIDList(item_list, temp);
      SetDlgItemText(IDC_PROJ_EXE_PATH, temp);
   }
}


void ProjectListCtrl::UpdateData()
{
   for (uint32_t i = 0; i < 4; i++)
      if (GetCheck(i))
         m_default = i;
}

BEGIN_MESSAGE_MAP(ProjectListCtrl, CListCtrl)
  ON_WM_LBUTTONDBLCLK()
  ON_WM_CREATE()
END_MESSAGE_MAP();

IMPLEMENT_DYNAMIC(ProjectListCtrl, CListCtrl);

void ProjectMakeExecParamDialog::DoDataExchange(CDataExchange *dx)
{
   DdxString(dx, IDC_PROJ_EXE_PROG, m_param.m_prog);
   DdxString(dx, IDC_PROJ_EXE_ARG,  m_param.m_args);
   DdxString(dx, IDC_PROJ_EXE_PATH, m_param.m_path);
   DdxString(dx, IDC_PROJ_EXE_LINE_REGEX, m_param.m_regexStr);
   DdxString(dx, IDC_PROJ_EXE_PATH_FIND, m_param.m_pathFind);
   DdxString(dx, IDC_PROJ_EXE_PATH_REPLACE, m_param.m_pathReplace);
   DdxString(dx, IDC_PROJ_EXE_NAME, m_param.m_name);
}

int ProjectListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   if (CListCtrl::OnCreate( lpCreateStruct) == -1)
   {
      return false;
   }
   std::string title = m_title + " Command";
   InsertColumn(0, title.c_str());
   SetColumnWidth(0, 300);
   InsertColumn(1, "Arguments");
   SetColumnWidth(1, 150);
   InsertColumn(2, "Path");
   SetColumnWidth(2, 280);
   InsertColumn(3, "Name");
   SetColumnWidth(3, 180);
   InsertColumn(4, "Line Regex");
   SetColumnWidth(4, 180);
   InsertColumn(5, "Line Find Regex");
   SetColumnWidth(5, 180);
   InsertColumn(6, "Line Replace Regex");
   SetColumnWidth(6, 180);

   for (uint32_t i = 0; i < 4; i++)
   {
      InsertItem(i, m_param[i].m_prog.c_str(), 0);
      SetItemText(i, 1, m_param[i].m_args.c_str());
      SetItemText(i, 2, m_param[i].m_path.c_str());
      SetItemText(i, 3, m_param[i].m_name.c_str());
      SetItemText(i, 4, m_param[i].m_regexStr.c_str());
      SetItemText(i, 5, m_param[i].m_pathFind.c_str());
      SetItemText(i, 6, m_param[i].m_pathReplace.c_str());
   }
   return 0;
}

/* Implementation of ProjectSetupDialogClass */
BEGIN_MESSAGE_MAP(ProjectSetupDialogClass, CDialog)
  ON_COMMAND(IDC_PS_BROWSE,           ProjBrowse)
  ON_BN_CLICKED(IDC_PS_INC_BROWSE,      IncPathBrowse)
  ON_BN_CLICKED(IDC_PROJ_TAGS,          TagSetup)
  ON_BN_CLICKED(IDOK,                   OnOk)
  ON_NOTIFY(LVN_ITEMCHANGING, IDC_PS_EXEC_GRID, OnItemChangingExec)
  ON_NOTIFY(LVN_ITEMCHANGING, IDC_PS_MAKE_GRID, OnItemChangingMake)
END_MESSAGE_MAP();

void ProjectSetupDialogClass::OnItemChangingExec(NMHDR* pNMHDR, LRESULT* pResult)
{
   *pResult = 0;
   if (m_inCheck)
   {
      return;
   }
   m_inCheck = true;
   LPNMLISTVIEW item = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
   int nItem = item->iItem;
   if (m_execList && (item->uOldState == 4096 || item->uNewState == 4096))
   {
      bool checked = m_execList->GetCheck(nItem);
      if (checked)
      {
         *pResult = true;
      }
      else
      {
         for (int i = 0; i < 4; i++)
         {
            m_execList->SetCheck(i, i == nItem);
         }
      }
   }
   m_inCheck = false;
}

void ProjectSetupDialogClass::OnItemChangingMake(NMHDR* pNMHDR, LRESULT* pResult)
{
   *pResult = 0;
   if (m_inCheck)
   {
      return;
   }
   m_inCheck = true;
   LPNMLISTVIEW item = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
   int nItem = item->iItem;
   if (m_makeList && (item->uOldState == 4096 || item->uNewState == 4096))
   {
      bool checked = m_makeList->GetCheck(nItem);
      if (checked)
      {
         *pResult = true;
      }
      else
      {
         for (int i = 0; i < 4; i++)
         {
            m_makeList->SetCheck(i, i == nItem);
         }
      }
   }
   m_inCheck = false;
}

IMPLEMENT_DYNAMIC(ProjectSetupDialogClass, CDialog)

ProjectSetupDialogClass::ProjectSetupDialogClass(CWnd *pParent ) : DialogBaseClass(ProjectSetupDialogClass::IDD, pParent)
{

}

ProjectSetupDialogClass::~ProjectSetupDialogClass()
{
   delete m_execList;
   delete m_makeList;
}

void ProjectSetupDialogClass::DoDataExchange(CDataExchange *dx)
{
  CDialog::DoDataExchange(dx);
  DdxString(dx, IDC_PS_FILE, m_projectOptions.m_makeFile);

  DdxString(dx, IDC_PS_INC_PATH, m_projectOptions.m_incPath);

  DdxCheck(dx, IDC_PS_MAKE_SD, m_projectOptions.m_makeSaveDoc);
  DdxCheck(dx, IDC_PS_MAKE_AA, m_projectOptions.m_makeAskArg);
  DdxCheck(dx, IDC_PS_MAKE_CSO, m_projectOptions.m_makeCaptureStrout);
  DdxCheck(dx, IDC_PS_MAKE_CSE, m_projectOptions.m_makeCaptureStderr);
  DdxCheck(dx, IDC_PS_MAKE_RUN_NORM, m_projectOptions.m_makeRunNorm);
  bool D = !m_projectOptions.m_makeRunNorm;
  DdxCheck(dx, IDC_PS_MAKE_RUN_HIDDEN, D);
  DdxCheck(dx, IDC_PS_MAKE_MD, m_projectOptions.m_makeRunDos);
  DdxCheck(dx, IDC_PS_MAKE_LOW, m_projectOptions.m_makeRunLow);

  DdxCheck(dx, IDC_PS_EXE_AA, m_projectOptions.m_exeAskArg);
  DdxCheck(dx, IDC_PS_EXE_CSO, m_projectOptions.m_exeCaptureStdout);
  DdxCheck(dx, IDC_PS_EXE_CSE, m_projectOptions.m_exeCaptureStderr);
  DdxCheck(dx, IDC_PS_EXE_MD, m_projectOptions.m_exeRunDos);
  DdxCheck(dx, IDC_PS_EXE_SAVE, m_projectOptions.m_exeSaveDoc);
  DdxCheck(dx, IDC_PS_EXE_LOW, m_projectOptions.m_exeRunLow);
  DdxCheck(dx, IDC_PS_EXE_RUN_NORM, m_projectOptions.m_exeRunNorm);
  D = !m_projectOptions.m_exeRunNorm;
  DdxCheck(dx, IDC_PS_EXE_RUN_HIDDEN, D);
}

bool ValidateRegex(CWnd* _parent, const std::string& _str)
{
   if (!_str.empty())
   {
      try
      {
         std::regex r(_str);
      }
      catch (const std::regex_error& e)
      {
         WainMessageBox( _parent, std::string("Regex error: ") + e.what() + " for " + _str, IDC_MSG_OK, IDI_ERROR_ICO);
         return false;
      }
   }
   return true;
}

void ProjectSetupDialogClass::OnOk(void)
{
   UpdateData(TRUE);
   if (m_execList)
   {
      m_execList->UpdateData();
      for (uint32_t i = 0; i < 4; i++)
      {
         m_projectOptions.m_execParam[i] = m_execList->m_param[i];
         if (!ValidateRegex(this, m_projectOptions.m_execParam[i].m_regexStr) ||
             !ValidateRegex(this, m_projectOptions.m_execParam[i].m_pathFind))
         {
            return;
         }
      }
      m_projectOptions.m_execDefault = m_execList->m_default;
   }
   if (m_makeList)
   {
      m_makeList->UpdateData();
      for (uint32_t i = 0; i < 4; i++)
      {
         m_projectOptions.m_makeParam[i] = m_makeList->m_param[i];
         if (!ValidateRegex(this, m_projectOptions.m_makeParam[i].m_regexStr) ||
             !ValidateRegex(this, m_projectOptions.m_makeParam[i].m_pathFind))
         {
            return;
         }
      }
      m_projectOptions.m_makeDefault = m_makeList->m_default;
   }
   EndDialog(IDOK);
}

void ProjectSetupDialogClass::TagSetup(void)
{
  GetMf()->m_navigatorDialog.GlobalTagSetup();
}

BOOL ProjectSetupDialogClass::OnInitDialog()
{
   CRect cr;
   cr.top = 60;
   cr.left = 10;
   cr.right = 1150;
   cr.bottom = 60 + 120;
   m_inCheck = true;

   m_makeList = new ProjectListCtrl(m_projectOptions.m_makeParam, m_projectOptions.m_makeDefault, "Make");
   m_makeList->Create(LBS_NOTIFY | LVS_REPORT | WS_VISIBLE | WS_CHILD | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_TABSTOP, cr, this, IDC_PS_MAKE_GRID);
   m_makeList->SetExtendedStyle(m_makeList->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES | LVS_EX_AUTOSIZECOLUMNS);
   m_makeList->ModifyStyleEx(0, WS_EX_CLIENTEDGE, FALSE);
   m_makeList->SetCheck(m_projectOptions.m_makeDefault, true);

   cr.top = 325;
   cr.bottom = 325 + 120;
   m_execList = new ProjectListCtrl(m_projectOptions.m_execParam, m_projectOptions.m_execDefault, "Exec");
   m_execList->Create(LBS_NOTIFY | LVS_REPORT | WS_VISIBLE | WS_CHILD | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_TABSTOP, cr, this, IDC_PS_EXEC_GRID);
   m_execList->SetExtendedStyle(m_execList->GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES | LVS_EX_AUTOSIZECOLUMNS);
   m_execList->ModifyStyleEx(0, WS_EX_CLIENTEDGE, FALSE);
   m_execList->SetCheck(m_projectOptions.m_execDefault, true);
   m_inCheck = false;

   CDialog::OnInitDialog();
   return FALSE;
}

void ProjectSetupDialogClass::ProjBrowse(void)
{
  MyFileDialogClass fd(FD_OPEN | FD_LAST_PATH | FD_SET_LAST_PATH, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY, "mak", NULL, "Make files (*.mak)\0*.mak\0All files (*.*)\0*.*\0\0");
  if(fd.DoModal() == IDOK)
  {
    m_projectOptions.m_makeFile = fd.GetPathName();
    CEdit *e = (CEdit *)GetDlgItem(IDC_PS_FILE);
    if(e)
      e->SetWindowText(m_projectOptions.m_makeFile.c_str());
  }
}

void ProjectSetupDialogClass::IncPathBrowse(void)
{
  BROWSEINFO bi;
  char temp[_MAX_PATH];
  bi.hwndOwner = m_hWnd;
  bi.pidlRoot = NULL;
  bi.pszDisplayName = temp;
  bi.lpszTitle = "Select project include path";
  bi.ulFlags = 0;
  bi.lpfn = NULL;
  bi.lParam = 0;
  bi.iImage = 0;
  LPITEMIDLIST item_list = SHBrowseForFolder(&bi);
  if(item_list)
  {
    SHGetPathFromIDList(item_list, temp);
    m_projectOptions.m_incPath = temp;
    SetDlgItemText(IDC_PS_INC_PATH, m_projectOptions.m_incPath.c_str());
  }
}
