//=============================================================================
// This source code file is a part of Wain.
// It implements various editor functions, which belongs to WainViewClass.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\Wain.h"

#include ".\..\src\WainDoc.h"
#include ".\..\src\WainView.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\taglist.h"
#include ".\..\src\WainSearch.h"
#include ".\..\src\Project.h"
#include ".\..\src\DocProp.h"
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL WainView::SearchFunc(const char* _string, unsigned int _flags)
//  Description:
//    Message handler, called from main_frame_class::SearchFunc()
//    from the Search and Search&Replase dialogs to find the next match
//  Parameters:
//    _string: The text to find.
//    _flags:  A combination of the SEARCH_xxx flags from WainDoc.h
{
   BOOL ret;
   m_markWordAction = NO_ACTION;
   m_searchString = _string;
   TxtLine *l;
   WainDoc *doc = GetDocument();
   int i = m_lineNo;
   int old_column = m_columnNo;
   size_t TextLen = 0;
   if(!m_currentTextLine)
      m_currentTextLine = doc->GetLineNo(m_lineNo);
   l = doc->FindString(m_currentTextLine, m_searchString, &m_columnNo, &i, _flags, &TextLen);
   m_searchFlags = (_flags) & (SEARCH_IGNORE_CASE | SEARCH_USE_REGEX);
   if(l)
   {
      m_undoList.AddEntry(UNDO_START_ENTRY, 0, 0);
      m_undoList.AddEntry(UNDO_MOVE_ENTRY, m_columnNo - old_column, i);
      m_currentTextLine = l;
      m_lineNo += i;
      HandlePostActions(PA_SCROLL_TO_VISIBLE);
      SaveMarkToUndo();
      RemoveMark();
      m_markStartX = m_columnNo;
      m_markEndX = m_columnNo + TextLen;
      m_markStartY = m_markEndY = m_lineNo;
      m_markType = CUA_MARK;
      DrawMarkLines(m_lineNo, m_lineNo);
      ret = TRUE;
   }
   else
   {
      SetStatusText("Did not find: %s", _string);
      RemoveMark();
      ret = FALSE;
  }
  return ret;
}

void WainView::SearchNext(void)
//  Description:
//    Message handler, find the next match
{
   WainDoc *doc = GetDocument();
   int i = m_lineNo;
   int old_column = m_columnNo;
   TxtLine *l;
   l = doc->FindString(m_currentTextLine, m_searchString, &m_columnNo, &i, SEARCH_FORWARD | m_searchFlags);
   if(l)
   {
      HandlePreActions(PA_REMOVE_CURSOR);
      m_undoList.AddEntry(UNDO_MOVE_ENTRY, m_columnNo - old_column, i);
      m_currentTextLine = l;
      m_lineNo += i;
      RemoveMark();
      m_markType = COLUMN_MARK;
      m_markStartX = m_columnNo;
      m_markEndX = m_columnNo + strlen(m_searchString);
      m_markStartY = m_markEndY = m_lineNo;
      DrawMarkLines(m_markStartY, m_markEndY);
      HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SCROLL_TO_VISIBLE);
   }
   else
   {
      HandlePreActions(PA_REMOVE_CURSOR);
      RemoveMark();
      HandlePostActions(PA_SET_CURSOR);
   }
}

void WainView::SearchPrev(void)
//  Description:
//    Message handler, find the previous match.
//  Parameters:
//
{
  WainDoc *doc = GetDocument();
  int i = m_lineNo;
  int old_column = m_columnNo;
  TxtLine *l;
  l = doc->FindString(m_currentTextLine, m_searchString, &m_columnNo, &i, SEARCH_BACKWARD | m_searchFlags);
  if(l)
  {
    HandlePreActions(PA_REMOVE_CURSOR);
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, m_columnNo - old_column, i);
    m_currentTextLine = l;
    m_lineNo += i;
    RemoveMark();
    m_markType = COLUMN_MARK;
    m_markStartX = m_columnNo;
    m_markEndX = m_columnNo + strlen(m_searchString);
    m_markStartY = m_markEndY = m_lineNo;
    DrawMarkLines(m_markStartY, m_markEndY);
    HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SCROLL_TO_VISIBLE);
  }
  else
  {
    HandlePreActions(PA_REMOVE_CURSOR);
    RemoveMark();
    HandlePostActions(PA_SET_CURSOR);
  }
}

BOOL WainView::ReplaceFunc(const char *_string[], unsigned int flags)
//  Description:
//    Message handler, called from main_frame_class::ReplaceFunc()
//    from the Search&Replace dialog to replace text
//  Parameters:
//    _string: The text to insert.
//    flags:  A combination of the SEARCH_xxx flags from WainDoc.h
{
  if(GetDocument()->GetReadOnly() || GetDocument()->m_isDebugFile)
  {
    SetStatusText("You are no supposed to modify readonly files!");
    return FALSE;
  }
  if(m_markStartY != m_lineNo || m_markEndY != m_lineNo || m_markStartX != m_columnNo)
    RemoveMark();
  if (flags & REPLACE_ALL)
  {
      flags &= ~REPLACE_ALL;
      do
      {
         if(m_markType != CUA_MARK && !SearchFunc(_string[0], flags))
         {
            UpdateAll();
            return false;
         }
         m_markWordAction = NO_ACTION;
         UndoCutTextEntryType *c = new UndoCutTextEntryType;
         c->m_text = new char [m_markEndX - m_markStartX + 1];
         int n;
         for(n = 0; n < m_markEndX - m_markStartX; n++)
           c->m_text[n] = m_currentTextLine->GetTextAt(m_markStartX + n);
         c->m_text[n] = '\0';
         c->m_x = m_columnNo;
         c->m_y = m_lineNo;
         m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, c);
         m_currentTextLine->DeleteAt(m_markStartX, m_markEndX);
         m_currentTextLine->InsertAt(m_markStartX, _string[1]);
         m_undoList.AddInsertEntry(0, 0, m_columnNo, m_lineNo, _string[1]);
         uint32_t toMove = max(min(strlen(_string[0]), strlen(_string[1])), 1);
         m_columnNo += toMove;
         m_undoList.AddEntry(UNDO_MOVE_ENTRY, toMove, 0);
         flags &= ~SEARCH_FIRST;
      }
      while(SearchFunc(_string[0], flags) && (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000)));
      UpdateAll();
      return false;
  }
  else
  {
      if(m_markType != CUA_MARK)
      {
         return SearchFunc(_string[0], flags);
      }
      m_markWordAction = NO_ACTION;
      UndoCutTextEntryType *c = new UndoCutTextEntryType;
      c->m_text = new char [m_markEndX - m_markStartX + 1];
      int n;
      for(n = 0; n < m_markEndX - m_markStartX; n++)
        c->m_text[n] = m_currentTextLine->GetTextAt(m_markStartX + n);
      c->m_text[n] = '\0';
      c->m_x = m_columnNo;
      c->m_y = m_lineNo;
      m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, c);
      m_currentTextLine->DeleteAt(m_markStartX, m_markEndX);

      m_currentTextLine->InsertAt(m_markStartX, _string[1]);

      m_undoList.AddInsertEntry(0, 0, m_columnNo, m_lineNo, _string[1]);

      m_columnNo += max(min(strlen(_string[0]), strlen(_string[1])), 1);
      m_undoList.AddEntry(UNDO_MOVE_ENTRY, strlen(_string[1]), 0);
      UpdateAll();
      return SearchFunc(_string[0], flags);
   }
}

