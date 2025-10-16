#ifndef PROJECTOPTIONS_H_INC
#define PROJECTOPTIONS_H_INC

struct ProjectExecParam
{
   std::string m_path;
   std::string m_args;
   std::string m_prog;
   std::string m_regexStr;
   std::string m_pathFind;
   std::string m_pathReplace;
   std::string m_name;
};

class ProjectOptionsHelper
{
public:
   void Add(const char* _parmName, std::string& _parm)
   {
      m_mapStr.insert(std::pair<std::string, std::string&>(_parmName, _parm));
   }
   void Add(const char* _parmName, uint32_t& _parm)
   {
      m_mapInt.insert(std::pair<std::string, uint32_t&>(_parmName, _parm));
   }
   void Write(std::ofstream& _file)
   {
      for (auto i = m_mapStr.begin(); i != m_mapStr.end(); i++)
      {
         _file << "!" << i->first << " " << i->second << std::endl;
      }
      for (auto i = m_mapInt.begin(); i != m_mapInt.end(); i++)
      {
         _file << "!" << i->first << " " << i->second << std::endl;
      }
   }
   bool Store(std::string& _parmName, std::string& _parm)
   {
      {
         auto i = m_mapStr.find(_parmName);
         if (i != m_mapStr.end())
         {
            i->second = _parm;
            return true;
         }
      }
      {
         auto i = m_mapInt.find(_parmName);
         if (i != m_mapInt.end())
         {
            i->second = strtol(_parm.c_str(), 0, 10);
            return true;
         }
      }
      return false;
   }
   bool Store(std::string& _parmName, uint32_t& _parm)
   {
      auto i = m_mapInt.find(_parmName);
      if (i != m_mapInt.end())
      {
         i->second = _parm;
         return true;
      }
      return false;
   }
   std::map<std::string, std::string&> m_mapStr;
   std::map<std::string, uint32_t&> m_mapInt;
};

class ProjectOptions
{
public:
   ProjectOptions();
   void Clear();
   std::string m_makeFile;

   ProjectExecParam m_execParam[4];
   ProjectExecParam m_makeParam[4];

   std::string m_incPath;

   bool m_makeCaptureStrout;
   bool m_makeCaptureStderr;
   bool m_makeRunDos;
   bool m_makeSaveDoc;
   bool m_makeAskArg;
   bool m_makeRunNorm;
   bool m_makeRunLow;

   bool m_exeCaptureStdout;
   bool m_exeCaptureStderr;
   bool m_exeRunDos;
   bool m_exeSaveDoc;
   bool m_exeAskArg;
   bool m_exeRunLow;
   bool m_exeRunNorm;

   uint32_t m_trackTags;

   // std::string m_newLineRegexp;

   std::string m_tagProgram;
   std::string m_tagOptions;
   std::string m_tagFiles;
   std::string m_tagFile;

   size_t m_firstFile;

   void Write(std::ofstream &aFile);
   void Read(std::ifstream &aFile);

   uint32_t m_makeDefault;
   uint32_t m_execDefault;
   ProjectOptionsHelper m_helper;
};

#endif