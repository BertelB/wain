//=============================================================================
// This source code file is a part of Wain.
// It implements MacroList_entry_class and MacroList_class as defined in
// MainFrm.h, and some macro related functions from main_frame_class.
//=============================================================================
#include ".\..\src\stdafx.h"
#include <regex>
#include ".\..\src\Wain.h"
#include ".\..\src\WainDoc.h"
#include ".\..\src\WainView.h"
#include ".\..\src\MainFrm.h"
#include ".\..\src\TagList.h"
#include ".\..\src\Tools.h"
#include ".\..\src\KeySetup.h"
#include ".\..\src\childfrm.h"
#include ".\..\src\SimpleDialog.h"
#include ".\..\src\ToolDialog.h"
#include "project.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#pragma warning (disable : 4100) // unreferenced formal parameter

ToolClass::ToolClass(CWnd *aParrent)
{
  m_parrentFrame = aParrent;
  m_timerId = 0;

  m_inTimer = false;
  m_timerRun = false;
  m_nextToolId = 0;
}

ToolClass::~ToolClass()
{
  std::list<ProcessClass *>::iterator it;
  for(it = m_processList.begin(); it != m_processList.end(); ++it)
     delete *it;
}

ProcessClass *ToolClass::NewTool(void)
{
  ProcessClass *new_tool = new ProcessClass;

  m_processList.push_back(new_tool);
  if(!m_timerId)
  {
    m_timerId = SetTimer(m_parrentFrame->m_hWnd, ID_TOOL_TIMER, 2000, NULL);
    m_timerRun = TRUE;
  }
  return new_tool;
}

void ToolClass::RemoveTool(ProcessClass *tool)
{
  std::list<ProcessClass *>::iterator it = std::find(m_processList.begin(), m_processList.end(), tool);
  if(it != m_processList.end())
  {
     m_processList.erase(it);
     delete tool;
  }
  else
  {
     WainMessageBox(GetMf(), "Unknown tool!", IDC_MSG_OK, IDI_WARNING_ICO);
  }
  if(m_processList.size() == 0)
  {
    KillTimer(m_parrentFrame->m_hWnd, m_timerId);
    m_timerId = 0;
    m_timerRun = false;
  }
}

void ToolClass::Activate(void)
{
  if(m_timerRun)
  {
    m_timerId = SetTimer(m_parrentFrame->m_hWnd, ID_TOOL_TIMER, 2000, NULL);
    m_timerRun = false;
  }
}

void ToolClass::DeActivate(void)
{
}

void ToolClass::OnTimer(void)
{
   if(!m_inTimer)
   {
      m_inTimer = true;
      std::list<ProcessClass *>::iterator it;
      for(it = m_processList.begin(); it != m_processList.end(); )
      {
         if((*it)->Done())
         {
            ProcessClass *p = *it;
            GetMf()->OnToolTimer(p, true);
            m_processList.erase(it);
            delete p;
            it = m_processList.begin();
         }
         else
         {
            GetMf()->OnToolTimer(*it, false);
            ++it;
         }
      }
      if(m_processList.empty())
      {
         KillTimer(m_parrentFrame->m_hWnd, m_timerId);
         m_timerId = 0;
         m_timerRun = false;
      }
      m_inTimer = false;
   }
}

void ToolClass::KillProcess(ProcessClass *process)
{
  TerminateProcess(process->m_pi.hProcess, 0);
}

bool ToolClass::OnCloseFile(const char *aFile, bool aCanCancel)
{
   m_inTimer = true;

   std::list<ProcessClass *>::iterator it;
   for(it = m_processList.begin(); it != m_processList.end(); ++it)
   {
      if((*it)->m_outFile == aFile)
      {
         WORD flags = IDC_MSG_YES | IDC_MSG_NO;
         const char *Msg = "The tool is still running,\r\n do you want to kill the process?";
         bool st = true;
         if(aCanCancel)
            flags |= IDC_MSG_CANCEL;
         switch(WainMessageBox(GetMf(), Msg, flags, IDI_QUESTION_ICO))
         {
         case IDC_MSG_YES:
            KillProcess(*it);
         case IDC_MSG_NO:
            (*it)->m_captureStdout = false;
            (*it)->m_captureStderr = false;
            (*it)->m_terminated = true;
            break;
         case IDC_MSG_CANCEL:
            st = false;
            break;
         }
         if(st)
         {
            // ProcessList.erase(it);
            // delete *it;
         }
         m_inTimer = false;
         return st;
      }
   }
   m_inTimer = false;
   return true;
}

void ToolClass::ReadToolFile(void)
{
   std::string FileName = std::string(wainApp.gs.m_configPath) + "ToolNew.set";
   // std::ifstream In(FileName.c_str());
   wainApp.gs.m_toolParm.Read(FileName.c_str());
}

