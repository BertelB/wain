//=============================================================================
// This source code file is a part of Wain.
// It implements the DockBar
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\wain.h"
#include ".\..\src\wainview.h"
#include ".\..\src\DockBar.h"
#include ".\..\src\mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#pragma warning (disable : 4100) // Unreferenced formal parameter

DockBar::DockBar() : m_buttonColorHighLight(::GetSysColor(COLOR_BTNHILIGHT)), m_buttonColorShadow(::GetSysColor(COLOR_BTNSHADOW))
{
  m_sizeMin = CSize(32, 32);
  m_sizeHorz = CSize(300, 300);
  m_sizeVert = CSize(300, 300);
  m_sizeFloat = CSize(300, 300);
  m_tracking = FALSE;
  m_inCalcNc = FALSE;
  m_cxEdge = 1;
  m_cxBorder = 1;
  m_cxGripper = 20;
  m_myDialog = NULL;
  m_inCalcDynamicLayout = false;
  m_brushBackground.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

  // Rob Wolpov 10/15/98 Added support for diagonal resizing
  m_cyBorder   = 3;
  m_captionSize = GetSystemMetrics(SM_CYSMCAPTION);
  m_minWidth    = GetSystemMetrics(SM_CXMIN);
  m_minHeight   = GetSystemMetrics(SM_CYMIN);
  m_enabled = FALSE;
  m_drawGrip = TRUE;
  m_closePushed = FALSE;
  m_resizePushed = FALSE;
  m_buttonTimer = 0;
}

DockBar::~DockBar()
{
   m_brushBackground.DeleteObject();
}

BEGIN_MESSAGE_MAP(DockBar, CControlBar)
  ON_WM_ERASEBKGND()
  ON_WM_SIZE()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_SETCURSOR()
  ON_WM_WINDOWPOSCHANGED()
  ON_WM_NCPAINT()
  ON_WM_NCLBUTTONDOWN()
  ON_WM_NCLBUTTONUP()
  ON_WM_NCHITTEST()
  ON_WM_NCCALCSIZE()
  ON_WM_LBUTTONDOWN()
  ON_WM_CAPTURECHANGED()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_NCLBUTTONDBLCLK()
  ON_WM_NCMOUSEMOVE()
  ON_WM_SIZE()
  ON_WM_PAINT()
  ON_WM_TIMER()
END_MESSAGE_MAP();

void DockBar::OnUpdateCmdUI(class CFrameWnd *pTarget, int bDisableIfNoHndler)
{
   UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

BOOL DockBar::Create(CWnd* parent, CDialog *dialog, CString &title, UINT id, DWORD style)
{
  ASSERT_VALID(parent);
  ASSERT(!((style & CBRS_SIZE_FIXED) && (style & CBRS_SIZE_DYNAMIC)));
  ASSERT(dialog);
  m_dwStyle = style & CBRS_ALL;

  // create the base window
  CString wndclass = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW),  m_brushBackground, 0);
  if (!CWnd::Create(wndclass, title, style, CRect(0,0,0,0), parent, 0))
    return FALSE;

  // create the child dialog
  m_myDialog = dialog;
  m_myDialog->Create(id, this);

  // use the dialog dimensions as default base dimensions
  CRect rc;
  GetRect(&rc);

  m_sizeHorz = m_sizeVert = m_sizeFloat = rc.Size();
  m_sizeHorz.cy += m_cxEdge + m_cxBorder;
  m_sizeVert.cx += m_cxEdge + m_cxBorder;

  return TRUE;
}

void DockBar::OnPaint()
{
  CControlBar::OnPaint();
}

CSize DockBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
   CRect rc;
   m_pDockSite->GetControlBar(AFX_IDW_DOCKBAR_TOP)->GetWindowRect(rc);
   int nHorzDockBarWidth = bStretch ? 32767 : rc.Width() + 1;
   m_pDockSite->GetControlBar(AFX_IDW_DOCKBAR_LEFT)->GetWindowRect(rc);
   int nVertDockBarHeight = bStretch ? 32767 : rc.Height() + 1;

   if(IsFloating())
      return m_sizeFloat;
   else if (bHorz)
      return CSize(nHorzDockBarWidth, m_sizeHorz.cy);
   else
      return CSize(m_sizeVert.cx, nVertDockBarHeight);
}