void WainView::InsertToggle(void)
//  Description:
//    Toggles from insert mode to overwrite and backwards.
{
  if(m_insert)
    m_insert = FALSE;
  else
    m_insert = TRUE;
  m_undoList.AddEntry(UNDO_INSERT_TOGGLE_ENTRY, 0, 0);
  HandlePreActions(PA_REMOVE_CURSOR);
  CreateSolidCaret(m_insert ? 2 : m_charWidth, m_lineHeight);
  GetMf()->SetInsertStatus(m_insert);
  HandlePreActions(PA_SET_CURSOR);
}

bool IsFile(const char *filename)
//  Description:
//    Function used to check if filename is a file
//  Parameters:
//    filename: The name to check
{
  struct _stat buffer;
  if(_stat(filename, &buffer))  /* _stat() return 0 on ok */
    return false;
  if(buffer.st_mode & _S_IFDIR) /* Dirs is not a file */
    return false;
  return true;
}

BOOL WainView::CheckFileName(CString &aDest, const CString &aName, int aToolNo, const CString &aMakeFile)
//  Description:
//    Helper function for FileOpenInLine().
//  Parameters:
//    name, the (partial) filename.
//    tool_no, a index in the global Tools, or -1
//    make_file, the name of the make_file.
{
  std::string LName = (const char *)aName;
  while(LName.size() && LName[0] == ' ' || LName[0] == '\t')
     LName = LName.substr(1);
  if(aName.IsEmpty())
     return FALSE;
  // First check if we can complete the path with the path of the tool.
  if(aToolNo != -1)
  {
    aDest = wainApp.gs.m_toolParm[aToolNo].m_lastRunPath.c_str();
    if(aDest.GetLength() && aDest.Right(1) != '\\')
      aDest += "\\";

    aDest += LName.c_str();

    if(IsFile(aDest))
    {
      return TRUE;
    }
  }
  // Then check if the makefile can be used to complete the filename
  else if(aMakeFile.GetLength())
  {
    MySplitPath(GetMf()->m_navigatorDialog.m_project->GetMakeFile().c_str(), SP_DRIVE | SP_DIR, aDest);
    aDest += LName.c_str();
    if(IsFile(aDest))
      return TRUE;
  }
  // Then check if the path is relative to my path
  const char *mfn = GetDocument()->GetPathName();
  if(mfn && strlen(mfn))
  {
    MySplitPath(mfn, SP_DRIVE | SP_DIR, aDest);
    aDest += LName.c_str();
    if(IsFile(aDest))
      return TRUE;
  }
  // Then check the global include path
  aDest = wainApp.gs.m_incPath.c_str();
  if(aDest.GetLength() && aDest.Right(1) != '\\')
    aDest += "\\";
  aDest += LName.c_str();
  if(IsFile(aDest))
    return TRUE;

  // And check the project include path.
  const char * pip = GetMf()->m_navigatorDialog.m_project->GetIncPath().c_str();
  if(pip && strlen(pip))
  {
    aDest = pip;
    if(aDest.GetLength() && aDest.Right(1) != '\\')
      aDest += "\\";
    aDest += LName.c_str();
    if(IsFile(aDest))
      return TRUE;
  }
  /* Check files in our project */
  if(GetMf()->m_navigatorDialog.m_project->CheckFileName(aDest, LName))
    return TRUE;

  // Try the name without any path
  std::string::size_type pos = LName.find_last_of("\\/");
  if(pos != std::string::npos && GetMf()->m_navigatorDialog.m_project->CheckFileName(aDest, LName.substr(pos + 1)))
     return TRUE;

  // Finnaly  check if it's a full file path.
  if(IsFile(LName.c_str()))
  {
    aDest = LName.c_str();
    return TRUE;
  }

  return FALSE; // :-(
}

void WainView::FileOpenInLine(void)
{
   DoOpenFileInLine();
}

BOOL WainView::DoOpenFileInLine(void)
//  Description:
//    Opens the file which name (& path) is in the current line.
{
  CString temp, temp2, file_name = "", my_file_name; // Fixme, to string
  int my_tool_no = -1;
  unsigned long LineNo = 0;
  size_t i, j;
  char *end;
  const char * const delims[] = {":", "()", "\"", "<>", " :", ""};
  const char *mfile;
  CString make_file = "";
  BOOL ret = FALSE;
  BOOL Success = FALSE;

  MySplitPath(GetDocument()->GetPathName(), SP_FILE, my_file_name);
  for(i = 0; i < wainApp.gs.m_toolParm.Size() && my_tool_no == -1; i++)
  {
    if(!stricmp(my_file_name, wainApp.gs.m_toolParm[i].m_menuText.c_str()))
      my_tool_no = i;
  }
   if(my_tool_no != -1)
   {
      if (OpenNewToolFile(my_tool_no))
      {
         return true;
      }
   }
   WainDoc* myDoc = GetDocument();

   if (myDoc->m_isProject)
   {
      mfile = GetMf()->m_navigatorDialog.m_project->GetMakeFile().c_str();
      if (mfile)
         MySplitPath(mfile, SP_FILE, make_file);
      if (stricmp(make_file, my_file_name))
         make_file = "";
      const char *ss = m_currentTextLine->GetText();
      std::string fn;
      uint32_t lineNo = 0;
      uint32_t columnNo = 0;
      if (GetMf()->m_navigatorDialog.m_project->GetFileInLine(ss, fn, lineNo, columnNo, myDoc->m_isMake, myDoc->m_nr))
      {
         CString F;
         my_file_name = fn.c_str();
         if(CheckFileName(F, my_file_name, -1, make_file))
         {
           WainDoc* doc = wainApp.OpenDocument(F);
           if(doc)
           {
             lineNo--;

             doc->m_view->GotoLineNo(lineNo, columnNo - 1);
             return TRUE;
           }
         }
         std::string debugPath = GetDocument()->GetDebugPath();
         if (!debugPath.empty())
         {
            debugPath += "\\";
            debugPath += fn;
            WainDoc* doc = wainApp.OpenDocument(debugPath.c_str());
            if(doc)
            {
              lineNo--;
              doc->m_view->GotoLineNo(lineNo, columnNo - 1);
              return TRUE;
            }
         }
      }
   }
  for(i = 0; delims[i][0] && !Success; i++)
  {
    char *tt = strdup(m_currentTextLine->GetText()), *s;
    s = strtok(tt, delims[i]);
    while(s)
    {
      if(!Success && CheckFileName(file_name, s, my_tool_no, make_file))
      {
        Success = TRUE;
      }
      else if(!LineNo && ((LineNo = strtol(s, &end, 0)) != 0))
      {
        if(!end || *end)
          LineNo = 0; // False alarm
      }
      s = strtok(NULL, delims[i]);
    }
    free(tt);
  }

  if(!Success)
  { // First round did not find it
    temp = m_currentTextLine->GetText();
    j = 0;

    while(j < temp.GetLength())
    {
      temp2 = "";
      for(i = 0; i < _MAX_PATH && j < temp.GetLength() && temp[(int )j] != ':'; i++, j++)
      {
        temp2 += temp[(int )j];
        if(j + 2 < temp.GetLength() && temp[int(j + 1)] == ':' && temp[int(j + 2)] == '\\')
        {
          i++;
          j++;
          temp2 += temp[int(j)];
        }
      }
      if(CheckFileName(file_name, temp2, my_tool_no, make_file))
        Success = TRUE;
      else if(!LineNo && ((LineNo = strtol(temp2, &end, 0)) != 0))
      {
        if(!end || *end)
          LineNo = 0; // False alarm
      }
      if(j < temp.GetLength() &&  temp[int(j)] == ':')
        j++;
    }
  }
  if(Success)
  {
    WainDoc *doc;
    doc = wainApp.OpenDocument(file_name);
    if(doc)
    {
      if(LineNo == 0)
      {
         try
         {
            std::regex reg("[0-9]+");
            std::cmatch cm;
            std::regex_match(m_currentTextLine->GetText(), cm, reg);
            if (cm.size() > 0)
            {
               const char *ns = cm.str(1).c_str();
               LineNo = strtol(ns, NULL, 0);
            }
         }
         catch (const std::regex_error&)
         {
         }
      }
      if(LineNo)
      {
        LineNo--;
        doc->m_view->GotoLineNo(LineNo);
      }
    }
    ret = TRUE;
  }
  return ret;
}

