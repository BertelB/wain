#error "Do not use"
#ifndef _myregex_h
#define _myregex_h

#include <regex>
class RegEx
{
public:
   RegEx(const char* _regEx, bool _ignoreCase = false) :
      m_regEx(_regEx, _ignoreCase ? std::regex::icase : std::regex::ECMAScript)
   {
   }

   const char* Find(const char* _str)
   {
      std::cmatch cm;
      std::regex_match (_str, cm, m_regEx, std::regex_constants::match_default);
      uint32_t firstMatch = UINT_MAX;
      for (unsigned i = 1; i < cm.size(); ++i)
      {
         if (uint32_t(cm.position(i)) < firstMatch)
         {
            firstMatch = cm.position(i);
         }
      }
      return firstMatch != UINT_MAX ? _str + firstMatch : nullptr;
   }

   std::vector<std::string> Find(const char* _str, uint32_t _n)
   {
      std::cmatch cm;
      std::regex_match (_str, cm, m_regEx, std::regex_constants::match_default);
      std::vector<std::string> res;
      if (cm.size() < _n + 1)
      {
         return res;
      }
      for (unsigned i = 1; i < cm.size(); ++i)
      {
         res.push_back(cm[i]);
      }
      return res;
   }

private:
   std::regex m_regEx;
};

#endif
