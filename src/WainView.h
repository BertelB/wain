//=============================================================================
// This source code file is a part of Wain.
// It defines , UndoList_class, Undo_entry_class, and SimpleDialog.
//=============================================================================
#ifndef WAINVIEW_H_INC
#define WAINVIEW_H_INC
#include "dialogbase.h"
#include "Resource.h"

#define PA_UPDATE_CURSOR_POS  0x0001
#define PA_REMOVE_CURSOR      0x0002
#define PA_SET_CURSOR         0x0004
#define PA_SCROLL_TO_VISIBLE  0x0008
#define PA_SET_TAG            0x0010
#define PA_REMOVE_MARK        0x0020
#define PA_REMOVE_MARK_STICKY 0x0040

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(a)    ((a) < 0 ? (-(a)) : (a))

#define TL_MAX_NOF_TAGS 20

class TxtLine;

struct LineColorEntryType
{
   LineColorEntryType(int aLen, ColorIndexType aIndex) : m_len(aLen), m_index(aIndex)
   {}
   int            m_len;
   ColorIndexType m_index;
};

class TxtLineColor
{
public:
   TxtLineColor() : m_bookMark(0)
   {
   }
   const LineColorEntryType &operator [] (size_t aIdx) const { return m_lineColorEntry[aIdx]; }
   void AddEntry(int aLen, ColorIndexType aIndex)
   {
      m_sumLen += aLen;
      if(!m_lineColorEntry.empty() &&
         m_lineColorIndex > 0 &&
         m_lineColorEntry[m_lineColorIndex - 1].m_index == aIndex &&
         m_lineColorEntry[m_lineColorIndex - 1].m_len + aLen < 4096 &&
         aIndex != TAB_IDX)
      {
         m_lineColorEntry[m_lineColorIndex - 1].m_len += aLen;
      }
      else if(m_lineColorIndex < m_lineColorEntry.size())
      {
         m_lineColorEntry[m_lineColorIndex].m_len = aLen;
         m_lineColorEntry[m_lineColorIndex].m_index = aIndex;
         m_lineColorIndex++;
      }
      else
      {
         m_lineColorEntry.push_back(LineColorEntryType(aLen, aIndex));
         m_lineColorIndex++;
      }
   }
   void Reset()
   {
      m_lineColorIndex = 0;
      m_sumLen = 0;
      if(!m_lineColorEntry.empty())
      {
         m_lineColorEntry[0].m_len = 0;
      }
      m_bookMark = 0;
   }
   size_t GetSumLen() const { return m_sumLen; }
   size_t GetNumColor() const { return m_lineColorEntry.size(); }
   void StoreBookMark()
   {
      m_bookMark = m_lineColorIndex;
      m_boolMarkSum = m_sumLen;
   }
   void ResetToBookMark()
   {
      m_lineColorIndex = m_bookMark;
      m_sumLen = m_boolMarkSum;
   }
private:
   size_t m_bookMark;
   size_t m_lineColorIndex;
   size_t m_sumLen;
   size_t m_boolMarkSum;
   std::vector<LineColorEntryType > m_lineColorEntry;
};

enum MarkTypeType
{
   NO_MARK,
   COLUMN_MARK,
   CUA_MARK
};

enum UndoEntryTypeType
{
   UNDO_MOVE_ENTRY,
   UNDO_CHAR_ENTRY,
   UNDO_DELETE_ENTRY,
   UNDO_RETURN_ENTRY,
   UNDO_INSERT_TOGGLE_ENTRY,
   UNDO_MARK_ENTRY,
   UNDO_CUT_ENTRY,
   UNDO_COPY_CLIP_ENTRY,
   UNDO_COPY_CLIP_OLD_DATA_ENTRY,
   UNDO_INSERT_ENTRY,
   UNDO_CUT_TEXT_ENTRY,
   UNDO_INSERT_LINE_ENTRY,
   UNDO_TOGGLE_COLUMN_MARK_ENTRY,
   UNDO_START_ENTRY,
   UNDO_SHIFT_BLOCK_ENTRY,
   UNDO_REMOVE_LINE_ENTRY
};

