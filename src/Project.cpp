//=============================================================================
// This source code file is a part of Wain.
// It implements ProjectClass Project.h
//=============================================================================
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
#include "WordListThread.h"
#include "DocProp.h"
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class ProjFileInfoSortClass
{
public:
   ProjFileInfoSortClass(){}
   bool operator () (const ProjFileInfo &lhs, const ProjFileInfo &rhs)
   {
      return stricmp(lhs.m_name.c_str(), rhs.m_name.c_str()) < 0;
   }
};

class ProjFileInfoSortShortClass
{
public:
   ProjFileInfoSortShortClass(){}
   bool operator () (const ProjFileInfo &lhs, const ProjFileInfo &rhs)
   {
      return stricmp(lhs.m_shortName.c_str(), rhs.m_shortName.c_str()) < 0;
   }
};

class ProjectClass : public ProjectBaseClass
{
private:
  bool m_first;
  ProjectOptions m_projectOptions;
  bool AddFileName(const std::string &aName);

  std::string m_makeErrorFile;
  std::string m_makeExeLog;
  NavigatorDialog *m_dlg;
  ProjectClass();
  ~ProjectClass();

  std::vector<ProjFileInfo >m_projFileList;

  int ReadProjectFile(const std::string &aFileName, bool aOpenFiles = true);

  void DoSave(bool aCloseFile);
  void DoMake(int _index);

  bool m_makeRunning = false;
  bool m_exeRunning = false;
  void Clean(void);
public:
  void AddFile(const std::string& _fileName);
  ProjectClass(NavigatorDialog *aDlg);

  std::string m_projectName;
  bool m_changed;

  unsigned int m_threadCounter;

   virtual void StartWordThread() override;
   virtual void SortFileList();
   virtual int SetProjectFile(const std::string &);
   virtual void Setup();
   virtual void Make(int _index);
   virtual void Execute(int _index);
   virtual void Open();
   virtual void Save();
   virtual void Manage();
   virtual void RemoveFile(unsigned int aIndex);
   virtual void Add();
   virtual bool Close(bool aCanCancel, bool aAskToCloseFiles, bool aForceClose);
   virtual void OpenProjectForFile(const char* _filename) override;
   virtual void OnCloseDebugFile(const std::string &aFileName);
   virtual const std::string &GetFileName(unsigned int aIndex, bool aShortName) const;
   virtual size_t GetNumFiles() const;
   virtual int GetFileIcon(unsigned int ) const;

   virtual const std::string &GetTagFile() const { return m_projectOptions.m_tagFile; }
   virtual const std::string &GetTagOptions() const { return m_projectOptions.m_tagOptions; }
   virtual const std::string &GetTagFiles() const { return m_projectOptions.m_tagFiles; }
   virtual const std::string &GetTagProgram() const { return m_projectOptions.m_tagProgram; }
   virtual bool GetTrackTags() const { return m_projectOptions.m_trackTags; }

   virtual void SetTagFile(const std::string &aFile) { m_projectOptions.m_tagFile = aFile; }
   virtual void SetTagOptions(const std::string &aOptions) { m_projectOptions.m_tagOptions = aOptions; }
   virtual void SetTagFiles(const std::string &aFiles) { m_projectOptions.m_tagFiles = aFiles; }
   virtual void SetTagProgram(const std::string &aProgram) { m_projectOptions.m_tagProgram = aProgram; }
   virtual void SetTrackTags(bool aTrack) { m_projectOptions.m_trackTags = aTrack; }

   virtual void UpdateFileList(bool _force) override;

   virtual const std::string &GetProjectName() const {return m_projectName; }
   virtual const std::string &GetIncPath() const {return m_projectOptions.m_incPath; }

   virtual bool Search(int aDirection, const std::string &aStr, bool aReset, int &aOffset) const;
   virtual RtvStatus MakeProjectFileList(std::string &aFileName, const std::string &aTypes) const;
   virtual RtvStatus MakeProjectFileList(std::vector<std::string>& _fileList, const std::string& _types) const;
   virtual void HandleGetIcon(ThreadGetIconConClass *);

   virtual void MakeDone();
   virtual void ExeDone();

   virtual const std::string &GetMakeFile() const {return m_projectOptions.m_makeFile; }

   virtual bool CheckFileName(CString &aDest, const std::string &aFileName);
   virtual bool GetFileInLine(const char* _ss, std::string& _fn, uint32_t& _lineNo, uint32_t& _columnNo, bool _isMake, uint32_t _nr);
   virtual void GetFileList(std::vector<std::string>& _files) override;

   virtual bool GetWordInfo(std::vector<WordInfo>& _wordInfo, const std::string& _word, int _propIndex);
   virtual void ReplaceWordParm(class WordThreadParam* _parm) override;
};

