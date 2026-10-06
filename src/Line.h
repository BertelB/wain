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

enum class DeleteTypeType
{
  NormalDelete,
  TabDelete,
  LineDelete,
  SpecialTabDelete,
  NoOnDelete
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
   TxtLine* m_next;
   TxtLine* m_prev;
   WainDoc* m_doc;
   char* m_text;
   int32_t m_len;
   int32_t m_tabLen;
   int m_allocLen;
   void CalcTabLen(void);
   int32_t TextPosToScreenPos(int32_t _pos) const;
   int32_t ScreenPosToTextPos(int32_t _pos) const;
   int32_t SpaceBefore(int32_t _pos) const;
   int32_t SpaceAfter(int32_t _pos) const;

   void MakeSpace(size_t _newLen);
public:
   int GetRawLen(int _from, int _to, BOOL _fixed) const;
   int CopyRaw(char* _dest, int _from, int _to, BOOL _fixed) const;
   void ToUpper(int _from = 0, int _to = -1);
   void ToLower(int _from = 0, int _to = -1);
   int GetTabLen(void) const;
   int GetTextLen(void) const;
   EndLineStatusType m_endLineStatus;
   TxtLine(WainDoc* _doc);
   TxtLine(void);
   ~TxtLine();
   void operator = (const char* _newText);
   void RemoveFrom(int column, char* _rest);
   bool InsertAt(int _pos, const char *_str, int32_t _length = -1);
   bool Append(char _ch);
   bool InsertAt(int _pos, char _ch);
   char ReplaceAt(int _pos, char _ch);
   bool SplitLineAt(int _pos);
   DeleteTypeType DeleteAt(int _pos);
   DeleteTypeType DeleteAt(int _start, int _end);
   const char* GetText(void) const { return m_text; }
   bool CopyTextAt(char* _buf, int _pos, int _len) const;
   char GetTextAt(int _pos) const;
   int GetWordLenRight(int _pos) const;
   int GetWordLenLeft(int _pos) const;
   int GetSpaceLenRight(int _pos) const;
   int GetSpaceLenLeft(int _pos) const;
   int GetSepLenRight(int _pos) const;
   int GetSepLenLeft(int _pos) const;
   int GoWordRight(int _pos) const;
   int32_t GoWordLeft(int32_t _pos) const;
   EndLineStatusType GetTextColor(class TxtLineColor& _lineColor, EndLineStatusType _prevEls, bool _justStatus = false) const;
   bool CheckStr(const char* _str, const std::string& _match, ColorIndexType _indexType, uint32_t& _size, TxtLineColor& _txtLine) const;

   void StripWhitespace(void);
   int GetEmptyCharBefore(int _pos) const;
   bool CheckSpace(int _from, int _to) const;
   void SetModified(ModifyStatusType _newStatus);
   ModifyStatusType m_modifyStatus;
   void RemoveTabs(void);
   int GetSpaceInFront() const;

   const char *FindFirstNotOf(int _firstPos, const char *chars) const;
   int FindChar(int _firstPos, char _char, int dir);
};

#endif
