//=============================================================================
// This source code file is a part of Wain.
// It implements TxtLine as defined in YateDoc.h.
//=============================================================================
#include ".\..\src\stdafx.h"
#include ".\..\src\wain.h"
#include ".\..\src\MainFrm.h"
#include ".\..\src\WainDoc.h"
#include ".\..\src\tabview.h"
#include ".\..\src\Wainview.h"
#include ".\..\src\WainDocUtil.h"
#include ".\..\src\DocProp.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4100) // unreferenced formal parameter

TxtLine::TxtLine(WainDoc* const aMyDoc)
{
   m_text = NULL;
   m_next = m_prev = this;
   m_len = 0;
   m_tabLen = 0;
   m_endLineStatus = NO_ELS;
   m_doc = aMyDoc;
   m_allocLen = 0;
   SetModified(NOT_MODIFIED);
}

TxtLine::TxtLine(void)
{
   m_text = NULL;
   m_next = m_prev = this;
   m_len = 0;
   m_allocLen = 0;
   m_tabLen = 0;
   m_endLineStatus = NO_ELS;
   m_doc = 0;
   SetModified(NOT_MODIFIED);
}

TxtLine::~TxtLine()
{
   if(m_text)
      free(m_text);
}

void TxtLine::operator = (const char *new_text)
{
  ASSERT(m_doc);
  m_len = strlen(new_text);
  MakeSpace(m_len);
  strcpy(m_text, new_text);
  CalcTabLen();
  m_endLineStatus = NO_ELS;
}

void TxtLine::CalcTabLen(void)
{
   m_tabLen = m_len;

   for(int i = 0, n, j = 0; i < m_len; i++, j++)
   {
      if(m_text[i] == '\t')
      {
         n = TAB_WIDTH(m_doc) - 1 - j%TAB_WIDTH(m_doc);
         j += n;
         m_tabLen += n;
      }
   }
}

int TxtLine::SpaceAfter(int pos) const
{
  return TAB_WIDTH(m_doc) - pos%TAB_WIDTH(m_doc);
}

int TxtLine::SpaceBefore(int pos) const
/* Calculate how many space positions there are befors pos if we are within TAB space */
{
  int n, p;

  if(pos > m_tabLen)
    return 0;
  if(pos % TAB_WIDTH(m_doc) == 0)
    return 0;

  n = pos - pos%TAB_WIDTH(m_doc);
  p = pos%TAB_WIDTH(m_doc);

  while(GetTextAt(n++) != '\t')
    p--;
  ASSERT(p >= 0);
  return p;
}

bool TxtLine::Append(char ch)
{
  MakeSpace(m_len + 1);
  m_text[m_len] = ch;
  m_text[++m_len] = 0;
  return true;
}

bool TxtLine::InsertAt(int pos, char ch)
{
  bool ret;
  int org_pos = pos;
  pos = ScreenPosToTextPos(pos);

  if(pos >= m_len)
  { /* Insert after last char */
    MakeSpace(pos + 1);
    m_text[pos] = ch;
    m_text[pos + 1] = '\0';
    char *s = &m_text[m_len];
    while(s < &m_text[pos])
    {
      *s++ = ' ';
      m_len++;
    }
    m_len++;
    ret = true;
  }
  else
  {
    MakeSpace(m_len + 1);
    if(m_text[pos] == '\t')
    { // We might have to replace a tab char
      int i = SpaceBefore(org_pos);
      int a = SpaceAfter(org_pos);
      if(a)
        a--;
      else
        a = 0;
      if(i)
      { // Yes, replace the tab char
        MakeSpace(m_len + TAB_WIDTH(m_doc));
        m_text[pos] = ' ';
        memmove(&m_text[pos + i + 1 + a], &m_text[pos], m_len - pos + 1);
        int j;
        for(j = 0; j < i; j++)
          m_text[pos + j] = ' ';
        m_text[pos + j] = ch;

        for(i = a; i; i--, j++)
           m_text[pos + j + 1] = ' ';

        m_len += j + 1;
        ret = true;
      }
      else
      { // No false alarm
        memmove(&m_text[pos + 1], &m_text[pos], m_len - pos + 1);
        m_text[pos] = ch;
        m_len++;
        ret = true;
      }
    }
    else
    {
      memmove(&m_text[pos + 1], &m_text[pos], m_len - pos + 1);
      m_text[pos] = ch;
      m_len++;
      ret = true;
    }
  }
  CalcTabLen();
  if(ret)
  {
    m_doc->SetModified(TRUE);
    SetModified(IS_MODIFIED);
  }
  return ret;
}