ProjectBaseClass *CreateProject(class NavigatorDialog *aDlg)
{
   return new ProjectClass(aDlg);
}

/* Implementation of ProjectClass */
ProjectClass::ProjectClass(NavigatorDialog *aDlg)
 : ProjectBaseClass(),
   m_dlg(aDlg),
   m_changed(false),
   m_makeRunning(false),
   m_exeRunning(false),
   m_threadCounter(0),
   m_first(true)
{
}

ProjectClass::~ProjectClass()
{
   for (int i = 0; i < 32; i++)
      delete m_wordParam[i];
}

RtvStatus ProjectClass::MakeProjectFileList(std::vector<std::string>& _fileList, const std::string& _types) const
{
   if(m_projectName.empty())
      return RtvStatus::rtv_no_project;
   if(m_projFileList.size() == 0)
      return RtvStatus::rtv_no_project_files;

   std::vector<ProjFileInfo >::size_type i;
   if(_types.empty())
   {
      for(i = 0; i < m_projFileList.size(); i++)
      {
         _fileList.push_back(m_projFileList[i].m_name);
      }
   }
   else
   {
      for(i = 0; i < m_projFileList.size(); i++)
      {
         std::string::size_type start = 0;
         std::string::size_type idx = _types.find(';');
         bool found = false;
         std::string fExt;
         MySplitPath(m_projFileList[i].m_name.c_str(), SP_EXT, fExt);
         while (!found && idx != std::string::npos)
         {
            if (_types[start] == '*')
            {
               start++;
            }
            std::string ext = _types.substr(start, idx - start);
            if (!stricmp(ext.c_str(), fExt.c_str()))
            {
               _fileList.push_back(m_projFileList[i].m_name);
               found = true;
            }
            start = idx + 1;
            idx = _types.find(';', start);
         }
      }
   }
   return _fileList.size() > 0 ? RtvStatus::rtv_no_error : RtvStatus::rtv_no_project_files;
}

RtvStatus ProjectClass::MakeProjectFileList(std::string &aFileName, const std::string &aTypes) const
{
  static unsigned int FileCounter;

  if(m_projectName.empty())
    return RtvStatus::rtv_no_project;
  if(m_projFileList.size() == 0)
    return RtvStatus::rtv_no_project_files;
  std::stringstream ss;
  ss << "$T$$PFE$" << FileCounter << ".lst";
  ss >> aFileName;

  if(++FileCounter >= 10)
    FileCounter = 0;

  RtvStatus ret;
  if((ret = wainApp.ReplaceTagValues(aFileName, std::string(""))) != RtvStatus::rtv_no_error)
  { /* No need to display the warning, ReplaceTagValues, who called me will pass the warning on */
    return ret;
  }
  FILE *f = fopen(aFileName.c_str(), "wt");
  if(!f)
    return RtvStatus::rtv_no_temp_path;  /* Probably not correct error, but this should never happen */

  char ext[256], get_ext[256];
  int nof_printed = 0;
  std::vector<ProjFileInfo >::size_type i;
  if(aTypes.empty())
  {
    for(i = 0; i < m_projFileList.size(); i++, nof_printed++)
      fprintf(f, "%s\n", m_projFileList[i].m_name.c_str());
  }
  else
  {
    std::string::size_type idx;
    for(idx = 0; idx < aTypes.size(); )
    {
      for(i = 0; i < 256 && idx < aTypes.size() && aTypes[idx] != '$' && aTypes[idx] != ';'; idx++, i++)
        get_ext[i] = aTypes[idx];
      get_ext[i] = 0;
      if(idx < aTypes.size() && aTypes[idx] == ';')
        idx++;
      if(!strlen(get_ext))
      {
        fclose(f);
        return RtvStatus::rtv_ill_formed_tag;
      }
      for(i = 0; i < m_projFileList.size(); i++)
      {
        MySplitPath(m_projFileList[i].m_name.c_str(), SP_EXT, ext);
        if(ext[0] != 0 && !stricmp(&ext[1], get_ext))
        {
          fprintf(f, "%s\n", m_projFileList[i].m_name.c_str());
          nof_printed++;
        }
      }
    }
  }
  fclose(f);

  return nof_printed ? RtvStatus::rtv_no_error : RtvStatus::rtv_no_project_files;
}


