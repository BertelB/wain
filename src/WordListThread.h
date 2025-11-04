#ifndef _WORD_LIST_THREAD_H_INC
#define _WORD_LIST_THREAD_H_INC

#include <string>
#include <vector>
#include "../src/Project.h"
#include <unordered_map>

class WordMatchInfo
{
public:
   WordMatchInfo(uint32_t _fnIdx, uint32_t _lineNo, const std::string& _line) :
      m_fnIdx(_fnIdx), m_lineNo(_lineNo), m_line(_line)
   {
   }
   uint32_t m_fnIdx;
   uint32_t m_lineNo;
   std::string m_line;
};

class WordThreadParam
{
public:
   std::vector<std::string> m_fileName; // The names of the files to scan
   std::vector<std::string> m_keyWords;  // The keywords for this extension type, do not include these
   std::unordered_map<std::string, std::list<std::shared_ptr<WordMatchInfo>>> m_wordMap; // Map of words and a list of info
   std::string m_seperators;
   bool m_ignoreCase = false;
   int m_propIndex = 0;
   bool m_replace;
};

void StartWordThread(std::vector<std::string>& _fileList, std::vector<std::string>* _keyWordList, std::string& _seperators, bool _ignoreCase, int _propIndex, bool _replace);

#endif