CSize DockBar::CalcDynamicLayout(int nLength, DWORD dwMode)
{
  if (IsFloating())
  {
    // Enable diagonal arrow cursor for resizing
    GetParent()->GetParent()->ModifyStyle(MFS_4THICKFRAME,0);
  }
  if (dwMode & (LM_HORZDOCK | LM_VERTDOCK))
  {
    // SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    m_pDockSite->RecalcLayout();
    return CControlBar::CalcDynamicLayout(nLength,dwMode);
  }

  if (dwMode & LM_MRUWIDTH)
    return m_sizeFloat;

  if (dwMode & LM_COMMIT)
  {
    m_sizeFloat.cx = nLength;
    return m_sizeFloat;
  }

  if (IsFloating())
  {
    RECT  window_rect;
    POINT  cursor_pt;

    GetCursorPos(&cursor_pt);
    GetParent()->GetParent()->GetWindowRect(&window_rect);

    switch (m_pDockContext->m_nHitTest)
    {
      case HTTOPLEFT:
        m_sizeFloat.cx = max(window_rect.right - cursor_pt.x, m_minWidth) - m_cxBorder;
        m_sizeFloat.cy = max(window_rect.bottom - m_captionSize - cursor_pt.y, m_minHeight) - 1;
        m_pDockContext->m_rectFrameDragHorz.top = min(cursor_pt.y, window_rect.bottom - m_captionSize - m_minHeight) - m_cyBorder;
        m_pDockContext->m_rectFrameDragHorz.left = min(cursor_pt.x, window_rect.right - m_minWidth) - 1;
        return m_sizeFloat;

      case HTTOPRIGHT:
        m_sizeFloat.cx = max(cursor_pt.x - window_rect.left, m_minWidth);
        m_sizeFloat.cy = max(window_rect.bottom - m_captionSize - cursor_pt.y, m_minHeight) - 1;
        m_pDockContext->m_rectFrameDragHorz.top = min(cursor_pt.y, window_rect.bottom - m_captionSize - m_minHeight) - m_cyBorder;
        return m_sizeFloat;

      case HTBOTTOMLEFT:
        m_sizeFloat.cx = max(window_rect.right - cursor_pt.x, m_minWidth) - m_cxBorder;
        m_sizeFloat.cy = max(cursor_pt.y - window_rect.top - m_captionSize, m_minHeight);
        m_pDockContext->m_rectFrameDragHorz.left = min(cursor_pt.x, window_rect.right - m_minWidth) - 1;
        return m_sizeFloat;

      case HTBOTTOMRIGHT:
        m_sizeFloat.cx = max(cursor_pt.x - window_rect.left, m_minWidth);
        m_sizeFloat.cy = max(cursor_pt.y - window_rect.top - m_captionSize, m_minHeight);
        return m_sizeFloat;
    }
  }

  if (dwMode & LM_LENGTHY)
    return CSize(m_sizeFloat.cx, m_sizeFloat.cy = max(m_sizeMin.cy, nLength));
  else
    return CSize(max(m_sizeMin.cx, nLength), m_sizeFloat.cy);
}

void DockBar::OnWindowPosChanged(WINDOWPOS FAR *window_pos)
{
  if(!::IsWindow(m_hWnd))
    return;
  if(m_myDialog == NULL || !::IsWindow(m_myDialog->m_hWnd))
    return;

  if(m_inCalcNc)
  {
    CRect rc;
    GetClientRect(rc);
    m_myDialog->MoveWindow(rc);
    SetRect(&rc);
    return;
  }

  // Find on which side are we docked
  // UINT nDockBarID = GetParent()->GetDlgCtrlID();
  UINT nDockBarID = GetDockBarId();
  // Return if dropped at same location
  if(nDockBarID == m_dockBarID && (window_pos->flags & SWP_NOSIZE) && ((m_dwStyle & CBRS_BORDER_ANY) != CBRS_BORDER_ANY))
    return;
  m_dockBarID = nDockBarID;

  // Force recalc the non-client area
  m_inCalcNc = TRUE;
  SetWindowPos(NULL, window_pos->x, window_pos->y, window_pos->cx, window_pos->cy,  /* SWP_NOSIZE | SWP_NOMOVE | */ SWP_NOZORDER | SWP_FRAMECHANGED);
  m_inCalcNc = FALSE;
  GetMf()->RecalcChildPos();
}