bool TxtLine::InsertAt(int pos, const char *str, int length)
{
  if(length == -1) // length == -1 by default, so str must be '\0' terminated, so calculate length
    length = strlen(str);
  bool ret = false;
  int org_pos = pos;
  pos = ScreenPosToTextPos(pos);
  if(pos >= m_len)
  { /* Insert after last char */
    MakeSpace(pos + length);
    char *s = &m_text[m_len];
    while(s < &m_text[pos])
    {
      *s++ = ' ';
      m_len++;
    }
    m_len += length;
    strcpy(&m_text[pos], str);
    ret = true;
  }
  else
  {
    if(m_tabLen != m_len)
    { // If the line contains tabs, insert the chars one by one
      int i;
      for(i = 0; i < length; i++)
      {
        InsertAt(org_pos, str[i]);
        if(str[i] == '\t')
          org_pos += TAB_WIDTH(m_doc) - (org_pos%TAB_WIDTH(m_doc));
        else
          org_pos++;
      }
    }
    else
    {
      MakeSpace(m_len + length);
      memmove(&m_text[pos + length], &m_text[pos], m_len - pos + 1);
      memmove(&m_text[pos], str, length);
      m_len += length;
      ret = true;
    }
  }
  if(ret)
  {
    CalcTabLen();
    m_doc->SetModified(TRUE);
    SetModified(IS_MODIFIED);
  }
  return ret;
}

bool TxtLine::SplitLineAt(int pos)
{
  pos = ScreenPosToTextPos(pos);

  if(pos >= m_len)
  {
    TxtLine *new_line = new TxtLine(m_doc);
    *new_line = "";
    new_line->m_next = this->m_next;
    new_line->m_prev = this;
    this->m_next->m_prev = new_line;
    this->m_next = new_line;
    new_line->SetModified(IS_MODIFIED);
  }
  else
  {
    TxtLine *new_line = new TxtLine(m_doc);
    *new_line = &m_text[pos];
    m_text[pos] = '\0';
    m_len = pos;
    new_line->m_next = this->m_next;
    new_line->m_prev = this;
    this->m_next->m_prev = new_line;
    this->m_next = new_line;
    SetModified(IS_MODIFIED);
    new_line->SetModified(IS_MODIFIED);
  }
  m_doc->m_lineCount++;
  CalcTabLen();
  m_doc->SetModified(TRUE);
  return TRUE;
}

DeleteTypeType TxtLine::DeleteAt(int pos)
{
  DeleteTypeType ret = normal_delete;
  int org_pos = pos;
  pos = ScreenPosToTextPos(pos);
  if(pos < m_len)
  {
    if(m_text[pos] == '\t')
    {
      MakeSpace(m_len + TAB_WIDTH(m_doc));

      int b = SpaceBefore(org_pos);
      if(b == TAB_WIDTH(m_doc))
        b = 0;
      int a = SpaceAfter(org_pos);

      if(a + b)
      {
        int i;
        memmove(&m_text[pos + a + b - 1], &m_text[pos + 1], m_len - pos);
        for(i = 0; i < a + b - 1; i++)
          m_text[pos + i] = ' ';
      }
      m_len += a + b - 2;
      ret = tab_delete;
    }
    else
    {
      memmove(&m_text[pos], &m_text[pos + 1], m_len - pos);
      m_len--;
      ret = normal_delete;
    }
  }
  m_doc->SetModified(TRUE);
  SetModified(IS_MODIFIED);
  CalcTabLen();
  return ret;
}

DeleteTypeType TxtLine::DeleteAt(int start, int end)
{
  DeleteTypeType ret = normal_delete;
  int org_start = start;
  int org_end = end;
  start = ScreenPosToTextPos(start);
  if(start < m_len)
  {
    int i, a, b;
    for(i = start; m_text[i] && i <= end; i++)
      if(m_text[i] == '\t')
        ret = tab_delete;

    /* If the character at start is a TAB, break it into the corresponding number of spaces */
    if(m_text[start] == '\t')
    {
      MakeSpace(m_len + TAB_WIDTH(m_doc));
      a = SpaceBefore(org_start);
      a += SpaceAfter(org_start);
      memmove(&m_text[start + a], &m_text[start + 1], m_len - start);
      m_len += a - 1;
      for(b = 0; a != 0; b++, a--)
        m_text[start + b] = ' ';
      start = ScreenPosToTextPos(org_start);
    }

    /* If the character at end is a TAB, break it into the corresponding number of spaces */
    end = ScreenPosToTextPos(end);
    if(end < m_len && m_text[end] == '\t')
    {
      MakeSpace(m_len + TAB_WIDTH(m_doc));
      a = SpaceBefore(org_end);
      a += SpaceAfter(org_end);
      memmove(&m_text[end + a], &m_text[end + 1], m_len - end);
      m_len += a - 1;
      for(b = 0; a != 0; b++, a--)
        m_text[end + b] = ' ';
      end = ScreenPosToTextPos(org_end);
    }
    if(m_len > end)
      memmove(&m_text[start], &m_text[end], m_len - end + 1);
    m_len -= end < m_len ? end - start : m_len - start;
    m_text[m_len] = 0; /* To handle the case where len < end */
  }
  m_doc->SetModified(TRUE);
  SetModified(IS_MODIFIED);
  CalcTabLen();
  return ret;
}

