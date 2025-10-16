//=============================================================================
// This source code file is a part of Wain.
// It implements various editor functions, which belongs to wain_ViewClass.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\Wain.h"
#include ".\..\src\WainDoc.h"
#include ".\..\src\WainView.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\DocProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4100) // unreferenced formal parameter

void WainView::MarkRight(void)
{
  HandleMark(1, 0);
}

void WainView::MarkLeft(void)
{
  HandleMark(-1, 0);
}

void WainView::MarkUp(void)
{
  HandleMark(0, -1);
}

void WainView::MarkDown(void)
{
  HandleMark(0, 1);
}

void WainView::MarkWordRight(void)
{
  int n = m_currentTextLine->GoWordRight(m_columnNo);

  if(n)
  {
    HandleMark(n, 0);
  }
}

void WainView::MarkWordLeft(void)
{
  int n = m_currentTextLine->GoWordLeft(m_columnNo);
  if(n)
  {
    HandleMark(-n, 0);
  }
}

void WainView::MarkDocStart(void)
{
  HandleMark(-m_columnNo, -m_lineNo);
}

void WainView::MarkDocEnd(void)
{
  TxtLine *l = GetDocument()->GetLineNo(GetDocument()->m_lineCount - 1);
  if(l)
  {
    HandleMark(l->GetTabLen() - m_columnNo, GetDocument()->m_lineCount - m_lineNo - 1);
  }
}


void WainView::MarkWordCurrent(void)
{
#define ST_BLOCK (doc->m_prop->m_blockBegin.c_str())

  switch(m_markWordAction)
  {
    case NO_ACTION:
      RemoveMark();
      m_markStartX = m_columnNo + m_currentTextLine->GetWordLenLeft(m_columnNo);
      m_markEndX   = m_columnNo + m_currentTextLine->GetWordLenRight(m_columnNo);
      if(m_markStartX != m_markEndX)
      {
        m_markStartY = m_markEndY = m_lineNo;
        m_markType = CUA_MARK;
        DrawMarkLines(m_markStartY, m_markEndY);
        m_markWordAction = MARK_WORD_WORD;
        GetMf()->SetMarkStatus(TRUE);
      }
      else
        SetStatusText("The cursor is not over a word");
      break;
    case MARK_WORD_WORD:
      SaveMarkToUndo();
      m_markType = CUA_MARK;
      m_markStartY = m_markEndY = m_lineNo;
      m_markStartX = 0;
      m_markEndX = m_currentTextLine->GetTabLen();
      DrawMarkLines(m_markStartY, m_markEndY);
      m_markWordAction = MARK_WORD_LINE;
      break;
    case MARK_WORD_LINE:
      {
        SaveMarkToUndo();
        WainDoc *doc = GetDocument();
        int b_column;
        int f_column;
        int b_lines;
        int f_lines;
        int f_add;
        f_add = 0;
        b_lines = f_lines = 0;
        b_column = m_currentTextLine->GetTabLen();
        f_column = 0;
        TxtLine *bl;
        TxtLine *fl;
        BOOL Done;
        bl = m_currentTextLine;
        int s_flags = SEARCH_BACKWARD;
        if(doc->m_prop->m_ignoreCase)
          s_flags |= SEARCH_IGNORE_CASE;
        do
        {
          Done = TRUE;
          if((bl = doc->FindString(bl, ST_BLOCK, &b_column, &b_lines, s_flags)) != NULL)
          {
            f_lines = m_lineNo + b_lines;
            f_column = b_column;
            if((fl = doc->FindMatchBrace(bl, &f_column, &f_lines)) != NULL)
            {
              if((f_lines + f_add) >= m_lineNo)
              { // The matching brace is after or at the currrent line
                m_markStartY = m_lineNo + b_lines + f_add;
                m_markEndY = f_lines + f_add;
                m_markStartX = 0;
                m_markEndX = fl->GetTabLen();
                m_markType = CUA_MARK;
                DrawMarkLines(m_markStartY, m_markEndY);
              }
              else if((bl = GetDocument()->GetPrev(bl)) != NULL)
              { // The matching brace is before the current line, try again
                f_add += b_lines;
                f_add--;
                b_column = bl->GetTabLen();
                Done = FALSE;
              }
            }
          }
        }
        while(!Done);
        m_markWordAction = MARK_WORD_PARAGRAPH;
      }
      break;
    case MARK_WORD_PARAGRAPH:
      if(m_markStartY > 0)
      {
        SaveMarkToUndo();
        WainDoc *doc = GetDocument();
        int StartLineNo;
        TxtLine *StartLine;
        int StartColumn;
        int SumStartLine = m_markStartY - m_lineNo;

        StartColumn = m_markStartX;
        StartLineNo = m_markStartY;
        StartLine = doc->GetLineNo(StartLineNo);
        int s_flags = SEARCH_BACKWARD;
        if(doc->m_prop->m_ignoreCase)
          s_flags |= SEARCH_IGNORE_CASE;
        do
        {
          if((StartLine = doc->FindString(StartLine, ST_BLOCK, &StartColumn, &StartLineNo, s_flags)) != NULL)
          {
            SumStartLine += StartLineNo;
            int EndLineNo = m_lineNo + SumStartLine;
            int EndColumn = StartColumn;
            if(doc->FindMatchBrace(StartLine, &EndColumn, &EndLineNo))
            {
              if(EndLineNo > m_lineNo)
              {
                m_markStartX = 0;
                m_markStartY = m_lineNo + SumStartLine;
                m_markEndY = EndLineNo;
                m_markEndX = doc->GetLineNo(m_markEndY)->GetTabLen();
                m_markType = CUA_MARK;
                DrawMarkLines(m_markStartY, m_markEndY);
                return;
              }
            }
            else
               return;
          }
          else
            return;
        }
        while(1);
      }
      break;
  }
#undef ST_BLOCK
}

