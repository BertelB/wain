//=============================================================================
// This source code file is a part of Wain.
// It defines navigator_list_class.
//=============================================================================
#ifndef NAVIGATOR_LIST_H_INC
#define NAVIGATOR_LIST_H_INC

class NavigatorDialog;

class NavigatorList : public CScrollView
{
private:
  CFont *m_navigatorListFont;
  unsigned int m_skipUpdateCount;
  bool m_init;
  void MoveSel(int old_pos);
public:
  bool m_canResizeColumn;
  int m_columnWidthRatio;
  NavigatorList(void *dialog);
  ~NavigatorList();
  void Setup(int NofItems, int nof_columns, HIMAGELIST image_list, int width_ratio = 50);
  void UpdateIcons(HIMAGELIST image_list);
  void SetNewFont(void);
  HIMAGELIST m_iconList;
  NavigatorDialog *m_dlg;
  int m_selected;
  void SetSel(int nr);
  void ReDraw(void);
  virtual BOOL PreTranslateMessage(MSG *msg);
  void ColorSetup(void);
private:
  void Update(void);
  void DoUpdate(CDC *dc, int old_pos = -1);
  bool m_repos;
  int m_lineHeight;
  int m_itemCount;
  int m_columnCount;
  int m_columnWidth2;
  int m_columnWidth3;
  virtual void OnDraw(CDC *dc);
  virtual void OnInitialUpdate(void);
  void DoViewPopupMenu(POINT *p);
  char *LimitText(const char *str, int len, CDC *dc);

protected:
  HACCEL m_accHandle;

  afx_msg void OnPaint(void);
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnRButtonDown(UINT flags, CPoint point);
  afx_msg void OnLButtonUp(UINT flags, CPoint point);
  afx_msg void OnLButtonDblClk(UINT flags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnKillFocus(CWnd *new_wnd);
  afx_msg void OnSetFocus(CWnd *old_wnd);
  afx_msg void MoveUp(void);
  afx_msg void MoveDown(void);
  afx_msg void MovePageUp(void);
  afx_msg void MoveCtrlPageUp(void);
  afx_msg void MovePageDown(void);
  afx_msg void HandleDelete(void);
  afx_msg void MoveBack(void);
  afx_msg void GotoEditor(void);
  afx_msg void Select(void);
  afx_msg void OnVScroll(UINT sb_code, UINT pos, CScrollBar *scroll_bar);
  afx_msg BOOL OnEraseBkgnd(CDC *dc);
  afx_msg void OpenAsTagPeek(void);
  afx_msg void OnChar(UINT ch, UINT repeat_count, UINT flags);
  afx_msg void OpenWith(void);
  afx_msg void PopupMenu(void);
  DECLARE_MESSAGE_MAP();
};

#endif

