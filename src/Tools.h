//=============================================================================
// This source code file is a part of wain.
// It defines one_tool_class, tool_class, tool_SetupDialogClass, and tool_arg_dialog_class
//=============================================================================
#ifndef TOOLS_H_INC
#define TOOLS_H_INC

class ProcessClass
{
   friend class ToolClass;
   PROCESS_INFORMATION m_pi;
   STARTUPINFO m_si;
public:
   HANDLE m_stdoutHandle;
   HANDLE m_stdinHandle;
   ProcessClass();
   ~ProcessClass()
   {
      if(m_stdoutHandle)
         CloseHandle(m_stdoutHandle);
      if(m_stdinHandle)
         CloseHandle(m_stdinHandle);
       if(m_pi.hProcess)
         CloseHandle(m_pi.hProcess);
       if(m_pi.hThread)
         CloseHandle(m_pi.hThread);
   }
   std::string m_title;
   std::string m_label;
   bool m_runDos;
   bool m_captureStderr;
   bool m_captureStdout;
   bool m_terminated;
   bool Run(void);
   std::string m_command;
   std::string m_outFile;
   std::string m_path;
   bool m_dispModeMax;
   bool m_dispModeHidden;
   bool m_dispModeNorm;
   bool m_append;
   bool Done(void);
   int m_msgId;
   int m_toolIndex = 0;
   bool m_runLow;
   bool m_isProject = false;
   bool m_isMake = false;
};

class ToolClass
{
  std::list<ProcessClass *>m_processList;
  CWnd *m_parrentFrame;
  bool m_inTimer;
  bool m_timerRun;

public:
  void ReadToolFile(void);
  void WriteToolFile(void);
  unsigned int m_timerId;
  void Activate(void);
  void DeActivate(void);
  ToolClass(CWnd *aParrent);

  ~ToolClass();
  ProcessClass *NewTool(void);
  void RemoveTool(ProcessClass *aTool);
  void OnTimer(void);
  void KillProcess(ProcessClass *aProcess);
  bool OnCloseFile(const char *File, bool aCanCancel);
  ProcessClass *GetProcess(int aMsgId);
private:
   unsigned int m_nextToolId;
};

#endif // TOOLS_H_INC