enum MarkWordActionType
{
   NO_ACTION,
   MARK_WORD_WORD,
   MARK_WORD_LINE,
   MARK_WORD_PARAGRAPH
};

struct UndoMarkEntryType
{
   int m_sX;
   int m_eX;
   int m_sY;
   int m_eY;
   MarkTypeType m_type;
};

struct UndoCopyClipOldDataEntryType
{
  char *m_buffer;
  int m_sX;
  int m_eX;
  int m_sY;
  int m_eY;
  MarkTypeType m_type;
};
typedef UndoCopyClipOldDataEntryType UndoCopyClipEntryType;
typedef UndoCopyClipOldDataEntryType UndoCutEntryType;

struct UndoInsertEntryType
{
   char *m_text;
   int  m_x;
   int  m_y;
};

typedef UndoInsertEntryType UndoCutTextEntryType;
typedef UndoInsertEntryType UndoRemoveLineEntryType;

class UndoEntryClass
{
   friend class UndoListClass;
   friend class WainView;
   unsigned int m_checkMark;
   UndoEntryClass *m_next;
   UndoEntryClass *m_prev;
public:
   UndoEntryTypeType m_type;
   int m_x;
   int m_y;
   int  m_i;
   void *m_p;
   UndoEntryClass();
   ~UndoEntryClass();
};

class UndoListClass
{
public:
   class WainView *m_view; // A nasty pointer to the view who owns me
   int m_nofEntries;
   UndoEntryClass m_list;
   UndoEntryClass m_redoList;
   UndoListClass();
   ~UndoListClass();
   void Clear(void);
   void ClearEntry(UndoEntryClass *e);
   UndoEntryClass *AddEntry(UndoEntryTypeType type, int x, int y, int add_i = 0, void *add_p = NULL);
   UndoEntryClass *AddInsertEntry(int mx, int my, int xp, int yp, const char *txt);
};

class WainView : public CView
{
  DECLARE_DYNCREATE(WainView)
  friend class UndoListClass;

private:
  void SetFontMetrics(CDC &aDc);
  bool m_firstPage;
  TxtLine* m_matchStartLine;
  int m_matchStartLineNo;
  int m_matchStartColumn;
  int m_matchStartLength;
  TxtLine* m_matchEndLine;
  int m_matchEndLineNo;
  int m_matchEndColumn;
  int m_matchEndLength;
  SIZE m_lineScroll;
  SIZE m_pageScroll;
  SIZE m_maxScroll;
  BOOL DoScroll(int h_code, int v_code, UINT pos);
  BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll);
  void TagListXxx(unsigned int flags);
  BOOL m_vertScrollOn;
  BOOL m_inSetScrollInfo;
  BOOL m_wasVScrollOn;
  BOOL m_wasHScrollOn;
  BOOL CheckFileName(CString &Dest, const CString &name, int tool_no, const CString &make_file);
  MarkWordActionType m_markWordAction;
  BOOL m_specialClip;
  BOOL m_macroRecording;
  BOOL m_columnMarking;
  BOOL m_incSearchOn;
  int m_orgIncSearchY;
  int m_orgIncSearchX;
  unsigned int m_searchFlags;
  CString m_searchString;
  void CheckRedraw(void);
  UndoListClass m_undoList;
  BOOL m_addUndo;
  void PutText(CDC *dc, TxtLine *line, int y);
  int PrintText(CDC *dc, TxtLine *line, int y, int CharWidthidth, int LineHeighteight, int cl, int max_line, int left_margin);
  void PrintHeader(CDC *dc, int CharWidthidth, int LineHeighteight, int page_no, int nof_pages, int left_margin, const char *format, BOOL is_header);
  int  CalculateNofLines(int first_line, int last_line, int char_pr_line, int lines_pr_page);

  void UpdateAll(void);

  void HandlePreActions(unsigned int action);
  void HandlePostActions(unsigned int action);

  MarkTypeType m_markType;
  void RemoveMark(void);
  void RemoveMarkSticky(void);
  void DrawMarkLines(int start_y, int end_y);
  unsigned int m_lastAction;
  RECT m_clientRect;
  CBrush m_backBrush;
  CBrush m_markBrush;
  bool m_insert;
  void PutTextRest(void);
  TxtLine *m_lastPutTextLine;
  int  m_lastPutTextLineNo;
  std::string m_completionString[TL_MAX_NOF_TAGS];
  void SetSpecialModeStatus(void);
  int m_maxLineLen;

  void AddCutTextUndoEntry(TxtLine *l, int LineNo, int start, int end);
  void AddRemoveLineUndoEntry(TxtLine *l, int LineNo);
  void InsertFormatedTime(const char *format); // Helper function for InsertTime/date
