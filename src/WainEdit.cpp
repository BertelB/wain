//=============================================================================
// This source code file is a part of Wain.
// It implements some editor functions which are part of wain_ViewClass.
// And SimpleDialog.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\Wain.h"

#include ".\..\src\WainDoc.h"
#include ".\..\src\WainView.h"
#include ".\..\src\MainFrm.h"
#include ".\..\src\Taglist.h"
#include ".\..\src\DocProp.h"
#include ".\..\src\SimpleDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4100) // unreferenced formal parameter

void WainView::MoveUp(void)
//  Description:
//    Message handler
//    Moves the cursor one line up (if posible)
{
  if(m_lineNo)
  {
    HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);
    WainDoc *doc = GetDocument();
    m_lineNo--;
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, 0, -1);
    m_currentTextLine = doc->GetPrev(m_currentTextLine);
    HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE  | PA_SET_TAG);
  }
}

void WainView::MoveDown(void)
//  Description:
//    Message handler
//    Moves the cursor one line down (if posible)
{
  WainDoc *doc = GetDocument();
  TxtLine *l = doc->GetNext(m_currentTextLine);
  if(l)
  {
    HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);
    m_lineNo++;
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, 0, 1);
    m_currentTextLine = l;
    HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE | PA_SET_TAG);
  }
}

void WainView::MovePageUp(void)
//  Description:
//    Message handler
//    Moves the cursor one page up (or as much as posible)
{
  HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);
  RECT rect;
  GetClientRect(&rect);
  int nof_lines = rect.bottom/m_lineHeight - 1;
  WainDoc *doc = GetDocument();
  if(nof_lines > m_lineNo)
  {
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, 0, -m_lineNo);
    m_lineNo = 0;
    m_currentTextLine = doc->GetLineNo(m_lineNo);
  }
  else
  {
    m_lineNo -= nof_lines;
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, 0, -nof_lines);
    m_currentTextLine = doc->GetRelLineNo(-nof_lines, m_currentTextLine);
  }
  HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE  | PA_SET_TAG);
}

void WainView::MovePageDown(void)
//  Description:
//    Message handler
//    Moves the cursor one page down (or as much as posible)
{
  HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);
  RECT rect;
  GetClientRect(&rect);
  int nof_lines = rect.bottom/m_lineHeight - 1;
  WainDoc *doc = GetDocument();
  ASSERT_VALID(doc);
  int LineCount = doc->m_lineCount;

  if(m_lineNo + nof_lines >= LineCount)
  {
    if(m_lineNo < LineCount - 1)
    {
      m_undoList.AddEntry(UNDO_MOVE_ENTRY, 0, LineCount - 1 - m_lineNo);
      m_lineNo = LineCount - 1;
      m_currentTextLine = doc->GetLineNo(m_lineNo);
    }
  }
  else
  {
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, 0, nof_lines);
    m_lineNo += nof_lines;
    m_currentTextLine = doc->GetRelLineNo(nof_lines, m_currentTextLine);
  }
  HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE  | PA_SET_TAG);
}

void WainView::MoveHome(void)
{
//  Description:
//    Message handler
//    Moves the cursor to the beginning of the line
  if(m_columnNo)
  {
    HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, -m_columnNo, 0);
    m_columnNo = 0;
    HandlePreActions(PA_SET_CURSOR  | PA_SET_TAG | PA_SCROLL_TO_VISIBLE);
  }
}

void WainView::MoveHomeEx(void)
{
//  Description:
//    Message handler
//    Slightly improved move home function
  int space = m_currentTextLine->GetSpaceInFront();
  int new_pos;
  int to_move;

  if(space == 0 || space == m_columnNo)
  {
    new_pos = 0;
    to_move = -m_columnNo;
  }
  else
  {
    new_pos = space;
    to_move = space - m_columnNo;
  }
  HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);
  m_undoList.AddEntry(UNDO_MOVE_ENTRY, to_move, 0);
  m_columnNo = new_pos;
  HandlePreActions(PA_SET_CURSOR  | PA_SET_TAG | PA_SCROLL_TO_VISIBLE);
}

void WainView::MoveEnd(void)
//  Description:
//    Message handler
//    Moves the cursor to the end of the line (ie. just after the last char).
{
  HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);
  ASSERT(m_currentTextLine);
  m_undoList.AddEntry(UNDO_MOVE_ENTRY, m_currentTextLine->GetTabLen() - m_columnNo, 0);
  m_columnNo = m_currentTextLine->GetTabLen();
  HandlePreActions(PA_SET_CURSOR | PA_SET_TAG | PA_SCROLL_TO_VISIBLE);
}

void WainView::MoveLeft(void)
//  Description:
//    Message handler
//    Moves the cursor one position left (if posible).
{
  if(m_columnNo)
  {
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, -1, 0);
    m_columnNo--;
    HandlePostActions(PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SCROLL_TO_VISIBLE | PA_REMOVE_MARK_STICKY);
  }
}

void WainView::MoveRight(void)
//  Description:
//    Message handler
//    Moves the cursor one position right (if posible).
{
  m_undoList.AddEntry(UNDO_MOVE_ENTRY, 1, 0);
  m_columnNo++;
  HandlePostActions(PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SCROLL_TO_VISIBLE | PA_REMOVE_MARK_STICKY);
}

void WainView::MoveLeftEx(void)
//  Description:
//    Message handler
//    Moves the cursor one position left or to the last character position of the previous line (if any).
{
  if(m_columnNo)
  {
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, -1, 0);
    m_columnNo--;
    HandlePostActions(PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SCROLL_TO_VISIBLE | PA_REMOVE_MARK_STICKY);
  }
  else if(m_lineNo)
  {
    HandlePreActions(PA_REMOVE_CURSOR);
    m_currentTextLine = GetDocument()->GetPrev(m_currentTextLine);
    ASSERT(m_currentTextLine);
    m_columnNo = m_currentTextLine->GetTabLen();
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, m_columnNo, -1);
    m_lineNo--;
    HandlePostActions(PA_SET_CURSOR | PA_SET_TAG | PA_SCROLL_TO_VISIBLE | PA_REMOVE_MARK_STICKY);
  }
}

void WainView::MoveRightEx(void)
//  Description:
//    Message handler
//    Moves the cursor one position right of if at (or beyond) the last character position to the first position on the next line.
{
  if(m_columnNo < m_currentTextLine->GetTabLen())
  {
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, 1, 0);
    m_columnNo++;
    HandlePostActions(PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SCROLL_TO_VISIBLE | PA_REMOVE_MARK_STICKY);
  }
  else if(m_lineNo < GetDocument()->m_lineCount - 1)
  {
    HandlePreActions(PA_REMOVE_CURSOR);
    m_currentTextLine = GetDocument()->GetNext(m_currentTextLine);
    ASSERT(m_currentTextLine);
    int nof = m_currentTextLine->GetSpaceLenRight(0);
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, nof - m_columnNo, 1);
    m_lineNo++;
    m_columnNo = nof;
    HandlePostActions(PA_SET_CURSOR | PA_SET_TAG | PA_SCROLL_TO_VISIBLE | PA_REMOVE_MARK_STICKY);
  }
}

void WainView::MoveDocStart(void)
//  Description:
//    Message handler
//    Moves the cursor to the first position in the first line.
{
  if(m_lineNo || m_columnNo)
  {
    HandlePreActions(PA_REMOVE_CURSOR  | PA_REMOVE_MARK_STICKY);
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, -m_columnNo, -m_lineNo);
    m_lineNo = m_columnNo = 0;
    m_currentTextLine = GetDocument()->GetLineNo(m_lineNo);
    HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SCROLL_TO_VISIBLE);
  }
}

