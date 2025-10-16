#include ".\..\src\stdafx.h"
#include ".\..\src\Wain.h"
#include ".\..\src\MainFrm.h"
#include ".\..\src\ChildFrm.h"
#include ".\..\src\TabView.h"
#include ".\..\src\wainview.h"
#include ".\..\src\waindoc.h"
#include "docprop.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

TabViewClass::TabViewClass(class WainView *view)
{
  ASSERT(view);
  m_wainView = view;
  for(int i = 0; i < NOF_BOOKMARKS; i++)
    m_bookmark[i] = -1;
}

void TabViewClass::OnDraw(CDC *dc)
{
  dc;
}

void TabViewClass::SetBookmark(int nr, int LineNo)
{
  m_bookmark[nr] = LineNo;
  InvalidateRect(NULL, TRUE);
}

void TabViewClass::OnPaint(void)
{
  CPaintDC dc(this);
  RECT cr;
  GetClientRect(&cr);
  ASSERT(m_wainView);
  CFont *old_font = dc.SelectObject(TextWinFont);
  int i;
  if(m_wainView->m_lineHeight == 0)
    return;
  int y_off = m_wainView->m_yOffset;
  int first = y_off/m_wainView->m_lineHeight;
  if(first)
    first--;
  int last = first + cr.bottom/m_wainView->m_lineHeight;
  last += 2;
  char temp[16];
  WainDoc *doc = m_wainView->GetDocument();
  if(last > doc->m_lineCount)
    last = doc->m_lineCount;
  TxtLine *l = doc->GetLineNo(first);

  COLORREF old_color = dc.SetBkColor(RGB(255, 255, 255));
  COLORREF old_txt_color = dc.SetTextColor(RGB(0, 0, 0));
  int k;
  BOOL found;
  dc.SetBkMode(TRANSPARENT);
  for(i = first; i < last; i++)
  {
    GlobalSettingsClass::TVCColorIndexType ColorIdx = GlobalSettingsClass::TVC_NORMAL;
    for(k = 0, found = FALSE; k < NOF_BOOKMARKS && !found; k += found ? 0 : 1)
      if(m_bookmark[k] == i)
        found = TRUE;
    if(found)
    {
      ColorIdx = GlobalSettingsClass::TVC_BOOKMARK;
      sprintf(temp, "%d    ", k);
    }
    else
    {
      if(l && l->m_modifyStatus == IS_MODIFIED)
        ColorIdx = GlobalSettingsClass::TVC_CHANGED;
      else if(l && l->m_modifyStatus == MODIFIED_SAVED)
        ColorIdx = GlobalSettingsClass::TVC_CHANGED_SAVED;
      sprintf(temp, "%5d", i + 1);
    }
    dc.SetTextColor(wainApp.gs.m_tvcColor[ColorIdx].m_textColor);
    RECT R;
    R.left = 0;
    R.top = i*m_wainView->m_lineHeight - y_off;
    R.bottom = R.top + m_wainView->m_lineHeight;
    R.right = cr.right;
    dc.FillSolidRect(&R, wainApp.gs.m_tvcColor[ColorIdx].m_backColor);
    TextOut(dc.m_hDC, 5, i*m_wainView->m_lineHeight - y_off, temp, strlen(temp));
    if(l)
      l = doc->GetNext(l);
  }
  dc.SetBkColor(old_color);
  dc.SetTextColor(old_txt_color);
  dc.SelectObject(old_font);
}

const char * const ColorStr[] =
{
  "Normal",
  "Changed",
  "Changed Saved",
  "Bookmark"
};

void MainFrame::OnColor()
{
  ColorType OrgColor[GlobalSettingsClass::TVC_NUM_COLOR];
  memcpy(OrgColor, wainApp.gs.m_tvcColor, sizeof(OrgColor));

  CPropertySheet setup("TabView Color", this);

  TabViewColorDialogClass color(OrgColor, GlobalSettingsClass::TVC_NUM_COLOR, ColorStr);
  setup.AddPage((CPropertyPage *)&color);
  setup.m_psh.dwFlags |= PSH_NOAPPLYNOW;

  if(setup.DoModal() == IDOK)
  {
    memcpy(wainApp.gs.m_tvcColor, OrgColor, sizeof(wainApp.gs.m_tvcColor));
    UpdateViews(TRUE);
  }
}

BEGIN_MESSAGE_MAP(TabViewClass, CView)
  ON_WM_PAINT()
END_MESSAGE_MAP();

//--- EOF
