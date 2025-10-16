#ifndef _TAGS_TYPE_H_INC
#define _TAGS_TYPE_H_INC
#include <string>
#include "..\src\NavigatorDlgType.h"
#include "../src/TagIndexType.h"

class GetTagElemClass
{
public:
  GetTagElemClass(const char *aFullName, const char *aShortName, int aLineNo, const char *aTag, TagIndexType _indexType, const char *aSign = 0) :
    m_fullName(aFullName),
    m_shortName(aShortName),
    m_lineNo(aLineNo),
    m_tag(aTag),
    m_indexType(_indexType),
    m_signature(aSign ? aSign : "")
  {
  }
  GetTagElemClass(GetTagElemClass *other) :
    m_fullName(other->m_fullName),
    m_shortName(other->m_shortName),
    m_lineNo(other->m_lineNo),
    m_tag(other->m_tag),
    m_indexType(other->m_indexType),
    m_signature(other->m_signature)
  {
  }
  std::string m_fullName;
  std::string m_shortName;
  std::string m_signature;
  int m_lineNo;
  std::string m_tag;
  TagIndexType m_indexType;
};

class TagClass
{
public:
   const NavBarStateType m_state;
   void Clear(bool aSetup = true);
   friend class AddClassInfoTreeCtrlClass;
   friend class AddClassInfoListClass;

   TagClass(NavigatorDialog *aDlg, NavBarStateType aState)
    : m_dlg(aDlg),
      m_state(aState)
   {
      m_tagList = 0;
      m_fileList = 0;
      m_tagThread = 0;
   }
   ~TagClass();
   virtual void UpdateTagList(void) = 0;
   class TagFileListClass* m_fileList;
   class TagListClass* m_tagList;

   UINT m_readTagsMsgId;
   unsigned int m_viewNr;

   void ReadTagFile(const char *aCommand, const char *aFile_name, bool aRebuild, bool aAutoBuild);
   void HandleTagsRead(class ReadTagClass *aReadTag);
   unsigned int m_readNr;
   CWinThread *m_tagThread;
protected:
    NavigatorDialog *m_dlg;
private:
    TagClass();
};

class GlobalTagClass : public TagClass
{
public:
   GlobalTagClass(NavigatorDialog *aDlg) :
      TagClass(aDlg, NavBarStateTags)
   {
      m_viewNr = IDB_LIST_TREE_1;
      m_readTagsMsgId = IDB_GLOB_TAG_DONE;
   }

   bool GetTagList(GetTagListClass *aMatchTag, BOOL aJustClasses);
   bool CheckTag(char *word);

   int SetTag(const char *name);
   void UpdateTagList(void);
private:
   GlobalTagClass();
};

class CurrentTagClass : public TagClass
{
public:
   CurrentTagClass(NavigatorDialog *aDlg) : TagClass(aDlg, NavBarStateCurr)
   {
      m_viewNr = 0;
      m_readTagsMsgId = IDB_TAG_DONE;
   }

  void UpdateTagList(void);
private:
   CurrentTagClass();
};


#endif