void WainView::MoveDocEnd(void)
//  Description:
//    Message handler
//    Moves the cursor to the last position in the last line.
{
  HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);
  WainDoc *doc = GetDocument();
  int LineCount = doc->m_lineCount -1;
  m_currentTextLine = doc->GetLineNo(LineCount);
  int pos = m_currentTextLine->GetTabLen();
  m_undoList.AddEntry(UNDO_MOVE_ENTRY, pos - m_columnNo, LineCount - m_lineNo);
  m_lineNo = LineCount;
  m_columnNo = pos;
  HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SCROLL_TO_VISIBLE);
}


void WainView::HandleReturn(void)
//  Description:
//    Message handler
//    Handles return, split the current line at current position.
{
   WainDoc *doc = GetDocument();
   if(doc->m_isDebugFile)
   {
      FileOpenInLine();
      return;
   }
   if(doc->GetReadOnly())
      return;
   RemoveLine(m_currentTextLine);
   HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK);
   ASSERT(m_currentTextLine);

   m_currentTextLine = doc->SplitLineAt(m_currentTextLine, m_columnNo);
   int n = 0;
   int i = doc->GetIndent(m_currentTextLine, n);
   m_undoList.AddEntry(UNDO_RETURN_ENTRY, -m_columnNo + i, 1);
   m_columnNo = i;
   m_lineNo++;
   if(n > 0)
   {
      char *t = (char *)malloc(n + 1);
      memset(t, ' ', n);
      t[n] = 0;
      m_undoList.AddInsertEntry(0, 0, 0, m_lineNo, t);
      m_currentTextLine->InsertAt(0, t, n);
      free(t);
   }
   else if(n < 0)
   {
      n = -n;
      UndoCutTextEntryType *p = new UndoCutTextEntryType;
      p->m_text = new char [n + 1];
      memset(p->m_text, ' ', n);
      p->m_text[n] = 0;
      p->m_x = 0;
      p->m_y = m_lineNo;
      m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, p);
      while(n--)
         m_currentTextLine->DeleteAt(0);
   }
   SetScrollInfo();
   UpdateAll();

   HandlePreActions(PA_SCROLL_TO_VISIBLE | PA_SET_CURSOR);
}

void WainView::HandleReturnEx(void)
{
  if(m_insert)
  {
    HandleReturn();
    return;
  }

  WainDoc *doc = GetDocument();
  if(doc->m_isDebugFile)
  {
    FileOpenInLine();
    return;
  }
  if(doc->GetReadOnly())
    return;

  HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK);
  ASSERT(m_currentTextLine);

  if(m_lineNo < doc->m_lineCount - 1)
  {
    HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK);
    ASSERT(m_currentTextLine);
    m_currentTextLine = doc->GetNext(m_currentTextLine);
    ASSERT(m_currentTextLine);
    int n = 0;
    int i = doc->GetIndent(m_currentTextLine, n);
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, i - m_columnNo, 1),
    m_columnNo = i;
    m_lineNo++;
    HandlePreActions(PA_SCROLL_TO_VISIBLE | PA_SET_CURSOR);
  }
  else
  { /* This was the last line, move to the end of the line & call handle_return */
    int i = m_currentTextLine->GetTabLen();
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, i - m_columnNo, 0);
    m_columnNo = i;
    HandleReturn();
  }
}

void WainView::IndentLine(void)
{
   WainDoc *doc = GetDocument();
   if(doc->GetReadOnly())
      return;
   HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);
   int n = 0;
   int i = doc->GetIndent(m_currentTextLine, n);
   m_undoList.AddEntry(UNDO_MOVE_ENTRY, -m_columnNo + i, 0);
   m_columnNo = i;
   if(n > 0)
   {
      char *t = (char *)malloc(n + 1);
      memset(t, ' ', n);
      t[n] = 0;
      m_currentTextLine->InsertAt(0, t, n);
      m_undoList.AddInsertEntry(0, 0, 0, m_lineNo, t);
      free(t);
   }
   else if(n < 0)
   {
      n = -n;
      UndoCutTextEntryType *p = new UndoCutTextEntryType;
      p->m_text = new char [n + 1];
      memset(p->m_text, ' ', n);
      p->m_text[n] = 0;
      p->m_x = 0;
      p->m_y = m_lineNo;
      m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, p);
      while(n--)
         m_currentTextLine->DeleteAt(0);
   }
   PutText(NULL, m_currentTextLine, m_lineNo);
   HandlePreActions(PA_SCROLL_TO_VISIBLE | PA_SET_CURSOR);
}

void WainView::HandleBackspace(void)
{
  WainDoc *doc = GetDocument();
  if(doc->GetReadOnly())
    return;

  if(m_columnNo)
  {
    HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK);
    int i = m_currentTextLine->GetTabLen();
    m_columnNo--;
    if(m_columnNo < i)
    {
      char ch = m_currentTextLine->GetTextAt(m_columnNo);
      switch(doc->DeleteAt(m_currentTextLine, m_columnNo))
      {
        case normal_delete:
          m_undoList.AddEntry(UNDO_DELETE_ENTRY, -1, 0, ch);
          PutText(NULL, m_currentTextLine, m_lineNo);
          CheckRedraw();
          break;
        case special_tab_delete:
        case tab_delete:
          m_undoList.AddEntry(UNDO_DELETE_ENTRY, -1, 0, ' ');
          PutText(NULL, m_currentTextLine, m_lineNo);
          CheckRedraw();
          break;
        case line_delete:
          break;
        case nOnDelete:
          break;
      }
    }
    else
      m_undoList.AddEntry(UNDO_MOVE_ENTRY, -1, 0);
    HandlePreActions(PA_SCROLL_TO_VISIBLE | PA_SET_CURSOR);
  }
}

void WainView::HandleBackspaceEx(void)
//  Description:
//    Message handler
//    Handles backspace, delete the previous char, or if at first position
//    join this and the previous line
{
  WainDoc *doc = GetDocument();
  if(doc->GetReadOnly())
    return;
  HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK);
  int i = m_currentTextLine->GetTabLen(), n;
  if(m_columnNo)
  {
    m_columnNo--;
    char ch = m_currentTextLine->GetTextAt(m_columnNo);
    switch(doc->DeleteAt(m_currentTextLine, m_columnNo))
    {
      case normal_delete:
        m_undoList.AddEntry(UNDO_DELETE_ENTRY, -1, 0, ch);
        PutText(NULL, m_currentTextLine, m_lineNo);
        CheckRedraw();
        break;
      case special_tab_delete:
      case tab_delete:
        m_undoList.AddEntry(UNDO_DELETE_ENTRY, -1, 0, ' ');
        PutText(NULL, m_currentTextLine, m_lineNo);
        CheckRedraw();
        break;
      case line_delete:
        n = m_columnNo - i + 1;
        if(n)
        {
          char *t = (char *)malloc(n + 1);
          memset(t, ' ', n);
          t[n] = 0;
          m_undoList.AddInsertEntry(0, 0, i, m_lineNo, t);
          free(t);
        }
        m_undoList.AddEntry(UNDO_MOVE_ENTRY, -1, 0);
        m_undoList.AddEntry(UNDO_DELETE_ENTRY, 0, 0, '\n');
        UpdateAll();
        break;
      case nOnDelete:
        break;
    }
  }
  else if(m_lineNo)
  {
    m_lineNo--;
    m_currentTextLine  = doc->GetPrev(m_currentTextLine);
    m_columnNo = m_currentTextLine->GetTabLen();
    m_undoList.AddEntry(UNDO_DELETE_ENTRY, m_columnNo, -1, '\n');
    switch(doc->DeleteAt(m_currentTextLine, m_columnNo))
    {
      case normal_delete:
        PutText(NULL, m_currentTextLine, m_lineNo);
        CheckRedraw();
        break;
      case tab_delete:
        PutText(NULL, m_currentTextLine, m_lineNo);
        CheckRedraw();
        break;
      case line_delete:
        UpdateAll();
        break;
      case nOnDelete:
        break;
    }
    SetScrollInfo();
  }
  HandlePreActions(PA_SCROLL_TO_VISIBLE | PA_SET_CURSOR);
}

