#ifndef _WORD_LIST_DIALOG_H_INC
#define _WORD_LIST_DIALOG_H_INC
#include <string>
#include "../src/WordListThread.h"

class WordListDialog : public CDialog
{
   DECLARE_DYNAMIC(WordListDialog);
   CWnd *m_parent;
   void SetSizes(int aCx, int aCy);
   int m_oldSizeCx;
public:
   CTreeCtrl m_tree;
   uint32_t m_selectedIndex = 0;
   bool m_ok = false;
   bool m_isPeek = false;
   std::string m_word;
   std::vector<WordInfo>& m_wordInfo;
   WordListDialog(CWnd *aParent, const std::string &aWord, std::vector<WordInfo>& _wordInfo, uint32_t _selectedWord);
   virtual ~WordListDialog(void);
   virtual BOOL OnInitDialog(void);
   BOOL DoInitDialog();
   enum {IDD = IDD_WORD_LIST_DIALOG};
   virtual void OnCancel();
   virtual void OnOk();
protected:
   void ExpandTree(HTREEITEM item);
   void ClearTree(HTREEITEM item);
   void DoDataExchange(CDataExchange *dx);
	void Close();
   afx_msg void OnSize(UINT type, int cx, int cy);
   afx_msg void OnPeek(void);
   afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR *min_max_info);
   DECLARE_MESSAGE_MAP();
};
#endif