void DockBar::OnSize(UINT type, int cx, int cy)
{
  CControlBar::OnSize(type, cx, cy);
}

BOOL DockBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
  if ((nHitTest != HTSIZE) || m_tracking)
    return CControlBar::OnSetCursor(pWnd, nHitTest, message);

  if (IsHorz())
    SetCursor(LoadCursor(NULL, IDC_SIZENS));
  else
    SetCursor(LoadCursor(NULL, IDC_SIZEWE));
  return TRUE;
}

void DockBar::OnLButtonUp(UINT nFlags, CPoint point)
{
  if (!m_tracking)
    CControlBar::OnLButtonUp(nFlags, point);
  else
  {
    ClientToWnd(point);
    StopTracking(TRUE);
  }
}

void DockBar::OnMouseMove(UINT nFlags, CPoint point)
{
  if(m_closePushed && !m_rectClose.PtInRect(point))
  {
    m_closePushed = m_resizePushed = FALSE;
    CWindowDC dc(this);
    EraseNonClient(&dc);
    dc.Draw3dRect(m_rectBorder, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_BTNSHADOW));
    DrawGripper(dc);
    if(m_buttonTimer)
    {
      KillTimer(m_buttonTimer);
      m_buttonTimer = 0;
    }
  }

  if (IsFloating() || !m_tracking)
  {
    CControlBar::OnMouseMove(nFlags, point);
    return;
  }

  CPoint cpt = m_rectTracker.CenterPoint();

  ClientToWnd(point);

  if (IsHorz())
  {
    if (cpt.y != point.y)
    {
      OnInvertTracker(m_rectTracker);
      m_rectTracker.OffsetRect(0, point.y - cpt.y);
      OnInvertTracker(m_rectTracker);
    }
  }
  else
  {
    if (cpt.x != point.x)
    {
      OnInvertTracker(m_rectTracker);
      m_rectTracker.OffsetRect(point.x - cpt.x, 0);
      OnInvertTracker(m_rectTracker);
    }
  }
}

void DockBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR *lpncsp)
{
  // Compute the rectangle of the mobile edge
  GetWindowRect(m_rectBorder);
  m_rectBorder = CRect(0, 0, m_rectBorder.Width(), m_rectBorder.Height());
  m_lastNcSize = lpncsp->rgrc[0];
  DWORD dwBorderStyle = m_dwStyle | CBRS_BORDER_ANY;

  switch(m_dockBarID)
  {
    case AFX_IDW_DOCKBAR_TOP:
      dwBorderStyle &= ~CBRS_BORDER_BOTTOM;
      lpncsp->rgrc[0].left += m_cxGripper;
      lpncsp->rgrc[0].bottom += -m_cxEdge;
      lpncsp->rgrc[0].top += m_cxBorder;
      lpncsp->rgrc[0].right += -m_cxBorder;
      m_rectBorder.top = m_rectBorder.bottom - m_cxEdge;
      break;
    case AFX_IDW_DOCKBAR_BOTTOM:
      dwBorderStyle &= ~CBRS_BORDER_TOP;
      lpncsp->rgrc[0].left += m_cxGripper;
      lpncsp->rgrc[0].top += m_cxEdge;
      lpncsp->rgrc[0].bottom += -m_cxBorder;
      lpncsp->rgrc[0].right += -m_cxBorder;
      m_rectBorder.bottom = m_rectBorder.top + m_cxEdge;
      break;
    case AFX_IDW_DOCKBAR_LEFT:
      dwBorderStyle &= ~CBRS_BORDER_RIGHT;
      lpncsp->rgrc[0].right += -m_cxEdge;
      lpncsp->rgrc[0].left += m_cxBorder;
      lpncsp->rgrc[0].bottom += -m_cxBorder;
      lpncsp->rgrc[0].top += m_cxGripper;
      m_rectBorder.left = m_rectBorder.right - m_cxEdge;
      break;
    case AFX_IDW_DOCKBAR_RIGHT:
      dwBorderStyle &= ~CBRS_BORDER_LEFT;
      lpncsp->rgrc[0].left += m_cxEdge;
      lpncsp->rgrc[0].right += -m_cxBorder;
      lpncsp->rgrc[0].bottom += -m_cxBorder;
      lpncsp->rgrc[0].top += m_cxGripper;
      m_rectBorder.right = m_rectBorder.left + m_cxEdge;
      break;
    default:
      m_rectBorder.SetRectEmpty();
      break;
  }

  SetBarStyle(dwBorderStyle);
}