void ToolClass::WriteToolFile(void)
{
   std::string FileName = wainApp.gs.m_configPath;
   FileName += "ToolNew.set";
   wainApp.gs.m_toolParm.Write(FileName.c_str());
}


void MainFrame::OnToolTimer(ProcessClass* _process, bool _done)
{
   if(_done)
   {
      switch(_process->m_msgId)
      {
      case IDM_PROJ_MAKE_ID:
         m_navigatorDialog.m_project->MakeDone();
         break;
      case IDM_PROJ_EXE_ID:
         m_navigatorDialog.m_project->ExeDone();
         break;
      }
   }
   if((!_process->m_captureStderr && !_process->m_captureStdout) || !_process->m_runDos)
      return;

   if(_done)
   {
      SetStatusText("Tool completed");
      WainDoc *doc = OpenDebugFile(_process->m_outFile.c_str(), ToolDebugFile, false, _process->m_path, _process->m_isProject, _process->m_isMake, _process->m_toolIndex, _process->m_label.c_str());
      if(doc)
         doc->Done();
   }
   else
   {
      OpenDebugFile(_process->m_outFile.c_str(), ToolDebugFile, false, _process->m_path, _process->m_isProject, _process->m_isMake, _process->m_toolIndex, _process->m_label.c_str());
   }
}

void MainFrame::ToolSetup(void)
{
  ToolSetupDialogClass tsd(this);
  tsd.DoModal();
  m_tools->WriteToolFile();

  // The tool setup might have changed the order of Tools, so we have to load the keysetup again.
  DestroyAcceleratorTable(m_accHandle);
  m_nofAccEntrys = 0;
  ReadKeySetupFile(m_keyboardSetupFile, (ACCEL *)&m_accEntry, &m_nofAccEntrys);
  m_accHandle = CreateAcceleratorTable((ACCEL *)&m_accEntry, m_nofAccEntrys);
  SetupMenu(TRUE);
}

void MainFrame::RunTool(UINT id)
{
   size_t index = size_t(id - IDM_RUN_TOOL_0);
   std::string OutFile("");

   if(index >= wainApp.gs.m_toolParm.Size())
   {
      return;
   }

   if(m_tools->GetProcess(index))
   {
     WainMessageBox(this, "The tool is running, Please wait for it to finish", IDC_MSG_OK, IDI_INFORMATION_ICO);
     return;
   }

  ToolParm *tp = &wainApp.gs.m_toolParm[index];
  if(tp->m_saveDoc)
    SaveAllDocs();
  ChildFrame *active_child = (ChildFrame *)MDIGetActive();
  std::string tpath(tp->m_path);
  const char *p = active_child ? (const char *)active_child->GetDocument()->GetPathName() : "";
  RtvStatus error;
  if((error = wainApp.ReplaceTagValues(tpath, std::string(p))) != RtvStatus::rtv_no_error)
  {
    DisplayRtvError(this, "Unable to get path", error, true);
    return;
  }

  std::string targs(tp->m_arguments);
  if((error = wainApp.ReplaceTagValues(targs, std::string(p))) != RtvStatus::rtv_no_error)
  {
    DisplayRtvError(this, "Unable to parse arguments", error, true);
    return;
  }
  std::string command(tp->m_program);
  if((error = wainApp.ReplaceTagValues(command, std::string(p))) != RtvStatus::rtv_no_error)
  {
    DisplayRtvError(this, "Unable to build command", error, true);
    return;
  }
  BOOL has_asked_to_append = FALSE;
  BOOL do_append = FALSE;
  if(tp->m_askForArguments)
  {
    ToolArgDialogClass ta;
    ta.m_path = tpath.c_str();
    ta.m_prog = command.c_str();
    ta.m_args = targs.c_str();
    ta.m_askToAppend = tp->m_append;
    if(ta.DoModal() == IDOK)
    {
      has_asked_to_append = TRUE;
      targs = ta.m_args;
      if((error = wainApp.ReplaceTagValues(targs, std::string(p))) != RtvStatus::rtv_no_error)
      {
        DisplayRtvError(this, "Unable to parse arguments", error, true);
        return;
      }
      tpath = ta.m_path;
      if((error = wainApp.ReplaceTagValues(tpath, std::string(p))) != RtvStatus::rtv_no_error)
      {
        DisplayRtvError(this, "Unable to parse path", error, true);
        return;
      }
      do_append = ta.m_append;
    }
    else
      return;
  }
  if(tp->m_append && !has_asked_to_append)
  {
    if(WainMessageBox(this, "Do you want to append to output?", IDC_MSG_YES | IDC_MSG_NO, IDI_QUESTION_ICO) == IDC_MSG_YES)
      do_append = TRUE;
  }
  bool get_output = false;

  if(tp->m_dosProgram && (tp->m_captureStdout || tp->m_captureStderr))
  {
    OutFile = "$T$";
    OutFile += tp->m_menuText;
    OutFile += ".tmp";
    if((error = wainApp.ReplaceTagValues(OutFile, std::string(""))) != RtvStatus::rtv_no_error)
    {
      DisplayRtvError(this, "Unable to create outfile name", error, true);
      return;
    }
    else
    {
      RemoveFile(OutFile.c_str());
      get_output = true;
    }
  }

  if(!targs.empty())
  {
    command += " ";
    command += targs;
  }
  tp->m_lastRunPath = tpath.c_str();
  ProcessClass *tool = m_tools->NewTool();
  tool->m_title = "Wain: ";
  tool->m_title += command;
  tool->m_command = command;
  tool->m_path = tpath;
  tool->m_msgId = IDM_TOOL_MSG_ID;
  tool->m_toolIndex = index;
  tool->m_runDos = tp->m_dosProgram ? true : false;
  tool->m_dispModeHidden = tp->m_dispModeHidden ? true : false;
  tool->m_dispModeNorm   = tp->m_dispModeNorm ? true : false;
  tool->m_dispModeMax    = tp->m_dispModeMax ? true : false;

  if(get_output)
  {
    tool->m_outFile = OutFile;
  }

  tool->m_captureStdout = tp->m_captureStdout ? true : false;
  tool->m_captureStderr = tp->m_captureStderr ? true : false;
  tool->m_append = do_append ? true : false;

  if(!tool->Run() || (!tool->m_captureStdout && !tool->m_captureStderr))
  {
    m_tools->RemoveTool(tool);
  }
}

