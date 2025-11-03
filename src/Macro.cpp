//=============================================================================
// This source code file is a part of Wain.
// It implements MacroListEntryClass and MacroList_class as defined in
// MainFrm.h, and some macro related functions from main_frame_class.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\Wain.h"

// #include ".\..\src\WainDoc.h"
#include ".\..\src\WainView.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\KeySetup.h"
#include ".\..\src\childfrm.h"
#include "dialogbase.h"
#include <iterator>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

MacroListEntryClass::MacroListEntryClass()
{
  m_info.m_code = 0;
  m_info.m_functionName = NULL;
  m_info.m_info = 0;
  m_next = m_prev = this;
}

MacroListEntryClass::~MacroListEntryClass()
{
  if(m_info.m_functionName)
    free(m_info.m_functionName);
}

MacroListClass::MacroListClass()
{
   m_recording = FALSE;
}

MacroListClass::~MacroListClass()
{
  CleanUp();
}

void MacroListClass::CleanUp(void)
{
  MacroListEntryClass *elem;
  while(m_list.m_next != &m_list)
  {
    elem = m_list.m_next;
    m_list.m_next->m_prev = &m_list;
    m_list.m_next = m_list.m_next->m_next;
    delete elem;
  }
  m_recording = FALSE;
  m_list.m_next = m_list.m_prev = &m_list;
}

void MainFrame::AddMacroEntry(int code, int add_info)
{
  MacroListEntryClass *elem = new MacroListEntryClass;
  const char *name = MsgId2FuncName((WORD )code);
  if(!name)
  {
    if(code == ID_PUT_CHAR)
    {
      name = "PutChar";
    }
    else if(code == IDV_HANDLE_ESC)
    {
      name = "Esc";
    }
    else
    {
      WainMessageBox(this, "Wrong macro entry", IDC_MSG_OK, IDI_WARNING_ICO);
      return;
    }
  }
  elem->m_next = &m_macroList.m_list;
  elem->m_prev = m_macroList.m_list.m_prev;
  m_macroList.m_list.m_prev->m_next = elem;
  m_macroList.m_list.m_prev = elem;
  elem->m_info.m_code = code;
  elem->m_info.m_functionName = strdup(name);
  elem->m_info.m_info = add_info;
}

BOOL MainFrame::MacroRecordToggle(void)
{
  if(m_macroList.m_recording)
  {
    m_macroList.m_recording = FALSE;
  }
  else
  {
    m_macroList.CleanUp();
    m_macroList.m_recording = TRUE;
  }
  SetMacroRecStatus(m_macroList.m_recording);
  return m_macroList.m_recording;
}

BOOL MainFrame::MacroRecordingOn(void)
{
  return m_macroList.m_recording;
}

BOOL MainFrame::GetMacroEntry(MacroEntryInfoType *info, BOOL first)
{
  if(m_macroList.m_recording)
  {
    SetStatusText("Can't playback a macro while recording");
    return FALSE;
  }
  if(first)
  {
    m_macroList.m_current = m_macroList.m_list.m_next;
  }
  if(m_macroList.m_current == &m_macroList.m_list)
    return FALSE;
  *info = m_macroList.m_current->m_info;
  m_macroList.m_current = m_macroList.m_current->m_next;

  return TRUE;
}

void MainFrame::SaveMacro(void)
{
  if(m_macroList.m_list.m_next == &m_macroList.m_list)
  {
    SetStatusText("No macro to save");
    return;
  }
  if(m_macroList.m_recording)
  {
    SetStatusText("Can't Save a macro while recording");
    return;
  }

  MyFileDialogClass fd(FD_SAVE | FD_CONFIG_PATH, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "wm", NULL, "Wain macro (*.wm)\0*.wm\0All Files (*.*)\0*.*\0\0", this);
  if(fd.DoModal() != IDOK)
    return;
  CString file_name = fd.GetPathName();
  FILE *f = fopen(file_name, "wt");
  if(!f)
  {
    CString msg = "Unable to open:\r\n";
    msg += file_name;
    WainMessageBox(this, msg, IDC_MSG_OK, IDI_WARNING_ICO);
    return;
  }
  fprintf(f, "void wain_macro(void)\n{\n");
  MacroListEntryClass *elem;
  for(elem = m_macroList.m_list.m_next; elem != &m_macroList.m_list; elem = elem->m_next)
  {
    if(elem->m_info.m_code == ID_PUT_CHAR)
    {
      fprintf(f, "  PutChar('%c')\n", (char )elem->m_info.m_info);
    }
    else if(elem->m_info.m_code == IDV_HANDLE_ESC)
    {
      fprintf(f, "  Esc()\n");
    }
    else
    {
      fprintf(f, "  %s()\n", elem->m_info.m_functionName);
    }
  }
  fprintf(f, "}");
  fclose(f);
}

