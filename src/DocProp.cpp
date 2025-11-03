//=============================================================================
// This source code file is a part of Wain.
// It implements the classes used by doc property setup.
// TagSetup_prop_class is implemented in Tags.cpp
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\resource.h"
#include ".\..\src\wain.h"
#include ".\..\src\DocProp.h"
#include ".\..\src\MainFrm.h"
#include ".\..\src\childfrm.h"
#include ".\..\src\wainview.h"
#include ".\..\src\waindoc.h"
#include "dialogbase.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class TemplateProp : public CPropertyPage
{
   DECLARE_DYNAMIC(TemplateProp);
   DocPropClass *m_prop;
public:
   TemplateProp(DocPropClass *doc_prop);
   ~TemplateProp(void);
   int m_sel;
   enum {IDD = IDD_TEMPLATE_PROP};

protected:
   virtual BOOL OnInitDialog();
   afx_msg void ChangedSelection(void);
   afx_msg void OnUpdate(void);
   afx_msg void OnDelete(void);
   afx_msg void OnNew(void);
   DECLARE_MESSAGE_MAP();
};

class KeywordSetupDialog : public PropertyPageBaseClass
{
private:
   void SaveKeywords(void);
   DocPropClass *m_prop;
public:
   enum { IDD = IDD_KEYWORD_SETUP};
   KeywordSetupDialog(DocPropClass *doc_prop);
   ~KeywordSetupDialog();
protected:
   virtual BOOL OnKillActive(void);
   virtual void DoDataExchange(CDataExchange *dx);
   virtual BOOL OnInitDialog();
   afx_msg void ListChanged(void);
   afx_msg void AddKeyword(void);
   afx_msg void DeleteKeyword(void);
   DECLARE_MESSAGE_MAP();
};

class LangOptionPropertyClass : public PropertyPageBaseClass
{
   DocPropClass *m_prop;
   std::string m_seps;
   std::string m_string;
   std::string m_ch;
   std::string m_braces;
   std::string m_blockBegin;
   std::string m_blockEnd;
   std::string m_lineComment;
   std::string m_commentBegin;
   std::string m_commentEnd;
   std::string m_preProcessor;
   std::string m_literal;
   std::string m_lineCon;
   std::string m_tabSize;
   std::string m_indent;
   std::string m_tabPos;
   std::string m_preProcWord;
   std::string m_numberStr;
   std::string m_indentAfter;
   std::string m_indentUnless;
   std::string m_unindent;
   std::string m_blockBeginWords;
   std::string m_blockEndWords;
public:
   enum {IDD = IDD_LANG_OPT_SETUP};
   virtual void DoDataExchange(CDataExchange *dx);
   virtual BOOL OnKillActive(void);
   LangOptionPropertyClass(DocPropClass *doc_prop);
   ~LangOptionPropertyClass();
   virtual BOOL OnInitDialog();
};

void PropStr2EditStr(std::string &aDest, const std::string &aSrc);
void EditStr2PropStr(std::string &aDest, const std::string &aSrc);

IMPLEMENT_DYNAMIC(TemplateProp, CPropertyPage)

BEGIN_MESSAGE_MAP(TemplateProp, CPropertyPage)
  ON_CBN_SELENDOK(IDC_TEMP_LIST, ChangedSelection)
  ON_BN_CLICKED(IDC_TEMP_OK,     OnUpdate)
  ON_BN_CLICKED(IDC_TEMP_DELETE, OnDelete)
  ON_BN_CLICKED(IDC_TEMP_NEW,    OnNew)
END_MESSAGE_MAP();


TemplateProp::TemplateProp(DocPropClass *doc_prop) : CPropertyPage(TemplateProp::IDD)
{
   m_prop = doc_prop;
}

TemplateProp::~TemplateProp()
{

}

BOOL TemplateProp::OnInitDialog()
{
  m_sel = 0;
  CComboBox *cb = (CComboBox *)GetDlgItem(IDC_TEMP_LIST);
  ASSERT(cb);
  size_t i;
  for(i = 0; i < m_prop->m_templateList.size(); i++)
    cb->AddString(m_prop->m_templateList[i]->m_name.c_str());
  CEdit *e = (CEdit *)GetDlgItem(IDC_TEMP_EDIT);
  ASSERT(e);
  HANDLE handle = GetStockObject(ANSI_FIXED_FONT);
  CFont *font = CFont::FromHandle((HFONT )handle);
  e->SetFont(font);
  if(i)
  {
    cb->SetCurSel(0);
    std::string Temp;
    PropStr2EditStr(Temp, m_prop->m_templateList[0]->m_expansion);
    e->SetWindowText(Temp.c_str());
  }
  return TRUE;
}

void TemplateProp::ChangedSelection(void)
{
  CComboBox *cb = (CComboBox *)GetDlgItem(IDC_TEMP_LIST);
  ASSERT(cb);
  CEdit *e = (CEdit *)GetDlgItem(IDC_TEMP_EDIT);
  ASSERT(e);

  m_sel = cb->GetCurSel();
  if(m_sel != CB_ERR)
  {
    std::string Temp;
    PropStr2EditStr(Temp, m_prop->m_templateList[m_sel]->m_expansion.c_str());
    e->SetWindowText(Temp.c_str());
  }
}

void TemplateProp::OnUpdate(void)
{
  if(m_prop->m_templateList.empty())
    OnNew();

  CComboBox *cb = (CComboBox *)GetDlgItem(IDC_TEMP_LIST);
  ASSERT(cb);
  CEdit *e = (CEdit *)GetDlgItem(IDC_TEMP_EDIT);
  ASSERT(e);
  if(cb->GetCurSel() != LB_ERR)
    m_sel = cb->GetCurSel();
  if(m_sel != LB_ERR)
  {
    /* Check if the name exist */
    int i, old_idx = -1;
    std::string Temp;
    GetWindowString(cb, Temp);

    for(i = 0; i < m_prop->m_templateList.size() && old_idx == -1; i++)
      if(m_sel != i && Temp == m_prop->m_templateList[i]->m_name)
      { /* Ups; same name */
        if(WainMessageBox(this, "The template name exist,\r\n would you like to owerwrite it?", IDC_MSG_YES | IDC_MSG_NO, IDI_QUESTION_ICO) == IDC_MSG_NO)
        { /* No, bail out ot let the user type a new name */
          return;
        }
        old_idx = i;
      }
    if(old_idx != -1)
    { /* Overwrite the existing template, so delete the current entry  */
      delete m_prop->m_templateList[m_sel];
      m_prop->m_templateList.erase(m_prop->m_templateList.begin() + m_sel);
      m_sel = old_idx;
    }
    GetWindowString(e, Temp);
    ASSERT(m_sel < m_prop->m_templateList.size());
    EditStr2PropStr(m_prop->m_templateList[m_sel]->m_expansion, Temp);
    GetWindowString(cb, Temp);
    if(Temp != m_prop->m_templateList[m_sel]->m_name)
    {
       cb->DeleteString(m_sel);
       cb->InsertString(m_sel, Temp.c_str());
    }
    m_prop->m_templateList[m_sel]->m_name = Temp;
  }
}

void TemplateProp::OnDelete(void)
{
  CComboBox *cb = (CComboBox *)GetDlgItem(IDC_TEMP_LIST);
  ASSERT(cb);
  CEdit *e = (CEdit *)GetDlgItem(IDC_TEMP_EDIT);
  ASSERT(e);
  if(cb->GetCurSel() != LB_ERR)
    m_sel = cb->GetCurSel();
  if(m_sel != LB_ERR)
  {
    delete m_prop->m_templateList[m_sel];
    std::vector<TemplateListClass *>::iterator it = m_prop->m_templateList.begin() + m_sel;
    m_prop->m_templateList.erase(it);

    cb->DeleteString(m_sel);
    if(cb->GetCount())
      cb->SetCurSel(0);

    m_sel = cb->GetCurSel();

    if(m_sel != LB_ERR)
    {
      cb->ResetContent();
      int i;
      for(i = 0; i < m_prop->m_templateList.size(); i++)
        cb->AddString(m_prop->m_templateList[i]->m_name.c_str());
      if(cb->GetCount())
        cb->SetCurSel(0);

      std::string Temp;
      PropStr2EditStr(Temp, m_prop->m_templateList[m_sel]->m_expansion);
      e->SetWindowText(Temp.c_str());
    }
    else
      e->SetWindowText("");
  }
}

