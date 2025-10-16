//=============================================================================
// This source code file is a part of Wain.
// It implements the NavigatorList.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\Wain.h"

// #include ".\..\src\WainDoc.h"
#include ".\..\src\WainView.h"
#include ".\..\src\MainFrm.h"
#include ".\..\src\NavigatorList.h"
#include ".\..\src\childfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4238) // nonstandard extension used
#pragma warning (disable : 4100) // unreferenced formal parameter

class NavigatorListColorEditClass : public CEdit
{
public:
  bool m_init;
  CFont *m_font;
  afx_msg void OnPaint(void);
  NavigatorListColorEditClass(void);
  class NavigatorListColorSetupDialogClass *m_dlg;
  DECLARE_MESSAGE_MAP();
};

class NavigatorListColorSetupDialogClass : public CDialog
{
  DECLARE_DYNAMIC(NavigatorListColorSetupDialogClass);
  NavigatorListColorEditClass m_colorEdit;
public:
  CFont *m_font;
  NavigatorListColorSetupDialogClass(CWnd *parent);
  ~NavigatorListColorSetupDialogClass(void);
  virtual BOOL OnInitDialog(void);
  enum {IDD = IDD_NAVIGATOR_LIST_COLOR_DLG};
  virtual void OnOK();
  uint32_t m_listColorBack;
  uint32_t m_listColorText;
  uint32_t m_listColorSel;
  uint32_t m_listColorSelFocus;
  uint32_t m_listColorTextSel;
  uint32_t m_sel;
protected:
  void DoDataExchange(CDataExchange *dx);
  afx_msg void BlueChanged(void);
  afx_msg void GreenChanged(void);
  afx_msg void RedChanged(void);
  afx_msg void NewSel(void);
  DECLARE_MESSAGE_MAP();
};

#define COLUMN_WIDTH_1 20

BEGIN_MESSAGE_MAP(NavigatorList, CScrollView)
  ON_COMMAND(IDL_MOVE_DOWN,          MoveDown)
  ON_COMMAND(IDL_MOVE_UP,            MoveUp)
  ON_COMMAND(IDL_MOVE_PAGE_DOWN,     MovePageDown)
  ON_COMMAND(IDL_MOVE_CTRL_PAGE_UP,  MoveCtrlPageUp)
  ON_COMMAND(IDL_MOVE_PAGE_UP,       MovePageUp)
  ON_COMMAND(IDL_DELETE,             HandleDelete)
  ON_COMMAND(IDB_EDIT,               GotoEditor)
  ON_COMMAND(IDL_SELECT,             Select)
  ON_COMMAND(IDB_TAG_PEEK,           OpenAsTagPeek)
  ON_COMMAND(IDL_MOVE_BACK,          MoveBack)
  ON_COMMAND(IDL_VIEW_POPUP,         PopupMenu)
  ON_WM_VSCROLL()
  ON_WM_SIZE()
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_KILLFOCUS()
  ON_WM_SETFOCUS()
  ON_WM_ERASEBKGND()
  ON_WM_CHAR()
END_MESSAGE_MAP();

PopupMenuListClass PopupMenuList[][5] =
{
  { /* Dir */
    PopupMenuListClass("", 0)
  },
  {  /* FTP */
    PopupMenuListClass("Parent dir\tBACK",      IDL_MOVE_CTRL_PAGE_UP),
    PopupMenuListClass("Select\tEnter",         IDL_SELECT),
    PopupMenuListClass("Jump to editor\tALT+E", IDB_EDIT),
    PopupMenuListClass("",                      0)
  },
  { /* Files */
    PopupMenuListClass("Open\tEnter",           IDL_SELECT),
    PopupMenuListClass("Close\tDelete",         IDL_DELETE),
    PopupMenuListClass("Jump to editor\tALT+E", IDB_EDIT),
    PopupMenuListClass("",                      0)
  },
  { /* tags */
    PopupMenuListClass("Select\tEnter",         IDL_SELECT),
    PopupMenuListClass("Jump to editor\tALT+E", IDB_EDIT),
    PopupMenuListClass("TagPeek\tCTRL+A",       IDB_TAG_PEEK),
    PopupMenuListClass("",                      0)
  },
  {  /* class, it has its own */
    PopupMenuListClass("",                      0)
  },
  { /* Project */
    PopupMenuListClass("Add files\tInsert",     IDL_INSERT),
    PopupMenuListClass("Remove from project\tDelete", IDL_DELETE),
    PopupMenuListClass("Open\tEnter",           IDL_SELECT),
    PopupMenuListClass("Jump to editor\tALT+E", IDB_EDIT),
    PopupMenuListClass("",                      0)
  },
  { /* Current */
    PopupMenuListClass("Select\tEnter",         IDL_SELECT),
    PopupMenuListClass("Jump to editor\tALT+E", IDB_EDIT),
    PopupMenuListClass("TagPeek\tCTRL+A",       IDB_TAG_PEEK),
    PopupMenuListClass("",                      0)
  }
};

