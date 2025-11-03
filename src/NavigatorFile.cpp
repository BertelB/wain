#include ".\..\src\stdafx.h"
#include ".\..\src\wain.h"
#include ".\..\src\wainview.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\Project.h"
#include ".\..\src\TagList.h"
#include ".\..\src\Navigatorlist.h"
#include ".\..\src\childfrm.h"
#include ".\..\src\waindoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* Implementation of FileList_class */
void NavFileList::UpdateFileList()
{
  if(m_dlg->m_navBarState == NavBarStateFiles)
    UpdateFileView();
}

void NavFileList::RemoveFile(void)
{
  if(m_dlg->m_navBarState == NavBarStateFiles)
    UpdateFileView();
}

void NavFileList::UpdateFileView(void)
{
  int k;
  ViewListElem *elem;
  for(k = 0, elem = NULL; (elem = GetMf()->m_viewList.GetRankNext(elem)) != NULL; k++)
    /* Nothing */;
  if(!m_dlg->m_sysImageList)
  {
    SHFILEINFO sh_FileInfo;
    m_dlg->m_sysImageList = (HIMAGELIST )SHGetFileInfo("c:\\", 0, &sh_FileInfo, sizeof(sh_FileInfo), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
  }
  if(::IsWindow(m_dlg->m_hWnd))
  {
    m_dlg->m_navigatorList->Setup(k, 3, m_dlg->m_sysImageList, -30);
  }
}

/* Implementation of dir_class */
void HdDirClass::UpdateDir(void)
{
  CWaitCursor wait;
  Clear();
  CFileFind finder;

  AddSlash(m_dirPath);

  std::string temp(m_dirPath);
  temp += "*.*";
  CComboBox *cb = (CComboBox *)m_dlg->GetDlgItem(IDB_COMBO);
  cb->ResetContent();
  cb->Dir(DDL_DRIVES | 0x8000, "*.*");
  cb->AddString("[-/-]");
  char d_str[10];
  if(temp[0] == '\\')
    sprintf(d_str, "[-%c-]", '/');
  else
    sprintf(d_str, "[-%c-]", toupper(unsigned(temp[0])));
  cb->SelectString(-1, d_str);

  BOOL working = finder.FindFile(temp.c_str());
  if(!working)
  {
    CString error_msg;
    char *msg_buf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                  (char *)&msg_buf, 0, NULL);
    error_msg = "Error in FindFile()\r\n";
    error_msg += "Windows reports the following error:\r\n";
    error_msg += msg_buf;
    WainMessageBox(GetMf(), error_msg, IDC_MSG_OK, IDI_WARNING_ICO);
    LocalFree(msg_buf);
    /* Back up to the previous dir */
    temp = temp.substr(0, temp.size() - 4); // remove \*.*

    std::string::size_type pos = temp.find_last_of("\\/");
    if(pos != std::string::npos)
    {
       temp = temp.substr(0, pos);
    }
    if(temp.size() < 3)
      m_dirPath = "c:\\";
    else
      m_dirPath = temp;
    UpdateDir();  /* Call me again */
    return;       /* And bail out */

  }

  BOOL is_dot;
  BOOL got_dotdot = FALSE;

  ThreadGetIconParmClass *req = new ThreadGetIconParmClass;
  req->m_index = ++m_threadCounter;
  m_dirInfoList.clear();
  std::string PathName;
  while(working)
  {
    working = finder.FindNextFile();
    is_dot = FALSE;
    PathName = finder.GetFileName();
    if(finder.IsDirectory())
    {
      if(PathName != ".")
      {
        if(PathName == "..")
          got_dotdot = TRUE;
        temp = "[" + PathName + "]";
        AddEntry(temp);
      }
      else
        is_dot = TRUE;
    }
    else
    {
      AddEntry(PathName);
    }
    if(!is_dot)
    {
      temp = m_dirPath;
      if(PathName != "..")
      {
        temp += PathName;
        if(finder.IsDirectory())
          temp += "\\";
      }
      req->m_nameList.push_back(temp);
    }
  }
  if(!got_dotdot && m_dirPath.size() > 1 && m_dirPath.find(':') == std::string::npos)
  {
    AddEntry("[..]");
  }
  finder.Close();

  ThreadGetIconNameListCompareClass Compare;
  std::sort(req->m_nameList.begin(), req->m_nameList.end(), Compare);
  std::sort(m_dirInfoList.begin(), m_dirInfoList.end());

  strcpy(wainApp.gs.m_lastUsedDir, m_dirPath.c_str());
  m_dlg->m_navigatorList->Setup(m_dirInfoList.size(), 2, m_dlg->m_sysImageList);

  /* It's time to get the icons */
  AfxBeginThread(ThreadGetIcon, req, THREAD_PRIORITY_LOWEST);
}

