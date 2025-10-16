//=============================================================================
// This source code file is a part of Wain.
// It implements TagListDialogClass as defined in TagList.h,
// and TagList() and CompletionList() from WainView.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\wain.h"
#include ".\..\src\wainview.h"
#include ".\..\src\taglist.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\childfrm.h"
#include ".\..\src\waindoc.h"
#include ".\..\src\NavigatorList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4100) // unreferenced formal parameter

ClassListDialogClass::ClassListDialogClass(CWnd* _parent, const std::string& _word) :
   CDialog(ClassListDialogClass::IDD, _parent),
   m_word(_word)
{
}

ClassListDialogClass::~ClassListDialogClass()
{
}

void ClassListDialogClass::OnCancel()
{
   EndDialog(IDCANCEL);
}

void ClassListDialogClass::OnOk()
{
   CRect cr;
   GetWindowRect(&cr);
   // wainApp.gs.m_classListCx = cr.right - cr.left;
   // wainApp.gs.m_classListCy = cr.bottom - cr.top;
   EndDialog(IDOK);
}

void ClassListDialogClass::OnPeek(void)
{
}

void ClassListDialogClass::OnType(void)
{
}

void ClassListDialogClass::OnClass(void)
{
#if 0
   bool found = false;
   uint32_t i;
   int nof = m_listBox->GetItemCount();
   for(i = 0, found = false; i < nof && !found; i += found ? 0 : 1)
   {
      if(m_listBox->GetItemState(i, LVIS_SELECTED))
      {
         found = true;
      }
   }

   if(found)
   {
      if (!m_tagList->m_list[i]->m_signature.empty())
      {
         std::string  tagWord = m_tagList->m_list[i]->m_signature;
         std::string::size_type pos = tagWord.find_first_of(" \t");
         if (pos != std::string::npos)
         {
            tagWord = tagWord.substr(0, pos);
         }
         if (GetMf()->m_navigatorDialog.ViewClass(tagWord))
         {
            EndDialog(IDCANCEL);
            GetMf()->m_navigatorDialog.SetFocus();
            GetMf()->m_navigatorDialog.m_navigatorList->SetFocus();
         }
      }
   }
#endif
}

void ClassListDialogClass::OnPeekClass(void)
{
}

void ClassListDialogClass::DoDataExchange(CDataExchange *dx)
{
  CDialog::DoDataExchange(dx);
}

BOOL ClassListDialogClass::OnInitDialog(void)
{
   // m_imageList.Create(IDB_SMALLICONS, 16, 1, RGB(255, 255, 255));
   // if ()
   // {
   //    EndDialog(IDCANCEL);
   //    return TRUE;
   // }
   DoInitDialog();
#if 0
   if (!m_tagList->m_list[i]->m_signature.empty())
   {
      std::string  tagWord = m_tagList->m_list[i]->m_signature;
      std::string::size_type pos = tagWord.find_first_of(" \t");
      if (pos != std::string::npos)
      {
         tagWord = tagWord.substr(0, pos);
      }
      if (GetMf()->m_navigatorDialog.ViewClass(tagWord))
      {
         EndDialog(IDCANCEL);
         GetMf()->m_navigatorDialog.SetFocus();
         GetMf()->m_navigatorDialog.m_navigatorList->SetFocus();
      }
   }
#endif
   return FALSE;
}

BOOL ClassListDialogClass::DoInitDialog()
{
   RECT pr;
   GetMf()->GetWindowRect(&pr);

   MoveWindow(pr.left + (pr.right - pr.left)/2 - wainApp.gs.m_tagListCx/2, pr.top + (pr.bottom - pr.top)/2 - wainApp.gs.m_tagListCy/2,
              wainApp.gs.m_tagListCx, wainApp.gs.m_tagListCy);
   // m_listBox->SetFocus();
   // m_listBox->SetItemState(0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
   CRect cr;
   GetClientRect(&cr);
   m_oldSizeCx = cr.right;
   SetSizes(cr.right, cr.bottom);

   return TRUE;
}

void ClassListDialogClass::OnSize(UINT type, int cx, int cy)
{
  SetSizes(cx, cy);
}

void ClassListDialogClass::SetSizes(int cx, int cy)
{
  CButton *b = (CButton *)GetDlgItem(IDOK);
  if(!b)
    return;
  b->MoveWindow(10, cy - 30, 70, 22);
  b = (CButton *)GetDlgItem(IDCANCEL);
  ASSERT(b);
  b->MoveWindow(90, cy - 30, 70, 22);
  b = (CButton *)GetDlgItem(IDC_TAG_LIST_PEEK);
  ASSERT(b);
  b->MoveWindow(170, cy - 30, 70, 22);
  b = (CButton *)GetDlgItem(IDC_TAG_LIST_TYPE);
  ASSERT(b);
  b->MoveWindow(250, cy - 30, 70, 22);
  b = (CButton *)GetDlgItem(IDC_TAG_LIST_CLASS);
  ASSERT(b);
  b->MoveWindow(330, cy - 30, 70, 22);
  b = (CButton *)GetDlgItem(IDC_TAG_LIST_PEAK_CLASS);
  ASSERT(b);
  b->MoveWindow(410, cy - 30, 70, 22);
}

void ClassListDialogClass::OnGetMinMaxInfo(MINMAXINFO FAR *min_max_info)
{
  CDialog::OnGetMinMaxInfo(min_max_info);
  min_max_info->ptMinTrackSize.x = 260;
  min_max_info->ptMinTrackSize.y = 120;
}
