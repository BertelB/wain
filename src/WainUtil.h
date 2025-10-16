#ifndef _WAIN_UTIL_H_INC
#define _WAIN_UTIL_H_INC

extern WORD WainMessageBox(CWnd *parent, const std::string& _str, WORD flags, WORD icon, const char *CustBText = 0);
extern WORD WainMessageBox(CWnd *parent, const char *msg, WORD flags, WORD icon, const char *CustBText = 0);
extern const UINT TheOtherAppMessage;
extern void ConvToTab(std::string &aStr);
extern void ConvFromTab(std::string &aDst, const std::string &aSrc);
extern void CreateLongFileName(char *short_path);
extern std::string SubStr(int _start, int _len, const char* _in);

#define SP_DRIVE 0x01
#define SP_DIR   0x02
#define SP_FILE  0x04
#define SP_EXT   0x08
extern char *MySplitPath(const char *full_name, unsigned int what, char *dest);
extern const char *MySplitPath(const char *full_name, unsigned int what, CString &dest);
extern const char *MySplitPath(const char *full_name, unsigned int what, std::string &dest);
extern bool PathExist(const char* path);

#define FD_SAVE             0x01
#define FD_OPEN             0x02
#define FD_CONFIG_PATH      0x04
#define FD_LAST_PATH        0x08
#define FD_SET_LAST_PATH    0x10
#define FD_DOC_FILTER       0x20
#define FD_MULTI_FILES      0x40

class MyFileDialogClass : public CFileDialog
{
  unsigned int m_flags;
public:
  MyFileDialogClass(unsigned int flags, unsigned int std_flags, const char *def_ext = NULL, const char *file_name = NULL, const char *filter = NULL, CWnd *parent = NULL);
  virtual int DoModal(void);
};

void AddSlash(std::string &aPath);
void RemoveSlash(std::string &aPath);
bool RemoveLastPath(std::string &aPath);

enum class RtvStatus
{
   rtv_no_error,
   rtv_missing_sep,
   rtv_no_temp_path,
   rtv_no_curr_file,
   rtv_ill_formed_tag,
   rtv_no_make_file,
   rtv_no_project,
   rtv_no_project_files,
   rtv_no_project_inc_path,
   rtv_unknown_tag,
   rtv_string_to_long
};

extern void DisplayRtvError(CWnd* _parent, const char* _msg, RtvStatus _error, bool _fatal = false);

enum class CreateModeType
{
   CREATE_DEFAULT,
   CREATE_MAX,
   CREATE_DUAL /* Must be the last one */
};

enum PrintColorType
{
   PC_BLACK_WHITE,
   PC_DOCUMENT,
   PC_SPECIFIED
};

class ColorType
{
public:
   ColorType() :
      m_textColor(0),
      m_backColor(0xFFFFFFU)
   {
   }

   COLORREF m_textColor;
   COLORREF m_backColor;
   bool operator == (ColorType other);
};

class StringIgnoreCaseCompareClass
{
public:
   StringIgnoreCaseCompareClass()
   {}
   bool operator () (const std::string &lhs, const std::string &rhs)
   {
      return stricmp(lhs.c_str(), rhs.c_str()) < 0;
   }
};

class StringIgnoreCaseFindIfClass
{
public:
   StringIgnoreCaseFindIfClass(const std::string &aToFind)
    : m_toFind(aToFind)
   {
   }
   bool operator () (const std::string &rhs)
   {
      return stricmp(rhs.c_str(), m_toFind.c_str()) == 0;
   }
private:
   StringIgnoreCaseFindIfClass(); // Not to be implemented
   std::string m_toFind;
};

enum ColorIndexType
{
   MARK_IDX,
   TEXT_IDX,
   MATCH_IDX,
   SEP_IDX,
   PRE_IDX,
   COM_IDX,
   STR_IDX,
   NUM_IDX,
   TAG_IDX,
   WORD_IDX,
   KEY_0_IDX,
   KEY_1_IDX,
   KEY_2_IDX,
   KEY_3_IDX,
   KEY_4_IDX,
   NOF_KEY_INDEX,
   TAB_IDX
};

extern const char * const KeyIndexStr[];

class TemplateListClass
{
public:
   TemplateListClass()
   {
   }
   TemplateListClass(const TemplateListClass *aOther)
    : m_name(aOther->m_name),
      m_expansion(aOther->m_expansion)
   {
   }

   std::string m_name;
   std::string m_expansion;
};

class TabPosClass : public std::vector<int  >
{
public:
   int GetNextTabPos(int aPos);
   int GetPrevTabPos(int aPos);
   void FromString(const std::string &aStr);
   std::string ToString();
};

struct MacroParmType
{
  std::string m_fileName;
  std::string m_menuText;
};

class PopupMenuListClass
{
public:
   PopupMenuListClass(const char *aText, WORD aId)
    : m_text(aText),
      m_id(aId)
   {}
   PopupMenuListClass() {}
   std::string m_text;
   WORD m_id;
};
#define MAX_NOF_POPUP_MENU_ITEMS 32


#endif
