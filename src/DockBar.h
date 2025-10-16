//=============================================================================
// This source code file is a part of Wain.
// It defines tool_bar_class.
//=============================================================================
#ifndef DOCK_BAR_H_INC
#define DOCK_BAR_H_INC

class DockBar : public CControlBar // CDialogBar // CReBar // CControlBar
{
  const COLORREF m_buttonColorHighLight;
  const COLORREF m_buttonColorShadow;

public:
  DockBar();

public:
  bool IsHorz() const;
  bool m_enabled;
  bool m_init;
  bool m_drawGrip;
public:
  virtual void OnUpdateCmdUI(CFrameWnd *target, BOOL disable_if_no_hndler);
  virtual UINT GetDockBarId() = 0;
public:
  virtual BOOL Create(CWnd *parent, CDialog *dialog, CString &title, UINT id, DWORD style = WS_CHILD | WS_VISIBLE | CBRS_LEFT);
  virtual CSize CalcFixedLayout(BOOL stretch, BOOL horz );
  virtual CSize CalcDynamicLayout(int length, DWORD mode );
  void EraseNonClient(CWindowDC *dc);
public:
  virtual void GetRect(RECT *rc) = 0;
  virtual void SetRect(RECT *rc) = 0;
  virtual void OnClose(void) = 0;
  virtual ~DockBar();
  void StartTracking();
  void StopTracking(BOOL accept);
  void OnInvertTracker(const CRect &rect);
  CPoint &ClientToWnd(CPoint &point);
  CDialog *m_myDialog;
  UINT    m_dockBarID;
  UINT    m_buttonTimer;
protected:
  void      DrawGripper(CDC &dc);
  CSize     m_sizeMin;
  CSize     m_sizeHorz;
  CSize     m_sizeVert;
  CSize     m_sizeFloat;
  CRect     m_rectBorder;
  CRect     m_rectTracker;

  CPoint    m_oldPoint;
  BOOL      m_tracking;
  BOOL      m_inCalcNc;
  int       m_cxEdge;
  // CRect     RectUndock;
  CRect     m_rectClose;
  CRect     m_rectGripper;
  int       m_cxGripper;
  int       m_cxBorder;
  CBrush    m_brushBackground;

  bool m_closePushed;
  bool m_resizePushed;

  int   m_cyBorder;
  int   m_minWidth;
  int   m_minHeight;
  int   m_captionSize;
  CRect m_rectDocFrame;
  CRect m_lastNcSize;
  bool m_inCalcDynamicLayout;
protected:
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
  afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
  afx_msg void OnNcPaint();
  afx_msg void OnNcLButtonDown(UINT hit_test, CPoint point);
  afx_msg void OnNcLButtonUp(UINT hit_test, CPoint point);
  afx_msg LRESULT OnNcHitTest(CPoint point);
  afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR *lpncsp);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnCaptureChanged(CWnd *pWnd);
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
  afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
  afx_msg void OnPaint(void);
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg BOOL OnEraseBkgnd(CDC *dc);
  afx_msg void OnTimer(UINT timer_id);
  DECLARE_MESSAGE_MAP();
};

class NavigatorBarClass : public DockBar
{
public:
  virtual UINT GetDockBarId()
  {
     return AFX_IDW_DOCKBAR_LEFT;
  }
  virtual void GetRect(RECT *rc);
  virtual void SetRect(RECT *rc);
  virtual void OnClose(void);
};

class PageBarClass : public DockBar
{
public:
  virtual UINT GetDockBarId()
  {
     return AFX_IDW_DOCKBAR_BOTTOM;
  }
  virtual void GetRect(RECT *rc);
  virtual void SetRect(RECT *rc);
  virtual void OnClose(void);
};

extern COLORREF Brighten(COLORREF old, unsigned int amount);

#endif // DOCK_BAR_H_INC


