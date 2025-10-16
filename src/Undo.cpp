//=============================================================================
// This source code file is a part of Wain.
// It implements Undo_entry_class and UndoList_class as defined in WainView.h.
// Undo from WainView is also implemented here.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\Wain.h"

#include ".\..\src\WainDoc.h"
#include ".\..\src\WainView.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\taglist.h"
#include ".\..\src\DocProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UndoEntryClass::UndoEntryClass()
{
  m_x = m_y = -1;
  m_next = m_prev = 0;
  m_checkMark = 0xDEADBEEF;
}

UndoEntryClass::~UndoEntryClass()
{
  ASSERT(m_checkMark == 0xDEADBEEF);
}

UndoListClass::UndoListClass()
{
   m_list.m_next = m_list.m_prev = &m_list;
   m_redoList.m_next = m_redoList.m_prev = &m_redoList;
   m_nofEntries = 0;
}

UndoListClass::~UndoListClass()
{
   Clear();
}

void UndoListClass::ClearEntry(UndoEntryClass *entry)
{
//   ASSERT(entry->CheckMark == 0xDEADBEEF);
   if(entry->m_checkMark != 0xDEADBEEF)
      ::MessageBox(0, "Ups", "Ups", MB_OK);

   if(entry->m_type == UNDO_CUT_ENTRY || entry->m_type == UNDO_COPY_CLIP_OLD_DATA_ENTRY ||
      entry->m_type == UNDO_COPY_CLIP_ENTRY)
   {
      UndoCutEntryType *p = (UndoCutEntryType *)(entry->m_p);
      delete [] p->m_buffer;
   }
   if(entry->m_type == UNDO_CUT_TEXT_ENTRY || entry->m_type == UNDO_INSERT_ENTRY ||
      entry->m_type == UNDO_REMOVE_LINE_ENTRY)
   {
      UndoInsertEntryType *p = (UndoInsertEntryType *)(entry->m_p);
      delete [] (char *)p->m_text;
   }
   if(entry->m_p)
      delete entry->m_p;
   delete entry;
}

void UndoListClass::Clear(void)
{
  UndoEntryClass *entry;
  while((entry = m_list.m_next) != &m_list)
  {
    // ASSERT(entry->CheckMark == 0xDEADBEEF);
    if(entry->m_checkMark != 0xDEADBEEF)
       ::MessageBox(0, "Ups", "Ups", MB_OK);

    m_list.m_next->m_next->m_prev = &m_list;
    m_list.m_next = m_list.m_next->m_next;
    ClearEntry(entry);
  }
  while((entry = m_redoList.m_next) != &m_redoList)
  {
    ASSERT(entry->m_checkMark == 0xDEADBEEF);
    m_redoList.m_next->m_next->m_prev = &m_redoList;
    m_redoList.m_next = m_redoList.m_next->m_next;
    ClearEntry(entry);
  }
  m_nofEntries = 0;
}

UndoEntryClass *UndoListClass::AddEntry(UndoEntryTypeType type, int x, int y, int add_i, void *add_p)
{
  UndoEntryClass *entry = NULL;
  if(m_view && m_view->m_addUndo)
  {
    if(type == UNDO_START_ENTRY &&
       !wainApp.gs.m_flushUndoOnSave &&
       ++m_nofEntries > wainApp.gs.m_maxUndoBufferSize)
    { /* We have to remove a entry */
      do
      {
        entry = m_list.m_prev;
        m_list.m_prev = m_list.m_prev->m_prev;
        m_list.m_prev->m_next = &m_list;

        ASSERT(entry->m_checkMark == 0xDEADBEEF);

        if(entry->m_type == UNDO_START_ENTRY)
          m_nofEntries--;
        ClearEntry(entry);
      }
      while(m_nofEntries > wainApp.gs.m_maxUndoBufferSize);
    }
    entry = new UndoEntryClass;
    entry->m_x = x;
    entry->m_y = y;
    entry->m_i = add_i;
    entry->m_p = add_p;
    entry->m_type = type;

    entry->m_prev = &m_list;
    entry->m_next = m_list.m_next;
    m_list.m_next->m_prev = entry;
    m_list.m_next = entry;
    ASSERT(entry->m_checkMark == 0xDEADBEEF);
  }
  else if(add_p)
    delete add_p;


  return entry;
}

UndoEntryClass *UndoListClass::AddInsertEntry(int mx, int my, int xp, int yp, const char *txt)
{
  UndoInsertEntryType *p = new UndoInsertEntryType;
  p->m_text = new char [strlen(txt) + 1];
  strcpy(p->m_text, txt);
  p->m_x = xp;
  p->m_y = yp;
  return AddEntry(UNDO_INSERT_ENTRY, mx, my, 0, p);
}