void FtpDirClass::UpdateDir(void)
{
  CWaitCursor wait;
  Clear();
  if(!m_dlg->m_ftpConnection)
  {
    m_dlg->m_navigatorList->Setup(0, 2, m_dlg->m_sysImageList);
    return;
  }
  std::string temp;
  std::string PathName;

  CFtpFileFind finder(m_dlg->m_ftpConnection);

  AddSlash(m_dirPath);

  temp = m_dirPath;
  temp += "*.*";

  BOOL working = finder.FindFile(temp.c_str());

  BOOL got_dotdot = FALSE;
  m_dirInfoList.clear();
  while(working)
  {
    working = finder.FindNextFile();
    PathName = finder.GetFileName();
    if(finder.IsDirectory())
    {
      if(PathName != ".")
      {
        if(PathName == "..")
          got_dotdot = TRUE;
        temp = "[" + PathName + "]";
        AddEntry(temp);
      }
    }
    else
    {
      AddEntry(PathName);
    }
  }
  if(!got_dotdot && m_dirPath.size() > 1 && m_dirPath.find(':') == std::string::npos)
  {
    AddEntry("[..]");
  }
  finder.Close();
  // DirInfoCompareClass Compare;
  std::sort(m_dirInfoList.begin(), m_dirInfoList.end());
  m_dlg->m_navigatorList->Setup(m_dirInfoList.size(), 2, m_dlg->m_sysImageList);
}

bool HdDirClass::GetFullPath(int aSel, std::string &aName)
{
  if(aSel < m_dirInfoList.size())
  {
    aName = m_dirPath;
    AddSlash(aName);
    aName += m_dirInfoList[aSel].m_name;
    return true;
  }
  return false;
}

HdDirClass::HdDirClass() : DirClass()
{
  if(wainApp.gs.m_lastUsedDir[0] == '\\')
  { /* Don't start up with net as default dir */
     char dp[1024];
    _getcwd(dp, sizeof(dp));
    m_dirPath = dp;
  }
  else
    m_dirPath = wainApp.gs.m_lastUsedDir;

  AddSlash(m_dirPath);
  m_threadCounter = UINT_MAX/2;

  for(int i = 0; i < MAX_NET_LEVEL; i++)
  {
    m_netResource[i] = NULL;
    m_nofNetResource[i] = 0;
  }
  m_netLevel = 0;
  m_inNet = FALSE;
  m_wasNet = FALSE;
}

HdDirClass::~HdDirClass()
{
  for(int i = 0; i < MAX_NET_LEVEL; i++)
    if(m_netResource[i])
    {
      free(m_netResource[i]);
      m_netResource[i] = NULL;
    }

  DirClass::Clear();
}

FtpDirClass::FtpDirClass() : DirClass()
{
  m_dirPath = "";
}

void DirClass::Clear(void)
{
   m_dirInfoList.clear();
}

