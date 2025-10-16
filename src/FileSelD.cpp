#include ".\..\src\stdafx.h"
#include "Resource.h"
#include ".\..\src\wain.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\NavigatorDlg.h"
#include ".\..\src\Project.h"
#include "FileSelD.h"
#include "..\src\DocProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(FileSelectDialogClass, CDialog)
  ON_CBN_SELENDOK(IDC_PM_DISK_COMBO,    ChangeDisk)
  ON_CBN_SELENDOK(IDC_PM_FILTER_COMBO,  ChangeFilter)
  ON_BN_CLICKED(IDC_PM_ADD,             Add)
  ON_BN_CLICKED(IDC_PM_REMOVE,          Remove)
  ON_BN_CLICKED(IDC_PM_HOME,            Home)
  ON_LBN_DBLCLK(IDC_PM_CURR_DIR,        DirListSelect)
  ON_WM_SIZE()
  ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP();

FileSelectDialogClass::FileSelectDialogClass(CWnd *aParent) : CDialog(FileSelectDialogClass::IDD, aParent)
{
   m_dirPath = wainApp.gs.m_lastUsedDir;
}

FileSelectDialogClass::~FileSelectDialogClass()
{
}

BOOL FileSelectDialogClass::OnInitDialog(void)
{
   CWaitCursor wait;
   char temp[MAX_PATH];
   temp[1] = ':';
   temp[2] = 0;

   m_tree.Create(TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_DISABLEDRAGDROP | TVS_NOTOOLTIPS | WS_VISIBLE | WS_TABSTOP | WS_CHILD, CRect(2*5, 2*5, 2*220, 2*190), this, IDC_PM_CURR_TREE);
   m_tree.ModifyStyleEx(0, WS_EX_CLIENTEDGE, FALSE);
   FileListClass::size_type i;
   for(i = 0; i < m_fileList.size(); i++)
      AddItem(m_fileList[i]);

   /* Expand the first levels in the tree */
   HTREEITEM item = m_tree.GetRootItem();
   SortTree(item);
   ExpandTree(item, 0);

   CComboBox *cb = (CComboBox *)GetDlgItem(IDC_PM_DISK_COMBO);
   ASSERT(cb);
   cb->ResetContent();
   cb->Dir(DDL_DRIVES | 0x8000, "*.*");
   char d_str[10];
   sprintf(d_str, "[-%c-]", toupper(unsigned(m_dirPath[0])));
   cb->SelectString(-1, d_str);

   cb = (CComboBox *)GetDlgItem(IDC_PM_FILTER_COMBO);
   ASSERT(cb);

   for(i = 0; i < wainApp.gs.m_docProp.size(); i++)
   {
      sprintf(temp, "%s (%s)", wainApp.gs.m_docProp[i]->m_extensionType.c_str(), wainApp.gs.m_docProp[i]->m_extensions.c_str());
      cb->AddString(temp);
   }
   cb->SetCurSel(0);
   m_filter = wainApp.gs.m_docProp[0]->m_extensions;

   ViewDir();

   RECT cr;
   GetClientRect(&cr);
   SetSizes(cr.right, cr.bottom);
   SetWindowText(m_caption);
   SetDlgItemText(IDC_PM_CURRENT_PATH, (char *)m_dirPath.c_str());
   return TRUE;
}

void FileSelectDialogClass::AddSub(HTREEITEM aPrevItem, int aFirst, int aLast, int aOffset)
{
   int i, j;
   bool rec;

   TVINSERTSTRUCT Insert;
   Insert.hParent = aPrevItem;
   Insert.item.mask = TVIF_TEXT;
   Insert.hInsertAfter = 0;
   HTREEITEM prev;
   std::string temp;

   for(i = aFirst; i < aLast; )
   {
      std::string::size_type n = m_fileList[i].find_first_of("\\/", aOffset);
      temp = m_fileList[i].substr(aOffset, n == std::string::npos ? std::string::npos : n - aOffset);

      if(n != std::string::npos)
         rec = true;
      else
         rec = false;

      Insert.item.pszText = strdup(temp.c_str());
      prev = m_tree.InsertItem(&Insert);
      m_tree.SetItemData(aPrevItem, (DWORD )Insert.item.pszText);
      Insert.hInsertAfter = aPrevItem;

      for(j = i; i < aLast && temp != m_fileList[i].substr(aOffset); i++)
      { /* Nothing */
      }
      if(rec)
      {
         AddSub(prev, j, i, n + 1);
      }
   }
}