void TxtLine::RemoveFrom(int column, char *rest)
{
  int org_pos = column;
  column = ScreenPosToTextPos(column);
  int i;
  if(column < m_len)
  {
    if(m_tabLen != m_len && m_text[column] == '\t' && SpaceBefore(org_pos) != 0)
    { // We have to insert within tab space, replace the tab with spaces
      DeleteAt(org_pos);
      InsertAt(org_pos, ' ');
      column = ScreenPosToTextPos(org_pos);
    }
    i = column;
    if(rest)
    {
      while(i < m_len)
        *rest++ = m_text[i++];
      *rest = '\0';
    }
    m_text[column] = '\0';
  }
  else
    *rest = 0;

  m_len = strlen(m_text);
  CalcTabLen();
  SetModified(IS_MODIFIED);
}

char TxtLine::ReplaceAt(int pos, char ch)
{
  char p;
  int org_pos = pos;
  pos = ScreenPosToTextPos(pos);
  MakeSpace(pos + 1);
  if(pos >= m_len)
  {
    char *s = &m_text[m_len];
    while(m_len < pos)
    {
      *s++ = ' ';
      m_len++;
    }
    m_len++;
    *s++ = ch;
    *s = '\0';
    p = '\0';
    CalcTabLen();
  }
  else
  {
    p = m_text[pos];
    if(p == '\t')
    {
      DeleteAt(org_pos);
      InsertAt(org_pos, ch);
      p = ' ';
    }
    else
    {
      m_text[pos] = ch;
      CalcTabLen();
    }
  }
  m_doc->SetModified(TRUE);
  SetModified(IS_MODIFIED);
  return p;
}

int TxtLine::GetWordLenRight(int pos) const
{
  pos = ScreenPosToTextPos(pos);
  if(pos < m_len)
  {
    int i;
    for(i = 0; i + pos < m_len && !strchr(SEPS(m_doc), m_text[pos + i]); i++);
    return i;
  }
  return 0;
}

int TxtLine::GetSpaceLenRight(int pos) const
//  Description:
//    Counts the number of space to the right of screen pos
//  Parameters:
//    pos: The screen position
//    Return the number of spaces, tabs are converted to space positions
{
  int org_pos = pos;
  pos = ScreenPosToTextPos(pos);
  int i = 0;

  if(pos < m_len)
  {
    for(;m_text[pos] == ' ' || m_text[pos] == '\t'; pos++)
    {
      if(m_text[pos] == ' ')
        i++;
      else
        i += TAB_WIDTH(m_doc) - (org_pos + i)%TAB_WIDTH(m_doc);
    }
  }
  return i;
}

int TxtLine::GetSpaceLenLeft(int pos) const
{
  if(!pos)
    return 0;

  int org_pos = pos - 1;
  pos = ScreenPosToTextPos(pos - 1);

  if(m_text[pos] != ' ' && m_text[pos] != '\t')
    return 0;

  int r = 0, p;
  do
  {
    p = ScreenPosToTextPos(org_pos--);
    if(m_text[p] == ' ' || m_text[p] == '\t')
      r++;
  }
  while((m_text[p] == ' ' || m_text[p] == '\t') && org_pos >= 0);

  return -r;
}

int TxtLine::GetSepLenRight(int pos) const
//  Description:
//    Counts the number of seperators to the right of screen pos
//  Parameters:
//    pos: The screen position
//    Return the number of seperators.
{
  pos = ScreenPosToTextPos(pos);
  int i = 0;

  while(pos < m_len && strchr(SEPS(m_doc), m_text[pos]) && m_text[pos] != ' ' && m_text[pos] != '\t')
  {
    pos++;
    i++;
  }

  return i;
}

int TxtLine::GetSepLenLeft(int pos) const
{
  pos = ScreenPosToTextPos(pos);

  if(pos && pos <= m_len)
  {
    int i;
    pos--;

    for(i = 0; pos >= 0 && strchr(SEPS(m_doc), m_text[pos]) && m_text[pos] != '\t'; i++, pos--)
    { /* Nothing */ }
    return -i;
  }
  return 0;
}

int TxtLine::GoWordRight(int pos) const
{
  int r, s;
  int org_pos = pos;
  pos = ScreenPosToTextPos(pos);
  if(pos >= m_len)
    return 0;


  if((r = GetWordLenRight(org_pos)) == 0)
    r = GetSepLenRight(org_pos);

  s = GetSpaceLenRight(org_pos + r);

  return s + r;
}