void WainView::CopyToClipboard(void)
//  Description:
//    Message handler, copy the marked area to the clipboard
{
  CopyToClip(UNDO_COPY_CLIP_ENTRY);
}

void WainView::CopyToClip(UndoEntryTypeType Undo_type)
//  Description:
//    Copy the marked area to clipboard.
//  Parameters:
//    Undo_type: Tell if Undo is a copy or a cut.
{
  int size;
  int n;

  if(m_markType == NO_MARK)
  {
    SetStatusText("No mark to copy");
    return;
  }

  if(!OpenClipboard())
  {
    WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
    return;
  }
  // Find the current clipboard format
  MarkTypeType mt = COLUMN_MARK;
  HGLOBAL data = ::GetClipboardData(WainColumnTextFormat);
  if(!data)
  {
    data = ::GetClipboardData(CF_TEXT);
    if(!data)
      mt = NO_MARK;
    else
      mt = CUA_MARK;
  }
  if(mt != NO_MARK)
  { /* Store the current clipboard data in an Undo entry */
    char *s = (char *)::GlobalLock(data);
    if(s)
    {
      size = strlen(s) + 1;
      UndoCopyClipOldDataEntryType *old_entry = new UndoCopyClipOldDataEntryType;
      if(old_entry)
      {
        old_entry->m_sX = m_markStartX;
        old_entry->m_eX = m_markEndX;
        old_entry->m_sY = m_markStartY;
        old_entry->m_eY = m_markEndY;
        old_entry->m_type = mt;
        old_entry->m_buffer = new char [size];
        strcpy(old_entry->m_buffer, s);
        m_undoList.AddEntry(UNDO_COPY_CLIP_OLD_DATA_ENTRY, 0, 0, mt, old_entry);
      }
    }
    GlobalUnlock(data);
  }
  if(!::EmptyClipboard())
  {
    WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
    ::CloseClipboard();
    return;
  }
  // Find the size of the data to be copied to the clipboard
  WainDoc *doc = GetDocument();
  TxtLine *l = doc->GetLineNo(m_markStartY);
  size = 0;
  if(m_markType == COLUMN_MARK)
  {
    for(n = m_markStartY; n <= m_markEndY && l != NULL; n++, l = doc->GetNext(l))
      size += l->GetRawLen(m_markStartX, m_markEndX, TRUE) + 2;
  }
  else
  {
    if(m_markEndY == m_markStartY)
    {
      size = l->GetRawLen(m_markStartX, m_markEndX, FALSE);
    }
    else
    {
      size += l->GetRawLen(m_markStartX, INT_MAX, FALSE) + 2;
      l = doc->GetNext(l);
      for(n = m_markStartY + 1; n < m_markEndY && l != NULL; n++, l = doc->GetNext(l))
        size += l->GetRawLen(0, INT_MAX, FALSE) + 2;
      if(l)
        size += l->GetRawLen(0, m_markEndX, FALSE);
    }
  }
  size++; // Add one for the '\0'

  HGLOBAL hData = ::GlobalAlloc(GMEM_DDESHARE, size);
  if (hData == NULL)
  {
    WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
    ::CloseClipboard();
    return;
  }

  char *out_str = (char *)::GlobalLock(hData);
  if(!out_str)
  {
    WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
    ::CloseClipboard();
    return;
  }
  // Copy text to out_str
  out_str[0] = '\0';
  l = doc->GetLineNo(m_markStartY);
  int i = 0;
  if(m_markType == COLUMN_MARK)
  {
    for(n = m_markStartY; n <= m_markEndY && l != NULL; n++, l = doc->GetNext(l))
    {
      i += l->CopyRaw(&out_str[i], m_markStartX, m_markEndX, TRUE);
      strcpy(&out_str[i], "\r\n");
      i += 2;
    }
  }
  else
  {
    if(m_markEndY == m_markStartY)
    {
      i = l->CopyRaw(out_str, m_markStartX, m_markEndX, FALSE);
    }
    else
    {
      i += l->CopyRaw(out_str, m_markStartX, INT_MAX, FALSE);
      strcpy(&out_str[i], "\r\n");
      i += 2;
      l = doc->GetNext(l);
      for(n = m_markStartY + 1; n < m_markEndY && l != NULL; n++, l = doc->GetNext(l))
      {
        i += l->CopyRaw(&out_str[i], 0, INT_MAX, FALSE);
        strcpy(&out_str[i], "\r\n");
        i += 2;
      }
      if(l)
        i += l->CopyRaw(&out_str[i], 0, m_markEndX, FALSE);
    }
  }

  out_str[i++] = '\0';

  ASSERT(i == size);

  // Add a Undo entry, with the current mark settings and the data copied to the clipboard
  UndoCopyClipEntryType *Undo_entry = new UndoCopyClipEntryType;
  if(Undo_entry)
  {
    Undo_entry->m_sX = m_markStartX;
    Undo_entry->m_eX = m_markEndX;
    Undo_entry->m_sY = m_markStartY;
    Undo_entry->m_eY = m_markEndY;
    Undo_entry->m_type = m_markType;
    Undo_entry->m_buffer = new char [size];
    strcpy(Undo_entry->m_buffer, out_str);
    m_undoList.AddEntry(Undo_type, 0, 0, m_markType, Undo_entry);
  }
  // Finally set the clipboard data
  if(!::SetClipboardData(m_markType == CUA_MARK ? CF_TEXT : WainColumnTextFormat, hData))
    WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
  if(m_markType == COLUMN_MARK)
  { /* If we are putting the data as our column format, add it as normal text, so other applications can use it */
    data = ::GlobalAlloc(GMEM_DDESHARE, size);
    if (data == NULL)
    {
      WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
      ::CloseClipboard();
      return;
    }
    char *str = (char *)::GlobalLock(data);
    if(!str)
    {
      WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
      ::CloseClipboard();
      return;
    }
    memcpy(str, out_str, size);
    ::SetClipboardData(CF_TEXT, data);
  }
  if(!::CloseClipboard())
    WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
  if(Undo_type != UNDO_CUT_ENTRY)
    RemoveMarkSticky();
}