void ProjectClass::Manage(void)
{
   if(m_projectName.empty())
   {
      WainMessageBox(GetMf(), "You must first open a project", IDC_MSG_OK, IDI_INFORMATION_ICO);
      return;
   }

  FileSelectDialogClass pmd;

  std::vector<ProjFileInfo >::size_type i;
  for(i = 0; i < m_projFileList.size(); i++)
     pmd.m_fileList.push_back(m_projFileList[i].m_name);

  pmd.m_caption = "Project Manage";

  if(pmd.DoModal() == IDOK)
  {
    Clean();
    for(i = 0; i < pmd.m_fileList.size(); i++)
    {
      AddFileName(pmd.m_fileList[i]);
    }

    SortFileList();
    UpdateFileList(true);
    GetMf()->m_navigatorDialog.BuildTags(TRUE);
    m_changed = TRUE;
  }
}

void ProjectClass::Add()
{
   if(m_projectName.empty())
      return;
   MyFileDialogClass fd(FD_OPEN | FD_LAST_PATH | FD_SET_LAST_PATH | FD_MULTI_FILES | FD_DOC_FILTER, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY);
   char *buffer = (char *)malloc(32767);
   *buffer = '\0';
   fd.m_ofn.lpstrFile = buffer;
   fd.m_ofn.nMaxFile = 32767;
   if(fd.DoModal() == IDOK)
   {
      POSITION pos = fd.GetStartPosition();
      CString s = "";
      while(pos)
      {
         s = fd.GetNextPathName(pos);
         if(AddFileName((const char *)s))
            m_changed = TRUE;
      }
      SortFileList();
      UpdateFileList(true);
      GetMf()->m_navigatorDialog.BuildTags(TRUE);
   }
   free(buffer);
}

void ProjectClass::UpdateFileList(bool _force)
{
   if (!_force)
      return;
   if (m_projFileList.empty())
       return;
   // Find common ground
   bool equal = true;
   uint32_t commonLen;
   for (commonLen = 0; commonLen < m_projFileList.size() && equal; commonLen += equal ? 1 : 0)
   {
      for (uint32_t i = 1; i < m_projFileList.size(); i++)
      {
         if (tolower(unsigned(m_projFileList[i].m_name[commonLen])) != tolower(unsigned(m_projFileList[0].m_name[commonLen])))
            equal = false;
      }
   }
   uint32_t lastButOneSlash = 0;
   uint32_t lastSlash = 0;
   for (uint32_t i = 0; !equal && i < m_projFileList[i].m_name.size() && i < commonLen; i++)
   {
      if (m_projFileList[0].m_name[i] == '\\')
      {
         lastButOneSlash = lastSlash;
         lastSlash = i + 1;
      }
   }
   for (uint32_t i = 0; i < m_projFileList.size(); i++)
   {
      m_projFileList[i].UpdateDepth();
   }
   m_dlg->m_projectTree->ClearTree(0);
   for (uint32_t depth = 64; depth > 0; depth--)
   {
      for (uint32_t i = 0; i < m_projFileList.size(); i++)
      {
         if (m_projFileList[i].m_depth == depth)
         {
            m_dlg->m_projectTree->AddItem(m_projFileList[i].m_name, m_projFileList[i].m_name.substr(lastButOneSlash));
         }
      }
   }
   m_dlg->m_projectTree->EndUpdate();
}

void ProjectClass::Setup(void)
{
  ProjectSetupDialogClass ps;
  ps.m_projectOptions = m_projectOptions;

  if(ps.DoModal() == IDOK)
  {
    m_projectOptions = ps.m_projectOptions;
     m_changed = TRUE;
  }
}

bool ProjectClass::AddFileName(const std::string &aName)
{
   ProjFileInfo Info;
   Info.m_name = aName;

   MySplitPath(aName.c_str(), SP_FILE | SP_EXT, Info.m_shortName);
   Info.m_iconIndex = -1;
   m_projFileList.push_back(Info);

   return true;
}

void ProjectClass::SortFileList()
{
   if(wainApp.gs.m_navigatorBarShortNames)
   {
      ProjFileInfoSortShortClass Compare1;
      std::sort(m_projFileList.begin(), m_projFileList.end(), Compare1);
   }
   else
   {
      ProjFileInfoSortClass Compare2;
      std::sort(m_projFileList.begin(), m_projFileList.end(), Compare2);
   }
   std::vector<ProjFileInfo >::iterator it, j;
   for(it = m_projFileList.begin(); it < m_projFileList.end(); )
   {
      j = it + 1;
      if(j < m_projFileList.end())
      {
         if(!StrICmp(it->m_name, j->m_name))
            m_projFileList.erase(j);
         else
            it++;
      }
      else
         it++;
   }
  /* It's time to get the icons */
  ThreadGetIconParmClass *req = new ThreadGetIconParmClass;
  req->m_index = ++m_threadCounter;

  for(it = m_projFileList.begin(); it != m_projFileList.end(); ++it)
     req->m_nameList.push_back(it->m_name);


  AfxBeginThread(ThreadGetIcon, req, THREAD_PRIORITY_LOWEST);
}