void WainView::MarkWord(void)
{
  HandlePreActions(PA_REMOVE_CURSOR);
  MarkWordCurrent();
  HandlePostActions(PA_SET_CURSOR);
}

void WainView::MarkPageDown(void)
{
  RECT rect = m_clientRect;

  int nof_lines = rect.bottom/m_lineHeight - 1;
  WainDoc *doc = GetDocument();
  ASSERT_VALID(doc);
  int LineCount = doc->m_lineCount;

  if(m_lineNo + nof_lines >= LineCount)
  {
    HandleMark(0, LineCount - 1 - m_lineNo);
  }
  else
  {
    HandleMark(0, nof_lines);
  }
}

void WainView::MarkPageUp(void)
{
  RECT rect = m_clientRect;
  int nof_lines = rect.bottom/m_lineHeight - 1;

  if(nof_lines > m_lineNo)
  {
    HandleMark(0, -m_lineNo);
  }
  else
  {
    HandleMark(0, -nof_lines);
  }
}

void WainView::MarkHome(void)
{
  if(m_columnNo)
  {
    HandleMark(-m_columnNo, 0);
  }
}

void WainView::MarkEnd(void)
{
  HandleMark(m_currentTextLine->GetTabLen() - m_columnNo, 0);
}

WainView::SpecialModeStatus WainView::HandleColumnMarkMode(UINT id)
{
  SpecialModeStatus ret = CONTINUE_SPECIAL_MODE;
  switch(id)
  {
    case IDV_MOVE_LEFT:
    case IDV_MOVE_LEFT_EX:
      HandleColumnMark(-1, 0);
      break;
    case IDV_MOVE_RIGHT:
    case IDV_MOVE_RIGHT_EX:
      HandleColumnMark(1, 0);
      break;
    case IDV_MOVE_UP:
      HandleColumnMark(0, -1);
      break;
    case IDV_MOVE_DOWN:
      HandleColumnMark(0, 1);
      break;
    case IDV_COLUMN_MARK:
      ret = LEAVE_SPECIAL_MODE;
      break;
    case IDV_EDIT_UNDO:
      ret = LEAVE_SPECIAL_MODE;
      break;
    case IDV_MOVE_PAGE_UP:
      {
        RECT rect;
        GetClientRect(&rect);
        int nof_lines = rect.bottom/m_lineHeight - 1;
        HandleColumnMark(0, -nof_lines);
      }
      break;
    case IDV_MOVE_PAGE_DOWN:
      {
        RECT rect;
        GetClientRect(&rect);
        int nof_lines = rect.bottom/m_lineHeight - 1;
        HandleColumnMark(0, nof_lines);
      }
      break;
    case IDV_MOVE_DOC_START:
      HandleColumnMark(0, -m_lineNo);
      break;
    case IDV_MOVE_DOC_END:
      HandleColumnMark(0, 0x40000000); /* Any random huge number, HandleColumnMark() will adjust */
      break;
    case IDV_MOVE_HOME:
      HandleColumnMark(-m_columnNo, 0);
      break;
    case IDV_MOVE_END:
      HandleColumnMark(m_currentTextLine->GetTabLen() - m_columnNo, 0);
      break;
    case IDV_COPY_CLIP:      /* These will terminate column marking mode & do there normal operation */
    case IDV_MARK_CUT:
    case IDV_COPY_SPECIAL:
    case IDV_CUT_SPECIAL:
      ColumnMarkToggle();
      ret = LEAVE_SPECIAL_MODE;
      break;
  }
  return ret;
}