void MainFrame::LoadMacro(void)
{
  if(m_macroList.m_recording)
  {
    SetStatusText("Can't load a macro while recording");
    return;
  }
  MyFileDialogClass fd(FD_OPEN | FD_CONFIG_PATH, OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, "wm", NULL, "Wain macro (*.wm)\0*.wm\0All Files (*.*)\0*.*\0\0", this);
  if(fd.DoModal() != IDOK)
    return;
  DoLoadMacro(fd.GetPathName());
}

BOOL MainFrame::DoLoadMacro(const char *file_name)
{
  FILE *f = fopen(file_name, "rt");
  if(!f)
  {
    CString msg = "Unable to read:\r\n";
    msg += file_name;
    WainMessageBox(this, msg, IDC_MSG_OK, IDI_WARNING_ICO);
    return FALSE;
  }
  char line[1024];
  BOOL end;
  char *s;
  for(end = FALSE; !end && fgets(line, 1024, f); )
  {
    for(s = line; *s == ' ' || *s == '\t'; s++);
    if(*s == '{')
      end = TRUE;
  }

  m_macroList.CleanUp();
  int func;
  for(end = FALSE; fgets(line, 1024, f) && !end; )
  {
    for(s = line; *s == ' ' || *s == '\t'; s++);
    if(*s == '}')
      end = TRUE;
    s = strtok(line, " (;");
    if(s && !end)
    {
      func = FuncName2MsgId(s);
      if(func == 0)
      {
        if(!strcmp(s, "PutChar"))
        {
          func = ID_PUT_CHAR;
          char *c = strtok(NULL, "'");
          if(!c)
          {
            WainMessageBox(this, "Char not found", IDC_MSG_OK, IDI_WARNING_ICO);
            fclose(f);
            return FALSE;
          }
          AddMacroEntry(func, *c);
        }
        else if(!strcmp(s, "Esc"))
        {
          func = IDV_HANDLE_ESC;
          AddMacroEntry(func);
        }
        else
        {
          CString msg = "Wrong function name:\r\n";
          msg += s;
          WainMessageBox(this, msg, IDC_MSG_OK, IDI_WARNING_ICO);
          fclose(f);
          return FALSE;
        }
      }
      else
        AddMacroEntry(func);
    }
  }
  fclose(f);
  return TRUE;
}

class MacroSetupDialogClass : public CDialog
{
  DECLARE_DYNAMIC(MacroSetupDialogClass);
public:
  int m_current;
  std::vector<MacroParmType >m_macroParmList;

  MacroSetupDialogClass(CWnd *aParrent = 0);
  virtual ~MacroSetupDialogClass(void);
  virtual BOOL OnInitDialog(void);
  enum {IDD = IDD_MACRO_SETUP};
  std::string m_currFileName;
  void SetupList();
protected:
  void DoDataExchange(CDataExchange *aDx);
  afx_msg void Find();
  afx_msg void Apply();
  afx_msg void ListChanged();
  afx_msg void NewMacro(void);
  afx_msg void Remove(void);
  DECLARE_MESSAGE_MAP();
};

BEGIN_MESSAGE_MAP(MacroSetupDialogClass, CDialog)
  ON_BN_CLICKED(IDC_MACRO_FIND,  Find)
  ON_BN_CLICKED(IDC_MACRO_APPLY, Apply)
  ON_BN_CLICKED(IDC_MACRO_NEW,   NewMacro)
  ON_LBN_SELCHANGE(IDC_MACRO_MENUTEXT, ListChanged)
  ON_BN_CLICKED(IDC_MACRO_REMOVE, Remove)
END_MESSAGE_MAP();

IMPLEMENT_DYNAMIC(MacroSetupDialogClass, CDialog)

MacroSetupDialogClass::MacroSetupDialogClass(CWnd *aParrent) : CDialog(MacroSetupDialogClass::IDD, aParrent)
{
  m_current = -1;
}

MacroSetupDialogClass::~MacroSetupDialogClass()
{

}

BOOL MacroSetupDialogClass::OnInitDialog(void)
{
  SetupList();
  return TRUE;
}

void MacroSetupDialogClass::SetupList(void)
{
  CListBox *lb = (CListBox *)GetDlgItem(IDC_MACRO_MENUTEXT);
  ASSERT(lb);
  lb->ResetContent();
  size_t i;
  for(i = 0; i < m_macroParmList.size(); i++)
    lb->AddString(m_macroParmList[i].m_menuText.c_str());
}

void MacroSetupDialogClass::DoDataExchange(CDataExchange *aDx)
{
  CDialog::DoDataExchange(aDx);
}

void MacroSetupDialogClass::Find(void)
{
  MyFileDialogClass fd(FD_OPEN | FD_CONFIG_PATH, OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, "wm", NULL, "Wain macro (*.wm)\0*.wm\0All Files (*.*)\0*.*\0\0", this);
  if(fd.DoModal() == IDOK)
  {
    char name[MAX_PATH];
    MySplitPath(fd.GetPathName(), SP_FILE | SP_EXT, name);
    CEdit *e = (CEdit *)GetDlgItem(IDC_MACRO_FILENAME_EDIT);
    ASSERT(e);
    e->SetWindowText(name);
    m_currFileName = (const char *)fd.GetPathName();
  }
}