void DockBar::EraseNonClient(CWindowDC *dc)
{
  CRect rectClient;
  GetClientRect(rectClient);
  CRect rectWindow;
  GetWindowRect(rectWindow);
  ScreenToClient(rectWindow);
  rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
  dc->ExcludeClipRect(rectClient);

  // draw borders in non-client area
  rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
  DrawBorders(dc, rectWindow);

  // erase parts not drawn
  dc->IntersectClipRect(rectWindow);
  SendMessage(WM_ERASEBKGND, (WPARAM )dc->m_hDC);
}

void DockBar::OnNcPaint()
{
  CControlBar::OnNcPaint();

  CWindowDC dc(this);

  EraseNonClient(&dc);

  dc.Draw3dRect(m_rectBorder, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_BTNSHADOW));

  DrawGripper(dc);
}

void DockBar::OnNcMouseMove(UINT hit_test, CPoint point)
{
  if((m_closePushed && hit_test != HTSYSMENU) || (m_resizePushed && hit_test != HTMINBUTTON))
  {
    m_closePushed = m_resizePushed = FALSE;
    CWindowDC dc(this);
    EraseNonClient(&dc);
    dc.Draw3dRect(m_rectBorder, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_BTNSHADOW));
    DrawGripper(dc);
    if(m_buttonTimer)
    {
      KillTimer(m_buttonTimer);
      m_buttonTimer = 0;
    }
  }
}

void DockBar::OnNcLButtonUp(UINT hit_test, CPoint point)
{
  if(hit_test == HTSYSMENU && m_closePushed)
  {
    GetDockingFrame()->ShowControlBar(this, FALSE, FALSE);
    m_enabled = FALSE;
    OnClose();
    GetMf()->RecalcChildPos();
    m_closePushed = FALSE;
  }
  else if(hit_test == HTMINBUTTON && m_resizePushed)
  {
    m_pDockContext->ToggleDocking();
    m_resizePushed = FALSE;
  }
  else if(m_closePushed || m_resizePushed)
  {
    m_closePushed = m_resizePushed = FALSE;
  }
  if(m_buttonTimer)
  {
    KillTimer(m_buttonTimer);
    m_buttonTimer = 0;
  }
  CWindowDC dc(this);
  EraseNonClient(&dc);
  dc.Draw3dRect(m_rectBorder, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_BTNSHADOW));
  DrawGripper(dc);
}

