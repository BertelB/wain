#include ".\..\src\stdafx.h"
#include ".\..\src\Resource.h"
#include ".\..\src\Wain.h"
#include "..\src\MainFrm.h"
#include "../src/TagList.h"
#include "../src/WordListDialog.h"
#include "../src/WainView.h"
#include "../src/WainDoc.h"
#include "../src/DocProp.h"
#include "../src/SimpleDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#pragma warning (disable : 4100) // unreferenced formal parameter

class TreeWordInfo
{
public:
   enum class EntryType
   {
      Folder, File, Line
   };

   TreeWordInfo(const char* _text, EntryType _entryType, uint32_t _counter, uint32_t _lineIdx) :
      m_text(strdup(_text)),
      m_entryType(_entryType),
      m_counter(_counter),
      m_lineIdx(_lineIdx)
   {
   }
   ~TreeWordInfo()
   {
      free(m_text);
   }
   char* m_text = 0;
   uint32_t m_counter = 0;
   EntryType m_entryType;
   uint32_t m_lineIdx;
};

BEGIN_MESSAGE_MAP(WordListDialog, CDialog)
  ON_BN_CLICKED(IDOK,         OnOk)
  ON_LBN_DBLCLK(IDC_WORD_LIST, OnOk)
  ON_BN_CLICKED(IDC_WORD_LIST_PEEK, OnPeek)
  ON_WM_SIZE()
  ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP();

IMPLEMENT_DYNAMIC(WordListDialog, CDialog)

void fake() {}

WordListDialog::WordListDialog(CWnd* _parent, const std::string& _word, std::vector<WordInfo>& _wordInfo, uint32_t _selectedWord) :
   CDialog(WordListDialog::IDD, _parent),
   m_word(_word),
   m_parent(_parent),
   m_wordInfo(_wordInfo),
   m_selectedIndex(_selectedWord)
{
   m_oldSizeCx = -1;
}

void WordListDialog::ClearTree(HTREEITEM _item)
{
   HTREEITEM child = m_tree.GetChildItem(_item);
   while(child)
   {
      HTREEITEM next = m_tree.GetNextSiblingItem(child);
      ClearTree(child);
      child = next;
   }
   TVITEM tvItem;
   memset(&tvItem, 0, sizeof(tvItem));
   tvItem.hItem = _item;
   tvItem.mask = TVIF_PARAM;
   if (m_tree.GetItem(&tvItem))
   {
      TreeWordInfo* e = (TreeWordInfo*)tvItem.lParam;
      delete e;
   }
   m_tree.DeleteItem(_item);
}


WordListDialog::~WordListDialog()
{
}

void WordListDialog::OnCancel()
{
   Close();
   EndDialog(IDCANCEL);
}

void WordListDialog::OnOk()
{
   HTREEITEM selectedItem = m_tree.GetSelectedItem();
   TVITEM tvItem;
   tvItem.hItem = selectedItem;
   tvItem.mask = TVIF_PARAM;
   if(m_tree.GetItem(&tvItem))
   {
      TreeWordInfo* e = (TreeWordInfo*)tvItem.lParam;
      if (e->m_entryType == TreeWordInfo::EntryType::Line)
      {
         m_selectedIndex = e->m_lineIdx;
         m_ok = true;
         m_isPeek = false;
         Close();
         EndDialog(IDOK);
         return;
      }
   }
   SetStatusText("No line selected");
}

void WordListDialog::OnPeek()
{
   HTREEITEM selectedItem = m_tree.GetSelectedItem();
   TVITEM tvItem;
   tvItem.hItem = selectedItem;
   tvItem.mask = TVIF_PARAM;
   if(m_tree.GetItem(&tvItem))
   {
      TreeWordInfo* e = (TreeWordInfo*)tvItem.lParam;
      if (e->m_entryType == TreeWordInfo::EntryType::Line)
      {
         m_selectedIndex = e->m_lineIdx;
         m_ok = true;
         m_isPeek = true;
         Close();
         EndDialog(IDOK);
         return;
      }
   }
   SetStatusText("No line selected");
}

void WordListDialog::DoDataExchange(CDataExchange *dx)
{
   CDialog::DoDataExchange(dx);
}

BOOL WordListDialog::OnInitDialog(void)
{
   if (DoInitDialog())
   {
      EndDialog(IDCANCEL);
      return TRUE;
   }
   return FALSE;
}

