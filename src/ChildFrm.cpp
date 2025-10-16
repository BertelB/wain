//=============================================================================
// This source code file is a part of Wain.
// It implements the child_frame_class
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\wain.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\ChildFrm.h"
#include ".\..\src\tabview.h"
#include ".\..\src\waindoc.h"
#include ".\..\src\wainview.h"
#include ".\..\src\WainUtil.h"
// Git check
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(ChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(ChildFrame, CMDIChildWnd)
  ON_WM_SIZE()
  ON_WM_PAINT()
  ON_WM_CLOSE()
  ON_WM_SYSCOMMAND()
END_MESSAGE_MAP();

void ChildFrame::OnSize(UINT aType, int aWidth, int aHeigth)
{
   RECT cr;
   if(m_first)
   {
      ASSERT(m_doc);
      POSITION pos = m_doc->GetFirstViewPosition();
      m_view = (WainView *)m_doc->GetNextView(pos);
      ASSERT(m_view);
      m_first = FALSE;
      GetClientRect(&cr);
      cr.right = cr.left + 50;
      if(wainApp.gs.m_showLineNo)
      {
         ShowLineNo(true);
      }
   }

   if(aType == SIZE_MAXIMIZED)
   {
      m_displayMode = CreateModeType::CREATE_MAX;
   }
   else
   {
      m_displayMode = wainApp.gs.m_createMode;
   }

   CMDIChildWnd::OnSize(aType, aWidth, aHeigth);
   SetTabViewSize();
}

void ChildFrame::SetTabViewSize(void)
{
   RECT cr;
   if(m_tabView)
   {
      GetClientRect(&cr);
      ASSERT(m_view);
      int w = 10 + m_view->m_charWidth * 6;
      if(m_view->m_charWidth == 0)
      {
         CDC *dc = GetDC();
         CFont *old_font = dc->SelectObject(TextWinFont);
         TEXTMETRIC tm;
         dc->GetOutputTextMetrics(&tm);
         int CharWidth = tm.tmAveCharWidth < 0 ? -tm.tmAveCharWidth : tm.tmAveCharWidth;
         w = 10 + CharWidth * 6;
         dc->SelectObject(old_font);
         ReleaseDC(dc);
      }
      cr.left += w;
      m_view->MoveWindow(&cr, TRUE);
      GetClientRect(&cr);
      cr.right = cr.left + w;
      m_tabView->MoveWindow(&cr, TRUE);
   }
   else
   {
      GetClientRect(&cr);
      ASSERT(m_view);
      m_view->MoveWindow(&cr, TRUE);
   }
}

void ChildFrame::ShowLineNo(bool aShow)
{
   if(aShow && !m_tabView)
   {
      RECT cr;
      GetClientRect(&cr);
      cr.right = cr.left + 50;
      m_tabView = new TabViewClass(m_view);
      m_tabView->Create(NULL, NULL, WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, cr, this, IDV_TAB_VIEW, NULL);
      m_view->m_tabView = m_tabView;
   }
   else if(!aShow && m_tabView)
   {
      m_view->m_tabView = NULL;
      delete m_tabView;
      m_tabView = NULL;
   }
   SetTabViewSize();
}

ChildFrame::ChildFrame()
{
  m_view = 0;
  m_doc = 0;
  m_first = true;
  m_tabView = 0;
}

BOOL ChildFrame::Create(LPCTSTR aClassName, LPCTSTR aWindowName, DWORD aStyle, const RECT &aRect, CMDIFrameWnd *aParentWnd, CCreateContext *aContext)
{
  RECT NewRect;
  m_position = GetMf()->GetNewWinRect(&NewRect);
  BOOL ret;
  switch(wainApp.gs.m_createMode)
  {
    case CreateModeType::CREATE_DEFAULT:
      ret = CMDIChildWnd::Create(aClassName, aWindowName, aStyle, aRect, aParentWnd, aContext);
      m_displayMode = CreateModeType::CREATE_DEFAULT;
      break;
    case CreateModeType::CREATE_MAX:
      ret = CMDIChildWnd::Create(aClassName, aWindowName, aStyle | WS_VISIBLE | WS_MAXIMIZE, aRect, aParentWnd, aContext);
      m_displayMode = CreateModeType::CREATE_MAX;
      break;
    case CreateModeType::CREATE_DUAL:
    default:
      ret = CMDIChildWnd::Create(aClassName, aWindowName, aStyle, NewRect, aParentWnd, aContext);
      m_displayMode = CreateModeType::CREATE_DUAL;
      break;
  }
  return ret;
}

ChildFrame::~ChildFrame()
{
}

BOOL ChildFrame::PreCreateWindow(CREATESTRUCT &cs)
{
  cs.style &= ~FWS_ADDTOTITLE;

  if(m_doc)
  {
    POSITION pos = m_doc->GetFirstViewPosition();
    m_view = (WainView *)m_doc->GetNextView(pos);
  }
  return CMDIChildWnd::PreCreateWindow(cs);
}

void ChildFrame::OnPaint(void)
{
   CMDIChildWnd::OnPaint();
}

void ChildFrame::OnClose(void)
{
  GetMf()->CloseNormalDoc(this);
}

void ChildFrame::OnSysCommand(UINT id, LPARAM l_param)
{
  if((id & 0xFFF0) == SC_MAXIMIZE)
  {
    wainApp.gs.m_createMode = CreateModeType::CREATE_MAX;
  }
  else if((id & 0xFFF0) == SC_RESTORE)
    wainApp.gs.m_createMode = CreateModeType::CREATE_DEFAULT;

  CMDIChildWnd::OnSysCommand(id, l_param);
}

#ifdef _DEBUG
void ChildFrame::AssertValid() const
{
  CMDIChildWnd::AssertValid();
}

void ChildFrame::Dump(CDumpContext& dc) const
{
  CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