void DockBar::OnNcLButtonDown(UINT hit_test, CPoint point)
{
  if(m_tracking)
    return;

  if((hit_test == HTSYSMENU) && !IsFloating())
  {
    m_closePushed = TRUE;
    CWindowDC dc(this);
    EraseNonClient(&dc);
    dc.Draw3dRect(m_rectBorder, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_BTNSHADOW));
    DrawGripper(dc);
    if(!m_buttonTimer)
      m_buttonTimer = SetTimer(IDM_DOCK_BAR_TIMER, 250, NULL);
  }
  else if ((hit_test == HTMINBUTTON) && !IsFloating())
  {
    // ResizePushed = TRUE;
    // CWindowDC dc(this);
    // EraseNonClient(&dc);
    // dc.Draw3dRect(RectBorder, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_BTNSHADOW));
    // DrawGripper(dc);
    // if(!ButtonTimer)
    //    ButtonTimer = SetTimer(IDM_DOCK_BAR_TIMER, 250, NULL);
  }
  else if ((hit_test == HTCAPTION) && !IsFloating() && (m_pDockBar != NULL))
  { // start the drag
    //ASSERT(m_pDockContext != NULL);
    //m_pDockContext->StartDrag(point);
  }
  else if ((hit_test == HTSIZE) && !IsFloating())
  {
    StartTracking();
  }
  //else
  //  CControlBar::OnNcLButtonDown(hit_test, point);
}

LRESULT DockBar::OnNcHitTest(CPoint point)
{
  if (IsFloating())
    return CControlBar::OnNcHitTest(point);

  CRect rc;
  GetWindowRect(rc);
  point.Offset(-rc.left, -rc.top);
  if(m_rectClose.PtInRect(point))
    return HTSYSMENU;
  else if (m_rectGripper.PtInRect(point))
    return HTCAPTION;
  else if (m_rectBorder.PtInRect(point))
    return HTSIZE;
  else
    return CControlBar::OnNcHitTest(point);
}

void DockBar::OnLButtonDown(UINT nFlags, CPoint point)
{
  // only start dragging if clicked in "void" space
  if (m_pDockBar != NULL)
  {
    // start the drag
    ASSERT(m_pDockContext != NULL);
    ClientToScreen(&point);
    m_pDockContext->StartDrag(point);
  }
  else
  {
    CWnd::OnLButtonDown(nFlags, point);
  }
}

void DockBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  // only toggle docking if clicked in "void" space
  if (m_pDockBar != NULL)
  {
    // toggle docking
    ASSERT(m_pDockContext != NULL);
    m_pDockContext->ToggleDocking();
  }
  else
  {
    CWnd::OnLButtonDblClk(nFlags, point);
  }
}

