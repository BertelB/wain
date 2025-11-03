#include ".\..\src\stdafx.h"
#include ".\..\src\Wain.h"
#include ".\..\src\Resource.h"
#include ".\..\src\PageDlg.h"
#include ".\..\src\Resource.h"
#include ".\..\src\WainView.h"
#include ".\..\src\MainFrm.h"
#include ".\..\src\waindoc.h"
#include ".\..\src\ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4100) // Unreferenced formal parameter

PageBarDialogClass::PageBarDialogClass(CWnd *parent) : CDialog(PageBarDialogClass::IDD, parent)
{
  m_activeTab = -1;

  m_buttonBrush = NULL;
  m_tabCtrl.m_dlg = this;
  m_closeButton = -1;
  m_buttonTimer = 0;
}

PageBarDialogClass::~PageBarDialogClass(void)
{
  if(m_buttonBrush)
  {
     m_buttonBrush->DeleteObject();
     delete m_buttonBrush;
  }
}

BEGIN_MESSAGE_MAP(PageBarDialogClass, CDialog)
  ON_WM_CTLCOLOR()
  ON_WM_CREATE()
  ON_WM_SIZE()
  ON_NOTIFY(TCN_SELCHANGE, IDC_PAGE_BAR_TAB, OnTabSelChange)
  ON_WM_DRAWITEM()
  ON_WM_MOUSEMOVE()
  ON_WM_TIMER()
END_MESSAGE_MAP();

int PageBarDialogClass::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CDialog::OnCreate(lpCreateStruct) == -1)
    return -1;

  //Create the Tab Control
  CRect rect;
  m_tabCtrl.Create(WS_VISIBLE | WS_CHILD | TCS_BOTTOM | TCS_OWNERDRAWFIXED, rect, this, IDC_PAGE_BAR_TAB);

  m_tabCtrl.SetFont(&wainApp.m_guiFont);
  return 0;
}

void PageBarDialogClass::OnSize(UINT nType, int cx, int cy)
{
  CDialog::OnSize(nType, cx, cy);

  if(m_tabCtrl.m_hWnd == NULL)
    return;

  int bottom = cy - 14;
  m_tabCtrl.MoveWindow(7, 7, cx - 14, bottom);

  for(int i = 0; i < m_tabCtrl.GetItemCount(); i++)
  {
    TCITEM item;
    item.mask = TCIF_PARAM;
    if(m_tabCtrl.GetItem(i, &item))
    {
      ViewListItem *view_item = (ViewListItem *)item.lParam;
      WainView *view = view_item->m_view;
      view->MoveWindow(9, 9, cx - 19, bottom - 26);
    }
  }
}

WainView *PageBarDialogClass::OpenDocument(const char* _fileName, DebugFileType _fileType, const std::string& _debugPath, bool _isProject, bool _isMake, uint32_t _nr, const char* _viewName)
{
  WainView *view;
  view = (WainView *)WainView::CreateObject();
  WainDoc *doc = (WainDoc *)WainDoc::CreateObject();
  DWORD dwStyle = AFX_WS_DEFAULT_VIEW;
  CRect rect;
  GetClientRect(&rect);
  rect.left += 10;
  rect.right -= 10;
  rect.top += 9;
  rect.bottom -= 32;
  if (!view->Create(NULL, NULL, dwStyle, rect, this, 0, NULL))
    return NULL;
  doc->AddView(view);
  doc->m_isDebugFile = true;
  doc->m_readAsStdioFile = true;
  doc->OnOpenDocument(_fileName);
  doc->SetPathName(_fileName, false);
  doc->SetReadOnly(true);
  doc->SetDebugPath(_debugPath);
  doc->m_isProject = _isProject;
  doc->m_isMake = _isMake;
  doc->m_nr = _nr;
  view->DoInitUpdate();
  view->InvalidateRect(NULL, TRUE);

  char fn[_MAX_PATH];
  if(_fileType == TagPeekDebugFile)
  {
    MySplitPath(_fileName, SP_FILE | SP_EXT, fn);
    doc->CloseFile();
  }
  else
  {
     MySplitPath(_fileName, SP_FILE, fn);
  }

  ViewListItem *view_item = new ViewListItem;
  view_item->m_view = view;
  view_item->m_label = _viewName && strlen(_viewName) ? _viewName : fn;
  view_item->m_fileName = _fileName;
  m_tabCtrl.SetMinTabWidth(130);
  TC_ITEM tci;
  tci.mask = TCIF_TEXT | TCIF_PARAM;
  tci.pszText = (char *)(_viewName && strlen(_viewName) ? _viewName : fn);
  tci.lParam = (long )view_item;
  int pos = m_tabCtrl.GetItemCount();
  m_tabCtrl.InsertItem(pos, &tci);
  SetActiveView(pos);
  return view;
}

