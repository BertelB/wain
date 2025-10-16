//=============================================================================
// This source code file is a part of Wain
// It defines KeySetupDialogClass.
//=============================================================================
#ifndef KEYSETUP_H_INC
#define KEYSETUP_H_INC

extern BOOL ReadKeySetupFile(const char *file_name, ACCEL *ac, size_t *nof_entrys);
extern BOOL WriteKeySetupFile(const char *file_name, ACCEL *ac, size_t nof_entrys);
extern const char *MsgId2FuncName(WORD id);
extern WORD FuncName2MsgId(const char *name);
extern void KeyState2MenuString(char *text, WORD key, BYTE flags);
extern void KeyState2MenuString(std::string &aText, WORD key, BYTE flags);
extern void MakePopupMenuStrings(std::vector<PopupMenuListClass >&aMenu);
extern void AddPopupMenuAccelerator(std::vector<PopupMenuListClass >&aMenu);
extern void RemovePopupMenuAccelerator(std::vector<PopupMenuListClass >&aMenu);

class KeySetupDialogClass : public CDialog
{
public:
  ACCEL *m_orgAccel;
  ACCEL m_accel[MAX_NOF_ACCELERATORS];
  size_t m_nofAccel;
  int m_orgNofAccel;
  KeySetupDialogClass(CWnd *parent = NULL);
  enum {IDD = IDD_KEY_SETUP};
  CString m_currKeyFile;
private:
  BYTE m_state; // ALT, CTRL and SHIFT state
  enum
  {
     CURR_NONE,
     CURR_FUNC,
     CURR_KEY
  }m_current;

protected:
  virtual void DoDataExchange(CDataExchange *dx);
  virtual BOOL OnInitDialog(void);
protected:

  afx_msg void KeyChanged(void);
  afx_msg void FuncChanged(void);
  afx_msg void AltHit(void);
  afx_msg void CtrlHit(void);
  afx_msg void ShiftHit(void);
  afx_msg void HandleApply(void);
  afx_msg void HandleRemove(void);
  afx_msg void CurrentChanged(void);
  afx_msg void BrowseForFile(void);
  DECLARE_MESSAGE_MAP();
};

class MenuSetupDialogClass : public CDialog
{
public:
  MenuSetupDialogClass(CWnd *aParent = NULL);
  enum {IDD = IDD_POPUP_MENU_SETUP};
  std::vector<PopupMenuListClass> m_popupMenu;

protected:
  virtual void DoDataExchange(CDataExchange *dx);
  virtual BOOL OnInitDialog(void);
  // virtual void OnOK(void);
  virtual void OnCancel(void);
protected:
  afx_msg void Add(void);
  afx_msg void Remove(void);
  afx_msg void Seperator(void);
  afx_msg void Up(void);
  afx_msg void Down(void);
  DECLARE_MESSAGE_MAP();
};

class PopupMenuSetupDialogClass : public MenuSetupDialogClass
{
public:
  PopupMenuSetupDialogClass(CWnd *parent);
  virtual void OnOK(void);
};

class UserMenuSetupDialogClass : public MenuSetupDialogClass
{
public:
  UserMenuSetupDialogClass(CWnd *parent);
  virtual void OnOK(void);
};

#endif // #ifndef KEYSETUP_H_INC


