//=============================================================================
// This source code file is a part of Wain.
// It implements some of the functions for WainView, the rest can be found
// in WainFunc.cpp, WainEdit.cpp, WainMark.cpp and TagList.cpp.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\Wain.h"

#include ".\..\src\WainDoc.h"
#include ".\..\src\WainView.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\taglist.h"
#include ".\..\src\tabview.h"
#include ".\..\src\childfrm.h"
#include ".\..\src\SimpleDialog.h"
#include ".\..\src\DocProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#pragma warning (disable : 4100) // unreferenced formal parameter

static char EmptyLine[MAX_LINE_LEN];
static char TabLine[] = "×                                       ";

static const unsigned int ScrollLockMessages[] =
{
  IDV_MOVE_LEFT, IDV_MOVE_RIGHT, IDV_MOVE_UP, IDV_MOVE_DOWN,
  IDV_MOVE_END, IDV_MOVE_HOME, IDV_MOVE_PAGE_DOWN, IDV_MOVE_PAGE_UP,
  IDV_MOVE_LEFT_EX, IDV_MOVE_RIGHT_EX
};

#define X_OFF 5 // Define the margin to the left.

IMPLEMENT_DYNCREATE(WainView, CView)

BEGIN_MESSAGE_MAP(WainView, CView)
  ON_COMMAND(ID_FILE_PRINT, PrintFile)
  ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
  ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
  ON_COMMAND_RANGE(FIRST_DISP_VAL, LAST_DISP_VAL, Dispatcher)
  ON_COMMAND_RANGE(IDM_COMP_ITEM0, IDM_COMP_ITEM19, CompletionFunc)

  ON_COMMAND(ID_WORD_LIST, WordList)
  ON_COMMAND(ID_WORD_LIST_REDO, WordListRedo)
  ON_COMMAND(ID_TAG_LIST,  TagList)
  ON_COMMAND(ID_TAG_LIST_CLASS,     TagListClass)
  ON_COMMAND(ID_TAG_LIST_DEFINE,    TagListDefine)
  ON_COMMAND(ID_TAG_LIST_TYPE,      TagListType)
  ON_COMMAND(ID_TAG_LIST_STRUCT,    TagListStruct)
  ON_COMMAND(ID_TAG_LIST_FUNCTION,  TagListFunction)
  ON_COMMAND(ID_TAG_LIST_PROTOTYPE, TagListPrototype)
  ON_COMMAND(ID_TAG_LIST_ENUM_NAME, TagListEnumName)
  ON_COMMAND(ID_TAG_LIST_ENUM,      TagListEnum)
  ON_COMMAND(ID_TAG_LIST_VAR,       TagListVar)
  ON_COMMAND(ID_TAG_LIST_EXTERN,    TagListExtern)
  ON_COMMAND(ID_TAG_LIST_MEMBER,    TagListMember)

  ON_COMMAND(IDM_VIEW_POPUP_MENU,   ViewPopupMenu)
  ON_COMMAND(IDM_MACRO_RECORD,      MacroRecordToggle)
  ON_COMMAND(IDM_MACRO_PLAYBACK,    MacroPlayback)

  ON_WM_SIZE()
  ON_WM_PAINT()
  ON_WM_SETFOCUS()
  ON_WM_KILLFOCUS()
  ON_WM_ACTIVATE()
  ON_WM_VSCROLL()
  ON_WM_HSCROLL()
  ON_WM_CHAR()
  ON_WM_RBUTTONDOWN()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_MOUSEMOVE()
  ON_WM_ERASEBKGND()
  ON_WM_TIMER()
  ON_WM_SETCURSOR()
  ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP();

#define DISPATCHER_ENTRY(id, func) DispatcherTable[id - FIRST_DISP_VAL].f_ptr = (&WainView::func)