bool PageBarDialogClass::NextView(void)
{
  int nof_view = m_tabCtrl.GetItemCount();
  if(nof_view > 1)
  {
    if(m_activeTab + 1 < nof_view)
      SetActiveView(m_activeTab + 1);
    else
      SetActiveView(0);
    return true;
  }
  return false;
}

bool PageBarDialogClass::PrevView(void)
{
  int nof_view = m_tabCtrl.GetItemCount();

  if(nof_view > 1)
  {
    if(m_activeTab > 0)
      SetActiveView(m_activeTab - 1);
    else
      SetActiveView(nof_view - 1);
    return true;
  }
  return false;
}

bool PageBarDialogClass::CloseDocument(const char *file_name)
{
  int i;
  int nof_view = m_tabCtrl.GetItemCount();

  for(i = 0; i < nof_view; i++)
  {
    TCITEM item;
    item.mask = TCIF_PARAM;

    if(m_tabCtrl.GetItem(i, &item))
    {
      ViewListItem *view_item = (ViewListItem *)item.lParam;
      if(view_item->m_fileName == file_name)
      {
        if(view_item->m_view == wainApp.m_lastAutoTagView)
          wainApp.m_lastAutoTagView = NULL;
        char fn[MAX_PATH];
        MySplitPath(file_name, SP_FILE | SP_EXT, fn);
        return DoCloseDoc(i);
      }
    }
  }
  return false;
}

bool PageBarDialogClass::DoCloseDoc(int nr)
{
  char label[MAX_PATH] = "";
  m_closeButton = -1;
  if(m_buttonTimer)
  {
    KillTimer(m_buttonTimer);
    m_buttonTimer = 0;
  }


  TCITEM item;
  item.mask = TCIF_TEXT | TCIF_PARAM;
  item.pszText = label;
  item.cchTextMax = MAX_PATH;

  if(m_tabCtrl.GetItem(nr, &item))
  {
    ViewListItem *view_item = (ViewListItem *)item.lParam;

    delete view_item->m_view;
    m_tabCtrl.DeleteItem(nr);
    delete view_item;
    int ii = m_tabCtrl.GetCurFocus();
    if(ii >= 0)
      SetActiveView(ii);
    if(m_tabCtrl.GetItemCount() > 0)
      SetActiveView(0);
    else
      m_activeTab = -1;

    return TRUE;
  }
  return FALSE;
}

void PageBarDialogClass::OnTabSelChange(NMHDR *pNMHDR, LRESULT *pResult)
{
  SetActiveView(m_tabCtrl.GetCurSel());
}

void PageBarDialogClass::SetActiveView(int new_tab)
{
  ASSERT_VALID(this);
  ASSERT(new_tab >= 0);
  TCITEM item;
  item.mask = TCIF_PARAM;

  if(m_activeTab != -1)
  {
    if(m_tabCtrl.GetItem(m_activeTab, &item))
    {
      ViewListItem *view_item = (ViewListItem *)item.lParam;
      view_item->m_view->EnableWindow(FALSE);
      view_item->m_view->ShowWindow(SW_HIDE);
    }
  }
  if(m_tabCtrl.GetItem(new_tab, &item))
  {
    m_activeTab = new_tab;
    m_tabCtrl.SetCurSel(m_activeTab);
    ViewListItem *view_item = (ViewListItem *)item.lParam;
    view_item->m_view->EnableWindow(TRUE);
    view_item->m_view->ShowWindow(SW_SHOW);
    view_item->m_view->SetFocus();

    m_tabCtrl.InvalidateRect(NULL, TRUE);
    view_item->m_view->InvalidateRect(NULL, TRUE);
  }
}

