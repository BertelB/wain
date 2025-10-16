#include ".\..\src\stdafx.h"
#include ".\..\src\SimpleDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4100) // unreferenced formal parameter

IMPLEMENT_DYNAMIC(SimpleDialog, CDialog)

BEGIN_MESSAGE_MAP(SimpleDialog, CDialog)
  ON_BN_CLICKED(IDOK, OnOk)
END_MESSAGE_MAP();


SimpleDialog::SimpleDialog(const char *cap, const char *init_msg, CWnd *parent) : CDialog(SimpleDialog::IDD, parent)
{
  m_caption = cap;
  m_msg = init_msg;
}

SimpleDialog::~SimpleDialog()
{

}

BOOL SimpleDialog::OnInitDialog(void)
{
  CEdit *ce = (CEdit *)GetDlgItem(IDC_SIMPLE_EDIT);
  if(ce)
  {
    ce->SetWindowText(m_msg);
  }
  SetWindowText(m_caption);
  return TRUE;
}

void SimpleDialog::OnOk(void)
{
  CEdit *ce = (CEdit *)GetDlgItem(IDC_SIMPLE_EDIT);
  if(ce)
  {
    ce->GetWindowText(m_msg);
  }
  EndDialog(IDOK);
}