WainView::WainView() :
  m_matchStartLine(0),
  m_matchStartLineNo(0),
  m_matchStartColumn(0),
  m_matchStartLength(0),
  m_matchEndLine(0),
  m_matchEndLineNo(0),
  m_matchEndLength(0),
  m_matchEndColumn(0)
{
    m_wasVScrollOn = FALSE;
    m_wasHScrollOn = 3;
    m_mouseDown = FALSE;
    m_macroRecording = FALSE;
    m_first = true;
    m_columnNo = m_lineNo = 0;
    m_yOffset = 0;
    m_xOffset = 0;
    m_currentTextLine = NULL;
    m_marking = FALSE;
    memset(EmptyLine, ' ', MAX_LINE_LEN - 1);

    m_markEndX = m_markStartX = 0;
    m_markEndY = m_markStartY = 0;
    m_markOrgX = m_markOrgY = 0;
    m_winNr = -1;

    m_pCtrlSite = NULL;
    m_markType = NO_MARK;
    m_lastAction = 0;
    m_cursorOn = FALSE;
    m_addUndo = TRUE;
    m_insert = TRUE;
    m_lastPutTextLine = NULL;
    m_undoList.m_view = this;
    m_columnMarking = FALSE;
    m_incSearchOn = FALSE;
    m_markWordAction = NO_ACTION;
    if(AfxGetApp()->GetProfileInt("Search Dialog", "IgnoreCase", TRUE))
       m_searchFlags = SEARCH_IGNORE_CASE;
    else
       m_searchFlags = 0;
    m_textOffset = 0;
    m_specialClip = FALSE;
    m_vertScrollOn = FALSE;
    m_charWidth = 0;
    m_lineHeight = 0;
    m_inSetScrollInfo = FALSE;
    m_tabView = NULL;
    m_maxLineLen = MAX_LINE_LEN;

    UINT nof = 1;
    BOOL res = SystemParametersInfo(SPI_GETWHEELSCROLLLINES, NULL, &nof, 0);
    if(!res)
       nof = 3;
    m_mouseWheelPrNotch = (int )nof;
    m_scrollLockOn = FALSE;

    memset(DispatcherTable, 0, sizeof(DispatcherTable));
    DISPATCHER_ENTRY(IDV_MOVE_LEFT,           MoveLeft);
    DISPATCHER_ENTRY(IDV_MOVE_RIGHT,          MoveRight);
    DISPATCHER_ENTRY(IDV_MOVE_LEFT_EX,        MoveLeftEx);
    DISPATCHER_ENTRY(IDV_MOVE_RIGHT_EX,       MoveRightEx);
    DISPATCHER_ENTRY(IDV_MOVE_UP,             MoveUp);
    DISPATCHER_ENTRY(IDV_MOVE_DOWN,           MoveDown);
    DISPATCHER_ENTRY(IDV_MOVE_PAGE_DOWN,      MovePageDown);
    DISPATCHER_ENTRY(IDV_MOVE_PAGE_UP,        MovePageUp);
    DISPATCHER_ENTRY(IDV_MOVE_HOME,           MoveHome);
    DISPATCHER_ENTRY(IDV_MOVE_END,            MoveEnd);
    DISPATCHER_ENTRY(IDV_SAVE_FILE,           SaveFile);
    DISPATCHER_ENTRY(IDV_HANDLE_RETURN,       HandleReturn);
    DISPATCHER_ENTRY(IDV_HANDLE_RETURN_EX,    HandleReturnEx);
    DISPATCHER_ENTRY(IDV_HANDLE_DELETE,       HandleDelete);
    DISPATCHER_ENTRY(IDV_HANDLE_DELETE_EX,    HandleDeleteEx);
    DISPATCHER_ENTRY(IDV_HANDLE_BACKSPACE,    HandleBackspace);
    DISPATCHER_ENTRY(IDV_HANDLE_BACKSPACE_EX, HandleBackspaceEx);
    DISPATCHER_ENTRY(IDV_GOTO_DOC_0,          GotoDoc0);
    DISPATCHER_ENTRY(IDV_GOTO_DOC_1,          GotoDoc1);
    DISPATCHER_ENTRY(IDV_GOTO_DOC_2,          GotoDoc2);
    DISPATCHER_ENTRY(IDV_GOTO_DOC_3,          GotoDoc3);
    DISPATCHER_ENTRY(IDV_GOTO_DOC_4,          GotoDoc4);
    DISPATCHER_ENTRY(IDV_GOTO_DOC_5,          GotoDoc5);
    DISPATCHER_ENTRY(IDV_GOTO_DOC_6,          GotoDoc6);
    DISPATCHER_ENTRY(IDV_GOTO_DOC_7,          GotoDoc7);
    DISPATCHER_ENTRY(IDV_GOTO_DOC_8,          GotoDoc8);
    DISPATCHER_ENTRY(IDV_GOTO_DOC_9,          GotoDoc9);
    DISPATCHER_ENTRY(IDV_COPY_CLIP,          CopyToClipboard);
    DISPATCHER_ENTRY(IDV_MARK_DOWN,          MarkDown);
    DISPATCHER_ENTRY(IDV_MARK_UP,            MarkUp);
    DISPATCHER_ENTRY(IDV_MARK_LEFT,          MarkLeft);
    DISPATCHER_ENTRY(IDV_MARK_RIGHT,         MarkRight);
    DISPATCHER_ENTRY(IDV_PASTE_CLIP,         PasteClip);
    DISPATCHER_ENTRY(IDV_MARK_CUT,           MarkCut);
    DISPATCHER_ENTRY(IDV_MARK_DELETE,        MarkDelete);
    DISPATCHER_ENTRY(IDV_MDI_MAX,            MdiMax);
    DISPATCHER_ENTRY(IDV_MDI_CAS,            MdiCascade);
    DISPATCHER_ENTRY(IDV_FILE_OPEN_IN_LINE,  FileOpenInLine);
    DISPATCHER_ENTRY(IDV_EDIT_UNDO,          Undo);
    DISPATCHER_ENTRY(IDV_INSERT_TOGGLE,      InsertToggle);
    DISPATCHER_ENTRY(IDV_MOVE_WORD_RIGHT,    MoveWordRight);
    DISPATCHER_ENTRY(IDV_MOVE_WORD_LEFT,     MoveWordLeft);
    DISPATCHER_ENTRY(IDV_MARK_WORD_RIGHT,    MarkWordRight);
    DISPATCHER_ENTRY(IDV_MARK_WORD_LEFT,     MarkWordLeft);
    DISPATCHER_ENTRY(IDV_MARK_PAGE_UP,       MarkPageUp);
    DISPATCHER_ENTRY(IDV_MARK_PAGE_DOWN,     MarkPageDown);
    DISPATCHER_ENTRY(IDV_MARK_END,           MarkEnd);
    DISPATCHER_ENTRY(IDV_MARK_HOME,          MarkHome);

    DISPATCHER_ENTRY(IDV_MARK_WORD,          MarkWord);
    DISPATCHER_ENTRY(IDV_INSERT_TAB,         InsertTab);
    DISPATCHER_ENTRY(IDV_MDI_DUAL,           MdiDual);
    DISPATCHER_ENTRY(IDV_SHIFT_BLOCK_RIGHT,  ShiftBlockRight);
    DISPATCHER_ENTRY(IDV_SHIFT_BLOCK_LEFT,   ShiftBlockLeft);
    DISPATCHER_ENTRY(IDV_FIND_MATCH_BRACE,   FindMatchBrace);
    DISPATCHER_ENTRY(IDV_FIND_MATCH_BLOCK,   FindMatchBlock);
    DISPATCHER_ENTRY(IDV_COPY_SPECIAL,       CopySpecial);
    DISPATCHER_ENTRY(IDV_CUT_SPECIAL,        CutSpecial);
    DISPATCHER_ENTRY(IDV_PASTE_SPECIAL,      PasteSpecial);
    DISPATCHER_ENTRY(IDV_COMP_LIST,          CompletionList);
    DISPATCHER_ENTRY(IDV_COLUMN_MARK,        ColumnMarkToggle);
    DISPATCHER_ENTRY(IDV_MOVE_DOC_START,     MoveDocStart);
    DISPATCHER_ENTRY(IDV_MOVE_DOC_END,       MoveDocEnd);
    DISPATCHER_ENTRY(IDV_GOTO_LINE_NO,       GotoLineNo);
    DISPATCHER_ENTRY(IDV_INC_SEARCH,         IncSearch);
    DISPATCHER_ENTRY(IDV_SEARCH_NEXT,        SearchNext);
    DISPATCHER_ENTRY(IDV_SEARCH_PREV,        SearchPrev);
    DISPATCHER_ENTRY(IDV_HANDLE_ESC,         HandleEsc);
    DISPATCHER_ENTRY(IDV_RELOAD_FILE,        DoReloadFile);
    DISPATCHER_ENTRY(IDV_SCROLL_LINE_UP,     ScrollLineUp);
    DISPATCHER_ENTRY(IDV_SCROLL_LINE_DOWN,   ScrollLineDown);
    DISPATCHER_ENTRY(IDV_DELETE_WORD,        DeleteWord);
    DISPATCHER_ENTRY(IDV_INCREASE_MARK_TOP,  IncreaseMarkTop);
    DISPATCHER_ENTRY(IDV_INCREASE_MARK_BOT,  IncreaseMarkBot);
    DISPATCHER_ENTRY(IDV_DECREASE_MARK_TOP,  DecreaseMarkTop);
    DISPATCHER_ENTRY(IDV_DECREASE_MARK_BOT,  DecreaseMarkBot);
    DISPATCHER_ENTRY(IDV_WORD_COMP,          WordCompletion);
    DISPATCHER_ENTRY(IDV_REMOVE_MARK,        DoRemoveMark);
    DISPATCHER_ENTRY(IDV_EXPAND_TEMPLATE,    ExpandTemplate);
    DISPATCHER_ENTRY(IDV_INDENT_LINE,        IndentLine);
    DISPATCHER_ENTRY(IDV_MARK_TOUPPER,       ToUpper);
    DISPATCHER_ENTRY(IDV_MARK_TOLOWER,       ToLower);
    DISPATCHER_ENTRY(IDV_MOVE_NEXT_TAB,      MoveToNexTab);
    DISPATCHER_ENTRY(IDV_MOVE_PREV_TAB,      MoveToPrevTab);
    DISPATCHER_ENTRY(IDV_INDENT_NEXT_TAB,    IndentToNextTab);
    DISPATCHER_ENTRY(IDV_INDENT_PREV_TAB,    IndentToPrevTab);
    DISPATCHER_ENTRY(IDV_FILE_INFO,          FileInfo);
    DISPATCHER_ENTRY(IDV_SAVE_FILE_AS,       SaveFileAs);
    DISPATCHER_ENTRY(IDV_OPEN_AS_DOC,        OpenAsNormalDoc);
    DISPATCHER_ENTRY(IDV_TAG_PEEK,           TagPeek);
    DISPATCHER_ENTRY(IDV_SCROLL_LOCK,        ScrollLock);
    DISPATCHER_ENTRY(IDV_MARK_DOC_START,     MarkDocStart);
    DISPATCHER_ENTRY(IDV_MARK_DOC_END,       MarkDocEnd);
    DISPATCHER_ENTRY(IDV_PRINT,              Print);
    DISPATCHER_ENTRY(IDV_MOVE_MARK_START,    MoveToMarkStart);
    DISPATCHER_ENTRY(IDV_MOVE_MARK_END,      MoveToMarkEnd);
    DISPATCHER_ENTRY(IDV_MACRO_REPEAT,       HandleMacroRepeat);
    DISPATCHER_ENTRY(IDV_MOVE_LEFT_EX,       MoveLeftEx);
    DISPATCHER_ENTRY(IDV_MOVE_RIGHT_EX,      MoveRightEx);
    DISPATCHER_ENTRY(IDV_INSERT_TIME,        InsertTime);
    DISPATCHER_ENTRY(IDV_INSERT_DATE,        InsertDate);
    DISPATCHER_ENTRY(IDV_INSERT_TIME_DATE,   InsertTimeDate);
    DISPATCHER_ENTRY(IDV_INSERT_DATE_TIME,   InsertDateTime);
    DISPATCHER_ENTRY(IDV_INSERT_USER_ID,     InsertUserId);
    DISPATCHER_ENTRY(IDV_REMOVE_TABS,        RemoveTabs);
    DISPATCHER_ENTRY(IDV_GOTO_BLOCK_START,   GotoBlockStart);
    DISPATCHER_ENTRY(IDV_GOTO_BLOCK_END,     GotoBlockEnd);
    DISPATCHER_ENTRY(IDV_MOVE_HOME_EX,       MoveHomeEx);
    DISPATCHER_ENTRY(IDV_SHELL_CONTEXT_MENU, ViewShellContextMenu);
    DISPATCHER_ENTRY(IDV_MOVE_WORD_LEFT_EX,  MoveWordLeftEx);
    DISPATCHER_ENTRY(IDV_MOVE_WORD_RIGHT_EX, MoveWordRightEx);
    DISPATCHER_ENTRY(IDV_SCROLL_LINE_CENTER, ScrollLineCenter);
    DISPATCHER_ENTRY(IDV_SCROLL_LINE_TOP,    ScrollLineTop);
    DISPATCHER_ENTRY(IDV_SCROLL_LINE_BOTTOM, ScrollLineBottom);
    DISPATCHER_ENTRY(IDV_NEXT_MATCH_WORD,    NextMatchWord);
    DISPATCHER_ENTRY(IDV_PREV_MATCH_WORD,    PrevMatchWord);
    DISPATCHER_ENTRY(IDV_INSERT_COMMENT,     InsertComment);
    DISPATCHER_ENTRY(IDV_INSERT_COMMENT_DOWN,InsertCommentDown);
    DISPATCHER_ENTRY(IDV_MARK_ALL,           MarkAll);
    DISPATCHER_ENTRY(IDV_COPY_FILE_NAME,     CopyFileName);
    DISPATCHER_ENTRY(IDV_OPEN_DIR_FOR_FILE,  OpenDirForFile);
    DISPATCHER_ENTRY(IDV_OPEN_PROJECT_FOR_FILE,  OpenProjectForFile);
    DISPATCHER_ENTRY(IDV_SWITCH_CPP_H,       SwitchCppH);
    DISPATCHER_ENTRY(IDV_MARK_ALL_WORDS,     MarkSimilarWords);
    DISPATCHER_ENTRY(IDV_MARK_ALL_WORDS_TOGGLE,  MarkSimilarWordsToggle);
    DISPATCHER_ENTRY(IDV_MARK_ALL_WORDS_OFF,     MarkSimilarWordsOff);
}

WainView::~WainView()
{
   if(m_macroRecording)
   {
      GetMf()->MacroRecordToggle();
      SetStatusText("Macro recording has been stopped");
   }
   if(m_winNr != -1)
      GetMf()->RemoveView(m_winNr);

   m_backBrush.DeleteObject();
   m_markBrush.DeleteObject();
}

BOOL WainView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
  return CView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

BOOL WainView::PreCreateWindow(CREATESTRUCT &cs)
{
  BOOL bPreCreated = CView::PreCreateWindow(cs);
  return bPreCreated;
}

void WainView::OnInitialUpdate(void)
{
  DoInitUpdate();
}