void FileSelectDialogClass::OnOK(void)
{
   CListBox *pdl = (CListBox *)GetDlgItem(IDC_PM_CURR_DIR);
   if(GetFocus() == &m_tree)
   {
      return;
   }
   if(GetFocus() == pdl)
   {
      DirListSelect();
      return;
   }
   m_dirList.clear();

   wainApp.gs.SetLastUsedDir(m_dirPath.c_str());
   EndDialog(IDOK);
}

void FileSelectDialogClass::OnCancel(void)
{
   m_fileList.clear();
   m_dirList.clear();
   ClearTree();

   EndDialog(IDCANCEL);
}

void FileSelectDialogClass::DoDataExchange(CDataExchange *dx)
{
   CDialog::DoDataExchange(dx);
}

void FileSelectDialogClass::ViewDir(void)
{
   std::string temp, temp2;
   _finddata_t data;

   if(m_dirPath.size() && m_dirPath[m_dirPath.size() - 1] != '\\')
      m_dirPath += "\\";

   m_dirList.clear();
   // First do dirs
   long handle = _findfirst((char *)(m_dirPath + "*.*").c_str(), &data);
   int Done = handle;
   while(Done != -1)
   {
      if(data.attrib & _A_SUBDIR && strcmp(data.name, "."))
      {
         m_dirList.push_back(std::string("[") + data.name + std::string("]"));
      }
      Done = _findnext(handle, &data);
   }
   _findclose(handle);

   // Then do files
   StrSplitterClass FilterSplitter(m_filter);
   while(FilterSplitter.Get(temp2, ";"))
   {
      temp = m_dirPath;
      temp += temp2;

      handle = _findfirst((char *)temp.c_str(), &data);
      Done = handle;

      while(Done != -1)
      {
         if(!(data.attrib & _A_SUBDIR))
         {
            m_dirList.push_back(data.name);
         }
         Done = _findnext(handle, &data);
      }
      _findclose(handle);
   }


   DirListCompareClass Compare;
   std::sort(m_dirList.begin(), m_dirList.end(), Compare);

   CListBox *lb = (CListBox *)GetDlgItem(IDC_PM_CURR_DIR);
   ASSERT(lb);
   lb->ResetContent();
   DirListClass::size_type i;
   for(i = 0; i < m_dirList.size(); i++)
      lb->AddString(m_dirList[i].c_str());
   if(m_dirList.size())
      lb->SetSel(0);
   SetDlgItemText(IDC_PM_CURRENT_PATH, (char *)m_dirPath.c_str());
}

void FileSelectDialogClass::DirListSelect(void)
{
   CListBox *lb = (CListBox *)GetDlgItem(IDC_PM_CURR_DIR);
   ASSERT(lb);
   if(lb->GetSelCount() != 1)
     return;

   int index;
   if(lb->GetSelItems(1, &index) != 1)
     return;
   char temp[MAX_PATH];
   lb->GetText(index, temp);
   if(temp[0] != '[')
     return;

   temp[strlen(temp) - 1] = 0;
   memmove(temp, &temp[1], strlen(temp));
   if(!strcmp(temp, ".."))
   {
      if(m_dirPath.empty())
         return;
      if(m_dirPath[m_dirPath.size() - 1] == '\\')
         m_dirPath = m_dirPath.substr(0, m_dirPath.size() - 1);
      std::string::size_type i = m_dirPath.find_last_of('\\');
      if(i == std::string::npos)
         return;
      m_dirPath = m_dirPath.substr(0, i);
   }
   else if(strcmp(temp, "."))
   {
      m_dirPath += temp;
      m_dirPath += "\\";
   }
   ViewDir();
}