NavigatorList::NavigatorList(void *dialog)
{
  m_skipUpdateCount = 0;
  m_dlg = (NavigatorDialog *)dialog;
  m_itemCount = 0;
  m_columnCount = 0;
  m_columnWidth2 = m_columnWidth3 = 0;
  m_navigatorListFont = new CFont();
  m_navigatorListFont->CreateFontIndirect(&wainApp.gs.m_navigatorListLogFont);
  m_selected = 0;
  m_repos = false;
  m_accHandle = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LIST_VIEW));
  m_init = false;
}

NavigatorList::~NavigatorList(void)
{
  delete m_navigatorListFont;
}

void NavigatorList::OnInitialUpdate(void)
{
}

void NavigatorList::Setup(int _nofItems, int _nofColumns, HIMAGELIST _imageList, int _widthRatio)
{
  m_itemCount = _nofItems;
  m_iconList = _imageList;
  m_columnCount = _nofColumns;
  m_columnWidthRatio = _widthRatio;
  m_init = true;
  RECT cr;
  GetClientRect(&cr);
  CDC *dc = GetDC();
  CFont *old_font = dc->SelectObject(m_navigatorListFont);

  if(m_columnCount == 3)
  {
    if(_widthRatio < 0)
    {
      m_columnWidth2 = dc->GetTextExtent("99WW", 4).cx;
      m_columnWidth3 = cr.right - m_columnWidth2 - COLUMN_WIDTH_1;
      m_canResizeColumn = FALSE;
    }
    else
    {
      if(m_columnWidthRatio > 100)
        m_columnWidthRatio = 50;
      int x = cr.right - COLUMN_WIDTH_1;
      m_columnWidth2 = x*_widthRatio/100;
      m_columnWidth3 = x*(100 - _widthRatio)/100;
      m_canResizeColumn = TRUE;
    }
  }
  else
  {
    m_columnWidth2 = cr.right - COLUMN_WIDTH_1;
    m_columnWidth3 = 0;
    m_canResizeColumn = FALSE;
  }

  TEXTMETRIC tm;
  dc->GetOutputTextMetrics(&tm);
  if (tm.tmHeight < 0)
    m_lineHeight = -tm.tmHeight;
  else
    m_lineHeight = (tm.tmHeight - tm.tmInternalLeading);

  m_lineHeight += 3;
  m_lineHeight = m_lineHeight < 16 ? 16 : m_lineHeight;
  SetScrollSizes(MM_TEXT, CSize(0, m_lineHeight*m_itemCount),
                          CSize(0, (cr.bottom/m_lineHeight  - 2)*m_lineHeight),
                          CSize(0, m_lineHeight));

  dc->SelectObject(old_font);
  ReleaseDC(dc);
  m_selected = 0;
  SetScrollPos(SB_VERT, 0, FALSE);
  InvalidateRect(NULL, TRUE);
}

void NavigatorList::MoveSel(int old_pos)
{
  CDC *dc = GetDC();
  DoUpdate(dc, old_pos);
  m_dlg->OnListSetSel(m_selected);

  ReleaseDC(dc);
}

void NavigatorList::UpdateIcons(HIMAGELIST image_list)
{
  m_iconList = image_list;
  Update();
}

void NavigatorList::Update(void)
{
  CDC *dc = GetDC();
  DoUpdate(dc);
  ReleaseDC(dc);
}