int TxtLine::GoWordLeft(int pos) const
{
  int r, s;
  int org_pos = pos;
  pos = ScreenPosToTextPos(pos);
  if(!pos)
  { /* At the beginning of the line or within the first tab stop */
    return org_pos;
  }
  if(pos > m_len)
    return pos - m_len;

  if((r = -GetWordLenLeft(org_pos)) == 0)
    r = -GetSepLenLeft(org_pos);

  s = -GetSpaceLenLeft(org_pos - r) + r;

  return s;
}

int TxtLine::GetWordLenLeft(int pos) const
{
  pos = ScreenPosToTextPos(pos);

  if(pos && pos <= m_len)
  {
    int i;
    pos--;

    for(i = 0; pos >= 0 && !strchr(SEPS(m_doc), m_text[pos]) && m_text[pos] != '\t'; i++, pos--)
    { /* Nothing */ }
    return -i;
  }
  return 0;
}

EndLineStatusType TxtLine::GetTextColor(TxtLineColor &aLineColor, EndLineStatusType aPrevEls, bool aJustStatus) const
{
   const DocPropClass* PROP = m_doc->m_prop;
   int i, n, k;
   char *temp = (char *)malloc(m_allocLen);

   const char *s = m_text;
   while(*s == ' ' || *s == '\t')
     s++;

   const char *comment_s = s;
   if(!PROP->m_commentFirstWord/*Hello world */)
     comment_s = NULL;

   bool inComment = (aPrevEls == COMMENT_ELS) || (aPrevEls == LINE_COMMENT_ELS) || (aPrevEls == COMMENT_IN_PREPROC_ELS);
   bool inPreproc = aPrevEls == PREPROC_ELS;
   bool inLineComment = false;
   bool comInPreproc = false;
   aLineColor.Reset();

   if(inComment || inPreproc)
   {
   }
   else if(IsSeqIc(s, PROP->m_preProcessor.c_str(), PROP->m_ignoreCase) ||
           (PROP->m_preProcessor.empty() && !PROP->m_preProcWord.empty()))
   {
     if(PROP->m_preProcWord.size() && PROP->m_preProcWord[0].size())
     {
       s += PROP->m_preProcessor.size();
       while(*s == ' ' || *s == '\t')
         s++;
       if(*s)
       {
         BOOL got_it;
         size_t idx;
         for(idx = 0, got_it = FALSE; idx < PROP->m_preProcWord.size() && !got_it; idx++)
           if(IsSeqIc(s, PROP->m_preProcWord[idx].c_str(), PROP->m_ignoreCase))
             got_it = TRUE;
         if(got_it)
         {
           inPreproc = TRUE;
         }
       }
     }
     else
     {
       inPreproc = TRUE;
     }
   }
   const char *numPtr = NULL;
   size_t numLen = 0;

   auto wordsBegin = std::cregex_iterator(m_text, m_text + strlen(m_text), PROP->m_numberRegEx);
   auto wordsEnd = std::cregex_iterator();
   std::cregex_iterator nrIterator = wordsBegin;

   if(!aJustStatus && nrIterator != wordsEnd)
   {
      numPtr = m_text + (*nrIterator).position();
      numLen = (*nrIterator).length();
      nrIterator++;
   }
   size_t Len;
   for(i = 0; m_text[i] != '\0'; i++)
   {
      if(m_text[i] == '\t')
      {
         aLineColor.AddEntry(TAB_WIDTH(m_doc) - (aLineColor.GetSumLen()%TAB_WIDTH(m_doc)), TAB_IDX);
      }
      else if(inLineComment)
      {
         int lc = 0;
         k = 0;
         do
         {
            if(m_text[i + k] == '\t')
            {
               aLineColor.AddEntry(lc, COM_IDX);
               lc = TAB_WIDTH(m_doc) - (aLineColor.GetSumLen()%TAB_WIDTH(m_doc));
               aLineColor.AddEntry(lc, TAB_IDX);
               lc = -1;
            }
            k++;
            lc++;
         }
         while(m_text[i + k]);
         aLineColor.AddEntry(lc, COM_IDX);
         i += k - 1;
      }
      else if(inComment)
      {
         int cc = 0;
         k = 0;
         while(m_text[i + k] != 0 && !IsSeqEnd(&m_text[i + k], PROP->m_commentEnd.c_str(), PROP->m_ignoreCase, PROP->m_commentWholeWord, SEPS(m_doc)))
         {
            if(m_text[i + k] == '\t')
            {
               aLineColor.AddEntry(cc, COM_IDX);
               cc = TAB_WIDTH(m_doc) - ((aLineColor.GetSumLen())%TAB_WIDTH(m_doc));
               aLineColor.AddEntry(cc, TAB_IDX);
               cc = -1;
            }
            k++;
            cc++;
         }
         if(IsSeqEnd(&m_text[i + k], PROP->m_commentEnd.c_str(), PROP->m_ignoreCase, PROP->m_commentWholeWord, SEPS(m_doc)))
         {
            inComment = false;
            k += PROP->m_commentEnd.size();
            cc += PROP->m_commentEnd.size();
            if(aPrevEls == COMMENT_IN_PREPROC_ELS || comInPreproc)
            {
               inPreproc = true;
            }
            comInPreproc = false;
         }
         int sizeCom = PROP->m_commentEnd.size();
         if (m_doc->m_view->IsMatchPos(this, i + k - 1))
         {
            cc -= sizeCom;
         }
         aLineColor.AddEntry(cc, COM_IDX);
         if (m_doc->m_view->IsMatchPos(this, i + k - 1))
         {
            inComment = false;
            aLineColor.AddEntry(sizeCom, MATCH_IDX);
         }
         i += k - 1;
      }
      else if(IsSeqEnd(&m_text[i], PROP->m_lineComment.c_str(), PROP->m_ignoreCase, PROP->m_commentWholeWord, SEPS(m_doc)) &&
              (!comment_s || comment_s == &m_text[i]))
      {
         aLineColor.AddEntry(PROP->m_lineComment.size(), COM_IDX);
         i += PROP->m_lineComment.size() - 1;
         inLineComment = TRUE;
      }
      else if(IsSeqEnd(&m_text[i], PROP->m_commentBegin.c_str(), PROP->m_ignoreCase, PROP->m_commentWholeWord, SEPS(m_doc)))
      {
         if (m_doc->m_view->IsMatchPos(this, i))
         {
            aLineColor.AddEntry(PROP->m_commentBegin.size(), MATCH_IDX);
         }
         else
         {
            aLineColor.AddEntry(PROP->m_commentBegin.size(), COM_IDX);
         }
         i += PROP->m_commentBegin.size() - 1;
         inComment = TRUE;
      }
      else if(inPreproc)
      {
         int pc = 0;
         k = 0;
         do
         {
            if(m_text[i + k] == '\t')
            {
               aLineColor.AddEntry(pc, PRE_IDX);
               pc = TAB_WIDTH(m_doc) - ((aLineColor.GetSumLen())%TAB_WIDTH(m_doc));
               aLineColor.AddEntry(pc, TAB_IDX);
               pc = -1;
            }
            pc++;
            k++;
         }
         while(m_text[i + k] &&
               !IsSeqEnd(&m_text[i + k], PROP->m_commentBegin.c_str(), PROP->m_ignoreCase, PROP->m_commentWholeWord, SEPS(m_doc)) &&
               !IsSeqEnd(&m_text[i + k], PROP->m_lineComment.c_str(), PROP->m_ignoreCase, PROP->m_commentWholeWord, SEPS(m_doc)));

         if(IsSeqEnd(&m_text[i + k], PROP->m_commentBegin.c_str(), PROP->m_ignoreCase, PROP->m_commentWholeWord, SEPS(m_doc)))
            comInPreproc = TRUE;

         aLineColor.AddEntry(pc, PRE_IDX);
         i += k - 1;
      }
      else if(CheckStr(&m_text[i], PROP->m_stringDelim, STR_IDX, Len, aLineColor))
      {
         i += Len - 1;
      }
      else if(CheckStr(&m_text[i], PROP->m_chDelim, STR_IDX, Len, aLineColor))
      {
         i += Len - 1;
      }
      else if(numPtr == &m_text[i])
      { /* It's a number */
         n = numLen;
         if((unsigned int)n > strlen(&m_text[i])) /* Just to be sure */
           n = strlen(&m_text[i]);
         aLineColor.AddEntry(n, NUM_IDX);
         i += n - 1;
         if (nrIterator != wordsEnd)
         {
            numPtr = m_text + (*nrIterator).position();
            numLen = (*nrIterator).length();
            nrIterator++;
         }
         else
         {
            numPtr = 0;
            numLen = 0;
         }
      }
      else if(m_doc->m_view->IsMatchPos(this, i))
      {
         aLineColor.AddEntry(1, MATCH_IDX);
      }
      else if(strchr(SEPS(m_doc), m_text[i]) != NULL)
      {
         aLineColor.AddEntry(1, SEP_IDX);
      }
      else if(!aJustStatus)
      {
         for(n = 0; !strchr(SEPS(m_doc), m_text[i + n]); n++)
         {
            temp[n] = m_text[i + n];
         }
         temp[n] = '\0';
         char *ss = temp;
         i += n - 1;

         if (m_doc->m_markWord == temp)
         {
            aLineColor.AddEntry(n, WORD_IDX);
         }
         else
         {
            BOOL keyword = keyword = FALSE;
            for(k = 0; k < 5 && !keyword; k += keyword ? 0 : 1)
            {
               if(m_doc->m_prop->m_keyWordList[k].size())
               {
                  if(m_doc->m_prop->m_ignoreCase)
                  {
                     StringIgnoreCaseCompareClass StringIgnoreCaseCompare;
                     keyword = std::binary_search(m_doc->m_prop->m_keyWordList[k].begin(),
                                                  m_doc->m_prop->m_keyWordList[k].end(),
                                                  ss,
                                                  StringIgnoreCaseCompare);
                  }
                  else
                  {
                     keyword = std::binary_search(m_doc->m_prop->m_keyWordList[k].begin(),
                                                  m_doc->m_prop->m_keyWordList[k].end(),
                                                  ss);
                  }
               }
            }
            if(keyword)
            {
               aLineColor.AddEntry(n, (ColorIndexType )(KEY_0_IDX + k));
            }
            else if(m_doc->m_prop->m_useTagColor && GetMf()->m_navigatorDialog.m_globalTags.CheckTag(temp))
            {
               aLineColor.AddEntry(n, TAG_IDX);
            }
            else
            {
               aLineColor.AddEntry(n, TEXT_IDX);
            }
         }
      }
      else /* Just get the status */
      {
         for(n = 0; !strchr(SEPS(m_doc), m_text[i + n]); n++)
         {
            temp[n] = m_text[i + n];
         }
         temp[n] = '\0';
         i += n - 1;
         aLineColor.AddEntry(n, TEXT_IDX);
      }
      /* If we have found a number in the middle of a word, search for the next */
      if(numPtr && numPtr < &m_text[i + 1])
      {
         if (nrIterator != wordsEnd)
         {
            numPtr = m_text + (*nrIterator).position();
            numLen = (*nrIterator).length();
            nrIterator++;
         }
         else
         {
            numPtr = 0;
            numLen = 0;
         }
      }
   }
   BOOL isLastCont = FALSE;
   EndLineStatusType retVal = NO_ELS;
   if(strlen(m_text))
   {
     s = &m_text[strlen(m_text) - 1];
     while((*s == ' ' || *s == '\t') && s != m_text)
       s--;
     if(IsSeq(s, PROP->m_lineCon.c_str()))
       isLastCont = TRUE;
   }
   if((aPrevEls == PREPROC_ELS && inComment == TRUE) || comInPreproc)
     retVal = COMMENT_IN_PREPROC_ELS;
   else if(inPreproc && isLastCont)
     retVal = PREPROC_ELS;
   else if((inLineComment || aPrevEls == LINE_COMMENT_ELS) && isLastCont)
     retVal = LINE_COMMENT_ELS;
   else if(inComment && aPrevEls != LINE_COMMENT_ELS && !inLineComment)
     retVal = COMMENT_ELS;
   free(temp);

   return retVal;
}