void FileSelectDialogClass::ChangeDisk(void)
{
   char str[32], d_str[32];

   CComboBox *cb = (CComboBox *)GetDlgItem(IDC_PM_DISK_COMBO);
   ASSERT(cb);
   cb->GetLBText(cb->GetCurSel(), str);
   int drive = toupper(unsigned(str[2])) - 'A' + 1;
   sprintf(d_str, "%c:\\", str[2]);
   char Temp[MAX_PATH];
   if(_access(d_str, 0) || !_getdcwd(drive, Temp, MAX_PATH))
   {
      sprintf(str, "[-%c-]", toupper(unsigned(m_dirPath[0])));
      cb->SelectString(-1, str);
      SetStatusText("Unable to change disk");
   }
   else
   {
      m_dirPath = Temp;
      ViewDir();
   }
}

void FileSelectDialogClass::Add(void)
{
   CListBox *lb = (CListBox *)GetDlgItem(IDC_PM_CURR_DIR);
   ASSERT(lb);

   int nof = lb->GetSelCount();
   if(nof == 0 || nof == LB_ERR)
      return;

   int *index = new int [nof*2];
   if(lb->GetSelItems(nof, index) != nof)
   {
      delete [] index;
      return;
   }
   m_changed = TRUE;
   char Temp[256];
   std::string Temp2;

   if(m_dirPath[m_dirPath.size() - 1] != '\\')
      m_dirPath += "\\";

   for(int i = 0; i < nof; i++)
   {
      if(lb->GetText(index[i], Temp) != LB_ERR)
      {
         if(Temp[0] == '[')
         { /* It's a directory */
            if(strcmp(Temp, "[..]"))
            {
               Temp2 = m_dirPath;
               Temp[strlen(Temp) - 1] = 0;
               Temp2 += &Temp[1];
               AddPath(Temp2);
            }
         }
         else
         {
            Temp2 = m_dirPath;
            Temp2 += Temp;
            if(std::find(m_fileList.begin(), m_fileList.end(), Temp2) == m_fileList.end())
            {
               AddItem(Temp2);
               m_fileList.push_back(Temp2);
            }
         }
      }
   }
   delete [] index;
   lb->SetSel(-1, FALSE);
   m_tree.SortChildren(0);
   HTREEITEM root = m_tree.GetRootItem();
   SortTree(root);
}

void FileSelectDialogClass::AddPath(const std::string &aPath)
{
   struct _finddata_t data;
   std::string temp;
   std::string temp2;
   temp = aPath;
   temp += "\\";
   const std::string FullPath = temp;
   // bool got_all = false;

   // bool HaveAll = false;
   long handle;
   int Done;

   StrSplitterClass FilterSplitter(m_filter);

   while(FilterSplitter.Get(temp2, ";"))
   {
      temp = FullPath + temp2;

      handle = _findfirst((char *)temp.c_str(), &data);
      Done = handle;

      while(Done != -1)
      {
         if(!(data.attrib & _A_SUBDIR))
         {
            std::string NewName(FullPath + data.name);
            if(std::find(m_fileList.begin(), m_fileList.end(), NewName) == m_fileList.end())
            {
               AddItem(NewName);
               m_fileList.push_back(NewName);
            }
         }
         Done = _findnext(handle, &data);
      }
      _findclose(handle);
   }

   temp = FullPath + "*";
   handle = _findfirst((char *)temp.c_str(), &data);
   Done = handle;
   while(Done != -1)
   {
      if(data.attrib & _A_SUBDIR)
      {
         if(strcmp(data.name, ".") && strcmp(data.name, ".."))
         {
            AddPath(FullPath + data.name);
         }
      }
      Done = _findnext(handle, &data);
   }
   _findclose(handle);
}

void FileSelectDialogClass::RemoveSub(HTREEITEM aItem)
{
   HTREEITEM child = m_tree.GetChildItem(aItem);
   while(child)
   {
      HTREEITEM next = m_tree.GetNextSiblingItem(child);
      RemoveSub(child);
      child = next;
   }
   DWORD d = m_tree.GetItemData(aItem);
   free((void *)d);
   m_tree.DeleteItem(aItem);
}