void WainView::HandleDelete(void)
//  Description:
//    Message handler
//    Handles delete, delete current char, or join this and the next line
//    if at end of line.
{
  WainDoc *doc = GetDocument();
  if(doc->GetReadOnly())
    return;
  HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK);

  char ch = m_currentTextLine->GetTextAt(m_columnNo);
  int i = m_currentTextLine->GetTabLen(), n;
  switch(doc->DeleteAt(m_currentTextLine, m_columnNo))
  {
    case normal_delete:
      m_undoList.AddEntry(UNDO_DELETE_ENTRY, 0, 0, ch);
      PutText(NULL, m_currentTextLine, m_lineNo);
      CheckRedraw();
      break;
    case tab_delete:
    case special_tab_delete:
      m_undoList.AddEntry(UNDO_DELETE_ENTRY, 0, 0, ' ');
      PutText(NULL, m_currentTextLine, m_lineNo);
      CheckRedraw();
      break;
    case line_delete:
      n = m_columnNo - i + 1;
      if(n)
      {
        char *t = (char *)malloc(n + 1);
        memset(t, ' ', n);
        t[n] = 0;
        m_undoList.AddInsertEntry(0, 0, i, m_lineNo, t);
        free(t);
      }
      m_undoList.AddEntry(UNDO_DELETE_ENTRY, 0, 0, '\n');
      UpdateAll();
      SetScrollInfo();
      break;
    case nOnDelete:
      break;
  }
  HandlePreActions(PA_SCROLL_TO_VISIBLE | PA_SET_CURSOR);
}

void WainView::HandleDeleteEx(void)
{
  if(m_markType != NO_MARK)
    MarkDelete();
  else
    HandleDelete();
}

void WainView::OnLButtonDown(UINT flags, CPoint point)
{
   WainDoc *doc = GetDocument();
   if((point.y + m_yOffset)/m_lineHeight >= doc->m_lineCount)
      return;

   HandlePreActions(PA_REMOVE_CURSOR);
   int tx = (point.x + m_xOffset)/m_charWidth;
   int ty = (point.y + m_yOffset)/m_lineHeight;
   m_undoList.AddEntry(UNDO_START_ENTRY, 0, 0);
   if(ty != m_lineNo)
      m_markWordAction = NO_ACTION;
   m_undoList.AddEntry(UNDO_MOVE_ENTRY, tx - m_columnNo, ty - m_lineNo);
   m_columnNo = tx;
   m_lineNo = ty;

   m_mouseDown = TRUE;
   m_mouseMarkingTimer = 0;
   m_mousePoint = point;
   m_nofMouseMarkTimeout = 0;

   if(m_markType != NO_MARK)
   { // Remove old mark
      RemoveMark();
      GetMf()->SetMarkStatus(FALSE);
   }
   TxtLine *p_line;
   m_markOrgX = m_markEndX = m_markStartX = m_columnNo;
   m_markOrgY = m_markEndY = m_markStartY = m_lineNo;
   p_line = doc->GetLineNo(MIN(m_markStartY, m_markEndY));
   m_currentTextLine = doc->GetLineNo(m_lineNo);
   if(p_line)
      m_marking = TRUE;
   m_markType = NO_MARK;

   SetCapture();

   if(wainApp.gs.m_cuaMouseMarking && !(flags & MK_CONTROL) ||
      !wainApp.gs.m_cuaMouseMarking && (flags & MK_CONTROL))
   {
      m_cuaMouseMarkMode = TRUE;
   }
   else
   {
      m_cuaMouseMarkMode = FALSE;
   }
}

void WainView::OnRButtonDown(UINT flags, CPoint point)
{
  ClientToScreen(&point);
  DoViewPopupMenu(&point);
}

void WainView::OnMouseMove(UINT flags, CPoint point)
{
  if(flags & MK_LBUTTON && m_marking)
  {
    m_mousePoint = point;
    int sx = 0;
    int sy = 0;

    CRect cr;
    GetClientRect(cr);
    if(!cr.PtInRect(point))
    { // The mouse is outside the client rect
      int max_x = m_maxScroll.cx; //GetScrollLimit(SB_HORZ);
      int max_y = m_maxScroll.cy; //GetScrollLimit(SB_VERT);
      if(point.x > cr.right)
      {
        sx = m_charWidth*(1 + m_nofMouseMarkTimeout);
        if(sx + m_xOffset > max_x)
          sx = max_x - m_xOffset;
      }
      else if(point.x < cr.left)
      {
        sx = -m_charWidth*(1 + m_nofMouseMarkTimeout);
        if(sx + m_xOffset < 0)
          sx = m_xOffset;
      }
      if(point.y > cr.bottom)
      {
        sy = m_lineHeight*(1 + m_nofMouseMarkTimeout);
        if(sy + m_yOffset > max_y)
          sy = max_y - m_yOffset;
      }
      else if(point.y < cr.top)
      {
        sy = -m_lineHeight*(1 + m_nofMouseMarkTimeout);
        if(-sy > m_yOffset)
          sy = -m_yOffset;
      }
      SetScrollPos(SB_HORZ, m_xOffset + sx, TRUE);
      SetScrollPos(SB_VERT, m_yOffset + sy, TRUE);
      ScrollWindow(-sx, -sy);
      m_xOffset += sx;
      m_yOffset += sy;
      UpdateTabViewPos();
      if(flags != 0xFFFF)
      {
        if(m_mouseMarkingTimer)
          KillTimer(m_mouseMarkingTimer);
        m_mouseMarkingTimer = SetTimer(IDM_MOUSE_MARK_TIMER, 250, NULL);
        m_nofMouseMarkTimeout = 0;
      }
    }
    else if(m_mouseMarkingTimer)
    {
      KillTimer(m_mouseMarkingTimer);
      m_mouseMarkingTimer = 0;
      m_nofMouseMarkTimeout = 0;
    }

    int ty, tx;
    tx = (point.x + m_xOffset)/m_charWidth;
    ty = (point.y + m_yOffset)/m_lineHeight;
    if(m_cuaMouseMarkMode)
    {
      m_markType = CUA_MARK;
      HandleMark(tx - m_columnNo, ty - m_lineNo, TRUE);
    }
    else
    {
      m_markType = COLUMN_MARK;
      HandleColumnMark(tx - m_columnNo, ty - m_lineNo, TRUE);
    }
    if(sx || sy)
      UpdateWindow();
    GetMf()->SetLineStatus(this, m_lineNo, m_columnNo);
  }
}

void WainView::OnLButtonUp(UINT nFlags, CPoint point)
{
  if(m_mouseDown)
  {
    WainDoc *doc = GetDocument();

    m_lineNo = (point.y + m_yOffset)/m_lineHeight;
    if(m_lineNo >= doc->m_lineCount)
      m_lineNo = doc->m_lineCount - 1;
    m_columnNo = (point.x + m_xOffset)/m_charWidth;
    if(m_columnNo < 0)
      m_columnNo = 0;

    m_currentTextLine = doc->GetLineNo(m_lineNo);
    m_mouseDown = FALSE;
    if(GetFocus() == this)
      HandlePostActions(PA_SET_CURSOR | PA_SET_TAG);

    GetMf()->SetLineStatus(this, m_lineNo, m_columnNo);
    m_marking = FALSE;
    if(m_mouseMarkingTimer)
    {
      KillTimer(m_mouseMarkingTimer);
      m_mouseMarkingTimer = 0;
      m_nofMouseMarkTimeout = 0;
    }
    ReleaseCapture();
    if(m_markStartX == m_markEndX && m_markStartY == m_markEndY)
      m_markType = NO_MARK;
    if(m_markType != NO_MARK)
      GetMf()->SetMarkStatus(TRUE);
  }
}