void WainView::PasteClip(void)
//  Description:
//    Paste from clipboard.
{
  WainDoc *doc = GetDocument();
  if(doc->GetReadOnly())
  {
    SetStatusText("Can't modify ReadOnly file");
    return;
  }
  MarkTypeType mt = COLUMN_MARK;
  if(!OpenClipboard())
  {
    WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
    return;
  }

  HGLOBAL data = ::GetClipboardData(WainColumnTextFormat);
  if(!data)
  {
    data = ::GetClipboardData(CF_TEXT);
    if(!data)
    {
      WainMessageBox(this, "Unable to paste from clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
      ::CloseClipboard();
      return;
    }
    mt = CUA_MARK;
  }
  CString temp = "";
  char *rest_text = (char *)malloc(m_maxLineLen);
  *rest_text = 0;

  int i;
  char *s = (char *)data;
  BOOL first = TRUE;
  TxtLine *l = m_currentTextLine;
  int y_o = 0;
  int x_o = 0;
  UndoCutTextEntryType *cp;

  while(*s != '\0')
  {
    temp = "";
    for(i = 0; !strchr("\r\n", *s); i++, s++)
      temp += *s;

    if(mt == CUA_MARK)
    {
      if(first)
      {
        first = FALSE;
        l->RemoveFrom(m_columnNo, rest_text);
        cp = new UndoCutTextEntryType;
        cp->m_text = new char [strlen(rest_text) + 1];
        strcpy(cp->m_text, rest_text);
        cp->m_x = m_columnNo;
        cp->m_y = m_lineNo;
        m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, cp);
        l->InsertAt(m_columnNo, temp);

        m_undoList.AddInsertEntry(0, 0, m_columnNo, m_lineNo, temp);
      }
      else
      {
        l->InsertAt(0, temp);
        m_undoList.AddInsertEntry(0, 0, 0, m_lineNo + y_o, temp);
      }
      if(*s == '\r' || *s == '\n')
      {
        l = doc->InsertLine(l, "");
        if(*s == '\r')
          s++;
        if(*s == '\n')
          s++;
        m_undoList.AddEntry(UNDO_INSERT_LINE_ENTRY, 0, 0, m_lineNo + ++y_o);
      }
    }
    else // COLUMN_MARK
    {
      if(!l)
      {
        l = doc->InsertLine(l, "");
        m_undoList.AddEntry(UNDO_INSERT_LINE_ENTRY, 0, 0, m_lineNo + y_o);
      }
      l->InsertAt(m_columnNo, temp);
      x_o = m_columnNo + i;
      m_undoList.AddInsertEntry(0, 0, m_columnNo, m_lineNo + y_o, temp);
      l = m_lineNo + y_o < doc->m_lineCount - 1 ? doc->GetNext(l) : NULL;
      if(*s == '\n' || *s == '\r')
      {
        if(*s == '\r')
          s++;
        if(*s == '\n')
          s++;
      }
      y_o++;
    }
  }
  if(mt == CUA_MARK)
  {
    x_o = l->GetTabLen();
    l->InsertAt(l->GetTabLen(), rest_text);
    m_undoList.AddInsertEntry(0, 0, l->GetTabLen() - strlen(rest_text), m_lineNo + y_o, rest_text);
  }
  SaveMarkToUndo();
  HandlePostActions(PA_REMOVE_MARK);
  if(mt == CUA_MARK && wainApp.gs.m_stickyMark)
  {
    m_markType = mt;
    m_markStartX = m_columnNo;
    m_markEndX = x_o;
    m_markStartY = m_lineNo;
    m_markEndY = m_lineNo + y_o - (mt == COLUMN_MARK ? 1 : 0);
  }
  else
  {
    m_markType = NO_MARK;
    m_markStartX = 0;
    m_markEndX = 0;
    m_markStartY = 0;
    m_markEndY = 0;
  }
  UpdateAll();
  SetScrollInfo();
  if(!::CloseClipboard())
    WainMessageBox(this, "Failed to paste from clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
  free(rest_text);
}

void WainView::MarkCut(void)
{
  MarkCutDelete(TRUE);
}

void WainView::MarkDelete(void)
{
  MarkCutDelete(FALSE);
}

void WainView::MarkCutDelete(BOOL copy)
//  Description:
//    Cuts the marked area.
//    If copy then the area is first copied to clipboard.
{
  WainDoc *doc = GetDocument();
  if(doc->GetReadOnly())
  {
    SetStatusText("Can't modify ReadOnly file");
    return;
  }
  if(m_markType == NO_MARK)
  {
    SetStatusText("No mark");
    return;
  }
  HandlePreActions(PA_REMOVE_CURSOR);
  UndoEntryClass *ume = m_undoList.AddEntry(UNDO_MOVE_ENTRY, 0, 0); /* We need to insert this now, will fill it in later */
  if(copy)
    CopyToClip(UNDO_CUT_ENTRY);

  if(!m_specialClip)
    SaveMarkToUndo();
  TxtLine *line = doc->GetLineNo(m_markStartY);
  int i;
  for(i = m_markStartY; line && i <= m_markEndY; i++)
  {
    if(m_markType == CUA_MARK)
    {
      if(i == m_markStartY && i == m_markEndY)
      {
        if(!copy)
          AddCutTextUndoEntry(line, i, m_markStartX, m_markEndX);
        line->DeleteAt(m_markStartX, m_markEndX);
        line = doc->GetNext(line);
      }
      else if(i == m_markStartY)
      {
        if(m_markStartX == 0)
        {
          if(!copy)
            AddRemoveLineUndoEntry(line, m_markStartY);
          line = doc->RemoveLine(line);
        }
        else
        {
          if(!copy)
            AddCutTextUndoEntry(line, i, m_markStartX, line->GetTabLen());
          line->DeleteAt(m_markStartX, line->GetTabLen());
          line = doc->GetNext(line);
        }
      }
      else if(i == m_markEndY)
      {
        if(MIN(m_markEndX, line->GetTabLen()))
        {
          if(!copy)
            AddCutTextUndoEntry(line, i, 0, MIN(m_markEndX, line->GetTabLen()));
          line->DeleteAt(0, MIN(m_markEndX, line->GetTabLen()));
          line = doc->GetNext(line);
        }
      }
      else
      {
        if(!copy)
          AddRemoveLineUndoEntry(line, m_markStartY);
        line = doc->RemoveLine(line);
      }
    }
    else
    {
      if(!copy)
        AddCutTextUndoEntry(line, i, m_markStartX, m_markEndX);
      line->DeleteAt(m_markStartX, m_markEndX);
      line = doc->GetNext(line);
    }
  }
  if(m_markType == CUA_MARK && m_markStartX && (m_markStartY != m_markEndY))
  { // We have to concat the first & the last line
    TxtLine *f = doc->GetLineNo(m_markStartY);
    doc->JoinLine(f);
  }
  if(m_markStartY >= doc->m_lineCount)
    m_markStartY = doc->m_lineCount - 1;
  if(ume)
  {
    ASSERT(ume->m_checkMark == 0xDEADBEEF);
    ume->m_x = m_markStartX - m_columnNo;
    ume->m_y = m_markStartY - m_lineNo;
    ASSERT(ume->m_checkMark == 0xDEADBEEF);
  }

  m_columnNo = m_markStartX;
  m_lineNo   = m_markStartY;
  m_currentTextLine   = doc->GetLineNo(m_lineNo);
  m_markStartX = m_markEndX = m_columnNo;
  m_markStartY = m_markEndY = m_lineNo;
  m_markType = NO_MARK;
  UpdateAll();
  SetScrollInfo();
  HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE | PA_UPDATE_CURSOR_POS | PA_SET_TAG);
  if(GetFocus() == this)
    GetMf()->SetMarkStatus(FALSE);
}