ProcessClass::ProcessClass()
{
   m_title = "";
   m_stdoutHandle = 0;
   m_stdinHandle = 0;
   m_runDos = true;
   m_captureStderr = m_captureStdout = false;
   m_command = "";
   m_outFile = "";
   m_path = "";
   m_dispModeNorm = true;
   m_dispModeHidden = m_dispModeMax = false;
   m_append = false;
   m_runLow = false;
   m_terminated = false;
   memset(&m_pi, 0, sizeof(m_pi));
}

bool ProcessClass::Run(void)
{
  m_si.cb = sizeof(m_si);
  m_si.lpReserved = NULL;
  m_si.lpDesktop = NULL;
  m_si.lpTitle = m_runDos ? (char *)m_title.c_str() : 0;
  m_si.dwX = 0;
  m_si.dwY = 0;
  m_si.dwXSize = 0;
  m_si.dwYSize = 0;
  m_si.dwXCountChars = 0;
  m_si.dwYCountChars = 0;
  m_si.dwFillAttribute = 0;
  m_si.dwFlags = m_runDos ? STARTF_USESHOWWINDOW : 0;

  if(m_dispModeHidden)
    m_si.wShowWindow = SW_HIDE;
  else if(m_dispModeMax)
    m_si.wShowWindow = SW_SHOWMAXIMIZED;
  else
    m_si.wShowWindow = SW_SHOWNORMAL;

  m_si.cbReserved2 = 0;
  m_si.lpReserved2 = NULL;
  m_si.hStdInput = NULL;
  if(m_captureStderr || m_captureStdout)
  {
    unsigned long wc;
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    m_stdoutHandle = CreateFile(m_outFile.c_str(),
                              GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              &sa,
                              m_append ? OPEN_ALWAYS : CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                              0);
    CString t;
    if(m_append)
    {
      SetFilePointer(m_stdoutHandle, 0, NULL, FILE_END);
      t = "\n-----------------------------------------------------------------------------------------------------------------------\n";
      WriteFile(m_stdoutHandle, t, t.GetLength(), &wc, NULL);
    }
    if(wainApp.gs.m_debugTools)
    {
      t = "Wain Tool output file - Command:\n";
      WriteFile(m_stdoutHandle, t, t.GetLength(), &wc, NULL);
      WriteFile(m_stdoutHandle, m_command.c_str(), m_command.size(), &wc, NULL);
      t = "\n-----------------------------------------------------------------------------------------------------------------------\n";
      WriteFile(m_stdoutHandle, t, t.GetLength(), &wc, NULL);
    }
    std::string InFile = m_outFile;
    InFile += ".in";

    m_stdinHandle = CreateFile(InFile.c_str(),
                             GENERIC_WRITE | GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             &sa,
                             CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                             0);
    #if 0
    StdinHandle = CreateNamedPipe(
                                   "\\\\.\\pipe\\mynamedpipe",             // pipe name
                                   PIPE_ACCESS_DUPLEX,       // read/write access
                                   PIPE_TYPE_BYTE |       // message type pipe
                                   PIPE_READMODE_BYTE |   // message-read mode
                                   PIPE_WAIT,                // blocking mode
                                   PIPE_UNLIMITED_INSTANCES, // max. instances
                                   4096,                  // output buffer size
                                   4096,                  // input buffer size
                                   NMPWAIT_WAIT_FOREVER, // client time-out
                                   NULL);                    // default security attribute
    #endif
    m_si.hStdOutput = m_captureStdout ? m_stdoutHandle : NULL;
    m_si.hStdError  = m_captureStderr ? m_stdoutHandle : NULL;
    m_si.hStdInput = m_stdinHandle;
    m_si.dwFlags |= STARTF_USESTDHANDLES;
  }
  else
  {
    m_si.hStdOutput = NULL;
    m_si.hStdError = NULL;
  }
  UINT CreateFlags = m_runDos ? CREATE_NEW_CONSOLE : 0;
  if(m_runLow)
     CreateFlags |= IDLE_PRIORITY_CLASS;
  BOOL is_Done = CreateProcess(NULL,
                               (char *)m_command.c_str(),
                               NULL,
                               NULL,
                               m_runDos ? TRUE : FALSE,
                               CreateFlags,
                               NULL,
                               m_path.empty() ? 0 : m_path.c_str(),
                               &m_si,
                               &m_pi);
   if(!is_Done)
   {
      std::string error_msg;
      char *msg_buf = 0;
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                    (char *)&msg_buf, 0, NULL);
      if(!msg_buf)
      {
         error_msg = "Error in running tool, the command line was:\r\n";
         error_msg += m_command;
         error_msg += "\r\nWindows reports the following error:\r\n";
         error_msg += "Unknown Windows error: ";
         error_msg += ToString(GetLastError());
         error_msg += "\r\n";
         WainMessageBox(GetMf(), error_msg.c_str(), IDC_MSG_OK, IDI_WARNING_ICO);
      }
      else
      {
         error_msg = "Error in running tool, the command line was:\r\n";
         error_msg += m_command;
         error_msg += "\r\nWindows reports the following error:\r\n";
         error_msg += msg_buf;
         WainMessageBox(GetMf(), error_msg.c_str(), IDC_MSG_OK, IDI_WARNING_ICO);
         LocalFree(msg_buf);
      }
   }
   else
   {
      SetStatusText(m_command.c_str());
   }

   if(is_Done)
      return TRUE;
   return FALSE;
}