BOOL WainView::OnMouseWheel(UINT flags, short delta, CPoint pt)
{
  if(!m_currentTextLine)
    return TRUE;
  WainDoc *doc = GetDocument();
  delta = (short )-delta;
  int lc = delta/WHEEL_DELTA;
  lc *= m_mouseWheelPrNotch;

  HandlePreActions(PA_REMOVE_CURSOR);
  if(lc > 0)
  {
    if(m_lineNo + lc < doc->m_lineCount - 1)
      m_lineNo += lc;
    else
    {
      lc = doc->m_lineCount - 1  -  m_lineNo ;
      m_lineNo = doc->m_lineCount - 1;
    }
    m_currentTextLine = doc->GetRelLineNo(lc, m_currentTextLine);
  }
  else if(lc < 0 )
  {
    if(m_lineNo >= -lc)
      m_lineNo += lc;
    else
    {
      lc = -m_lineNo;
      m_lineNo = 0;
    }
    m_currentTextLine = doc->GetRelLineNo(lc, m_currentTextLine);
  }
  HandlePostActions(PA_SCROLL_TO_VISIBLE | PA_SET_CURSOR | PA_SET_TAG | PA_REMOVE_MARK_STICKY);
  if(GetFocus() == this)
    GetMf()->SetLineStatus(this, m_lineNo, m_columnNo);

  return TRUE;
}

void WainView::OnTimer(UINT timer_id)
{
  if(timer_id == m_mouseMarkingTimer)
  {
    OnMouseMove(0xFFFF, m_mousePoint);
    m_nofMouseMarkTimeout++;
  }
  else
    SetStatusText("Unknown timer");
}


void WainView::OnLButtonDblClk(UINT flags, CPoint point)
{
  WainDoc *doc = GetDocument();
  if(this == wainApp.m_lastAutoTagView)
  { /* Open me as a normal document, OnIdle() will do the work */
    wainApp.m_openAutoTagFileAsNormalDoc = TRUE;
  }
  if(doc->m_isDebugFile)
  {
    m_lineNo = (point.y + m_yOffset)/m_lineHeight;
    if(m_lineNo >= doc->m_lineCount)
      m_lineNo = doc->m_lineCount - 1;
    m_columnNo = point.x/m_charWidth;
    m_currentTextLine = doc->GetLineNo(m_lineNo);
    FileOpenInLine();
  }
  else
  {
    m_markWordAction = NO_ACTION;
    MarkWord();
  }
}

void WainView::OpenAsNormalDoc(void)
{
  if(this == wainApp.m_lastAutoTagView)
  { /* Open me as a normal document, OnIdle() will do the work */
    wainApp.m_openAutoTagFileAsNormalDoc = TRUE;
  }
  else
  {
    wainApp.m_openAsNormalDoc = GetDocument()->GetPathName();
  }
}

void WainView::TagPeek(void)
{
  std::string word;
  GetDocument()->GetCurrentWord(word, m_columnNo, m_currentTextLine);

  if(!word.empty())
  {
    GetTagListClass *TagList = new GetTagListClass(word.c_str(), TL_ANY); // fixme

    if(!GetMf()->m_navigatorDialog.GetTagList(TagList))
    {
      SetStatusText("No tags, use the navigator-dialog->tags to setup tags");
      delete TagList;
      return;
    }
    if(TagList->m_list.size() == 0)
    {
      SetStatusText("No matching tags");
      delete TagList;
      return;
    }
    wainApp.SetTagPeek(new GetTagElemClass(TagList->m_list[0]));
    delete TagList;
  }
  else
    SetStatusText("The cursor is not over a word");
}

void WainView::MoveWordRight(void)
{
  int i = m_currentTextLine->GoWordRight(m_columnNo);
  if(i)
  {
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, i, 0);
    m_columnNo += i;
    HandlePostActions(PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SCROLL_TO_VISIBLE  | PA_REMOVE_MARK_STICKY);
  }
}

void WainView::MoveWordLeft(void)
{
  int i = m_currentTextLine->GoWordLeft(m_columnNo);
  if(i)
  {
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, -i, 0);
    m_columnNo -= i;
    HandlePostActions(PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SCROLL_TO_VISIBLE | PA_REMOVE_MARK_STICKY);
  }
}

void WainView::MoveWordRightEx(void)
{
   int i = m_currentTextLine->GoWordRight(m_columnNo);
   if(i)
   {
      m_undoList.AddEntry(UNDO_MOVE_ENTRY, i, 0);
      m_columnNo += i;
      HandlePostActions(PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SCROLL_TO_VISIBLE  | PA_REMOVE_MARK_STICKY);
   }
   else
   {
      TxtLine *next = GetDocument()->GetNext(m_currentTextLine);
      if(next)
      {
         HandlePostActions(PA_REMOVE_CURSOR);
         m_lineNo++;
         if(next->GetTextAt(0) == ' ' || next->GetTextAt(0) == '\t')
            i = next->GoWordRight(0);
         else
            i = 0;
         m_undoList.AddEntry(UNDO_MOVE_ENTRY, i - m_columnNo, 1);
         m_columnNo = i;
         m_currentTextLine = next;
         HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SCROLL_TO_VISIBLE  | PA_REMOVE_MARK_STICKY);
      }
   }
}

void WainView::MoveWordLeftEx(void)
{
   int i = m_currentTextLine->GoWordLeft(m_columnNo);
   if(i)
   {
      m_undoList.AddEntry(UNDO_MOVE_ENTRY, -i, 0);
      m_columnNo -= i;
      HandlePostActions(PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SCROLL_TO_VISIBLE | PA_REMOVE_MARK_STICKY);
   }
   else
   {
      TxtLine *prev = GetDocument()->GetPrev(m_currentTextLine);
      if(prev)
      {
         HandlePostActions(PA_REMOVE_CURSOR);
         m_lineNo--;
         i = prev->GetTabLen();
         m_undoList.AddEntry(UNDO_MOVE_ENTRY, i - m_columnNo, -1);
         m_columnNo = i;
         m_currentTextLine = prev;
         HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SCROLL_TO_VISIBLE  | PA_REMOVE_MARK_STICKY);
      }
   }
}

void WainView::DeleteWord(void)
{
  if(!GetDocument()->GetReadOnly())
  {
    if(m_columnNo >= m_currentTextLine->GetTabLen())
    {
      HandleDelete();
    }
    else
    {
      int i;
      if((i = m_currentTextLine->GetWordLenRight(m_columnNo)) != 0 ||
         (i = m_currentTextLine->GetSpaceLenRight(m_columnNo)) != 0 ||
         (i = m_currentTextLine->GetSepLenRight(m_columnNo)) != 0)
      {
        HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK);
        UndoCutTextEntryType *c = new UndoCutTextEntryType;
        c->m_text = new char [i + 1];
        m_currentTextLine->CopyTextAt(c->m_text, m_columnNo, i);
        c->m_text[i] = '\0';
        c->m_x = m_columnNo;
        c->m_y = m_lineNo;
        m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, c);
        m_currentTextLine->DeleteAt(m_columnNo, m_columnNo + i);
        PutText(NULL, m_currentTextLine, m_lineNo);
        HandlePostActions(PA_SET_CURSOR);
      }
    }
  }
}

void WainView::GotoLineNo(void)
{
  SimpleDialog sd("Goto Line No", "", this);
  if(sd.DoModal() == IDOK)
  {
    WainDoc *doc = GetDocument();
    char *end;
    int line = (int )strtol(sd.m_msg, &end, 0);
    line--;
    if(!end || *end != '\0' || line <= 0 || line >= doc->m_lineCount)
      SetStatusText("Invalid line number");
    else
      GotoLineNo(line);
  }
}

