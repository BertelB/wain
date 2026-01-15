#include ".\..\src\stdafx.h"
#include "../src/WainUtil.h"
#include <chrono>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool RemoveLastPath(std::string &aPath)
{
   if(aPath.empty())
      return false;

   if(aPath[aPath.size() - 1] == '\\' || aPath[aPath.size() - 1] == '/')
      aPath = aPath.substr(0, aPath.size() - 1);

   std::string::size_type pos = aPath.find_last_of("\\/");
   if(pos != std::string::npos)
   {
      aPath = aPath.substr(0, pos);
      return true;
   }
   return false;
}

void AddSlash(std::string &aPath)
{
   if(aPath.size())
   {
      char last = aPath[aPath.size() - 1];
      if(last == 0)
      {
         if(aPath.size() > 2)
            last = aPath[aPath.size() - 2];
         else
         {
            aPath += "\\";
            return;
         }
      }
      if(last != '\\' && last != '/')
      {
         if(aPath.find('/') != std::string::npos)
            aPath += "/";
         else
            aPath += "\\";
      }
   }
   else
   {
      aPath += "/";
   }
}

void RemoveSlash(std::string &aPath)
{
   if(!aPath.empty() && aPath.substr(aPath.size() - 1) == "\\")
   {
      aPath = aPath.substr(0, aPath.size() - 1);
   }
}

std::string SubStr(int _start, int _len, const char* _in)
{
   std::string out = _in;
   return out.substr(_start, _len);
}

uint64_t GetUSec()
{
   return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}