void TemplateProp::OnNew(void)
{
  CComboBox *cb = (CComboBox *)GetDlgItem(IDC_TEMP_LIST);
  ASSERT(cb);
  CEdit *e = (CEdit *)GetDlgItem(IDC_TEMP_EDIT);
  ASSERT(e);
  m_prop->m_templateList.push_back(new TemplateListClass);
  m_sel = m_prop->m_templateList.size() - 1;
  cb->AddString("");
  cb->SetCurSel(m_sel);
  e->SetWindowText("");
}


void PropStr2EditStr(std::string &aDest, const std::string &aSrc)
{ // replace '\n' with \\n\r\n
   std::string Src(aSrc);
   aDest = "";
   std::string::size_type n;
   while((n = FindStrX(Src, 'n')) != std::string::npos)
   {
      aDest += Src.substr(0, n);
      aDest += "\\n\r\n";
      Src = Src.substr(n + 2);
   }
   aDest += Src;
}

void EditStr2PropStr(std::string &aDest, const std::string &aSrc)
{
   aDest = "";
   std::string Src(aSrc);
   std::string::size_type n;
   while((n = Src.find("\r\n")) != std::string::npos)
   {
      aDest += Src.substr(0, n);
      Src = Src.substr(n + 2);
   }
   aDest += Src;
}


BEGIN_MESSAGE_MAP(ColorEditClass, ColorEditBaseClass)
  ON_WM_PAINT()
END_MESSAGE_MAP();

BEGIN_MESSAGE_MAP(TabViewEditClass, ColorEditBaseClass)
  ON_WM_PAINT()
END_MESSAGE_MAP();

void ColorEditClass::OnPaint(void)
{
  m_init = true;
  CPaintDC dc(this);
  CFont *old_font = dc.SelectObject(TextWinFont);
  int LineHeight;
  TEXTMETRIC tm;
  dc.GetOutputTextMetrics(&tm);
  if (tm.tmHeight < 0)
    LineHeight = -tm.tmHeight;
  else
    LineHeight = (tm.tmHeight - tm.tmInternalLeading);
  LineHeight += 3;
  int CharWidth;
  CharWidth = tm.tmAveCharWidth < 0 ? -tm.tmAveCharWidth : tm.tmAveCharWidth;
  RECT cr;
  GetClientRect(&cr);
  dc.FillSolidRect(&cr, m_dlg->m_color[TEXT_IDX].m_backColor);

  #define SET_C(idx)                             \
    dc.SetTextColor(m_dlg->m_color[idx].m_textColor); \
    dc.SetBkColor(m_dlg->m_color[idx].m_backColor)

  SET_C(PRE_IDX);
  dc.TextOut(0, 0, "#include <stdio.h>", 18);
  SET_C(KEY_0_IDX);
  dc.TextOut(0, LineHeight, "int", 3);
  SET_C(TEXT_IDX);
  dc.TextOut(3*CharWidth, LineHeight, " main", 5);
  SET_C(TEXT_IDX);
  dc.TextOut(8*CharWidth, LineHeight, "(", 1);
  SET_C(KEY_0_IDX);
  dc.TextOut(9*CharWidth, LineHeight, "void", 4);
  SET_C(TEXT_IDX);
  dc.TextOut(13*CharWidth, LineHeight, ")", 1);

  SET_C(MATCH_IDX);
  dc.TextOut(0, 2*LineHeight, "{", 1);
  SET_C(COM_IDX);
  dc.TextOut(2*CharWidth, 2*LineHeight, "// dummy program", 16);

  SET_C(TEXT_IDX);
  dc.TextOut(0, 3*LineHeight, "  printf", 8);
  SET_C(SEP_IDX);
  dc.TextOut(8*CharWidth, 3*LineHeight, "(", 1);
  SET_C(STR_IDX);
  dc.TextOut(9*CharWidth, 3*LineHeight, "\"Hello World: %s\\n\"", 19);
  SET_C(SEP_IDX);
  dc.TextOut(28*CharWidth, 3*LineHeight, ", ", 2);
  SET_C(WORD_IDX);
  dc.TextOut(30*CharWidth, 3*LineHeight, "Word", 4);
  SET_C(SEP_IDX);
  dc.TextOut(34*CharWidth, 3*LineHeight, ");", 2);

  SET_C(TEXT_IDX);
  dc.TextOut(0, 4*LineHeight, "  ", 2);
  SET_C(MARK_IDX);
  dc.TextOut(2*CharWidth, 4*LineHeight, "marked_text", 11);
  SET_C(TEXT_IDX);
  dc.TextOut(13*CharWidth, 4*LineHeight, " ", 1);
  SET_C(SEP_IDX);
  dc.TextOut(14*CharWidth, 4*LineHeight, "=", 1);
  SET_C(TEXT_IDX);
  dc.TextOut(15*CharWidth, 4*LineHeight, " ", 1);

  SET_C(NUM_IDX);
  dc.TextOut(16*CharWidth, 4*LineHeight, "321", 3);
  SET_C(SEP_IDX);
  dc.TextOut(19*CharWidth, 4*LineHeight, ";", 1);

  SET_C(TEXT_IDX);
  dc.TextOut(0, 5*LineHeight, "  ", 2);
  SET_C(KEY_1_IDX);
  dc.TextOut(2*CharWidth, 5*LineHeight, "Keyword1", 8);
  SET_C(SEP_IDX);
  dc.TextOut(10*CharWidth, 5*LineHeight, " = ", 3);
  SET_C(KEY_2_IDX);
  dc.TextOut(13*CharWidth, 5*LineHeight, "Keyword2", 8);
  SET_C(SEP_IDX);
  dc.TextOut(21*CharWidth, 5*LineHeight, ";", 1);

  SET_C(TEXT_IDX);
  dc.TextOut(0, 6*LineHeight, "  ", 2);
  SET_C(KEY_3_IDX);
  dc.TextOut(2*CharWidth, 6*LineHeight, "Keyword3", 8);
  SET_C(SEP_IDX);
  dc.TextOut(10*CharWidth, 6*LineHeight, " = ", 3);
  SET_C(KEY_4_IDX);
  dc.TextOut(13*CharWidth, 6*LineHeight, "Keyword4", 8);
  SET_C(SEP_IDX);
  dc.TextOut(21*CharWidth, 6*LineHeight, ";", 1);

  SET_C(TEXT_IDX);
  dc.TextOut(0, 7*LineHeight, "  ", 2);
  SET_C(KEY_0_IDX);
  dc.TextOut(2*CharWidth, 7*LineHeight, "return", 6);
  SET_C(TEXT_IDX);
  dc.TextOut(8*CharWidth, 7*LineHeight, " ", 1);
  SET_C(TAG_IDX);
  dc.TextOut(9*CharWidth, 7*LineHeight, "tag", 3);
  SET_C(SEP_IDX);
  dc.TextOut(12*CharWidth, 7*LineHeight, ";", 1);
  SET_C(MATCH_IDX);
  dc.TextOut(0, 8*LineHeight, "}", 1);

  dc.SelectObject(old_font);
  #undef SET_C
}

void TabViewEditClass::OnPaint(void)
{
  m_init = true;
  CPaintDC dc(this);
  CFont *old_font = dc.SelectObject(TextWinFont);
  int LineHeight;
  TEXTMETRIC tm;
  dc.GetOutputTextMetrics(&tm);
  if (tm.tmHeight < 0)
    LineHeight = -tm.tmHeight;
  else
    LineHeight = (tm.tmHeight - tm.tmInternalLeading);
  LineHeight += 3;
  int CharWidth;
  CharWidth = tm.tmAveCharWidth < 0 ? -tm.tmAveCharWidth : tm.tmAveCharWidth;
  RECT cr;
  GetClientRect(&cr);
  dc.FillSolidRect(&cr, m_dlg->m_color[GlobalSettingsClass::TVC_NORMAL].m_backColor);

  for(int i = 0; i < m_dlg->m_numColor; i++)
  {
    dc.SetTextColor(m_dlg->m_color[i].m_textColor);
    dc.SetBkColor(m_dlg->m_color[i].m_backColor);
    dc.TextOut(CharWidth, i*LineHeight, m_dlg->m_indexStr[i]);
  }
  dc.SelectObject(old_font);
}