extern void SplitPath(char **part, const char *fn); // Fixme
void WordListDialog::ExpandTree(HTREEITEM item)
{
   while(item)
   {
      TVITEM tvItem;
      tvItem.hItem = item;
      tvItem.mask = TVIF_PARAM;
      if(m_tree.GetItem(&tvItem))
      {
         TreeWordInfo* e = (TreeWordInfo*)tvItem.lParam;
         if (e->m_entryType == TreeWordInfo::EntryType::Folder)
         {
            m_tree.Expand(item, TVE_EXPAND);
            HTREEITEM child = m_tree.GetChildItem(item);
            ExpandTree(child);
         }
         else if (m_wordInfo.size() < 10)
         {
            m_tree.Expand(item, TVE_EXPAND);
         }
      }
      item = m_tree.GetNextSiblingItem(item);
   }
}

BOOL WordListDialog::DoInitDialog()
{
   CRect cr;
   GetClientRect(&cr);
   cr.DeflateRect(5, 5);
   cr.bottom -= 40;

   m_tree.Create(TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_DISABLEDRAGDROP | TVS_NOTOOLTIPS | WS_VISIBLE | WS_TABSTOP | WS_CHILD, cr, this, IDC_WORD_LIST);
   m_tree.ModifyStyleEx(0, WS_EX_CLIENTEDGE, FALSE);
   uint32_t entryIdx = 0;
   for(uint32_t i = 0; i < m_wordInfo.size(); )
   {
      HTREEITEM item = m_tree.GetRootItem();
      HTREEITEM candidate = 0;
      int candidateIdx = -1;
      char *pathPart[MAX_PATH/2];
      SplitPath(pathPart, m_wordInfo[i].m_fileName.c_str());
      bool found = false;
      int j = 0;
      while(!found && item)
      {
         CString p = m_tree.GetItemText(item);
         if(pathPart[j] && !stricmp(pathPart[j], p))
         {
            do
            {
               candidate = item;
               candidateIdx = j;
               item = m_tree.GetChildItem(item);
               p = m_tree.GetItemText(item);
               j++;
            }
            while(item && pathPart[j] && !stricmp(pathPart[j], p));
         }
         else
         {
            item = m_tree.GetNextSiblingItem(item);
         }
      }
      TVINSERTSTRUCT insStr;
      insStr.hParent = candidate;
      insStr.item.mask = TVIF_TEXT | TVIF_PARAM;
      insStr.hInsertAfter = 0;
      j = candidateIdx + 1;
      while(pathPart[j])
      {
         TreeWordInfo::EntryType type = pathPart[j + 1] ? TreeWordInfo::EntryType::Folder : TreeWordInfo::EntryType::File;
         TreeWordInfo* e = new TreeWordInfo(pathPart[j++], type, entryIdx++, i);
         insStr.item.lParam = (LPARAM)e;
         insStr.item.pszText = e->m_text;
         candidate = m_tree.InsertItem(&insStr);
         insStr.hParent = candidate;
      }
      for(j = 0; pathPart[j]; j++)
      {
         free(pathPart[j]);
      }
      uint32_t oldI = i;
      for (; i < m_wordInfo.size() && m_wordInfo[i].m_fileName == m_wordInfo[oldI].m_fileName; i++)
      {
         TVINSERTSTRUCT insLine;
         insLine.hParent = candidate;
         insLine.item.mask = TVIF_TEXT | TVIF_PARAM;
         insLine.hInsertAfter = 0;
         TreeWordInfo* e = new TreeWordInfo(m_wordInfo[i].m_line.c_str(), TreeWordInfo::EntryType::Line, entryIdx++, i);
         insLine.item.lParam = (LPARAM)e;
         insLine.item.pszText = e->m_text;
         HTREEITEM hItem = m_tree.InsertItem(&insLine);
         if (i == m_selectedIndex)
         {
            m_tree.SelectItem(hItem);
         }
      }
   }
   ExpandTree(m_tree.GetRootItem());
   RECT pr;
   GetMf()->GetWindowRect(&pr);
   MoveWindow(pr.left + (pr.right - pr.left)/2 - wainApp.gs.m_wordListCx/2, pr.top + (pr.bottom - pr.top)/2 - wainApp.gs.m_wordListCy/2,
               wainApp.gs.m_wordListCx, wainApp.gs.m_wordListCy);
   m_tree.SetFocus();
   GetClientRect(&cr);
   m_oldSizeCx = cr.right;
   SetSizes(cr.right, cr.bottom);
   return FALSE;
}