void WainView::ColumnMarkToggle(void)
//  Description:
//    Message handler, toggle column marking mode
{
  if(!m_columnMarking)
  {
    HandlePreActions(PA_REMOVE_CURSOR);
    SaveMarkToUndo();
    RemoveMark();
    m_markOrgX = m_markStartX = m_markEndX = m_columnNo;
    m_markOrgY = m_markStartY = m_markEndY = m_lineNo;
    m_columnMarking = TRUE;
    m_markType = COLUMN_MARK;
    GetMf()->SetMarkStatus(TRUE);
    HandlePreActions(PA_SET_CURSOR);
  }
  else
  {
    m_columnMarking = FALSE;
    if(m_markStartX == m_markEndX)
    {
      GetMf()->SetMarkStatus(FALSE);
      m_markType = NO_MARK;
    }
  }
  m_undoList.AddEntry(UNDO_TOGGLE_COLUMN_MARK_ENTRY, 0, 0);
  SetSpecialModeStatus();
}

void WainView::IncreaseMarkTop(void)
{
  if(m_markType != NO_MARK)
  {
    if(m_markStartY)
    {
      HandlePreActions(PA_REMOVE_CURSOR);
      SaveMarkToUndo();
      m_markStartY--;
      DrawMarkLines(m_markStartY, m_markEndY);
      HandlePostActions(PA_SET_CURSOR);
    }
  }
}

void WainView::IncreaseMarkBot(void)
{
  if(m_markType != NO_MARK)
  {
    if(m_markEndY < GetDocument()->m_lineCount - 1)
    {
      HandlePreActions(PA_REMOVE_CURSOR);
      SaveMarkToUndo();
      m_markEndY++;
      if(m_markEndX && m_markType == CUA_MARK)
      {
        m_markEndX = GetDocument()->GetLineNo(m_markEndY)->GetTabLen();
      }
      DrawMarkLines(m_markStartY, m_markEndY);
      HandlePostActions(PA_SET_CURSOR);
    }
  }
}

void WainView::DecreaseMarkTop(void)
{
  if(m_markType != NO_MARK)
  {
    if(m_markEndY > m_markStartY)
    {
      HandlePreActions(PA_REMOVE_CURSOR);
      SaveMarkToUndo();
      m_markStartY++;
      DrawMarkLines(m_markStartY - 1, m_markEndY);
      HandlePostActions(PA_SET_CURSOR);
    }
  }
}

void WainView::DecreaseMarkBot(void)
{
  if(m_markType != NO_MARK)
  {
    if(m_markEndY > m_markStartY)
    {
      HandlePreActions(PA_REMOVE_CURSOR);
      SaveMarkToUndo();
      m_markEndY--;
      if(m_markEndX && m_markType == CUA_MARK)
      {
        m_markEndX = GetDocument()->GetLineNo(m_markEndY)->GetTabLen();
      }
      DrawMarkLines(m_markStartY, m_markEndY + 1);
      HandlePostActions(PA_SET_CURSOR);
    }
  }
}