void WainView::DoInitUpdate(void)
{
  WainDoc *doc = GetDocument();
  if(!doc->m_isDebugFile)
    m_winNr = GetMf()->AddView(this, doc->GetPathName(), doc->m_isFtpFile);
  else
    m_winNr = -1; // Don't care

  CDC *dc = GetDC();
  CFont *old_font = dc->SelectObject(TextWinFont);
  SetFontMetrics(*dc);

  dc->SelectObject(old_font);
  ReleaseDC(dc);
  SetScrollInfo();
  m_backBrush.CreateSolidBrush(doc->m_prop->m_color[TEXT_IDX].m_backColor);
  m_markBrush.CreateSolidBrush(doc->m_prop->m_color[MARK_IDX].m_backColor);
}

void WainView::SetScrollInfo(void)
{
  if(!m_lineHeight || m_inSetScrollInfo)
    return;
  m_inSetScrollInfo = TRUE;
  int lc = GetDocument()->m_lineCount;
  RECT cr;
  GetClientRect(&cr);
  CSize s1(m_maxLineLen*m_charWidth, m_lineHeight*(lc + 1));   // TODO
  if(s1.cy < 0)
    s1.cy = 0;
  CSize s2(cr.right/2, (cr.bottom/m_lineHeight - 2)*m_lineHeight);
  if(s2.cy < 0)
    s2.cy = 0;

  m_lineScroll = CSize(m_charWidth, m_lineHeight);
  m_pageScroll = s2;
  m_maxScroll = s1;
  if(wainApp.gs.m_horzScrollBar != (m_wasHScrollOn ? true : false))
  {
    EnableScrollBarCtrl(SB_HORZ, wainApp.gs.m_horzScrollBar);
    m_wasHScrollOn = wainApp.gs.m_horzScrollBar;
  }

  if(m_wasHScrollOn)
  {
    SCROLLINFO hsi;
    hsi.cbSize = sizeof(hsi);
    hsi.fMask = SIF_RANGE | SIF_PAGE;
    hsi.nMin = 0;
    hsi.nMax = s1.cx;
    hsi.nPage = s2.cx;
    CView::SetScrollInfo(SB_HORZ, &hsi);
  }
  if(s1.cy > cr.bottom)
    m_vertScrollOn = wainApp.gs.m_vertScrollBar;
  else
    m_vertScrollOn = FALSE;

  if(m_vertScrollOn != m_wasVScrollOn)
  {
    EnableScrollBarCtrl(SB_VERT, m_vertScrollOn);
    m_wasVScrollOn = m_vertScrollOn;
  }
  if(m_wasVScrollOn)
  {
    SCROLLINFO vsi;
    vsi.cbSize = sizeof(vsi);
    vsi.fMask = SIF_RANGE | SIF_PAGE;
    vsi.nMin = 0;
    vsi.nMax = s1.cy;
    vsi.nPage = s2.cy;
    CView::SetScrollInfo(SB_VERT, &vsi);
  }
  m_inSetScrollInfo = FALSE;
}

void WainView::OnDebugFileUpdate(int prev_LineCount)
{
  SetScrollInfo();
  InvalidateRect(NULL, TRUE);
  if(m_lineNo + 1 ==  prev_LineCount)
  {
    MoveDocEnd();
    MoveHome();
  }
}

void WainView::OnChar(UINT ch, UINT nRepCnt, UINT flags)
// Normal ASCII char is entered thru this function
{
   if(!m_currentTextLine)
      m_currentTextLine = GetDocument()->GetLineNo(m_lineNo);

   if(HandleSpecialModes(ID_PUT_CHAR, ch) == LEAVE_SPECIAL_MODE && !GetDocument()->GetReadOnly() && ch >= 32 && !(flags & (1 << 8 )))
   {
      m_undoList.AddEntry(UNDO_START_ENTRY, 0, 0);
      int i = m_currentTextLine->GetEmptyCharBefore(m_columnNo);
      if(i)
      {
         UndoInsertEntryType *p = new UndoInsertEntryType;
         p->m_text = new char [i + 1];
         memset(p->m_text, ' ', i);
         p->m_text[i] = '\0';
         p->m_x = m_columnNo - i;
         p->m_y = m_lineNo;
         m_undoList.AddEntry(UNDO_INSERT_ENTRY, 0, 0, 0, p);
      }
      if(m_insert)
      {
         m_currentTextLine->InsertAt(m_columnNo, (char )ch);
         m_undoList.AddEntry(UNDO_CHAR_ENTRY, 1, 0, ch);
      }
      else
      {
         char prev = m_currentTextLine->ReplaceAt(m_columnNo, (char )ch);
         m_undoList.AddEntry(UNDO_CHAR_ENTRY, 1, 0, ch | (prev << 8));
      }
      HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK);
      m_columnNo++;
      PutText(NULL, m_currentTextLine, m_lineNo);
      m_lastAction = 0;
      PutTextRest();
      #if 0
      DocPropClass *DocProp = wainApp.gs.m_docProp[GetDocument()->PropIndex];
      if((DocProp->BlockBegin.size() == 1 && DocProp->BlockBegin[0] == ch) ||
         (DocProp->BlockEnd.size() == 1 && DocProp->BlockEnd[0] == ch))
      {
         int OldCurrColumn = CurrentColumn;
         IndentLine();
         if(CurrentColumn != OldCurrColumn)
         {
            UndoList.AddEntry(UNDO_MOVE_ENTRY, 1, 0);
            CurrentColumn++;
         }
      }
      #endif
      HandlePostActions(PA_SET_TAG | PA_UPDATE_CURSOR_POS | PA_SET_CURSOR | PA_SCROLL_TO_VISIBLE);
   }
   if(m_macroRecording && !GetDocument()->GetReadOnly() && ch >= 32 && !(flags & (1 << 8 )))
      GetMf()->AddMacroEntry(ID_PUT_CHAR, (int )ch);
}