void DockBar::StartTracking()
{
  SetCapture();

  // make sure no Updates are pending
  ::RedrawWindow(m_hWnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
  m_pDockSite->LockWindowUpdate();

  m_oldPoint = m_rectBorder.CenterPoint();
  m_tracking = true;

  m_rectTracker = m_rectBorder;
  if (!IsHorz())
    m_rectTracker.bottom -= 4;

  OnInvertTracker(m_rectTracker);
}

void DockBar::OnCaptureChanged(CWnd *pWnd)
{
  if (m_tracking && pWnd != this)
    StopTracking(FALSE); // cancel tracking

  CControlBar::OnCaptureChanged(pWnd);
}

void DockBar::StopTracking(BOOL accept)
{
  OnInvertTracker(m_rectTracker);
  m_pDockSite->UnlockWindowUpdate();
  m_tracking = false;
  ReleaseCapture();

  if (!accept)
    return;

  int maxsize, minsize, newsize;
  CRect rcc;
  m_pDockSite->GetWindowRect(rcc);

  newsize = IsHorz() ? m_sizeHorz.cy : m_sizeVert.cx;
  maxsize = newsize + (IsHorz() ? rcc.Height() : rcc.Width());
  minsize = IsHorz() ? m_sizeMin.cy : m_sizeMin.cx;

  CPoint point = m_rectTracker.CenterPoint();
  switch (m_dockBarID)
  {
    case AFX_IDW_DOCKBAR_TOP:
      newsize += point.y - m_oldPoint.y;
      break;
    case AFX_IDW_DOCKBAR_BOTTOM:
      newsize += -point.y + m_oldPoint.y;
      break;
    case AFX_IDW_DOCKBAR_LEFT:
      newsize += point.x - m_oldPoint.x;
      break;
    case AFX_IDW_DOCKBAR_RIGHT:
      newsize += -point.x + m_oldPoint.x;
      break;
  }

  newsize = max(minsize, min(maxsize, newsize));

  if (IsHorz())
    m_sizeHorz.cy = newsize;
  else
    m_sizeVert.cx = newsize;

  m_pDockSite->RecalcLayout();
}

void DockBar::OnInvertTracker(const CRect& rect)
{
  ASSERT_VALID(this);
  ASSERT(!rect.IsRectEmpty());
  ASSERT(m_tracking);

  CRect rct = rect, rcc, rcf;
  GetWindowRect(rcc);
  m_pDockSite->GetWindowRect(rcf);

  rct.OffsetRect(rcc.left - rcf.left, rcc.top - rcf.top);
  rct.DeflateRect(1, 1);

  CDC *pDC = m_pDockSite->GetDCEx(NULL, DCX_WINDOW|DCX_CACHE|DCX_LOCKWINDOWUPDATE);

  CBrush *pBrush = CDC::GetHalftoneBrush();
  HBRUSH hOldBrush = NULL;
  if(pBrush != NULL)
    hOldBrush = (HBRUSH )SelectObject(pDC->m_hDC, pBrush->m_hObject);

  pDC->PatBlt(rct.left, rct.top, rct.Width(), rct.Height(), PATINVERT);

  if(hOldBrush != NULL)
    SelectObject(pDC->m_hDC, hOldBrush);

  m_pDockSite->ReleaseDC(pDC);
}

bool DockBar::IsHorz() const
{
  return (m_dockBarID == AFX_IDW_DOCKBAR_TOP || m_dockBarID == AFX_IDW_DOCKBAR_BOTTOM);
}

CPoint& DockBar::ClientToWnd(CPoint& point)
{
  if (m_dockBarID == AFX_IDW_DOCKBAR_BOTTOM)
    point.y += m_cxEdge;
  else if (m_dockBarID == AFX_IDW_DOCKBAR_RIGHT)
    point.x += m_cxEdge;

  return point;
}

void DockBar::DrawGripper(CDC &dc)
{
  // no gripper if floating
  if( m_dwStyle & CBRS_FLOATING )
    return;

  // -==HACK==-
  // in order to calculate the client area properly after docking,
  // the client area must be recalculated twice (I have no idea why)
  m_pDockSite->RecalcLayout();
  // -==END HACK==-
  m_pDockSite->RecalcLayout();

  CRect gripper;
  GetWindowRect(gripper);
  ScreenToClient(gripper);
  gripper.OffsetRect(-gripper.left, -gripper.top);

  if(m_dwStyle & CBRS_ORIENT_HORZ)
  {
    // gripper at left
    m_rectGripper.top    = gripper.top + 20;
    m_rectGripper.bottom = gripper.bottom;
    m_rectGripper.left   = gripper.left;
    m_rectGripper.right  = gripper.left + 20;

    // draw close box
    m_rectClose.left = gripper.left + 4;
    m_rectClose.right = m_rectClose.left + 12;
    m_rectClose.top = gripper.top + 10;
    m_rectClose.bottom = m_rectClose.top + 12;
    if(m_closePushed)
      dc.DrawFrameControl(m_rectClose, DFC_CAPTION, DFCS_CAPTIONCLOSE | DFCS_PUSHED);
    else
      dc.DrawFrameControl(m_rectClose, DFC_CAPTION, DFCS_CAPTIONCLOSE);


    if(m_drawGrip)
    {
      gripper.top += 28;
      gripper.bottom -= 10;
      gripper.left += 7;
      gripper.right = gripper.left + 3;
      dc.Draw3dRect(gripper, m_buttonColorHighLight, m_buttonColorShadow);

      gripper.OffsetRect(4, 0);
      dc.Draw3dRect(gripper, m_buttonColorHighLight, m_buttonColorShadow);
    }
  }
  else
  {
    // gripper at top
    m_rectGripper.top    = gripper.top;
    m_rectGripper.bottom = gripper.top + 20;
    m_rectGripper.left   = gripper.left;
    m_rectGripper.right  = gripper.right - 20;

    // draw close box
    m_rectClose.right  = gripper.right - 10;
    m_rectClose.left   = m_rectClose.right - 11;
    m_rectClose.top    = gripper.top + 5;
    m_rectClose.bottom = m_rectClose.top + 11;
    if(m_closePushed)
      dc.DrawFrameControl(m_rectClose, DFC_CAPTION, DFCS_CAPTIONCLOSE | DFCS_PUSHED);
    else
      dc.DrawFrameControl(m_rectClose, DFC_CAPTION, DFCS_CAPTIONCLOSE);

    // draw docking toggle box
    if(m_drawGrip)
    {
      gripper.right  -= 28;
      gripper.left   += 10;
      gripper.top    += 7;
      gripper.bottom = gripper.top + 3;
      dc.Draw3dRect(gripper, m_buttonColorHighLight, m_buttonColorShadow);

      gripper.OffsetRect(0, 4);
      dc.Draw3dRect(gripper, m_buttonColorHighLight, m_buttonColorShadow);
    }
  }
}

void DockBar::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
  if ((m_pDockBar != NULL) && (nHitTest == HTCAPTION))
  {
    // toggle docking
    ASSERT(m_pDockContext != NULL);
    m_pDockContext->ToggleDocking();
  }
  else
  {
    CWnd::OnNcLButtonDblClk(nHitTest, point);
  }
}

