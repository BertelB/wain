//=============================================================================
// This source code file is a part of Wain.
// It implements various printer functions.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\wain.h"

#include ".\..\src\WainDoc.h"
#include ".\..\src\WainView.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\taglist.h"
#include ".\..\src\tabview.h"
#include ".\..\src\childfrm.h"
#include "DocProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#pragma warning (disable : 4100) // unreferenced formal parameter
static char EmptyLine[] = "                                                                              ";

class PrintSetupDialogClass : public DialogBaseClass
{
  LOGFONT m_lf;
  bool m_print_bw;
  bool m_print_doc;
  bool m_print_spec;
  CString m_header;
  CString m_footer;
  bool    m_lineNo;
  bool    m_wrap;
  int     m_left_margin;
  int m_printScale;
  CString m_left_margin_str;
public:
  PrintSetupDialogClass(CWnd *parent);
  ~PrintSetupDialogClass();
  virtual void DoDataExchange(CDataExchange *dx);
  virtual BOOL OnInitDialog();
  afx_msg void OnOK(void);
  afx_msg void OnCancel(void);
  afx_msg void SelectFont(void);
  afx_msg void SelectColor(void);
  DECLARE_MESSAGE_MAP();
};

void WainView::OnPrepareDC(CDC *pDC, CPrintInfo *print_info)
{
  if(!m_firstPage)
  {
    print_info->m_bContinuePrinting = FALSE;
    return;
  }
  CView::OnPrepareDC(pDC, print_info);
}

BOOL WainView::OnPreparePrinting(CPrintInfo *print_info)
{
  print_info->m_pPD->m_pd.Flags |= PD_DISABLEPRINTTOFILE | PD_HIDEPRINTTOFILE | PD_NOPAGENUMS;
  if(m_markType != NO_MARK)
  {
    print_info->m_pPD->m_pd.Flags |= PD_SELECTION;
    print_info->m_pPD->m_pd.Flags &= ~PD_NOSELECTION;
  }
  PrintSetupDialogClass ps(this);
  if(ps.DoModal() == IDCANCEL)
    return 0;

  return DoPreparePrinting(print_info);
}

void WainView::OnBeginPrinting(CDC *dc, CPrintInfo *print_info)
{
  CView::OnBeginPrinting(dc, print_info);
}

void WainView::OnEndPrinting(CDC *dc, CPrintInfo *print_info)
{
   CView::OnEndPrinting(dc, print_info);
}