void ProjectClass::Save(void)
{
  DoSave(false);
}

void ProjectClass::DoSave(bool CloseFile)
{
  if(m_projectName.empty())
    return;
  std::ofstream File(m_projectName.c_str());
  if(!File)
  {
    CString s;
    s = "Unable to open\r\n";
    s += m_projectName.c_str();
    s += "\r\nDo you want to save the project with another name?";

    if(WainMessageBox(GetMf(), s, IDC_MSG_YES | IDC_MSG_NO, IDI_ERROR_ICO) == IDC_MSG_YES)
    {
      char s1[124], s2[124], s3[1024];
      memset(s3, 0, sizeof(s3));
      int n1 = sprintf(s1, "Wain Projects (*%s)", wainApp.gs.m_projectExtension.c_str()); // Fixme, use std::string
      int n2 = sprintf(s2, "*%s", wainApp.gs.m_projectExtension.c_str());
      strcpy(s3, s1);
      strcpy(&s3[n1 + 1], s2);
      memcpy(&s3[n1 + n2 + 2], "All Files (*.*)\0*.*", 20);
      const char *DefExt = wainApp.gs.m_projectExtension.c_str();
      while(*DefExt == '.')
        DefExt++;
      MyFileDialogClass fd(FD_SAVE | FD_LAST_PATH | FD_SET_LAST_PATH, OFN_CREATEPROMPT | OFN_HIDEREADONLY, DefExt, NULL, s3, m_dlg);
      if(fd.DoModal() == IDOK)
      {
        m_projectName = fd.GetPathName();
        strcpy(wainApp.gs.m_projectName, m_projectName.c_str());
        File.open(m_projectName.c_str());
      }
    }
  }

   if(File.is_open())
   {
      m_projectOptions.Write(File);
      m_changed = FALSE;
      /* Try to see if the active document is one of our project files  */
      std::vector<ProjFileInfo >::size_type i;
      if(GetMf()->m_viewList.m_currentView)
      {
        std::string temp = GetMf()->m_viewList.m_currentView->m_name;
        int jx = -1;
        for(int idx = 0; idx < int(m_projFileList.size()); idx++)
          if(m_projFileList[idx].m_name == temp)
            jx = idx;
        File << "!FirstFile " << jx << std::endl;
      }
      else
        File << "!FirstFile -1" << std::endl;

      for(i = 0; i < m_projFileList.size(); i++)
      {
        int LineNo = -1;
        if(CloseFile)
        {
          LineNo = GetMf()->TryCloseFile(m_projFileList[i].m_name.c_str());
          if(LineNo >= 1)
            LineNo--;
        }
        File << "\"" << m_projFileList[i].m_name << "\" " << LineNo << std::endl;
      }
      File.close();
   }
}

bool ProjectClass::Close(bool aCanCancel, bool aAskToCloseFiles, bool aForceClose)
{
  if(m_projectName.empty())
    return true;
  bool CloseFiles = aForceClose;
  if(aAskToCloseFiles)
  {
    if(aCanCancel)
    {
      WORD res = WainMessageBox(GetMf(), "Do you want to close files belonging to the project", IDC_MSG_YES | IDC_MSG_NO | IDC_MSG_CANCEL, IDI_QUESTION_ICO);
      if(res == IDC_MSG_CANCEL)
        return false;
      else if(res == IDC_MSG_YES)
        CloseFiles = true;
    }
    else
    {
      if(WainMessageBox(GetMf(), "Do you want to close files belonging to the project", IDC_MSG_YES | IDC_MSG_NO, IDI_QUESTION_ICO) == IDC_MSG_YES)
        CloseFiles = true;
    }
  }

  DoSave(CloseFiles);
  m_changed = false;
  Clean();
  return true;
}

void ProjectClass::Clean(void)
{
   m_projFileList.clear();
}

void ProjectClass::Open(void)
{
  if(!Close(true, true, false))
    return;
  std::string S1;
  S1 = "Wain Projects (*";
  S1 += wainApp.gs.m_projectExtension;
  S1 += ")";
  std::string S2;
  S2 = "*";
  S2 += wainApp.gs.m_projectExtension;
  S1.append(std::string::size_type(1), '\0');
  S1 += S2;
  S1.append(std::string::size_type(1), '\0');
  S1 += "All Files (*.*)";
  S1.append(std::string::size_type(1), '\0');
  S1 += "*.*";
  S1.append(std::string::size_type(1), '\0');
  S1.append(std::string::size_type(1), '\0');

  const char *DefExt = wainApp.gs.m_projectExtension.c_str();
  while(*DefExt == '.')
    DefExt++;
  MyFileDialogClass fd(FD_OPEN | FD_LAST_PATH | FD_SET_LAST_PATH, OFN_CREATEPROMPT | OFN_HIDEREADONLY, DefExt, NULL, S1.c_str(), m_dlg);
  if(fd.DoModal() == IDOK)
  {
    GetMf()->HandleRecentProjects(m_projectName.c_str());
    m_projectName = fd.GetPathName();
    m_dlg->m_projectChanged = TRUE;
  }
  if(!access(m_projectName.c_str(), 4))
  {
     ReadProjectFile(m_projectName);
  }
  strcpy(wainApp.gs.m_projectName, m_projectName.c_str());
  UpdateFileList(true);
  GetMf()->ProjectChanged(m_projectName.c_str());
}

