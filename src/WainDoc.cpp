//=============================================================================
// This source code file is a part of Wain.
// It implements WainDoc.
//=============================================================================
#include ".\..\src\stdafx.h"
#include "Wain.h"
#include "MainFrm.h"
#include "WainDoc.h"
#include "WainSearch.h"
#include "wainview.h"
#include "childfrm.h"
#include "WainFile.h"
#include "WainDocUtil.h"
#include "DocProp.h"
#include "WordListThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4100) // unreferenced formal parameter

IMPLEMENT_DYNCREATE(WainDoc, CDocument)

WainDoc::WainDoc()
  : m_prop(wainApp.gs.m_docProp[0]),
    m_childFrame(NULL),
    m_view(NULL),
    m_isDebugFile(FALSE),
    m_lineCount(0),
    m_propIndex(0),
    m_newFile(FALSE),
    m_maxLen(0),
    m_unixStyle(FALSE),
    m_isFtpFile(FALSE),
    m_file(0),
    m_gotInput(FALSE)
{
    m_line.m_doc = this;
    SetReadOnly(false);
}

WainDoc::~WainDoc()
{
  TxtLine *l = m_line.m_next, *n;
  while(l != &m_line)
  {
    n = l->m_next;
    delete l;
    l = n;
  }
  if(m_file)
  {
     FileClass *f = m_file;
     m_file = 0;
     delete f;
  }
}

BOOL WainDoc::OnNewDocument()
{
  if (!CDocument::OnNewDocument())
    return FALSE;
  *this += "";
  SetModified(true);
  return TRUE;
}

void WainDoc::Serialize(CArchive &ar)
{
}

EndLineStatusType WainDoc::GetPrevEls(TxtLine *l) const
{
  ASSERT(l);
  TxtLine *p = l->m_prev;
  return p->m_endLineStatus;
}

