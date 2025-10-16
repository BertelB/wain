#ifndef TAGS_H_INC
#define TAGS_H_INC
#include "Resource.h"
#include "ClassView.h"
#include "NavigatorDlg.h"
#include "TagsType.h"
#include "../src/TagIndexType.h"
// This defines how the tags are sorted and how they are displayed on the NavigatorBar
// It must match Smallico.bmp

struct ParseInfo
{
   std::string m_name;
   std::string m_fn;
   std::string m_typename;
   uint32_t m_lineNo;
   char m_type;
   enum AddInfoType
   {
      Inherit,
      Class,
      Struct,
      Signature,
      Typeref,
      Typename
   };
   struct AddInfoSet
   {
      AddInfoType m_infoType;
      std::string m_info;
   };
   std::list<AddInfoSet> m_addInfo;
};

class TagElemClass
{
public:
   TagElemClass(ParseInfo& _parseInfo, class ReadTagClass *aReadTag);

   TagElemClass(TagElemClass *other)
   {
      m_tag = other->m_tag;
      m_signature = other->m_signature;
      m_indexType = other->m_indexType;
      m_nr = other->m_nr;
      m_fileIdx = other->m_fileIdx;
      m_lineNo = other->m_lineNo;
   }
   TagElemClass() {}
   ~TagElemClass()
   {
   }
   std::string m_tag;
   std::string m_signature;
   TagIndexType m_indexType;
   unsigned int m_nr;
   int m_fileIdx;
   int m_lineNo;
private:

};

class TagElemSortClass
{  // To sort the list
public:
   TagElemSortClass(){}
   bool operator() (const TagElemClass &lhs, const TagElemClass &rhs)
   {
      if(lhs.m_indexType != rhs.m_indexType)
         return lhs.m_indexType < rhs.m_indexType;
      return stricmp(lhs.m_tag.c_str(),  rhs.m_tag.c_str()) < 0;
   }
};

class TagElemFindClass
{  // To find a elem, assume all index_type is same
public:
   TagElemFindClass(){}
   bool operator() (const TagElemClass &lhs, const TagElemClass &rhs)
   {
      return stricmp(lhs.m_tag.c_str(), rhs.m_tag.c_str()) < 0;
   }
};

class TagElemFindCaseClass
{  // To find a elem, assume all index_type is same
public:
   TagElemFindCaseClass(){}
   bool operator() (const TagElemClass &lhs, const TagElemClass &rhs)
   {
      return lhs.m_tag < rhs.m_tag;
   }
};

struct CountOffsetType
{
   unsigned int m_count[TagIndexType::NOF_INDEXES];
   unsigned int m_offset[TagIndexType::NOF_INDEXES];
};

class ReadTagParmClass
{
public:
   std::string m_command;
   std::string m_fileName;
   unsigned int m_nr;
   UINT m_readTagsMsgId;
   unsigned int m_priority;
   int m_viewNr;
   AddClassInfoTreeCtrlClass *m_tree;
};

class AutoRebuildTagClass
{
public:
   AutoRebuildTagClass(const std::string &aProg, const std::string &aOptions, const std::string &aTagFile, const std::string &aName)
   {
      m_command = aProg;
      m_command += " ";
      m_command += aOptions;
      if(!strstr(aOptions.c_str(), "--excmd=number"))
        m_command += " --excmd=number";
      m_command += " \"-f";
      m_command += aTagFile;
      m_command += "\" \"";
      m_command += aName;
      m_command += "\"";
   }
   std::string m_command;
private:
   AutoRebuildTagClass(); // Not to be implemented
};


typedef std::vector<AutoRebuildTagClass *>AutoRebuildTagVector;

class TagFileListClass
{
public:
  typedef std::vector<std::string>::size_type SizeType;

  SizeType Add(const char *aFullName);

  const char *GetFullName(SizeType aIndex)
  {
    return aIndex < m_fullName.size() ? m_fullName[aIndex].c_str() : "";
  }
  const char *GetShortName(SizeType aIndex)
  {
    return aIndex < m_shortName.size() ? m_shortName[aIndex].c_str() : "";
  }

private:
  std::vector<std::string> m_fullName;
  std::vector<std::string> m_shortName;
};

class TagListClass
{
public:
   typedef std::vector<TagElemClass *> TagVectorClass;
   TagVectorClass m_tagVector;
   typedef TagVectorClass::iterator  TagVectorIterator;
   typedef TagVectorClass::size_type  TagVectorSizeType;

   TagVectorSizeType GetNofTags() const {return m_tagVector.size();}

   TagListClass()
   {
     memset(&m_countOffset, 0, sizeof(m_countOffset));
   };
   ~TagListClass()
   {
      for(TagVectorSizeType i = 0; i < m_tagVector.size(); i++)
        delete m_tagVector[i];
   }