void PageBarDialogClass::SetActiveView(WainView *new_view)
{
  BOOL found;
  int i;
  for(i = 0, found = FALSE; i < m_tabCtrl.GetItemCount() && !found; i++)
  {
    TCITEM item;
    item.mask = TCIF_PARAM;
    if(m_tabCtrl.GetItem(i, &item))
    {
      ViewListItem *view_item = (ViewListItem *)item.lParam;
      if(view_item->m_view == new_view)
      {
        if(m_activeTab != i)
          SetActiveView(i);
        found = TRUE;
      }
    }
  }
}

WainView *PageBarDialogClass::GetActiveView()
{
  TCITEM item;
  item.mask = TCIF_PARAM;

  if(m_tabCtrl.GetItemCount() && m_tabCtrl.GetItem(m_activeTab, &item))
  {
    ViewListItem *view_item = (ViewListItem *)item.lParam;
    return view_item->m_view;
  }
  return NULL;
}

WainView *PageBarDialogClass::GetView(int nr)
{
  ASSERT_VALID(this);
  TCITEM item;
  item.mask = TCIF_PARAM;
  if(m_tabCtrl.GetItemCount() > nr && m_tabCtrl.GetItem(nr, &item))
  {
    ViewListItem *view_item = (ViewListItem *)item.lParam;
    return view_item->m_view;
  }
  return NULL;
}

BOOL PageBarDialogClass::PreTranslateMessage(MSG *msg)
{
  if(msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN)
    return FALSE;

  return CDialog::PreTranslateMessage(msg);
}

HBRUSH PageBarDialogClass::OnCtlColor(CDC *_dc, CWnd *wnd, UINT ctrl_color)
//  Description:
//    Message handler, called to get the brush to be used for drawing controls
//  Parameters:
//
{
  if(!m_buttonBrush)
  {
    unsigned int back_color = GetSysColor(COLOR_3DFACE);
    unsigned int new_color;
    CWindowDC dc(this);
    if(dc.GetDeviceCaps(BITSPIXEL) > 8)
    {
      back_color = Brighten(back_color, 32);
      new_color = dc.GetNearestColor(back_color);
      if(new_color == CLR_INVALID)
        new_color = back_color;
    }
    else
    {
       new_color = back_color;
    }
    m_buttonBrush = new CBrush(new_color);
  }

  HBRUSH br = CDialog::OnCtlColor(_dc, wnd, ctrl_color);
  if(ctrl_color == CTLCOLOR_DLG || ctrl_color == CTLCOLOR_STATIC || ctrl_color == CTLCOLOR_BTN)
  {
    br = *m_buttonBrush;
  }
  return br;
}

void PageBarDialogClass::OnDrawItem(int id_ctrl, LPDRAWITEMSTRUCT draw_item_struct)
{
  CRect dr = draw_item_struct->rcItem;
  if(!(draw_item_struct->itemState & ODS_SELECTED))
    dr.top -= 2;
  CBrush Br(::GetBkColor(draw_item_struct->hDC));
  ::FillRect(draw_item_struct->hDC, dr, Br);
  Br.DeleteObject();
  TCITEM item;
  char buffer[128];
  item.pszText = buffer;
  item.cchTextMax = 128;
  item.mask = TCIF_TEXT | TCIF_PARAM;
  int id = draw_item_struct->itemID;

  m_tabCtrl.GetItem(id, &item);
  dr = draw_item_struct->rcItem;
  dr.left += 4;
  if(draw_item_struct->itemState & ODS_SELECTED)
    dr.top += 4;

  ::DrawText(draw_item_struct->hDC, item.pszText, strlen(item.pszText), dr, 0);
  ViewListItem *view_item = (ViewListItem *)item.lParam;

  view_item->m_rectClose = draw_item_struct->rcItem;
  view_item->m_rectClose.left = view_item->m_rectClose.right - 12;
  int h = (view_item->m_rectClose.bottom - view_item->m_rectClose.top - 12)/2;
  view_item->m_rectClose.top += h;
  if(!(draw_item_struct->itemState & ODS_SELECTED))
    view_item->m_rectClose.top -= 2;

  view_item->m_rectClose.right -= h;
  view_item->m_rectClose.left -= h;
  view_item->m_rectClose.bottom = view_item->m_rectClose.top + 12;
  if(id == m_closeButton)
    ::DrawFrameControl(draw_item_struct->hDC, &view_item->m_rectClose, DFC_CAPTION, DFCS_CAPTIONCLOSE | DFCS_PUSHED);
  else
    ::DrawFrameControl(draw_item_struct->hDC, &view_item->m_rectClose, DFC_CAPTION, DFCS_CAPTIONCLOSE);
}