void WainView::SaveMarkToUndo(void)
{
/* Save the current mark settings to a Undo entry */
  UndoMarkEntryType *Undo_entry = new UndoMarkEntryType;
  if(Undo_entry)
  {
    Undo_entry->m_sX = m_markStartX;
    Undo_entry->m_eX = m_markEndX;
    Undo_entry->m_sY = m_markStartY;
    Undo_entry->m_eY = m_markEndY;
    Undo_entry->m_type = m_markType;
    m_undoList.AddEntry(UNDO_MARK_ENTRY, 0, 0, m_markType, Undo_entry);
  }
}

void WainView::PrintFile(void)
{
  WainMessageBox(this, "Feature not implemented", IDC_MSG_OK, IDI_INFORMATION_ICO);
}

void WainView::ScrollLineUp(void)
//  Description:
//    Message handler, scoll one line up, the cursor will stay in the same line (if posible)
{
   RECT cr;
   GetClientRect(&cr);
   if(m_lineNo)
   {
      HandlePreActions(PA_REMOVE_CURSOR);
      int flags = 0;
      if(!m_yOffset || m_lineNo + 1 >= (m_yOffset + cr.bottom)/m_lineHeight)
      {  // The curor can not stay in the same line, so move it one up.
         m_lineNo--;
         m_undoList.AddEntry(UNDO_MOVE_ENTRY, 0, -1);
         m_currentTextLine = GetDocument()->GetPrev(m_currentTextLine);
         flags |= PA_SET_TAG;
      }
      if(m_yOffset)
      { // Scroll one line
         CDC *dc = GetDC();
         CRgn rgn;
         dc->ScrollDC(0, m_lineHeight, &cr, &cr, &rgn, NULL);
         m_yOffset -= m_lineHeight;
         SetScrollPos(SB_VERT, m_yOffset, TRUE);
         int i = m_yOffset/m_lineHeight;

         TxtLine *l = GetDocument()->GetRelLineNo(i - m_lineNo, m_currentTextLine);
         for(int j = 0; j < 2 && l; j++, l = GetDocument()->GetNext(l))
         {
            PutText(dc, l, i + j);
         }
         ReleaseDC(dc);
      }
      HandlePostActions(PA_SET_CURSOR | flags);
      UpdateTabViewPos();
   }
}

void WainView::ScrollLineDown(void)
//  Description:
//    Message handler, scoll one line down, the cursor will stay in the same line (if posible)
{
   RECT cr;
   GetClientRect(&cr);
   if(m_lineNo < GetDocument()->m_lineCount - 1)
   {
      HandlePreActions(PA_REMOVE_CURSOR);
      int max = m_maxScroll.cy; // GetScrollLimit(SB_VERT);
      int flags = 0;
      if((m_lineNo - 1)*m_lineHeight < m_yOffset)
      {
         m_lineNo++;
         m_undoList.AddEntry(UNDO_MOVE_ENTRY, 0, 1);
         m_currentTextLine = GetDocument()->GetNext(m_currentTextLine);
         flags |= PA_SET_TAG;
      }
      if(m_yOffset < max - m_lineHeight - cr.bottom)
      {
         CDC *dc = GetDC();
         CRgn rgn;
         dc->ScrollDC(0, -m_lineHeight, &cr, &cr, &rgn, NULL);
         m_yOffset += m_lineHeight;
         SetScrollPos(SB_VERT, m_yOffset, TRUE);
         int i = (m_yOffset + cr.bottom)/m_lineHeight - 2;
         TxtLine *l = GetDocument()->GetRelLineNo(i - m_lineNo, m_currentTextLine);

         cr.top = cr.bottom - (m_lineHeight  + 3);
         dc->FillRect(&cr, &m_backBrush);

         for(int j = 0; j < 3 && l; j++, l = GetDocument()->GetNext(l))
         {
            PutText(dc, l, i + j);
         }
         ReleaseDC(dc);
      }
      HandlePostActions(PA_SET_CURSOR | flags);
      UpdateTabViewPos();
   }
}

void WainView::ToolOutputNext(void)
{
  WainDoc *doc = GetDocument();
  int first_line = wainApp.gs.m_debugTools ? 2 : 0;

  if(m_lineNo < first_line)
  {
    m_lineNo = first_line;
    m_currentTextLine = doc->GetLineNo(m_lineNo);
  }

  while(m_lineNo < doc->m_lineCount -1)
  {
    m_lineNo++;
    m_currentTextLine = doc->GetNext(m_currentTextLine);
    ASSERT(m_currentTextLine);
    if(DoOpenFileInLine())
    {
      HandlePostActions(PA_SCROLL_TO_VISIBLE);
      return;
    }
    if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
      SetStatusText("User abort");
      return;
    }
  }

  SetStatusText("No more output");
}

void WainView::ToolOutputPrev(void)
{
  WainDoc *doc = GetDocument();

  int first_line = wainApp.gs.m_debugTools ? 2 : 0;

  while(m_lineNo > first_line)
  {
    m_lineNo--;
    m_currentTextLine = doc->GetPrev(m_currentTextLine);
    ASSERT(m_currentTextLine);
    if(DoOpenFileInLine())
    {
      HandlePostActions(PA_SCROLL_TO_VISIBLE);
      return;
    }
    if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
      SetStatusText("User abort");
      return;
    }
  }
  SetStatusText("No more output");
}

void WainView::MoveToNexTab(void)
{
   int pos = GetDocument()->m_prop->m_tabPos.GetNextTabPos(m_columnNo + 1);

   if(pos >= m_columnNo)
   {
      HandlePreActions(PA_REMOVE_CURSOR);
      m_undoList.AddEntry(UNDO_MOVE_ENTRY, pos - m_columnNo - 1, 0);
      m_columnNo = pos - 1;
      HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE | PA_SET_TAG | PA_REMOVE_MARK_STICKY);
   }
}

void WainView::MoveToPrevTab(void)
{
   int pos = GetDocument()->m_prop->m_tabPos.GetPrevTabPos(m_columnNo + 1);
   if(pos >= 0 && pos <= m_columnNo)
   {
      HandlePreActions(PA_REMOVE_CURSOR);
      m_undoList.AddEntry(UNDO_MOVE_ENTRY, pos - m_columnNo - 1, 0);
      m_columnNo = pos - 1;
      HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE | PA_SET_TAG | PA_REMOVE_MARK_STICKY);
   }
}

