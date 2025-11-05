//=============================================================================
// This source code file is a part of Wain.
// It defines ProjectBaseClass
//=============================================================================
#ifndef PROJECT_H_INC
#define PROJECT_H_INC
// #include "nrex.h"
#include "WainUtil.h"

class NavigatorDialog;

class ProjFileInfo
{
public:
   ProjFileInfo()
   {
   }
   ProjFileInfo(const std::string _name, const std::string _shortName, int _iconIndex)
    : m_name(_name),
      m_shortName(_shortName),
      m_iconIndex(_iconIndex),
      m_depth(0)
   {
   }
   void UpdateDepth()
   {
      m_depth = 0;
      for (uint32_t i = 0; i < m_name.size(); i++)
         if (m_name[i] == '\\' || m_name[i] == '/')
            m_depth++;
   }
   std::string m_name;
   std::string m_shortName;
   int m_iconIndex = 0;
   uint32_t m_depth;
};

class WordInfo
{
public:
   WordInfo(std::string& _fileName, uint32_t _lineNo, const std::string& _line) :
      m_fileName(_fileName), m_lineNo(_lineNo), m_line(_line)
   {
   }
   std::string m_fileName;
   uint32_t m_lineNo;
   std::string m_line;
};

class ProjectBaseClass
{
public:
   virtual ~ProjectBaseClass()
   {
   }
   virtual void StartWordThread() = 0;
   virtual void SortFileList() = 0;
   virtual int SetProjectFile(const std::string &) = 0;
   virtual void Setup() = 0;
   virtual void Make(int _index) = 0;
   virtual void Execute(int _index) = 0;
   virtual void Open() = 0;
   virtual void Save() = 0;
   virtual void Manage() = 0;
   virtual void RemoveFile(unsigned int aIndex) = 0;
   virtual void Add() = 0;
   virtual void AddFile(const std::string& _fileName) = 0;
   virtual bool Close(bool aCanCancel, bool aAskToCloseFiles, bool aForceClose) = 0;
   virtual void OpenProjectForFile(const char* _filename) = 0;
   virtual void OnCloseDebugFile(const std::string &) = 0;
   virtual const std::string &GetFileName(unsigned int aIndex, bool aShortName = false) const = 0;
   virtual size_t GetNumFiles() const = 0;
   virtual int GetFileIcon(unsigned int ) const = 0;
   virtual void ReplaceWordParm(class WordThreadParam* _parm) = 0;

   virtual const std::string &GetTagFile() const = 0;
   virtual const std::string &GetTagOptions() const = 0;
   virtual const std::string &GetTagFiles() const = 0;
   virtual const std::string &GetTagProgram() const = 0;
   virtual bool GetTrackTags() const = 0;

   virtual void SetTagFile(const std::string &) = 0;
   virtual void SetTagOptions(const std::string &) = 0;
   virtual void SetTagFiles(const std::string &) = 0;
   virtual void SetTagProgram(const std::string &) = 0;
   virtual void SetTrackTags(bool ) = 0;

   virtual void UpdateFileList(bool _force) = 0;

   virtual const std::string &GetProjectName() const = 0;

   virtual const std::string &GetIncPath() const = 0;

   virtual bool Search(int aDirection, const std::string &aStr, bool aReset, int &aOffset) const = 0;
   virtual RtvStatus MakeProjectFileList(std::string &aFileName, const std::string &aTypes) const = 0;
   virtual RtvStatus MakeProjectFileList(std::vector<std::string>& _fileList, const std::string& _types) const = 0;

   virtual void HandleGetIcon(class ThreadGetIconConClass *) = 0;

   virtual void MakeDone() = 0;
   virtual void ExeDone() = 0;

   virtual const std::string &GetMakeFile() const = 0;

   virtual bool CheckFileName(CString &aDest, const std::string &aFileName) = 0;

   virtual bool GetFileInLine(const char* _ss, std::string& _fn, uint32_t& _lineNo, uint32_t& _columnNo, bool _isMake, uint32_t _nr) = 0;
   virtual void GetFileList(std::vector<std::string>& _files) = 0;
   class WordThreadParam* m_wordParam[32] = {0}; // Todo fix 32
   virtual bool GetWordInfo(std::vector<WordInfo>& _wordInfo, const std::string& _word, int _propIndex) = 0;
protected:
   ProjectBaseClass() {}
};

extern ProjectBaseClass *CreateProject(class NavigatorDialog *);


#endif // PROJECT_H_INC