void WainView::ShiftBlockRight(void)
{
  WainDoc *doc = GetDocument();
  if(doc->GetReadOnly())
    return;
  HandlePreActions(PA_REMOVE_CURSOR);

  m_undoList.AddEntry(UNDO_SHIFT_BLOCK_ENTRY, 0, 0, 1);
  if(m_markType == CUA_MARK)
  {
    TxtLine *l = doc->GetLineNo(m_markStartY);
    int i;
    for(i = m_markStartY; i < m_markEndY && l; i++)
    {
      l->InsertAt(0, ' ');
      PutText(NULL, l, i);
      l = doc->GetNext(l);
    }
    if(l && m_markEndX >= l->GetTabLen())
    {
      l->InsertAt(0, ' ');
      m_markEndX++;
      PutText(NULL, l, i);
    }
  }
  else if(m_markType == NO_MARK)
  {
    m_currentTextLine->InsertAt(0, ' ');
    PutText(NULL, m_currentTextLine, m_lineNo);
  }
  else if(m_markType == COLUMN_MARK)
  {
    TxtLine *l = doc->GetLineNo(m_markStartY);
    BOOL is_space = TRUE;
    int i;
    for(i = m_markStartY; i <= m_markEndY && l; i++)
    {
      if(l->GetTabLen() > m_markEndX && l->GetTextAt(m_markEndX) != ' ' && l->GetTextAt(m_markEndX) != '\t')
        is_space = FALSE;
    }
    if(is_space)
    {
      m_markStartX++;
      m_markEndX++;

      for(i = m_markStartY; i <= m_markEndY && l; i++)
      {
        l->InsertAt(m_markStartX - 1, ' ');
        l->DeleteAt(m_markEndX);
        PutText(NULL, l, i);
        l = doc->GetNext(l);
      }
    }
  }
  HandlePostActions(PA_SET_CURSOR);
}

void WainView::ShiftBlockLeft(void)
{
  WainDoc *doc = GetDocument();
  if(doc->GetReadOnly())
    return;
  m_undoList.AddEntry(UNDO_SHIFT_BLOCK_ENTRY, 0, 0, -1);
  HandlePreActions(PA_REMOVE_CURSOR);
  if(m_markType == CUA_MARK)
  {
    BOOL is_space = TRUE;
    const char *t;
    int i;
    int end = m_markEndY;
    TxtLine *l = doc->GetLineNo(m_markStartY);
    TxtLine *el = doc->GetLineNo(m_markEndY);
    if(el && m_markEndX >= el->GetTabLen())
      end++;

    for(i = m_markStartY; i < end && l; i++)
    {
      t = l->GetText();
      if(t[0] != ' ' && t[0] != '\t' && t[0] != 0)
        is_space = FALSE;
      l = doc->GetNext(l);
    }
    if(is_space)
    {
      l = doc->GetLineNo(m_markStartY);
      for(i = m_markStartY; i < end && l; i++)
      {
        t = l->GetText();
        if(t[0] == '\t')
        {
          l->DeleteAt(0);
          l->InsertAt(0, ' ');
        }
        else
        {
          l->DeleteAt(0);
        }
        PutText(NULL, l, i);
        l = doc->GetNext(l);
      }
    }
  }
  else if(m_markType == NO_MARK)
  {
    const char *t = m_currentTextLine->GetText();
    if(t[0] == ' ')
    {
      m_currentTextLine->DeleteAt(0);
    }
    else if(t[0] == '\t')
    {
      m_currentTextLine->DeleteAt(0);
      m_currentTextLine->InsertAt(0, ' ');
    }
    PutText(NULL, m_currentTextLine, m_lineNo);
  }
  else if(m_markType == COLUMN_MARK && m_markStartX > 0)
  {
    BOOL is_space = TRUE;
    const char *t;
    int i;
    int end = m_markEndY;
    TxtLine *l = doc->GetLineNo(m_markStartY);

    end++;

    for(i = m_markStartY; i < end && l; i++)
    {
      t = l->GetText();
      if(t[m_markStartX - 1] != ' ' && t[m_markStartX - 1] != '\t' && t[m_markStartX - 1] != 0)
        is_space = FALSE;
      l = doc->GetNext(l);
    }
    if(is_space)
    {
      m_markStartX--;
      m_markEndX--;
      l = doc->GetLineNo(m_markStartY);
      for(i = m_markStartY; i < end && l; i++)
      {
        t = l->GetText();
        if(t[m_markStartX] == '\t')
        {
          l->DeleteAt(m_markStartX);
          l->InsertAt(m_markStartX, ' ');
        }
        else
        {
          l->DeleteAt(m_markStartX);
          if(l->GetTabLen() > m_markEndX)
            l->InsertAt(m_markEndX, ' ');
        }
        PutText(NULL, l, i);
        l = doc->GetNext(l);
      }
    }
  }
  HandlePostActions(PA_SET_CURSOR);
}

void WainView::FindMatchBrace(void)
{
  HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);

  WainDoc *doc = GetDocument();
  TxtLine *l;
  int x = m_columnNo;
  int y = m_lineNo;
  l = doc->FindMatchBrace(m_currentTextLine, &x, &y);
  if(l)
  {
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, x - m_columnNo, y - m_lineNo);
    m_columnNo = x;
    m_lineNo = y;
    m_currentTextLine = l;
  }
  else
    SetStatusText("Matching brace was not found");

  HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE);
}

void WainView::FindMatchBlock(void)
{
  HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);

  WainDoc *doc = GetDocument();
  TxtLine *l;
  int x = m_columnNo;
  int y = m_lineNo;
  l = doc->FindMatchBlock(m_currentTextLine, &x, &y);
  if(l)
  {
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, x - m_columnNo, y - m_lineNo);
    m_columnNo = x;
    m_lineNo = y;
    m_currentTextLine = l;
  }
  else
  {
      SetStatusText("Matching block was not found");
   }

  HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE);
}


void WainView::GotoBlockStart(void)
{
  HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);

  WainDoc *doc = GetDocument();
  TxtLine *l;
  int x = m_columnNo;
  int y = m_lineNo;
  l = doc->GotoBlockStart(m_currentTextLine, &x, &y);
  if(l)
  {
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, x - m_columnNo, y - m_lineNo);
    m_columnNo = x;
    m_lineNo = y;
    m_currentTextLine = l;
  }
  else
    SetStatusText("Block start was not found");

  HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE);
}

void WainView::GotoBlockEnd(void)
{
  HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);

  WainDoc *doc = GetDocument();
  TxtLine *l;
  int x = m_columnNo;
  int y = m_lineNo;
  l = doc->GotoBlockEnd(m_currentTextLine, &x, &y);
  if(l)
  {
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, x - m_columnNo, y - m_lineNo);
    m_columnNo = x;
    m_lineNo = y;
    m_currentTextLine = l;
  }
  else
    SetStatusText("Block end was not found");

  HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE);
}

WainView::SpecialModeStatus WainView::HandleSpecialModes(UINT id, int ch)
{
   SpecialModeStatus ret = LEAVE_SPECIAL_MODE;
   if(m_columnMarking)
   {
      ret = HandleColumnMarkMode(id);
      SetSpecialModeStatus();
   }
   else if(m_incSearchOn)
   {
      ret = HandleIncSearch(id, ch);
      SetSpecialModeStatus();
   }

   return ret;
}


void WainView::IncSearch(void)
{
  m_incSearchOn = TRUE;
  m_searchString = "";
  HandlePreActions(PA_REMOVE_CURSOR);
  RemoveMark();
  m_searchFlags &= ~SEARCH_USE_REGEX;
  m_searchFlags |= SEARCH_IGNORE_CASE;
  m_orgIncSearchX = m_columnNo;
  m_orgIncSearchY = m_lineNo;
  m_markStartX = m_markEndX = m_columnNo;
  m_markStartY = m_markEndY = m_lineNo;
  m_markType = COLUMN_MARK;
  HandlePostActions(PA_SET_CURSOR);
  SetSpecialModeStatus();
}