void HdDirClass::DoSelect(int aSelection)
{
  if(aSelection >= m_dirInfoList.size())
    return;
  std::string sel(m_dirInfoList[aSelection].m_name);

  bool handle_net = false;

  if(m_inNet)
    handle_net = CheckNet(aSelection);
  if(!m_inNet || !handle_net)
  {
    if(sel[0] == '[' || (sel[0] == '\\' && sel[1] == '\\'))
    {
      if(sel[1] == '.')
      {
        // if(sel.find_first_of("\\/") == std::string::npos) /* If we have a subst'ed drive or a net drive we might have a [..] in the root */
        //    return;
        if(RemoveLastPath(m_dirPath))
        {
           UpdateDir();
        }
      }
      else
      {
        std::string temp;
        if(sel[0] == '\\' && sel[1] == '\\')
        {
          m_dirPath = sel;
        }
        else
        {
           AddSlash(m_dirPath);
           sel = sel.substr(1, sel.size()- 2);

           temp = m_dirPath + sel + "\\";

           if(access(temp.c_str(), 4)) // No access rights
           {
             SetStatusText("Unable to change dir");
             return;
           }
           m_dirPath += sel;
           AddSlash(m_dirPath);
        }
        UpdateDir();
      }
      SetStatusText(m_dirPath.c_str());
    }
    else
    {
      AddSlash(m_dirPath);
      std::string str = m_dirPath + sel;
      wainApp.OpenDocument(str.c_str());
    }
  }
  else
  {
    SelNet(m_netLevel, aSelection);
  }
}

void FtpDirClass::DoSelect(int aSelection)
{
  if(aSelection >= m_dirInfoList.size())
    return;
  std::string sel(m_dirInfoList[aSelection].m_name);
  if(sel[0] == '[')
  {
    if(sel[1] == '.')
    {
       if(m_dirPath.find_first_of("\\/") == std::string::npos) /* If we have a subst'ed drive or a net drive we might have a [..] in the root */
          return;
       if(RemoveLastPath(m_dirPath))
          UpdateDir();
    }
    else
    {
      AddSlash(m_dirPath);
      sel = sel.substr(1, sel.size() - 2);
      m_dirPath += sel;
      AddSlash(m_dirPath);
      UpdateDir();
    }
    SetStatusText(m_dirPath.c_str());
  }
  else
  {

    AddSlash(m_dirPath);
    std::string str(m_dirPath);
    str += sel;
    wainApp.OpenDocument(str.c_str(), ODF_FTP_FILE);
  }
}

const char *MyStrIStr(const char *s1, const char *s2)
{
  const char *t = s1, *n, *c;

  while(*t)
  {
    n = t;
    c = s2;
    while(tolower(*(unsigned char *)n) == tolower(*(unsigned char *)c) && *c != '\0' && *n != 0)
    {
      c++;
      n++;
    }
    if(*c == '\0')
      return t;
    t++;
  }
  return 0;
}

const char *MyStrIStr2(const char *s1, const char *s2)
/* Like MyStrIStr, but s2 is expected to be in lower case */
{
  const char *t = s1, *n, *c;

  while(*t)
  {
    n = t;
    c = s2;
    while(tolower(*(unsigned char *)n) == *c && *c != '\0' && *n != 0)
    {
      c++;
      n++;
    }
    if(*c == '\0')
      return t;
    t++;
  }
  return 0;
}

void FtpDirClass::EnableFtpMode(const char *new_path)
{
  m_dirPath = new_path;
  UpdateDir();
}

void FtpDirClass::DisableFtpMode(void)
{
  UpdateDir();
}

int DirClass::GetIconIndex(int nr)
{
  if(nr < m_dirInfoList.size())
    return m_dirInfoList[nr].m_iconIndex;
  return -1;
}

const char *DirClass::GetListText(int nr)
{
  if(nr < m_dirInfoList.size())
    return m_dirInfoList[nr].m_name.c_str();
  return 0;
}