// #define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
IMPLEMENT_DYNAMIC(ColorDialogBaseClass, CPropertyPage)
IMPLEMENT_DYNAMIC(DocColorDialogClass, ColorDialogBaseClass)
IMPLEMENT_DYNAMIC(PrintColorDialogClass, ColorDialogBaseClass)
IMPLEMENT_DYNAMIC(TabViewColorDialogClass, ColorDialogBaseClass)

BEGIN_MESSAGE_MAP(ColorDialogBaseClass, CPropertyPage)
  ON_EN_CHANGE(IDC_RED_EC, RedChanged)
  ON_EN_CHANGE(IDC_GREEN_EC, GreenChanged)
  ON_EN_CHANGE(IDC_BLUE_EC, BlueChanged)
  ON_EN_CHANGE(IDC_RED_BG_EC, RedBackChanged)
  ON_EN_CHANGE(IDC_GREEN_BG_EC, GreenBackChanged)
  ON_EN_CHANGE(IDC_BLUE_BG_EC, BlueBackChanged)
  ON_LBN_SELCHANGE(IDC_COLOR_LIST, NewSel)
  ON_BN_CLICKED(IDC_LOAD_SCHEME, LoadScheme)
  ON_BN_CLICKED(IDC_SAVE_SCHEME, SaveScheme)
  ON_BN_CLICKED(IDC_VERIFY_COLOR, VerifyColor)
  ON_BN_CLICKED(IDC_CD_SEL_FORG,  SelForg)
  ON_BN_CLICKED(IDC_CD_SEL_BACK,  SelBack)
END_MESSAGE_MAP();

void ColorDialogBaseClass::DoDataExchange(CDataExchange *aDx)
{
  CDialog::DoDataExchange(aDx);
}

BOOL ColorDialogBaseClass::OnKillActive(void)
{
  UpdateData(TRUE);
  return CPropertyPage::OnKillActive();
}

void ColorDialogBaseClass::NewSel(void)
{
  CListBox *eb = (CListBox *)GetDlgItem(IDC_COLOR_LIST);
  UpdateData(TRUE);
  m_sel = eb->GetCurSel();
  unsigned int r, g, b;
  #define SET_COLORS(color_) r = color_ & 0xFF; g = (color_ >> 8) & 0xFF; b = (color_ >> 16) & 0xFF;
  SET_COLORS(m_color[m_sel].m_textColor);
  #undef SET_COLORS
  char text[10];
  CEdit *ce;
  sprintf(text, "%d", r);
  ce = (CEdit *)GetDlgItem(IDC_RED_EC);
  ce->SetWindowText(text);
  sprintf(text, "%d", g);
  ce = (CEdit *)GetDlgItem(IDC_GREEN_EC);
  ce->SetWindowText(text);
  sprintf(text, "%d", b);
  ce = (CEdit *)GetDlgItem(IDC_BLUE_EC);
  ce->SetWindowText(text);

  #define SET_COLORS(color) r = color & 0xFF; g = (color >> 8) & 0xFF; b = (color >> 16) & 0xFF;
  SET_COLORS(m_color[m_sel].m_backColor);
  #undef SET_COLORS
  sprintf(text, "%d", r);
  ce = (CEdit *)GetDlgItem(IDC_RED_BG_EC);
  ce->SetWindowText(text);
  sprintf(text, "%d", g);
  ce = (CEdit *)GetDlgItem(IDC_GREEN_BG_EC);
  ce->SetWindowText(text);
  sprintf(text, "%d", b);
  ce = (CEdit *)GetDlgItem(IDC_BLUE_BG_EC);
  ce->SetWindowText(text);

  UpdateData(FALSE);
}

void ColorDialogBaseClass::RedChanged(void)
{
  #define SET_COLOR(color_, c) color_ = RGB(c, (color_ >> 8) & 0xFF, (color_ >> 16) & 0xFF);

  CEdit *ce = (CEdit *)GetDlgItem(IDC_RED_EC);
  CListBox *eb = (CListBox *)GetDlgItem(IDC_COLOR_LIST);
  m_sel = eb->GetCurSel();
  if(ce && m_edit->m_init && m_sel != LB_ERR)
  {
    char temp[128];
    ce->GetWindowText(temp, 128);
    unsigned int r = atoi(temp);
    if(r > 255)
    {
      r = 255;
      ce->SetWindowText("255");
    }
    SET_COLOR(m_color[m_sel].m_textColor, r);

    m_edit->InvalidateRect(NULL, TRUE);
  }
  #undef SET_COLOR
}

void ColorDialogBaseClass::RedBackChanged(void)
{
  #define SET_COLOR(color, c) color = RGB(c, (color >> 8) & 0xFF, (color >> 16) & 0xFF);

  CEdit *ce = (CEdit *)GetDlgItem(IDC_RED_BG_EC);
  CListBox *eb = (CListBox *)GetDlgItem(IDC_COLOR_LIST);
  m_sel = eb->GetCurSel();
  if(ce && m_edit->m_init && m_sel != LB_ERR)
  {
    char temp[128];
    ce->GetWindowText(temp, 128);
    unsigned int r = atoi(temp);
    if(r > 255)
    {
      r = 255;
      ce->SetWindowText("255");
    }
    SET_COLOR(m_color[m_sel].m_backColor, r);
    m_edit->InvalidateRect(NULL, TRUE);
  }
  #undef SET_COLOR
}

void ColorDialogBaseClass::GreenChanged(void)
{
  #define SET_COLOR(color, c) color = RGB(color & 0xFF, c, (color >> 16) & 0xFF);
  CEdit *ce = (CEdit *)GetDlgItem(IDC_GREEN_EC);
  CListBox *eb = (CListBox *)GetDlgItem(IDC_COLOR_LIST);
  m_sel = eb->GetCurSel();

  if(ce && m_edit->m_init && m_sel != LB_ERR)
  {
    char temp[128];
    ce->GetWindowText(temp, 128);
    unsigned int g = atoi(temp);
    if(g > 255)
    {
      g = 255;
      ce->SetWindowText("255");
    }
    SET_COLOR(m_color[m_sel].m_textColor, g);
    m_edit->InvalidateRect(NULL, TRUE);
  }
  #undef SET_COLOR
}

void ColorDialogBaseClass::GreenBackChanged(void)
{
  #define SET_COLOR(color, c) color = RGB(color & 0xFF, c, (color >> 16) & 0xFF);

  CEdit *ce = (CEdit *)GetDlgItem(IDC_GREEN_BG_EC);
  CListBox *eb = (CListBox *)GetDlgItem(IDC_COLOR_LIST);
  m_sel = eb->GetCurSel();

  if(ce && m_edit->m_init && m_sel != LB_ERR)
  {
    char temp[128];
    ce->GetWindowText(temp, 128);
    unsigned int g = atoi(temp);
    if(g > 255)
    {
      g = 255;
      ce->SetWindowText("255");
    }
    SET_COLOR(m_color[m_sel].m_backColor, g);
    m_edit->InvalidateRect(NULL, TRUE);
  }
  #undef SET_COLOR
}

void ColorDialogBaseClass::BlueChanged(void)
{
  #define SET_COLOR(color, c) color = RGB(color & 0xFF, (color >> 8) & 0xFF, c);
  CEdit *ce = (CEdit *)GetDlgItem(IDC_BLUE_EC);
  CListBox *eb = (CListBox *)GetDlgItem(IDC_COLOR_LIST);
  m_sel = eb->GetCurSel();

  if(ce && m_edit->m_init && m_sel != LB_ERR)
  {
    char temp[128];
    ce->GetWindowText(temp, 128);
    unsigned int b = atoi(temp);
    if(b > 255)
    {
      b = 255;
      ce->SetWindowText("255");
    }
    SET_COLOR(m_color[m_sel].m_textColor, b);
    m_edit->InvalidateRect(NULL, TRUE);
  }
  #undef SET_COLOR
}

void ColorDialogBaseClass::BlueBackChanged(void)
{
  #define SET_COLOR(color, c) color = RGB(color & 0xFF, (color >> 8) & 0xFF, c);
  CEdit *ce = (CEdit *)GetDlgItem(IDC_BLUE_BG_EC);
  CListBox *eb = (CListBox *)GetDlgItem(IDC_COLOR_LIST);
  m_sel = eb->GetCurSel();

  if(ce && m_edit->m_init && m_sel != LB_ERR)
  {
    char temp[128];
    ce->GetWindowText(temp, 128);
    unsigned int b = atoi(temp);
    if(b > 255)
    {
      b = 255;
      ce->SetWindowText("255");
    }
    SET_COLOR(m_color[m_sel].m_backColor, b);
    m_edit->InvalidateRect(NULL, TRUE);
  }
  #undef SET_COLOR
}