void MacroSetupDialogClass::Apply(void)
{
  if(m_current != -1 && !m_currFileName.empty())
  {
    CEdit *e = (CEdit *)GetDlgItem(IDC_MACRO_MENUTEXT_EDIT);
    ASSERT(e);
    GetWindowString(e, m_macroParmList[m_current].m_menuText);
    if(m_macroParmList[m_current].m_menuText.empty())
      return;
    m_macroParmList[m_current].m_fileName = m_currFileName;
    m_currFileName = "";
    m_current = -1;
    e->SetWindowText("");
    e = (CEdit *)GetDlgItem(IDC_MACRO_FILENAME_EDIT);
    ASSERT(e);
    e->SetWindowText("");
    SetupList();
  }
}

void MacroSetupDialogClass::ListChanged(void)
{
  CListBox *eb = (CListBox *)GetDlgItem(IDC_MACRO_MENUTEXT);
  ASSERT(eb);
  size_t sel = eb->GetCurSel();
  if(sel != LB_ERR && sel < m_macroParmList.size())
  {
    m_current = sel;
    m_currFileName = m_macroParmList[m_current].m_fileName;
    CEdit *e = (CEdit *)GetDlgItem(IDC_MACRO_MENUTEXT_EDIT);
    ASSERT(e);
    e->SetWindowText(m_macroParmList[m_current].m_menuText.c_str());
    e = (CEdit *)GetDlgItem(IDC_MACRO_FILENAME_EDIT);
    ASSERT(e);
    std::string name;
    MySplitPath(m_currFileName.c_str(), SP_FILE | SP_EXT, name);
    e->SetWindowText(name.c_str());
  }
}

void MacroSetupDialogClass::NewMacro(void)
{
   if(m_macroParmList.size() < MAX_NUM_MACRO - 1)
   {
      CEdit *e = (CEdit *)GetDlgItem(IDC_MACRO_MENUTEXT_EDIT);
      ASSERT(e);
      e->SetWindowText("");
      e = (CEdit *)GetDlgItem(IDC_MACRO_FILENAME_EDIT);
      ASSERT(e);
      e->SetWindowText("");
      MacroParmType T;
      m_macroParmList.push_back(T);
      m_current = m_macroParmList.size() - 1;
      m_currFileName = "";
   }
}

void MacroSetupDialogClass::Remove(void)
{
   if(m_current != -1)
   {
      m_macroParmList.erase(m_macroParmList.begin() + m_current);
      m_current = -1;
      SetupList();
   }
}

void MainFrame::MacroSetup(void)
{
   MacroSetupDialogClass ms;
   std::copy(wainApp.gs.m_macroParmList.begin(), wainApp.gs.m_macroParmList.end(), std::back_inserter(ms.m_macroParmList));
   if(ms.DoModal() == IDOK)
   {
      wainApp.gs.m_macroParmList.clear();
      std::copy(ms.m_macroParmList.begin(), ms.m_macroParmList.end(), std::back_inserter(wainApp.gs.m_macroParmList));

      std::string fn(wainApp.gs.m_configPath);
      fn += "macro.set";
      std::ofstream File;
      File.open(fn.c_str());
      if(File.is_open())
      {
         for(size_t i = 0; i < wainApp.gs.m_macroParmList.size(); i++)
            File << "\"" << wainApp.gs.m_macroParmList[i].m_fileName << "\" " << wainApp.gs.m_macroParmList[i].m_menuText << std::endl;
      }
      else
         SetStatusText("Unable to write macro setup");
      // The macro setup might have changed the order of macros, so we have to load the keysetup again.
      DestroyAcceleratorTable(m_accHandle);
      m_nofAccEntrys = 0;
      ReadKeySetupFile(m_keyboardSetupFile, (ACCEL *)&m_accEntry, &m_nofAccEntrys);
      m_accHandle = CreateAcceleratorTable((ACCEL *)&m_accEntry, m_nofAccEntrys);

      SetupMenu(TRUE);
   }
}

void MainFrame::RunMacro(UINT id)
{
  size_t my_id = id;
  my_id -= IDM_RUN_MACRO_0;
  if(m_macroList.m_recording)
  {
    SetStatusText("Can't run a macro while recording");
    return;
  }
  if(my_id >= 0 && my_id < wainApp.gs.m_macroParmList.size())
  {
    if(DoLoadMacro(wainApp.gs.m_macroParmList[my_id].m_fileName.c_str()))
    {
      ChildFrame *cf = (ChildFrame *)MDIGetActive();
      if(cf)
        cf->GetView()->PlaybackMacro();
    }
  }
}

//--- EOF
