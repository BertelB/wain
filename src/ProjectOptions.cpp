#include ".\..\src\stdafx.h"
#include "../src/ProjectOptions.h"
#include "DialogBase.h"
#include "MainFrm.h"
#include "wain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

ProjectOptions::ProjectOptions()
{
   Clear();
   m_helper.Add("File", m_makeFile);
   m_helper.Add("Make1", m_makeParam[0].m_prog);
   m_helper.Add("Make2", m_makeParam[1].m_prog);
   m_helper.Add("Make3", m_makeParam[2].m_prog);
   m_helper.Add("Make4", m_makeParam[3].m_prog);
   m_helper.Add("MakeArg1", m_makeParam[0].m_args);
   m_helper.Add("MakeArg2", m_makeParam[1].m_args);
   m_helper.Add("MakeArg3", m_makeParam[2].m_args);
   m_helper.Add("MakeArg4", m_makeParam[3].m_args);
   m_helper.Add("MakeDefault", m_makeDefault);
   m_helper.Add("MakePath1", m_makeParam[0].m_path);
   m_helper.Add("MakePath2", m_makeParam[1].m_path);
   m_helper.Add("MakePath3", m_makeParam[2].m_path);
   m_helper.Add("MakePath4", m_makeParam[3].m_path);
   m_helper.Add("MakeRegex1", m_makeParam[0].m_regexStr);
   m_helper.Add("MakeRegex2", m_makeParam[1].m_regexStr);
   m_helper.Add("MakeRegex3", m_makeParam[2].m_regexStr);
   m_helper.Add("MakeRegex4", m_makeParam[3].m_regexStr);
   m_helper.Add("MakePathFind1", m_makeParam[0].m_pathFind);
   m_helper.Add("MakePathFind2", m_makeParam[1].m_pathFind);
   m_helper.Add("MakePathFind3", m_makeParam[2].m_pathFind);
   m_helper.Add("MakePathFind4", m_makeParam[3].m_pathFind);
   m_helper.Add("MakePathReplace1", m_makeParam[0].m_pathReplace);
   m_helper.Add("MakePathReplace2", m_makeParam[1].m_pathReplace);
   m_helper.Add("MakePathReplace3", m_makeParam[2].m_pathReplace);
   m_helper.Add("MakePathReplace4", m_makeParam[3].m_pathReplace);
   m_helper.Add("MakeName1", m_makeParam[0].m_name);
   m_helper.Add("MakeName2", m_makeParam[1].m_name);
   m_helper.Add("MakeName3", m_makeParam[2].m_name);
   m_helper.Add("MakeName4", m_makeParam[3].m_name);

   m_helper.Add("Exe1",  m_execParam[0].m_prog);
   m_helper.Add("Exe2",  m_execParam[1].m_prog);
   m_helper.Add("Exe3",  m_execParam[2].m_prog);
   m_helper.Add("Exe4",  m_execParam[3].m_prog);
   m_helper.Add("ExeArg1",  m_execParam[0].m_args);
   m_helper.Add("ExeArg2",  m_execParam[1].m_args);
   m_helper.Add("ExeArg3",  m_execParam[2].m_args);
   m_helper.Add("ExeArg4",  m_execParam[3].m_args);
   m_helper.Add("ExeDefault", m_execDefault);
   m_helper.Add("ExePath1", m_execParam[0].m_path);
   m_helper.Add("ExePath2", m_execParam[1].m_path);
   m_helper.Add("ExePath3", m_execParam[2].m_path);
   m_helper.Add("ExePath4", m_execParam[3].m_path);
   m_helper.Add("ExeRegex1", m_execParam[0].m_regexStr);
   m_helper.Add("ExeRegex2", m_execParam[1].m_regexStr);
   m_helper.Add("ExeRegex3", m_execParam[2].m_regexStr);
   m_helper.Add("ExeRegex4", m_execParam[3].m_regexStr);
   m_helper.Add("ExecName1", m_execParam[0].m_name);
   m_helper.Add("ExecName2", m_execParam[1].m_name);
   m_helper.Add("ExecName3", m_execParam[2].m_name);
   m_helper.Add("ExecName4", m_execParam[3].m_name);


   m_helper.Add("ExePathFind1", m_execParam[0].m_pathFind);
   m_helper.Add("ExePathFind2", m_execParam[1].m_pathFind);
   m_helper.Add("ExePathFind3", m_execParam[2].m_pathFind);
   m_helper.Add("ExePathFind4", m_execParam[3].m_pathFind);
   m_helper.Add("ExePathReplace1", m_execParam[0].m_pathReplace);
   m_helper.Add("ExePathReplace2", m_execParam[1].m_pathReplace);
   m_helper.Add("ExePathReplace3", m_execParam[2].m_pathReplace);
   m_helper.Add("ExePathReplace4", m_execParam[3].m_pathReplace);


   m_helper.Add("IncPath", m_incPath);
   m_helper.Add("TagProgram", m_tagProgram);
   m_helper.Add("TagOptions", m_tagOptions);
   m_helper.Add("TagFiles", m_tagFiles);
   m_helper.Add("TagFile", m_tagFile);
   m_helper.Add("TrackTags", m_trackTags);
   // m_helper.Add("NewLineRegExp", m_newLineRegexp);
   m_helper.Add("FirstFile", m_firstFile);
}