   const TagElemClass &Get(TagVectorSizeType aIdx) const
   {
      if(aIdx < m_tagVector.size())
         return *m_tagVector[aIdx];
      return m_dummyTag;
   }
   CountOffsetType m_countOffset;
   void Add(TagElemClass *aElem)
   {
      m_tagVector.push_back(aElem);
   }
   void AddDup(TagElemClass *aElem)
   {
      TagElemClass *Dub = new TagElemClass(aElem);
      Add(Dub);
   }
   void PostProcess(void);

   bool Find(TagElemClass &Tag, const char *word);
   bool FindClass(TagElemClass &Tag, const char *word);

   bool IsTag(const char *word);
   // GetTagElemClass* GetClass(const char *aWord);
   void Remove(std::string& aName);

private:
    TagElemClass m_dummyTag;
};

class ReadTagClass
{
public:
   ReadTagClass() :
      m_tagList(0),
      m_fileList(0),
      m_addClassInfo(0),
      m_treeX(0)
   {
   }
   ~ReadTagClass()
   {
      delete m_tagList;
      delete m_fileList;
      delete m_addClassInfo;
      // delete m_treeX;
   }
   TagListClass *GetTagList()
   {
      return m_tagList;
   }
   TagFileListClass *GetFileList()
   {
      return m_fileList;
   }

   AddClassInfoListClass *GetAddClassInfo()
   {
      return m_addClassInfo;
   }

   bool HasAddClassInfo()
   {
      return m_addClassInfo != 0;
   }

   AddClassInfoTreeCtrlClass *GetTree()
   {
      return m_treeX;
   }
   void SetTagList(TagListClass *aTagList)
   {
      m_tagList = aTagList;
   }
   void SetFileList(TagFileListClass *aFileList)
   {
      m_fileList = aFileList;
   }
   void SetAddClassInfo(AddClassInfoListClass *aAddClassInfo)
   {
      m_addClassInfo = aAddClassInfo;
   }
   void SetTree(AddClassInfoTreeCtrlClass* _tree)
   {
      m_treeX = _tree;
   }
   void HandleTyperef(const char* aNewName, const char* aOldName);

   void PostProcessTyperef();

private:
   TagListClass *m_tagList = 0;
   TagFileListClass *m_fileList = 0;
   AddClassInfoListClass *m_addClassInfo = 0;
   AddClassInfoTreeCtrlClass *m_treeX = 0;
   std::list<std::string >m_typerefList;
};

class GetTagElemCompareClass
{
public:
   GetTagElemCompareClass()
   {}
   bool operator () (const class GetTagElemClass *lhs, const class GetTagElemClass *rhs)
   {
      int diff;
      if(lhs->m_indexType != rhs->m_indexType)
         return lhs->m_indexType < rhs->m_indexType;
      if((diff = stricmp(lhs->m_tag.c_str(), rhs->m_tag.c_str())) != 0)
         return diff < 0;
      if(lhs->m_fullName != rhs->m_fullName)
         return lhs->m_fullName < rhs->m_fullName;
      return lhs->m_lineNo < rhs->m_lineNo;
   }
};

class GetTagListClass
{
public:
  GetTagListClass(const std::string &aWord, unsigned int aTypes)
   : m_word(aWord),
     m_types(aTypes)
  {
  }
  ~GetTagListClass()
  {
    std::vector<GetTagElemClass *>::iterator it;
    for(it = m_list.begin(); it != m_list.end(); ++it)
    {
       delete *it;
    }
  }
  void Add(const char *aFullName, const char *aShortName, int aLineNo, const char *aTag, TagIndexType _indexType, const char *aSign = 0)
  {
    GetTagElemClass *elem = new GetTagElemClass(aFullName, aShortName, aLineNo, aTag, _indexType, aSign);
    std::vector<GetTagElemClass *>::iterator it;
    for(it = m_list.begin(); it != m_list.end(); ++it)
    {
       if((*it)->m_fullName == aFullName && (*it)->m_lineNo == aLineNo && (*it)->m_indexType == _indexType)
       {
          if(strlen(aTag) > (*it)->m_tag.size() || elem->m_signature.size() > (*it)->m_signature.size())
          {
             std::swap(*it, elem);
             delete elem;
             return;
          }
          else
          {
             delete elem;
             return;
          }
       }
    }
    m_list.push_back(elem);
  }
   void Sort()
   {
      GetTagElemCompareClass Compare;
      std::sort(m_list.begin(), m_list.end(), Compare);
   }

  std::string m_word;
  unsigned int m_types;
  std::vector<GetTagElemClass *>m_list;
};

#endif