void WordListDialog::OnSize(UINT type, int cx, int cy)
{
   SetSizes(cx, cy);
}

void WordListDialog::SetSizes(int cx, int cy)
{
  CButton *b = (CButton *)GetDlgItem(IDOK);
  if(!b)
    return;
  b->MoveWindow(10, cy - 30, 70, 22);
  b = (CButton *)GetDlgItem(IDCANCEL);
  ASSERT(b);
  b->MoveWindow(90, cy - 30, 70, 22);
  b = (CButton *)GetDlgItem(IDC_WORD_LIST_PEEK);
  ASSERT(b);
  b->MoveWindow(170, cy - 30, 70, 22);
  m_tree.MoveWindow(10, 10, cx - 20, cy - 50);
}

void WordListDialog::OnGetMinMaxInfo(MINMAXINFO FAR *min_max_info)
{
  CDialog::OnGetMinMaxInfo(min_max_info);
  min_max_info->ptMinTrackSize.x = 260;
  min_max_info->ptMinTrackSize.y = 120;
}

void WordListDialog::Close()
{
   RECT cr;
   GetWindowRect(&cr);
   wainApp.gs.m_wordListCx = cr.right - cr.left;
   wainApp.gs.m_wordListCy = cr.bottom - cr.top;
   HTREEITEM item = m_tree.GetRootItem();
   if(item)
      ClearTree(item);
}

void WainView::WordList(void)
{
   std::string word;
   WainDoc *doc = GetDocument();
   if (!doc->GetCurrentWord(word, m_columnNo, m_currentTextLine))
   {
      SimpleDialog sd("The word to find", "", this);
      if(sd.DoModal() == IDOK)
      {
         word = sd.m_msg;
      }
      else
      {
         return;
      }
   }
   GetMf()->m_lastWord = word;
   DoWordList(word, 0, GetDocument()->GetPropIndex());
}

void WainView::WordListRedo()
{
   if (!GetMf()->m_lastWord.empty())
      DoWordList(GetMf()->m_lastWord, GetMf()->m_lastSelectedWord, GetDocument()->GetPropIndex());
   else
      SetStatusText("No word to redo");
}

void WainView::DoWordList(std::string& _word, uint32_t _lastSelectedWord, int _propIndex)
{
   std::vector<WordInfo> wordInfo;
   WainDoc *doc = GetDocument();
   if (GetMf()->m_navigatorDialog.m_project->GetWordInfo(wordInfo, _word, _propIndex))
   {
      WordListDialog wl(this, _word, wordInfo, _lastSelectedWord);
      int result = wl.DoModal();
      if(result == IDOK && wl.m_ok)
      {
         GetMf()->m_lastSelectedWord = wl.m_selectedIndex;
         if (!wl.m_isPeek)
         {
            if(doc->GetPathName().IsEmpty())
               SetStatusText("Unable to add Undo jump to tag entry");
            else
               GetMf()->AddJumpToTag(doc->GetPathName(), m_lineNo, m_columnNo);
            WainDoc* nDoc = wainApp.OpenDocument(wordInfo[wl.m_selectedIndex].m_fileName.c_str());
            if(nDoc)
            {
               nDoc->m_view->GotoLineNo(wordInfo[wl.m_selectedIndex].m_lineNo - 1);
            }
         }
         else
         {
            GetMf()->CloseDebugFile(wainApp.m_lastAutoTagFile.c_str(), false, false);
            WainDoc *pDoc = GetMf()->OpenDebugFile(wordInfo[wl.m_selectedIndex].m_fileName.c_str(), TagPeekDebugFile, true, "", false, false, 0, 0);
            if(pDoc)
            {
               pDoc->m_view->GotoLineNo(wordInfo[wl.m_selectedIndex].m_lineNo - 1);
               wainApp.m_lastAutoTagView = pDoc->m_view;
            }
            wainApp.m_lastAutoTagFile = wordInfo[wl.m_selectedIndex].m_fileName.c_str();
            if(pDoc->m_view)
            {
               pDoc->m_view->SetFocus();
            }
         }
      }
   }
}