void DockBar::OnTimer(UINT id)
{
  if(id == m_buttonTimer)
  {
    POINT p;
    CPoint point;
    GetCursorPos(&p);
    CRect rc;
    GetWindowRect(rc);
    point = p;
    point.Offset(-rc.left, -rc.top);
    if(m_closePushed && !m_rectClose.PtInRect(point))
    {
      m_closePushed = FALSE;
      CWindowDC dc(this);
      EraseNonClient(&dc);
      dc.Draw3dRect(m_rectBorder, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_BTNSHADOW));
      DrawGripper(dc);
    }
    if(!m_resizePushed && !m_closePushed)
    {
      KillTimer(m_buttonTimer);
      m_buttonTimer = 0;
    }
  }
}

BOOL DockBar::OnEraseBkgnd(CDC *dc)
{
  return CControlBar::OnEraseBkgnd(dc);
}

void NavigatorBarClass::GetRect(RECT *rc)
{
  m_myDialog->GetWindowRect(rc);
  if(wainApp.gs.m_navigatorBarWidth < 40)
    wainApp.gs.m_navigatorBarWidth = 40;
  rc->right = wainApp.gs.m_navigatorBarWidth + rc->left;
}

void NavigatorBarClass::SetRect(RECT *rc)
{
  wainApp.gs.m_navigatorBarWidth = rc->right - rc->left;
}

void NavigatorBarClass::OnClose(void)
{
  GetMf()->SetupMenu(TRUE);
}

void PageBarClass::GetRect(RECT *rc)
{
  m_myDialog->GetWindowRect(rc);
  if(wainApp.gs.m_pageBarHeight < 40)
    wainApp.gs.m_pageBarHeight = 40;
  rc->bottom = rc->top + wainApp.gs.m_pageBarHeight;
}

void PageBarClass::SetRect(RECT *rc)
{
  if(rc->bottom - rc->top > 40)
    wainApp.gs.m_pageBarHeight = rc->bottom - rc->top;
}

void PageBarClass::OnClose(void)
{
  ((PageBarDialogClass *)m_myDialog)->OnClose();
}

COLORREF Brighten(COLORREF old, unsigned int amount)
{
   unsigned int t;
   t = old & 0xFF;
   t += t*amount / 256;
   if (t > 255)
      t = 255;
   old = (old & 0xFFFF00) | t;

   t = (old >> 8) & 0xFF;
   t += t*amount / 256;
   if (t > 255)
      t = 255;
   old = (old & 0xFF00FF) | (t << 8);

   t = (old >> 16) & 0xFF;
   t += t*amount / 256;
   if (t > 255)
      t = 255;
   old = (old & 0x00FFFF) | (t << 16);

   return old;
}