void NavigatorList::DoUpdate(CDC *dc, int old_pos)
{
  if(!m_init)
    return;
  int i, j;
  const char *s;
  CPoint sp = GetScrollPosition();
  RECT cr;
  GetClientRect(&cr);
  /* setup() might have been called before GetClientRect() could give a proper answer, ColumnWidth3 will in that case be < 0 */
  if(m_columnWidth3 < 0)
    m_columnWidth3 = cr.right - m_columnWidth2 - COLUMN_WIDTH_1;
  int first = sp.y/m_lineHeight;
  if(first)
    first--;
  int last = (sp.y + cr.bottom)/m_lineHeight;
  last++;
  if(last > m_itemCount)
    last = m_itemCount;

  CFont *old_font = dc->SelectObject(m_navigatorListFont);
  int cp2 = COLUMN_WIDTH_1;
  int cp3 = COLUMN_WIDTH_1 + m_columnWidth2;
  int image;
  COLORREF bk_clr;
  COLORREF text_clr;

  RECT curr;
  curr.left = 0;
  curr.right = cr.right;

  if(old_pos != -1)
  { /* Just draw old_pos & selected */
    first = MIN(old_pos, m_selected);
    last = MAX(old_pos, m_selected);
    last++;
  }

  for(i = first; i < last; i++)
  {
    if(i == m_selected)
    {
      if(GetFocus() == this)
        bk_clr = wainApp.gs.m_listColorSelFocus;
      else
        bk_clr = wainApp.gs.m_listColorSel;
      text_clr = wainApp.gs.m_listColorTextSel;
      curr.top = i*m_lineHeight - sp.y;
      curr.bottom = (i + 1)*m_lineHeight - sp.y;
      CBrush Br(bk_clr);
      dc->FillRect(&curr, &Br);
      Br.DeleteObject();
    }
    else
    {
      curr.top = i*m_lineHeight - sp.y;
      curr.bottom = (i + 1)*m_lineHeight - sp.y;
      text_clr = wainApp.gs.m_listColorText;
      bk_clr = wainApp.gs.m_listColorBack;
      CBrush Br(wainApp.gs.m_listColorBack);
      dc->FillRect(&curr, &Br);
      Br.DeleteObject();
    }

    image = m_dlg->GetListIconNr(i);
    if(image >= 0)
    {
      ImageList_Draw(m_iconList, image, dc->m_hDC, 1, i*m_lineHeight - sp.y, ILD_TRANSPARENT);
    }
    dc->SetBkColor(bk_clr);
    dc->SetTextColor(text_clr);
    for(j = 1; j < m_columnCount; j++)
    {
      s = m_dlg->GetListText(i, j, wainApp.gs.m_navigatorBarShortNames);
      if(s)
      {
        s = LimitText(s,  j == 1 ? m_columnWidth2 : m_columnWidth3, dc);
        dc->TextOut((j == 1 ? cp2 : cp3) - sp.x, i*m_lineHeight - sp.y, s, strlen(s));
      }
    }
  }
  dc->SelectObject(old_font);
}

void NavigatorList::OnDraw(CDC *dc)
{

}

void NavigatorList::OnPaint(void)
{
  if(!m_skipUpdateCount)
  {
    CPaintDC dc(this);
    DoUpdate(&dc);
  }
  else
    m_skipUpdateCount--;
}

afx_msg void NavigatorList::OnLButtonDown(UINT flags, CPoint point)
{
  if(!m_init)
     return;
  if(m_canResizeColumn && point.x > (COLUMN_WIDTH_1 + m_columnWidth2 - 2) && point.x < (COLUMN_WIDTH_1 + m_columnWidth2 + 2))
  {
    m_repos = TRUE;
    SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
    SetCapture();
  }
  else
  {
    CPoint sp = GetScrollPosition();
    sp.y += point.y;
    int i = sp.y/m_lineHeight;
    if(i < m_itemCount)
    {
      m_selected = i;
      Update();
      m_dlg->OnListSetSel(m_selected);
    }
  }
}

void NavigatorList::OnMouseMove(UINT flags, CPoint point)
{
  if(!m_init)
     return;
  if(m_repos)
  {
    if(point.x < COLUMN_WIDTH_1 + 10)
    {
      m_repos = FALSE;
      ReleaseCapture();
    }
    else
    {
      int w = point.x - COLUMN_WIDTH_1;
      if(w < 15)
        w = 15;
      RECT cr;
      GetClientRect(&cr);
      int x = cr.right - COLUMN_WIDTH_1;
      if(!x)
        x = 10;
      m_columnWidthRatio = 100*w/x;
      m_columnWidth2 = x*m_columnWidthRatio/100;
      m_columnWidth3 = x*(100 - m_columnWidthRatio)/100;
    }
    Update();
  }
  if(m_repos || (m_canResizeColumn && point.x > (COLUMN_WIDTH_1 + m_columnWidth2 - 2) && point.x < (COLUMN_WIDTH_1 + m_columnWidth2 + 2)))
    SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
  else if(!m_repos)
    SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}

