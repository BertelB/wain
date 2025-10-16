//=============================================================================
// This source code file is a part of Wain.
// It implements tag_class and TagSetupDialogClass as defined in NavigatorDlg.h
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\wain.h"
#include ".\..\src\wainview.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\navigatorDlg.h"
#include ".\..\src\Project.h"
#include ".\..\src\TagList.h"
#include ".\..\src\Navigatorlist.h"
#include ".\..\src\DocProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool Split(std::string &aDest, std::string &aSrc, char aDelim)
{
   if(aSrc.empty())
      return false;
   std::string::size_type pos = aSrc.find(aDelim);
   aDest = aSrc.substr(0, pos);
   if(pos != std::string::npos)
     aSrc = aSrc.substr(pos + 1);
   else
     aSrc = "";

   return true;
}

bool Parse2(std::string& _str, ParseInfo& _parseInfo);

TagElemClass::TagElemClass(ParseInfo& _parseInfo, class ReadTagClass *_readTag)
{
   m_tag = _parseInfo.m_name;
   m_fileIdx = _readTag->GetFileList()->Add(_parseInfo.m_fn.c_str());
   m_lineNo = _parseInfo.m_lineNo - 1;
   switch(_parseInfo.m_type)
   {
   case 'p':
   case 'P':
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::PROTOTYPE_IDX)]++;
      m_indexType = TagIndexType::PROTOTYPE_IDX;
      break;
   case 'f':
   case 'F':
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::FUNCTION_IDX)]++;
      m_indexType = TagIndexType::FUNCTION_IDX;
      break;
   case 'c':
   case 'C':
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::CLASS_IDX)]++;
      m_indexType = TagIndexType::CLASS_IDX;
      if(_readTag->HasAddClassInfo())
      {
         _readTag->GetAddClassInfo()->SetClassInfo(m_tag, m_fileIdx, m_lineNo, false);
      }
      break;
   case 't':
   case 'T':
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::TYPE_IDX)]++;
      m_indexType = TagIndexType::TYPE_IDX;
      break;
   case 'e':
   case 'E':
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::ENUM_IDX)]++;
      m_indexType = TagIndexType::ENUM_IDX;
      break;
   case 'd':
   case 'D':
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::DEFINE_IDX)]++;
      m_indexType = TagIndexType::DEFINE_IDX;
      break;
   case 'v':
   case 'V':
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::VAR_IDX)]++;
      m_indexType = TagIndexType::VAR_IDX;
      break;
   case 'g': /* enumeration names */
   case 'G': /* enumeration names */
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::ENUM_NAME_IDX)]++;
      m_indexType = TagIndexType::ENUM_NAME_IDX;
      break;
   case 'm': /* class, struct, and union members */
   case 'M': /* class, struct, and union members */
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::MEMBER_IDX)]++;
      m_indexType = TagIndexType::MEMBER_IDX;
      break;
   case 's': /* structure names */
   case 'u': /* union names */
   case 'S': /* structure names */
   case 'U': /* union names */
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::STRUCT_IDX)]++;
      m_indexType = TagIndexType::STRUCT_IDX;
      break;
   case 'x': /* extern var */
   case 'X': /* extern var */
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::EXTERN_IDX)]++;
      m_indexType = TagIndexType::EXTERN_IDX;
      break;
   case 'l':
   case 'L':
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::LABEL_IDX)]++;
      m_indexType = TagIndexType::LABEL_IDX;
      break;
   case 'b':
   case 'B':
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::BLOCK_IDX)]++;
      m_indexType = TagIndexType::BLOCK_IDX;
      break;
   case 'i':
   case 'I':
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::INTERFACE_IDX)]++;
      m_indexType = TagIndexType::INTERFACE_IDX;
      break;
   case 'k':
   case 'K':
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::TYPE_COMP_IDX)]++;
      m_indexType = TagIndexType::TYPE_COMP_IDX;
      break;
   case 'n':
   case 'N':
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::NAME_SPACE_IDX)]++;
      m_indexType = TagIndexType::NAME_SPACE_IDX;
      break;
   default:
      _readTag->GetTagList()->m_countOffset.m_count[int(TagIndexType::UNKNOWN_IDX)]++;
      m_indexType = TagIndexType::UNKNOWN_IDX;
      break;
   }
}