void WainView::PutText(CDC *dc, TxtLine *line, int y)
//  Description:
//    Draw one line of text.
//  Parameters:
//    dc,    the DC to be used, if NULL one will be provided
//    line,  the line which text has to be drawn
//    y,     the line number of the line
{
   #define VI_TAB wainApp.gs.m_visibleTabs

   #define DRAW_LINE(width, color_)                                                             \
   if(m_textOffset)                                                                               \
   {                                                                                            \
      CPen bp(PS_SOLID, 1,  color_);                                                            \
      CPen *old_pen = dc->SelectObject(&bp);                                                    \
      dc->MoveTo(text_pos, (y)*m_lineHeight - m_yOffset);                          \
      dc->LineTo(text_pos + width*m_charWidth, (y)*m_lineHeight - m_yOffset);        \
      dc->MoveTo(text_pos, (y)*m_lineHeight - m_yOffset + 1);                      \
      dc->LineTo(text_pos + width*m_charWidth, (y)*m_lineHeight - m_yOffset + 1);    \
      dc->MoveTo(text_pos, (y)*m_lineHeight - m_yOffset + 2);                      \
      dc->LineTo(text_pos + width*m_charWidth, (y)*m_lineHeight - m_yOffset + 2);    \
      dc->SelectObject(old_pen);                                                                \
   }
   #define Y_POS (y*m_lineHeight - m_yOffset + m_textOffset)

   #define SET_C(col) \
     dc->SetTextColor(col.m_textColor); \
     dc->SetBkColor(col.m_backColor)

   WainDoc *doc = GetDocument();
   const DocPropClass* Prop = doc->m_prop;
   BOOL rel = FALSE;
   if(!dc)
   {
      dc = GetDC();
      rel = TRUE;
   }
   CFont *old_font = dc->SelectObject(TextWinFont);
   int i, j, m = 0;

   EndLineStatusType prevEls = doc->GetPrevEls(line);
   EndLineStatusType thisPrevEls = line->m_endLineStatus;
   line->m_endLineStatus = line->GetTextColor(m_lineColor, prevEls, FALSE);
   if(line->m_endLineStatus != thisPrevEls)
   {
      m_lastPutTextLine = line;  // These are used to redraw the rest if repainting this will cause
      m_lastPutTextLineNo = y;  // The next line(s) to change color
   }
   else
   {
      m_lastPutTextLine = NULL;
   }

   enum class MarkingType
   {
     NoMarking,               // There is no marking in this line
     StartEndMarking,         // This line has both mark_start_x && MarkEnd_x
     StartMarking,            // This line has mark_start_x
     EndMarking,              // This line has MarkEnd_x
     TotalMarking             // The complete line has to be marked
   };
   MarkingType markingType =  MarkingType::NoMarking;

   if(m_markType == COLUMN_MARK || m_markStartY == m_markEndY)
   {
     if(y >= m_markStartY && y <= m_markEndY)
       markingType = MarkingType::StartEndMarking;
   }
   else if(m_markType == CUA_MARK)
   {
     if(y == m_markStartY)
       markingType = MarkingType::StartMarking;
     else if(y == m_markEndY)
       markingType = MarkingType::EndMarking;
     else if(y >= m_markStartY && y < m_markEndY)
       markingType = MarkingType::TotalMarking;
   }

   int text_pos;
   const char *t = line->GetText();
   int text_len = line->GetTabLen();
   int bm = 0, am = 0;
   dc->SetBkColor(Prop->m_color[TEXT_IDX].m_backColor);

   for(i = 0, j = 0, text_pos = X_OFF - m_xOffset; i < text_len && j < m_lineColor.GetNumColor(); i += m_lineColor[j].m_len, j++)
   {
     // Determine how much has to be marked
     switch(markingType)
     {
       case MarkingType::TotalMarking:
         bm = 0;
         m = m_lineColor[j].m_len;
         am = 0;
         break;
       case MarkingType::StartMarking:
         if(i + m_lineColor[j].m_len <= m_markStartX)
           bm = m_lineColor[j].m_len;
         else if(i >= m_markStartX)
           bm = 0;
         else
           bm = m_markStartX - i;
         m = m_lineColor[j].m_len - bm;
         am = 0;
         break;
       case MarkingType::EndMarking:
         bm = 0;
         if(i >= m_markEndX)
           am = m_lineColor[j].m_len;
         else if(i + m_lineColor[j].m_len < m_markEndX)
           am = 0;
         else
           am = i + m_lineColor[j].m_len - m_markEndX;
         m = m_lineColor[j].m_len - am;
         break;
       case MarkingType::StartEndMarking:
         if(i + m_lineColor[j].m_len <= m_markStartX)
           bm = m_lineColor[j].m_len;
         else if(i >= m_markStartX)
           bm = 0;
         else
           bm = m_markStartX - i;
         if(i >= m_markEndX)
           am = m_lineColor[j].m_len;
         else if(i + m_lineColor[j].m_len < m_markEndX)
           am = 0;
         else
           am = i + m_lineColor[j].m_len - m_markEndX;
         m = m_lineColor[j].m_len - bm - am;
         break;
       case MarkingType::NoMarking:
         bm = m_lineColor[j].m_len;
         m = 0;
         am = 0;
         break;
     }
     if(bm)
     {
       if(m_lineColor[j].m_index == TAB_IDX)
       {
         DRAW_LINE(m_lineColor[j].m_len, Prop->m_color[TEXT_IDX].m_backColor);
         SET_C(Prop->m_color[TEXT_IDX]);
         ::TextOut(dc->m_hDC, text_pos, Y_POS, VI_TAB ? TabLine : EmptyLine, m_lineColor[j].m_len);
       }
       else
       {
         DRAW_LINE(bm, Prop->m_color[m_lineColor[j].m_index].m_backColor);
         SET_C(Prop->m_color[m_lineColor[j].m_index]);
         ::TextOut(dc->m_hDC, text_pos, Y_POS, t, bm);
         t += bm;
       }
       text_pos += bm*m_charWidth;
     }
     if(m)
     {
       SET_C(doc->m_prop->m_color[MARK_IDX]);
       if(m_lineColor[j].m_index == TAB_IDX)
       {
         ::TextOut(dc->m_hDC, text_pos, Y_POS, bm || !VI_TAB ? EmptyLine : TabLine, m);
         DRAW_LINE(m, doc->m_prop->m_color[MARK_IDX].m_backColor);
       }
       else
       {
         ::TextOut(dc->m_hDC, text_pos, Y_POS, t, m);
         DRAW_LINE(m, doc->m_prop->m_color[MARK_IDX].m_backColor);
         t += m;
       }
       text_pos += m*m_charWidth;
     }
     if(am)
     {
       if(m_lineColor[j].m_index == TAB_IDX)
       {
         DRAW_LINE(am, doc->m_prop->m_color[TEXT_IDX].m_backColor);
         SET_C(doc->m_prop->m_color[TEXT_IDX]);
         ::TextOut(dc->m_hDC, text_pos, Y_POS, bm + m || !VI_TAB ? EmptyLine : TabLine, am);
       }
       else
       {
         DRAW_LINE(am, Prop->m_color[m_lineColor[j].m_index].m_backColor);
         SET_C(Prop->m_color[m_lineColor[j].m_index]);
         ::TextOut(dc->m_hDC, text_pos, Y_POS, t, am);
         t += am;
       }
       text_pos += am*m_charWidth;
     }
     if(m_lineColor[j].m_index == TAB_IDX)
       t++;
   }
   RECT mr;
   RECT er;
   GetClientRect(&mr);
   mr.top = Y_POS - m_textOffset;
   mr.bottom = mr.top + m_lineHeight;
   mr.left = text_pos;
   er = mr;

   // Fill the area after any text
   switch(markingType)
   {
     case MarkingType::TotalMarking:
       dc->FillRect(&mr, &m_markBrush);
       break;
     case MarkingType::StartMarking:
       if(i >= m_markStartX)
         dc->FillRect(&mr, &m_markBrush);
       else
       {
         er.right = text_pos + (m_markStartX - i)*m_charWidth;
         dc->FillRect(&er, &m_backBrush);
         mr.left = er.right;
         dc->FillRect(&mr, &m_markBrush);
       }
       break;
     case MarkingType::EndMarking:
       if(i >= m_markEndX)
         dc->FillRect(&er, &m_backBrush);
       else
       {
         mr.right = text_pos + (m_markEndX - i)*m_charWidth;
         dc->FillRect(&mr, &m_markBrush);
         er.left = mr.right;
         dc->FillRect(&er, &m_backBrush);
       }
       break;
     case MarkingType::StartEndMarking:
       int tt;
       tt = er.right;
       if(i < m_markStartX)
       {
         er.right = text_pos + (m_markStartX - i)*m_charWidth;
         dc->FillRect(&er, &m_backBrush);
         mr.left = er.right;
       }
       if(m_markEndX > i)
       {
         mr.right = text_pos + (m_markEndX - i)*m_charWidth;
         dc->FillRect(&mr, &m_markBrush);
         er.left = mr.right;
       }
       er.right = tt;
       dc->FillRect(&er, &m_backBrush);
       break;
     case MarkingType::NoMarking:
       dc->FillRect(&er, &m_backBrush);
       break;
   }
   dc->SetTextColor(Prop->m_color[TEXT_IDX].m_textColor);
   dc->SetBkColor(Prop->m_color[TEXT_IDX].m_backColor);
   dc->SelectObject(old_font);
   if(rel)
     ReleaseDC(dc);
   #undef VI_TAB
   #undef DRAW_LINE
   #undef Y_POS
   #undef SET_C
}

void WainView::OnSize(UINT type, int cx, int cy)
{
   if(type != SIZE_MAXHIDE && type != SIZE_MAXSHOW)
   {
      m_clientRect.left = 0;
      m_clientRect.top  = 0;
      m_clientRect.right = cx;
      m_clientRect.bottom = cy;
      if(!m_first)
      {
         if(m_cursorOn)
         {
            if(type == SIZE_RESTORED && wainApp.gs.m_createMode == CreateModeType::CREATE_MAX)
            { /* Nothing */
            }
            else
            {
               ScrollToVisible();
            }
         }
      }
      CView::OnSize(type, cx, cy);
   }
   SetScrollInfo();
}

void WainView::UpdateFont(void)
//   Function Name:  UpdateFont
//     Description:
//     Called when the font has been chanded, or other aspects of the way the text is viewed has been changed.
//     OnPaint will do the work.
{
   WainDoc *doc = GetDocument();
   doc->ReCalcTabLen();
   m_first = 2;
   m_backBrush.Detach();
   m_backBrush.CreateSolidBrush(doc->m_prop->m_color[TEXT_IDX].m_backColor);
   m_markBrush.Detach();
   m_markBrush.CreateSolidBrush(doc->m_prop->m_color[MARK_IDX].m_backColor);
   InvalidateRect(NULL, TRUE);
   UpdateTabViewPos();
}

void WainView::OnPaint(void)
//   Function Name:  0nPaint
//     Description:  Default paint function.
{
  BOOL co = m_cursorOn;
  RemoveCursor();

  CPaintDC dc(this);
  CFont *old_font = dc.SelectObject(TextWinFont);
  if(dc.GetMapMode() != MM_TEXT)
    dc.SetMapMode(MM_TEXT);
  RECT cr;
  GetClientRect(&cr);
  WainDoc *doc = GetDocument();
  ASSERT_VALID(doc);
  TxtLine *p_line;
  m_currentTextLine = doc->GetLineNo(m_lineNo);

   if(m_first)
   {
      if(m_first == 1)
      {
         // WainDoc *doc = GetDocument();
         doc->m_view = this;

         if(doc->m_childFrame)
         {
            std::string Title = (const char *)doc->GetPathName();
            if(doc->GetReadOnly())
               Title += " (RO)";
            doc->m_childFrame->SetWindowText(Title.c_str());
         }
      }
      else
      { // If font is changed
         SetFontMetrics(dc);
         // RECT cr;
         GetClientRect(&cr);
         dc.FillRect(&cr, &m_backBrush);
         if(GetFocus() == this)
           CreateSolidCaret(m_insert ? 2 : m_charWidth, m_lineHeight - 1);
         SetScrollInfo();
      }
      m_first = 0;
   }

  RECT br = cr;
  br.bottom += m_lineHeight;
  if(!m_lineHeight)
    return;
  int first_line = m_yOffset/m_lineHeight;
  if(first_line)
    first_line--;

  int last = (m_yOffset + cr.bottom)/m_lineHeight + 2;
  br = cr;
  br.right = X_OFF;
  dc.FillRect(&br, &m_backBrush); /* Erace the space before any text */
  if((doc->m_lineCount - 1)*m_lineHeight - m_yOffset < cr.bottom)
  {
    br = cr;
    br.top = (doc->m_lineCount - 1)*m_lineHeight - m_yOffset;
    dc.FillRect(&br, &m_backBrush);
  }
  int i;
  for(p_line = doc->GetLineNo(first_line), i = first_line; p_line != NULL && i < last; p_line = doc->GetNext(p_line), i++)
  {
    PutText(&dc, p_line, i);
  }
  dc.SelectObject(old_font);
  if(GetFocus() == this && co)
    SetCursor();
}

void WainView::OnDraw(CDC *dc)
{}

void WainView::MacroRecordToggle(void)
//     Description:  Toggle macro recording.
//      Parameters:
{
  if(GetMf()->MacroRecordingOn() != m_macroRecording)
  {
    SetStatusText("Macro recording is on in another view");
    return;
  }
  if(GetMf()->MacroRecordToggle())
    m_macroRecording = TRUE;
  else
    m_macroRecording = FALSE;
}

void WainView::PlaybackMacro(void)
{
  MacroPlayback();
}