TxtLine *WainDoc::FindString(TxtLine *l, const char *string, int *column, int *line_offset, int flags, size_t *MatchLen)
{
   int co = l->ScreenPosToTextPos(*column);
   // Please notice the "nice" typecast in the line below
   const char *(*s_strstr)(const char *, const char *) = flags & SEARCH_IGNORE_CASE ? MyStrIStr2 : (const char *(*)(const char *, const char *))strstr;
   const char *s;
   CString sstring = string;

   if(flags & SEARCH_IGNORE_CASE)
      sstring.MakeLower();

   if(flags & SEARCH_BACKWARD)
   {
      if(flags & SEARCH_FROM_START)
      { // Ie. from end
         *line_offset = m_lineCount - *line_offset;
         (*line_offset)--;
         l = m_line.m_prev;
         co = l->m_len;
      }
      else
      {
         if(co > l->m_len)
            co = l->m_len;
          co--;
         *line_offset = 0;
      }
      int pco = co;
      s = NULL;
      while(co >= 0 && !s)
      {
#if 0
      std::regex e(m_projectOptions.m_newLineRegexp.c_str());
      std::cmatch cm;
      std::regex_match(_ss, cm, e);
      if (cm.size() >= 3)
      {
         _fn = cm[1];
         _lineNo = strtol(cm.str(2).c_str(), NULL, 0);
#endif

         if(flags & SEARCH_USE_REGEX)
         {
            std::cmatch cm;
            std::regex_match(&l->m_text[co], cm, GetMf()->m_searchDialog->m_regEx);
            if (cm.size() > 0)
            {
               s = cm.str(1).c_str();
            }
         }
         else
         {
            s = s_strstr(&l->m_text[co], sstring);
         }
         if(s)
         {
            if(s >= &l->m_text[pco])
               s = NULL;
         }
         if(!s)
            co--;
      }

      if(s && co >= 0 && s < &l->m_text[pco])
      {
      }
      else
      {
         do
         {
            s = NULL;
            do
            { /* First find a line which has the string to search for */
               l = GetPrev(l);
               (*line_offset)--;
               if(l)
               {
                  if(flags & SEARCH_USE_REGEX)
                  {
                     // s = GetMf()->m_searchDialog->m_regEx.find(l->m_text);
                    std::cmatch cm;
                    std::regex_match(l->m_text, cm, GetMf()->m_searchDialog->m_regEx);
                    if (cm.size() > 0)
                    {
                       s = cm.str(1).c_str();
                    }
                  }
                  else
                     s = s_strstr(l->m_text, sstring);
               }
            }
            while(l && !s);
            /* Then find the last occurence of the string */
            if(l)
            {
               co = l->m_len - 1;
               s = NULL;
               while(co >= 0 && !s)
               {
                  if(flags & SEARCH_USE_REGEX)
                  {
                     // s = GetMf()->m_searchDialog->m_regEx.find(&l->m_text[co]);
                    std::cmatch cm;
                    std::regex_match(&l->m_text[co], cm, GetMf()->m_searchDialog->m_regEx);
                    if (cm.size() > 0)
                    {
                       s = cm.str(1).c_str();
                    }
                  }
                  else
                     s = s_strstr(&l->m_text[co], sstring);
                  if(!s)
                     co--;
               }
            }
         }
         while(l && !s);
      }
   }
   else // Search Forward
   {
      s = nullptr;
      if(flags & SEARCH_FROM_START)
      {
         co = 0;
         l = m_line.m_next;
         *line_offset = -*line_offset;
      }
      else
      {
         co++;
         *line_offset = 0;
      }
      if(flags & SEARCH_FIRST)
         co = 0;
      if(co >= l->GetTextLen())
         s = NULL;
      else if(flags & SEARCH_USE_REGEX)
      {
         // s = GetMf()->m_searchDialog->m_regEx.find(&l->m_text[co], MatchLen);
         std::cmatch cm;
         std::regex_match(&l->m_text[co], cm, GetMf()->m_searchDialog->m_regEx);
         if (cm.size() > 0)
         {
            s = cm.str(1).c_str();
         }
      }
      else
      {
         s = s_strstr(&l->m_text[co], sstring);
         if(s && MatchLen)
            *MatchLen = strlen(sstring);
      }
      while(l && !s)
      {
         (*line_offset)++;
         l = GetNext(l);
         co = 0;
         if(l)
         {
            if(flags & SEARCH_USE_REGEX)
            {
               // s = GetMf()->m_searchDialog->m_regEx.find(&l->m_text[co], MatchLen);
               std::cmatch cm;
               std::regex_match(&l->m_text[co], cm, GetMf()->m_searchDialog->m_regEx);
               if (cm.size() > 0)
               {
                  s = cm.str(0).c_str();
               }
            }
            else
            {
               s = s_strstr(&l->m_text[co], sstring);
               if(s && MatchLen)
                  *MatchLen = strlen(sstring);
            }
         }
      }
   }
   if(l && s)
   {
      *column = s - l->m_text;
      *column = l->TextPosToScreenPos(*column);
   }
   return l;
}

TxtLine *WainDoc::InsertLine(TxtLine *l, const char *str)
{
  TxtLine *nl = new TxtLine(this);
  if(!l)
    l = m_line.m_prev;
  nl->m_next = l->m_next;
  nl->m_prev = l;
  l->m_next->m_prev = nl;
  l->m_next = nl;
  m_lineCount++;
  *nl = str;
  SetModified(true);
  return nl;
}

TxtLine *WainDoc::InsertLineBefore(TxtLine *l, const char *str)
{
  TxtLine *nl = new TxtLine(this);
  if(!l)
    l = m_line.m_next;
  l = l->m_prev;
  nl->m_next = l->m_next;
  nl->m_prev = l;
  l->m_next->m_prev = nl;
  l->m_next = nl;
  m_lineCount++;
  *nl = str;
  SetModified(true);
  return nl;
}

TxtLine *WainDoc::RemoveLine(TxtLine *l)
{
   m_view->RemoveLine(l);
  TxtLine *r = l->m_next;
  ASSERT(l != &m_line);
  l->m_next->m_prev = l->m_prev;
  l->m_prev->m_next = l->m_next;
  delete l;
  SetModified(TRUE);
  m_lineCount--;
  if(!m_lineCount)
    *this += "";
  return r;
}

void WainDoc::operator += (const char *new_text)
{
  TxtLine *new_line = new TxtLine(this);
  new_line->m_prev = m_line.m_prev;
  new_line->m_next = &m_line;
  m_line.m_prev->m_next = new_line;
  m_line.m_prev = new_line;
  *new_line = new_text;  // Must be Done last
  m_lineCount++;
  SetModified(TRUE);
}

TxtLine *WainDoc::GetLineNo(int LineNo)
{
  TxtLine *l;
  int i;
  for(i = 0, l = m_line.m_next; i < LineNo && l != &m_line; i++, l = l->m_next)
  { /* Nothing */ }

  return l == &m_line ? NULL : l;
}

TxtLine *WainDoc::GetRelLineNo(int lines, TxtLine *CurrentLine)
{
  TxtLine *r = CurrentLine;
  int l = lines;
  while(l && r != &m_line)
  {
    if(l > 0)
    {
      r = r->m_next;
      l--;
    }
    else
    {
      r = r->m_prev;
      l++;
    }
  }
  return r == &m_line ? NULL : r;
}

BOOL WainDoc::JoinLine(TxtLine *l)
// Join the next line to the end of l, delete the next line
{
  BOOL ret;
  if(l->m_next !=  &m_line)
  {
    ret = l->InsertAt(l->m_tabLen, l->m_next->m_text, l->m_next->m_len);
    m_lineCount--;
    TxtLine *n = l->m_next;
    l->m_next->m_next->m_prev = l;
    l->m_next = l->m_next->m_next;
    delete n;
  }
  else
    ret = FALSE;

  return ret;
}

TxtLine *WainDoc::SplitLineAt(TxtLine *c_line, int pos)
{
  c_line->SplitLineAt(pos);
  SetModified(TRUE);
  TxtLine *next_line = c_line->m_next;

  return next_line;
}

DeleteTypeType WainDoc::DeleteAt(TxtLine *c_line, int pos)
{
  DeleteTypeType ret_parm;
  int org_pos = pos;
  pos = c_line->ScreenPosToTextPos(pos);

  if(pos >= c_line->m_len)
  { /* Concatinate this line & the next */
    if(c_line->m_next != &m_line)
    {
      TxtLine *n_line = c_line->m_next;
      c_line->InsertAt(org_pos, n_line->m_text, n_line->m_len);
      c_line->m_next = n_line->m_next;
      c_line->m_next->m_prev = c_line;
      delete n_line;
      m_lineCount--;
      ret_parm = line_delete;
    }
    else
      ret_parm = nOnDelete;
  }
  else
    ret_parm = c_line->DeleteAt(org_pos);

  SetModified(TRUE);

  return ret_parm;
}

void WainDoc::Reset(void)
{
  TxtLine *l = m_line.m_next, *n;
  while(l != &m_line)
  {
    n = l->m_next;
    delete l;
    l = n;
  }
  m_line.m_next = m_line.m_prev = &m_line;
  m_lineCount = 0;
}

bool WainDoc::ReadFile(const char *file_name)
{
   if (m_earlyExit)
      return false;
   TxtLine *l = NULL;
   if(m_isDebugFile && m_lineCount)
     l = m_line.m_prev;

   if(!m_file)
   {
     if(m_isFtpFile)
       m_file = new InetFileClass(file_name, GENERIC_READ);
     else if(m_isDebugFile && !m_readAsStdioFile)
       m_file = new DebugFileClass(file_name, GENERIC_READ);
     else
       m_file = new StdioFileClass(file_name, GENERIC_READ);

     if(m_file->Error())
     {
       delete m_file;
       m_file = 0;
       return false;
     }
   }

   GetStrLineEndType status;
   int unix_lines = 0;
   int dos_lines = 0;

   do
   {
      char ch;
      status = m_file->Read(&ch);
      switch(status)
      {
      case CR_LF_LINE_END:
         dos_lines++;
         if(!l)
           l = InsertLine(0, "");
         l->CalcTabLen();
         l->m_endLineStatus = GetEndLineStatus(l);
         l = InsertLine(l, "");
         break;
      case CR_LINE_END:
         unix_lines++;
         if(!l)
           l = InsertLine(0, "");
         l->CalcTabLen();
         l->m_endLineStatus = GetEndLineStatus(l);
         l = InsertLine(l, "");
         break;
      case EOF_LINE_END:
         break;
      default:
         m_gotInput = TRUE;
         if(!l)
            l = InsertLine(0, "");
         l->Append(ch);
         break;
      }
      if(dos_lines + unix_lines == 100000)
      {
         if(WainMessageBox(GetMf(), "The file seem to be very huge,\r\ncontinue reading?", IDC_MSG_YES | IDC_MSG_NO, IDI_WARNING_ICO) == IDC_MSG_NO)
         {
            status = EOF_LINE_END; // Just to jump out of the while loop
            m_earlyExit = true;
            SetReadOnly(true);     //
         }
      }
   }
   while(status != EOF_LINE_END && !m_earlyExit);

   if(!l) // TODO
      l = InsertLine(0, "");

   l->CalcTabLen();
   l->m_endLineStatus = GetEndLineStatus(l);

   if(unix_lines > dos_lines)
      m_unixStyle = true;
   else
      m_unixStyle = false;

   if(m_view && GetMf()->GetActiveView() == m_view)
      GetMf()->SetCrLfStatus(m_unixStyle);

   if(!m_isDebugFile)
   {
      delete m_file;
      m_file = 0;
   }

   return true;
}

BOOL WainDoc::OnOpenDocument(LPCTSTR path_name)
{
  if (!CDocument::OnNewDocument())
    return FALSE;
  TRACE("Opening: %s\n", path_name);
  GetExtType(path_name, FALSE);
  POSITION pos = GetFirstViewPosition();
  WainView *view = NULL;
  while(pos != NULL)
  {
    view = (WainView *)GetNextView(pos);
  }
  ASSERT(view);
  if(!view->IsKindOf(RUNTIME_CLASS(WainView)))
  {
    WainMessageBox(GetMf(), "Something went wrong here", IDC_MSG_OK, IDI_WARNING_ICO);
    return FALSE;
  }
  m_view = view;
  m_view->SetDocument(this);

  if(!m_newFile && !ReadFile(path_name))
    return FALSE;
  if(m_newFile)
    *this += "";

  SetReadOnly(IsFileReadOnly(path_name));

  if(m_newFile)
    SetReadOnly(false);
  if(m_isDebugFile)
    SetReadOnly(true);

  SetModified(FALSE);
  return TRUE;
}

void WainDoc::ReloadFile(void)
{
  CWaitCursor wait;
  // First remove content
  Reset();
  // Then read the new content

  if(!ReadFile(GetPathName()))
  {
    *this += "";
    return;
  }
  SetReadOnly(IsFileReadOnly(GetPathName()));
  if(m_isDebugFile)
    SetReadOnly(true);

  ASSERT(m_view);

  SetModified(FALSE);
}

void WainDoc::OnCloseDocument(void)
{
   if(GetModified())
   {
      CString msg = "The file: ";
      msg += GetPathName().IsEmpty() ? (const char *)"Unnamed" : (const char *)GetPathName();
      msg += "\r\nHas been modified, do you want to save it?";
      if(WainMessageBox(GetMf(), msg, IDC_MSG_YES | IDC_MSG_NO, IDI_QUESTION_ICO) == IDC_MSG_YES)
        SaveFile(TRUE);
      SetModified(FALSE);
   }
   #if 0
   if(my_atom)
   {
      GlobalDeleteAtom(my_atom);
      my_atom = 0;
   }
   #endif
   CDocument::OnCloseDocument();
}

void WainDoc::DoCloseFile(void)
{
  if(GetModified())
  {
    CString msg;
    msg = "The File: ";
    msg += GetPathName();
    msg += " has been changed\n";
    msg += "Do you want to save your changes ?";
    int id = WainMessageBox(GetMf(), msg, IDC_MSG_YES | IDC_MSG_NO | IDC_MSG_CANCEL, IDI_QUESTION_ICO);
    if(GetMf()->GetActiveView())   /* WainMessageBox(...) might have set my view to be the active view */
      GetMf()->SetActiveView(NULL);
    if(id == IDC_MSG_YES)
      if(!SaveFile(TRUE))
        id = IDC_MSG_CANCEL;

    if(id != IDC_MSG_CANCEL)
    {
      // if(my_atom)
      // {
      //   GlobalDeleteAtom(my_atom);
      //   my_atom = 0;
      // }
      CDocument::OnCloseDocument();
    }
  }
  else if(m_newFile)
  {
    CString msg;
    int id = WainMessageBox(GetMf(), "This new file is not saved, do you want to do it?", IDC_MSG_YES | IDC_MSG_NO | IDC_MSG_CANCEL, IDI_QUESTION_ICO);
    if(GetMf()->GetActiveView())   /* WainMessageBox(...) might have set my view to be the active view */
      GetMf()->SetActiveView(NULL);
    if(id == IDC_MSG_YES)
    {
      SaveFileAs();
      if(m_newFile) // The user hit CANCEL in the "Save As ..." dialog
      {
        return;
      }
    }
    if(id != IDC_MSG_CANCEL)
    {
      // if(my_atom)
      // {
      //   GlobalDeleteAtom(my_atom);
      //   my_atom = 0;
      // }
      CDocument::OnCloseDocument();
    }
  }
  else
  {
    // if(my_atom)
    // {
    //   GlobalDeleteAtom(my_atom);
    //   my_atom = 0;
    // }
    CDocument::OnCloseDocument();
  }
}

void WainDoc::SaveFileAs(void)
{
   if(m_isDebugFile)
      return;
   CString name = GetPathName();
   ASSERT(name.GetLength());
   MyFileDialogClass fd(FD_SAVE | FD_LAST_PATH | FD_SET_LAST_PATH | FD_DOC_FILTER, 0);
   if(fd.DoModal() != IDOK)
      return;
   CString new_name = fd.GetPathName();
   SetPathName(new_name, m_isDebugFile ? FALSE : TRUE);
   bool WasNewFile = m_newFile;
   m_newFile = FALSE;
   bool WasReadOnly = GetReadOnly();
   m_isReadOnly = false;
   if(!SaveFile(TRUE))
   {
      m_newFile = WasNewFile;
      m_isReadOnly = WasReadOnly ? true : false;
      SetPathName(name, FALSE); /* We could not write the file, so revert to the old name */
   }
   else
   {
      m_newFile = FALSE;
      GetExtType(NULL, TRUE);
      m_view->InvalidateRect(NULL, TRUE);
   }
   m_isFtpFile = FALSE; /* Can't "save as" on FTP */
   GetMf()->SetViewName(m_view->m_winNr, (const char *)new_name, m_isFtpFile);
   char title[_MAX_PATH];
   sprintf(title, "%d - %s", m_view->m_winNr, (const char *)new_name);
   if(m_childFrame)
      m_childFrame->SetWindowText(title);
}

BOOL WainDoc::SaveFile(BOOL prompt)
{
  if(m_isDebugFile)
    return FALSE;
  if(!prompt && !GetModified())
    return FALSE;
  if(m_isReadOnly)
    return TRUE;

  CString name = GetPathName();

  if(name.GetLength() == 0 || m_newFile)
  {
    if(!prompt)
      return FALSE;
    else
    {
      MyFileDialogClass fd(FD_SAVE | FD_LAST_PATH | FD_SET_LAST_PATH | FD_DOC_FILTER, OFN_OVERWRITEPROMPT);
      if(fd.DoModal() != IDOK)
        return FALSE;
      name = fd.GetPathName();
      SetPathName(name, !m_newFile);
      CString s = "";
      s += name;
      m_childFrame->SetWindowText(s);
      GetExtType(NULL, TRUE);
    }
  }
  if(m_isFtpFile)
     m_file = new InetFileClass(name, GENERIC_WRITE);
  else // debug files are ! saved
     m_file = new StdioFileClass(name, GENERIC_WRITE);

  if(m_file->Error())
  {
    if(!m_isFtpFile) /* file.open will inform the user in case of an error */
    {
      CString msg;
      msg = "Unable to open:\r\n";
      msg += GetPathName();
      msg += "\r\nFor writing";
      WainMessageBox(GetMf(), msg, IDC_MSG_OK, IDI_WARNING_ICO);
    }
    delete m_file;
    m_file = 0;
    return FALSE;
  }
  TxtLine *p_line = &m_line;
  BOOL error = FALSE;
  while((p_line = GetNext(p_line)) != NULL && !error)
  {
    if(wainApp.gs.m_stripWhitespace)
      p_line->StripWhitespace();
    error |= !m_file->Write(p_line->GetText());
    if(p_line->m_next != &m_line)
      error |= !m_file->Write(m_unixStyle ? "\n" : "\r\n");
  }
  delete m_file;
  m_file = 0;

  if(error)
  {
    CString msg = "An error occurred while writing:\r\n";
    msg += GetPathName();
    WainMessageBox(GetMf(), msg, IDC_MSG_OK, IDI_ERROR_ICO);
    return FALSE;
  }
  SetModified(FALSE);

  POSITION pos = GetFirstViewPosition();
  if(pos != NULL)
  {
    WainView *p = (WainView *)GetNextView(pos);
    p->OnSaveDoc();
  }

  ViewListElem *elem = GetMf()->m_viewList.GetViewNr(m_view->m_winNr);
  if(m_newFile)
    GetMf()->SetViewName(m_childFrame->m_view->m_winNr, name, m_isFtpFile);

  m_newFile = FALSE;

  if(elem)
    elem->UpdateStatus();

   std::vector<std::string> fileList(1);
   fileList[0] = GetPathName();
   ::StartWordThread(fileList, wainApp.gs.m_docProp[m_propIndex]->m_keyWordList, wainApp.gs.m_docProp[m_propIndex]->m_seps, wainApp.gs.m_docProp[m_propIndex]->m_ignoreCase, m_propIndex, true);
   void ReadTagsForFile(const std::string& _fileName);
   ReadTagsForFile(fileList[0].c_str());
  return TRUE;
}

void WainDoc::Update(void)
{
  if(!m_file)
     return;
  int prev_LineCount = m_lineCount;
  ReadFile("");

  if(GetModified())
  {
    m_view->OnDebugFileUpdate(prev_LineCount);
    SetModified(false);
  }
}

void WainDoc::Done(void)
{
  if(!m_file)
    return; // Should only happen if I'm called WHILE I'm closing
  int prev_LineCount = m_lineCount;
  ReadFile("");
  delete m_file;
  m_file = 0;
  if(!wainApp.gs.m_debugTools)
    *this += "Done";
  if(GetModified())
  {
    m_view->OnDebugFileUpdate(prev_LineCount);
    SetModified(false);
  }
}

void WainDoc::CloseFile()
{  // Called for tag peek Files to close the File.
   if(!m_file)
      return; // Should only happen if I'm called WHILE I'm closing
   delete m_file;
   m_file = 0;
}

bool WainDoc::GetCurrentWord(std::string &aWord, int offset, const TxtLine *CurrentLine) const
{
  aWord = "";
  offset = CurrentLine->ScreenPosToTextPos(offset);
  if(offset > CurrentLine->m_len)
    return FALSE;
  char *s = &CurrentLine->m_text[offset];
  if(strchr(SEPS(this), *s))
  {
    if(!offset)
      return FALSE;
    s--;
    if(strchr(SEPS(this), *s))
      return FALSE;
  }
  while(s != CurrentLine->m_text && !strchr(SEPS(this), *s))
    s--;
  if(strchr(SEPS(this), *s))
    s++;
  int n;
  for(n = 0; !strchr(SEPS(this), *s); n++, s++)
    aWord += *s;

  return TRUE;
}

int WainDoc::RemoveWordCurrent(int offset, TxtLine *CurrentLine)
{
  if(offset > CurrentLine->m_tabLen)
    return NOT_A_OFFSET;
  int off = CurrentLine->ScreenPosToTextPos(offset);
  char *s = &CurrentLine->m_text[off];
  if(strchr(SEPS(this), *s))
  {
    if(!off)
      return NOT_A_OFFSET;
    s--;
    if(strchr(SEPS(this), *s))
      return NOT_A_OFFSET;
  }
  while(s != CurrentLine->m_text && !strchr(SEPS(this), *s))
    s--;
  if(strchr(SEPS(this), *s))
    s++;
  int st = s - CurrentLine->m_text;
  int n;
  for(n = 0; !strchr(SEPS(this), *s); n++, s++);
  int end = s - CurrentLine->m_text;
  CurrentLine->DeleteAt(CurrentLine->TextPosToScreenPos(st), CurrentLine->TextPosToScreenPos(end));

  return st - off;
}

void WainDoc::SetModified(bool aMod)
{
   if(aMod != GetModified())
   {
      if(m_childFrame)
      {
         std::string Title = GetPathName();
         if(aMod)
         {
            Title += "*";
         }
         m_childFrame->SetWindowText(Title.c_str());
      }
      GetMf()->SetModifiedStatus(m_view, aMod);
   }
   SetModifiedFlag(aMod);
}

void WainDoc::SetReadOnly(bool aReadOnly)
{
   if(m_childFrame)
   {
      std::string Title = GetPathName();
      if(aReadOnly)
      {
         Title += " (RO";
      }
      m_childFrame->SetWindowText(Title.c_str());

      m_isReadOnly = aReadOnly;
   }
   GetMf()->SetReadOnlyStatus(aReadOnly);
}


bool WainDoc::GetModified(void)
{
   return IsModified() ? true : false;
}

// return new cursor position, aIndent is number of spaces to add/remove at the beginning of the file
int WainDoc::GetIndent(TxtLine *aLine, int &aIndent) const
{
#if 1
   #define INDENT m_prop->m_indent
   const char *s;
   const char *t = aLine->m_text;
   int xx = 0;

   /* Count the number of "spaces" at the beginning of the line */
   while(*t == ' ' || *t == '\t')
   {
     if(*t == ' ')
       xx++;
     else
       xx += TAB_WIDTH(this) - xx%TAB_WIDTH(this);
     t++;
   }

   int i;
   int n;
   BOOL x = FALSE;

   /* Find the first char of the last not empty line */
   while(!x && (aLine = aLine->m_prev) != &m_line)
   {
     s = aLine->m_text;
     while(*s == ' ' || *s == '\t')
       s++;
     if(*s)
       x = TRUE;
   }

   if(aLine != &m_line)
   {
     s = aLine->m_text;
     for(n = 0; *s == ' ' || *s == '\t'; s++)
     {
       if(*s == ' ')
         n++;
       else
         n += TAB_WIDTH(this) - n%TAB_WIDTH(this);
     }
     if(IsSeqIc(s, m_prop->m_blockBegin.c_str(), m_prop->m_ignoreCase))
     {
       int got_it;
       const char *p;
       for(p = s + m_prop->m_blockBegin.size(), got_it = 1; *p; p++)
       {
         if(IsSeqIc(p, m_prop->m_blockBegin.c_str(), m_prop->m_ignoreCase))
           got_it++;
         else if(IsSeqIc(p, m_prop->m_blockEnd.c_str(), m_prop->m_ignoreCase))
           got_it--;
       }
       if(got_it > 0)
         n += INDENT;
     }
     if(IsSeqIc(t, m_prop->m_blockEnd.c_str(), m_prop->m_ignoreCase))
       n -= INDENT;
     if(n < 0)
       n = 0;
     if(*t)
       aIndent = n - xx;
     else
       aIndent = 0;
     i = n;
   }
   else
   {
     i = 0;
     aIndent = 0;
   }
   return i;
   #undef INDENT

#else
   const int Indent = m_prop->Indent;
   const char *s;
   const char *t = aLine->FindFirstNotOf(0, "\t ");

   int SpaceInFrontCurrent = aLine->GetSpaceInFront();
   int SpaceInFrontPrevious = 0;
   std::string FirstWordCurrent;
   std::string FirstWordPrevious;
   std::string FirstWordNext;
   if(!GetCurrentWord(FirstWordCurrent, SpaceInFrontCurrent, aLine))
   {
      char T[2];
      T[0] = SpaceInFrontCurrent < aLine->GetTabLen() ? aLine->GetTextAt(SpaceInFrontCurrent) : 0;
      T[1] = 0;
      FirstWordCurrent = T;
   }
   int i;
   int n;
   bool GotPreviousLine = false;

   /* Find the first char of the last not empty line */
   const TxtLine *Line = aLine;
   while(!GotPreviousLine && (Line = Line->prev) != &line)
   {
      s = Line->FindFirstNotOf(0, "\t ");
      if(*s)
      {
         GotPreviousLine = true;
         SpaceInFrontPrevious = Line->GetSpaceInFront();
         if(!GetCurrentWord(FirstWordPrevious, SpaceInFrontPrevious, Line))
         {
            char T[2];
            T[0] = SpaceInFrontPrevious < Line->GetTabLen() ? Line->GetTextAt(SpaceInFrontPrevious) : 0;
            T[1] = 0;
            FirstWordPrevious = T;
         }
      }
   }
   bool GotNextLine = false;
   Line = aLine;
   while(!GotNextLine && (Line = Line->next) != &line)
   {
      s = Line->FindFirstNotOf(0, "\t ");
      if(*s)
      {
         GotNextLine = true;
         int ttt = Line->GetSpaceInFront();
         if(!GetCurrentWord(FirstWordNext, ttt, Line))
         {
            char T[2];
            T[0] = ttt < Line->GetTabLen() ? Line->GetTextAt(ttt) : 0;
            T[1] = 0;
            FirstWordNext = T;
         }
      }
   }

   if(GotPreviousLine)
   {
      n = aLine->GetSpaceInFront();
      //if(!m_prop->IndentAfter.empty())
      if(0)
      { // do some "smart indention"
         bool IsIndentWord = false;
         size_t idx;
         for(idx = 0; idx != m_prop->IndentAfter.size() && !IsIndentWord; idx++)
            if(!StrCmp(m_prop->IndentAfter[idx], FirstWordPrevious, m_prop->IgnoreCase))
               IsIndentWord = true;
         for(idx = 0; IsIndentWord && idx != m_prop->IndentUnless.size(); idx++)
            if(!StrCmp(m_prop->IndentUnless[idx], FirstWordNext, m_prop->IgnoreCase))
               IsIndentWord = false;
         bool IsUnindentWord = false;
         for(idx = 0; !IsUnindentWord && idx < m_prop->Unindent.size(); idx++)
            if(!StrCmp(m_prop->Unindent[idx], FirstWordCurrent, m_prop->IgnoreCase))
               IsUnindentWord = true;

         aIndent = SpaceInFrontPrevious;
         if(IsIndentWord)
         {
            aIndent += Indent;
         }
         if(IsUnindentWord)
         {
            aIndent -= Indent;
            if(aIndent < 0)
               aIndent = 0;
         }
         int T = aIndent;
         aIndent -= SpaceInFrontCurrent;
         return T;
      }
      else
      {
         if(IsSeqIc(s, m_prop->BlockBegin.c_str(), m_prop->IgnoreCase))
         {
           int got_it;
           const char *p;
           for(p = s + m_prop->BlockBegin.size(), got_it = 1; *p; p++)
           {
             if(IsSeqIc(p, m_prop->BlockBegin.c_str(), m_prop->IgnoreCase))
               got_it++;
             else if(IsSeqIc(p, m_prop->BlockEnd.c_str(), m_prop->IgnoreCase))
               got_it--;
           }
           if(got_it > 0)
             n += Indent*got_it;
         }
         if(IsSeqIc(t, m_prop->BlockEnd.c_str(), m_prop->IgnoreCase))
           n -= Indent;
         if(n < 0)
           n = 0;
         if(*t)
           aIndent = n - SpaceInFrontPrevious;
         else
           aIndent = 0;
         i = n;
      }
   }
   else
   {
      i = 0;
      aIndent = 0;
   }
   return i;
#endif
}

TxtLine *WainDoc::FindMatchBrace(TxtLine *l, int *x, int *y, bool allowAfter, int* findOff,  int* startLen, int* endLen)
{
#define doc this // needed by SEPS
  int new_x = l->ScreenPosToTextPos(*x);
  int new_y = *y;
  if(new_x > l->GetTextLen())
    return NULL;
  int count = 1;
  const char *s = l->GetText();
  s += new_x;
  BOOL move = FALSE;
  while(!strchr(SEPS(this), *s) && new_x)
  {
    s--;
    new_x--;
    move = TRUE;
  }
  if(strchr(SEPS(this), *s) && move)
  {
    new_x++;
    s++;
  }
  char match_str[128];
  char end_match_str[128];

  int i;
  const char *c = m_prop->m_braces.c_str();
  while(*c)
  {
    for(i = 0; i < 128 && *c != ',' && *c != 0; c++, i++)
      match_str[i] = *c;
    match_str[i] = 0;
    if(*c)
      c++;
    for(i = 0; i < 128 && *c != ':' && *c != 0; i++, c++)
      end_match_str[i] = *c;
    end_match_str[i] = 0;
    if(*c)
      c++;

    if(IsSeqIc(s, match_str, m_prop->m_ignoreCase))
    {
      do
      {
        s++;
        new_x++;
        while(*s == 0)
        {
          l = GetNext(l);
          if(!l)
            return NULL;
          new_y++;
          new_x = 0;
          s = l->GetText();
        }
        if(IsSeqIc(s, match_str, m_prop->m_ignoreCase))
          count++;
        else if(IsSeqIc(s, end_match_str, m_prop->m_ignoreCase))
          count--;
        if(!count)
        { /* We are Done */
          *x = l->TextPosToScreenPos(new_x);
          *y = new_y;
          if (startLen)
          {
            *startLen = strlen(match_str);
          }
          if (endLen)
          {
            *endLen = strlen(end_match_str);
          }
          return l;
        }
      }
      while(1);
    }
  }
  // return 0;
  c = m_prop->m_braces.c_str();
  while(*c)
  {
    for(i = 0; i < 128 && *c != ',' && *c != 0; c++, i++)
      match_str[i] = *c;
    match_str[i] = 0;
    if(*c)
      c++;
    for(i = 0; i < 128 && *c != ':' && *c != 0; i++, c++)
      end_match_str[i] = *c;
    end_match_str[i] = 0;
    if(*c)
      c++;

    if(IsSeqIc(s, end_match_str, m_prop->m_ignoreCase) ||
       (allowAfter && s - l->GetText() >= strlen(end_match_str) && IsSeqIc(s - strlen(end_match_str), end_match_str, m_prop->m_ignoreCase)))
    {
      if (!IsSeqIc(s, end_match_str, m_prop->m_ignoreCase))
      {
         s -= strlen(end_match_str);
         new_x -= strlen(end_match_str);
         if (findOff)
         {
            *findOff = strlen(end_match_str);
         }
      }
      do
      {
        if(new_x > 0)
          s--;    /* CodeGuard Warning */
        new_x--;
        while(new_x < 0)
        {
          l = GetPrev(l);
          new_y--;
          if(!l)
            return NULL;
          new_x = l->GetTextLen() - 1;
          s = l->GetText();
          if(new_x >= 0)
            s = &s[new_x];
        }
        if(IsSeqIc(s, end_match_str, m_prop->m_ignoreCase))
          count++;
        else if(IsSeqIc(s, match_str, m_prop->m_ignoreCase))
          count--;
        if(!count)
        { /* We are Done */
          *x = l->TextPosToScreenPos(new_x);
          *y = new_y;
          if (startLen)
          {
            *startLen = strlen(match_str);
          }
          if (endLen)
          {
            *endLen = strlen(end_match_str);
          }
          return l;
        }
      }
      while(1);
    }
  }
  return NULL;
#undef doc
}

TxtLine *WainDoc::FindMatchBlock(TxtLine *l, int *x, int *y)
{
#define doc this // needed by SEPS
   int new_x = l->ScreenPosToTextPos(*x);
   // int new_y = *y;
   if(new_x > l->GetTextLen())
      return 0;
   // int count = 1;
   std::string CurrentWord;
   if(!GetCurrentWord(CurrentWord, new_x, l))
      return 0;
   std::list<std::string>& bbw = m_prop->m_blockBeginWords;
   std::list<std::string>& bew = m_prop->m_blockEndWords;
   bool beginBlock = std::find(bbw.begin(), bbw.end(), CurrentWord) != bbw.end();
   bool endBlock = std::find(bew.begin(), bew.end(), CurrentWord) != bew.end();
   if (beginBlock)
   {

   }
   else if (endBlock)
   {

   }
   return 0;
#undef doc
}

EndLineStatusType WainDoc::GetEndLineStatus(TxtLine *line) const
//  Description:
//    Finds which color is to be used when starting to draw the next line.
//  Parameters:
//    line:
{
  EndLineStatusType els;
  EndLineStatusType prev_els = GetPrevEls(line);
  els = line->GetTextColor(m_view->m_lineColor, prev_els, TRUE);
  return els;
}

void WainDoc::GetExtType(const char *aPathName, bool aAll)
{
   char ext[_MAX_PATH] = "", name[_MAX_PATH] = "";
   if(aPathName)
   {
      MySplitPath(aPathName, SP_FILE, name);
      MySplitPath(aPathName, SP_EXT, ext);
   }
   else
   {
      MySplitPath(GetPathName(), SP_FILE, name);
      MySplitPath(GetPathName(), SP_EXT, ext);
   }
   // int old_m_propIndex = m_propIndex;
   m_propIndex = wainApp.gs.GetPropIndex(name, ext);
   // if (old_m_propIndex != m_propIndex)
   {
      m_prop = wainApp.gs.m_docProp[m_propIndex];
      if (aAll)
      { /* Update the end line status */
         TxtLine* l;
         for (l = m_line.m_next; l != &m_line; l = l->m_next)
            GetEndLineStatus(l);
      }
   }
   if(m_view)
      m_view->UpdateFont();
}

void WainDoc::SetMaxLength(int len)
{
  if(len > m_maxLen)
  {
    m_maxLen = len;
    if(m_view)
      m_view->SetHorzSize(m_maxLen);
  }
}

UINT AFXAPI AfxGetFileTitle(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);

void WainDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
  // store the path fully qualified
  if(!m_isFtpFile)
  {
    CDocument::SetPathName(lpszPathName, bAddToMRU);
    return;
  }
  m_strPathName = lpszPathName;
  ASSERT(!m_strPathName.IsEmpty());       // must be set to something
  m_bEmbedded = FALSE;
  ASSERT_VALID(this);

  // set the document title based on path name
  TCHAR szTitle[_MAX_FNAME];
  if (AfxGetFileTitle(lpszPathName, szTitle, _MAX_FNAME) == 0)
    SetTitle(szTitle);

  ASSERT_VALID(this);
}