void WainView::Undo(void)
{
  m_addUndo = FALSE; // We don't want to add Undo entrys now
  BOOL redraw_all = FALSE;
  WainDoc *m_doc = GetDocument(); // For SEPS
  if(m_undoList.m_list.m_next != &m_undoList.m_list)
  {
    HandlePreActions(PA_REMOVE_CURSOR);
    UndoEntryClass *e;
    do
    {
      e = m_undoList.m_list.m_next;
      ASSERT(e->m_checkMark == 0xDEADBEEF);
      e->m_next->m_prev = e->m_prev;  // remove this entry from the Undo list and put it into the redo list
      e->m_prev->m_next = e->m_next;
      e->m_prev = &m_undoList.m_redoList;
      e->m_next = m_undoList.m_redoList.m_next;
      m_undoList.m_redoList.m_next->m_prev = e;
      m_undoList.m_redoList.m_next = e;

      if(e->m_type == UNDO_START_ENTRY)
        m_undoList.m_nofEntries--;

      if(e->m_type == UNDO_MOVE_ENTRY || e->m_x || e->m_y)
      {
        m_columnNo -= e->m_x;
        m_lineNo   -= e->m_y;
        WainDoc *doc = GetDocument();
        m_currentTextLine = doc->GetLineNo(m_lineNo);
        ASSERT(m_currentTextLine);
      }
      switch(e->m_type)
      {
        case UNDO_CHAR_ENTRY:
          if(e->m_i == '\t')
          {
            m_currentTextLine->DeleteAt(m_columnNo, m_columnNo + e->m_x);
          }
          else
          {
            if(m_insert)
            {
              m_currentTextLine->DeleteAt(m_columnNo);
            }
            else
            {
              m_currentTextLine->ReplaceAt(m_columnNo, (char )(e->m_i >> 8));
            }
          }
          PutText(NULL, m_currentTextLine, m_lineNo);
          break;
        case UNDO_DELETE_ENTRY:
          if(e->m_i == '\n')
          {
            if(e->m_y)
            {
              TxtLine *pl = m_doc->GetRelLineNo(e->m_y, m_currentTextLine);
              pl->SplitLineAt(e->m_x);
              m_currentTextLine = m_doc->GetRelLineNo(-e->m_y, pl);
            }
            else
              m_currentTextLine->SplitLineAt(m_columnNo);
            redraw_all = TRUE;
          }
          else
          {
            m_currentTextLine->InsertAt(m_columnNo + e->m_x, (char )e->m_i);
            PutText(NULL, m_currentTextLine, m_lineNo);
          }
          break;
        case UNDO_RETURN_ENTRY:
          m_doc->JoinLine(m_currentTextLine);
          redraw_all = TRUE;
          break;
        case UNDO_INSERT_TOGGLE_ENTRY:
          InsertToggle();
          break;
        case UNDO_MARK_ENTRY:
          UndoMarkEntryType *p;
          p = (UndoMarkEntryType *)e->m_p;
          int sy;
          sy = MIN(m_markStartY, p->m_sY);
          int ey;
          ey = MAX(m_markEndY, p->m_eY);
          m_markStartX = p->m_sX;
          m_markEndX = p->m_eX;
          m_markStartY = p->m_sY;
          m_markEndY = p->m_eY;
          m_markType = p->m_type;
          DrawMarkLines(sy, ey);
          if(m_markStartX == m_markEndX && m_markStartY == m_markEndY && !m_columnMarking)
            m_markType = NO_MARK;
          GetMf()->SetMarkStatus(m_markType != NO_MARK || m_columnMarking);
          break;
        case UNDO_COPY_CLIP_OLD_DATA_ENTRY:
          UndoCopyClipOldDataEntryType *np;
          np = (UndoCopyClipOldDataEntryType *)e->m_p;
          if(OpenClipboard())
          {
            if(::EmptyClipboard())
            {
              HGLOBAL data = ::GlobalAlloc(GMEM_DDESHARE, strlen(np->m_buffer) + 1);
              if(data)
              {
                char *out_str = (char *)::GlobalLock(data);
                if(out_str)
                {
                  strcpy(out_str, np->m_buffer);
                  ::SetClipboardData(np->m_type == CUA_MARK ? CF_TEXT : WainColumnTextFormat, data);
                  ::CloseClipboard();
                }
              }
              else
                ::CloseClipboard();
            }
            else
             ::CloseClipboard();
          }
          break;
        case UNDO_COPY_CLIP_ENTRY:
          UndoCopyClipEntryType *u;
          u = (UndoCopyClipEntryType *)e->m_p;
          m_markStartX = u->m_sX;
          m_markEndX   = u->m_eX;
          m_markStartY = u->m_sY;
          m_markEndY   = u->m_eY;
          m_markType  = u->m_type; // We do not have to care about the data
          break;
        case UNDO_CUT_TEXT_ENTRY:
          UndoCutTextEntryType *tp;
          tp = (UndoCutTextEntryType *)e->m_p;
          ASSERT(tp);
          TxtLine *l;
          l = m_doc->GetLineNo(tp->m_y);
          ASSERT(l);
          l->InsertAt(tp->m_x, tp->m_text, strlen(tp->m_text));
          redraw_all = TRUE;
          break;
        case UNDO_INSERT_LINE_ENTRY:
          TxtLine *li;
          li = m_doc->GetLineNo(e->m_i);
          ASSERT(li);
          m_doc->RemoveLine(li);
          redraw_all = TRUE;
          break;
        case UNDO_INSERT_ENTRY:
          UndoInsertEntryType *xp;
          xp = (UndoInsertEntryType *)e->m_p;
          ASSERT(xp);
          TxtLine *ln;
          ln = m_doc->GetLineNo(xp->m_y);
          ASSERT(ln);
          char *us;
          int ul;

          for(us = xp->m_text, ul = xp->m_x; *us; us++)
          {
            if(*us == '\t')
              ul += TAB_WIDTH(m_doc) - ul % TAB_WIDTH(m_doc);
            else
              ul++;
          }
          ln->DeleteAt(xp->m_x, ul);
          redraw_all = TRUE;
          break;
        case UNDO_CUT_ENTRY:
          UndoCutEntryType *dp;
          dp = (UndoCutEntryType *)e->m_p;
          ASSERT(dp);
          TxtLine *lx;
          lx = m_doc->GetLineNo(dp->m_sY);
          if(!lx && dp->m_sY > 0 && (lx = m_doc->GetLineNo(dp->m_sY - 1)) != NULL)
          { /* In case the last line has been removed, by eg cut special */
            m_doc->InsertLine(lx, "");
            lx = m_doc->GetNext(lx);
            ASSERT(lx);
          }
          if(lx)
          {
            CString temp = "";
            char *s;
            s = dp->m_buffer;
            unsigned int i;
            BOOL first = TRUE;
            if(dp->m_type == CUA_MARK)
            {
              while(*s)
              {
                temp = "";
                for(i = 0; !strchr("\r\n", *s); i++, s++)
                  temp += *s;
                if(*s == '\n' || *s == '\r')
                {
                  s += 2;
                  int len = temp.GetLength();
                  #if 0
                  lx->InsertAt(first ? dp->sx : 0, temp, len);
                  lx = doc->SplitLineAt(lx, (first ? dp->sx : 0) + len);
                  #else
                  m_doc->SplitLineAt(lx, first ? dp->m_sX : 0);
                  lx->InsertAt(first ? dp->m_sX : 0, temp, len);
                  lx = m_doc->GetNext(lx);
                  ASSERT(lx);
                  #endif
                  first = FALSE;
                }
                else
                {
                  lx->InsertAt(first ? dp->m_sX : 0, temp, temp.GetLength());
                }
              }
            }
            else /* COLUMN_MARK */
            {
              int j;
              for(j = dp->m_sY; j <= dp->m_eY && *s; j++)
              {
                temp = "";
                for(i = 0; !strchr("\r\n", *s); i++, s++)
                  temp += *s;

                while(*s == '\n' || *s == '\r')
                  s++;
                lx->InsertAt(dp->m_sX, temp);
                lx = m_doc->GetNext(lx);
              }
            }
          }
          else
            *m_doc += "";
          redraw_all = TRUE;
          break;
        case UNDO_TOGGLE_COLUMN_MARK_ENTRY:
          if(m_columnMarking)
            m_columnMarking = FALSE;
          else
            m_columnMarking = TRUE;
          break;
        case UNDO_SHIFT_BLOCK_ENTRY:
          if(e->m_i == 1)
            ShiftBlockLeft();
          else
            ShiftBlockRight();
          break;
        case UNDO_MOVE_ENTRY:
        case UNDO_START_ENTRY:
          break;
        case UNDO_REMOVE_LINE_ENTRY:
          UndoRemoveLineEntryType *rle;
          rle = (UndoRemoveLineEntryType *)e->m_p;
          TxtLine *rli;
          rli = m_doc->GetLineNo(rle->m_y);
          m_doc->InsertLineBefore(rli, rle->m_text);
          redraw_all = TRUE;
          break;
        default:
          WainMessageBox(GetMf(), "Unknown Undo entry type", IDC_MSG_OK, IDI_ERROR_ICO);
          break;
      }
      if(m_markType != NO_MARK && m_markType != CUA_MARK && m_markType != COLUMN_MARK)
        SetStatusText("Wrong mark type");
    }
    while(e->m_type != UNDO_START_ENTRY && m_undoList.m_list.m_next != &m_undoList.m_list);

    HandlePreActions(PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE | PA_UPDATE_CURSOR_POS);
    if(redraw_all)
      UpdateAll();
  }
  m_currentTextLine = m_doc->GetLineNo(m_lineNo);
  m_addUndo = TRUE;
}