void WainView::HandleMacroRepeat(void)
{
  if(GetMf()->m_macroList.m_recording)
  {
    SetStatusText("Can't playback a macro while recording");
    return;
  }
  SimpleDialog sd("Repeat Macro:", "", this);
  if(sd.DoModal() == IDOK)
  {
    int count;
    count = strtol(sd.m_msg, NULL, 0);
    while(count-- > 0)
    {
      MacroPlayback();
      if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)
      {
        SetStatusText("User abort");
        return;
      }
    }
  }
}

void WainView::MacroPlayback(void)
//     Description:  Playback macros.
//      Parameters:
{
  BOOL first = TRUE;
  MacroEntryInfoType info;
  while(GetMf()->GetMacroEntry(&info, first))
  {
    first = FALSE;
    if(info.m_code == ID_PUT_CHAR)
      OnChar(info.m_info, 0, 0);
    else
      Dispatcher(info.m_code);
  }
}

void WainView::Dispatcher(UINT code)
//   Description:
//     Most messages are dispatched thru this function.
//     It does some default stuff and calls the appropriate function
//   Parameters:
//     code: messages from FIRST_DISP_VAL to LAST_DISP_VAL
{
  if(code != IDV_EDIT_UNDO)
    m_undoList.AddEntry(UNDO_START_ENTRY, 0, 0);
  if(!m_currentTextLine)
    m_currentTextLine = GetDocument()->GetLineNo(m_lineNo);
  if(HandleSpecialModes(code) != CONTINUE_SPECIAL_MODE)
  {
    DispatcherTableType f = DispatcherTable[code - FIRST_DISP_VAL];
    if(f.f_ptr == NULL)
      WainMessageBox(this, "NULL pointer in dispatcher\r\nDid you forget to add a dispatcher entry?", IDC_MSG_OK, IDI_ERROR_ICO);
    else
    {
      (this->*f.f_ptr)();
      if(m_scrollLockOn)
      {
        int i;
        BOOL found;
        for(i = 0, found = FALSE; i < sizeof(ScrollLockMessages)/sizeof(ScrollLockMessages[0]) && !found; i++)
          if(code == ScrollLockMessages[i])
            found = TRUE;
        if(found)
        {
          WainView *other_view = GetMf()->GetOtherView(this);
          if(other_view)
            other_view->Dispatcher(code);
        }
      }
    }
  }

  if(code != IDV_MARK_WORD)
    m_markWordAction = NO_ACTION;
  if(GetFocus() == this)
    GetMf()->SetLineStatus(this, m_lineNo, m_columnNo);
  if(m_macroRecording)
    GetMf()->AddMacroEntry(code);
  m_lastAction = code;
  PutTextRest();
}

void WainView::OnSetFocus(CWnd *old_wnd)
//  Description:
//    Called whenever this view gets focus.
//    Sets the cursor.
//  Parameters:
{
   CView::OnSetFocus(old_wnd);
   MainFrame *mf = GetMf();
   if(!m_first)
     ScrollToVisible();
   CreateSolidCaret(m_insert ? 2 : m_charWidth, m_lineHeight - 1);

   mf->SetLineStatus(this, m_lineNo, m_columnNo);
   mf->SetMarkStatus(m_markType != NO_MARK);
   mf->SetInsertStatus(m_insert);
   SetSpecialModeStatus();
   mf->SetMacroRecStatus(m_macroRecording);
   mf->SetScrollLock_status(m_scrollLockOn);
   WainDoc *doc = GetDocument();
   if(doc)
   {
      mf->PutInTopOfStack(0, m_winNr);
      if(!doc->m_isDebugFile)
         mf->SetFileName(doc->GetPathName(), doc->m_propIndex);
   }
   mf->SetModifiedStatus(this, doc->GetModified());
   mf->SetCrLfStatus(doc->m_unixStyle);
   mf->SetReadOnlyStatus(doc->GetReadOnly());
   SetCursor();

   if(mf->IsChild(this))
   {
      mf->SetActiveView(this);
   }
}

void WainView::OnKillFocus(CWnd *aNewWnd)
//  Description:
//    Called whenever this view loses focus.
//    Removes the cursor
//  Parameters:
{
   RemoveCursor();
   RemoveMarkSticky();
   DestroyCaret();
   CView::OnKillFocus(aNewWnd);
}

void WainView::SetSpecialModeStatus(void)
{
   if(m_columnMarking)
      GetMf()->SetSpecialModeStatus("Column Mark");
   else if(m_incSearchOn)
      GetMf()->SetSpecialModeStatus("Inc Search");
   else
      GetMf()->SetSpecialModeStatus(NULL);
}

void WainView::ScrollToVisible(void)
//  Description:
//    Scrolls the view until the cursor is within the visible part of the window
//  Parameters:
//    sans
{
  RECT rect = m_clientRect;
  WainDoc *doc = GetDocument();
  int i;
  int old_pos = m_yOffset; //GetScrollPos(SB_VERT);
  int scroll_min, scroll_max;
  GetScrollRange(SB_VERT, &scroll_min, &scroll_max);
  rect.bottom -= 2*m_lineHeight;

  if(!m_lineNo)
  {
    if(m_yOffset)
    {
      BOOL co = m_cursorOn;
      if(co)
        RemoveCursor();
      SetScrollPos(SB_VERT, 0, TRUE);
      ScrollWindow(0, m_yOffset, NULL, NULL);
      m_yOffset = 0;
      if(co)
        SetCursor();
      UpdateTabViewPos();
    }
  }
  else if(m_lineNo*m_lineHeight < m_yOffset)
  {
    BOOL co = m_cursorOn;
    if(co)
      RemoveCursor();
    i = m_yOffset - m_lineNo*m_lineHeight;
    i += (m_lineHeight - 1);
    i /= m_lineHeight;
    if(!i)
      i = 1;
    int y = i;
    i *= m_lineHeight;
    m_yOffset -= i;
    SetScrollPos(SB_VERT, old_pos > i ? old_pos - i : 0, TRUE);
    CDC *dc = GetDC();
    RECT scroll_rect = m_clientRect;
    scroll_rect.top -= i;
    CRgn reg;
    dc->ScrollDC(0, i, &scroll_rect, &scroll_rect, &reg, NULL);
    scroll_rect = m_clientRect;
    scroll_rect.bottom = scroll_rect.top + i;
    dc->FillRect(&scroll_rect, &m_backBrush);
    if(i < m_clientRect.bottom)
    {
      TxtLine *p_line = m_currentTextLine;
      if(m_lineNo)
        p_line = doc->GetPrev(p_line);
      int yy;
      y++;
      for(yy = m_lineNo ? -1 : 0; y && p_line; yy++, y--)
      {
        PutText(dc, p_line, m_lineNo + yy);
        p_line = doc->GetNext(p_line);
      }
    }
    else
      ScrollWindow(0, old_pos > i ? i : old_pos, NULL, NULL);
    ReleaseDC(dc);
    UpdateTabViewPos();
    if(co)
      SetCursor();
  }
  else if((m_lineNo - 1)*m_lineHeight + m_lineHeight/2 > rect.bottom + m_yOffset)
  {
    BOOL co = m_cursorOn;
    if(co)
      RemoveCursor();
    i = (m_lineNo)*m_lineHeight - (rect.bottom + m_yOffset);
    i /= m_lineHeight;
    int y = i;
    i *= m_lineHeight;
    m_yOffset += i;
    ::SetScrollPos(m_hWnd, SB_VERT, old_pos + i > scroll_max ? scroll_max : old_pos + i, TRUE);

    CDC *dc = GetDC();
    RECT scroll_rect = m_clientRect;
    CRgn rgn;
    dc->ScrollDC(0, -i, &scroll_rect, &scroll_rect, &rgn, NULL);
    scroll_rect.top = scroll_rect.bottom - i;
    dc->FillRect(&scroll_rect, &m_backBrush);
    if(i < m_clientRect.bottom)
    {
      TxtLine *p_line = doc->GetRelLineNo(-y, m_currentTextLine);
      int yy = y + 2;
      while(yy && p_line)
      {
        PutText(dc, p_line, m_lineNo - y);
        p_line = doc->GetNext(p_line);
        y--;
        yy--;
      }
    }
    else
      ScrollWindow(0, old_pos + i > scroll_max ? -(scroll_max - old_pos) : -i, NULL, NULL);
    ReleaseDC(dc);
    UpdateTabViewPos();
    if(co)
      SetCursor();
  }
  int w = m_clientRect.right/m_charWidth - 2;

  if(m_columnNo - m_xOffset/m_charWidth > w)
  {
    i = m_columnNo - m_xOffset/m_charWidth - w;
    #if 0
    i = (10 + i)/10;
    i *= 10;
    #endif
    m_xOffset += i*m_charWidth;
    ::SetScrollPos(m_hWnd, SB_HORZ, m_xOffset, TRUE);
    UpdateAll();
  }
  #if 0
  else if(XOffset && CurrentColumn - XOffset/CharWidth < w - n)
  {
    i = CurrentColumn - XOffset/CharWidth - (w - n);
    i = (10 - i)/10;
    i *= 10;
    XOffset -= i*CharWidth;
    if(XOffset < 0)
      XOffset = 0;
    ::SetScrollPos(m_hWnd, SB_HORZ, XOffset, TRUE);
    UpdateAll();
  }
  #else
  else if(m_xOffset && m_columnNo - m_xOffset/m_charWidth < 0)
  {
    i = m_columnNo - m_xOffset/m_charWidth;
    m_xOffset += i*m_charWidth;
    if(m_xOffset < 0)
      m_xOffset = 0;
    ::SetScrollPos(m_hWnd, SB_HORZ, m_xOffset, TRUE);
    UpdateAll();
  }

  #endif
}