BOOL ColorDialogBaseClass::OnInitDialog(void)
{
  m_sel = 0;
  CPropertyPage::OnSetActive();
  RECT er = {110, 5, 295, 90 + 5};
  MapDialogRect(&er);
  m_edit->CreateEx(WS_EX_CLIENTEDGE, "EDIT", 0, WS_CHILD | WS_VISIBLE | ES_MULTILINE, er, this, IDC_COLOR_SAMPLE);
  m_edit->m_font = GetFont();

  CSpinButtonCtrl *spin;
  spin = (CSpinButtonCtrl *)GetDlgItem(IDC_RED_SCROLL);
  ASSERT(spin);
  spin->SetRange(0, 256 - 1);
  spin = (CSpinButtonCtrl *)GetDlgItem(IDC_GREEN_SCROLL);
  ASSERT(spin);
  spin->SetRange(0, 256 - 1);
  spin = (CSpinButtonCtrl *)GetDlgItem(IDC_BLUE_SCROLL);
  ASSERT(spin);
  spin->SetRange(0, 256 - 1);
  spin = (CSpinButtonCtrl *)GetDlgItem(IDC_RED_BG_SCROLL);
  ASSERT(spin);
  spin->SetRange(0, 256 - 1);
  spin = (CSpinButtonCtrl *)GetDlgItem(IDC_GREEN_BG_SCROLL);
  ASSERT(spin);
  spin->SetRange(0, 256 - 1);
  spin = (CSpinButtonCtrl *)GetDlgItem(IDC_BLUE_BG_SCROLL);
  ASSERT(spin);
  spin->SetRange(0, 256 - 1);

  CListBox *eb = (CListBox *)GetDlgItem(IDC_COLOR_LIST);
  if(eb)
  {
    int i;
    for(i = 0; i < m_numColor; i++)
      eb->AddString(m_indexStr[i]);
    eb->SetCurSel(0);
    eb->SetFocus();
  }
  NewSel();
  return TRUE;
}

BOOL DocColorDialogClass::OnInitDialog()
{
  return ColorDialogBaseClass::OnInitDialog();
}

BOOL PrintColorDialogClass::OnInitDialog()
{
  BOOL Status = ColorDialogBaseClass::OnInitDialog();

  CEdit *e;
  e = (CEdit *)GetDlgItem(IDC_RED_BG_EC);
  ASSERT(e);
  e->ModifyStyle(0, WS_DISABLED);
  e = (CEdit *)GetDlgItem(IDC_GREEN_BG_EC);
  ASSERT(e);
  e->ModifyStyle(0, WS_DISABLED);
  e = (CEdit *)GetDlgItem(IDC_BLUE_BG_EC);
  ASSERT(e);
  e->ModifyStyle(0, WS_DISABLED);

  CSpinButtonCtrl *spin;
  spin = (CSpinButtonCtrl *)GetDlgItem(IDC_RED_BG_SCROLL);
  ASSERT(spin);
  spin->ModifyStyle(0, WS_DISABLED);
  spin = (CSpinButtonCtrl *)GetDlgItem(IDC_GREEN_BG_SCROLL);
  ASSERT(spin);
  spin->ModifyStyle(0, WS_DISABLED);
  spin = (CSpinButtonCtrl *)GetDlgItem(IDC_BLUE_BG_SCROLL);
  ASSERT(spin);
  spin->ModifyStyle(0, WS_DISABLED);

  CButton *b = (CButton *)GetDlgItem(IDC_CD_SEL_BACK);
  ASSERT(b);
  b->ModifyStyle(0, WS_DISABLED);

  return Status;
}

BOOL TabViewColorDialogClass::OnInitDialog()
{
  BOOL Status = ColorDialogBaseClass::OnInitDialog();
  CButton *b = (CButton *)GetDlgItem(IDC_LOAD_SCHEME);
  ASSERT(b);
  b->ModifyStyle(WS_VISIBLE, WS_DISABLED);
  b = (CButton *)GetDlgItem(IDC_SAVE_SCHEME);
  ASSERT(b);
  b->ModifyStyle(WS_VISIBLE, WS_DISABLED);
  return Status;
}

void ColorDialogBaseClass::LoadScheme(void)
{
  MyFileDialogClass fd(FD_OPEN | FD_CONFIG_PATH, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "wcs", NULL, "WainColorScheme (*.wcs)\0*.wcs\0All Files (*.*)\0*.*\0\0", this);
  if(fd.DoModal() != IDOK)
    return;

  std::string file_name = (const char *)fd.GetPathName();
  FILE *f = fopen(file_name.c_str(), "rt");
  if(!f)
  {
    std::string msg = "Failed to open:\r\n";
    msg += file_name;
    WainMessageBox(this, msg.c_str(), IDC_MSG_OK, IDI_ERROR_ICO);
    return;
  }
  char temp[128], temp2[128], *s;
  int k;
  while(fgets(temp, sizeof(temp), f))
  {
    if((s = strchr(temp, '\n')) != NULL)
      *s = 0;
    s = strtok(temp, ": ");
    BOOL found;
    if(s && (s = strtok(NULL, ": ")) != NULL)
      found = FALSE;
    else
      found = TRUE;

    for(k = 0; k < m_numColor && !found; k++)
    {
      sprintf(temp2, "%sText", m_indexStr[k]);
      if(!strcmp(temp2, temp))
      {
        m_color[k].m_textColor = strtol(s, NULL, 0);
      }
      else
      {
        sprintf(temp2, "%sBackGround", m_indexStr[k]);
        if(!strcmp(temp2, temp))
        {
          m_color[k].m_backColor = strtol(s, NULL, 0);
        }
      }
    }
  }
  fclose(f);
  NewSel();
  m_edit->InvalidateRect(NULL, TRUE);
}

void ColorDialogBaseClass::SaveScheme(void)
{
  MyFileDialogClass fd(FD_SAVE | FD_CONFIG_PATH, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "wcs", NULL, "WainColorScheme (*.wcs)\0*.wcs\0All Files (*.*)\0*.*\0\0", this);
  if(fd.DoModal() != IDOK)
    return;

  CString file_name = fd.GetPathName();
  FILE *f = fopen(file_name, "wt");
  if(!f)
  {
    CString msg = "Failed to open:\r\n";
    msg += file_name;
    WainMessageBox(this, msg, IDC_MSG_OK, IDI_ERROR_ICO);
    return;
  }
  int k;
  for(k = 0; k < m_numColor; k++)
  {
    fprintf(f, "%sText: 0x%06X\n", m_indexStr[k], m_color[k].m_textColor);
    fprintf(f, "%sBackGround: 0x%06X\n", m_indexStr[k], m_color[k].m_backColor);
  }
  fclose(f);
  SetStatusText("Color scheme saved");
}

void ColorDialogBaseClass::VerifyColor(void)
{
  CDC *dc = m_edit->GetDC();
  ASSERT(dc);

  BOOL all_ok = TRUE;
  int k;

  ColorType new_color[NOF_KEY_INDEX];

  for(k = 0; k < m_numColor; k++)
  {
    new_color[k].m_textColor = dc->GetNearestColor(m_color[k].m_textColor);
    if(new_color[k].m_textColor != m_color[k].m_textColor)
      all_ok = FALSE;
    new_color[k].m_backColor = dc->GetNearestColor(m_color[k].m_backColor);
    if(new_color[k].m_backColor != m_color[k].m_backColor)
      all_ok = FALSE;
  }

  ReleaseDC(dc);
  if(all_ok)
    SetStatusText("All colors seems to be ok");
  else
  {
    /* Set color to the adjusted colors so my edit will use them when drawing */
    ColorType *temp_color = m_color;
    m_color = new_color;
    m_edit->InvalidateRect(NULL, TRUE);
    CString msg = "Windows can't display one or more colors correct\r\n";
    msg += "Do you want to use the adjusted colors instead?";
    if(WainMessageBox(this, msg, IDC_MSG_YES | IDC_MSG_NO, IDI_QUESTION_ICO) == IDC_MSG_YES)
    {
      m_color = temp_color;
      memcpy(m_color, new_color, sizeof(new_color));
      NewSel();
    }
    else
    {
      m_color = temp_color;
    }
  }
}