void NavigatorList::OnLButtonUp(UINT flags, CPoint point)
{
  if(!m_init)
    return;

  if(m_repos)
  {
    Update();
    SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
    wainApp.gs.m_widthRatio = m_columnWidthRatio;
    m_repos = FALSE;
    ReleaseCapture();
  }
}

void NavigatorList::OnVScroll(UINT sb_code, UINT pos, CScrollBar *scroll_bar)
{
  if(sb_code == SB_THUMBTRACK)
  {
    SCROLLINFO si;
    if(GetScrollInfo(SB_VERT, &si, SIF_TRACKPOS))
      pos = si.nTrackPos;
  }
  CScrollView::OnVScroll(sb_code, pos, scroll_bar);
}

void NavigatorList::OnSize(UINT type, int cx, int cy)
{
  CScrollView::OnSize(type, cx, cy);

  if(cx < COLUMN_WIDTH_1 + 20 || m_columnCount == 0)
    return;
  if(m_canResizeColumn)
  {
    int x = cx - COLUMN_WIDTH_1;
    m_columnWidth2 = x*m_columnWidthRatio/100;
    m_columnWidth3 = x*(100 - m_columnWidthRatio)/100;
  }
  else if(m_columnCount == 2)
    m_columnWidth2 = cx - COLUMN_WIDTH_1;
  Update();
}

void NavigatorList::OnLButtonDblClk(UINT flags, CPoint point)
{
  if(!m_init)
    return;
  CPoint sp = GetScrollPosition();
  sp.y += point.y;
  int i = sp.y/m_lineHeight;
  if(i < m_itemCount)
  {
    m_dlg->DoSelect(i);
  }
}

void NavigatorList::Select(void)
{
  if(m_selected < m_itemCount)
    m_dlg->DoSelect(m_selected);
}

void NavigatorList::ReDraw(void)
{
  InvalidateRect(NULL, TRUE);
}

void NavigatorList::MoveUp(void)
{
  if(m_selected)
  {
    m_selected--;
    CPoint sp = GetScrollPosition();
    if(m_selected*m_lineHeight < sp.y)
    {
      sp.y -= m_lineHeight;
      if(sp.y < 0)
        sp.y = 0;
      ScrollToPosition(sp);
      Update();
      m_dlg->OnListSetSel(m_selected);
    }
    else
      MoveSel(m_selected + 1);
  }
}

void NavigatorList::MoveDown(void)
{
  if(m_selected < m_itemCount - 1)
  {
    m_selected++;
    CPoint sp = GetScrollPosition();
    RECT cr;
    GetClientRect(&cr);
    if(cr.bottom + sp.y < (m_selected + 1)*m_lineHeight)
    {
      sp.y += m_lineHeight;
      ScrollToPosition(sp);
      Update();
      m_dlg->OnListSetSel(m_selected);
    }
    else
      MoveSel(m_selected - 1);
  }
}

void NavigatorList::MovePageUp(void)
{
  if(m_selected)
  {
    RECT cr;
    GetClientRect(&cr);
    int nof_lines = cr.bottom/m_lineHeight;
    nof_lines--;
    if(m_selected < nof_lines)
    {
      nof_lines = m_selected;
      m_selected = 0;
    }
    else
      m_selected -= nof_lines;

    CPoint sp = GetScrollPosition();
    if(m_selected*m_lineHeight < sp.y)
    {
      sp.y -= m_lineHeight*nof_lines;
      if(sp.y < 0)
        sp.y = 0;
      ScrollToPosition(sp);
    }
    Update();
    m_dlg->OnListSetSel(m_selected);
  }
}

void NavigatorList::MoveCtrlPageUp(void)
{
  m_dlg->HandleListCtrlPageUp();
}