protected:
  WainView();

public:
  void SetHorzSize(int size);
  TxtLineColor m_lineColor;

  void SetScrollInfo(void);
  void OnSaveDoc();
  class WainDoc *GetDocument();
  int m_winNr;
  void SetDocument(WainDoc *doc);
  void GetPopupPos(POINT *p, int NofItems) const;
  void DisableScrollLock(void);
  virtual void OnDraw(CDC *dc);
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
  virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
  virtual void OnPrepareDC( CDC* pDC, CPrintInfo* pInfo = NULL );
  virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
  virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

public:
  class TabViewClass *m_tabView;
  void UpdateTabViewPos();
  void GetCurrentWord(std::string &aStr);
  void GetCurrentLine(std::string& _line);
  bool IsCurrentLine(const TxtLine* _line)
  {
      return _line == m_currentTextLine;
  }
  virtual ~WainView();
  void GotoLineNo(int line, int column = 0);
  void GotoLineNoRaw(int line, int column = 0);
  void UpdateFont(void);
  void ReloadFile(void);
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif
  void SetCursor(void);
  void RemoveCursor(void);
  void MarkBraces(bool _on);
private:
  BOOL m_scrollLockOn;
  int m_mouseWheelPrNotch;
  BOOL DoOpenFileInLine(void);
  bool OpenNewToolFile(int _toolNo);
  BOOL m_mouseDown;
  void CopyToClip(UndoEntryTypeType _undoType);
  void MarkWordCurrent(void);
  void HandleMark(int x, int y, BOOL from_mouse = FALSE);
  void HandleColumnMark(int x, int y, BOOL from_mouse = FALSE);
  void SaveMarkToUndo(void);
  BOOL m_cursorOn;
  RECT m_focusRect;
  int m_first;
  void SetCursorPos(void);
  class TxtLine *m_currentTextLine;
  BOOL m_marking;
  BOOL m_cuaMouseMarkMode;
  CPoint m_mousePoint;
  UINT m_mouseMarkingTimer;
  unsigned int m_nofMouseMarkTimeout;
  int m_markOrgX;
  int m_markOrgY;
  int m_markStartX;
  int m_markStartY;
  int m_markEndX;
  int m_markEndY;
  enum SpecialModeStatus
  {
     CONTINUE_SPECIAL_MODE,
     LEAVE_SPECIAL_MODE,
     LEAVE_SPECIAL_MODE_IGNORE_KEY
  };
  SpecialModeStatus HandleSpecialModes(UINT id, int ch = 0);
  SpecialModeStatus HandleColumnMarkMode(UINT id);
  SpecialModeStatus HandleIncSearch(UINT id, int ch);

  void MarkCutDelete(BOOL copy);
  void DoViewPopupMenu(POINT *p);
  int ExpandSpecial(std::string &aStr);
  void UpdateVisible();