void ColorDialogBaseClass::SelForg(void)
{

  CListBox *eb = (CListBox *)GetDlgItem(IDC_COLOR_LIST);
  UpdateData(TRUE);
  m_sel = eb->GetCurSel();
  if(m_sel != LB_ERR)
  {
    CColorDialog cd(m_color[m_sel].m_textColor, CC_RGBINIT | CC_SOLIDCOLOR, this);
    if(cd.DoModal() == IDOK)
    {
      m_color[m_sel].m_textColor = cd.GetColor();
      NewSel();
    }
  }
}

void ColorDialogBaseClass::SelBack(void)
{
  CListBox *eb = (CListBox *)GetDlgItem(IDC_COLOR_LIST);
  UpdateData(TRUE);
  m_sel = eb->GetCurSel();
  if(m_sel != LB_ERR)
  {
    CColorDialog cd(m_color[m_sel].m_backColor, CC_RGBINIT | CC_SOLIDCOLOR, this);
    if(cd.DoModal() == IDOK)
    {
      m_color[m_sel].m_backColor = cd.GetColor();
      NewSel();
    }
  }
}


BEGIN_MESSAGE_MAP(KeywordSetupDialog, PropertyPageBaseClass)
  ON_LBN_SELCHANGE(IDC_KEYGROUP_LIST, ListChanged)
  ON_BN_CLICKED(IDC_KEYWORD_APPLY, AddKeyword)
  ON_BN_CLICKED(IDC_KEYWORD_DEL, DeleteKeyword)
END_MESSAGE_MAP();

KeywordSetupDialog::KeywordSetupDialog(DocPropClass *doc_prop) : PropertyPageBaseClass(KeywordSetupDialog::IDD)
{
   m_prop = doc_prop;
}

KeywordSetupDialog::~KeywordSetupDialog()
{

}

void KeywordSetupDialog::DoDataExchange(CDataExchange *dx)
{
   CPropertyPage::DoDataExchange(dx);
   DdxCheck(dx, IDC_KEYWORD_IC, m_prop->m_ignoreCase);
}

BOOL KeywordSetupDialog::OnInitDialog(void)
{
  CListBox *lb = (CListBox *)GetDlgItem(IDC_KEYGROUP_LIST);
  if(lb)
  {
    lb->AddString("Keywords");
    lb->AddString("Keywords 1");
    lb->AddString("Keywords 2");
    lb->AddString("Keywords 3");
    lb->AddString("Keywords 4");
  }
  lb->SetCurSel(0);
  lb = (CListBox *)GetDlgItem(IDC_KEYWORD_LIST);
  if(lb)
  {
    for(int i = 0; i < m_prop->m_keyWordList[0].size(); i++)
      lb->AddString(m_prop->m_keyWordList[0][i].c_str());
  }
  UpdateData(FALSE);
  return TRUE;
}

BOOL KeywordSetupDialog::OnKillActive(void)
{
  UpdateData(TRUE);
  return CPropertyPage::OnKillActive();
}

void KeywordSetupDialog::ListChanged(void)
{
  CListBox *lb = (CListBox *)GetDlgItem(IDC_KEYGROUP_LIST);
  int sel = lb->GetCurSel();
  if(sel < 5 && sel >= 0)
  {
    lb = (CListBox *)GetDlgItem(IDC_KEYWORD_LIST);
    lb->ResetContent();
    for(int i = 0; i < m_prop->m_keyWordList[sel].size(); i++)
       lb->AddString(m_prop->m_keyWordList[sel][i].c_str());
  }
}

void KeywordSetupDialog::AddKeyword(void)
{
  CEdit *e = (CEdit *)GetDlgItem(IDC_KEYWORD_EDIT);
  if(e)
  {
    char temp[1024];
    e->GetWindowText(temp, 1024);
    if(strlen(temp))
    {
      e->SetWindowText("");
      CListBox *lb = (CListBox *)GetDlgItem(IDC_KEYGROUP_LIST);
      int sel = lb->GetCurSel();
      if(sel < 5 && sel >= 0)
      {
        lb = (CListBox *)GetDlgItem(IDC_KEYWORD_LIST);
        int index = -1;
        int top = -1;
        BOOL match = FALSE;
        BOOL Done = FALSE;
        while(!match && !Done && (index = lb->FindStringExact(index, temp)) != LB_ERR)
        {
          char lstr[1024];
          lb->GetText(index, lstr);
          if(index < top)
            Done = TRUE;
          index++;
          top = index;
          if(!strcmp(temp, lstr))
            match = TRUE;
        }
        if(!match)
        {
          m_prop->m_keyWordList[sel].push_back(temp);
          lb->AddString(temp);
        }
      }
    }
  }
}

int KeywordSortFunc(const void *p1, const void *p2)
{
  return strcmp(*(const char **)p1, *(const char **)p2);
}

int KeywordICaseSortFunc(const void *p1, const void *p2)
{
  return stricmp(*(const char **)p1, *(const char **)p2);
}

void KeywordSetupDialog::SaveKeywords(void)
{
  int i;
  StringIgnoreCaseCompareClass StringIgnoreCaseCompare;
  for(i = 0; i < 5; i++)
  {
     if(m_prop->m_ignoreCase)
        std::sort(m_prop->m_keyWordList[i].begin(), m_prop->m_keyWordList[i].end(), StringIgnoreCaseCompare);
     else
        std::sort(m_prop->m_keyWordList[i].begin(), m_prop->m_keyWordList[i].end());
  }

  char file_name[_MAX_PATH];
  sprintf(file_name, "%skeywords.lst", wainApp.gs.m_configPath);
  FILE *f = fopen(file_name, "wt");
  if(f)
  {
    for(i = 0; i < 5; i++)
    {
      fprintf(f, "[Keyword %d]\n", i);
      for(int j = 0; j < m_prop->m_keyWordList[i].size(); j++)
         fprintf(f, "%s\n", m_prop->m_keyWordList[i][j].c_str());
    }
    fclose(f);
  }
}

void KeywordSetupDialog::DeleteKeyword(void)
{
  CListBox *lb = (CListBox *)GetDlgItem(IDC_KEYGROUP_LIST);
  int group = lb->GetCurSel();
  if(group >= 0 && group < 5)
  {
    lb = (CListBox *)GetDlgItem(IDC_KEYWORD_LIST);
    if(lb)
    {
      int sel = lb->GetCurSel();
      if(sel != LB_ERR && sel < m_prop->m_keyWordList[group].size())
      {
        lb->DeleteString(sel);
        m_prop->m_keyWordList[group].erase(m_prop->m_keyWordList[group].begin() + sel);
      }
    }
  }
}

LangOptionPropertyClass::LangOptionPropertyClass(DocPropClass *aDocProp) :
   PropertyPageBaseClass(LangOptionPropertyClass::IDD)
{
   m_prop = aDocProp;
}

LangOptionPropertyClass::~LangOptionPropertyClass()
{

}

void LangOptionPropertyClass::DoDataExchange(CDataExchange *dx)
{
   CPropertyPage::DoDataExchange(dx);
   DdxString(dx, IDC_LO_SEPS, m_seps);
   DdxString(dx, IDC_LO_BBEGIN, m_blockBegin);
   DdxString(dx, IDC_LO_BEND, m_blockEnd);
   DdxString(dx, IDC_LO_BRACES, m_braces);
   DdxString(dx, IDC_LO_STRING, m_string);
   DdxString(dx, IDC_LO_CHAR, m_ch);
   DdxString(dx, IDC_LO_LINE_COMMENT, m_lineComment);
   DdxString(dx, IDC_LO_COMMENT_BEGIN, m_commentBegin);
   DdxString(dx, IDC_LO_COMMENT_END, m_commentEnd);
   DdxString(dx, IDC_LO_PREPROCESSOR, m_preProcessor);
   DdxCheck(dx, IDC_LO_COM_WHOLE_WORDS, m_prop->m_commentWholeWord);
   DdxCheck(dx, IDC_LO_COM_FIRST_WORD, m_prop->m_commentFirstWord);
   DdxString(dx, IDC_LO_LITERAL, m_literal);
   DdxString(dx, IDC_LO_LINE_CON, m_lineCon);
   DdxString(dx, IDC_TAB_SIZE_EDIT, m_tabSize);
   DdxString(dx, IDC_INDENT_EDIT, m_indent);
   DdxString(dx, IDC_LO_TAB_POS, m_tabPos);
   DdxString(dx, IDC_LO_PREPROC_WORDS, m_preProcWord);
   DdxString(dx, IDC_LO_NUMBER_STR, m_numberStr);
   DdxString(dx, IDC_LO_BLOCK_START_WORDS, m_blockBeginWords);
   DdxString(dx, IDC_LO_BLOCK_END_WORDS, m_blockEndWords);
}