bool TxtLine::CheckStr(const char *aStr, const std::string &aMatch, ColorIndexType aIndexType, size_t &aSize, TxtLineColor &aLineColor) const
{
   const DocPropClass* PROP = m_doc->m_prop;
   aLineColor.StoreBookMark();
   if(IsSeq(aStr, aMatch.c_str()))
   {
      size_t k = aMatch.size();
      size_t sc = aMatch.size();
      do
      {
         if(IsSeq(&aStr[k], PROP->m_literal.c_str()) &&
            aStr[k + PROP->m_literal.size()] != '\0' &&
            aStr[k + PROP->m_literal.size()] != '\t')
         {
            k += PROP->m_literal.size() + 1;
            sc += PROP->m_literal.size() + 1;
         }
         else if(aStr[k] == '\t')
         { /* A tab in a string, create a entry, and a tab entry */
            aLineColor.AddEntry(sc, aIndexType);
            sc = TAB_WIDTH(m_doc) - (aLineColor.GetSumLen() % TAB_WIDTH(m_doc));
            aLineColor.AddEntry(sc, TAB_IDX);
            sc = 0;
            k++;
         }
         else if(aStr[k] != 0 && !IsSeq(&aStr[k], aMatch.c_str()))
         {
            k++;
            sc++;
         }
      }
      while(aStr[k] != '\0' && !IsSeq(&aStr[k], aMatch.c_str()));

      if(aStr[k])
      {
         k += aMatch.size();
         sc += aMatch.size();
         aLineColor.AddEntry(sc, aIndexType);
         aSize = k;
         return true;
      }
      aLineColor.ResetToBookMark();
      return false;
   }
   return false;
}