void HdDirClass::ChangeDisk(void)
{
  std::string Str, OldPath(m_dirPath), DStr;

  CComboBox *cb = (CComboBox *)m_dlg->GetDlgItem(IDB_COMBO);
  ASSERT(cb);
  int Len = cb->GetLBTextLen(cb->GetCurSel());
  Str.resize(Len + 1);
  cb->GetLBText(cb->GetCurSel(), (char *)Str.c_str());

  if(Str[2] != '/')
  {
    m_inNet = false;
    int drive;
    drive = toupper(unsigned(Str[2])) - 'A' + 1;
    DStr = Str[2];
    DStr += ":\\";
    m_dirPath.resize(MAX_PATH);
    if(_access(DStr.c_str(), 0) || !_getdcwd(drive, (char *)m_dirPath.c_str(), MAX_PATH))
    {
      m_dirPath = OldPath;
      Str = "[-";
      Str += (char )toupper(unsigned(OldPath[0]));
      Str += "-]";
      cb->SelectString(-1, Str.c_str());
      SetStatusText("Unable to change disk");
    }
    else
    {
       m_dirPath.resize(strlen(m_dirPath.c_str()));
       UpdateDir();
    }
  }
  else
  {
    m_inNet = true;
    SelNet(0);
  }
}

void HdDirClass::HandleGetIcon(ThreadGetIconConClass *aMsg)
{
  if(aMsg->m_index == m_threadCounter)
  {
    unsigned int i;
    for(i = 0; i < m_dirInfoList.size() && i < aMsg->m_iconList.size(); i++)
    {
      m_dirInfoList[i].m_iconIndex = aMsg->m_iconList[i];
    }
    if(aMsg->m_imageList)
      m_dlg->m_sysImageList = aMsg->m_imageList;
  }
}

void DirClass::HandlePrev(void)
{
  if(!m_dirInfoList.empty() && m_dirInfoList[0].m_name == "[..]")
    DoSelect(0);
}

bool DirClass::Search(int *aIndex, int aDir, const char *aSText)
{
  bool found;
  for(found = false; *aIndex < m_dirInfoList.size() && *aIndex >= 0 && !found; *aIndex += aDir)
    if(MyStrIStr(m_dirInfoList[*aIndex].m_name.c_str(), aSText))
      found = true;
  return found;
}

void HdDirClass::AddNet(int level, NETRESOURCE *res)
{
   HANDLE hEnum;
   DWORD Result = WNetOpenEnum(RESOURCE_GLOBALNET, // all network resources
                               RESOURCETYPE_ANY,   // all resources
                               0,                  // enumerate all resources
                               res,                // NULL first time the function is called
                               &hEnum);            // handle to the resource
   if(Result == NO_ERROR)
   {
      DWORD ResultEnum;
      DWORD cbBuffer = 16384;       // 16K is a good size
      DWORD cEntries = (DWORD )-1;  // enumerate all possible entries

      if(!m_netResource[level])
         m_netResource[level] = (NETRESOURCE *)malloc(cbBuffer);

      ResultEnum = WNetEnumResource(hEnum,               // resource handle
                                    &cEntries,           // defined locally as -1
                                    m_netResource[level], // LPNETRESOURCE
                                    &cbBuffer);          // buffer size
      if(ResultEnum == NO_ERROR)
      {
         for(unsigned int i = 0; i < cEntries; i++)
            AddEntry(m_netResource[level][i].lpRemoteName ? m_netResource[level][i].lpRemoteName : m_netResource[level][i].lpProvider);
         m_nofNetResource[level] = cEntries;
      }
      else
         SetStatusText("Unable To Enum Net");
   }
   else
      SetStatusText("Unable To Open Net");
}