int ProjectClass::SetProjectFile(const std::string &aFileName)
{
   if(!m_first)
   {
      if(!Close(false, true, false))
         return false;

      GetMf()->HandleRecentProjects(m_projectName.c_str());
   }
   m_projectName = aFileName;
   int active = -1;
   if(!access(m_projectName.c_str(), 4))
   {
      active = ReadProjectFile(m_projectName);
   }
   strcpy(wainApp.gs.m_projectName, m_projectName.c_str());
   UpdateFileList(true);
   m_dlg->DoBuildTags(NULL, FALSE);
   if(!m_first)
   {
      m_dlg->m_globalTags.Clear(m_dlg->m_globalTags.m_state == m_dlg->m_navBarState);
      GetMf()->ProjectChanged(m_projectName.c_str());
   }
   m_first = false;
   return active;
}

int ProjectClass::ReadProjectFile(const std::string &aName, bool aOpenFiles)
{
  int rv = -1;

  m_projectOptions.Clear();
  std::ifstream File(aName.c_str());

  if(File.is_open())
  {
    m_projectOptions.Read(File);
    File.close();

    std::ifstream File2(aName.c_str());
    std::string Line;

    while(std::getline(File2, Line))
    {

      if(Line[0] != '!')
      {
        std::string::size_type d = Line.find_last_of(' ');
        if(d != std::string::npos)
        {
           std::string Number = Line.substr(d);
           Line = Line.substr(0, d - 1);
           Line = Line.substr(1);

           AddFileName(Line.c_str());
           if(aOpenFiles)
           {
               int LineNo = strtol(Number.c_str(), NULL, 10);
               if(LineNo >= 0)
               {
                 WainDoc *doc = wainApp.OpenDocument(Line.c_str());
                 if(doc)
                 {
                   doc->m_view->GotoLineNo(LineNo + 1);
                   if(m_projFileList.size() - 1 == m_projectOptions.m_firstFile)
                     rv = doc->m_view->m_winNr;
                 }
             }
          }
        }
      }
    }
    File2.close();
  }

  SortFileList();
  return rv;
}

void ProjectClass::RemoveFile(unsigned int aIndex)
{
   if(aIndex < m_projFileList.size())
   {
      m_projFileList.erase(m_projFileList.begin() + aIndex);
      UpdateFileList(true);
      m_changed = true;
   }
}

void ProjectClass::Make(int _index)
{
   if (_index < 0)
   {
      DoMake(m_projectOptions.m_makeDefault);
   }
   else
   {
      DoMake(_index);
   }
}