void WainView::IndentToNextTab(void)
{
   if(GetDocument()->GetReadOnly())
   {
      SetStatusText("Can't modify ReadOnly file");
      return;
   }

   if(m_columnNo >= m_currentTextLine->GetTabLen())
   {
      return;
   }
   char ch = m_currentTextLine->GetTextAt(m_columnNo);
   int pos = GetDocument()->m_prop->m_tabPos.GetNextTabPos(m_columnNo + 1);
   if(pos < 0)
      return;
   pos--;
   HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK);
   if(ch == ' ' || ch == '\t')
   {
      int off = m_currentTextLine->GetSpaceLenRight(m_columnNo);
      if(off >= (pos - m_columnNo))
      {
         m_undoList.AddEntry(UNDO_MOVE_ENTRY, pos - m_columnNo, 0, 0, 0);
         m_columnNo = pos;
      }
      else
      {
         int num = pos - m_columnNo - off;
         char *t = (char *)malloc(num + 2);
         memset(t, ' ',  num);
         t[num] = 0;
         m_currentTextLine->InsertAt(m_columnNo, t);
         m_undoList.AddInsertEntry(num, 0, m_columnNo, m_lineNo, t);
         m_columnNo = pos;
         free(t);
         PutText(0, m_currentTextLine, m_lineNo);
      }
   }
   else
   {
      int len = -m_currentTextLine->GetWordLenLeft(m_columnNo);
      int num = len + pos - m_columnNo;
      char *t = (char *)malloc(num + 2);
      memset(t, ' ',  num);
      t[num] = 0;
      m_currentTextLine->InsertAt(m_columnNo - len, t);
      m_undoList.AddInsertEntry(num, 0, m_columnNo - len, m_lineNo, t);
      m_columnNo = pos;
      free(t);
      PutText(0, m_currentTextLine, m_lineNo);
   }
   HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE | PA_SET_TAG | PA_REMOVE_MARK_STICKY);
}

void WainView::IndentToPrevTab(void)
{
   if(GetDocument()->GetReadOnly())
   {
      SetStatusText("Can't modify ReadOnly file");
      return;
   }

   if(!m_columnNo)
      return;
   int t_off;
   if(m_currentTextLine->GetTextAt(m_columnNo) == ' ')
      t_off = m_currentTextLine->GetSpaceLenRight(m_columnNo);
   else if(m_currentTextLine->GetTextAt(m_columnNo - 1) != ' ')
      t_off = m_currentTextLine->GetWordLenLeft(m_columnNo);
   else
      t_off = 0;

   int pos = GetDocument()->m_prop->m_tabPos.GetPrevTabPos(m_columnNo + 1 + t_off);
   if(pos <= 0)
      return;

   if(m_currentTextLine->CheckSpace(pos - 1, m_columnNo + t_off))
   {
      HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK);
      int offset = pos - m_columnNo - 1;
      m_currentTextLine->DeleteAt(pos - 1, m_columnNo + t_off);
      m_undoList.AddEntry(UNDO_MOVE_ENTRY, offset, 0);
      for(int j = - t_off; j < -offset; j++)
         m_undoList.AddEntry(UNDO_DELETE_ENTRY, 0, 0, ' ');
      m_columnNo = pos - 1;
      PutText(NULL, m_currentTextLine, m_lineNo);
      HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE | PA_SET_TAG | PA_REMOVE_MARK_STICKY);
   }
}

void WainView::CutSpecial(void)
{
  if(GetDocument()->GetReadOnly())
  {
    SetStatusText("Can't modify ReadOnly file");
    return;
  }
  if(m_markType)
  {
    MarkCut();
  }
  else
  {
    m_specialClip = TRUE;
    m_markType = CUA_MARK;
    m_markStartY = m_lineNo;
    m_markEndY = m_lineNo + 1;
    m_markStartX = 0;
    m_markEndX = 0;
    MarkCut();
    m_markType = NO_MARK;
    m_markStartY = m_markEndY = m_lineNo;
    m_specialClip = FALSE;
  }
}

void WainView::CopySpecial(void)
{
  if(m_markType)
  {
    CopyToClipboard();
  }
  else
  {
    m_specialClip = TRUE;
    m_markType = CUA_MARK;
    m_markStartY = m_lineNo;
    m_markEndY = m_lineNo + 1;
    m_markStartX = m_markEndX = 0;
    CopyToClipboard();
    m_markType = NO_MARK;
    m_markStartY = m_markEndY = m_lineNo;
    m_specialClip = FALSE;
  }
}

void WainView::PasteSpecial(void)
{
  if(GetDocument()->GetReadOnly())
  {
    SetStatusText("Can't modify ReadOnly File");
    return;
  }
  int old_c = m_columnNo;
  m_columnNo = 0;
  PasteClip();
  m_columnNo = old_c;
  HandlePostActions(PA_UPDATE_CURSOR_POS);
}

IMPLEMENT_DYNAMIC(FileInfoDialogClass, CDialog)

BEGIN_MESSAGE_MAP(FileInfoDialogClass, CDialog)
END_MESSAGE_MAP();


FileInfoDialogClass::FileInfoDialogClass(CWnd *parent) : DialogBaseClass(FileInfoDialogClass::IDD, parent)
{
  m_lineCount = 0;
  m_charCount = 0;
  m_readOnly = FALSE;
  m_unixStyle = FALSE;
}

FileInfoDialogClass::~FileInfoDialogClass()
{

}

BOOL FileInfoDialogClass::OnInitDialog(void)
{
  char str[128];
  sprintf(str, "Line Count: %d", m_lineCount);
  CStatic *s = (CStatic *)GetDlgItem(IDC_FILE_INFO_LINES);
  ASSERT(s);
  s->SetWindowText(str);
  sprintf(str, "Size: %d", m_charCount);
  s = (CStatic *)GetDlgItem(IDC_FILE_INFO_CHARS);
  ASSERT(s);
  s->SetWindowText(str);
  return CDialog::OnInitDialog();
}

void FileInfoDialogClass::DoDataExchange(CDataExchange *dx)
{
  CDialog::DoDataExchange(dx);
  DdxCheck(dx, IDC_FILE_INFO_UNIX, m_unixStyle);
  DdxCheck(dx, IDC_FILE_INFO_READONLY, m_readOnly);
}

void FileInfoDialogClass::OnOK(void)
{
  UpdateData(TRUE);
  EndDialog(IDOK);
}

void FileInfoDialogClass::OnCancel(void)
{
  EndDialog(IDCANCEL);
}

void WainView::FileInfo(void)
{
  WainDoc *doc = GetDocument();
  TxtLine *l = doc->GetLineNo(0);
  int LineCount, CharCount;

  for(LineCount = 0, CharCount = 0; l != NULL; l = doc->GetNext(l))
  {
    LineCount++;
    CharCount += l->GetTabLen();
  }
  FileInfoDialogClass fi(this);
  fi.m_lineCount = LineCount;
  fi.m_charCount = CharCount;
  fi.m_readOnly = doc->GetReadOnly();
  fi.m_unixStyle = doc->m_unixStyle;
  if(fi.DoModal() == IDOK)
  {
    doc->m_unixStyle = fi.m_unixStyle;
    if(fi.m_readOnly != doc->GetReadOnly() && !doc->m_isDebugFile && !doc->m_isFtpFile)
    {
      if(doc->GetReadOnly())
      {
        if(doc->m_isFtpFile)
          SetStatusText("Can't change Readonly status on FTP-files");
        else if(SetFileAttributes(doc->GetPathName(), FILE_ATTRIBUTE_NORMAL))
        {
          doc->SetReadOnly(false);
          GetMf()->SetModifiedStatus(this, doc->GetModified());
          ViewListElem *elem = GetMf()->m_viewList.GetViewNr(m_winNr);
          if(elem)
            elem->UpdateStatus();
        }
        else
          WainMessageBox(this, "Unable to remove ReadOnly attribute", IDC_MSG_OK, IDI_WARNING_ICO);
      }
      else
      {
        doc->SetReadOnly(true);
        GetMf()->SetModifiedStatus(this, doc->GetModified());
      }
    }
    GetMf()->SetCrLfStatus(doc->m_unixStyle);
  }
}