void FileSelectDialogClass::Remove(void)
{
  int i;
  HTREEITEM item = m_tree.GetSelectedItem();
  if(!item)
    return;

  HTREEITEM next = item;
  CString path = "";
  /* First get the path to remove */
  do
  {
    CString p = m_tree.GetItemText(next);
    if(path.IsEmpty())
      path = p;
    else
      path = p + "\\" + path;
    next = m_tree.GetParentItem(next);
  }
  while(next);

  /* Then remove the items from our list */
  for(i = 0; i < m_fileList.size(); i++)
  {
    if(!strnicmp(m_fileList[i].c_str(), path, path.GetLength()))
    {
       m_fileList.erase(m_fileList.begin() + i);
       i--;
    }
  }
  m_changed = TRUE;
  /* Finnaly remove the items from the tree */
  RemoveSub(item);
}

void FileSelectDialogClass::Home(void)
{
   if (GetMf()->m_navigatorDialog.m_project->GetProjectName().empty())
   {
      SetStatusText("Unable to change path");
      return;
   }
   char Temp[MAX_PATH];
   MySplitPath(GetMf()->m_navigatorDialog.m_project->GetProjectName().c_str(), SP_DRIVE | SP_DIR, Temp);
   if(!PathExist(Temp))
   {
      SetStatusText("Unable to change disk");
   }
   else
   {
      m_dirPath = Temp;
      ViewDir();
   }
}

void FileSelectDialogClass::ChangeFilter(void)
{
   CComboBox *cb = (CComboBox *)GetDlgItem(IDC_PM_FILTER_COMBO);
   ASSERT(cb);
   char str[256];
   if(cb->GetLBText(cb->GetCurSel(), str) != LB_ERR)
   {
      std::string S(str);
      std::string::size_type i = S.find_last_of(')');
      std::string::size_type j = S.find_first_of('(');
      if(i != std::string::npos && j != std::string::npos)
      {
         m_filter = S.substr(j + 1, i  - j - 1);
         ViewDir();
      }
   }
}

void FileSelectDialogClass::OnSize(UINT aType, int aX, int aY)
{
  SetSizes(aX, aY);
  CDialog::OnSize(aType, aX, aY);
}

void FileSelectDialogClass::SetSizes(int aX, int aY)
{
   CButton *b = (CButton *)GetDlgItem(IDC_PM_ADD);
   if(!b)
     return;
   b->MoveWindow(aX/2 + 10, 58, 60, 20, FALSE);

   m_tree.MoveWindow(10, 28, aX/2 - 10, aY - 20 - 18, FALSE);

   b = (CButton *)GetDlgItem(IDC_PM_REMOVE);
   ASSERT(b);
   b->MoveWindow(aX/2 + 10, 88, 60, 20, FALSE);

   b = (CButton *)GetDlgItem(IDC_PM_HOME);
   ASSERT(b);
   b->MoveWindow(aX/2 + 10, aY - 30 - 60, 60, 20, FALSE);

   b = (CButton *)GetDlgItem(IDOK);
   ASSERT(b);
   b->MoveWindow(aX/2 + 10, aY - 30 - 30, 60, 20, FALSE);

   b = (CButton *)GetDlgItem(IDCANCEL);
   ASSERT(b);
   b->MoveWindow(aX/2 + 10, aY - 30, 60, 20, FALSE);

   CListBox *lb = (CListBox *)GetDlgItem(IDC_PM_CURR_DIR);
   ASSERT(lb);
   lb->MoveWindow(aX/2 + 70 + 10, 23, aX - (aX/2 + 70 + 10) - 10, aY - 20 - 30 - 18, FALSE);

   CComboBox *cb = (CComboBox *)GetDlgItem(IDC_PM_DISK_COMBO);
   ASSERT(cb);
   cb->MoveWindow(aX/2 + 10, 28, 60, 20, FALSE);

   cb = (CComboBox *)GetDlgItem(IDC_PM_FILTER_COMBO);
   ASSERT(cb);
   cb->MoveWindow(aX/2 + 70 + 10, aY - 20 - 30 + 20, aX - (aX/2 + 70 + 10) - 10, 20, FALSE);

   CStatic* s = (CStatic*)GetDlgItem(IDC_PM_CURRENT_PATH);
   s->MoveWindow(5, 5, aX - 10, 14, FALSE);

   InvalidateRect(NULL, TRUE);
}