BEGIN_MESSAGE_MAP(TabCtrlClass, CTabCtrl)
  ON_WM_LBUTTONUP()
  ON_WM_LBUTTONDOWN()
  ON_WM_MOUSEMOVE()
  ON_WM_ERASEBKGND()
END_MESSAGE_MAP();

BOOL TabCtrlClass::OnEraseBkgnd(CDC *dc)
{
  if(GetItemCount())
  {
    RECT cr;
    GetClientRect(&cr);
    CBrush bb(GetSysColor(COLOR_3DFACE));
    dc->FillRect(&cr, &bb);
    cr.top = cr.bottom - 20;
    dc->FillRect(&cr, m_dlg->m_buttonBrush);
    bb.DeleteObject();
    return TRUE;
  }
  return CTabCtrl::OnEraseBkgnd(dc);
}

void TabCtrlClass::OnLButtonDown(UINT flags, CPoint point)
{
  CTabCtrl::OnLButtonDown(flags, point);
  m_dlg->LeftButtonuttonDown(point);
}

void TabCtrlClass::OnLButtonUp(UINT flags, CPoint point)
{
  CTabCtrl::OnLButtonDown(flags, point);
  m_dlg->LeftButtonuttonUp(point);
}

void TabCtrlClass::OnMouseMove(UINT flags, CPoint point)
{
  CTabCtrl::OnMouseMove(flags, point);
  m_dlg->MouseMove(point);
}

void PageBarDialogClass::LeftButtonuttonDown(CPoint point)
{
  for(int i = 0; i < m_tabCtrl.GetItemCount(); i++)
  {
    TCITEM item;
    item.mask = TCIF_PARAM;
    if(m_tabCtrl.GetItem(i, &item))
    {
      ViewListItem *view_item = (ViewListItem *)item.lParam;
      if(view_item->m_rectClose.PtInRect(point))
      {
        m_closeButton = i;
        if(!m_buttonTimer)
          m_buttonTimer = SetTimer(IDM_PAGE_BAR_TIMER, 500, NULL);
        m_tabCtrl.InvalidateRect(view_item->m_rectClose, TRUE);
        return;
      }
    }
  }
  m_closeButton = -1;
  if(m_buttonTimer)
  {
    KillTimer(m_buttonTimer);
    m_buttonTimer = 0;
  }
}

void PageBarDialogClass::LeftButtonuttonUp(CPoint point)
{
  if(m_closeButton == -1)
    return;
  for(int i = 0; i < m_tabCtrl.GetItemCount(); i++)
  {
    TCITEM item;
    item.mask = TCIF_PARAM;
    if(m_tabCtrl.GetItem(i, &item))
    {
      ViewListItem *view_item = (ViewListItem *)item.lParam;
      if(i == m_closeButton && view_item->m_rectClose.PtInRect(point))
      {
        GetMf()->CloseDebugFile(view_item->m_fileName.c_str(), true, true);
        m_closeButton = -1;

        if(m_buttonTimer)
        {
          KillTimer(m_buttonTimer);
          m_buttonTimer = 0;
        }
        return;
      }
    }
  }
  TCITEM item;
  item.mask = TCIF_PARAM;
  if(m_closeButton != -1 && m_tabCtrl.GetItem(m_closeButton, &item))
  {
    ViewListItem *view_item = (ViewListItem *)item.lParam;
    m_closeButton = -1;
    m_tabCtrl.InvalidateRect(view_item->m_rectClose, TRUE);
    if(m_buttonTimer)
    {
      KillTimer(m_buttonTimer);
      m_buttonTimer = 0;
    }
  }
  m_closeButton = -1;
}

