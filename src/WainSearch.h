//=============================================================================
// This source code file is a part of Wain.
// It defines SearchDialog_class and ReplaceDialog_class.
//=============================================================================
#ifndef SEARCH_H_INC
#define SEARCH_H_INC

#include <regex>

class SearchDialogClass : public CDialog
{
  DECLARE_DYNAMIC(SearchDialogClass);
  CString m_searchString;
  BOOL m_forward;
  int m_fromStart;
  bool m_first;
  int m_ignoreCase;
  int m_useRegex;
public:
  SearchDialogClass(CWnd *parent = NULL);
  ~SearchDialogClass(void);
  enum {IDD = IDD_SEARCH_DIALOG};
  void SetSearchString(const std::string &aStr);
  std::regex m_regEx;
protected:
  virtual void DoDataExchange(CDataExchange *dx);
  virtual BOOL OnInitDialog();
  virtual void OnCancel();
  afx_msg void SearchNext(void);
  afx_msg void ForwardButton(void);
  afx_msg void BackwardButton(void);
  afx_msg void TextChanged(void);
  DECLARE_MESSAGE_MAP();
};

class ReplaceDialogClass : public DialogBaseClass
{
   DECLARE_DYNAMIC(ReplaceDialogClass);
   std::string m_searchString;
   std::string m_replaceString;
   bool m_forward;
   bool m_fromStart;
   bool m_ignoreCase;
   bool m_first;
   bool m_isGlobal;
   bool DoReplace(void);
   bool SearchNext(void);
   void GetSearchString(void); // sets SearchString
   void GetReplaceString(void); // sets ReplaceString
   void ClearFromStart(void);
public:
   enum ReplaceResultEnum
   {
      ReplaceEnd,
      ReplaceContinue,
      ReplaceDone
   };
   ReplaceDialogClass(CWnd *parent = NULL);
   ~ReplaceDialogClass(void);
   enum {IDD = IDD_REPLACE_DIALOG};
   void SetSearchString(const std::string &aStr);
   void SetGlobal(bool aIsGlobal);
protected:
   virtual void DoDataExchange(CDataExchange *dx);
   virtual BOOL OnInitDialog();
   virtual void OnCancel();
   afx_msg void ReplaceAll(void);
   afx_msg void ReplaceNext(void);
   afx_msg void ReplaceSkip(void);
   afx_msg void ForwardButton(void);
   afx_msg void BackwardButton(void);
   afx_msg void TextChanged(void);
   DECLARE_MESSAGE_MAP();
};

extern UINT search_message_id;

#endif // #ifnded SEARCH_H_INC