WainView::SpecialModeStatus WainView::HandleIncSearch(UINT id, int ch)
{
  if(!m_incSearchOn)
     return LEAVE_SPECIAL_MODE;

  WainDoc *doc = GetDocument();
  SpecialModeStatus run = CONTINUE_SPECIAL_MODE;
  switch(id)
  {
    case IDV_MOVE_RIGHT:
    case IDV_MOVE_RIGHT_EX:
      if(m_markEndX < m_currentTextLine->GetTabLen())
      {
        HandlePreActions(PA_REMOVE_CURSOR);
        m_searchString += m_currentTextLine->GetTextAt(m_markEndX);
        m_markEndX++;
        DrawMarkLines(m_markStartY, m_markEndY);
        HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SCROLL_TO_VISIBLE);
      }
      break;
    case IDV_MOVE_LEFT:
    case IDV_MOVE_LEFT_EX:
      if(strlen(m_searchString))
      {
        HandlePreActions(PA_REMOVE_CURSOR);
        m_searchString.Delete(m_searchString.GetLength() - 1);
        m_markEndX--;
        DrawMarkLines(m_markStartY, m_markEndY);
        HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SCROLL_TO_VISIBLE);
      }
      break;
    case IDV_MOVE_UP:
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
      break;
    case IDV_MOVE_DOWN:
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
      break;
    case IDV_HANDLE_RETURN:
      m_incSearchOn = FALSE;
      HandlePreActions(PA_REMOVE_CURSOR);
      RemoveMark();
      HandlePostActions(PA_SET_CURSOR);
      break;
    case IDV_HANDLE_ESC:
      HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK);
      m_incSearchOn = FALSE;
      run = LEAVE_SPECIAL_MODE;
      m_columnNo = m_orgIncSearchX;
      m_lineNo = m_orgIncSearchY;
      m_currentTextLine = doc->GetLineNo(m_lineNo);
      RemoveMark();
      HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SCROLL_TO_VISIBLE);
      GetMf()->SetSpecialModeStatus("");
      break;
    case IDV_MOVE_END:
      int nof;
      nof = m_currentTextLine->GetWordLenRight(m_markEndX);
      if(nof)
      {
        HandlePreActions(PA_REMOVE_CURSOR);
        CString temp;
        for(int i = 0; i < nof; i++)
          temp += m_currentTextLine->GetTextAt(m_markEndX + i);
        m_searchString += temp;
        m_markEndX += nof;
        DrawMarkLines(m_markStartY, m_markEndY);
        HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SCROLL_TO_VISIBLE);
      }
      break;
    case ID_PUT_CHAR:
      if(ch < 256 && ch >= 32)
      {
        if((m_currentTextLine->GetTextAt(m_markEndX) == (char )ch) ||
           ((m_searchFlags & SEARCH_IGNORE_CASE) && (::tolower(m_currentTextLine->GetTextAt(m_markEndX)) == ::tolower(*(unsigned char *)&ch))))
          HandleIncSearch(IDV_MOVE_RIGHT, 0);
        else
        {
          m_searchString += (char )ch;
          SearchNext();
          if(m_markType == NO_MARK)
            SearchPrev();
          if(m_markType == NO_MARK)
          {
            SetStatusText("Did not find: %s", (const char *)m_searchString);
            HandlePreActions(PA_REMOVE_CURSOR);
            m_searchString.Delete(m_searchString.GetLength() - 1);
            m_markStartX = m_columnNo;
            m_markEndX = m_columnNo + strlen(m_searchString);
            m_markStartY = m_markEndY = m_lineNo;
            m_markType = COLUMN_MARK;
            DrawMarkLines(m_markStartY, m_markEndY);
            HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SCROLL_TO_VISIBLE);
          }
        }
      }
      break;
  }
  return run;
}

void WainView::HandleEsc(void)
{
   HandleSpecialModes(IDV_HANDLE_ESC);
}

int WainView::ExpandSpecial(std::string &aStr)
{
  int ret = -1;
  std::string Temp(aStr);
  std::string::size_type n;

  if((n = FindStrX(Temp, 'c')) != std::string::npos)
  { // "\c" is cursor position
     ret = (int )n;
     Temp.erase(n, 2);
  }

  while((n = FindStrX(Temp, 't')) != std::string::npos)
  {
    int ts = GetDocument()->m_prop->m_tabSize;
    Temp[n] = '\t';
    Temp.erase(n + 1, 1);
    if(ret > (int )n)
       ret += ts - 2;
  }
  while((n = FindStrX(Temp, 'i')) != std::string::npos)
  {
     int is = GetDocument()->m_prop->m_indent;
     if(ret > (int )n)
        ret += is - 2;

     Temp.erase(n, 2);
     Temp.insert(n, " ", is);
  }


  while((n = FindStrX(Temp, 'T')) != std::string::npos)
  {
     std::string T = Temp.substr(n + 2);
     Temp = Temp.substr(0, n);
     char time_str[1024];
     time_t aclock;
     time(&aclock);
     struct tm *newtime = localtime(&aclock);

     strftime(time_str, sizeof(time_str), wainApp.gs.m_timeFormat.c_str(), newtime);
     Temp += time_str;
     Temp += T;
     if(ret > (int )n)
        ret += strlen(time_str) - 2;
  }

  while((n = FindStrX(Temp, 'U')) != std::string::npos)
  {
    std::string ui = wainApp.gs.CreateUserId();
    std::string T = Temp.substr(n + 2);
    Temp = Temp.substr(0, n);
    Temp += ui;
    Temp += T;
    if(ret > (int )n)
      ret += ui.size() - 2;
  }

  while((n = FindStrX(Temp, 'D')) != std::string::npos)
  {
    std::string T = Temp.substr(n + 2);
    Temp = Temp.substr(0, n);

    char time_str[1024];
    time_t aclock;
    time(&aclock);
    struct tm *newtime = localtime(&aclock);

    strftime(time_str, sizeof(time_str), wainApp.gs.m_dateFormat.c_str(), newtime);

    Temp += time_str;
    Temp += T;
    if(ret > (int )n)
      ret += strlen(time_str) - 2;
  }

  while((n = FindStrX(Temp, 'S')) != std::string::npos)
  {
    std::string T = Temp.substr(n + 2);
    Temp = Temp.substr(0, n);

    char time_str[1024];
    time_t aclock;
    time(&aclock);
    struct tm *newtime = localtime(&aclock);

    strftime(time_str, sizeof(time_str), wainApp.gs.m_timeSep.c_str(), newtime);
    Temp += time_str;
    Temp += T;
    if(ret > (int )n)
      ret += strlen(time_str) - 2;
  }

  /* Remove any \\ */
  while((n = Temp.find("\\\\")) != std::string::npos)
  {
     Temp.erase(n, 1);
     if(ret > (int )n)
       ret--;
  }

  aStr = Temp;

  return ret;
}