void WainView::OnPrint(CDC *dc, CPrintInfo *print_info)
{
   int prev_map_mode = dc->SetMapMode(MM_TEXT);
   WainDoc *doc = GetDocument();
   LOGFONT lf = wainApp.gs.m_printerFont;

   // map to printer font metrics
   HDC hDCFrom = ::GetDC(NULL);
   lf.lfHeight = ::MulDiv(lf.lfHeight, dc->GetDeviceCaps(LOGPIXELSY), ::GetDeviceCaps(hDCFrom, LOGPIXELSY));
   lf.lfWidth = ::MulDiv(lf.lfWidth, dc->GetDeviceCaps(LOGPIXELSX), ::GetDeviceCaps(hDCFrom, LOGPIXELSX));
   ::ReleaseDC(NULL, hDCFrom);

   lf.lfHeight = lf.lfHeight*wainApp.gs.m_printScale/100;
   lf.lfWidth = lf.lfWidth*wainApp.gs.m_printScale/100;

   CFont PrinterFont;
   PrinterFont.CreateFontIndirect(&lf);

   CFont *old_font = dc->SelectObject(&PrinterFont);
   TEXTMETRIC tm;
   dc->GetOutputTextMetrics(&tm);
   int LineHeighteight;
   int CharWidthidth;

   if (tm.tmHeight < 0)
      LineHeighteight = -tm.tmHeight;
   else
      LineHeighteight = tm.tmHeight;

   LineHeighteight += tm.tmInternalLeading ? 0 : 2;
   CharWidthidth = tm.tmAveCharWidth < 0 ? -tm.tmAveCharWidth : tm.tmAveCharWidth;
   //LineHeighteight = LineHeighteight*wainApp.gs.PrintScale/100;
   //CharWidthidth = CharWidthidth*wainApp.gs.PrintScale/100;

   print_info->m_rectDraw.SetRect(0, 0, dc->GetDeviceCaps(HORZRES), dc->GetDeviceCaps(VERTRES));
   dc->DPtoLP(&print_info->m_rectDraw);
   dc->IntersectClipRect(print_info->m_rectDraw);
   dc->SetBkMode(TRANSPARENT);

   int first_line, last_line;
   if(print_info->m_pPD->m_pd.Flags & PD_SELECTION)
   {
      first_line = m_markStartY;
      last_line = m_markEndY;
   }
   else
   {
      first_line = 0;
      last_line = doc->m_lineCount;
   }

   int lines_pr_page = dc->GetDeviceCaps(VERTRES)/LineHeighteight;
   int YOffset = 0;
   if(wainApp.gs.m_printHeader.GetLength() != 0)
   {
      lines_pr_page -= 2; /* Header */
      YOffset = 2;
   }
   if(wainApp.gs.m_printFooter.GetLength() != 0)
      lines_pr_page -= 2; /* Footer */

   int page_width = dc->GetDeviceCaps(HORZSIZE);
   int margin_points = dc->GetDeviceCaps(HORZRES)*wainApp.gs.m_printMargin/page_width;
   margin_points = ((margin_points + (CharWidthidth - 1))/CharWidthidth)*CharWidthidth;
   int max_line;
   int i, j;

   for(i = 0, j = 10; j < last_line; i++, j *= 10)
   { /* Just counting */ }
   max_line = i + 1;

   int nof_lines;
   int char_pr_line = 0x7FFFFFFF;
   if(wainApp.gs.m_printWrap)
   {
      char_pr_line = dc->GetDeviceCaps(HORZRES)/CharWidthidth;
      char_pr_line -= margin_points/CharWidthidth;
      if(wainApp.gs.m_printLineNo)
         char_pr_line -= (max_line + 2);
      nof_lines = CalculateNofLines(first_line, last_line, char_pr_line, lines_pr_page);
   }
   else /* Just a long line to make wain wrap here                                1234567890123456 */
   {
      nof_lines = last_line - first_line + 1;
   }

   int nof_pages = (nof_lines + (lines_pr_page - 1))/lines_pr_page;
   int cl = first_line;
   TxtLine *l = doc->GetLineNo(first_line);

   for(int page_counter = 1; page_counter <= nof_pages && l; page_counter++)
   {
      if(wainApp.gs.m_printHeader.GetLength() != 0)
      {
         PrintHeader(dc, CharWidthidth, LineHeighteight, page_counter, nof_pages, margin_points, wainApp.gs.m_printHeader, TRUE);
      }
      for(i = 1; l != 0 && i <= lines_pr_page && cl <= last_line; i++)
      {
         i += PrintText(dc, l, i + YOffset, CharWidthidth, LineHeighteight, ++cl, max_line, margin_points);
         l = doc->GetNext(l);
         if(l && wainApp.gs.m_printWrap && l->GetTabLen() > char_pr_line)
         {
            j = l->GetTabLen()/char_pr_line;
            if(l->GetTabLen()%char_pr_line)
               j++;
            if(i + j > lines_pr_page && j < lines_pr_page)
               i += j; /* Start a new page */
         }
      }
      if(wainApp.gs.m_printFooter.GetLength() != 0)
      {
         PrintHeader(dc, CharWidthidth, LineHeighteight, page_counter, nof_pages, margin_points, wainApp.gs.m_printFooter, FALSE);
      }

      if(page_counter != nof_pages)
         dc->EndPage();
   }
   dc->SelectObject(old_font);
   dc->SetMapMode(prev_map_mode);

   m_firstPage = FALSE;
}