void WainView::HandleColumnMark(int x, int y, BOOL from_mouse)
{
  WainDoc *doc = GetDocument();

  if(y < 0 && -y > m_lineNo)
    y = -m_lineNo;
  else if(y > 0 && m_lineNo + y >= doc->m_lineCount - 1)
    y = doc->m_lineCount - 1 - m_lineNo;
  if(x < 0 && -x > m_columnNo)
    x = -m_columnNo;
  if(!x && !y)
    return;

  if(!from_mouse)
    HandlePreActions(PA_REMOVE_CURSOR);
  SaveMarkToUndo();
  m_undoList.AddEntry(UNDO_MOVE_ENTRY, x, y);

  int new_cc = m_columnNo + x;
  int new_cl = m_lineNo + y;

  int psy = m_markStartY;
  int pey = m_markEndY;
  if(new_cc == m_markOrgX)
  {
    RemoveMark();
  }
  else
  {
    m_markType = COLUMN_MARK;
    if(new_cc < m_markOrgX)
    {
      m_markStartX = new_cc;
      m_markEndX = m_markOrgX;
    }
    else if(new_cc > m_markOrgX)
    {
      m_markStartX = m_markOrgX;
      m_markEndX = new_cc;
    }
    if(new_cl < m_markOrgY)
    {
      m_markStartY = new_cl;
      m_markEndY = m_markOrgY;
    }
    else if(new_cl > m_markOrgY)
    {
      m_markStartY = m_markOrgY;
      m_markEndY = new_cl;
    }
    else if(new_cl == m_markOrgY)
    {
      m_markStartY = m_markEndY = new_cl;
    }
  }
  if(y)
  {
    m_lineNo = new_cl;
    m_currentTextLine = doc->GetRelLineNo(y, m_currentTextLine);
    ASSERT(m_currentTextLine);
  }
  m_columnNo = new_cc;

  DrawMarkLines(MIN(psy, m_markStartY), MAX(pey, m_markEndY));
  if(!from_mouse)
    HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE | PA_SET_TAG);
}

void WainView::RemoveMark(void)
//  Description:
//    Removes the marked area, and draw the lines with mark.
//  Parameters:
//
{
  CDC *dc = GetDC();
  ASSERT(dc);
  int y;
  WainDoc *doc = GetDocument();
  TxtLine *p_line;

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

  int sy = m_yOffset/m_lineHeight;
  if(sy)
    sy--;
  int ey = (m_yOffset + m_clientRect.bottom)/m_lineHeight;
  ey++;
  p_line = doc->GetLineNo(sy);
  m_markStartX = m_markEndX = 0;
  m_markStartY = m_markEndY = 0;
  m_markType = NO_MARK;
  for(y = sy; y <= ey && p_line; y++)
  {
    PutText(dc, p_line, y);
    p_line = doc->GetNext(p_line);
  }
  ReleaseDC(dc);
}