UINT ThreadReadTagFile(LPVOID rp)
{
   ReadTagClass *ReadTag = new ReadTagClass;

   ReadTagParmClass *ReadParm = (ReadTagParmClass *)rp;

   ReadTag->SetFileList(new TagFileListClass);
   ReadTag->SetTagList(new TagListClass);
   MSG msg;

   if(ReadParm->m_command.size())
   { // We have to build the tag file
      STARTUPINFO si =
      {
        sizeof(STARTUPINFO),
        NULL,
        NULL,
        "Wain Tags",
        0, 0, 0, 0,  /* Ignore size */
        0, 0,
        0,
        STARTF_USESHOWWINDOW,
        SW_HIDE,
        0,
        0,
        0,
        0,
        0
      };

      PROCESS_INFORMATION pi;
      if(!CreateProcess(NULL,
                        (char *)ReadParm->m_command.c_str(),
                        NULL,
                        NULL,
                        FALSE,
                        CREATE_NEW_CONSOLE | ReadParm->m_priority,
                        NULL,
                        NULL,
                        &si,
                        &pi))
      { // Could not start the tag program, deallocate and return
        delete ReadTag;
        wainApp.PostThreadMessage(ReadParm->m_readTagsMsgId, NULL, (LPARAM )ReadParm->m_nr);
        delete ReadParm;
        return 1;
      }
      DWORD exit_code;
      do
      {
         Sleep(500);
         if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
         { // We are told to quit, so dealocate and return.
            delete ReadTag;
            wainApp.PostThreadMessage(ReadParm->m_readTagsMsgId, NULL, (LPARAM )ReadParm->m_nr);
            delete ReadParm;
            return 2;
         }
         else
           GetExitCodeProcess(pi.hProcess, &exit_code);
      }
      while(exit_code == STILL_ACTIVE);
   }
   // FILE *tag_file;
   // if((tag_file = fopen(ReadParm->m_fileName.c_str(), "rt")) == NULL)
   std::ifstream tagFile(ReadParm->m_fileName);
   if (!tagFile)
   { // Unable to open the output, deallocate and return
      delete ReadTag;
      wainApp.PostThreadMessage(ReadParm->m_readTagsMsgId, NULL, (LPARAM )ReadParm->m_nr);
      delete ReadParm;
      return 3;
   }
   std::string line;
   unsigned int LoopCount = 0;
   if(ReadParm->m_viewNr)
   {
      ReadTag->SetAddClassInfo(new AddClassInfoListClass);
   }
   while (getline(tagFile, line))
   {
      if(!line.empty() && line[0] != '!') // A '!' as the first char denotes a comment line
      {
         if(++LoopCount%256 == 0 && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
         { // We are told to quit, so dealocate and return.
            delete ReadTag;
            wainApp.PostThreadMessage(ReadParm->m_readTagsMsgId, NULL, (LPARAM )ReadParm->m_nr);
            delete ReadParm;
            return 4;
         }
         ParseInfo parseInfo;
         if (Parse2(line, parseInfo))
         {
            TagElemClass *tag = new TagElemClass(parseInfo, ReadTag);
            if(ReadParm->m_viewNr)
            {
               AddClassElementInfoClass *node = 0;
               bool isStruct = false;
               for (auto i = parseInfo.m_addInfo.begin(); i != parseInfo.m_addInfo.end(); i++)
               {
                  switch(i->m_infoType)
                  {
                  case ParseInfo::Inherit:
                     ReadTag->GetAddClassInfo()->Add(tag->m_tag, i->m_info, TagIndexType::INHERITANCE_IDX, tag->m_fileIdx, tag->m_lineNo, isStruct);
                     break;
                  case ParseInfo::Class:
                     node = ReadTag->GetAddClassInfo()->Add(i->m_info, tag->m_tag, tag->m_indexType, tag->m_fileIdx, tag->m_lineNo, false);
                     break;
                  case ParseInfo::Struct:
                     isStruct = true;
                     node = ReadTag->GetAddClassInfo()->Add(i->m_info, tag->m_tag, tag->m_indexType, tag->m_fileIdx, tag->m_lineNo, true);
                     break;
                  case ParseInfo::Signature:
                     if(node)
                        node->SetSignature(i->m_info);
                     tag->m_signature = i->m_info;
                     break;
                  case ParseInfo::Typeref:
                     ReadTag->HandleTyperef(tag->m_tag.c_str(), i->m_info.c_str());
                     break;
                  case ParseInfo::Typename:
                     tag->m_signature = i->m_info;
                     break;
                  default:
                     {
                        char txt[1024];
                        sprintf(txt, "Hello: %u\n", i->m_infoType);
                        printf(txt);
                     }
                     break;
                  }
               }
            }
            ReadTag->GetTagList()->Add(tag);
         }
      }
   }
   ReadTag->PostProcessTyperef();
   if(ReadParm && ReadParm->m_tree)
   {
      if(ReadTag->GetAddClassInfo())
         ReadTag->GetAddClassInfo()->Sort();
      ReadTag->SetTree(ReadParm->m_tree);
      if (ReadTag->GetAddClassInfo())
         ReadTag->GetAddClassInfo()->InitTree(ReadParm->m_tree);
   }
   ReadTag->GetTagList()->PostProcess();

   wainApp.PostThreadMessage(ReadParm->m_readTagsMsgId, (WPARAM )ReadTag, (LPARAM )ReadParm->m_nr);
   delete ReadParm;
   return 0;
}

UINT ThreadAutoRebuildTag(LPVOID parm)
{
  AutoRebuildTagVector *ReadParm = (AutoRebuildTagVector *)parm;
  ASSERT(parm);
  AutoRebuildTagVector::size_type i;
  for(i = 0; i < ReadParm->size(); i++)
  {
    STARTUPINFO si =
    {
      sizeof(STARTUPINFO),
      NULL,
      NULL,
      "Wain Tags",
      0, 0, 0, 0,  /* Ignore size */
      0, 0,
      0,
      STARTF_USESHOWWINDOW,
      SW_HIDE,
      0,
      0,
      0,
      0,
      0
    };

    PROCESS_INFORMATION pi;
    if(!CreateProcess(NULL, (char *)(*ReadParm)[i]->m_command.c_str(), NULL, NULL, FALSE, CREATE_NEW_CONSOLE | IDLE_PRIORITY_CLASS, NULL, NULL, &si, &pi  ))
    {
      return 2;
    }
    DWORD exit_code = STILL_ACTIVE;
    while(exit_code == STILL_ACTIVE)
    {
      GetExitCodeProcess(pi.hProcess, &exit_code);
      if(exit_code == STILL_ACTIVE)
        Sleep(250);
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    delete (*ReadParm)[i];
  }
  delete ReadParm;
  return 0;
}

bool GlobalTagClass::GetTagList(GetTagListClass *aTagList, BOOL aJustClasses)
{
   if(m_tagList && m_tagList->GetNofTags())
   {
      TagElemClass ToFind;
      ToFind.m_tag = aTagList->m_word;
      TagElemFindClass TagCompare;
      for(char idx = 0; idx < char((aJustClasses ? 1 : int(TagIndexType::NOF_INDEXES))); idx++)
      {
         if(aTagList->m_types & (1 << idx))
         {
            ToFind.m_indexType = TagIndexType(idx);
            TagListClass::TagVectorIterator It, Start, End;
            Start =  m_tagList->m_tagVector.begin() + m_tagList->m_countOffset.m_offset[idx];
            End = m_tagList->m_tagVector.begin() + m_tagList->m_countOffset.m_offset[idx] + m_tagList->m_countOffset.m_count[idx];
            It = std::lower_bound(Start, End, ToFind, TagCompare);

            while(It != m_tagList->m_tagVector.end() && It != End)
            {
               if((*It)->m_tag == ToFind.m_tag)
               {
                  aTagList->Add(m_fileList->GetFullName((*It)->m_fileIdx),
                                m_fileList->GetShortName((*It)->m_fileIdx),
                                (*It)->m_lineNo,
                                (*It)->m_tag.c_str(),
                                (*It)->m_indexType,
                                (*It)->m_signature.c_str());
               }
               ++It;
            } // Todo, fix double entries
         }
      }
      aTagList->Sort();
      return TRUE;
   }

   return FALSE;
}

bool GlobalTagClass::CheckTag(char *word)
{
  if(m_tagList && m_tagList->GetNofTags())
  {
    return m_tagList->IsTag(word) ? true : false;
  }

  return false;
}

int GlobalTagClass::SetTag(const char *name)
{
  if(!m_tagList)
    return -1;
  TagElemClass Tag;
  if(!m_tagList->Find(Tag, name))
     return -1;
  return Tag.m_nr;
}

void TagClass::HandleTagsRead(ReadTagClass *aReadTag)
{
   if(m_tagList)
   {
      if(m_dlg->m_navBarState == m_state)
         Clear(true);
      else
        Clear(false);
   }
   if(aReadTag)
   {
      m_tagList = aReadTag->GetTagList();
      m_fileList = aReadTag->GetFileList();
      aReadTag->SetFileList(0);
      aReadTag->SetTagList(0);
      UpdateTagList();
      if(aReadTag->GetTree())
      {
         ASSERT(::IsWindow(aReadTag->GetTree()->m_hWnd));
         m_dlg->SetClassList(aReadTag->GetAddClassInfo(), aReadTag->GetFileList(), aReadTag->GetTree());
         aReadTag->SetAddClassInfo(0);
         aReadTag->SetTree(0);
      }
      aReadTag->SetFileList(0);
      aReadTag->SetTagList(0);

      delete aReadTag;
      SetStatusText("Tags has been read");
  }
  else
  {
     SetStatusText("Failed to build tags");
  }

  GetMf()->UpdateViews();
}

void TagClass::ReadTagFile(const char *aCommand, const char *aFileName, bool aRebuild, bool aAutoBuild)
{
   DWORD ExitCode = ~STILL_ACTIVE;
   if(m_tagThread)
   {
      ::PostThreadMessage(m_tagThread->m_nThreadID, IDB_THREAD_MSG, 0, 0);
      m_tagThread->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
      unsigned int Counter = 0;
      while(::GetExitCodeThread(m_tagThread->m_hThread, &ExitCode) && ExitCode == STILL_ACTIVE && Counter++ < 50)
      {
         Sleep(100);
      }
      if(Counter >= 50)
      { // OH-no, a timeout, Terminate the thread
         TerminateThread(m_tagThread->m_hThread, 0);
      }
      delete m_tagThread;
      m_tagThread = 0;
   }
   ReadTagParmClass *ReadParm = new ReadTagParmClass;

   if(aAutoBuild)
     ReadParm->m_priority = IDLE_PRIORITY_CLASS;
   else
     ReadParm->m_priority = NORMAL_PRIORITY_CLASS;

   if(aRebuild)
     ReadParm->m_command = aCommand;

   ReadParm->m_fileName = aFileName;
   ReadParm->m_nr = m_readNr++;
   ReadParm->m_viewNr = m_viewNr;
   ReadParm->m_tree = 0;
   if(m_viewNr != 0)
   {
      if(::IsWindow(m_dlg->m_hWnd))
      {
         ReadParm->m_tree = new AddClassInfoTreeCtrlClass(m_dlg);
         RECT r = {5, 5, 10, 10};
         ReadParm->m_tree->CreateEx(WS_EX_CLIENTEDGE,
                                  // WC_TREEVIEW,
                                  // 0,
                                  TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_DISABLEDRAGDROP | TVS_INFOTIP | WS_CHILD | WS_TABSTOP | TVS_SHOWSELALWAYS,
                                  r,
                                  m_dlg,
                                  m_viewNr);
         m_viewNr++;
         if(m_viewNr > IDB_LIST_TREE_7)
            m_viewNr = IDB_LIST_TREE_1;
      }
      else
      {
         ReadParm->m_tree = 0;
      }
   }
   ReadParm->m_readTagsMsgId = m_readTagsMsgId;
   m_tagThread = AfxBeginThread(ThreadReadTagFile, ReadParm, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
   m_tagThread->m_bAutoDelete = FALSE;
   m_tagThread->ResumeThread();
}

void GlobalTagClass::UpdateTagList(void)
{
  if(m_dlg->m_navBarState == m_state)
  {
    if(m_tagList)
      m_dlg->m_navigatorList->Setup(m_tagList->GetNofTags(), 3, m_dlg->m_tagImageList.m_hImageList, wainApp.gs.m_widthRatio);
    else
      m_dlg->m_navigatorList->Setup(0, 3, m_dlg->m_tagImageList.m_hImageList, wainApp.gs.m_widthRatio);
  }
}

void CurrentTagClass::UpdateTagList(void)
{
  if(m_dlg->m_navBarState == m_state)
  {
    if(m_tagList)
      m_dlg->m_navigatorList->Setup(m_tagList->GetNofTags(), 2, m_dlg->m_tagImageList.m_hImageList);
    else
      m_dlg->m_navigatorList->Setup(0, 2, m_dlg->m_tagImageList.m_hImageList);
  }
}

TagClass::~TagClass()
{
   if(m_tagThread)
   {
      DWORD exit_code = ~STILL_ACTIVE;
      ::GetExitCodeThread(m_tagThread->m_hThread, &exit_code);
      if(exit_code == STILL_ACTIVE)
      { // The thread is still running, raise priority and wait for it to end
         ::PostThreadMessage(m_tagThread->m_nThreadID, IDB_THREAD_MSG, 0, 0);
         m_tagThread->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
         while(exit_code == STILL_ACTIVE)
         {
            Sleep(100);
            ::GetExitCodeThread(m_tagThread->m_hThread, &exit_code);
         }
      }
      delete m_tagThread;
      m_tagThread = NULL;
   }
   Clear(false);
}

void TagClass::Clear(bool aSetup)
{
  delete m_tagList;
  m_tagList = 0;
  delete m_fileList;
  m_fileList = 0;
  if(aSetup)
    m_dlg->m_navigatorList->Setup(0, 3, m_dlg->m_tagImageList.m_hImageList, wainApp.gs.m_widthRatio);
}

/* Implementation of TagSetupDialogClass */
BEGIN_MESSAGE_MAP(TagSetupDialogClass , CDialog)
  ON_COMMAND(IDC_TS_LOOKUP, LookupTagProg)
END_MESSAGE_MAP();

IMPLEMENT_DYNAMIC(TagSetupDialogClass , CDialog)

TagSetupDialogClass ::TagSetupDialogClass (BOOL global, CWnd *pParent ) :
   DialogBaseClass(TagSetupDialogClass::IDD, pParent)
{
  m_glob = global;
}

TagSetupDialogClass ::~TagSetupDialogClass ()
{

}

BOOL TagSetupDialogClass ::OnInitDialog()
{
  if(!m_glob)
  {
    CEdit *e = (CEdit *)GetDlgItem(IDC_TS_FILE);
    if(e)
      e->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE);
    SetDlgItemText(IDC_TS_FILES, "");
    CButton *b = (CButton *)GetDlgItem(IDC_TS_TRACK);
    ASSERT(b);
    b->ModifyStyle(WS_VISIBLE, WS_DISABLED, SWP_NOACTIVATE);
  }
  CDialog::OnInitDialog();
  return TRUE;
}

void TagSetupDialogClass ::DoDataExchange(CDataExchange *dx)
{
  CDialog::DoDataExchange(dx);
  DdxString(dx, IDC_TS_PROG, m_prog);
  DdxString(dx, IDC_TS_OPT,  m_options);
  DdxString(dx, IDC_TS_FILE, m_files);
  DdxString(dx, IDC_TS_TAG_FILE, m_tagFile);
  DDX_Check(dx, IDC_TS_TRACK, m_track);
}

void TagSetupDialogClass ::LookupTagProg(void)
{
  MyFileDialogClass fd(FD_OPEN | FD_LAST_PATH | FD_SET_LAST_PATH, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY, "exe", NULL, "Programs (*.exe;*.cmd;*.bat)\0*.exe;*.cmd;*.bat\0All files (*.*)\0*.*\0\0");
  if(fd.DoModal() == IDOK)
  {
    m_prog = fd.GetPathName();
    CEdit *e = (CEdit *)GetDlgItem(IDC_TS_PROG);
    if(e)
      e->SetWindowText(m_prog.c_str());
  }
}

/* Implementation of TagSetup_prop_class */
BEGIN_MESSAGE_MAP(TagSetupPropClass, CPropertyPage)
  ON_COMMAND(IDC_TS_LOOKUP, LookupTagProg)
END_MESSAGE_MAP();

IMPLEMENT_DYNAMIC(TagSetupPropClass, CPropertyPage)

TagSetupPropClass::TagSetupPropClass(DocPropClass *doc_prop) : PropertyPageBaseClass(TagSetupPropClass::IDD)
{
  m_prop = doc_prop;
}

TagSetupPropClass::~TagSetupPropClass()
{

}

BOOL TagSetupPropClass::OnInitDialog()
{
  m_prog = m_prop->m_tagProgram;
  m_options = m_prop->m_tagOptions;
  m_tagFile = m_prop->m_tagFile;

  UpdateData(FALSE);
  return CPropertyPage::OnInitDialog();
}

BOOL TagSetupPropClass::OnKillActive(void)
{
  UpdateData(TRUE);
  m_prop->m_tagProgram, m_prog;
  m_prop->m_tagOptions, m_options;
  m_prop->m_tagFile, m_tagFile;

  return CPropertyPage::OnKillActive();
}

void TagSetupPropClass::DoDataExchange(CDataExchange *dx)
{
  CPropertyPage::DoDataExchange(dx);
  DdxString(dx, IDC_TS_PROG, m_prog);
  DdxString(dx, IDC_TS_OPT,  m_options);
  DdxString(dx, IDC_TS_TAG_FILE, m_tagFile);
  DdxCheck(dx, IDC_TS_COLOR, m_prop->m_useTagColor);
}

void TagSetupPropClass::LookupTagProg(void)
{
  MyFileDialogClass fd(FD_OPEN | FD_LAST_PATH | FD_SET_LAST_PATH, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY, "exe", NULL, "Programs (*.exe;*.cmd;*.bat) \0*.exe;*.cmd;*.bat\0All files (*.*)\0*.*\0\0");
  if(fd.DoModal() == IDOK)
  {
     m_prog = fd.GetPathName();
     CEdit *e = (CEdit *)GetDlgItem(IDC_TS_PROG);
     if(e)
        e->SetWindowText(m_prog.c_str());
  }
}

void DeallocateTagsRead(WPARAM w)
{ // Called if the tag thread sends messages after the main frame is closed.
  ReadTagClass *rs = (ReadTagClass *)w;
  delete rs;
}

TagFileListClass::SizeType TagFileListClass::Add(const char *aFullName)
{
  std::vector<std::string>::iterator it = std::find(m_fullName.begin(), m_fullName.end(), std::string(aFullName));
  if(it != m_fullName.end())
     return it - m_fullName.begin();

  m_fullName.push_back(aFullName);

  char sn[MAX_PATH];
  MySplitPath(aFullName, SP_FILE | SP_EXT, sn);
  m_shortName.push_back(sn);

  return m_fullName.size() - 1;
}

void TagListClass::PostProcess(void)
{
   TagVectorSizeType i;
   for(i = 0; i < m_tagVector.size(); i++)
   {
      if(strlen(m_tagVector[i]->m_tag.c_str()) == 0)
      {
         delete m_tagVector[i];
         for( ; i < m_tagVector.size() - 1; i++)
            m_tagVector[i] = m_tagVector[i + 1];
         m_tagVector.pop_back();
         PostProcess();
         return;
      }
   }
   TagElemSortClass Compare;
   std::sort(m_tagVector.begin(), m_tagVector.end(), Compare);

   for(i = 0; i < m_tagVector.size(); i++) // Must be Done after sorting.
      m_tagVector[i]->m_nr = i;
   int idx;
   for(idx = 0, i = 0; idx < int(TagIndexType::NOF_INDEXES); idx++)
      m_countOffset.m_count[idx] = 0;
   for(i = 0; i < m_tagVector.size(); i++)
      m_countOffset.m_count[int(m_tagVector[i]->m_indexType)]++;
   for(idx = 0, i = 0; idx < int(TagIndexType::NOF_INDEXES); i += m_countOffset.m_count[idx], idx++)
      m_countOffset.m_offset[idx] = i;
}

bool TagListClass::Find(TagElemClass& _tag, const char* _word)
{
   TagElemClass ToFind;
   ToFind.m_tag = _word;
   TagElemFindClass TagCompare;
    for(char idx = 0; idx < int(TagIndexType::NOF_INDEXES); idx++)
   {
      ToFind.m_indexType = TagIndexType(idx);
      TagListClass::TagVectorIterator It, Start, End;
      Start = m_tagVector.begin() + m_countOffset.m_offset[idx];
      End =   m_tagVector.begin() + m_countOffset.m_offset[idx] + m_countOffset.m_count[idx];
      It = std::lower_bound(Start, End, ToFind, TagCompare);
      if(It != m_tagVector.end() && It != End && (*It)->m_tag == _word)
      {
         _tag = *It;
         return true;
      }
   }
   return false;
}

bool TagListClass::FindClass(TagElemClass& _tag, const char* _word)
{
   TagElemClass ToFind;
   std::string tagWord(_word);
   std::string::size_type pos = tagWord.find_first_of(" \t");
   if (pos != std::string::npos)
   {
      tagWord = tagWord.substr(0, pos);
   }
   ToFind.m_tag = tagWord;

   TagElemFindClass TagCompare;
   TagIndexType idxToCheck[2] = {TagIndexType::CLASS_IDX, TagIndexType::STRUCT_IDX};
   for(char x = 0; x < 2; x++)
   {
      TagIndexType idx = idxToCheck[x];
      ToFind.m_indexType = idx;
      TagListClass::TagVectorIterator It, Start, End;
      Start = m_tagVector.begin() + m_countOffset.m_offset[int(idx)];
      End =   m_tagVector.begin() + m_countOffset.m_offset[int(idx)] + m_countOffset.m_count[int(idx)];
      It = std::lower_bound(Start, End, ToFind, TagCompare);
      if(It != m_tagVector.end() && It != End && (*It)->m_tag == tagWord)
      {
         _tag = *It;
         return true;
      }
   }
   return false;
}

bool TagListClass::IsTag(const char *aWord)
{
   TagElemFindClass TagCompare;
   TagElemClass ToFind;
   ToFind.m_tag = aWord;

   for(int idx = 0; idx < int(TagIndexType::NOF_INDEXES); idx++)
   {
      TagListClass::TagVectorIterator Start, End, it;
      if(m_countOffset.m_offset[idx] >= m_tagVector.size())
      {
         Start = m_tagVector.end();
      }
      else
      {
         Start = m_tagVector.begin() + m_countOffset.m_offset[idx];
      }
      if(m_countOffset.m_offset[idx] + m_countOffset.m_count[idx] >= m_tagVector.size())
      {
          End = m_tagVector.end();
      }
      else
      {
         End = m_tagVector.begin() + m_countOffset.m_offset[idx] + m_countOffset.m_count[idx];
      }

      if((it = std::lower_bound(Start, End, ToFind, TagCompare)) != End)
      {
         End = std::upper_bound(Start, End, ToFind, TagCompare);
         while(it != m_tagVector.end() && it != End)
         {
            if((*it)->m_tag == aWord)
            {
               return true;
            }
            ++it;
         }
      }
   }
   return false;
}
#if 0
GetTagElemClass* TagListClass::GetClass(const char *aWord)
{
   TagElemFindClass TagCompare;
   TagElemClass ToFind;
   ToFind.m_tag = aWord;
   for(char x = 0; x < 2; x++)
   {
      char idx = idxToCheck[x];
      TagListClass::TagVectorIterator Start, End, it;
      if(m_countOffset.m_offset[idx] >= m_tagVector.size())
      {
         Start = m_tagVector.end();
      }
      else
      {
         Start = m_tagVector.begin() + m_countOffset.m_offset[idx];
      }
      if(m_countOffset.m_offset[idx] + m_countOffset.m_count[idx] >= m_tagVector.size())
      {
          End = m_tagVector.end();
      }
      else
      {
         End = m_tagVector.begin() + m_countOffset.m_offset[idx] + m_countOffset.m_count[idx];
      }

      if((it = std::lower_bound(Start, End, ToFind, TagCompare)) != End)
      {
         End = std::upper_bound(Start, End, ToFind, TagCompare);
         while(it != m_tagVector.end() && it != End)
         {
            if((*it)->m_tag == aWord)
            {
               return new GetTagElemClass(m_fileList->GetFullName((*it)->m_fileIdx),
                                          m_fileList->GetShortName((*it)->m_fileIdx),
                                          (*it)->m_lineNo,
                                          (*it)->m_tag,
                                          (*it)->m_indexType,
                                          (*it)->m_signature);
            }
            ++it;
         }
      }
   }
   return 0;
}
#endif
void ReadTagClass::HandleTyperef(const char* aNewName, const char* aOldName)
{
   if(m_tagList)
   {
      TagListClass::TagVectorIterator it;
      for(it = m_tagList->m_tagVector.begin(); it != m_tagList->m_tagVector.end(); ++it)
      {
         if((*it)->m_tag == aOldName)
         {
            (*it)->m_tag = aNewName;
         }
      }
   }
   if(m_addClassInfo)
   {
      AddClassInfoListClass::ListClass::iterator it;
      for(it = m_addClassInfo->m_list.begin(); it != m_addClassInfo->m_list.end(); ++it)
      {
         if((*it)->m_className == aOldName)
         {
            (*it)->m_className = aNewName;
         }
      }
   }
   m_typerefList.push_back(aOldName);
}

void TagListClass::Remove(std::string& aName)
{
   TagVectorIterator it;
   for(it = m_tagVector.begin(); it != m_tagVector.end(); ++it)
   {
      if((*it)->m_tag == aName)
      {
         delete *it;
         m_tagVector.erase(it);
         return;
      }
   }

}

void ReadTagClass::PostProcessTyperef()
{
    return; //!!
   if(!m_addClassInfo)
   {
      return;
   }
   std::list<std::string >::iterator it;
   for(it = m_typerefList.begin(); it != m_typerefList.end(); it++)
   {
      m_tagList->Remove(*it);
      m_addClassInfo->Remove(*it);
   }
}

void Trim(std::string& _str)
{
   size_t last = _str.find_last_not_of(' ');
   if (last != std::string::npos && last + 1 != _str.size())
      _str = _str.substr(0, last + 1);
}

bool Parse2(std::string& _str, ParseInfo& _parseInfo)
{
   std::string tmp;
   if (!Split(_parseInfo.m_name, _str, '\t'))
      return false;
   // Trim(_parseInfo.name);
   uint32_t pos;
   if (!Split(_parseInfo.m_fn, _str, '\t'))
      return false;
   while((pos = _parseInfo.m_fn.find('/')) != std::string::npos)
   {
      _parseInfo.m_fn[pos] = '\\';
   }
   if (!Split(tmp, _str, '\t'))
      return false;
   _parseInfo.m_lineNo = strtol(tmp.c_str(), 0, 0);
   if (!Split(tmp, _str, '\t'))
      return false;
   _parseInfo.m_type = tmp[0];
   while (Split(tmp, _str, '\t'))
   {
      std::string s1;
      if (Split(s1, tmp, ':'))
      {
         if (s1 == "class")
         {
            ParseInfo::AddInfoSet addInfoSet;
            addInfoSet.m_infoType = ParseInfo::Class;
            addInfoSet.m_info = tmp;
            _parseInfo.m_addInfo.push_back(addInfoSet);
         }
         else if (s1 == "struct")
         {
            ParseInfo::AddInfoSet addInfoSet;
            addInfoSet.m_infoType = ParseInfo::Struct;
            addInfoSet.m_info = tmp;
            _parseInfo.m_addInfo.push_back(addInfoSet);
         }
         else if (s1 == "signature")
         {
            ParseInfo::AddInfoSet addInfoSet;
            addInfoSet.m_infoType = ParseInfo::Signature;
            addInfoSet.m_info = tmp;
            _parseInfo.m_addInfo.push_back(addInfoSet);
         }
         else if (s1 == "inherits")
         {
            ParseInfo::AddInfoSet addInfoSet;
            addInfoSet.m_infoType = ParseInfo::Inherit;
            addInfoSet.m_info = tmp;
            _parseInfo.m_addInfo.push_back(addInfoSet);
         }
         else if (s1 == "typeref")
         {
            std::string s2;
            if (Split(s2, tmp, ':'))
            {
               if ((s2 == "class" || s2 == "struct") && tmp.substr(0, 6) == "__anon")
               {
                  ParseInfo::AddInfoSet addInfoSet;
                  addInfoSet.m_infoType = ParseInfo::Typeref;
                  addInfoSet.m_info = tmp;
                  _parseInfo.m_addInfo.push_back(addInfoSet);
               }
               else if (s2 == "typename" || s2 == "class" || s2 == "struct")
               {
                  ParseInfo::AddInfoSet addInfoSet;
                  addInfoSet.m_infoType = ParseInfo::Typename;
                  addInfoSet.m_info = tmp;
                  _parseInfo.m_addInfo.push_back(addInfoSet);
               }
            }
         }
         #if 0
         else if (s1 == "file")
         {
            // Ignore for now
         }
         else if (s1 == "enum")
         {
            // Ignore for now
         }
         else if (s1 == "function")
         {
            // Ignore for now
         }
         else
         #endif
      }
   }
   return true;
}

//-- EOF