void ProjectClass::DoMake(int _index)
{
   if(m_makeRunning)
   {
      WainMessageBox(GetMf(), "Make is currently running\r\nPlease wait for it to finish", IDC_MSG_OK, IDI_INFORMATION_ICO);
      return;
   }
   if(m_projectOptions.m_makeSaveDoc)
      GetMf()->SaveAllDocs();
   ChildFrame *child_frame = (ChildFrame *)GetMf()->MDIGetActive();
   if (_index < 0 || _index >= 4)
   {
      _index = 0;
   }
   std::string command = m_projectOptions.m_makeParam[_index].m_prog + std::string(" ") + m_projectOptions.m_makeParam[_index].m_args;
   std::string dir = m_projectOptions.m_makeParam[_index].m_path;
   const char *s = NULL;
   if(child_frame)
      s = child_frame->GetDocument()->GetPathName();
  RtvStatus error;
  if((error = wainApp.ReplaceTagValues(command, std::string(s ? s : ""))) != RtvStatus::rtv_no_error)
  {
    DisplayRtvError(GetMf(), "Unable to build make command", error, true);
    return;
  }

  if(m_projectOptions.m_makeAskArg)
  {
    SimpleDialog sd("Make Arguments", command.c_str());
    if(sd.DoModal() != IDOK)
      return;
    command = sd.m_msg;
  }
   if (dir == "")
   {
      dir = "$MP$";
      if((error = wainApp.ReplaceTagValues(dir, std::string(""))) != RtvStatus::rtv_no_error)
      {
         DisplayRtvError(GetMf(), "Unable to get dir for make", error, true);
         return;
       }
   }
  m_makeErrorFile = "$T$$MF$.err";
  if((error = wainApp.ReplaceTagValues(m_makeErrorFile, std::string(""))) != RtvStatus::rtv_no_error)
  {
    DisplayRtvError(GetMf(), "Unable to build err file name", error, true);
    return;
  }
  ::remove(m_makeErrorFile.c_str());
  GetMf()->RemoveFile(m_makeErrorFile.c_str());

  ProcessClass *tool = GetMf()->m_tools->NewTool();
  tool->m_title = std::string("Wain Project Make: ") + m_projectOptions.m_makeParam[_index].m_name;
  tool->m_label = m_projectOptions.m_makeParam[_index].m_name;
  tool->m_runDos = TRUE;
  tool->m_captureStderr = TRUE;
  tool->m_captureStdout = TRUE;
  tool->m_command = command;
  tool->m_outFile = m_makeErrorFile;
  tool->m_path = dir;
  tool->m_dispModeMax = FALSE;
  tool->m_dispModeHidden = m_projectOptions.m_makeRunNorm ? false : true;
  tool->m_dispModeNorm = m_projectOptions.m_makeRunNorm ? true : false;
  tool->m_append = false;
  tool->m_msgId = IDM_PROJ_MAKE_ID;
  tool->m_runLow = m_projectOptions.m_makeRunLow ? true : false;
  tool->m_isProject = true;
  tool->m_isMake = true;
  tool->m_toolIndex = _index + 0x1000;

   if(!tool->Run())
   {
      GetMf()->m_tools->RemoveTool(tool);
   }
   else
   {
      m_makeRunning = true;
   }
}

void ProjectClass::Execute(int _index)
{
  if(m_exeRunning)
  {
    WainMessageBox(GetMf(), "Project execute is currently running\r\nPlease wait for it to finish", IDC_MSG_OK, IDI_INFORMATION_ICO);
    return;
  }
  ChildFrame *child_frame = (ChildFrame *)GetMf()->MDIGetActive();
   if (_index < 0)
   {
      _index = m_projectOptions.m_execDefault;
   }
   std::string command;
   std::string exePath;
   if (_index < 0 || _index >= 4)
      _index = 0;
   command = m_projectOptions.m_execParam[_index].m_prog + std::string(" ") + m_projectOptions.m_execParam[_index].m_args;
   exePath = m_projectOptions.m_execParam[_index].m_path;

  const char *s = NULL;
  if(child_frame)
    s = child_frame->GetDocument()->GetPathName();
  RtvStatus error;
  if((error = wainApp.ReplaceTagValues(command,  std::string(s ? s : ""))) != RtvStatus::rtv_no_error)
  {
    DisplayRtvError(GetMf(), "Unable to build Project execute command", error, true);
    return;
  }
  m_makeExeLog = "$T$$PF$.log";
  if((error = wainApp.ReplaceTagValues(m_makeExeLog, std::string(""))) != RtvStatus::rtv_no_error)
  {
    DisplayRtvError(GetMf(), "Unable to build log file name", error, true);
    return;
  }
  ::remove(m_makeExeLog.c_str());
  GetMf()->RemoveFile(m_makeExeLog.c_str());

  ProcessClass *tool = GetMf()->m_tools->NewTool();
  tool->m_isProject = true;
  tool->m_isMake = false;
  tool->m_toolIndex = _index+0x1100;

  tool->m_title = "Wain Project Execute: " + m_projectOptions.m_execParam[_index].m_name;
  tool->m_label = m_projectOptions.m_execParam[_index].m_name;
  if(m_projectOptions.m_exeRunDos)
  {
     tool->m_runDos = true;
     tool->m_captureStderr = m_projectOptions.m_exeCaptureStderr;
     tool->m_captureStdout = m_projectOptions.m_exeCaptureStdout;
  }
  else
  {
     tool->m_runDos = false;
     tool->m_captureStderr = false;
     tool->m_captureStdout = false;
  }

  tool->m_command = command;
  tool->m_outFile = m_makeExeLog;
  tool->m_path = exePath;
  tool->m_dispModeMax = false;
  tool->m_dispModeHidden = m_projectOptions.m_exeRunNorm ? false : true;
  tool->m_dispModeNorm = m_projectOptions.m_exeRunNorm ? true : false;
  tool->m_append = false;
  tool->m_msgId = IDM_PROJ_EXE_ID;
  tool->m_runLow = m_projectOptions.m_exeRunLow ? true : false;

  if(!tool->Run())
  {
     GetMf()->m_tools->RemoveTool(tool);
  }
  else
  {
     m_exeRunning = true;
  }
}