void WainView::PrintHeader(CDC *dc, int CharWidthidth, int LineHeighteight, int page_no,
                                   int nof_pages, int left_margin, const char *format, BOOL is_header)
{
  char *f_buf = (char *)malloc(strlen(format));  // Fixme-> sts::string
  char *p_buf = (char *)malloc(1024);            // Fixme-> sts::string
  const char *s;
  int i, j, state = 0;
  BOOL underline = FALSE;
  char time_str[256];
  time_t aclock;
  struct tm *newtime;

  while(*format && state < 3)
  {
    *p_buf = 0;
    s = strstr(format, "\\t");
    if(s)
    {
      for(i = 0; format != s; format++)
        f_buf[i++] = *format;
      f_buf[i] = 0;
      format = s + 2;
    }
    else
    {
      strcpy(f_buf, format);
      while(*format)
        format++;
    }

    if(strlen(f_buf))
    {
      for(i = 0; f_buf[i]; i++)
      {
        if(f_buf[i] == '\\')
        {
          i++;
          switch(f_buf[i])
          {
            case 'T':
              time(&aclock);
              newtime = localtime(&aclock);
              strftime(time_str, sizeof(time_str), wainApp.gs.m_timeFormat.c_str(), newtime);
              strcat(p_buf, time_str);
              break;
            case 'D':
              time(&aclock);
              newtime = localtime(&aclock);
              strftime(time_str, sizeof(time_str), wainApp.gs.m_dateFormat.c_str(), newtime);
              strcat(p_buf, time_str);
              break;
            case 'S':
              strcat(p_buf, wainApp.gs.m_timeSep.c_str());
              break;
            case 'U':
            {
              std::string ui;
              ui = wainApp.gs.m_userId;
              strcat(p_buf, ui.c_str());
              break;
            }
            case 'P':
              sprintf(time_str, "%d", page_no);
              strcat(p_buf, time_str);
              break;
            case 'C':
              sprintf(time_str, "%d", nof_pages);
              strcat(p_buf, time_str);
              break;
            case 'F':
              strcat(p_buf, GetDocument()->GetPathName());
              break;
            case 'f':
              MySplitPath(GetDocument()->GetPathName(), SP_FILE | SP_EXT, time_str);
              strcat(p_buf, time_str);
              break;
            case 'o':
              underline = !underline;
              break;
            default:
              j = strlen(p_buf);
              p_buf[j++] = f_buf[i];
              p_buf[j] = 0;
              break;
          }
        }
        else
        {
          j = strlen(p_buf);
          p_buf[j++] = f_buf[i];
          p_buf[j] = 0;
        }
      }

      int nof_chars = dc->GetDeviceCaps(HORZRES)/CharWidthidth;
      int y_pos = is_header ? LineHeighteight : (dc->GetDeviceCaps(VERTRES) - LineHeighteight);

      dc->SetTextColor(RGB(0, 0, 0));
      if(state == 0)
      { /* Left */
        dc->TextOut(left_margin, y_pos, p_buf, strlen(p_buf));
      }
      else if(state == 1)
      { /* Mid */
        int p_pos = nof_chars/2 - strlen(p_buf)/2;
        p_pos += left_margin/(2*CharWidthidth);
        dc->TextOut(CharWidthidth*p_pos, y_pos, p_buf, strlen(p_buf));
      }
      else
      { /* Right */
        int p_pos = nof_chars - strlen(p_buf);
        dc->TextOut(p_pos*CharWidthidth, y_pos, p_buf, strlen(p_buf));
      }
    }
    state++;
  }

  CPen pen(PS_SOLID, 2, RGB(0, 0, 0));
  CPen *old_pen = dc->SelectObject(&pen);
  if(is_header)
  {
    dc->MoveTo(left_margin, 2*LineHeighteight);
    dc->LineTo((dc->GetDeviceCaps(HORZRES)/CharWidthidth)*CharWidthidth, 2*LineHeighteight);
  }
  else
  {
    int yy = dc->GetDeviceCaps(VERTRES) - LineHeighteight - (LineHeighteight/4);
    dc->MoveTo(left_margin, yy);
    dc->LineTo((dc->GetDeviceCaps(HORZRES)/CharWidthidth)*CharWidthidth, yy);
  }
  dc->SelectObject(old_pen);
  free(p_buf);
  free(f_buf);
}

