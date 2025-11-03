//=============================================================================
// This source code file is a part of Wain.
// It defines ChildFrame
//=============================================================================
#ifndef CHILDFRM_H
#define CHILDFRM_H

class ChildFrame : public CMDIChildWnd
{
   DECLARE_DYNCREATE(ChildFrame)
public:
   ChildFrame();
   virtual ~ChildFrame();

  BOOL Create(LPCTSTR aClassName,
              LPCTSTR aWindowName,
              DWORD aStyle = WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
              const RECT &rect = rectDefault,
              CMDIFrameWnd *pParentWnd = 0,
              CCreateContext *pContext = 0);

   void ShowLineNo(bool _show);
   virtual BOOL PreCreateWindow(CREATESTRUCT &cs);
   class WainView* GetView() { return m_view; }
   class WainDoc* GetDocument() { return m_doc; }
   void SetDocument(class WainDoc* _doc) { m_doc = _doc; }
   int GetPosition() const { return m_position; }
   void SetPosition(int _position) { m_position = _position; }
   void TogglePosition() { m_position ^= 1; }
   CreateModeType GetDisplayMode() const { return m_displayMode; }
   void SetDisplayMode(CreateModeType _displayMode) { m_displayMode = _displayMode; }
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
   BOOL m_first;
   void SetTabViewSize(void);
   class TabViewClass *m_tabView;
   int m_position;
   class WainView *m_view;
   class WainDoc *m_doc;
   CreateModeType m_displayMode;
};

#endif // CHILDFRM_H