void WainView::MoveToMarkStart(void)
{
  if(m_markType == NO_MARK)
  {
    SetStatusText("No Mark");
    return;
  }
  HandlePreActions(PA_REMOVE_CURSOR);
  int dx = m_columnNo - m_markStartX;
  int dy = m_lineNo - m_markStartY;
  m_columnNo = m_markStartX;
  m_lineNo = m_markStartY;
  m_undoList.AddEntry(UNDO_MOVE_ENTRY, -dx, -dy);
  m_currentTextLine = GetDocument()->GetLineNo(m_lineNo);
  HandlePreActions(PA_SCROLL_TO_VISIBLE | PA_SET_TAG | PA_SET_CURSOR);
}

void WainView::MoveToMarkEnd(void)
{
  if(m_markType == NO_MARK)
  {
    SetStatusText("No Mark");
    return;
  }
  HandlePreActions(PA_REMOVE_CURSOR);
  int dx = m_columnNo - m_markEndX;
  int dy = m_lineNo - m_markEndY;
  m_columnNo = m_markEndX;
  m_lineNo = m_markEndY;
  m_undoList.AddEntry(UNDO_MOVE_ENTRY, -dx, -dy);
  m_currentTextLine = GetDocument()->GetLineNo(m_lineNo);
  HandlePreActions(PA_SCROLL_TO_VISIBLE | PA_SET_TAG | PA_SET_CURSOR);
}

void WainView::AddCutTextUndoEntry(TxtLine *l, int LineNo, int start, int end)
{
  UndoCutTextEntryType *c = new UndoCutTextEntryType;
  c->m_x = start;
  c->m_y = LineNo;
  c->m_text = new char [end - start + 1];
  int n;
  for(n = 0; n < end - start; n++)
    c->m_text[n] = l->GetTextAt(start + n);
  c->m_text[n] = 0;
  m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, c);
}

void WainView::AddRemoveLineUndoEntry(TxtLine *l, int LineNo)
{
  UndoRemoveLineEntryType *c = new UndoRemoveLineEntryType;
  c->m_x = 0;
  c->m_y = LineNo;
  c->m_text = new char [l->GetTabLen() + 1];
  strcpy(c->m_text, l->GetText());
  m_undoList.AddEntry(UNDO_REMOVE_LINE_ENTRY, 0, 0, 0, c);
}

void WainView::InsertTime(void)
{
   InsertFormatedTime(wainApp.gs.m_timeFormat.c_str());
}

void WainView::InsertDate(void)
{
   InsertFormatedTime(wainApp.gs.m_dateFormat.c_str());
}

void WainView::InsertTimeDate(void)
{
  CString format = (wainApp.gs.m_timeFormat + wainApp.gs.m_timeSep + wainApp.gs.m_dateFormat).c_str();
  InsertFormatedTime(format);
}

void WainView::InsertUserId(void)
{
   if(GetDocument()->GetReadOnly())
       return;
   HandlePreActions(PA_REMOVE_MARK | PA_REMOVE_CURSOR);

   std::string ui = wainApp.gs.CreateUserId();
   m_currentTextLine->InsertAt(m_columnNo, ui.c_str());

   m_undoList.AddInsertEntry(ui.size(), 0, m_columnNo, m_lineNo, ui.c_str());

   m_columnNo += ui.size();
   PutText(NULL, m_currentTextLine, m_lineNo);

   HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE | PA_SET_TAG);
}

void WainView::InsertDateTime(void)
{
  CString format = (wainApp.gs.m_dateFormat + wainApp.gs.m_timeSep + wainApp.gs.m_timeFormat).c_str();
  InsertFormatedTime(format);
}

void WainView::InsertFormatedTime(const char *format)
{
  if(GetDocument()->GetReadOnly())
    return;
  HandlePreActions(PA_REMOVE_MARK | PA_REMOVE_CURSOR);

  char time_str[1024]; // Fixme
  time_t aclock;
  time(&aclock);
  struct tm *newtime = localtime(&aclock);

  strftime(time_str, sizeof(time_str), format, newtime);
  m_currentTextLine->InsertAt(m_columnNo, time_str);

  m_undoList.AddInsertEntry(strlen(time_str), 0, m_columnNo, m_lineNo, time_str);

  m_columnNo += strlen(time_str);
  PutText(NULL, m_currentTextLine, m_lineNo);

  HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE | PA_SET_TAG);
}

void WainView::NextMatchWord(void)
{
   std::string Temp;
   if(GetDocument()->GetCurrentWord(Temp, m_columnNo, m_currentTextLine))
   {
      m_searchString = Temp.c_str();
      SearchNext();
      if(m_markType == NO_MARK)
      {
        SetStatusText("Did not find: %s", (const char *)m_searchString);
        HandlePreActions(PA_REMOVE_CURSOR);
        m_markStartX = m_columnNo;
        m_markEndX = m_columnNo + strlen(m_searchString);
        m_markStartY = m_markEndY = m_lineNo;
        m_markType = COLUMN_MARK;
        DrawMarkLines(m_markStartY, m_markEndY);
        HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SCROLL_TO_VISIBLE);
      }
   }
}

void WainView::PrevMatchWord(void)
{
   std::string Temp;
   if(GetDocument()->GetCurrentWord(Temp, m_columnNo, m_currentTextLine))
   {
      m_searchString = Temp.c_str();
      SearchPrev();
      if(m_markType == NO_MARK)
      {
        SetStatusText("Did not find: %s", (const char *)m_searchString);
        HandlePreActions(PA_REMOVE_CURSOR);
        m_markStartX = m_columnNo;
        m_markEndX = m_columnNo + strlen(m_searchString);
        m_markStartY = m_markEndY = m_lineNo;
        m_markType = COLUMN_MARK;
        DrawMarkLines(m_markStartY, m_markEndY);
        HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SCROLL_TO_VISIBLE);
      }
   }
}

void WainView::InsertComment(void)
{
  if(GetDocument()->GetReadOnly())
    return;
  HandlePreActions(PA_REMOVE_MARK | PA_REMOVE_CURSOR);

  std::string toInsert = GetDocument()->m_prop->m_lineComment;
  toInsert += " ";

  m_currentTextLine->InsertAt(m_columnNo, toInsert.c_str());

  m_undoList.AddInsertEntry(0, 0, m_columnNo, m_lineNo, toInsert.c_str());

  PutText(NULL, m_currentTextLine, m_lineNo);

  HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE | PA_SET_TAG);
}

