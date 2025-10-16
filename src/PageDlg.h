#ifndef SIZECTRLBAR_H_INC
#define SIZECTRLBAR_H_INC

class WainView;

class ViewListItem
{
public:
   WainView *m_view;
   std::string m_label;
   std::string m_fileName;
   CRect m_rectClose;
};

class TabCtrlClass : public CTabCtrl
{
public:
  class PageBarDialogClass *m_dlg;
protected:
  afx_msg void OnLButtonDown(UINT flags, CPoint point);
  afx_msg void OnLButtonUp(UINT flags, CPoint point);
  afx_msg void OnMouseMove(UINT flags, CPoint point);
  afx_msg BOOL OnEraseBkgnd(CDC *dc);
  DECLARE_MESSAGE_MAP();
};

class PageBarDialogClass : public CDialog
{
  int m_closeButton;
  bool DoCloseDoc(int nr);
public:
  void OnClose(void);
  CBrush *m_buttonBrush;
  PageBarDialogClass(CWnd *parent = NULL);
  ~PageBarDialogClass(void);
  virtual BOOL PreTranslateMessage(MSG *msg);

  void SetActiveView(int nNewTab);
  void SetActiveView(WainView *new_view);
  enum {IDD = IDD_PAGE_BAR_DIALOG};
public:
  WainView *GetActiveView();
  WainView *GetView(int view);
  bool NextView(void);
  bool PrevView(void);
  WainView *OpenDocument(const char *file_name, enum DebugFileType FileType, const std::string& _debugFilePath, bool _isProject, bool _isMake, uint32_t _nr, const char* _viewName);
  bool CloseDocument(const char *file_name);
  TabCtrlClass m_tabCtrl;
  void LeftButtonuttonDown(CPoint point);
  void LeftButtonuttonUp(CPoint point);
  void MouseMove(CPoint point);
  void UpdateViews(BOOL hard = FALSE);
  WainDoc *FindDocument(const char *file_name);

protected:
  int m_activeTab;
  UINT m_buttonTimer;

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnTabSelChange(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg HBRUSH OnCtlColor(CDC *dc, CWnd *wnd, UINT ctl_color);
  afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
  afx_msg void OnMouseMove(UINT flags, CPoint point);
  afx_msg void OnTimer(UINT timer_id);
  DECLARE_MESSAGE_MAP();
};

#endif