char TxtLine::GetTextAt(int pos) const
{
  pos = ScreenPosToTextPos(pos);
  if(pos >= m_len)
    return (char )-1;
  return m_text[pos];
}

int TxtLine::ScreenPosToTextPos(int pos) const
{
  if(strchr(m_text, '\t'))
  {
    int i, j, org_pos, n;
    for(i = 0, j = 0, org_pos = pos, n = 0; j < org_pos && i < m_len; j++)
    {
      if(!n && m_text[i] == '\t')
      {
        n = TAB_WIDTH(m_doc) - j%TAB_WIDTH(m_doc) - 1;
        if(n == 0)
         i++;
      }
      else if(n)
      {
        n--;
        if(!n)
          i++;
      }
      else
      {
        i++;
      }
    }
    n = j - i;
    ASSERT(pos >= n);
    pos -= n;
  }
  return pos;
}

int TxtLine::TextPosToScreenPos(int pos) const
{
  int i, n;
  for(i = 0, n = 0; i < pos; i++)
  {
    if(m_text[i] == '\t')
      n += TAB_WIDTH(m_doc) - n%TAB_WIDTH(m_doc);
    else
      n++;
  }
  return n;
}

bool TxtLine::CopyTextAt(char *buf, int pos, int nof) const
{
  ASSERT(buf);
  int sp = pos;
  pos = ScreenPosToTextPos(pos);
  int i, j = 0;

  while(j < nof)
  {
    if(m_text[pos] == '\t')
    {
      int n = TAB_WIDTH(m_doc) - sp%TAB_WIDTH(m_doc);
      for(i = 0; i < n && j < nof; i++, j++)
      {
        *buf++ = ' ';
        sp++;
      }
      pos++;
    }
    else
    {
      *buf++ = m_text[pos];
      j++;
      pos++;
      sp++;
    }
  }
  return TRUE;
}