void WainDoc::ReCalcTabLen(void)
{
  TxtLine *l;
  for(l = GetLineNo(0); l != NULL; l = GetNext(l))
    l->CalcTabLen();
}

TxtLine *WainDoc::GotoBlockStart(TxtLine *l, int *x, int *y)
{
   std::string &Start = m_prop->m_blockBegin;
   std::string &End = m_prop->m_blockEnd;

   if(Start.empty() || End.empty())
      return 0;

   const char *s = l->GetText();
   int count = 1;
   int new_x = 0;
   int new_y = *y;
   bool first_line = true;

   do
   {
      while(*s == 0)
      {
         l = GetPrev(l);
         if(!l)
           return NULL;
         new_y--;
         new_x = 0;
         s = l->GetText();
         first_line = FALSE;
      }
      if(IsSeqIc(s, Start.c_str(), m_prop->m_ignoreCase))
         count--;
      else if(!first_line && IsSeqIc(s, End.c_str(), m_prop->m_ignoreCase))
         count++;
      if(!count)
      { /* We are Done */
         *x = l->TextPosToScreenPos(new_x);
         *y = new_y;
         return l;
      }
      s++;
      new_x++;
   }
   while(1);
}

TxtLine *WainDoc::GotoBlockEnd(TxtLine *l, int *x, int *y)
{
  std::string &Start = m_prop->m_blockBegin;
  std::string &End = m_prop->m_blockEnd;

  if(End.empty() || Start.empty())
    return 0;
  const char *s = l->GetText();
  int count = 1;
  int new_x = 0;
  int new_y = *y;
  BOOL first_line = TRUE;
  do
  {
    while(*s == 0)
    {
      l = GetNext(l);
      if(!l)
        return NULL;
      new_y++;
      new_x = 0;
      s = l->GetText();
      first_line = FALSE;
    }
    if(!first_line && IsSeqIc(s, Start.c_str(), m_prop->m_ignoreCase))
      count++;
    else if(IsSeqIc(s, End.c_str(), m_prop->m_ignoreCase))
      count--;
    if(!count)
    { /* We are Done */
      *x = l->TextPosToScreenPos(new_x);
      *y = new_y;
      return l;
    }
    s++;
    new_x++;
  }
  while(1);
}


#ifdef _DEBUG
void WainDoc::AssertValid() const
{
  CDocument::AssertValid();
}

void WainDoc::Dump(CDumpContext &dc) const
{
  CDocument::Dump(dc);
}
#endif //_DEBUG