std::string ConcatStringList(const std::list<std::string>& aList, char aSep)
{
   std::list<std::string>::const_iterator i = aList.begin();
   std::string res;
   for( ; i != aList.end(); i++)
   {
      if (!res.empty())
      {
         res += aSep;
      }
      res += *i;
   }
   return res;
}

std::list<std::string> SplitStringList(const std::string& aString, char /* aSep */)
{
   StrSplitterClass ss(aString);
   std::list<std::string> theList;
   std::string s;
   while(ss.Get(s, ";"))
   {
      theList.push_back(s);
   }
   return theList;
}

BOOL LangOptionPropertyClass::OnInitDialog(void)
{
   CPropertyPage::OnInitDialog();

   std::string temp;
   ConvFromTab(temp, m_prop->m_seps);
   m_seps = temp;
   m_string = m_prop->m_stringDelim;
   m_ch = m_prop->m_chDelim;
   m_braces = m_prop->m_braces;
   m_blockBegin = m_prop->m_blockBegin;
   m_blockEnd = m_prop->m_blockEnd;
   m_lineComment = m_prop->m_lineComment;
   m_commentBegin = m_prop->m_commentBegin;
   m_commentEnd = m_prop->m_commentEnd;
   m_preProcessor = m_prop->m_preProcessor;
   m_numberStr = m_prop->m_numberStr;
   m_tabPos = m_prop->m_tabPos.ToString();
   m_blockBeginWords = ConcatStringList(m_prop->m_blockBeginWords, ';');
   m_blockEndWords = ConcatStringList(m_prop->m_blockEndWords, ';');

   size_t i;
   m_indentAfter = "";
   for(i = 0; i < m_prop->m_indentAfter.size(); i++)
   {
      m_indentAfter += m_prop->m_indentAfter[i];
      m_indentAfter += " ";
   }
   m_indentUnless = "";
   for(i = 0; i < m_prop->m_indentUnless.size(); i++)
   {
      m_indentUnless += m_prop->m_indentUnless[i];
      m_indentUnless += " ";
   }
   m_unindent = "";
   for(i = 0; i < m_prop->m_unindent.size(); i++)
   {
      m_unindent += m_prop->m_unindent[i];
      m_unindent += " ";
   }

   m_preProcWord = "";

   for(i = 0; i < m_prop->m_preProcWord.size(); i++)
   {
      m_preProcWord += m_prop->m_preProcWord[i];
      m_preProcWord += " ";
   }

   m_literal = m_prop->m_literal;
   m_lineCon = m_prop->m_lineCon;
   CSpinButtonCtrl *sb = (CSpinButtonCtrl *)GetDlgItem(IDC_TAB_SIZE_SPIN);
   sb->SetRange(1, 32);

   m_tabSize = ToString(m_prop->m_tabSize);

   sb = (CSpinButtonCtrl *)GetDlgItem(IDC_INDENT_SPIN);
   sb->SetRange(0, 32);
   m_indent = ToString(m_prop->m_indent);
   UpdateData(FALSE);
   return TRUE;
}

BOOL LangOptionPropertyClass::OnKillActive(void)
{
   UpdateData(TRUE);
   m_prop->m_seps = m_seps;
   ConvToTab(m_prop->m_seps);
   m_prop->m_stringDelim = m_string;
   m_prop->m_chDelim = m_ch;
   m_prop->m_braces = m_braces;
   m_prop->m_blockBegin = m_blockBegin;
   m_prop->m_blockEnd = m_blockEnd;
   m_prop->m_lineComment = m_lineComment;
   m_prop->m_commentBegin = m_commentBegin;
   m_prop->m_commentEnd = m_commentEnd;
   m_prop->m_preProcessor = m_preProcessor;
   m_prop->m_literal = m_literal;
   m_prop->m_lineCon = m_lineCon;
   m_prop->m_tabSize = FromString<int>(m_tabSize);
   m_prop->m_indent = FromString<int>(m_indent);
   m_prop->m_tabPos.FromString(m_tabPos);
   m_prop->m_blockBeginWords = SplitStringList(m_blockBeginWords, ';');
   m_prop->m_blockEndWords = SplitStringList(m_blockEndWords, ';');

   std::string T, Temp;
   Temp = m_indentAfter;
   m_prop->m_indentAfter.clear();
   do
   {
      T = StrTok(Temp, ", ");
      if(!T.empty())
      {
         m_prop->m_indentAfter.push_back(T);
      }
   }
   while(!T.empty());

   Temp = m_indentUnless;
   m_prop->m_indentUnless.clear();
   do
   {
      T = StrTok(Temp, ", ");
      if(T.size())
      {
         m_prop->m_indentUnless.push_back(T);
      }
   }
   while(!T.empty());

   Temp = m_unindent;
   m_prop->m_unindent.clear();
   do
   {
      T = StrTok(Temp, ", ");
      if(!T.empty())
      {
         m_prop->m_unindent.push_back(T);
      }
   }
   while(!T.empty());

   m_prop->m_preProcWord.clear();
   Temp = m_preProcWord;
   do
   {
      T = StrTok(Temp, ", ");
      if(!T.empty())
         m_prop->m_preProcWord.push_back(T);
   }
   while(!T.empty());
   try
   {
      m_prop->m_numberRegEx = m_numberStr.c_str();
   }
   catch (const std::regex_error& e)
   {
      CString msg = "The number reg exp failed to compile\r\n";
      msg += "With the following error: ";
      msg += e.what();
      WainMessageBox(this, msg, IDC_MSG_OK, IDI_ERROR_ICO);
      return FALSE;
   }
   m_prop->m_numberStr = m_numberStr;

   m_prop->m_modified = true;
   return CPropertyPage::OnKillActive();
}

LangSelectDialogClass::LangSelectDialogClass(CWnd *parent) : DialogBaseClass(LangSelectDialogClass::IDD, parent)
{

}

LangSelectDialogClass::~LangSelectDialogClass(void)
{

}

BOOL LangSelectDialogClass::OnInitDialog(void)
{
   CListBox *lb = (CListBox *)GetDlgItem(IDC_LS_TYPE_LIST);
   ASSERT(lb);
   int i;
   CString text;
   for(i = 0; i < wainApp.gs.m_docProp.size(); i++)
     lb->AddString(wainApp.gs.m_docProp[i]->m_extensionType.c_str());
   m_currentSel = 0;
   ChildFrame *cf = (ChildFrame *)GetMf()->MDIGetActive();
   if(cf)
     m_currentSel = cf->GetDocument()->GetPropIndex();

   lb->SetCurSel(m_currentSel);
   lb->GetText(m_currentSel, text);
   SetDlgItemText(IDC_LS_TYPE_EDIT, text);

   lb = (CListBox *)GetDlgItem(IDC_LS_EXT_LIST);
   ASSERT(lb);
   lb->AddString("*.*");
   for(i = 1; i < wainApp.gs.m_docProp.size(); i++)
     lb->AddString(wainApp.gs.m_docProp[i]->m_extensions.c_str());

   lb->SetCurSel(m_currentSel);
   lb->GetText(m_currentSel, text);
   SetDlgItemText(IDC_LS_EXT_EDIT, text);
   CEdit *e1 = (CEdit *)GetDlgItem(IDC_LS_EXT_EDIT);
   CEdit *e2 = (CEdit *)GetDlgItem(IDC_LS_TYPE_EDIT);
   ASSERT(e1);
   ASSERT(e2);
   if(m_currentSel == 0)
   {
     e1->SetReadOnly();
     e2->SetReadOnly();
   }
   e1->SetLimitText(128);
   e2->SetLimitText(128);

   return TRUE;
}