void WainView::OnHScroll(UINT sb_code, UINT pos, CScrollBar *scroll_bar)
//  Description:
//    Called when the user does stuff with the horizontal scrollbar.
//    Removes the cursor until endscroll.
//    The works is Done by CView
//  Parameters:
//    Whatever Windows dictates
{
  m_markWordAction = NO_ACTION;
  if(sb_code == SB_ENDSCROLL)
  {
    CView::OnHScroll(sb_code, pos, scroll_bar);
    SetCursorPos();
    SetCursor();
  }
  else
  {
    RemoveCursor();
    RemoveMarkSticky();
    if(sb_code == SB_THUMBTRACK)
    { // Get the position as a 32 bit int
      SCROLLINFO si;
      if(GetScrollInfo(SB_HORZ, &si, SIF_TRACKPOS))
        pos = si.nTrackPos;
    }
    DoScroll(sb_code, -1, pos);
  }
}

void WainView::OnVScroll(UINT sb_code, UINT pos, CScrollBar *scroll_bar)
//  Description:
//    Called when the user does stuff with the vertical scrollbar.
//    Removes the cursor until endscroll.
//    The works is Done by CView
//  Parameters:
{
   m_markWordAction = NO_ACTION;
   if(sb_code == SB_ENDSCROLL)
   {
      CView::OnVScroll(sb_code, pos, scroll_bar);
      SetCursorPos();
      SetCursor();
      if(GetFocus() == this)
      {
         GetMf()->SetLineStatus(this, m_lineNo, m_columnNo);
      }
      UpdateTabViewPos();
   }
   else
   {
      RemoveCursor();
      if(sb_code == SB_THUMBTRACK)
      { // Get the position as a 32 bit int
         SCROLLINFO si;
         if(GetScrollInfo(SB_VERT, &si, SIF_TRACKPOS))
         {
            pos = si.nTrackPos;
         }
         if(m_lineHeight)
         {
            pos /= m_lineHeight;
            pos *= m_lineHeight;
         }
      }
      DoScroll(-1, sb_code, pos);
      UpdateTabViewPos();
   }
}

void WainView::SetCursorPos(void)
//  Description:
//    Sets the cursor (ie. m_lineNo) somewhere within the visible part of the window
//    This is Done by change position as little as posible
//  Parameters:
//
{
  RECT rect = m_clientRect;
  int old_m_lineNo = m_lineNo;
  if(m_lineNo*m_lineHeight < m_yOffset)
  {
    m_lineNo = (m_yOffset/m_lineHeight) + 1;
  }
  if((m_lineNo + 1)*m_lineHeight - m_yOffset > rect.bottom)
  {
    m_lineNo = ((rect.bottom + m_yOffset) /m_lineHeight) - 1;
  }
  WainDoc *doc = GetDocument();
  if(m_lineNo != old_m_lineNo)
    m_currentTextLine = doc->GetLineNo(m_lineNo);
  while(!m_currentTextLine)
    m_currentTextLine = doc->GetLineNo(--m_lineNo);
}

void WainView::SaveFile(void)
{
  WainDoc *doc = GetDocument();
  ASSERT(doc);
  if(doc->m_isDebugFile)
    return;
  doc->SaveFile();
  if(GetFocus() == this)
    GetMf()->FileHasBeenSaved();
}

void WainView::SaveFileAs(void)
{
  WainDoc *doc = GetDocument();
  ASSERT(doc);
  if(doc->m_isDebugFile)
    return;
  doc->SaveFileAs();
  if(GetFocus() == this)
    GetMf()->FileHasBeenSaved();
}

void WainView::HandlePreActions(unsigned int action)
//  Description:
//    Called from a lot of message handlers before the do anything.
//  Parameters:
//    action: A combination of any of the PA_xxx
{
  MainFrame *mf = GetMf();

  if(m_columnNo > m_maxLineLen)
    SetHorzSize(m_columnNo);
  if(action & PA_SCROLL_TO_VISIBLE)
  { /* Must be Done first */
    ScrollToVisible();
  }
  if(action & PA_UPDATE_CURSOR_POS)
  {
    if(m_cursorOn)
    {
      POINT p = {X_OFF - 1 - m_xOffset + (m_columnNo*m_charWidth), (m_lineNo*m_lineHeight) - m_yOffset + 1};
      SetCaretPos(p);
    }
    GetMf()->SetLineStatus(this, m_lineNo, m_columnNo);
  }
  if(action & PA_REMOVE_CURSOR)
  {
    RemoveCursor();
    MarkBraces(false);
  }
  if (action & (PA_SET_CURSOR | PA_UPDATE_CURSOR_POS))
  {
     if (m_cursorOn)
     {
       RemoveCursor();
       action |= PA_SET_CURSOR;
     }
     MarkBraces(true);
  }
  if(action & PA_SET_CURSOR)
  {
    SetCursor();
  }
  if(action & PA_SET_TAG && !GetDocument()->m_isDebugFile)
  {
    GetCurrentWord(wainApp.m_tagStr);
  }
  if(action & PA_REMOVE_MARK)
  {
    if(m_markType != NO_MARK)
    {
      RemoveMark();
      m_markType = NO_MARK;
      mf->SetMarkStatus(FALSE);
    }
  }
  else if(action & PA_REMOVE_MARK_STICKY)
  {
    if(m_markType != NO_MARK)
    {
      RemoveMarkSticky();
    }
  }
}

void WainView:: HandlePostActions(unsigned int action)
{
  HandlePreActions(action);
}

#define GOTO_DOC(x) \
void WainView::GotoDoc##x(void) \
{                                        \
  GetMf()->ActivateWin(x);             \
}

GOTO_DOC(0)
GOTO_DOC(1)
GOTO_DOC(2)
GOTO_DOC(3)
GOTO_DOC(4)
GOTO_DOC(5)
GOTO_DOC(6)
GOTO_DOC(7)
GOTO_DOC(8)
GOTO_DOC(9)

#undef GOTO_DOC

void WainView::GotoLineNo(int line, int column)
{
   if(line > GetDocument()->m_lineCount -1 || line < 0)
      line = 0;
   if(!m_first)
   {
      HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);
      WainDoc *doc = GetDocument();

      ASSERT(line < doc->m_lineCount);

      m_undoList.AddEntry(UNDO_MOVE_ENTRY, column - m_columnNo, line - m_lineNo);
      m_lineNo = line;
      m_columnNo = column;

      m_currentTextLine = doc->GetLineNo(m_lineNo);
      HandlePostActions(PA_SET_CURSOR | PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SCROLL_TO_VISIBLE);
  }
  else
  {
     m_lineNo = line;
     m_columnNo = column;
     RECT cr;
     GetClientRect(&cr);
     int lc = (cr.bottom - cr.top)/(m_lineHeight * 2);
     if(line > lc)
     {
        SetScrollPos(SB_VERT, (line - lc)*m_lineHeight, FALSE);
        m_yOffset = (line - lc)*m_lineHeight;
     }
     else
     {
        SetScrollPos(SB_VERT, line*m_lineHeight, FALSE);
        m_yOffset = line*m_lineHeight;
     }
  }
  GetMf()->SetLineStatus(this, m_lineNo, m_columnNo);
}

void WainView::GotoLineNoRaw(int line, int column)
{
  if(line > GetDocument()->m_lineCount -1 || line < 0)
    return;
  if(!m_first)
  {
    HandlePreActions(PA_REMOVE_CURSOR | PA_REMOVE_MARK_STICKY);
    WainDoc *doc = GetDocument();
    m_undoList.AddEntry(UNDO_MOVE_ENTRY, column - m_columnNo, line - m_lineNo);
    m_lineNo = line;
    m_columnNo = column;

    m_currentTextLine = doc->GetLineNo(m_lineNo);
    HandlePostActions(PA_SCROLL_TO_VISIBLE);
  }
  else
  {
    m_lineNo = line;
    SetScrollPos(SB_VERT, line*m_lineHeight, FALSE);
    m_yOffset = line*m_lineHeight;
  }
}

void WainView::DoRemoveMark(void)
{
  HandlePreActions(PA_REMOVE_CURSOR);
  RemoveMark();
  GetMf()->SetMarkStatus(FALSE);
  HandlePreActions(PA_SET_CURSOR);
}

void WainView::DrawMarkLines(int start_y, int end_y)
//  Description:
//    Draw the visible lines from start_y to end_y
//  Parameters:
//    start_y: The first line to be drawen.
//    end_y:   The last line to be drawn.
{
  CDC *dc = GetDC();
  ASSERT(dc);
  WainDoc *doc = GetDocument();
  RECT cr;
  GetClientRect(&cr);
  int y;
  if(m_yOffset/m_lineHeight > start_y)
    start_y = m_yOffset/m_lineHeight;
  if(end_y > (m_yOffset + cr.bottom)/m_lineHeight)
    end_y = (m_yOffset + cr.bottom)/m_lineHeight;
  TxtLine *p_line = doc->GetLineNo(start_y);
  for(y = start_y; y <= end_y && p_line; y++)
  {
    PutText(dc, p_line, y);
    p_line = doc->GetNext(p_line);
  }
  ReleaseDC(dc);
}

void WainView::MdiMax(void)
//  Description:
//    Maximize this window, Windows will then maximize any other window
//  Parameters:
{
   WainDoc *doc = GetDocument();
   if(doc->m_childFrame && !doc->m_isDebugFile)
   {
      wainApp.gs.m_createMode = CreateModeType::CREATE_MAX;
      // doc->m_childFrame->ShowWindow(SW_SHOWMAXIMIZED);
      GetMf()->MDIMaximize(doc->m_childFrame);
      GetMf()->SetScrollLock();
      GetMf()->SetScrollLock_status(FALSE);
   }
}

void WainView::MdiCascade(void)
//  Description:
//    Cascades the windows.
//  Parameters:
{
  if(!GetDocument()->m_isDebugFile)
    GetMf()->MDICascade();
}

