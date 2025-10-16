#ifndef _WAIN_DOC_UTIL_H_INC
#define _WAIN_DOC_UTIL_H_INC

inline bool IsSeq(const char* _str, const char* _seq)
//  Description:
//    Check if the first strlen(seq) chars of str is equal to to seq
//    An empty seq is newer equal
{
  if(*_seq == 0)
    return false;

  while(*_str && *_seq && *_seq == *_str)
  {
    _seq++;
    _str++;
  }
  return *_seq ? false : true;
}

inline bool IsSeqIc(const char *_str, const char *_seq, bool _ignoreCase)
//  Description:
//    Check if the first strlen(seq) chars of str is equal to to seq
//    An empty seq is newer equal
//    if ic ignore case
{
  if(!_ignoreCase)
    return IsSeq(_str, _seq);
  if(*_seq == 0)
    return false;
  while(*_str && *_seq && tolower(*(unsigned char *)_seq) == tolower(*(unsigned char *)_str))
  {
    _seq++;
    _str++;
  }
  return *_seq ? false : true;
}

inline bool IsSeqEnd(const char* _str, const char* _seq, bool _ignoreCase, bool _wholeWords, const char* _seps)
{
  if(*_seq == 0)
    return false;
  if(!_wholeWords)
    return IsSeq(_str, _seq);
  if(!IsSeqIc(_str, _seq, _ignoreCase))
    return false;
  if(strchr(_seps, _str[strlen(_seq)]))
    return true;
  return false;
}
#endif
