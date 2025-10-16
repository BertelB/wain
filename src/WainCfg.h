#ifndef WAINCFG_H
#define WAINCFG_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

class IniGroup
{
public:
   IniGroup()
   {
   }

   int32_t GetInt(const char* _key, int32_t _default)
   {
      auto kv = m_map.find(_key);
      if (kv != m_map.end())
      {
         uint32_t value = 0;
         value = strtoul(kv->second.c_str(), 0, 16);
         return value;
      }
      return _default;
   }
   void SetInt(const char* _key, int32_t _value)
   {
      char v[128];
      sprintf(v, "%X", _value);
      m_map[_key] = v;
   }
   std::string GetString(const char* _key, const char* _default)
   {
      auto kv = m_map.find(_key);
      if (kv != m_map.end())
      {
         return kv->second;
      }
      return _default;
   }
   void SetString(const char* _key, const char* _value)
   {
      m_map[_key] = _value;
   }
   std::map<std::string, std::string> m_map;
};

class WainIni
{
public:
   WainIni(const char* _fn) :
      m_fn(_fn)
   {
      std::ifstream in(_fn);
      char line[2048];
      std::string cGroup;
      while(in.getline(line, sizeof(line)))
      {
         if (line[0] == '[')
         {
            std::string group = TrimBeginEnd(line);
            // printf("Group: {%s}\n", group.c_str());
            cGroup = group;
            m_table[group] = IniGroup();
         }
         else
         {
            std::string l(line);
            std::string::size_type sep = l.find('=');
            std::string key = l.substr(0, sep);
            std::string value = l.substr(sep + 1);
            // printf("Value: [%s][%s]\n", key.c_str(), value.c_str());
            m_table[cGroup].SetString(key.c_str(), value.c_str());
         }
      }
   }

   void Write()
   {
      std::ofstream out(m_fn);

      for (std::map<std::string, IniGroup>::iterator g = m_table.begin(); g != m_table.end(); g++)
      {
         // printf("[%s]\n", g->first.c_str());
         out << "[" << g->first << "]" << std::endl;
         for (auto& x : g->second.m_map)
         {
            // printf("%s=%s\n", x.first.c_str(), x.second.c_str());
            out << x.first << "=" << x.second << std::endl;
         }
      }
   }

   void SetInt(const char* _group, const char* _key, int32_t _value)
   {
      auto g = m_table.find(_group);
      if (g == m_table.end())
      {
         m_table[_group] = IniGroup();
         g = m_table.find(_group);
      }
      g->second.SetInt(_key, _value);
   }

   void SetBool(const char* _group, const char* _key, bool _value)
   {
      auto g = m_table.find(_group);
      if (g == m_table.end())
      {
         m_table[_group] = IniGroup();
         g = m_table.find(_group);
      }
      g->second.SetInt(_key, _value);
   }

   bool GetBool(const char* _group, const char* _key, bool _default)
   {
      int32_t v = GetInt(_group, _key, _default);
      return v ? true : false;
   }

   int32_t GetInt(const char* _group, const char* _key, int32_t _default)
   {
      auto g = m_table.find(_group);
      if (g != m_table.end())
      {
         return g->second.GetInt(_key, _default);
      }
      return _default;
   }

   std::string GetString(const char* _group, const char* _key, const char* _default)
   {
      auto g = m_table.find(_group);
      if (g != m_table.end())
      {
         return g->second.GetString(_key, _default);
      }
      return _default ? _default : "";
   }

   void SetString(const char* _group, const char* _key, const std::string& _value)
   {
      auto g = m_table.find(_group);
      if (g == m_table.end())
      {
         m_table[_group] = IniGroup();
         g = m_table.find(_group);
      }
      g->second.SetString(_key, _value.c_str());
   }

   void SetString(const char* _group, const char* _key, const char* _value)
   {
      auto g = m_table.find(_group);
      if (g == m_table.end())
      {
         m_table[_group] = IniGroup();
         g = m_table.find(_group);
      }
      g->second.SetString(_key, _value);
   }

private:
   std::map<std::string, IniGroup> m_table;
   std::string m_fn;

   std::string TrimBeginEnd(const char* _input)
   {
      std::string s(_input);
      return s.substr(1, s.size() - 2);
   }

   std::string TrimBeginEnd(const std::string& _input)
   {
      return _input.substr(1, _input.size() - 2);
   }
};

#endif