public:
  bool IsMatchPos(const class TxtLine* line, int xPos);
  void RemoveLine(TxtLine* line);
  void OnDebugFileUpdate(int prev_LineCount);
  void ToolOutputNext(void);
  void ToolOutputPrev(void);
  BOOL SearchFunc(const char *string, unsigned int flags);
  BOOL ReplaceFunc(const char *string[2], unsigned int flags);
  void PlaybackMacro(void);
  int m_columnNo;
  int m_lineNo;
  int m_lineHeight;
  int m_charWidth;
  int m_textOffset;
  int m_yOffset;
  int m_xOffset;

  struct DispatcherTableType
  {
    void (WainView::*f_ptr)(void);
  };
  DispatcherTableType DispatcherTable[LAST_DISP_VAL - FIRST_DISP_VAL + 2];
  // Message handlers for messages handled by Dispatcher()
  void MoveUp(void);
  void MoveDown(void);
  void MoveLeft(void);
  void MoveRight(void);
  void MoveLeftEx(void);
  void MoveRightEx(void);
  void MovePageUp(void);
  void MovePageDown(void);
  void MoveHome(void);
  void MoveHomeEx(void);
  void MoveEnd(void);
  void SaveFile(void);
  void SaveFileAs(void);
  void HandleReturn(void);
  void HandleReturnEx(void);
  void HandleDelete(void);
  void DeleteWord(void);
  void HandleBackspace(void);
  void HandleBackspaceEx(void);
  void GotoDoc0(void);
  void GotoDoc1(void);
  void GotoDoc2(void);
  void GotoDoc3(void);
  void GotoDoc4(void);
  void GotoDoc5(void);
  void GotoDoc6(void);
  void GotoDoc7(void);
  void GotoDoc8(void);
  void GotoDoc9(void);
  void ScrollToVisible(void);
  void CopyToClipboard(void);
  void MarkUp(void);
  void MarkDown(void);
  void MarkRight(void);
  void MarkLeft(void);
  void PasteClip(void);
  void MarkCut(void);
  void MarkDelete(void);
  void MdiMax(void);
  void MdiCascade(void);
  void MdiDual(void);
  void FileOpenInLine(void);
  void Undo(void);
  void InsertToggle(void);
  void MoveWordRight(void);
  void MoveWordLeft(void);
  void MoveWordRightEx(void);
  void MoveWordLeftEx(void);
  void MarkWordRight(void);
  void MarkWordLeft(void);
  void MarkPageDown(void);
  void MarkPageUp(void);
  void MarkHome(void);
  void MarkEnd(void);
  void MarkWord(void);
  void MarkDocStart(void);
  void MarkDocEnd(void);
  void InsertTab(void);
  void ShiftBlockRight(void);
  void ShiftBlockLeft(void);
  void FindMatchBrace(void);
  void FindMatchBlock(void);
  void CutSpecial(void);
  void CopySpecial(void);
  void PasteSpecial(void);
  void CompletionList(void);
  void WordCompletion(void);
  void ColumnMarkToggle(void);
  void CompletionFunc(UINT id);
  void MoveDocStart(void);
  void MoveDocEnd(void);
  void GotoLineNo(void);
  void IncSearch(void);
  void SearchNext(void);
  void SearchPrev(void);
  void DoReloadFile(void);
  void ScrollLineUp(void);
  void ScrollLineDown(void);
  void IncreaseMarkTop(void);
  void IncreaseMarkBot(void);
  void DecreaseMarkTop(void);
  void DecreaseMarkBot(void);
  void DoRemoveMark(void);
  void ExpandTemplate(void);
  void IndentLine(void);
  void ToUpper(void);
  void ToLower(void);
  void MoveToNexTab(void);
  void MoveToPrevTab(void);
  void IndentToNextTab(void);
  void IndentToPrevTab(void);
  void FileInfo(void);
  void OpenAsNormalDoc(void);
  void TagPeek(void);
  void ScrollLock(void);
  void Print(void);
  void MoveToMarkStart(void);
  void MoveToMarkEnd(void);
  void HandleDeleteEx(void);
  void HandleMacroRepeat(void);
  void InsertTime(void);
  void InsertDate(void);
  void InsertDateTime(void);
  void InsertTimeDate(void);
  void InsertUserId(void);
  void RemoveTabs(void);
  void GotoBlockStart(void);
  void GotoBlockEnd(void);
  void ViewShellContextMenu(void);
  void ScrollLineCenter(void);
  void ScrollLineTop(void);
  void ScrollLineBottom(void);
  void NextMatchWord(void);
  void PrevMatchWord(void);
  void InsertComment(void);
  void InsertCommentDown(void);
  void MarkAll(void);
  void CopyFileName(void);
  void OpenDirForFile(void);
  void OpenProjectForFile(void);
  void SwitchCppH(void);
  void MarkSimilarWords();
  void MarkSimilarWordsToggle();
  void MarkSimilarWordsOff();
  virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
  void DoInitUpdate(void);
  void ScrollLinePos(int aPos);