void FileSelectDialogClass::OnGetMinMaxInfo(MINMAXINFO FAR *min_max_info)
{
  CDialog::OnGetMinMaxInfo(min_max_info);
  min_max_info->ptMinTrackSize.x = 400;
  min_max_info->ptMinTrackSize.y = 200;
}

void FileSelectDialogClass::ClearTree(void)
{
  HTREEITEM item = m_tree.GetRootItem();
  if(item)
    RemoveSub(item);
}

void SplitPath(char **part, const char *fn) // Fixme
{
  int i, j = 1, n;
  part[0] = (char *)malloc(3);
  part[0][0] = fn[0];
  part[0][1] = ':';
  part[0][2] = 0;
  char temp[MAX_PATH];
  i = 3;

  while(fn[i])
  {
    for(n = 0; fn[i] && fn[i] != '\\' && fn[i] != '/'; n++, i++)
      temp[n] = fn[i];
    temp[n] = 0;
    part[j++] = strdup(temp);
    if(fn[i])
      i++;
  }
  part[j] = NULL;
}

void FileSelectDialogClass::AddItem(const std::string& _fileName)
{
   std::string dirPath;
   MySplitPath(_fileName.c_str(), SP_DRIVE | SP_DIR, dirPath);
   if (dirPath == m_lastPath)
   {
      std::string fnExt;
      MySplitPath(_fileName.c_str(), SP_FILE | SP_EXT, fnExt);
      TVINSERTSTRUCT insStruct;
      insStruct.hParent = m_lastItem;
      insStruct.item.mask = TVIF_TEXT;
      insStruct.hInsertAfter = 0;
      insStruct.item.pszText = strdup(fnExt.c_str());
      m_tree.InsertItem(&insStruct);
      return;
   }
   m_lastPath = dirPath;

   HTREEITEM item = m_tree.GetRootItem();
   HTREEITEM candidate = 0;
   int candidateIdx = -1;

   char *pathPart[MAX_PATH/2];
   SplitPath(pathPart, _fileName.c_str());
   bool found = false;
   int j = 0;

   while(!found && item)
   {
      CString p = m_tree.GetItemText(item);
      if(pathPart[j] && !stricmp(pathPart[j], p))
      {
         do
         {
            candidate = item;
            candidateIdx = j;
            item = m_tree.GetChildItem(item);
            p = m_tree.GetItemText(item);
            j++;
         }
         while(item && pathPart[j] && !stricmp(pathPart[j], p));
      }
      else
      {
         item = m_tree.GetNextSiblingItem(item);
      }
   }

   TVINSERTSTRUCT insStr;
   insStr.hParent = candidate;
   insStr.item.mask = TVIF_TEXT;
   insStr.hInsertAfter = 0;

   j = candidateIdx + 1;

   while(pathPart[j])
   {
      insStr.item.pszText = strdup(pathPart[j++]);
      m_lastItem = candidate;
      candidate = m_tree.InsertItem(&insStr);
      m_tree.Expand(candidate, TVE_EXPAND);
      insStr.hParent = candidate;
   }
   for(j = 0; pathPart[j]; j++)
   {
      free(pathPart[j]);
   }
}

void FileSelectDialogClass::ExpandTree(HTREEITEM item, int level)
{
  while(item)
  {
    m_tree.Expand(item, TVE_EXPAND);
    if(level < 3)
    {
      HTREEITEM child = m_tree.GetChildItem(item);
      ExpandTree(child, level + 1);
    }
    item = m_tree.GetNextSiblingItem(item);
  }
}

void FileSelectDialogClass::SortTree(HTREEITEM item)
{
   m_tree.SortChildren(item);
   while(item)
   {
      m_tree.SortChildren(item);
      HTREEITEM child = m_tree.GetChildItem(item);
      SortTree(child);
      item = m_tree.GetNextSiblingItem(item);
   }
}