void WainView::ExpandTemplate(void)
{
   WainDoc *doc = GetDocument();
   DocPropClass* Prop = doc->m_prop;
   if(doc->GetReadOnly())
   {
     SetStatusText("Can't modify readOnly file");
     return;
   }

   std::string word;
   if(!m_currentTextLine->GetWordLenLeft(m_columnNo) || !doc->GetCurrentWord(word, m_columnNo, m_currentTextLine))
   {
     int len = -m_currentTextLine->GetSepLenLeft(m_columnNo);
     if(!len)
     {
       SetStatusText("The cursor is not at a word");
       return;
     }
     char *temp = new char [len + 1];
     m_currentTextLine->CopyTextAt(temp, m_columnNo - len, len);
     temp[len] = 0;
     word = temp;
     delete [] temp;
   }
   int i;
   int Size = Prop->m_templateList.size();
   std::string TS;
   for(i = 0; i < Size; i++)
   {
      TS = Prop->m_templateList[i]->m_name;
      if(TS == word ||
         (word.size() > TS.size() &&
          word.substr(word.size() - TS.size()) == TS))
      { // It's a template
         if(word.size() > TS.size())
            word =  word.substr(word.size() - TS.size());

         HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK);
         UndoCutTextEntryType *cp;
         cp = new UndoCutTextEntryType;
         cp->m_text = new char [word.size() + 1];
         strcpy(cp->m_text, word.c_str());
         cp->m_x = m_columnNo - word.size();
         cp->m_y = m_lineNo;
         m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, cp);

         m_currentTextLine->DeleteAt(m_columnNo - word.size(), m_columnNo);
         int c = word.size();
         c = -c;

         /* int c = doc->RemoveWordCurrent(m_columnNo, m_currentTextLine); */
         m_undoList.AddEntry(UNDO_MOVE_ENTRY, c, 0);
         m_columnNo += c;

         char *rest_text = (char *)malloc(m_maxLineLen);
         m_currentTextLine->RemoveFrom(m_columnNo, rest_text);
         cp = new UndoCutTextEntryType;
         cp->m_text = new char [strlen(rest_text) + 1];
         strcpy(cp->m_text, rest_text);
         cp->m_x = m_columnNo;
         cp->m_y = m_lineNo;
         m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, cp);

         std::string expansion(Prop->m_templateList[i]->m_expansion);

         int line_offset = -1;
         int column_offset = 0;
         int ll = 0;
         int s_l = -1;
         int s_len = 0;
         TxtLine *l = m_currentTextLine;

         while(expansion.size())
         {
           s_l++;
           std::string::size_type n = FindStrX(expansion, 'n');

           std::string temp = expansion.substr(0, n);
           if(n != std::string::npos)
             expansion = expansion.substr(n + 2);
           else
             expansion = "";

           int co = ExpandSpecial(temp);
           s_len = temp.size();
           if(co >= 0)
           { /* We got a \c in this line */
             line_offset = ll;
             column_offset = co;
             if(ll)
               column_offset += m_columnNo;
           }
           m_undoList.AddInsertEntry(0, 0, ll ? 0 : m_columnNo, m_lineNo + ll, temp.c_str());
           l->InsertAt(m_columnNo, temp.c_str());

           if(n != std::string::npos)
           {
             l = doc->InsertLine(l, "");
             ll++;
             m_undoList.AddEntry(UNDO_INSERT_LINE_ENTRY, 0, 0, m_lineNo + ll);
           }
         }

         m_undoList.AddInsertEntry(0, 0, l->GetTabLen(), m_lineNo + ll, rest_text);
         l->InsertAt(l->GetTabLen(), rest_text);

         if(line_offset != -1)
         {
           m_currentTextLine = doc->GetRelLineNo(line_offset, m_currentTextLine);
           if(line_offset == 0)
           {
             m_undoList.AddEntry(UNDO_MOVE_ENTRY, column_offset, line_offset);
             m_columnNo += column_offset;
           }
           else
           {
             m_undoList.AddEntry(UNDO_MOVE_ENTRY, column_offset - m_columnNo, line_offset);
             m_columnNo = column_offset;
           }
           m_lineNo += line_offset;
         }
         else
         {
           m_currentTextLine = doc->GetRelLineNo(s_l, m_currentTextLine);
           m_undoList.AddEntry(UNDO_MOVE_ENTRY, s_len, s_l);
           m_lineNo += s_l;
           m_columnNo += s_len;
         }
         UpdateAll();
         HandlePreActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SCROLL_TO_VISIBLE);
         free(rest_text);

         return; // We are Done
      }
   }
   SetStatusText("No matching templates found");
}

void WainView::ToUpper(void)
{
  if(m_markType == NO_MARK)
  {
    SetStatusText("No mark");
    return;
  }
  WainDoc *doc = GetDocument();
  if(doc->GetReadOnly())
    return;

  HandlePreActions(PA_REMOVE_CURSOR);
  UndoCutTextEntryType *pc;

  if(m_markType == CUA_MARK)
  {
    TxtLine *l = doc->GetLineNo(m_markStartY);
    if(m_markEndY == m_markStartY)
    {
      pc = new UndoCutTextEntryType;
      pc->m_text = new char [m_markEndX - m_markStartX + 1];
      l->CopyTextAt(pc->m_text, m_markStartX, m_markEndX - m_markStartX);
      pc->m_text[m_markEndX - m_markStartX] = 0;
      pc->m_x = m_markStartX;
      pc->m_y = m_markStartY;
      m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, pc);

      l->ToUpper(m_markStartX, m_markEndX);

      char *t = (char *)malloc(2 + m_markEndX - m_markStartX);
      l->CopyTextAt(t, m_markStartX, m_markEndX - m_markStartX);
      t[m_markEndX - m_markStartX] = 0;
      m_undoList.AddInsertEntry(0, 0, m_markStartX, m_markStartY, t);
      free(t);
    }
    else
    {
      if(m_markStartX < l->GetTabLen())
      {
        pc = new UndoCutTextEntryType;
        pc->m_text = new char [l->GetTabLen() - m_markStartX + 1];
        l->CopyTextAt(pc->m_text, m_markStartX, l->GetTabLen() - m_markStartX);
        pc->m_text[l->GetTabLen() - m_markStartX] = 0;
        pc->m_x = m_markStartX;
        pc->m_y = m_markStartY;
        m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, pc);

        l->ToUpper(m_markStartX);

        char *t = (char *)malloc(l->GetTabLen() - m_markStartX + 2);
        l->CopyTextAt(t, m_markStartX, l->GetTabLen() - m_markStartX);
        t[l->GetTabLen() - m_markStartX] = 0;
        m_undoList.AddInsertEntry(0, 0, m_markStartX, m_markStartY, t);
        free(t);
      }
      l = doc->GetNext(l);

      int i;
      for(i = m_markStartY + 1; i < m_markEndY && l; i++, l = doc->GetNext(l))
      {
        pc = new UndoCutTextEntryType;
        pc->m_text = new char [l->GetTabLen()+ 1];
        l->CopyTextAt(pc->m_text, 0, l->GetTabLen());
        pc->m_text[l->GetTabLen()] = 0;
        pc->m_x = 0;
        pc->m_y = i;
        m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, pc);

        l->ToUpper();

        char *t = (char *)malloc(l->GetTabLen() + 2);
        l->CopyTextAt(t, 0, l->GetTabLen());
        t[l->GetTabLen()] = 0;
        m_undoList.AddInsertEntry(0, 0, 0, i, t);
        free(t);
      }
      if(l)
      {
        int len = MIN(m_markEndX, l->GetTabLen());
        pc = new UndoCutTextEntryType;
        pc->m_text = new char [len + 1];
        l->CopyTextAt(pc->m_text, 0, len);
        pc->m_text[len] = 0;
        pc->m_x = 0;
        pc->m_y = m_markEndY;
        m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, pc);

        l->ToUpper(0, m_markEndX);

        char *t = (char *)malloc(len + 2);
        l->CopyTextAt(t, 0, len);
        t[len] = 0;
        m_undoList.AddInsertEntry(0, 0, 0, m_markEndY, t);
        free(t);
      }
    }
  }
  else if(m_markType == COLUMN_MARK)
  {
    int i;
    TxtLine *l = doc->GetLineNo(m_markStartY);
    for(i = m_markStartY; i <= m_markEndY && l; i++, l = doc->GetNext(l))
    {
      int len = MIN(m_markEndX, l->GetTextLen()) - m_markStartX;
      if(len > 0)
      {
        pc = new UndoCutTextEntryType;
        pc->m_text = new char [len + 1];
        l->CopyTextAt(pc->m_text, m_markStartX, len);
        pc->m_text[len] = 0;
        pc->m_x = m_markStartX;
        pc->m_y = i;
        m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, pc);

        l->ToUpper(m_markStartX, m_markEndX);

        char *t = (char *)malloc(len + 2);
        l->CopyTextAt(t, m_markStartX, len);
        t[len] = 0;
        m_undoList.AddInsertEntry(0, 0, m_markStartX, i, t);
        free(t);
      }
    }
  }

  UpdateAll();
  HandlePostActions(PA_SET_CURSOR | PA_REMOVE_MARK_STICKY);
}

