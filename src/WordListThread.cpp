#include ".\..\src\stdafx.h"
#include "../src/WordListThread.h"
#include "../src/Wain.h"
#include <map>
#include <list>
#include <stdint.h>
#include <string.h>
#include <fstream>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#pragma warning (disable : 4100) // unreferenced formal parameter

UINT ThreadGetWords(LPVOID _parm);

void StartWordThread(std::vector<std::string>& _fileList, std::vector<std::string>* _keyWordList, std::string& _seperators, bool _ignoreCase, int _propIndex, bool _replace)
{
   auto parm = new WordThreadParam;
   for (uint32_t i = 0; i < _fileList.size(); i++)
   {
      parm->m_fileName.push_back(_fileList[i]);
   }
   for (uint32_t i = 0; i < /* _keyWordList->size() */ 5; i++)
   {
      for (uint32_t j = 0; j < _keyWordList[i].size(); j++)
      {
         parm->m_keyWords.push_back(_keyWordList[i][j]);
      }
   }
   parm->m_seperators = _seperators;
   parm->m_ignoreCase = _ignoreCase;
   parm->m_propIndex = _propIndex;
   parm->m_replace = _replace;
   AfxBeginThread(ThreadGetWords, (LPVOID)parm, THREAD_PRIORITY_LOWEST);
}

bool IsKeyword(const std::string& _word, std::vector<std::string>& _keywords)
{
   for (uint32_t i = 0; i < _keywords.size(); i++)
   {
      if (_word == _keywords[i])
      {
         return true;
      }
   }
   return false;
}

bool AddWord(const std::string& _word, std::vector<std::string>& _keywords)
{
   if (!_word.size())
      return false;
   if (strchr("0123456789", _word[0])) // Numbers are not words
      return false;
   return !IsKeyword(_word, _keywords);
}

uint32_t FindWords(std::list<std::string>& _list,
                   std::string& _text,
                   std::vector<std::string>& _keywords,
                   std::string& _seperators,
                   bool _ignoreCase)
{
   std::string::size_type start = 0;
   _list.clear();
   uint32_t nWords = 0;
   const std::string::size_type textLength = _text.size();
   while(start < textLength)
   {
      bool cont = true;
      while (cont)
      {
         if (start >= textLength) return nWords;
         if (_text[start] == '\"') // Skip strings
         {
            start++;
            if (start >= textLength) return nWords;
            while (start < textLength && _text[start] != '\"')
            {
               if (_text[start] == '\\')
               {
                  if (start + 1 >= textLength) return nWords;
                  if (_text[start + 1] == '\"')
                     start += 2;
                  else
                     start++;
               }
               else
                  start++;
            }
            if (start >= textLength) return nWords;
         }
         else if (_text[start] == '/') // Skip comments
         {
            if (start + 1 >= textLength) return nWords;
            if (_text[start + 1] == '/') return nWords;
            start++;
         }
         if (strchr(_seperators.c_str(), _text[start]))
            start++;
         else
           cont = false;
      }
      std::string::size_type y = textLength;
      if (start != y)
         y = _text.find_first_of(_seperators.c_str(), start);
      if (y == std::string::npos)
      {
         y = textLength;
      }
      if (start >= textLength || y > textLength) return nWords;

      std::string word = _text.substr(start, y - start);
      if (AddWord(word, _keywords))
      {
         if (std::find(_list.begin(), _list.end(), word) == _list.end())
         {
            _list.push_back(word);
         }
      }
      nWords++;
      start = y;
   }
   return nWords;
}

void AddToWordList(std::unordered_map<std::string, std::list<std::shared_ptr<WordMatchInfo>>>& _map, const std::string& _word, uint32_t _fileIdx, uint32_t _lineNo, const std::string& _line)
{
   auto mit = _map.find(_word);
   if (mit != _map.end())
   {
      mit->second.push_back(std::make_shared<WordMatchInfo>(_fileIdx, _lineNo, _line));
   }
   else
   {
      std::list<std::shared_ptr<WordMatchInfo>> l;
      l.push_back(std::make_shared<WordMatchInfo>(_fileIdx, _lineNo, _line));
      _map[_word] = l;
   }
}

UINT ThreadGetWords(LPVOID _parm)
{
   WordThreadParam* parm = (WordThreadParam*)_parm;
   for (uint32_t fnIdx = 0; fnIdx < parm->m_fileName.size(); fnIdx++)
   {
      std::ifstream f(parm->m_fileName[fnIdx]);
      std::string line;
      std::list<std::string> wordList;
      uint32_t lineNo = 1;
      while(getline(f, line))
      {
         if (FindWords(wordList, line, parm->m_keyWords, parm->m_seperators, parm->m_ignoreCase))
         {
            for (auto it : wordList)
            {
               AddToWordList(parm->m_wordMap, it, fnIdx, lineNo, line);
            }
         }
         lineNo++;
      }
   }
   wainApp.PostThreadMessage(IDB_WORD_THREAD_DONE, (WPARAM )_parm, 0);
   return 0;
}