void NavigatorList::MovePageDown(void)
{
  if(m_selected < m_itemCount - 1)
  {
    RECT cr;
    GetClientRect(&cr);
    int nof_lines = cr.bottom/m_lineHeight;
    nof_lines--;
    if(m_selected + nof_lines >= m_itemCount)
    {
      nof_lines = (m_itemCount - 1) - m_selected;
      m_selected = m_itemCount - 1;
    }
    else
      m_selected += nof_lines;

    CPoint sp = GetScrollPosition();
    if(cr.bottom + sp.y < (m_selected + 1)*m_lineHeight)
    {
      sp.y += m_lineHeight*nof_lines;
      ScrollToPosition(sp);
    }
    Update();
    m_dlg->OnListSetSel(m_selected);
  }
}

BOOL NavigatorList::PreTranslateMessage(MSG *msg)
{
  if(m_accHandle && TranslateAccelerator(m_hWnd, m_accHandle, msg))
    return TRUE;

  if(msg->message == WM_CHAR)
  {
    if(msg->wParam >= 32 && msg->wParam <= 127)
    {
      OnChar(msg->wParam, LOWORD(msg->lParam), HIWORD(msg->lParam));
      return TRUE;
    }
  }

  return CScrollView::PreTranslateMessage(msg);
}

void NavigatorList::OnKillFocus(CWnd *new_wnd)
{
  CScrollView::OnKillFocus(new_wnd);
  Update();
}

void NavigatorList::OnSetFocus(CWnd *old_wnd)
{
  CScrollView::OnSetFocus(old_wnd);
  Update();
  if(m_dlg)
    m_dlg->OnListSetSel(m_selected);
}

void NavigatorList::SetNewFont(void)
{
  m_navigatorListFont->Detach();
  m_navigatorListFont->CreateFontIndirect(&wainApp.gs.m_navigatorListLogFont);

  CDC *dc = GetDC();
  CFont *old_font = dc->SelectObject(m_navigatorListFont);
  TEXTMETRIC tm;
  dc->GetOutputTextMetrics(&tm);
  if (tm.tmHeight < 0)
    m_lineHeight = -tm.tmHeight;
  else
    m_lineHeight = (tm.tmHeight - tm.tmInternalLeading);

  m_lineHeight += 3;
  m_lineHeight = m_lineHeight < 16 ? 16 : m_lineHeight;
  RECT cr;
  GetClientRect(&cr);
  SetScrollSizes(MM_TEXT, CSize(COLUMN_WIDTH_1 + m_columnWidth2 + m_columnWidth3, m_lineHeight*m_itemCount),
                          CSize(cr.right/2, (cr.bottom/m_lineHeight  - 2)*m_lineHeight),
                          CSize(20, m_lineHeight));
  m_selected = 0;
  SetScrollPos(SB_VERT, 0, FALSE);
  if(m_columnCount == 3 && m_columnWidthRatio < 0)
  {
    m_columnWidth2 = dc->GetTextExtent("99WW", 4).cx;
    m_columnWidth3 = cr.right - m_columnWidth2 - COLUMN_WIDTH_1;
  }
  dc->SelectObject(old_font);
  ReleaseDC(dc);

  InvalidateRect(NULL, TRUE);
}

void NavigatorList::HandleDelete(void)
{
  m_dlg->HandleDelete(m_selected);
  SetFocus(); // The mainframe will put focus to a view if the user removes the active view
}

void NavigatorList::SetSel(int nr)
{
  if(nr >= m_itemCount)
    return;
  if(nr == m_selected)
    return;
  m_selected = nr;

  RECT cr;
  GetClientRect(&cr);
  CPoint sp = GetScrollPosition();
  if(cr.bottom + sp.y < (m_selected + 1)*m_lineHeight)
  {
    sp.y = m_lineHeight*nr;
    m_skipUpdateCount++;
    ScrollToPosition(sp);
  }
  if(m_selected*m_lineHeight < sp.y)
  {
    sp.y = m_lineHeight*nr;
    m_skipUpdateCount++;
    ScrollToPosition(sp);
  }
  Update();
}

void NavigatorList::GotoEditor(void)
{
  ChildFrame *cf = (ChildFrame *)GetMf()->MDIGetActive();
  if(cf)
  {
    cf->m_view->SetFocus();
  }
}

void NavigatorList::OnRButtonDown(UINT flags, CPoint point)
{
   ClientToScreen(&point);
   DoViewPopupMenu(&point);
}