void WainView::MdiDual(void)
//  Description:
//    Arange this window so it will fill up half of the screen
//  Parameters:
{
   if(!GetDocument()->m_isDebugFile)
      GetMf()->RecalcChildPos(true);
}

void WainView::SetCursor(void)
//  Description:
//    Display the caret and the focus rect.
//  Parameters:
{
   if(GetFocus() != this || m_mouseDown)
      return;
   RemoveCursor();
   m_cursorOn = TRUE;
   if(wainApp.gs.m_frameCurrLine)
   {
      CDC *dc = GetDC();
      m_focusRect.left = 1;
      m_focusRect.top = m_lineNo*m_lineHeight - m_yOffset + 1;
      m_focusRect.right = m_clientRect.right - 1;
      m_focusRect.bottom = (m_lineNo + 1)*m_lineHeight - m_yOffset + 1;
      dc->DrawFocusRect(&m_focusRect);
      ReleaseDC(dc);
   }
   POINT cp = {m_columnNo*m_charWidth + X_OFF - 1 - m_xOffset, m_lineNo*m_lineHeight - m_yOffset + 1};
   SetCaretPos(cp);
   ShowCaret();
}

void WainView::RemoveCursor(void)
//  Description:
//    Removes the caret and the focus rect.
//  Parameters:
{
   if(m_cursorOn)
   {
      m_cursorOn = FALSE;
      HideCaret();
      CDC *dc = GetDC();
      if(wainApp.gs.m_frameCurrLine)
        dc->DrawFocusRect(&m_focusRect);
      ReleaseDC(dc);
   }
}

void WainView::MarkBraces(bool _on)
{
   if (!m_currentTextLine)
      return;
   if (_on)
   {
      if (m_matchStartLine)
      {
         MarkBraces(false);
      }
      WainDoc *doc = GetDocument();
      TxtLine *l;
      int x = m_columnNo;
      int y = m_lineNo;
      int findOff = 0;
      l = doc->FindMatchBrace(m_currentTextLine, &x, &y, true, &findOff, &m_matchStartLength, &m_matchEndLength);
      if (l)
      {
         m_matchStartLine = m_currentTextLine;
         m_matchStartLineNo = m_lineNo;
         m_matchStartColumn = m_columnNo - findOff;
         m_matchEndLine = l;
         m_matchEndLineNo = y;
         m_matchEndColumn = x;
         PutText(0, m_matchStartLine, m_matchStartLineNo);
         if (m_matchEndLine != m_matchStartLine)
         {
            PutText(0, m_matchEndLine, m_matchEndLineNo);
         }
         /* Hello */
      }
      else
      {
         TxtLine* tMatchStartLine = m_matchStartLine;
         int tMatchStartLineNo = m_matchStartLineNo;
         // int tMatchStartColumn = m_matchStartColumn;
         TxtLine* tMatchEndLine = m_matchEndLine;
         int tMatchEndLineNo = m_matchEndLineNo;
         // int tMatchEndColumn = m_matchEndColumn;
         m_matchStartLine = 0;
         m_matchStartLineNo = 0;
         m_matchStartColumn = 0;
         m_matchEndLine = 0;
         m_matchEndLineNo = 0;
         m_matchEndColumn = 0;

         if (tMatchStartLine)
         {
            PutText(0, tMatchStartLine, tMatchStartLineNo);
            if (tMatchEndLine != tMatchStartLine)
            {
               PutText(0, tMatchEndLine, tMatchEndLineNo);
            }
         }
      }
   }
   else
   {
      if (m_matchStartLine)
      {
         TxtLine* tMatchStartLine = m_matchStartLine;
         int tMatchStartLineNo = m_matchStartLineNo;
         // int tMatchStartColumn = m_matchStartColumn;
         TxtLine* tMatchEndLine = m_matchEndLine;
         int tMatchEndLineNo = m_matchEndLineNo;
         // int tMatchEndColumn = m_matchEndColumn;
         m_matchStartLine = 0;
         m_matchStartLineNo = 0;
         m_matchStartColumn = 0;
         m_matchEndLine = 0;
         m_matchEndLineNo = 0;
         m_matchEndColumn = 0;

         if (tMatchStartLine)
         {
            PutText(0, tMatchStartLine, tMatchStartLineNo);
            if (tMatchEndLine != tMatchStartLine)
            {
                PutText(0, tMatchEndLine, tMatchEndLineNo);
            }
         }
      }
   }
}

void WainView::DoReloadFile(void)
{
   if(GetDocument()->GetModified())
   {
      std::string msg = "The file:\r\n";
      msg += GetDocument()->GetPathName();
      msg += "\r\nHas been modified, are you shure you want to reload it?";
      if(WainMessageBox(this, msg.c_str(), IDC_MSG_YES | IDC_MSG_NO, IDI_QUESTION_ICO) == IDC_MSG_NO)
         return;
      RemoveMarkSticky();
   }
   GetMf()->ReenableFileCheck(this, GetDocument()->m_isFtpFile);
   ReloadFile();
}

void WainView::ReloadFile(void)
//  Description:
//    Reload the file.
//  Parameters:
{
   HandlePreActions(PA_REMOVE_CURSOR);
   GetDocument()->ReloadFile();
   if(m_lineNo >= GetDocument()->m_lineCount - 1)
   {
      m_columnNo = 0;
      m_lineNo = GetDocument()->m_lineCount - 1;
   }
   m_currentTextLine = GetDocument()->GetLineNo(m_lineNo);
   RemoveMark();
   m_undoList.Clear();
   InvalidateRect(NULL, TRUE);
   SetScrollInfo();
   if(GetFocus() == this)
      HandlePostActions(PA_SCROLL_TO_VISIBLE | PA_UPDATE_CURSOR_POS | PA_SET_TAG | PA_SET_CURSOR);
   else
      HandlePostActions(PA_SCROLL_TO_VISIBLE | PA_UPDATE_CURSOR_POS | PA_SET_TAG);
}

void WainView::GetCurrentWord(std::string &aWord)
//  Description:
//    Gets the word under the cursor.
//  Parameters:
//    word: pointer to a string where the text will be put.
{
  WainDoc *doc = GetDocument();
  if(!doc->GetCurrentWord(aWord, m_columnNo, m_currentTextLine))
    aWord = "";
}

void WainView::OnSaveDoc(void)
//  Description:
//    Called when the document is saved.
//    Clear the Undo list.
//  Parameters:
{
  if(wainApp.gs.m_flushUndoOnSave)
    m_undoList.Clear();
  RemoveMarkSticky();
}

void WainView::PutTextRest(void)
//  Description:
//    Called from Dispatcher(), to draw any undrawn text which color has changed.
//  Parameters:
{
  if(m_lastPutTextLine)
  {
    HandlePreActions(PA_REMOVE_CURSOR);
    WainDoc *doc = GetDocument();
    RECT cr = m_clientRect;
    int end_y = m_lastPutTextLineNo + cr.bottom/m_lineHeight;

    if(end_y > (m_yOffset + cr.bottom)/m_lineHeight)
      end_y = (m_yOffset + cr.bottom)/m_lineHeight;
    int LineNo = m_lastPutTextLineNo;
    TxtLine *l = m_lastPutTextLine;
    if (!l)
    {
       l = doc->GetLineNo(LineNo);
    }
    while((l = doc->GetNext(l)) != NULL && LineNo <= end_y)
    {
      LineNo++;
      PutText(NULL, l, LineNo);
    }
    m_lastPutTextLine = NULL;
    // TODO HandlePreActions(PA_SET_CURSOR);
  }
}

void WainView::CheckRedraw(void)
//  Description:
//    Checks wether the text after m_lineNo is to be drawn if m_currentTextLine has changed.
//    PutTextRest() does the drawing.
//  Parameters:
{
  EndLineStatusType els = GetDocument()->GetEndLineStatus(m_currentTextLine);
  if(els != m_currentTextLine->m_endLineStatus)
  { // Put text will do the rest
    PutText(NULL, m_currentTextLine, m_lineNo);
  }
}

void WainView::UpdateAll(void)
{
  CDC *dc = GetDC();
  WainDoc *doc = GetDocument();
  BOOL co = m_cursorOn;
  if(co)
    HandlePreActions(PA_REMOVE_CURSOR);

  int first_line = m_yOffset/m_lineHeight;
  if(first_line)
    first_line--;

  RECT br;
  GetClientRect(&br);
  int last = (m_yOffset + br.bottom)/m_lineHeight + 1;

  /* First erase the left margin */
  br.right = X_OFF;
  dc->FillRect(&br, &m_backBrush);

  TxtLine *p_line;
  int i;
  for(p_line = doc->GetLineNo(first_line), i = first_line; i < last && p_line; i++, p_line = doc->GetNext(p_line))
    PutText(dc, p_line, i);
  GetClientRect(&br);
  for(; i <= last; i++)
  {
    br.top = i*m_lineHeight - m_yOffset;
    br.bottom = br.top + m_lineHeight;
    dc->FillRect(&br, &m_backBrush);
  }

  if(co)
    HandlePostActions(PA_SET_CURSOR);
  ReleaseDC(dc);
  UpdateTabViewPos();
}

BOOL WainView::OnEraseBkgnd(CDC *dc)
{
  return TRUE;
}

void WainView::SetDocument(WainDoc *doc)
{
  ASSERT(doc);
  m_pDocument = (CDocument *)doc;
}

BOOL WainView::OnSetCursor(CWnd *wnd, UINT hit_test, UINT message)
{
  if(hit_test == HTHSCROLL || hit_test == HTVSCROLL)
    return CView::OnSetCursor(wnd, hit_test, message);
  // SetCursor(LoadCursor(NULL, IDC_IBEAM));
  ::SetCursor(wainApp.m_viewCursor);
  return TRUE;
}