void WainView::InsertCommentDown(void)
{
  if(GetDocument()->GetReadOnly())
    return;
  HandlePreActions(PA_REMOVE_MARK | PA_REMOVE_CURSOR);

  std::string toInsert = GetDocument()->m_prop->m_lineComment;
  toInsert += " ";

  m_currentTextLine->InsertAt(m_columnNo, toInsert.c_str());

  WainDoc *doc = GetDocument();
  int yOff = 0;
  TxtLine *l = doc->GetNext(m_currentTextLine);
  if(l)
  {
    PutText(NULL, m_currentTextLine, m_lineNo);
    m_lineNo++;
    m_currentTextLine = l;
    yOff = 1;
  }
  else
  {
     PutText(NULL, m_currentTextLine, m_lineNo);
  }
  m_undoList.AddInsertEntry(0, yOff, m_columnNo, m_lineNo - yOff, toInsert.c_str());

  HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE | PA_SET_TAG);
}

void WainView::MarkAll(void)
{
   WainDoc *doc = GetDocument();
   m_undoList.AddEntry(UNDO_START_ENTRY, 0, 0);
   SaveMarkToUndo();
   RemoveMark();
   m_markStartX = 0;
   TxtLine* line = doc->GetLineNo(doc->m_lineCount - 1);
   m_markEndX = line->GetTextLen();
   m_markStartY = 0;
   m_markEndY = doc->m_lineCount - 1;
   m_markType = CUA_MARK;
   DrawMarkLines(m_markStartY, m_markEndY);
}

void WainView::CopyFileName(void)
{
   if(!OpenClipboard())
   {
      WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
      return;
   }
   if(!::EmptyClipboard())
   {
      WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
      ::CloseClipboard();
      return;
   }
   // Find the size of the data to be copied to the clipboard
   WainDoc *doc = GetDocument();
   const char* fn = doc->GetPathName();
   int size = strlen(fn) + 1;

   HGLOBAL hData = ::GlobalAlloc(GMEM_DDESHARE, size);
   if (hData == NULL)
   {
      WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
      ::CloseClipboard();
      return;
   }
   char *out_str = (char *)::GlobalLock(hData);
   if(!out_str)
   {
      WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
      ::CloseClipboard();
      return;
  }
  // Copy text to out_str
  strcpy(out_str, fn);
  if(!::SetClipboardData(CF_TEXT, hData))
    WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
  if(!::CloseClipboard())
    WainMessageBox(this, "Failed to copy to clipboard", IDC_MSG_OK, IDI_ERROR_ICO);
   SetStatusText("File name copied to clipboard");
}

void WainView::OpenDirForFile()
{
   GetMf()->OpenDirForFile(GetDocument()->GetPathName());
}

void WainView::OpenProjectForFile()
{
   GetMf()->OpenProjectForFile(GetDocument()->GetPathName());
}

void WainView::SwitchCppH(void)
{
   const char* fullName = GetDocument()->GetPathName();
   std::string ext;
   std::string fnWithoutExt;
   MySplitPath(fullName, SP_EXT, ext);
   MySplitPath(fullName, SP_DRIVE | SP_DIR | SP_FILE, fnWithoutExt);
   const char* const cppExt[3] = {".c", ".cpp", ".cc"};
   const char* const hExt[3] = {".h", ".hpp", ".hh"};
   for (uint32_t i = 0; i < 3; i++)
   {
      if (!stricmp(ext.c_str(), cppExt[i]))
      {
         for (uint32_t j = 0; j < 3; j++)
         {
            std::string newName(fnWithoutExt + hExt[j]);
            if (IsFile(newName.c_str()))
            {
               wainApp.OpenDocument(newName.c_str());
               return;
            }
         }
      }
      else if (!stricmp(ext.c_str(), hExt[i]))
      {
         for (uint32_t j = 0; j < 3; j++)
         {
            std::string newName(fnWithoutExt + cppExt[j]);
            if (IsFile(newName.c_str()))
            {
               wainApp.OpenDocument(newName.c_str());
               return;
            }
         }
      }
   }
   SetStatusText("Dit not find a matching cpp/h file");
}

bool WainView::OpenNewToolFile(int _toolNo)
{
   if (wainApp.gs.m_toolParm[_toolNo].m_regExp.empty())
   {
      return false;
   }

   std::string fn;
   uint32_t lineNo;
   const char *ss = m_currentTextLine->GetText();

   std::regex e(wainApp.gs.m_toolParm[_toolNo].m_regExp.c_str());
   std::cmatch cm;    // same as std::match_results<const char*> cm;
   std::regex_match(ss, cm, e);

   if (cm.size() >= 3)
   {
      fn = cm[1];
      lineNo = strtol(cm.str(2).c_str(), NULL, 0);
      uint32_t columnNo = 0;
      if (cm.size() >= 4)
      {
         columnNo = strtol(cm.str(3).c_str(), NULL, 0);
      }
      CString F;
      CString my_file_name = fn.c_str();
      CString make_file = "";
      if(CheckFileName(F, my_file_name, -1, make_file))
      {
        WainDoc* doc = wainApp.OpenDocument(F);
        if(doc)
        {
          doc->m_view->GotoLineNo(lineNo - 1, columnNo - 1);
          return true;
        }
      }
      std::string debugPath = GetDocument()->GetDebugPath();
      if (!debugPath.empty())
      {
         debugPath += "\\";
         debugPath += fn;
         WainDoc* doc = wainApp.OpenDocument(debugPath.c_str());
         if(doc)
         {
           lineNo--;
           doc->m_view->GotoLineNo(lineNo - 1, columnNo - 1);
           return true;
         }
      }
   }
   return false;
}

void WainView::MarkSimilarWords()
{
   WainDoc *doc = GetDocument();
   std::string word;
   doc->GetCurrentWord(word, m_columnNo, m_currentTextLine);
   if (word != doc->m_markWord)
   {
      doc->m_markWord = word;
      UpdateVisible();
   }
}

void WainView::MarkSimilarWordsToggle()
{
   WainDoc *doc = GetDocument();
   if (doc->m_markWord.empty())
   {
      std::string word;
      doc->GetCurrentWord(word, m_columnNo, m_currentTextLine);
      if (word != doc->m_markWord)
      {
         doc->m_markWord = word;
         UpdateVisible();
      }
   }
   else
   {
      doc->m_markWord.clear();
      UpdateVisible();
   }
}

void WainView::MarkSimilarWordsOff()
{
   WainDoc *doc = GetDocument();
   if (!doc->m_markWord.empty())
   {
      doc->m_markWord.clear();
      UpdateVisible();
   }
}

void WainView::UpdateVisible()
{
   WainDoc *doc = GetDocument();
   bool corsorOn = m_cursorOn;
   RemoveCursor();

   RemoveCursor();
   RECT cr;
   GetClientRect(&cr);
   if(m_lineHeight == 0)
      return;
   int y_off = m_yOffset;
   int firstL = y_off/m_lineHeight;
   if(firstL)
      firstL--;
   int lastL = firstL + cr.bottom/m_lineHeight;
   lastL += 2;
   if(lastL > doc->m_lineCount)
     lastL = doc->m_lineCount;
   for (int l = firstL; l < lastL; l++)
   {
      TxtLine *line = doc->GetLineNo(l);
      PutText(0, line, l);
   }
   if(GetFocus() == this && corsorOn)
      SetCursor();
}

/*-------------------------------- */