void NavigatorList::PopupMenu(void)
{
  POINT p = {10, 10};
  ClientToScreen(&p);
  DoViewPopupMenu(&p);
}

void NavigatorList::DoViewPopupMenu(POINT *p)
{
  if(m_dlg->m_navBarState == NavBarStateDisabled)
    return;
  if(m_dlg->m_navBarState == NavBarStateDir)
  {
    m_dlg->m_hdDir.DoPopupMenu(p);
    m_dlg->ReReadDir();
  }
  else
  {
    CMenu bar;
    bar.LoadMenu(IDL_POPUP_MENU);
    CMenu &popup = *bar.GetSubMenu(0);
    ASSERT(popup.m_hMenu != NULL);

    int pos;
    for(pos = popup.GetMenuItemCount() - 1; pos >= 0; pos--)
      popup.DeleteMenu(pos, MF_BYPOSITION);

    for(pos = 0; PopupMenuList[m_dlg->m_navBarState][pos].m_id != 0; pos++)
    {
      popup.AppendMenu(MF_STRING, PopupMenuList[m_dlg->m_navBarState][pos].m_id, PopupMenuList[m_dlg->m_navBarState][pos].m_text.c_str());
    }

    popup.TrackPopupMenu(TPM_LEFTALIGN, p->x, p->y, this);
  }
}

char *NavigatorList::LimitText(const char *str, int len, CDC *dc)
{
  static char temp[MAX_PATH];
  static char *tree_dots = "...";
  int slen = strlen(str);

  len -= dc->GetTextExtent("W", 1).cx;
  if(!slen || dc->GetTextExtent(str, slen).cx < len)
    return (char *)str;

  if(slen < 4)
    return tree_dots;

  strcpy(temp, str);
  len -= dc->GetTextExtent(tree_dots, 3).cx;
  while(slen > 4 && dc->GetTextExtent(str, slen).cx > len)
  {
    strcpy(&temp[3], &temp[4]);
    slen--;
  }
  if(dc->GetTextExtent(str, slen).cx > len)
    return tree_dots;
  memmove(&temp[6], &temp[3], strlen(&temp[3]) + 1);
  temp[3] = temp[4] = temp[5] = '.';

  return temp;
}


BOOL NavigatorList::OnEraseBkgnd(CDC *dc)
{
  RECT cr;
  GetClientRect(&cr);
  CBrush Br(wainApp.gs.m_listColorBack);
  dc->FillRect(&cr, &Br);
  Br.DeleteObject();
  return TRUE;
}

NavigatorListColorSetupDialogClass::NavigatorListColorSetupDialogClass(CWnd *parent) : CDialog(NavigatorListColorSetupDialogClass::IDD, parent)
{

}

NavigatorListColorSetupDialogClass::~NavigatorListColorSetupDialogClass()
{

}

BOOL NavigatorListColorSetupDialogClass::OnInitDialog(void)
{
  RECT er = {110, 5, 195, 50 + 5};
  MapDialogRect(&er);
  m_colorEdit.m_font = m_font;
  m_colorEdit.m_dlg = this;
  m_colorEdit.Create(WS_VISIBLE | ES_MULTILINE, er, this, IDC_BLC_COLOR_SAMPLE);

  CSpinButtonCtrl *sb = (CSpinButtonCtrl *)GetDlgItem(IDC_BLC_RED_SPIN);
  ASSERT(sb);
  sb->SetRange(0, 255);
  sb = (CSpinButtonCtrl *)GetDlgItem(IDC_BLC_GREEN_SPIN);
  ASSERT(sb);
  sb->SetRange(0, 255);
  sb = (CSpinButtonCtrl *)GetDlgItem(IDC_BLC_BLUE_SPIN);
  ASSERT(sb);
  sb->SetRange(0, 255);

  CListBox *list = (CListBox *)GetDlgItem(IDC_BLC_LIST);
  ASSERT(list);
  list->AddString("Background");
  list->AddString("Text");
  list->AddString("Selected Text");
  list->AddString("Selected Background");
  list->AddString("Selected Background Focus");
  m_sel = 0;
  list->SetCurSel(m_sel);
  NewSel();
  return CDialog::OnInitDialog();
}

