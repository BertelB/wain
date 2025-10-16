#ifndef _LINE_H_INC
#define _LINE_H_INC

enum EndLineStatusType
{
  NO_ELS,
  COMMENT_ELS,
  PREPROC_ELS,
  LINE_COMMENT_ELS,
  COMMENT_IN_PREPROC_ELS
};

enum DeleteTypeType
{
  normal_delete,
  tab_delete,
  line_delete,
  special_tab_delete,
  nOnDelete
};

enum ModifyStatusType
{
   NOT_MODIFIED,
   IS_MODIFIED,
   MODIFIED_SAVED
};

enum GetStrLineEndType
{
   NORMAL_LINE_END,
   CR_LF_LINE_END,
   CR_LINE_END,
   EOF_LINE_END
};

class TxtLine
{
   friend class WainDoc;
private:
   TxtLine *m_next;
   TxtLine *m_prev;
   WainDoc *m_doc;
   char *m_text;
   int m_len;
   int m_tabLen;
   int m_allocLen;
   void CalcTabLen(void);
   int TextPosToScreenPos(int pos) const;
   int ScreenPosToTextPos(int pos) const;
   int SpaceBefore(int pos) const ;
   int SpaceAfter(int pos) const;

   void MakeSpace(int new_len);
public:
   int GetRawLen(int _from, int _to, BOOL _fixed) const;
   int CopyRaw(char* _dest, int _from, int _to, BOOL _fixed) const;
   void ToUpper(int from = 0, int to = -1);
   void ToLower(int from = 0, int to = -1);
   int GetTabLen(void) const;
   int GetTextLen(void) const;
   EndLineStatusType m_endLineStatus;
   TxtLine(WainDoc *doc);
   TxtLine(void);
   ~TxtLine();
   void operator = (const char *new_text);
   void RemoveFrom(int column, char *rest);
   bool InsertAt(int pos, const char *str, int length = -1);
   bool Append(char ch);
   bool InsertAt(int pos, char ch);
   char ReplaceAt(int pos, char ch);
   bool SplitLineAt(int pos);
   DeleteTypeType DeleteAt(int pos);
   DeleteTypeType DeleteAt(int start, int end);
   const char *GetText(void) const { return m_text; }
   bool CopyTextAt(char *buf, int pos, int len) const;
   char GetTextAt(int pos) const;
   int GetWordLenRight(int pos) const;
   int GetWordLenLeft(int pos) const;
   int GetSpaceLenRight(int pos) const;
   int GetSpaceLenLeft(int pos) const;
   int GetSepLenRight(int pos) const;
   int GetSepLenLeft(int pos) const;
   int GoWordRight(int pos) const;
   int GoWordLeft(int pos) const;
   EndLineStatusType GetTextColor(class TxtLineColor &line_color, EndLineStatusType prev_els, bool just_status = FALSE) const;
   bool CheckStr(const char *aStr, const std::string &aMatch, ColorIndexType aIndexType, size_t &Size, TxtLineColor &aTxtLine) const;

   void StripWhitespace(void);
   int GetEmptyCharBefore(int pos) const;
   bool CheckSpace(int aFrom, int aTo) const;
   void SetModified(ModifyStatusType new_status);
   ModifyStatusType m_modifyStatus;
   void RemoveTabs(void);
   int GetSpaceInFront() const;
   void HelloWorld(char* /*  _x */);
   const char *FindFirstNotOf(int aFirstPos, const char *aChars) const;
   int FindChar(int aFirstPos, char aChar, int aDir);
};

#endif