void WainView::ViewPopupMenu(void)
{
  POINT p;
  GetPopupPos(&p, wainApp.gs.m_popupMenu.size());
  DoViewPopupMenu(&p);
}

void WainView::DoViewPopupMenu(POINT *p)
{
   CMenu bar;
   bar.LoadMenu(IDD_POPUP_MENU);
   CMenu &popup = *bar.GetSubMenu(0);
   ASSERT(popup.m_hMenu != NULL);

   int pos;
   for(pos = popup.GetMenuItemCount() - 1; pos >= 0; pos--)
   {
      popup.DeleteMenu(pos, MF_BYPOSITION);
   }
   size_t i;
   for(i = 0; i < wainApp.gs.m_popupMenu.size(); i++)
   {
      if(wainApp.gs.m_popupMenu[i].m_id == 0)
         popup.AppendMenu(MF_SEPARATOR);
      else
         popup.AppendMenu(MF_STRING, wainApp.gs.m_popupMenu[i].m_id, wainApp.gs.m_popupMenu[i].m_text.c_str());
   }

   popup.TrackPopupMenu(TPM_LEFTALIGN, p->x, p->y, GetMf());
}

void WainView::UpdateTabViewPos(void)
{
  if(m_tabView)
    m_tabView->InvalidateRect(NULL, TRUE);
}

void WainView::SetHorzSize(int size)
{
   if(size > m_maxLineLen)
   {
      m_maxLineLen = size;
      SetScrollInfo();
   }
}

BOOL WainView::DoScroll(int h_code, int v_code, UINT pos)
{
  // calc new x position
  int x = m_xOffset;
  int xOrig = x;

  switch(h_code)
  {
    case SB_TOP:
      x = 0;
      break;
    case SB_BOTTOM:
      x = INT_MAX;
      break;
    case SB_LINEUP:
      x -= m_lineScroll.cx;
      break;
    case SB_LINEDOWN:
      x += m_lineScroll.cx;
      break;
    case SB_PAGEUP:
      x -= m_pageScroll.cx;
      break;
    case SB_PAGEDOWN:
      x += m_pageScroll.cx;
      break;
    case SB_THUMBTRACK:
      x = pos;
      break;
    }

  // calc new y position
  int y = m_yOffset;
  int yOrig = y;

  switch(v_code)
  {
  case SB_TOP:
    y = 0;
    break;
  case SB_BOTTOM:
    y = INT_MAX;
    break;
  case SB_LINEUP:
    y -= m_lineScroll.cy;
    break;
  case SB_LINEDOWN:
    y += m_lineScroll.cy;
    break;
  case SB_PAGEUP:
    y -= m_pageScroll.cy;
    break;
  case SB_PAGEDOWN:
    y += m_pageScroll.cy;
    break;
  case SB_THUMBTRACK:
    y = pos;
    break;
  }

  BOOL bResult = OnScrollBy(CSize(x - xOrig, y - yOrig), TRUE);
  if (bResult)
    UpdateWindow();

  return bResult;
}

BOOL WainView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
  int xOrig, x;
  int yOrig, y;

  // don't scroll if there is no valid scroll range (ie. no scroll bar)
  CScrollBar *pBar;
  DWORD dwStyle = GetStyle();
  pBar = GetScrollBarCtrl(SB_VERT);
  if ((pBar != NULL && !pBar->IsWindowEnabled()) || (pBar == NULL && !(dwStyle & WS_VSCROLL)))
  {
    // vertical scroll bar not Enabled
    sizeScroll.cy = 0;
  }
  pBar = GetScrollBarCtrl(SB_HORZ);
  if ((pBar != NULL && !pBar->IsWindowEnabled()) || (pBar == NULL && !(dwStyle & WS_HSCROLL)))
  {
    // horizontal scroll bar not Enabled
    sizeScroll.cx = 0;
  }

  // adjust current x position
  // xOrig = x = GetScrollPos(SB_HORZ);
  xOrig = x = m_xOffset;
  int xMax = m_maxScroll.cx; //GetScrollLimit(SB_HORZ);
  x += sizeScroll.cx;
  if (x < 0)
    x = 0;
  else if (x > xMax)
    x = xMax;

  // adjust current y position
  // yOrig = y = GetScrollPos(SB_VERT);
  yOrig = y = m_yOffset;
  int yMax = m_maxScroll.cy; // GetScrollLimit(SB_VERT);
  y += sizeScroll.cy;
  if (y < 0)
    y = 0;
  else if (y > yMax)
    y = yMax;

  // did anything change?
  if (x == xOrig && y == yOrig)
    return FALSE;

  if (bDoScroll)
  {
    // do scroll and Update scroll positions
    ScrollWindow(-(x-xOrig), -(y-yOrig));
    if (x != xOrig)
      SetScrollPos(SB_HORZ, x);
    if (y != yOrig)
      SetScrollPos(SB_VERT, y);
  }
  m_xOffset = x;
  m_yOffset = y;
  return TRUE;
}

void WainView::Print(void)
{
  m_firstPage = true;
  CView::OnFilePrint();
}

void WainView::SetFontMetrics(CDC &aDc)
{
   TEXTMETRIC tm;
   aDc.GetOutputTextMetrics(&tm);

   if (tm.tmHeight < 0)
     m_lineHeight = -tm.tmHeight;
   else
     m_lineHeight = tm.tmHeight;
   m_lineHeight += 3;

   m_textOffset = 3;

   m_charWidth = tm.tmAveCharWidth < 0 ? -tm.tmAveCharWidth : tm.tmAveCharWidth;
}

bool WainView::IsMatchPos(const class TxtLine* line, int xPos)
{
   return (line == m_matchStartLine && xPos >= m_matchStartColumn && xPos < m_matchStartColumn + m_matchStartLength) ||
          (line == m_matchEndLine && xPos >= m_matchEndColumn && xPos < m_matchEndColumn + m_matchEndLength);
}

void WainView::RemoveLine(TxtLine* line)
{
   if (line == m_matchStartLine || line == m_matchEndLine)
   {
      m_matchStartLine = 0;
      m_matchStartLineNo = 0;
      m_matchStartColumn = 0;
      m_matchEndLine = 0;
      m_matchEndLineNo = 0;
      m_matchEndColumn = 0;
   }
}

#ifdef _DEBUG
void WainView::AssertValid() const
{
  CView::AssertValid();
}

void WainView::Dump(CDumpContext &dc) const
{
  CView::Dump(dc);
}

WainDoc *WainView::GetDocument() // non-debug version is inline
{
  ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(WainDoc)));
  return (WainDoc *)m_pDocument;
}
#endif //_DEBUG


#ifdef __BORLANDC__
#ifdef _AFX_NO_DEBUG_CRT
LONG afxAssertBusy = -1;
LONG afxAssertReallyBusy = -1;
BOOL (AFXAPI* afxAssertFailedLine)(LPCSTR, int);
#endif

BOOL AFXAPI AfxAssertFailedLine(LPCSTR lpszFileName, int nLine)
{
#ifndef _AFX_NO_DEBUG_CRT
  // we remove WM_QUIT because if it is in the queue then the message box
  // won't display
  MSG msg;
  BOOL bQuit = PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
  BOOL bResult = _CrtDbgReport(_CRT_ASSERT, lpszFileName, nLine, NULL, NULL);
  if (bQuit)
    PostQuitMessage(msg.wParam);
  return bResult;
#else
  TCHAR szMessage[_MAX_PATH*2];

  // handle the (hopefully rare) case of AfxGetAllocState ASSERT
  if (InterlockedIncrement(&afxAssertReallyBusy) > 0)
  {
    // assume the debugger or auxiliary port
    wsprintf(szMessage, _T("Assertion Failed: File %hs, Line %d\n"), lpszFileName, nLine);
    OutputDebugString(szMessage);
    InterlockedDecrement(&afxAssertReallyBusy);

    // assert w/in assert (examine call stack to determine first one)
    DebugBreak();
    return FALSE;
  }

  // check for special hook function (for testing diagnostics)
  AfxGetThreadState();
  InterlockedDecrement(&afxAssertReallyBusy);
  if (afxAssertFailedLine != NULL)
    return (*afxAssertFailedLine)(lpszFileName, nLine);

  // get app name or NULL if unknown (don't call assert)
  LPCTSTR lpszAppName = afxCurrentAppName;
  if (lpszAppName == NULL)
    lpszAppName = _T("<unknown application>");

  // format message into buffer
  wsprintf(szMessage, _T("%s: File %hs, Line %d"), lpszAppName, lpszFileName, nLine);

  if (InterlockedIncrement(&afxAssertBusy) > 0)
  {
    InterlockedDecrement(&afxAssertBusy);
    // assert within assert (examine call stack to determine first one)
    DebugBreak();
    return FALSE;
  }

  // active popup window for the current thread
  HWND hWndParent = GetActiveWindow();
  if (hWndParent != NULL)
    hWndParent = GetLastActivePopup(hWndParent);

  // we remove WM_QUIT because if it is in the queue then the message box
  // won't display
  MSG msg;
  BOOL bQuit = ::PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
  // display the assert
  int nCode = ::MessageBox(hWndParent, szMessage, _T("Assertion Failed!"), MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SETFOREGROUND);
  if (bQuit)
    PostQuitMessage(msg.wParam);

  // cleanup
  InterlockedDecrement(&afxAssertBusy);

  if (nCode == IDIGNORE)
    return FALSE;   // ignore

  if (nCode == IDRETRY)
  {
    DebugBreak();
    return TRUE;    // will cause AfxDebugBreak
  }

  AfxAbort();     // should not return (but otherwise AfxDebugBreak)
  return TRUE;
#endif // _AFX_NO_DEBUG_CRT
}
#endif