void LangSelectDialogClass::TypeChanged(void)
{
   CListBox *lb = (CListBox *)GetDlgItem(IDC_LS_TYPE_LIST);
   ASSERT(lb);
   int sel = lb->GetCurSel();
   if(sel != LB_ERR)
   {
      CString text;
      CString temp;
      if(m_currentSel)
      {
        GetDlgItemText(IDC_LS_TYPE_EDIT, text);
        wainApp.gs.m_docProp[m_currentSel]->m_extensionType = text;
        lb->DeleteString(m_currentSel);
        lb->InsertString(m_currentSel, text);
        GetDlgItemText(IDC_LS_EXT_EDIT, temp);
        wainApp.gs.m_docProp[m_currentSel]->m_extensions = temp;
        wainApp.gs.m_docProp[m_currentSel]->m_modified = TRUE;
      }
      lb->GetText(sel, text);
      SetDlgItemText(IDC_LS_TYPE_EDIT, text);
      lb = (CListBox *)GetDlgItem(IDC_LS_EXT_LIST);
      ASSERT(lb);
      if(m_currentSel)
      {
        lb->DeleteString(m_currentSel);
        lb->InsertString(m_currentSel, temp);
      }
      lb->SetCurSel(sel);
      lb->GetText(sel, text);
      SetDlgItemText(IDC_LS_EXT_EDIT, text);
      CEdit *e1 = (CEdit *)GetDlgItem(IDC_LS_EXT_EDIT);
      CEdit *e2 = (CEdit *)GetDlgItem(IDC_LS_TYPE_EDIT);
      ASSERT(e1);
      ASSERT(e2);
      e1->SetReadOnly(sel == 0);
      e2->SetReadOnly(sel == 0);
      m_currentSel = sel;
   }
}

void LangSelectDialogClass::ExtChanged(void)
{
   CListBox *lb = (CListBox *)GetDlgItem(IDC_LS_EXT_LIST);
   ASSERT(lb);
   int sel = lb->GetCurSel();
   if(sel != LB_ERR)
   {
     CString text;
     CString temp;
     if(m_currentSel)
     {
       GetDlgItemText(IDC_LS_EXT_EDIT, text);
       wainApp.gs.m_docProp[m_currentSel]->m_extensions = text;
       lb->DeleteString(m_currentSel);
       lb->InsertString(m_currentSel, text);
       GetDlgItemText(IDC_LS_TYPE_EDIT, temp);
       wainApp.gs.m_docProp[m_currentSel]->m_extensionType = temp;
       wainApp.gs.m_docProp[m_currentSel]->m_modified = TRUE;
     }
     lb->GetText(sel, text);
     SetDlgItemText(IDC_LS_EXT_EDIT, text);
     lb = (CListBox *)GetDlgItem(IDC_LS_TYPE_LIST);
     ASSERT(lb);
     lb->SetCurSel(sel);
     if(m_currentSel)
     {
       lb->DeleteString(m_currentSel);
       lb->InsertString(m_currentSel, temp);
     }
     lb->GetText(sel, text);
     SetDlgItemText(IDC_LS_TYPE_EDIT, text);
     CEdit *e1 = (CEdit *)GetDlgItem(IDC_LS_EXT_EDIT);
     CEdit *e2 = (CEdit *)GetDlgItem(IDC_LS_TYPE_EDIT);
     ASSERT(e1);
     ASSERT(e2);
     e1->SetReadOnly(sel == 0);
     e2->SetReadOnly(sel == 0);
     m_currentSel = sel;
   }
}

void LangSelectDialogClass::Select(void)
{
  GetMf()->MakePropSheet(m_currentSel);
}

void MainFrame::DocPropSetup(void)
{
   WainView *av = GetActiveView();
   if(av)
   {
      int index = av->GetDocument()->GetPropIndex();
      MakePropSheet(index);
   }
   else
   {
      SetStatusText("No active view");
   }
}

void MainFrame::MakePropSheet(int _currSel)
{
   // Make a copy of the properties
   DocPropClass* docProp = new DocPropClass(*wainApp.gs.m_docProp[_currSel]);
   CPropertySheet setup("Document Properties", this);
   DocColorDialogClass color(docProp->m_color, NOF_KEY_INDEX, KeyIndexStr);
   KeywordSetupDialog keywords(docProp);
   LangOptionPropertyClass lang_option(docProp);
   TagSetupPropClass tag_options(docProp);
   TemplateProp template_options(docProp);
   setup.AddPage((CPropertyPage *)&color);
   setup.AddPage((CPropertyPage *)&keywords);
   setup.AddPage((CPropertyPage *)&lang_option);
   setup.AddPage((CPropertyPage *)&tag_options);
   setup.AddPage((CPropertyPage *)&template_options);
   setup.m_psh.dwFlags |= PSH_NOAPPLYNOW;
   if(setup.DoModal() == IDOK)
   {
      docProp->m_modified = true;     // The extension file will be saved when Wain is closing down (write_profile()).
      for(int i = 0; i < 5; i++)
      {
         auto& kwList = docProp->m_keyWordList[i];
         if(docProp->m_ignoreCase)
         {
            StringIgnoreCaseCompareClass stringIgnoreCaseCompare;
            std::sort(kwList.begin(), kwList.end(), stringIgnoreCaseCompare);
         }
         else
         {
            std::sort(kwList.begin(), kwList.end());
         }
         kwList.erase(unique(kwList.begin(), kwList.end()), kwList.end());
      }
      *wainApp.gs.m_docProp[_currSel] = *docProp;
      GetMf()->UpdateViews(TRUE);
   }
   delete docProp;
}

void LangSelectDialogClass::OnOk(void)
{
  TypeChanged();
  ExtChanged();
  EndDialog(IDOK);
}

void LangSelectDialogClass::Clone(void)
{
  if(m_currentSel >= 0)
  {
    LangSelectDialogNewTypeDialogClass lsnt(this);
    if(lsnt.DoModal() == IDOK)
    {
      std::string temp = wainApp.gs.m_configPath;
      if(lsnt.m_fileName.find('.') == std::string::npos)
         lsnt.m_fileName += ".ext";
      temp += lsnt.m_fileName;
      DocPropClass *dp = new DocPropClass(*wainApp.gs.m_docProp[m_currentSel]);
      dp->m_extensions = lsnt.m_ext;
      dp->m_extensionType = lsnt.m_type;
      dp->m_fileName = temp;
      dp->m_modified = TRUE;

      wainApp.gs.m_docProp.push_back(dp);
      m_currentSel = wainApp.gs.m_docProp.size() -1;
      CListBox *lb = (CListBox *)GetDlgItem(IDC_LS_TYPE_LIST);
      ASSERT(lb);
      lb->AddString(lsnt.m_type.c_str());
      lb->SetCurSel(m_currentSel);
      SetDlgItemString(IDC_LS_TYPE_EDIT, lsnt.m_type);
      SetDlgItemString(IDC_LS_EXT_EDIT,  lsnt.m_ext);
      lb = (CListBox *)GetDlgItem(IDC_LS_EXT_LIST);
      ASSERT(lb);
      lb->AddString(lsnt.m_ext.c_str());

      lb->SetCurSel(m_currentSel);
      SetDlgItemString(IDC_LS_EXT_EDIT, lsnt.m_ext);
    }
  }
}

void LangSelectDialogClass::OnNew(void)
{
   LangSelectDialogNewTypeDialogClass lsnt(this);
   if(lsnt.DoModal() == IDOK)
   {
      DocPropClass *dp = new DocPropClass;
      dp->CreateDefault();
      dp->m_extensions = lsnt.m_ext;
      dp->m_extensionType = lsnt.m_type;
      std::string Temp = wainApp.gs.m_configPath;
      if(lsnt.m_fileName.find('.') == std::string::npos)
        lsnt.m_fileName += ".ext";
      Temp += lsnt.m_fileName;
      dp->m_fileName = Temp;
      dp->m_modified = TRUE;
      wainApp.gs.m_docProp.push_back(dp);
      m_currentSel = wainApp.gs.m_docProp.size() - 1;
      CListBox *lb = (CListBox *)GetDlgItem(IDC_LS_TYPE_LIST);
      ASSERT(lb);
      lb->AddString(lsnt.m_type.c_str());
      lb->SetCurSel(m_currentSel);
      SetDlgItemString(IDC_LS_TYPE_EDIT, lsnt.m_type);
      SetDlgItemString(IDC_LS_EXT_EDIT,  lsnt.m_ext);
      lb = (CListBox *)GetDlgItem(IDC_LS_EXT_LIST);
      ASSERT(lb);
      lb->AddString(lsnt.m_ext.c_str());

      lb->SetCurSel(m_currentSel);
      SetDlgItemString(IDC_LS_EXT_EDIT, lsnt.m_ext);
   }
}

