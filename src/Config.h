#ifndef _CONFIG_H_INC
#define _CONFIG_H_INC

#include <stdint.h>
#include <string.h>
#include <sstream>
#include <fstream>
#include <map>
#include <iostream>
#include <vector>

class ConfigItemBase
{
public:
   virtual ~ConfigItemBase() {}
   virtual void Read(const std::string& _in) = 0;
   virtual const std::string Write() = 0;
};

class ConfigItemInt : public ConfigItemBase
{
public:
   ConfigItemInt(int32_t& _val) : m_val(_val) {}
   void Read(const std::string& _in)
   {
      m_val = strtol(_in.c_str(), 0, 0);
   }

   const std::string Write()
   {
      std::stringstream ss;
      ss << m_val;
      return ss.str();
   }
private:
   int32_t& m_val;
};

class ConfigItemBool : public ConfigItemBase
{
public:
   ConfigItemBool(bool& _val) : m_val(_val) {}
   void Read(const std::string& _in)
   {
      m_val = strtol(_in.c_str(), 0, 0) ? true : false;
   }

   const std::string Write()
   {
      std::stringstream ss;
      ss << m_val;
      return ss.str();
   }
private:
   bool& m_val;
};

class ConfigItemString : public ConfigItemBase
{
public:
   ConfigItemString(std::string& _val) : m_val(_val) {}
   void Read(const std::string& _in)
   {
      m_val = _in;
   }
   const std::string Write()
   {
      return m_val;
   }
private:
   std::string& m_val;
};

class Config
{
public:
   virtual ~Config()
   {
      Clear();
   }

   void Clear()
   {
      for (auto i : m_map)
      {
         delete i.second;
      }
      m_map.clear();
   }

   void AddItem(const char* _name, int32_t& _val)
   {
      m_map[_name] = new ConfigItemInt(_val);
   }

   void AddItem(const char* _name, bool& _val)
   {
      m_map[_name] = new ConfigItemBool(_val);
   }

   void AddItem(const char* _name, std::string& _val)
   {
      m_map[_name] = new ConfigItemString(_val);
   }
   void Write(const char* _fn)
   {
      std::ofstream o(_fn);
      Write(o);
   }

   void Write(std::ofstream& _o)
   {
      for (auto i : m_map)
      {
         _o << i.first << " " << i.second->Write() << std::endl;
      }
   }
   void Read(const char* _fn)
   {
      std::ifstream i(_fn);
      std::string line;
      while (getline(i, line))
      {
         ReadLine(line);
      }
   }
   void ReadLine(std::string& _line)
   {
      std::string::size_type pos = _line.find(" ");
      if (pos != std::string::npos)
      {
         std::string key = _line.substr(0, pos);
         std::string value = _line.substr(pos + 1);
         // std::cout << "[" << key << "] - [" << value << "]" << std::endl;
         auto i = m_map.find(key);
         if (i != m_map.end())
         {
            // std::cout << "Read[" << key << "] = " << value << std::endl;
            i->second->Read(value);
         }
         // else
         // {
         //    std::cout << "No such key: " << key << std::endl;
         // }
      }
   }

private:
   std::map<std::string, ConfigItemBase*> m_map;
};

template<typename T>
class ConfigArray
{
public:
   ~ConfigArray()
   {
      for (uint32_t i = 0; i < m_array.size(); i++)
      {
         delete m_array[i];
      }
   }
   T& operator [] (uint32_t _idx)
   {
      return *m_array[_idx];
   }

   void Write(const char* _fn)
   {
      std::ofstream o(_fn);
      for (auto i : m_array)
      {
         o << "#" << std::endl;
         i->Write(o);
      }
   }

   void Read(const char* _fn)
   {
      std::ifstream i(_fn);
      std::string line;
      while (getline(i, line))
      {
         if (line[0] == '#')
         {
            m_array.push_back(new T);
         }
         else if (!m_array.empty())
         {
            m_array.back()->ReadLine(line);
         }
      }
   }

   uint32_t Size()
   {
       return m_array.size();
   }

   void Remove(uint32_t _idx)
   {
       delete m_array[_idx];
       m_array.erase(m_array.begin() + _idx);
   }

   void Add(T& _item)
   {
      m_array.push_back(new T(_item));
   }

   void Update(uint32_t _itemNo, T& _item)
   {
      if (_itemNo < m_array.size())
      {
         *m_array[_itemNo] = _item;
      }
   }
private:
   std::vector<T*> m_array;
};

#endif