bool ProcessClass::Done(void)
{
   if(m_terminated)
   {
      return true;
   }
   DWORD ExitCode;
   GetExitCodeProcess(m_pi.hProcess, &ExitCode);
   bool IsDone = ExitCode == STILL_ACTIVE ? false : true;
   if(IsDone && m_runDos && m_stdoutHandle)
   {
      if(wainApp.gs.m_debugTools)
      {
         unsigned long wc;
         CString t;
         t = "\n-----------------------------------------------------------------------------------------------------------------------\n";
         WriteFile(m_stdoutHandle, t, t.GetLength(), &wc, NULL);
         t.Format("The process terminated with exit code: %lu", (unsigned long)ExitCode);
         WriteFile(m_stdoutHandle, t, t.GetLength(), &wc, NULL);
      }
      CloseHandle(m_stdoutHandle);
      m_stdoutHandle = 0;
      if(m_stdinHandle)
      {
         CloseHandle(m_stdinHandle);
         m_stdinHandle = 0;
      }
   }
   return IsDone;
}

ProcessClass *ToolClass::GetProcess(int aIndex)
{
   std::list<ProcessClass *>::iterator it;
   for(it = m_processList.begin(); it != m_processList.end(); ++it)
      if((*it)->m_toolIndex == aIndex)
         return *it;

   return 0;
}

void MainFrame::SendToolCmd()
{
   WainView *av = m_pageBarDialog.GetActiveView();
   if(!av)
   {
      SetStatusText("No Active View");
      return;
   }
   std::string FileName;
   MySplitPath(av->GetDocument()->GetPathName(), SP_FILE, FileName);
   int i, ToolNo = -1;
   for(i = 0; i < wainApp.gs.m_toolParm.Size() && ToolNo == -1; i++)
   {
      if(wainApp.gs.m_toolParm[i].m_menuText == FileName)
         ToolNo = i;
   }
   if(ToolNo == -1)
   {
      SetStatusText("Tool Not Found");
      return;
   }
   ProcessClass *Process = m_tools->GetProcess(ToolNo);
   if(!Process)
   {
      SetStatusText("Failed to find process");
      return;
   }
   if(!Process->m_stdinHandle)
   {
      SetStatusText("No input handle");
      return;
   }
   SimpleDialog Dialog("Tool Cmd", "", this);
   if(Dialog.DoModal() == IDOK)
   {
      std::string Msg = Dialog.m_msg;
      Msg += "\r\n";
      DWORD Dummy;
      WriteFile(Process->m_stdinHandle, Msg.c_str(), Msg.size(), &Dummy, 0);
   }
}