void TxtLine::StripWhitespace(void)
{
/* Called when the doc is saved, so set modify_status to MODIFIED_SAVED */
  if(m_modifyStatus == IS_MODIFIED)
    SetModified(MODIFIED_SAVED);

  if(!m_len)
    return;
  char *s = &m_text[m_len - 1];
  while(*s == ' ' || *s == '\t')
  {
    *s = '\0';
    m_len--;
    if(!m_len)
    {
      m_tabLen = 0;
      return;
    }
    s--;
  }
  CalcTabLen();
}

int TxtLine::GetTabLen(void) const
{
  return m_tabLen;
}

int TxtLine::GetTextLen(void) const
{
  return m_len;
}

int TxtLine::GetEmptyCharBefore(int pos) const
{
  if(pos <= m_tabLen)
    return 0;
  return pos - m_tabLen;
}

void TxtLine::ToUpper(int from, int to)
{
  if(to == -1)
    to = GetTabLen();
  to = ScreenPosToTextPos(to);
  from = ScreenPosToTextPos(from);
  int i;
  for(i = from; i < to; i++)
    m_text[i] = (char )::toupper(unsigned(m_text[i]));
  SetModified(IS_MODIFIED);
  m_doc->SetModified(TRUE);
}

void TxtLine::ToLower(int from, int to)
{
  if(to == -1)
    to = GetTabLen();
  to = ScreenPosToTextPos(to);
  from = ScreenPosToTextPos(from);
  int i;
  for(i = from; i < to; i++)
    m_text[i] = (char )::tolower(*(unsigned char *)&(m_text[i]));

  SetModified(IS_MODIFIED);
  m_doc->SetModified(TRUE);
}

bool TxtLine::CheckSpace(int aFrom, int aTo) const
{
   int nof = aTo - aFrom;
   ASSERT(nof >= 0);
   int org_pos = aFrom;
   aFrom = ScreenPosToTextPos(aFrom);
   if(aFrom >= m_len)
     return true;
   int i, n = org_pos;
   for(i = aFrom; nof && m_text[i] != 0; i++)
   {
      if(m_text[i] == ' ')
      {
         nof--;
         n++;
      }
      else if(m_text[i] == '\t')
      {
         nof -= (TAB_WIDTH(m_doc) - n%TAB_WIDTH(m_doc));
         n += (TAB_WIDTH(m_doc) - n%TAB_WIDTH(m_doc));
      }
      else
      {
         return false;
      }
   }
   return true;
}

void TxtLine::SetModified(ModifyStatusType new_status)
{
  if(new_status != m_modifyStatus)
  {
    m_modifyStatus = new_status;
    if(m_doc && m_doc->m_view && m_doc->m_view->m_tabView)
       m_doc->m_view->m_tabView->InvalidateRect(NULL, TRUE);
  }
}

void TxtLine::MakeSpace(int new_len)
{
#define GET_LEN(l) (((((l) + 1) & ~0x0F) + 0x10))

  if(new_len >= m_allocLen)
  {
    m_allocLen = GET_LEN(new_len);
    m_text = (char *)realloc(m_text, m_allocLen);
    m_doc->SetMaxLength(m_allocLen);
  }
#undef GET_LEN
}

