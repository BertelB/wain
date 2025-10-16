#ifndef _TOOL_PARM_H_INC
#define _TOOL_PARM_H_INC
#include "../src/Config.h"

class ToolParm : public Config
{
public:
   ToolParm()
   {
      Setup();
   }

   ToolParm& operator = (const ToolParm& _other)
   {
      CopyFrom(_other);
      return *this;
   }

   ToolParm(const ToolParm& _other)
   {
      CopyFrom(_other);
      Setup();
   }

   std::string m_menuText;
   std::string m_program;
   std::string m_arguments;
   std::string m_path;
   std::string m_lastRunPath;
   std::string m_regExp;
   bool m_dispModeMax;
   bool m_dispModeNorm;
   bool m_dispModeHidden;
   bool m_captureStdout;
   bool m_captureStderr;
   bool m_winProgram;
   bool m_dosProgram;
   bool m_askForArguments;
   bool m_saveDoc;
   bool m_append;

private:
   void Setup()
   {
      AddItem("MenuText", m_menuText);
      AddItem("Program", m_program);
      AddItem("Arguments", m_arguments);
      AddItem("Path", m_path);
      AddItem("LastRunPath", m_lastRunPath);
      AddItem("Regexp", m_regExp);
      AddItem("DispModeMax", m_dispModeMax);
      AddItem("DispModeNorm", m_dispModeNorm);
      AddItem("DispModeHidden", m_dispModeHidden);
      AddItem("CaptureStdout", m_captureStdout);
      AddItem("CaptureStderr", m_captureStderr);
      AddItem("WinProgram", m_winProgram);
      AddItem("DosProgram", m_dosProgram);
      AddItem("AskForArguments", m_askForArguments);
      AddItem("SaveDoc", m_saveDoc);
      AddItem("Append", m_append);
   }

   void CopyFrom(const ToolParm& _other)
   {
      m_menuText = _other.m_menuText;
      m_program = _other.m_program;
      m_arguments = _other.m_arguments;
      m_path = _other.m_path;
      m_lastRunPath = _other.m_lastRunPath;
      m_regExp = _other.m_regExp;
      m_dispModeMax = _other.m_dispModeMax;
      m_dispModeNorm = _other.m_dispModeNorm;
      m_dispModeHidden = _other.m_dispModeHidden;
      m_captureStdout = _other.m_captureStdout;
      m_captureStderr = _other.m_captureStderr;
      m_winProgram = _other.m_winProgram;
      m_dosProgram = _other.m_dosProgram;
      m_askForArguments = _other.m_askForArguments;
      m_saveDoc = _other.m_saveDoc;
      m_append = _other.m_append;
   }
};
#endif
