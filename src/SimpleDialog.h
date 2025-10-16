
#include ".\..\src\Resource.h"
class SimpleDialog : public CDialog
{
  DECLARE_DYNAMIC(SimpleDialog);
public:
  SimpleDialog(const char *cap, const char *init_msg, CWnd *parent = NULL);
  ~SimpleDialog(void);
  CString m_msg;
  CString m_caption;
  enum {IDD = IDD_SIMPLE_DIALOG};
protected:
  virtual BOOL OnInitDialog(void);
  afx_msg void OnOk(void);
  DECLARE_MESSAGE_MAP();
};

