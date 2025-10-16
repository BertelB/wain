//=============================================================================
// This source code file is a part of Wain.
// It defines WainDoc and TxtLine.
//=============================================================================
#ifndef WAINDOC_H_INC
#define WAINDOC_H_INC

#include "../src/Line.h"

#define NOT_A_OFFSET 0x80000000
#define TAB_WIDTH(owner_) (owner_->m_prop->m_tabSize)
#define SEPS(obj_)  (obj_->m_prop->m_seps.c_str())

#define SEARCH_FORWARD      0x00  // flags used in search message
#define SEARCH_BACKWARD     0x01
#define SEARCH_FROM_START   0x02
#define SEARCH_FIRST        0x04
#define SEARCH_IGNORE_CASE  0x10
#define SEARCH_USE_REGEX    0x20
#define REPLACE_ALL         0x40
#include "GlobSet.h"

class WainDoc : public CDocument
{
   TxtLine m_line;

   friend class TxtLine;
   void SetMaxLength(int len);
   bool ReadFile(const char *file_name);
   bool IsFileReadOnly(const char *file_name) const;
   class FileClass *m_file;
   bool m_gotInput;
public:
   std::string m_markWord;
   int m_propIndex; // Make private !!
   DocPropClass* m_prop;
   int GetPropIndex() { return m_propIndex; }
   void GetExtType(const char *aName, bool aAll);
   void ReCalcTabLen(void);

   bool m_unixStyle;
   int m_maxLen;
   bool m_newFile;

   EndLineStatusType GetEndLineStatus(TxtLine *line) const;
   void SetModified(bool mod = true);
   bool GetModified(void);

   void Reset(void);
   int m_lineCount;
   void operator += (const char *new_text);
   TxtLine *GetLineNo(int LineNo);
   TxtLine *GetRelLineNo(int lines, TxtLine *CurrentLine);
   TxtLine *GetNext(TxtLine *aCurr)
   {
      return aCurr->m_next == &m_line ? 0 : aCurr->m_next;
   }
   TxtLine *GetPrev(TxtLine *aCurr)
   {
      return aCurr->m_prev == &m_line ? 0 : aCurr->m_prev;
   }
   TxtLine *SplitLineAt(TxtLine *c_line, int pos);
   TxtLine *InsertLine(TxtLine *l, const char *str);
   TxtLine *InsertLineBefore(TxtLine *l, const char *str);
   TxtLine *RemoveLine(TxtLine *l);
   BOOL JoinLine(TxtLine *l);
   DeleteTypeType DeleteAt(TxtLine *c_line, int pos);
   bool GetCurrentWord(std::string &word, int offset, const TxtLine *CurrentLine) const;
   int RemoveWordCurrent(int offset, TxtLine *CurrentLine);
   EndLineStatusType GetPrevEls(TxtLine *l) const;
   TxtLine *FindString(TxtLine *l, const char *string, int *column, int *line_offset, int flags, size_t *MatchLen = 0);
   TxtLine *FindMatchBrace(TxtLine *l, int *x, int *y, bool allowAfter = false, int* findOff = 0, int* startLen = 0, int* endLen = 0);
   TxtLine *FindMatchBlock(TxtLine *l, int *x, int *y);
   TxtLine *GotoBlockStart(TxtLine *l, int *x, int *y);
   TxtLine *GotoBlockEnd(TxtLine *l, int *x, int *y);

   int GetIndent(TxtLine *aLine, int &aIndent) const;

   void Update(); // Called if I am a tool-window
   void Done();
   void CloseFile();
   bool m_isProject = false;
   bool m_isMake = false;
   uint32_t m_nr = 0;
protected:
   WainDoc();
   DECLARE_DYNCREATE(WainDoc)

public:
   void ReloadFile(void);
   class ChildFrame *m_childFrame;
   class WainView *m_view;
   void DoCloseFile(void);

public:
   virtual BOOL OnNewDocument();
   virtual void Serialize(CArchive &ar);
   virtual void SetPathName( LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE );
public:
   void SetDebugPath(const std::string& _debugPath) { m_debugPath = _debugPath; }
   std::string& GetDebugPath() { return m_debugPath; }
   bool m_isDebugFile;
   bool m_isFtpFile;
   virtual ~WainDoc();
   virtual void OnCloseDocument(void);
   BOOL OnOpenDocument(LPCTSTR lpszPathName);
   BOOL SaveFile(BOOL prompt = TRUE);
   void SaveFileAs(void);
   bool m_readAsStdioFile = false;
#ifdef _DEBUG
   virtual void AssertValid() const;
   virtual void Dump(CDumpContext& dc) const;
#endif
   bool GetReadOnly() const { return m_isReadOnly; }
   void SetReadOnly(bool aReadOnly);
private:
   bool m_isReadOnly;
   std::string m_debugPath;
   bool m_earlyExit = false;
};

#endif // WAINDOC_H_INC