void WainView::ToLower(void)
{
  if(m_markType == NO_MARK)
  {
    SetStatusText("No mark");
    return;
  }
  WainDoc *doc = GetDocument();
  if(doc->GetReadOnly())
    return;
  HandlePreActions(PA_REMOVE_CURSOR);

  UndoCutTextEntryType *pc;

  if(m_markType == CUA_MARK)
  {
    TxtLine *l = doc->GetLineNo(m_markStartY);
    if(m_markEndY == m_markStartY)
    {
      pc = new UndoCutTextEntryType;
      pc->m_text = new char [m_markEndX - m_markStartX + 1];
      l->CopyTextAt(pc->m_text, m_markStartX, m_markEndX - m_markStartX);
      pc->m_text[m_markEndX - m_markStartX] = 0;
      pc->m_x = m_markStartX;
      pc->m_y = m_markStartY;
      m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, pc);

      l->ToLower(m_markStartX, m_markEndX);

      char *t = (char *)malloc(m_markEndX - m_markStartX + 2);
      l->CopyTextAt(t, m_markStartX, m_markEndX - m_markStartX);
      t[m_markEndX - m_markStartX] = 0;
      m_undoList.AddInsertEntry(0, 0, m_markStartX, m_markStartY, t);
      free(t);
    }
    else
    {
      if(m_markStartX < l->GetTabLen())
      {
        pc = new UndoCutTextEntryType;
        pc->m_text = new char [l->GetTabLen() - m_markStartX + 1];
        l->CopyTextAt(pc->m_text, m_markStartX, l->GetTabLen() - m_markStartX);
        pc->m_text[l->GetTabLen() - m_markStartX] = 0;
        pc->m_x = m_markStartX;
        pc->m_y = m_markStartY;
        m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, pc);

        l->ToLower(m_markStartX);

        char *t = (char *)malloc(l->GetTabLen() - m_markStartX + 2);
        l->CopyTextAt(t, m_markStartX, l->GetTabLen() - m_markStartX);
        t[l->GetTabLen() - m_markStartX] = 0;
        m_undoList.AddInsertEntry(0, 0, m_markStartX, m_markStartY, t);
        free(t);
      }
      l = doc->GetNext(l);

      int i;
      for(i = m_markStartY + 1; i < m_markEndY && l; i++, l = doc->GetNext(l))
      {
        pc = new UndoCutTextEntryType;
        pc->m_text = new char [l->GetTabLen()+ 1];
        l->CopyTextAt(pc->m_text, 0, l->GetTabLen());
        pc->m_text[l->GetTabLen()] = 0;
        pc->m_x = 0;
        pc->m_y = i;
        m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, pc);

        l->ToLower();

        char *t = (char *)malloc(l->GetTabLen() + 2);
        l->CopyTextAt(t, 0, l->GetTabLen());
        t[l->GetTabLen()] = 0;
        m_undoList.AddInsertEntry(0, 0, 0, i, t);
        free(t);
      }
      if(l)
      {
        int len = MIN(m_markEndX, l->GetTabLen());
        pc = new UndoCutTextEntryType;
        pc->m_text = new char [len + 1];
        l->CopyTextAt(pc->m_text, 0, len);
        pc->m_text[len] = 0;
        pc->m_x = 0;
        pc->m_y = m_markEndY;
        m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, pc);

        l->ToLower(0, m_markEndX);

        char *t = (char *)malloc(len + 2);
        l->CopyTextAt(t, 0, len);
        t[len] = 0;
        m_undoList.AddInsertEntry(0, 0, 0, m_markEndY, t);
        free(t);
      }
    }
  }
  else if(m_markType == COLUMN_MARK)
  {
    int i;
    TxtLine *l = doc->GetLineNo(m_markStartY);
    for(i = m_markStartY; i <= m_markEndY && l; i++, l = doc->GetNext(l))
    {
      int len = MIN(m_markEndX, l->GetTextLen()) - m_markStartX;
      if(len > 0)
      {
        pc = new UndoCutTextEntryType;
        pc->m_text = new char [len + 1];
        l->CopyTextAt(pc->m_text, m_markStartX, len);
        pc->m_text[len] = 0;
        pc->m_x = m_markStartX;
        pc->m_y = i;
        m_undoList.AddEntry(UNDO_CUT_TEXT_ENTRY, 0, 0, 0, pc);

        l->ToLower(m_markStartX, m_markEndX);

        char *t = (char *)malloc(len + 2);
        l->CopyTextAt(t, m_markStartX, len);
        t[len] = 0;
        m_undoList.AddInsertEntry(0, 0, m_markStartX, i, t);
        free(t);
      }
    }
  }

  UpdateAll();
  HandlePostActions(PA_SET_CURSOR | PA_REMOVE_MARK_STICKY);
}

void WainView::ScrollLock(void)
{
  if(wainApp.gs.m_createMode == CreateModeType::CREATE_DUAL)
  {
    BOOL was_on = m_scrollLockOn; /*  SetScrollLock() will disable ScrollLock */
    if(!m_scrollLockOn)
      GetMf()->SetScrollLock();
    m_scrollLockOn = !was_on;
  }
  else
  {
    WainMessageBox(this, "You must use dual windows to get scroll lock to work", IDC_MSG_OK, IDI_INFORMATION_ICO);
    m_scrollLockOn = FALSE;
  }
  GetMf()->SetScrollLock_status(m_scrollLockOn);
}

void WainView::DisableScrollLock(void)
{
  m_scrollLockOn = FALSE;
}

void WainView::InsertTab(void)
{
  WainDoc *m_doc = GetDocument();
  if(m_doc->GetReadOnly())
    return;
  HandlePreActions(PA_REMOVE_CURSOR);
  m_currentTextLine->InsertAt(m_columnNo, '\t');
  int n = TAB_WIDTH(m_doc) - m_columnNo%TAB_WIDTH(m_doc);
  m_columnNo += n;
  PutText(NULL, m_currentTextLine, m_lineNo);
  m_undoList.AddEntry(UNDO_CHAR_ENTRY, n, 0, '\t');
  HandlePostActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE);
}

void WainView::RemoveTabs(void)
{
  WainDoc *doc = GetDocument();
  if(doc->GetReadOnly())
    return;
  HandlePreActions(PA_REMOVE_CURSOR);
  TxtLine *l;
  for(l = doc->GetLineNo(0); l; l = doc->GetNext(l))
    l->RemoveTabs();

  UpdateAll();
  HandlePostActions(PA_SET_CURSOR);
}

void WainView::ViewShellContextMenu()
{
  POINT p;
  GetPopupPos(&p, wainApp.gs.m_popupMenu.size());
  GetMf()->DoContextMenu(p, GetDocument()->GetPathName());
}

void WainView::ScrollLineCenter(void)
{
   RECT cr;
   GetClientRect(&cr);
   int Middle = (cr.bottom - cr.top)/2;
   ScrollLinePos(Middle);
}

void WainView::ScrollLineTop(void)
{
   ScrollLinePos(m_lineHeight);
}

void WainView::ScrollLineBottom(void)
{
   RECT cr;
   GetClientRect(&cr);
   int Bottom = (cr.bottom - cr.top) - m_lineHeight;
   ScrollLinePos(Bottom);
}

void WainView::ScrollLinePos(int aPos)
{
   HandlePreActions(PA_REMOVE_CURSOR);

   // int max = m_maxScroll.cy;

   if(m_lineNo*m_lineHeight - m_yOffset > aPos)
   {
      int Num = (m_lineNo*m_lineHeight - m_yOffset - aPos)/m_lineHeight;
      m_yOffset += Num*m_lineHeight;
  }
   else
   {
      int Num = (aPos - (m_lineNo*m_lineHeight - m_yOffset))/m_lineHeight;
      m_yOffset -= Num*m_lineHeight;
   }
   if(m_yOffset > m_maxScroll.cy)
      m_yOffset = m_maxScroll.cy;
   else if(m_yOffset < 0)
      m_yOffset = 0;
   SetScrollPos(SB_VERT, m_yOffset, TRUE);
   UpdateAll();
   UpdateTabViewPos();
   HandlePreActions(PA_SET_CURSOR);

}