void ProjectClass::MakeDone()
{
   m_makeRunning = false;
}

void ProjectClass::ExeDone()
{
   m_exeRunning = false;
}

bool ProjectClass::CheckFileName(CString &aDest, const std::string &aFileName)
{
  for(std::vector<ProjFileInfo >::size_type i = 0; i < m_projFileList.size(); i++)
  {
    if(!stricmp(aFileName.c_str(), m_projFileList[i].m_shortName.c_str()))
    {
      aDest = m_projFileList[i].m_name.c_str();
      return true;
    }
  }
  return false;
}

void ProjectClass::OpenProjectForFile(const char* _filename)
{
   m_dlg->m_projectTree->Select(_filename);
}

void ProjectClass::OnCloseDebugFile(const std::string& _fileName)
{
   if(m_makeErrorFile == _fileName)
   {
      m_makeErrorFile.erase();
      m_makeRunning = false;
   }
   else if(m_makeExeLog == _fileName)
   {
      m_makeExeLog.erase();
      m_makeRunning = false;
   }
}

UINT ThreadGetIcon(LPVOID parm)
{
  ThreadGetIconParmClass *read_parm = (ThreadGetIconParmClass *)parm;
  ThreadGetIconConClass *get_parm = new ThreadGetIconConClass;
  get_parm->m_index = read_parm->m_index;
  get_parm->m_imageList = 0;

  for(unsigned int i = 0; i < read_parm->m_nameList.size(); i++)
  {
    SHFILEINFO sh_FileInfo;
    HIMAGELIST image_list;
    image_list = (HIMAGELIST )SHGetFileInfo(read_parm->m_nameList[i].c_str(), 0, &sh_FileInfo, sizeof(sh_FileInfo), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
    if(image_list)
      get_parm->m_imageList = image_list;
    get_parm->m_iconList.push_back(sh_FileInfo.iIcon);
  }
  delete read_parm;
  wainApp.PostThreadMessage(GetIconMsgId, (WPARAM )get_parm, 0);

  return 0;
}

void ProjectClass::HandleGetIcon(ThreadGetIconConClass *aMsg)
{
  if(aMsg->m_index == m_threadCounter)
  {
    unsigned int i;
    for(i = 0; i < aMsg->m_iconList.size() && i < m_projFileList.size(); i++)
    {
      m_projFileList[i].m_iconIndex = aMsg->m_iconList[i];
    }
    if(aMsg->m_imageList)
      m_dlg->m_sysImageList = aMsg->m_imageList;
  }
}

const std::string &ProjectClass::GetFileName(unsigned int aIndex, bool aShortName) const
{
   static const std::string Dummy;
   if(aIndex < m_projFileList.size())
      return aShortName ? m_projFileList[aIndex].m_shortName : m_projFileList[aIndex].m_name;
   return Dummy;
}

size_t ProjectClass::GetNumFiles() const
{
   return m_projFileList.size();
}

int ProjectClass::GetFileIcon(unsigned int aIndex) const
{
   if(aIndex < m_projFileList.size())
      return m_projFileList[aIndex].m_iconIndex;
   return -1;
}

bool ProjectClass::Search(int _direction, const std::string& _str, bool _reset, int& _offset) const
{
   return m_dlg->m_projectTree->DoSearch(_direction, _str.c_str(), _reset, _offset);
}

void ProjectClass::AddFile(const std::string& _fileName)
{
   AddFileName(_fileName);
   SortFileList();
   UpdateFileList(true);
   GetMf()->m_navigatorDialog.BuildTags(true);
   m_changed = true;
}

bool ProjectClass::GetFileInLine(const char* _ss, std::string& _fn, uint32_t& _lineNo, uint32_t& _columnNo, bool _isMake, uint32_t _nr)
{
   _nr &= 0x0003;
   std::string lineRegexStr = _isMake ? m_projectOptions.m_makeParam[_nr].m_regexStr : m_projectOptions.m_execParam[_nr].m_regexStr;
   std::string lineFindRegex = _isMake ? m_projectOptions.m_makeParam[_nr].m_pathFind : m_projectOptions.m_execParam[_nr].m_pathFind;
   std::string lineReplaceRegex = _isMake ? m_projectOptions.m_makeParam[_nr].m_pathReplace : m_projectOptions.m_execParam[_nr].m_pathReplace;
   if (lineRegexStr.empty())
   {
      return false;
   }
   try
   {
      std::regex regEx(lineRegexStr.c_str());
      std::cmatch cm;
      std::regex_match(_ss, cm, regEx);
      if (cm.size() > 3)
      {
         _fn = cm.str(1).c_str();
         if (!lineFindRegex.empty())
         {
            try
            {
               std::regex e(lineFindRegex);
               _fn = std::regex_replace(_fn, e, lineReplaceRegex);
            }
            catch (const std::regex_error& err)
            {
               std::string msg("Path replace Regex error: ");
               msg += err.what();
               WainMessageBox(GetMf(), msg.c_str(), IDC_MSG_OK, IDI_ERROR_ICO);
               return false;
            }
         }
         else if (!lineReplaceRegex.empty())
         {  // We'll just put the replace tekst in at the begining
            _fn = lineReplaceRegex + _fn;
         }

         _lineNo = strtol(cm.str(2).c_str(), NULL, 0);
         if (cm.size() > 3)
         {
            _columnNo = strtol(cm.str(3).c_str(), 0, 0);
         }
         else
         {
            _columnNo = 0;
         }
         return true;
      }
   }
   catch (const std::regex_error& err)
   {
      std::string msg("Regex error: ");
      msg += err.what();
      WainMessageBox(GetMf(), msg.c_str(), IDC_MSG_OK, IDI_ERROR_ICO);
      return false;
   }
   return false;
}

void ProjectClass::GetFileList(std::vector<std::string>& _files)
{
   for (auto i : m_projFileList)
   {
      _files.push_back(i.m_name);
   }
}

bool ProjectClass::GetWordInfo(std::vector<WordInfo>& _wordInfo, const std::string& _word, int _propIndex)
{
   if (!m_wordParam[_propIndex])
   {
      SetStatusText("No Word Parameters");
      return false;
   }
   auto it = m_wordParam[_propIndex]->m_wordMap.find(_word);
   if (it == m_wordParam[_propIndex]->m_wordMap.end())
   {
      SetStatusText("Word Not found");
      return false;
   }
   for (auto j : it->second)
   {
      _wordInfo.push_back(WordInfo(m_wordParam[_propIndex]->m_fileNameX[j.m_fnIdx], j.m_lineNo, j.m_line));
   }
   return !_wordInfo.empty();
}

void ProjectClass::ReplaceWordParm(class WordThreadParam* _parm)
{
   if (!m_wordParam[_parm->m_propIndex])
   {
      SetStatusText("No Word Parameters");
      return;
   }
   auto wordParam = m_wordParam[_parm->m_propIndex];
   uint32_t fnIdx;
   bool fnFound = false;
   for (fnIdx = 0; fnIdx < wordParam->m_fileNameX.size() && !fnFound; )
   {
      if (wordParam->m_fileNameX[fnIdx] == _parm->m_fileNameX[0])
      {
         fnFound = true;
      }
      else
      {
         fnIdx++;
      }
   }

   // First remove all entries with this file in the list
   if (fnFound)
   {
      for (auto p : _parm->m_wordMap)
      {
         for (auto& wordMap : wordParam->m_wordMap)
         {
            auto& info = wordMap.second;
            info.erase(std::remove_if(info.begin(),
                                      info.end(),
                                      [&] (auto& x) { return x.m_fnIdx < wordParam->m_fileNameX.size() && wordParam->m_fileNameX[x.m_fnIdx] == _parm->m_fileNameX[0]; }),
                                      info.end());
         }
      }
   }
   else
   {
      fnIdx = wordParam->m_fileNameX.size();
      wordParam->m_fileNameX.push_back(_parm->m_fileNameX[0]);
   }
   // Then insert them again
   for (auto& newWordParm : _parm->m_wordMap)
   {
      bool found = false;
      for (auto& wordParm : wordParam->m_wordMap)
      {
         if (newWordParm.first == wordParm.first)
         {
            found = true;
            for (auto& entry : newWordParm.second)
            {
               wordParm.second.push_back(WordMatchInfo(fnIdx, entry.m_lineNo, entry.m_line));
            }
         }
      }
      if (!found)
      {
         wordParam->m_wordMap[newWordParm.first] = newWordParm.second;
      }
   }
   //ToDo If a word is not used anymore, remove it from the list
}

void ProjectClass::StartWordThread()
{
   for (int propIndex = 1; propIndex < wainApp.gs.m_docProp.size(); propIndex++) // index 0 is *.*, we don't want that
   {
      std::vector<std::string> fileList;
      MakeProjectFileList(fileList, wainApp.gs.m_docProp[propIndex]->m_extensions);
      if (fileList.size())
      {
         ::StartWordThread(fileList, wainApp.gs.m_docProp[propIndex]->m_keyWordList, wainApp.gs.m_docProp[propIndex]->m_seps, wainApp.gs.m_docProp[propIndex]->m_ignoreCase, propIndex, false);
      }
   }
}