void PageBarDialogClass::OnMouseMove(UINT flags, CPoint point)
{
  CDialog::OnMouseMove(flags, point);
  MouseMove(point);
}

void PageBarDialogClass::MouseMove(CPoint point)
{
  if(m_closeButton == -1)
    return;
  TCITEM item;
  item.mask = TCIF_PARAM;
  if(m_tabCtrl.GetItem(m_closeButton, &item))
  {
    ViewListItem *view_item = (ViewListItem *)item.lParam;
    if(!view_item->m_rectClose.PtInRect(point))
    {
      m_closeButton = -1;
      m_tabCtrl.InvalidateRect(view_item->m_rectClose, TRUE);
      if(m_buttonTimer)
      {
        KillTimer(m_buttonTimer);
        m_buttonTimer = 0;
      }
    }
  }
}

void PageBarDialogClass::OnClose(void)
{
   while(m_tabCtrl.GetItemCount())
   {
      GetMf()->SetActiveView(NULL);
      TCITEM item;
      item.mask = TCIF_PARAM;
      if(m_tabCtrl.GetItem(0, &item))
      {
         ViewListItem *view_item = (ViewListItem *)item.lParam;
         delete view_item->m_view;
         delete view_item;
         m_tabCtrl.DeleteItem(0);
      }
      wainApp.m_lastAutoTagView = NULL;
      m_activeTab = -1;
   }
   ChildFrame *cf = (ChildFrame *)(GetMf()->MDIGetActive());
   if(cf)
   {
      GetMf()->SetActiveView(cf->GetView());
   }
   if(m_buttonTimer)
   {
      KillTimer(m_buttonTimer);
      m_buttonTimer = 0;
   }
   m_closeButton = -1;
}

void PageBarDialogClass::OnTimer(UINT timer_id)
{
  if(timer_id == m_buttonTimer && m_closeButton != -1)
  {
    POINT p;
    CPoint point;
    GetCursorPos(&p);
    CRect rc;
    GetWindowRect(rc);
    point = p;
    point.Offset(-rc.left, -rc.top);
    point.Offset(-4, -4);
    TCITEM item;
    item.mask = TCIF_PARAM;
    if(m_tabCtrl.GetItem(m_closeButton, &item))
    {
      ViewListItem *view_item = (ViewListItem *)item.lParam;
      if(!view_item->m_rectClose.PtInRect(point))
      {
        m_closeButton = -1;
        m_tabCtrl.InvalidateRect(view_item->m_rectClose, TRUE);
      }
    }
  }
  if(m_closeButton == -1 && m_buttonTimer != 0)
  {
    KillTimer(m_buttonTimer);
    m_buttonTimer = 0;
  }
}

void PageBarDialogClass::UpdateViews(BOOL _hard)
{
   int i, nof = m_tabCtrl.GetItemCount();
   for(i = 0; i < nof; i++)
   {
      TCITEM item;
      item.mask = TCIF_PARAM;
      if(m_tabCtrl.GetItem(0, &item))
      {
         ViewListItem *viewItem = (ViewListItem *)item.lParam;
         if(_hard)
         {
            viewItem->m_view->GetDocument()->GetExtType(NULL, TRUE);
         }
         else
         {
            viewItem->m_view->InvalidateRect(NULL, TRUE);
         }
      }
   }
}

WainDoc *PageBarDialogClass::FindDocument(const char *file_name)
{
  int i;
  int nof_view = m_tabCtrl.GetItemCount();
  for(i = 0; i < nof_view; i++)
  {
    TCITEM item;
    item.mask = TCIF_PARAM;

    if(m_tabCtrl.GetItem(i, &item))
    {
      ViewListItem *view_item = (ViewListItem *)item.lParam;

      if(view_item->m_fileName == file_name)
      {
        return view_item->m_view->GetDocument();
      }
    }
  }

   return 0;
}
