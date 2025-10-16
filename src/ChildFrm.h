//=============================================================================
// This source code file is a part of Wain.
// It defines ChildFrame
//=============================================================================
#ifndef CHILDFRM_H
#define CHILDFRM_H

class ChildFrame : public CMDIChildWnd
{
  DECLARE_DYNCREATE(ChildFrame)
  BOOL m_first;
  void SetTabViewSize(void);
public:
  ChildFrame();

  BOOL Create(LPCTSTR aClassName,
              LPCTSTR aWindowName,
              DWORD aStyle = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
              const RECT &rect = rectDefault,
              CMDIFrameWnd *pParentWnd = 0,
              CCreateContext *pContext = 0);

  class TabViewClass *m_tabView;
public:
  class WainView *m_view;
  class WainDoc *m_doc;

public:
  void ShowLineNo(bool aShow);
  int m_position;
  CreateModeType m_displayMode;
  virtual BOOL PreCreateWindow(CREATESTRUCT &cs);

public:

  virtual ~ChildFrame();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

protected:
  afx_msg void OnSize(UINT type, int cx, int cy);
  afx_msg void OnPaint(void);
  afx_msg void OnClose(void);
  afx_msg void OnSysCommand(UINT aId, LPARAM aLParam);
  DECLARE_MESSAGE_MAP();
};

#endif // CHILDFRM_H
