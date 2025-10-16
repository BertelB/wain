#ifndef DOC_PROP_H_INC
#define DOC_PROP_H_INC

#include "dialogbase.h"
#include <regex>

std::string ConcatStringList(const std::list<std::string>& aList, char aSep);
std::list<std::string> SplitStringList(const std::string& aString, char aSep);

class LangSelectDialogClass : public DialogBaseClass
{
private:
   int m_currentSel;
public:
   LangSelectDialogClass(CWnd *aParent);
   ~LangSelectDialogClass();
   enum {IDD = IDD_LANG_SEL_SETUP};
protected:
   virtual BOOL OnInitDialog();
   afx_msg void TypeChanged(void);
   afx_msg void ExtChanged(void);
   afx_msg void Select(void);
   afx_msg void OnOk(void);
   afx_msg void Clone(void);
   afx_msg void OnNew(void);
   afx_msg void Remove(void);
   DECLARE_MESSAGE_MAP();
};

class LangSelectDialogNewTypeDialogClass : public DialogBaseClass
{
public:
   std::string m_type;
   std::string m_ext;
   std::string m_fileName;
   LangSelectDialogNewTypeDialogClass(CWnd *aParent);
   ~LangSelectDialogNewTypeDialogClass(void);
   enum {IDD = IDD_LANG_SEL_NEW_SETUP};
protected:
   void DoDataExchange(CDataExchange *aDx);
   virtual BOOL OnInitDialog();
   afx_msg void OnOK(void);
   DECLARE_MESSAGE_MAP();
};

extern int KeywordSortFunc(const void *p1, const void *p2);
extern int KeywordICaseSortFunc(const void *p1, const void *p2);

class ColorEditBaseClass : public CEdit
{
public:
   class ColorDialogBaseClass *m_dlg;
   CFont *m_font;
   bool m_init;
   ColorEditBaseClass() : m_init(false) {}
};

class ColorEditClass : public ColorEditBaseClass
{
public:
  afx_msg void OnPaint(void);
  DECLARE_MESSAGE_MAP();
};

class TabViewEditClass : public ColorEditBaseClass
{
public:
  afx_msg void OnPaint(void);
  DECLARE_MESSAGE_MAP();
};

class ColorDialogBaseClass : public CPropertyPage
{
  DECLARE_DYNAMIC(ColorDialogBaseClass);
public:
  ColorDialogBaseClass(const char * const *aIndexStr) :
     m_indexStr(aIndexStr),
     CPropertyPage(ColorDialogBaseClass::IDD) {}

  ColorEditBaseClass *m_edit;

  enum {IDD = IDD_COLOR};
  int m_sel;
  virtual void DoDataExchange(CDataExchange *aDx);
  virtual BOOL OnKillActive( );
  ColorType *m_color;
  int m_numColor;
  const char * const *m_indexStr;

protected:

  virtual BOOL OnInitDialog();
  afx_msg void RedChanged(void);
  afx_msg void GreenChanged(void);
  afx_msg void BlueChanged(void);
  afx_msg void RedBackChanged(void);
  afx_msg void GreenBackChanged(void);
  afx_msg void BlueBackChanged(void);
  afx_msg void NewSel(void);
  afx_msg void LoadScheme(void);
  afx_msg void SaveScheme(void);
  afx_msg void VerifyColor(void);
  afx_msg void SelForg(void);
  afx_msg void SelBack(void);
  DECLARE_MESSAGE_MAP();
};

class DocColorDialogClass : public ColorDialogBaseClass
{
  DECLARE_DYNAMIC(DocColorDialogClass);

public:
  DocColorDialogClass(ColorType *aOrgColor,
                      int aNumColor,
                      const char * const *aIndexStr) : ColorDialogBaseClass(aIndexStr)
  {
    m_color = aOrgColor;
    m_numColor = aNumColor;
    m_edit = new ColorEditClass;
    m_edit->m_dlg = this;
  }
  ~DocColorDialogClass()
  {
    delete m_edit;
  }
  virtual BOOL OnInitDialog();
};

class PrintColorDialogClass : public ColorDialogBaseClass
{
  DECLARE_DYNAMIC(PrintColorDialogClass);

public:
  PrintColorDialogClass(ColorType *aOrgColor,
                        int aNumColor,
                        const char * const *aIndexStr) : ColorDialogBaseClass(aIndexStr)
  {
    m_color = aOrgColor;
    m_numColor = aNumColor;
    m_edit = new ColorEditClass;
    m_edit->m_dlg = this;
  }
  ~PrintColorDialogClass()
  {
    delete m_edit;
  }
  virtual BOOL OnInitDialog();
};

class TabViewColorDialogClass : public ColorDialogBaseClass
{
  DECLARE_DYNAMIC(TabViewColorDialogClass);
public:
  TabViewColorDialogClass(ColorType *aOrgColor, int aNumColors, const char * const *aIndexStr) : ColorDialogBaseClass(aIndexStr)
  {
    m_color = aOrgColor;
    m_numColor = aNumColors;
    m_edit = new TabViewEditClass();
    m_edit->m_dlg = this;
  }
  ~TabViewColorDialogClass()
  {
    delete m_edit;
  }
  virtual BOOL OnInitDialog();
};

inline std::string::size_type FindStrX(const std::string &aStr, char aMatch)
{
   std::string::size_type n = 0;
   const char *src = aStr.c_str();
   while(*src)
   {
      if(src[0] == '\\' && src[1] == '\\')
      {
         src += 2;
         n += 2;
      }
      else if(src[0] == '\\' && src[1] == aMatch)
         return n;
      else
      {
         src++;
         n++;
      }
   }
   return std::string::npos;
}

class DocPropClass
{
public:
   DocPropClass()
   {
   }
   DocPropClass & operator = (const DocPropClass& rhs)
   {
      Assign(rhs);
      return *this;
   }
   DocPropClass(const DocPropClass &rhs)
   {
      Assign(rhs);
   }
   void CreateDefault();

   void Assign(const DocPropClass &rhs);

   ~DocPropClass()
   {
      std::vector<TemplateListClass *>::iterator idx;
      for(idx = m_templateList.begin(); idx < m_templateList.end(); idx++)
         delete *idx;
      m_templateList.clear();
   }
   typedef std::vector<std::string >KeyWordList;
   KeyWordList m_keyWordList[5];
   ColorType m_color[NOF_KEY_INDEX];
   std::string m_fileName;
   std::string m_extensions;
   std::string m_extensionType;
   std::string m_seps;
   std::string m_stringDelim;
   std::string m_chDelim;
   std::string m_braces;
   std::string m_blockBegin;
   std::string m_blockEnd;
   std::list<std::string> m_blockBeginWords;
   std::list<std::string> m_blockEndWords;

   std::string m_lineComment;
   std::string m_commentBegin;
   std::string m_commentEnd;
   std::string m_preProcessor;
   std::string m_literal;
   std::string m_lineCon;
   bool m_commentWholeWord;
   bool m_commentFirstWord;
   bool m_ignoreCase;
   bool m_modified;
   std::string m_tagProgram;
   std::string m_tagOptions;
   std::string m_tagFile;
   std::vector<std::string> m_indentAfter;
   std::vector<std::string> m_indentUnless;
   std::vector<std::string> m_unindent;

   int  m_tabSize;
   int  m_indent;
   bool m_useTagColor;

   std::vector<TemplateListClass *>m_templateList;

   TabPosClass m_tabPos;
   std::vector<std::string >m_preProcWord;
   std::string m_numberStr;
   std::regex m_numberRegEx;
};
#endif