void ProjectOptions::Write(std::ofstream &File)
{
   m_helper.Write(File);
   File << "!Makeoptions";
   if(m_makeCaptureStrout)
     File << " Stdout";
   if(m_makeCaptureStderr)
     File << " Stderr";
   if(m_makeRunDos)
     File << " Dos";
   if(m_makeSaveDoc)
     File << " Save";
   if(m_makeAskArg)
     File << " Ask";
   if(m_makeRunNorm)
     File << " Norm";
   if(m_makeRunLow)
     File << " RunLow";
   File << std::endl;

   File << "!Exeoptions";
   if(m_exeCaptureStdout)
     File << " Stdout";
   if(m_exeCaptureStderr)
     File << " Stderr";
   if(m_exeRunDos)
     File << " Dos";
   if(m_exeAskArg)
     File << " Ask";
   if(m_exeSaveDoc)
     File << " Save";
   if(m_exeRunLow)
     File << " RunLow";
   if(m_exeRunNorm)
     File << " Norm";
   File << std::endl;
}

void ProjectOptions::Read(std::ifstream &File)
{
   std::string Line;

   while(std::getline(File, Line))
   {
      if(!Line.empty() && Line[0] == '!')
      {
         Line = Line.substr(1);
         std::string Field = StrTok(Line, " ");
         std::string::size_type pos;
         for(pos = 0; pos < Field.size() && Field[pos] == ' '; pos++)
         {}
         Field = Field.substr(pos);
         for(pos = 0; pos < Line.size() && Line[pos] == ' '; pos++)
         {}
         Line = Line.substr(pos);

         if(!Field.empty())
         {
            if (!m_helper.Store(Field, Line))
            {
               if(Field == "Makeoptions")
               {
                  std::string Sub;
                  while(!(Sub = StrTok(Line, " ")).empty())
                  {
                    if(Sub == "Stdout")
                      m_makeCaptureStrout = true;
                    else if(Sub == "Stderr")
                      m_makeCaptureStderr = true;
                    else if(Sub == "Dos")
                      m_makeRunDos = true;
                    else if(Sub == "Win")
                      m_makeRunDos = FALSE;
                    else if(Sub == "Save")
                      m_makeSaveDoc = TRUE;
                    else if(Sub == "Ask")
                      m_makeAskArg = TRUE;
                    else if(Sub == "Norm")
                      m_makeRunNorm = TRUE;
                    else if(Sub == "RunLow")
                      m_makeRunLow = TRUE;
                    else
                      WainMessageBox(GetMf(), "Wrong line in Project File", IDC_MSG_OK, IDI_WARNING_ICO);
                  }
                }
                else if(Field == "Exeoptions")
                {
                  std::string Sub;
                  while(!(Sub = StrTok(Line, " ")).empty())
                  {
                    if(Sub == "Stdout")
                      m_exeCaptureStdout = true;
                    else if(Sub == "Stderr")
                      m_exeCaptureStderr = TRUE;
                    else if(Sub == "Dos")
                      m_exeRunDos = TRUE;
                    else if(Sub == "Win")
                      m_exeRunDos = FALSE;
                    else if(Sub == "Ask")
                      m_exeAskArg = TRUE;
                    else if(Sub == "Norm")
                      m_exeRunNorm = TRUE;
                    else if(Sub == "RunLow")
                      m_exeRunLow = TRUE;
                    else if(Sub == "Save")
                      m_exeSaveDoc = TRUE;
                    else
                      WainMessageBox(GetMf(), "Wrong line in Project File", IDC_MSG_OK, IDI_WARNING_ICO);
                  }
                }
                else
                {
                   std::string msg = "Wrong line in Project File: " + Field  + " " + Line;
                   WainMessageBox(GetMf(), msg, IDC_MSG_OK, IDI_WARNING_ICO);
                }
            }
         }
      }
   }
}

void ProjectOptions::Clear()
{
   m_makeCaptureStrout = false;
   m_makeCaptureStderr = false;
   m_makeRunDos = false;
   m_makeSaveDoc = false;
   m_makeAskArg = false;
   m_makeRunNorm = false;
   m_makeRunLow = false;

   m_exeCaptureStdout = false;
   m_exeCaptureStderr = false;
   m_exeRunDos = false;
   m_exeAskArg = false;
   m_exeSaveDoc = false;
   m_exeRunLow = false;
   m_exeRunNorm = false;
   m_makeDefault = 0;
   m_execDefault = 0;

   m_trackTags = false;

   m_tagProgram.resize(0);
   m_tagOptions.resize(0);
   m_tagFiles.resize(0);
   m_tagFile.resize(0);

   m_firstFile = 0;
   m_makeFile = "";
   for (uint32_t i = 0; i < 4; i++)
   {
      m_execParam[i].m_path.clear();
      m_execParam[i].m_args.clear();
      m_execParam[i].m_prog.clear();
   }
   for (uint32_t i = 0; i < 4; i++)
   {
      m_makeParam[i].m_path.clear();
      m_makeParam[i].m_args.clear();
      m_makeParam[i].m_prog.clear();
   }
   m_incPath = "";
}