void WainView::RemoveMarkSticky(void)
{
  if(!wainApp.gs.m_stickyMark && m_markType != NO_MARK)
  {
    BOOL co = m_cursorOn;
    RemoveCursor();
    RemoveMark();
    m_markType = NO_MARK;
    GetMf()->SetMarkStatus(FALSE);
    if(co)
      SetCursor();
  }
}
void WainView::HandleMark(int x, int y, BOOL from_mouse)
//  Description:
//    Extends the marked area in dir x, y.
//    If no mark, it will start a new mark.
//    The cursor position is Updated.
//    Will do a cua mark.
//  Parameters:
//    x, y: the direction in which the mark has to be extended.
{
  WainDoc *doc = GetDocument();
  if(m_lineNo + y < 0 || m_lineNo + y >= doc->m_lineCount)
    return;
  if(m_columnNo + x < 0)
    return;
  if(!x && !y)
    return;
  UndoMarkEntryType *Undo_entry = new UndoMarkEntryType;
  if(Undo_entry)
  {
    Undo_entry->m_sX = m_markStartX;
    Undo_entry->m_eX = m_markEndX;
    Undo_entry->m_sY = m_markStartY;
    Undo_entry->m_eY = m_markEndY;
    Undo_entry->m_type = m_markType;
    m_undoList.AddEntry(UNDO_MARK_ENTRY, x, y, m_markType, Undo_entry);
  }
  if(!from_mouse)
    HandlePreActions(PA_REMOVE_CURSOR);
  if(m_lastAction != IDV_MARK_DOWN && m_lastAction != IDV_MARK_UP &&
     m_lastAction != IDV_MARK_WORD_RIGHT && m_lastAction != IDV_MARK_WORD_LEFT &&
     m_lastAction != IDV_MARK_PAGE_UP && m_lastAction != IDV_MARK_PAGE_DOWN &&
     m_lastAction != IDV_MARK_END && m_lastAction != IDV_MARK_HOME &&
     m_lastAction != IDV_MARK_DOC_START && m_lastAction != IDV_MARK_DOC_END &&
     m_lastAction != IDV_MARK_RIGHT && m_lastAction != IDV_MARK_LEFT && m_markType == CUA_MARK && !from_mouse)
    m_markType = COLUMN_MARK;
  switch(m_markType)
  {
    case COLUMN_MARK:
      RemoveMark();
      // No break;
    case NO_MARK:
      m_markType = CUA_MARK;
      m_markStartX = MIN(m_columnNo + x, m_columnNo);
      m_markEndX   = MAX(m_columnNo + x, m_columnNo);
      m_markStartY = MIN(m_lineNo + y, m_lineNo);
      m_markEndY   = MAX(m_lineNo + y, m_lineNo);
      m_markOrgX   = m_columnNo;
      m_markOrgY   = m_lineNo;
      DrawMarkLines(m_markStartY, m_markEndY);
      break;
    case CUA_MARK:
      int sy = -1;
      int ey = -1;
      int new_cc = m_columnNo + x;
      int new_cl = m_lineNo + y;
      if(new_cc == m_markOrgX && new_cl == m_markOrgY)
      { /* Remove the mark */
        sy = m_markStartY;
        ey = m_markEndY;
        m_markEndX   = new_cc;
        m_markStartX = new_cc;
        m_markEndY   = new_cl;
        m_markStartY = new_cl;
        m_markType = NO_MARK;
      }
      else
      {
        if(x)
        {
          sy = m_lineNo;
          ey = m_lineNo + 1;
          if(new_cl < m_markOrgY)
          {
            m_markStartX = new_cc;
            m_markEndX = m_markOrgX;
          }
          else if(m_lineNo + y > m_markOrgY)
          {
            m_markEndX = new_cc;
            m_markStartX = m_markOrgX;
          }
          else if(x < 0)
          {
            if(new_cc < m_markOrgX)
            {
              m_markEndX = m_markOrgX;
              m_markStartX = new_cc;
            }
            else
            {
              m_markStartX = m_markOrgX;
              m_markEndX = new_cc;
            }
          }
          else // if(x > 0)
          {
            if(new_cc > m_markOrgX)
            {
              m_markStartX = m_markOrgX;
              m_markEndX = new_cc;
            }
            else
            {
              m_markStartX = new_cc;
              m_markEndX = m_markOrgX;
            }
          }
        }
        if(y)
        {
          if(new_cl < m_markOrgY)
          {
            sy = MIN(m_markStartY, new_cl);
            if(m_markEndY != m_markOrgY)
              ey = MAX(m_markEndY, m_markOrgY);
            else
              ey = MAX(m_markStartY, new_cl);
            m_markStartY = new_cl;
            m_markEndY = m_markOrgY;
          }
          else if(new_cl > m_markOrgY)
          {
            if(m_markStartY != m_markOrgY)
              sy = MIN(m_markStartY, m_markOrgY);
            else
              sy = MIN(m_markEndY, new_cl);
            ey = MAX(m_markEndY, new_cl);
            m_markStartY = m_markOrgY;
            m_markEndY = new_cl;
          }
          else
          {
            sy = MIN(m_markStartY, new_cl);
            ey = MAX(m_markEndY, new_cl);
            m_markStartY = m_markEndY = new_cl;
          }
          if(new_cl < m_markOrgY)
          {
            m_markStartX = new_cc;
            m_markEndX = m_markOrgX;
          }
          else if(new_cl > m_markOrgY)
          {
            m_markStartX = m_markOrgX;
            m_markEndX = new_cc;
          }
          else if(new_cc > m_markOrgX)
          {
            m_markEndX = new_cc;
            m_markStartX = m_markOrgX;
          }
          else if(new_cc < m_markOrgX)
          {
            m_markStartX = new_cc;
            m_markEndX = m_markOrgX;
          }
        }
      }
      // ASSERT(sy != -1 && ey != -1);
      // ASSERT(MarkStartX == new_cc || MarkEndX == new_cc);
      // ASSERT(MarkStartY == new_cl || MarkEndY == new_cl);
      DrawMarkLines(sy, ey);
      break;
  }

  m_columnNo += x;
  m_lineNo   += y;
  if(y)
    m_currentTextLine = doc->GetRelLineNo(y, m_currentTextLine);
  if(!from_mouse)
    HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE | PA_SET_TAG);

  if(m_markStartX == m_markEndX && m_markStartY == m_markEndY)
    m_markType = NO_MARK;
  GetMf()->SetMarkStatus(m_markType != NO_MARK);
}