int WainView::PrintText(CDC *dc, TxtLine *line, int y, int CharWidthidth, int LineHeighteight, int cl, int max_line, int left_margin)
{
  #define Y_OFFSET ((y + line_offset)*LineHeighteight)
  int i, j;
  int chars_pr_line = 0x7FFFFFFF;
  int line_offset = 0;

  if(wainApp.gs.m_printWrap)
  {
    chars_pr_line =  dc->GetDeviceCaps(HORZRES)/CharWidthidth;
    if(wainApp.gs.m_printLineNo)
      chars_pr_line -= (max_line + 2);
  }

  WainDoc *doc = GetDocument();
  TxtLineColor col;

  EndLineStatusType prev_els = doc->GetPrevEls(line);
  ColorType *c_color;

  if(wainApp.gs.m_printColor == PC_SPECIFIED)
    c_color = wainApp.gs.m_printingColor;
  else
    c_color = doc->m_prop->m_color;

  line->m_endLineStatus = line->GetTextColor(col, prev_els);

  int text_pos;
  const char *t = line->GetText();
  int text_len = line->GetTabLen();

  if(wainApp.gs.m_printColor == PC_BLACK_WHITE)
    dc->SetBkColor(RGB(255, 255, 255));
  else
    dc->SetBkColor(c_color[TEXT_IDX].m_backColor);

  text_pos = left_margin;

  if(wainApp.gs.m_printLineNo)
  {
    char LineNo_str[32];
    sprintf(LineNo_str, "%*d: ", max_line, cl);
    dc->SetTextColor(RGB(0, 0, 0));
    ::TextOut(dc->m_hDC, text_pos, Y_OFFSET, LineNo_str, strlen(LineNo_str));
    text_pos += CharWidthidth*(max_line + 2);
  }

  for(i = 0, j = 0; i < text_len; i += col[j].m_len, j++)
  {
    if(col[j].m_index == TAB_IDX)
    {
      if(wainApp.gs.m_printColor == PC_BLACK_WHITE)
        dc->SetTextColor(RGB(0, 0, 0));
      else
        dc->SetTextColor(c_color[TEXT_IDX].m_textColor);
      ::TextOut(dc->m_hDC, text_pos, Y_OFFSET, EmptyLine, col[j].m_len);
      t++;
      text_pos += col[j].m_len*CharWidthidth;
    }
    else
    {
      if(wainApp.gs.m_printColor == PC_BLACK_WHITE)
        dc->SetTextColor(RGB(0, 0, 0));
      else
        dc->SetTextColor(c_color[col[j].m_index].m_textColor);

      if(text_pos/CharWidthidth + col[j].m_len > chars_pr_line)
      { /* Wrap */
        int pl = chars_pr_line - (text_pos/CharWidthidth);
        ::TextOut(dc->m_hDC, text_pos, Y_OFFSET, t, pl);
        text_pos = left_margin;
        if(wainApp.gs.m_printLineNo)
          text_pos += CharWidthidth*(max_line + 2);
        line_offset++;

        ::TextOut(dc->m_hDC, text_pos, Y_OFFSET, t + pl, col[j].m_len - pl);
        t += col[j].m_len;
        text_pos += (col[j].m_len - pl)*CharWidthidth;
      }
      else
      { /* Don't wrap */
        ::TextOut(dc->m_hDC, text_pos, Y_OFFSET, t, col[j].m_len);
        t += col[j].m_len;
        text_pos += col[j].m_len*CharWidthidth;
      }
    }

  }
  dc->SetTextColor(c_color[TEXT_IDX].m_textColor);
  dc->SetBkColor(c_color[TEXT_IDX].m_backColor);
  #undef VI_TAB
  #undef DRAW_LINE
  #undef Y_OFFSET

  return line_offset; /* I.E. how many additional lines did we make */
}

int WainView::CalculateNofLines(int first_line, int last_line, int char_pr_line, int lines_pr_page)
{
  int i, j, k;
  int LineCount = 0;
  int curr_page_count = 0;
  TxtLine *line;
  WainDoc *doc = GetDocument();

  for(i = first_line, line = doc->GetLineNo(first_line); i <= last_line && line; i++, line = doc->GetNext(line))
  {
    j = line->GetTabLen();
    if(j == 0)
    {
      k = 1;
    }
    else
    {
      k = j/char_pr_line;
      if(j%char_pr_line)
        k++;
    }
    if(k + curr_page_count > lines_pr_page)
    { /* We will have to make a new page */
      LineCount += k;
      LineCount += lines_pr_page - curr_page_count;
      curr_page_count = 0;
    }
    else
    {
      LineCount += k;
      curr_page_count += k;
    }
  }
  return LineCount;
}

PrintSetupDialogClass::PrintSetupDialogClass(CWnd *parent) : DialogBaseClass(IDC_PRINT_SETUP, parent)
{
   m_lf = wainApp.gs.m_printerFont;

   if(wainApp.gs.m_printColor == PC_BLACK_WHITE)
   {
     m_print_bw = TRUE;
     m_print_doc = FALSE;
     m_print_spec = FALSE;
   }
   else if(wainApp.gs.m_printColor == PC_DOCUMENT)
   {
     m_print_bw = FALSE;
     m_print_doc = TRUE;
     m_print_spec = FALSE;
   }
   else
   {
     m_print_bw = FALSE;
     m_print_doc = FALSE;
     m_print_spec = TRUE;
   }


   m_header = wainApp.gs.m_printHeader;
   m_footer = wainApp.gs.m_printFooter;
   m_lineNo = wainApp.gs.m_printLineNo;
   m_wrap = wainApp.gs.m_printWrap;

   m_left_margin_str.Format("%d", wainApp.gs.m_printMargin);
   m_printScale = wainApp.gs.m_printScale;
}