void NavigatorListColorSetupDialogClass::NewSel(void)
{
  CListBox *list = (CListBox *)GetDlgItem(IDC_BLC_LIST);
  ASSERT(list);
  m_sel = list->GetCurSel();
  if(m_sel != LB_ERR)
  {
    unsigned int *p;
    switch(m_sel)
    {
      case 0:
        p = &m_listColorBack;
        break;
      case 1:
        p = &m_listColorText;
        break;
      case 2:
        p = &m_listColorTextSel;
        break;
      case 3:
        p = &m_listColorSel;
        break;
      case 4:
        p = &m_listColorSelFocus;
        break;
      default:
        return;
    }
    char buf[32];
    sprintf(buf, "%d", (*p) & 0xFF);
    CEdit *e = (CEdit *)GetDlgItem(IDC_BLC_RED_EDIT);
    ASSERT(e);
    e->SetWindowText(buf);

    sprintf(buf, "%d", ((*p) >> 8) & 0xFF);
    e = (CEdit *)GetDlgItem(IDC_BLC_GREEN_EDIT);
    ASSERT(e);
    e->SetWindowText(buf);

    sprintf(buf, "%d", ((*p) >> 16) & 0xFF);
    e = (CEdit *)GetDlgItem(IDC_BLC_BLUE_EDIT);
    ASSERT(e);
    e->SetWindowText(buf);
  }
}

void NavigatorListColorSetupDialogClass::DoDataExchange(CDataExchange *dx)
{
  CDialog::DoDataExchange(dx);
}
// #define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
void NavigatorListColorSetupDialogClass::BlueChanged(void)
{
  CListBox *list = (CListBox *)GetDlgItem(IDC_BLC_LIST);
  ASSERT(list);
  m_sel = list->GetCurSel();
  if(m_sel != LB_ERR)
  {
    unsigned int *p;
    switch(m_sel)
    {
      case 0:
        p = &m_listColorBack;
        break;
      case 1:
        p = &m_listColorText;
        break;
      case 2:
        p = &m_listColorTextSel;
        break;
      case 3:
        p = &m_listColorSel;
        break;
      case 4:
        p = &m_listColorSelFocus;
        break;
      default:
        return;
    }
    CEdit *e = (CEdit *)GetDlgItem(IDC_BLC_BLUE_EDIT);
    ASSERT(e);
    char buf[32];
    e->GetWindowText(buf, 32);
    int b = strtol(buf, NULL, 10);
    if(b < 0)
      b = 0;
    if(b > 255)
      b = 255;
    *p = (*p & 0xFFFF) | (b << 16);
    m_colorEdit.InvalidateRect(NULL, TRUE);
  }
}

void NavigatorListColorSetupDialogClass::GreenChanged(void)
{
  CListBox *list = (CListBox *)GetDlgItem(IDC_BLC_LIST);
  ASSERT(list);
  m_sel = list->GetCurSel();
  if(m_sel != LB_ERR)
  {
    unsigned int *p;
    switch(m_sel)
    {
      case 0:
        p = &m_listColorBack;
        break;
      case 1:
        p = &m_listColorText;
        break;
      case 2:
        p = &m_listColorTextSel;
        break;
      case 3:
        p = &m_listColorSel;
        break;
      case 4:
        p = &m_listColorSelFocus;
        break;
      default:
        return;
    }
    CEdit *e = (CEdit *)GetDlgItem(IDC_BLC_GREEN_EDIT);
    ASSERT(e);
    char buf[32];
    e->GetWindowText(buf, 32);
    int b = strtol(buf, NULL, 10);
    if(b < 0)
      b = 0;
    if(b > 255)
      b = 255;
    *p = (*p & 0xFF00FF) | (b << 8);
    m_colorEdit.InvalidateRect(NULL, TRUE);
  }
}

void NavigatorListColorSetupDialogClass::RedChanged(void)
{
  CListBox *list = (CListBox *)GetDlgItem(IDC_BLC_LIST);
  ASSERT(list);
  m_sel = list->GetCurSel();
  if(m_sel != LB_ERR)
  {
    unsigned int *p;
    switch(m_sel)
    {
      case 0:
        p = &m_listColorBack;
        break;
      case 1:
        p = &m_listColorText;
        break;
      case 2:
        p = &m_listColorTextSel;
        break;
      case 3:
        p = &m_listColorSel;
        break;
      case 4:
        p = &m_listColorSelFocus;
        break;
      default:
        return;
    }
    CEdit *e = (CEdit *)GetDlgItem(IDC_BLC_RED_EDIT);
    ASSERT(e);
    char buf[32];
    e->GetWindowText(buf, 32);
    int b = strtol(buf, NULL, 10);
    if(b < 0)
      b = 0;
    if(b > 255)
      b = 255;
    *p = (*p & 0xFFFF00) | (b << 0);
    m_colorEdit.InvalidateRect(NULL, TRUE);
  }
}