void LangSelectDialogClass::Remove(void)
{
  if(m_currentSel != -1 && m_currentSel < wainApp.gs.m_docProp.size())
  {
    std::string msg = "Are you sure you want to remove the:\r\n";
    msg += wainApp.gs.m_docProp[m_currentSel]->m_extensionType;
    if(WainMessageBox(this, msg.c_str(), IDC_MSG_YES | IDC_MSG_NO, IDI_QUESTION_ICO) == IDC_MSG_YES)
    {
      /* Save a copy of the old ext-file */
      std::string fn;
      MySplitPath(wainApp.gs.m_docProp[m_currentSel]->m_fileName.c_str(), SP_DRIVE | SP_DIR | SP_FILE, fn);
      fn += ".ex";
      if(rename(wainApp.gs.m_docProp[m_currentSel]->m_fileName.c_str(), fn.c_str()))
      { /* We could not rename it, so remove it */
        ::remove(fn.c_str());
        if(::rename(wainApp.gs.m_docProp[m_currentSel]->m_fileName.c_str(), fn.c_str()))
           ::remove(wainApp.gs.m_docProp[m_currentSel]->m_fileName.c_str());
      }
      delete wainApp.gs.m_docProp[m_currentSel];

      wainApp.gs.m_docProp.erase(wainApp.gs.m_docProp.begin() + m_currentSel);

      CListBox *lb = (CListBox *)GetDlgItem(IDC_LS_TYPE_LIST);
      ASSERT(lb);
      lb->DeleteString(m_currentSel);
      lb->SetCurSel(0);
      lb = (CListBox *)GetDlgItem(IDC_LS_EXT_LIST);
      ASSERT(lb);
      lb->DeleteString(m_currentSel);
      lb->SetCurSel(0);
      m_currentSel = 0;
      SetDlgItemText(IDC_LS_TYPE_EDIT, wainApp.gs.m_docProp[0]->m_extensionType.c_str());
      SetDlgItemText(IDC_LS_EXT_EDIT,  wainApp.gs.m_docProp[0]->m_extensions.c_str());
    }
  }
}


BEGIN_MESSAGE_MAP(LangSelectDialogClass, CDialog)
  ON_LBN_SELCHANGE(IDC_LS_TYPE_LIST, TypeChanged)
  ON_LBN_SELCHANGE(IDC_LS_EXT_LIST,  ExtChanged)
  ON_BN_CLICKED(IDC_LS_SELECT, Select)
  ON_BN_CLICKED(IDOK, OnOk)
  ON_BN_CLICKED(IDC_LS_CLONE, Clone)
  ON_BN_CLICKED(IDC_LS_NEW, OnNew)
  ON_BN_CLICKED(IDC_LS_REMOVE, Remove)
END_MESSAGE_MAP();

LangSelectDialogNewTypeDialogClass::LangSelectDialogNewTypeDialogClass(CWnd *parent) : DialogBaseClass(LangSelectDialogNewTypeDialogClass::IDD, parent)
{
  m_type = "";
  m_ext = "";
  m_fileName = "";
}

LangSelectDialogNewTypeDialogClass::~LangSelectDialogNewTypeDialogClass()
{

}

void LangSelectDialogNewTypeDialogClass::DoDataExchange(CDataExchange *aDx)
{
  CDialog::DoDataExchange(aDx);
  DdxString(aDx, IDC_LS_N_TYPE_EDIT, m_type);
  DdxString(aDx, IDC_LS_N_EXT_EDIT, m_ext);
  DdxString(aDx, IDC_LS_N_FILE_EDIT, m_fileName);
}

BOOL LangSelectDialogNewTypeDialogClass::OnInitDialog()
{
  return CDialog::OnInitDialog();
}

void LangSelectDialogNewTypeDialogClass::OnOK(void)
{
  UpdateData(TRUE);
  EndDialog(IDOK);
}

BEGIN_MESSAGE_MAP(LangSelectDialogNewTypeDialogClass, CDialog)
  ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP();


void DocPropClass::Assign(const DocPropClass &rhs)
{
   int i, j;
   for(i = 0; i < 5; i++)
   {
      m_keyWordList[i].clear();
      for(j = 0; j < rhs.m_keyWordList[i].size(); j++)
         m_keyWordList[i].push_back(rhs.m_keyWordList[i][j]);
   }
   memcpy(m_color, rhs.m_color, sizeof(m_color));
   m_fileName = rhs.m_fileName;
   m_extensions = rhs.m_extensions;
   m_extensionType = rhs.m_extensionType;
   m_seps = rhs.m_seps;
   m_stringDelim = rhs.m_stringDelim;
   m_chDelim = rhs.m_chDelim;
   m_braces = rhs.m_braces;
   m_blockBegin = rhs.m_blockBegin;
   m_blockEnd = rhs.m_blockEnd;
   m_lineComment = rhs.m_lineComment;
   m_commentBegin = rhs.m_commentBegin;
   m_commentEnd = rhs.m_commentEnd;
   m_preProcessor = rhs.m_preProcessor;
   m_literal = rhs.m_literal;
   m_lineCon = rhs.m_lineCon;
   m_commentWholeWord = rhs.m_commentWholeWord;
   m_commentFirstWord = rhs.m_commentFirstWord;
   m_ignoreCase = rhs.m_ignoreCase;
   m_modified = rhs.m_modified;
   m_useTagColor = rhs.m_useTagColor;

   m_tagProgram = rhs.m_tagProgram;
   m_tagOptions = rhs.m_tagOptions;
   m_tagFile = rhs.m_tagFile;

   m_indentAfter = rhs.m_indentAfter;
   m_indentUnless = rhs.m_indentUnless;
   m_unindent = rhs.m_unindent;

   m_tabPos.clear();
   for(i = 0; i < rhs.m_tabPos.size(); i++)
      m_tabPos.push_back(rhs.m_tabPos[i]);

   m_preProcWord.clear();
   for(i = 0; i < rhs.m_preProcWord.size(); i++)
      m_preProcWord.push_back(rhs.m_preProcWord[i]);

   m_numberStr = rhs.m_numberStr;
   m_numberRegEx = std::regex(rhs.m_numberStr);
   std::vector<TemplateListClass *>::iterator idx;
   for(idx = m_templateList.begin(); idx < m_templateList.end(); idx++)
      delete *idx;
   m_templateList.clear();
   std::vector<TemplateListClass *>::const_iterator it;
   for(it = rhs.m_templateList.begin(); it != rhs.m_templateList.end(); it++)
      m_templateList.push_back(new TemplateListClass(*it));

   m_tabSize = rhs.m_tabSize;
   m_indent = rhs.m_indent;
   m_blockBeginWords = rhs.m_blockBeginWords;
   m_blockEndWords = rhs.m_blockEndWords;
}

void DocPropClass::CreateDefault()
{
   int i;
   for(i = 0; i < NOF_KEY_INDEX; i++)
   {
      m_color[i].m_textColor = 0;
      m_color[i].m_backColor = RGB(255, 255, 255);
   }
   m_color[MARK_IDX].m_textColor = RGB(255, 255, 255);
   m_color[MARK_IDX].m_backColor = RGB(128, 128, 128);
   m_tabSize = 3;
   m_indent = 3;
   m_useTagColor = false;
}

int TabPosClass::GetNextTabPos(int aPos)
{
   size_type idx;
   if(!size())
      return -1;

   for(idx = 0; idx < size(); idx++)
      if(at(idx) > aPos)
         return at(idx);
   return -1;
}

int TabPosClass::GetPrevTabPos(int aPos)
{
   size_type idx;
   if(!size())
      return -1;
   if(size() == 1)
     return at(0) <= aPos ? at(0) : -1;
   if(aPos < at(0))
     return 1;

   for(idx = 1; idx < size(); idx++)
      if(at(idx) >= aPos)
         return at(idx - 1);
   return at(size() - 1);
}

void TabPosClass::FromString(const std::string &aStr)
{
   std::string Temp(aStr), T;
   clear();
   do
   {
      T = StrTok(Temp, ", ");
      if(!T.empty())
      {
         int n = ::FromString<int>(T);
         if(n > 0)
           push_back(n);
      }
   }
   while(!T.empty());

   std::sort(begin(), end());
}

std::string TabPosClass::ToString()
{
   size_type idx;
   std::string Ret;
   if(size())
   {
      Ret = ::ToString(at(0));
      for(idx = 1; idx < size(); idx++)
      {
         Ret += ", ";
         Ret += ::ToString(at(idx));
      }
   }
   return Ret;
}