void HdDirClass::SelNet(int aLevel, int aSel)
{
   unsigned int i;
   if(aSel < 0 || aSel >= m_dirInfoList.size())
      return;
   CWaitCursor Wait;
   std::string temp(m_dirInfoList[aSel].m_name);
   Clear();

   if(aLevel == 0)
   {
      for(i = 0; i < MAX_NET_LEVEL; i++)
         if(m_netResource[i])
         {
            free(m_netResource[i]);
            m_netResource[i] = NULL;
         }
      AddNet(0, NULL);
      m_netSel[aLevel] = 0;
   }
   else
   {
      ASSERT(m_netLevel < MAX_NET_LEVEL - 1);
      ASSERT(m_netResource[aLevel - 1] || (aSel == 0 && aLevel > 1));
      ASSERT(aSel <= m_nofNetResource[aLevel - 1]);
      if(aLevel > 1 && aSel == 0)
      { /* Selected the [..] */
         if(temp != "[..]")
            aLevel -= 2;
         else
            aLevel--;
         if(aLevel <= 0)
         {
            for(i = 0; i < MAX_NET_LEVEL; i++)
               if(m_netResource[i])
               {
                  free(m_netResource[i]);
                  m_netResource[i] = NULL;
               }
            AddNet(0, NULL);
            m_netSel[0] = 0;
         }
         else
         {
            AddEntry("..");
            AddNet(aLevel, &m_netResource[aLevel - 1][m_netSel[aLevel]]);
         }
      }
      else
      {
         AddEntry("..");
         if(aLevel > 1)
            aSel--; /* Levels after 0 has [..] at first index */
         AddNet(aLevel, &m_netResource[aLevel - 1][aSel]);
         m_netSel[aLevel] = aSel;
      }
   }
   m_netLevel = ++aLevel;
   m_dlg->m_navigatorList->Setup(m_dirInfoList.size(), 2, m_dlg->m_sysImageList);
}

bool HdDirClass::CheckNet(int sel)
/* Return false if the normal handling should be used to handle the selection */
/* Return true if SelNet() is to handle the selection */
{
   if(m_dirInfoList[sel].m_name == "..")
      return true;
   if(m_dirInfoList[sel].m_name == "[..]")
   { /* If this is back to first level, return FALSE */
      std::string Temp = m_dirPath;
      if(RemoveLastPath(Temp))
      {
         std::string::size_type pos = Temp.rfind('\\');
         if(pos != std::string::npos && pos > 2)
            return false;
      }
      return true;
   }

   if(sel > m_nofNetResource[m_netLevel -1])
      return true;

   if(m_netResource[m_netLevel - 1][sel].dwType == RESOURCETYPE_DISK)
   {
      return false;
   }
   return true;
}

void HdDirClass::DoPopupMenu(POINT *p)
{
  static std::string temp; // Fixme
  temp = m_dirPath;

  if(m_dirInfoList[m_dlg->m_navigatorList->m_selected].m_name[0] == '[')
  {
    if(m_dirInfoList[m_dlg->m_navigatorList->m_selected].m_name != "[..]")
    {
      temp += m_dirInfoList[m_dlg->m_navigatorList->m_selected].m_name.substr(1, m_dirInfoList[m_dlg->m_navigatorList->m_selected].m_name.size() - 2);
    }
    else
      temp = temp.substr(0, temp.size() - 1);
  }
  else
    temp += m_dirInfoList[m_dlg->m_navigatorList->m_selected].m_name;

  GetMf()->DoContextMenu(*p, temp.c_str());
}

void HdDirClass::OpenDirForFile(const char* _filename)
{
   char driveDir[MAX_PATH];
   MySplitPath(_filename, SP_DRIVE | SP_DIR, driveDir);
   m_dirPath = driveDir;
   UpdateDir(); // done by the caller
}

BEGIN_MESSAGE_MAP(NavigatorTabCtrlClass, CTabCtrl)
  ON_WM_ERASEBKGND()
END_MESSAGE_MAP();

BOOL NavigatorTabCtrlClass::OnEraseBkgnd(CDC *dc)
{
   if(GetItemCount())
   {
      RECT cr;
      GetClientRect(&cr);
      CBrush bb(GetSysColor(COLOR_3DFACE));
      dc->FillRect(&cr, &bb);
      cr.bottom = cr.top + 20;
      dc->FillRect(&cr, m_dlg->m_buttonBrush);
      bb.DeleteObject();
      return TRUE;
   }
   return CTabCtrl::OnEraseBkgnd(dc);
}