void NavigatorListColorSetupDialogClass::OnOK()
{
  EndDialog(IDOK);
}

BEGIN_MESSAGE_MAP(NavigatorListColorSetupDialogClass, CDialog)
  ON_EN_CHANGE(IDC_BLC_BLUE_EDIT,  BlueChanged)
  ON_EN_CHANGE(IDC_BLC_RED_EDIT,   RedChanged)
  ON_EN_CHANGE(IDC_BLC_GREEN_EDIT, GreenChanged)
  ON_LBN_SELCHANGE(IDC_BLC_LIST,   NewSel)
END_MESSAGE_MAP();

IMPLEMENT_DYNAMIC(NavigatorListColorSetupDialogClass, CDialog)

void NavigatorList::ColorSetup(void)
{
  NavigatorListColorSetupDialogClass csd(this);
  csd.m_listColorBack = wainApp.gs.m_listColorBack;
  csd.m_listColorText = wainApp.gs.m_listColorText;
  csd.m_listColorSel = wainApp.gs.m_listColorSel;
  csd.m_listColorSelFocus = wainApp.gs.m_listColorSelFocus;
  csd.m_listColorTextSel = wainApp.gs.m_listColorTextSel;
  csd.m_font = m_navigatorListFont;

  if(csd.DoModal() == IDOK)
  {
    wainApp.gs.m_listColorBack = csd.m_listColorBack;
    wainApp.gs.m_listColorText = csd.m_listColorText;
    wainApp.gs.m_listColorSel = csd.m_listColorSel;
    wainApp.gs.m_listColorSelFocus = csd.m_listColorSelFocus;
    wainApp.gs.m_listColorTextSel = csd.m_listColorTextSel;
    InvalidateRect(NULL, TRUE);
  }
}

BEGIN_MESSAGE_MAP(NavigatorListColorEditClass, CEdit)
  ON_WM_PAINT()
END_MESSAGE_MAP();

NavigatorListColorEditClass::NavigatorListColorEditClass()
{
   m_init = FALSE;
}

void NavigatorListColorEditClass::OnPaint(void)
{
  CPaintDC dc(this);
  RECT cr;
  GetClientRect(&cr);
  CBrush Br(m_dlg->m_listColorBack);
  dc.FillRect(&cr, &Br);
  Br.DeleteObject();
  CFont *old_font = dc.SelectObject(m_font);

  TEXTMETRIC tm;
  dc.GetOutputTextMetrics(&tm);
  int LineHeight;
  if (tm.tmHeight < 0)
    LineHeight = -tm.tmHeight;
  else
    LineHeight = (tm.tmHeight - tm.tmInternalLeading);
  LineHeight += 3;

  ASSERT(m_dlg);
  dc.SetBkColor(m_dlg->m_listColorBack);
  dc.SetTextColor(m_dlg->m_listColorText);
  dc.TextOut(5, 0, "Normal Text", strlen("Normal Text"));

  dc.SetBkColor(m_dlg->m_listColorSel);
  dc.SetTextColor(m_dlg->m_listColorTextSel);
  dc.TextOut(5, LineHeight, "Selected Text", strlen("Selected Text"));

  dc.SetBkColor(m_dlg->m_listColorSelFocus);
  dc.SetTextColor(m_dlg->m_listColorTextSel);
  dc.TextOut(5, 2*LineHeight, "Selected Text Focus", strlen("Selected Text Focus"));

  dc.SelectObject(old_font);
}

void NavigatorList::OpenAsTagPeek(void)
{
  m_dlg->OpenAsTagPeek();
}

void NavigatorList::OnChar(UINT ch, UINT repeat_count, UINT flags)
{
  SetStatusText("On char: %c", (char )ch);
  m_dlg->AddSearchChar((char )tolower(*(unsigned char *)&ch));
}

void NavigatorList::MoveBack(void)
{
  m_dlg->AddSearchChar(-1);
}