protected:
  virtual void OnInitialUpdate(); // called first time after construct
  virtual void OnPrint(CDC *dc, CPrintInfo *print_info);
  void DoWordList(std::string& _word, uint32_t _selectedItem, int _propIndex);
    bool m_skipDraw{false};
protected:
  afx_msg void OnSize( UINT nType, int cx, int cy );
  afx_msg void OnPaint(void);
  afx_msg void OnSetFocus(CWnd *old_wnd);
  afx_msg void OnKillFocus(CWnd *new_wnd);
  afx_msg void OnVScroll(UINT sb_code, UINT pos, CScrollBar *scroll_bar);
  afx_msg void OnHScroll(UINT sb_code, UINT pos, CScrollBar *scroll_bar);
  afx_msg void OnChar(UINT ch, UINT nRepCnt, UINT flags);
  afx_msg void Dispatcher(UINT code);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT flags, CPoint point);
  afx_msg void OnMouseMove(UINT flags, CPoint point);
  afx_msg void OnLButtonUp(UINT flags, CPoint point);
  afx_msg void OnLButtonDblClk(UINT flags, CPoint point);
  afx_msg BOOL OnMouseWheel(UINT flags, short delta, CPoint pt);
  afx_msg void WordListRedo(void);
  afx_msg void WordList(void);
  afx_msg void TagList(void);
  afx_msg void TagListClass();
  afx_msg void TagListDefine();
  afx_msg void TagListType();
  afx_msg void TagListStruct();
  afx_msg void TagListFunction();
  afx_msg void TagListPrototype();
  afx_msg void TagListEnumName();
  afx_msg void TagListEnum();
  afx_msg void TagListVar();
  afx_msg void TagListExtern();
  afx_msg void TagListMember();
  afx_msg void MacroRecordToggle(void);
  afx_msg void MacroPlayback(void);
  afx_msg void HandleEsc(void);
  afx_msg void PrintFile(void);
  afx_msg BOOL OnEraseBkgnd(CDC *dc);
  afx_msg void OnTimer(UINT timer_id);
  afx_msg BOOL OnSetCursor(CWnd *wnd, UINT hit_test, UINT message);
  afx_msg void ViewPopupMenu(void);
  DECLARE_MESSAGE_MAP();
};

#ifndef _DEBUG  // debug version in WAINView.cpp
inline WainDoc *WainView::GetDocument()
{
  return (WainDoc *)m_pDocument;
}
#endif

extern UINT ReplaceMessageId;

class FileInfoDialogClass : public DialogBaseClass
{
  DECLARE_DYNAMIC(FileInfoDialogClass);
public:
  FileInfoDialogClass(CWnd *parent = NULL);
  ~FileInfoDialogClass(void);
  enum {IDD = IDD_FILE_INFO_DIALOG};
  bool m_unixStyle;
  bool m_readOnly;
  int  m_lineCount;
  int  m_charCount;
protected:
  virtual void DoDataExchange(CDataExchange *dx);
  virtual BOOL OnInitDialog();
  virtual void OnCancel();
  virtual void OnOK();
  DECLARE_MESSAGE_MAP();
};
#endif // #ifdef WAINVIEW_H_INC