PrintSetupDialogClass::~PrintSetupDialogClass()
{

}

BEGIN_MESSAGE_MAP(PrintSetupDialogClass, CDialog)
  ON_BN_CLICKED(IDC_PRINT_FONT, SelectFont)
  ON_BN_CLICKED(IDC_PRINT_COLOR, SelectColor)
  ON_BN_CLICKED(IDOK, OnOK)
  ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MESSAGE_MAP();

void PrintSetupDialogClass::OnOK(void)
{
   UpdateData(TRUE);
   wainApp.gs.m_printerFont = m_lf;
   if(m_print_bw)
      wainApp.gs.m_printColor = PC_BLACK_WHITE;
   else if(m_print_doc)
      wainApp.gs.m_printColor = PC_DOCUMENT;
   else
      wainApp.gs.m_printColor = PC_SPECIFIED;

   wainApp.gs.m_printHeader = m_header;
   wainApp.gs.m_printFooter = m_footer;
   wainApp.gs.m_printLineNo = m_lineNo;
   wainApp.gs.m_printWrap = m_wrap;
   wainApp.gs.m_printMargin = strtol(m_left_margin_str, NULL, 0);
   if(m_left_margin < 0 || m_left_margin > 100)
      m_left_margin = 10;
   wainApp.gs.m_printScale = m_printScale;
   EndDialog(IDOK);
}

void PrintSetupDialogClass::OnCancel(void)
{
  EndDialog(IDCANCEL);
}

void PrintSetupDialogClass::SelectFont(void)
{
  LOGFONT log_font = m_lf;

  CFontDialog dlg(&log_font);
  dlg.m_cf.Flags |= CF_ANSIONLY | CF_FORCEFONTEXIST | CF_FIXEDPITCHONLY;
  dlg.m_cf.Flags &= ~CF_EFFECTS;
  if (dlg.DoModal() == IDOK)
  {
    m_lf = log_font;
  }
}

void PrintSetupDialogClass::SelectColor(void)
{
  ColorType org_color[NOF_KEY_INDEX];
  memcpy(org_color, wainApp.gs.m_printingColor, sizeof(org_color));

  CPropertySheet setup("Printing Color", this);
  PrintColorDialogClass color(org_color, NOF_KEY_INDEX, KeyIndexStr);

  setup.AddPage((CPropertyPage *)&color);
  setup.m_psh.dwFlags |= PSH_NOAPPLYNOW;

  if(setup.DoModal() == IDOK)
  {
    memcpy(wainApp.gs.m_printingColor, org_color, sizeof(org_color));
  }
}

void PrintSetupDialogClass::DoDataExchange(CDataExchange *dx)
{
   CDialog::DoDataExchange(dx);
   DdxCheck(dx, IDC_PRINT_COLOR_BW, m_print_bw);
   DdxCheck(dx, IDC_PRINT_COLOR_DOC, m_print_doc);
   DdxCheck(dx, IDC_PRINT_COLOR_SPEC, m_print_spec);
   DDX_Text(dx, IDC_PRINT_HEADER, m_header);
   DDX_Text(dx, IDC_PRINT_FOOTER, m_footer);
   DdxCheck(dx, IDC_PRINT_LINE_NO, m_lineNo);
   DdxCheck(dx, IDC_PRINT_WRAP, m_wrap);
   DDX_Text(dx, IDC_PRINT_MARGIN, m_left_margin_str);
   DdxInt(dx, IDC_PRINT_SCALE, m_printScale, 10, 1000);
}

BOOL PrintSetupDialogClass::OnInitDialog()
{
  CSpinButtonCtrl *sb = (CSpinButtonCtrl *)GetDlgItem(IDC_PRINT_SPIN);
  ASSERT(sb);
  sb->SetRange(0, 100);
  sb = (CSpinButtonCtrl *)GetDlgItem(IDC_PRINT_SCALE_SPIN);
  sb->SetRange(10, 1000);

  UpdateData(FALSE);
  return CDialog::OnInitDialog();
}