int TxtLine::GetRawLen(int from, int to, BOOL fixed) const
/* calculate how many bytes is requred to hold the chars between from and to            */
/* Special care must be taken if from starts within tab space or to is within tab space */
/* The space is in both cases converted to spaces                                       */
/* If fixed, spaces are appended as requred, the to is truncated to len otherwice       */
{
  if(from >= m_tabLen) /* All spaces */
  {
    if(fixed)
      return to - from;
    else
      return 0;
  }
  if(m_len == m_tabLen) /* We does not have any tabs, or each tab fills one position */
  {
    if(fixed)
      return to - from;
    if(to <= m_len)
      return to - from;
    else
      return m_len - from;
  }
  int last = MIN(m_tabLen, to);
  int nof = ScreenPosToTextPos(last) - ScreenPosToTextPos(from);

  int start = ScreenPosToTextPos(from);
  if(m_text[start] == '\t' && from%TAB_WIDTH(m_doc) != 0)
    nof += SpaceAfter(from) - 1;

  int end = ScreenPosToTextPos(last);
  if(m_text[end] == '\t' && last%TAB_WIDTH(m_doc) != 0)
    nof += SpaceBefore(last);
  if(fixed && to > last)
    nof += to - last;
  return nof;
}

int TxtLine::CopyRaw(char *dest, int from, int to, BOOL fixed) const
{
  int a, b, i;
  if(m_tabLen == m_len)
  {
    if(from < m_len)
      memcpy(dest, &m_text[from], MIN(m_len, to) - from);

    if(fixed)
    { /* We might have to append spaces */
      a = to - MAX(m_len, from);
      if(a > 0)
      {
        b = m_len - from;
        if(b < 0)
          b = 0;
        for(i = 0; i < a; i++)
          dest[b + i] = ' ';
      }
    }
    i = fixed ? to - from : MIN(m_len, to) - from;
    if(i < 0)
      i = 0;
    return i;
  }

  int start = ScreenPosToTextPos(from);
  int last = MIN(m_tabLen, to);
  int end = ScreenPosToTextPos(last);

  i = 0;
  if(m_text[start] == '\t' && from%TAB_WIDTH(m_doc) != 0)
  {
    a = SpaceAfter(from);
    for(b = 0; b < a; b++)
      dest[i++] = ' ';
    start++;
  }
  if(m_text[end] == '\t' && last%TAB_WIDTH(m_doc) != 0)
  {
    for(b = start; b < end; b++)
      dest[i++] = m_text[b];
    a = SpaceBefore(last);
    for(b = 0; b < a; b++)
      dest[i++] = ' ';
  }
  else
  {
    for(b = start; b < end; b++)
      dest[i++] = m_text[b];
  }
  if(fixed && to > m_tabLen)
  {
    a = to - MAX(m_tabLen, from);
    for(b = 0; b < a; b++)
      dest[i++] = ' ';
  }
  return i;
}

void TxtLine::RemoveTabs(void)
{
  BOOL changed = FALSE;
  int i, n;
  for(i = 0, n = 0; i < m_len; i++, n++)
  {
    if(m_text[i] == '\t')
    {
      int j = SpaceBefore(n);
      j += SpaceAfter(n);
      MakeSpace(m_len + j);
      if(j > 1)
        memmove(&m_text[i + j], &m_text[i + 1], m_len - i);
      for(int k = 0; k < j; k++)
        m_text[i + k] = ' ';
      m_len += j - 1;
      changed = TRUE;
    }
  }
  if(changed)
  {
    CalcTabLen();
    SetModified(IS_MODIFIED);
    m_doc->SetModified(TRUE);
  }
}

int TxtLine::GetSpaceInFront(void) const
{
  /* Count the number of "spaces" at the beginning of the line */
  const char *t = m_text;
  int xx = 0;

  while(*t == ' ' || *t == '\t')
  {
    if(*t == ' ')
      xx++;
    else
      xx += TAB_WIDTH(m_doc) - xx%TAB_WIDTH(m_doc);
    t++;
  }
  return xx;
}

const char *TxtLine::FindFirstNotOf(int aFirstPos, const char *aChars) const
{
   aFirstPos = ScreenPosToTextPos(aFirstPos);
   if(aFirstPos >= m_tabLen)
      return &m_text[m_tabLen];
   const char *p = &m_text[aFirstPos];
   while(aFirstPos < m_tabLen && strchr(aChars, *p))
   {
      aFirstPos++;
      p++;
   }
   return p;
}

int TxtLine::FindChar(int aFirstPos, char aChar, int aDir)
{
   aFirstPos = ScreenPosToTextPos(aFirstPos);
   if(aDir > 0)
   {  // Search Forward
      int i;
      for(i = aFirstPos + 1; i < m_len; i++)
         if(m_text[i] == aChar)
            return TextPosToScreenPos(i);
      return -1;
   }
   else
   {  // Search backward
      int i;
      for(i = aFirstPos - 1; i >= 0; i--)
         if(m_text[i] == aChar)
            return TextPosToScreenPos(i);
      return -1;
   }
}
