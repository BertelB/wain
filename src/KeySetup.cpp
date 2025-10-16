//=============================================================================
// This source code file is a part of Wain.
// It implements the KeySetupDialogClass as defined in KeySetup.h,
// and various helper functions for mapping function names to ID's.
//=============================================================================
#include ".\..\src\stdafx.h"

#include ".\..\src\wain.h"
#include ".\..\src\mainfrm.h"
#include ".\..\src\KeySetup.h"
#include <iterator>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NOF_KEY_TEXTS (sizeof(KeyTextMap)/sizeof(KeyTextMap[0]))
static WORD KeyName2KeyId(const char *name);
static const char *KeyId2KeyName(WORD id);
static BOOL String2KeyState(WORD *key, BYTE *st, const char *text);
static BOOL KeyState2String(WORD key, BYTE st, char *text);
static void MakePopupMenuAmpersand(std::vector<PopupMenuListClass >&aMenu);

struct KeyTextMapType
{
  WORD m_id;
  const char *m_text;
};

KeyTextMapType KeyTextMap[] =
{
  {IDM_SET_FONT,                    "SelectFont"},
  {IDM_SET_NAVIGATOR_FONT,          "SelectNavigatorListFont"},
  {IDM_VIEW_NAVIGATOR_BAR,          "ViewNavigatorBar"},
  {IDM_SEL_NAVIGATOR_BAR,           "SelectNavigatorBar"},
  {IDM_SEL_NAVIGATOR_LIST,          "SelectNavigatorList"},
  {IDM_NEXT_WIN,                    "NextWindow"},
  {IDM_NEXT_WIN_HERE,               "NextWindowHere"},
  {IDM_PREV_WIN_HERE,               "PreviousWindowHere"},
  {IDM_PREV_WIN,                    "PreviousWindow"},
  {IDM_SWAP_WIN,                    "SwapWindow"},
  {IDM_OTHER_WIN,                   "OtherWindow"},
  {IDM_VIEW_DIR,                    "ViewDir"},
  {IDM_VIEW_TAGS,                   "ViewTags"},
  {IDM_VIEW_PROJ,                   "ViewProject"},
  {IDM_VIEW_CURR,                   "ViewCurrentTags"},
  {IDM_VIEW_FILES,                  "ViewOpenFiles"},
  {IDM_VIEW_CLASS,                  "ViewClass"},
  {IDM_TOOL_SETUP,                  "ToolSetup"},
  {IDM_VIEW_TOOLBAR,                "ViewToolBar"},
  {IDM_MACRO_SAVE,                  "MacroSave"},
  {IDM_MACRO_LOAD,                  "MacroLoad"},
  {IDM_PROJ_SETUP,                  "ProjectSetup"},
  {IDM_PROJ_MAKE,                   "ProjectMake"},
  {IDM_PROJ_MAKE_0,                 "ProjectMake1"},
  {IDM_PROJ_MAKE_1,                 "ProjectMake2"},
  {IDM_PROJ_MAKE_2,                 "ProjectMake3"},
  {IDM_PROJ_MAKE_3,                 "ProjectMake4"},
  {IDM_PROJ_EXE,                    "ProjectExecute"},
  {IDM_PROJ_EXE_1,                   "ProjectExecute1"},
  {IDM_PROJ_EXE_2,                   "ProjectExecute2"},
  {IDM_PROJ_EXE_3,                   "ProjectExecute3"},
  {IDM_PROJ_EXE_4,                   "ProjectExecute4"},
  {IDM_PROJ_OPEN,                   "ProjectOpen"},
  {IDM_PROJ_SAVE,                   "ProjectSave"},
  {IDM_PROJ_ADD_CUR_FILE,           "ProjectAddCurrentFile"},
  {IDM_KEY_SETUP,                   "KeyboardSetup"},
  {IDV_MOVE_LEFT,                   "MoveLeft"},
  {IDV_MOVE_RIGHT,                  "MoveRight"},
  {IDV_MOVE_UP,                     "MoveUp"},
  {IDV_MOVE_DOWN,                   "MoveDown"},
  {IDV_MOVE_PAGE_DOWN,              "MovePageDown"},
  {IDV_MOVE_PAGE_UP,                "MovePageUp"},
  {IDV_MOVE_HOME,                   "MoveHome"},
  {IDV_MOVE_HOME_EX,                "MoveHomeEx"},
  {IDV_MOVE_END,                    "MoveEnd"},
  {IDV_SAVE_FILE,                   "SaveFile"},
  {IDV_SAVE_FILE_AS,                "SaveFileAs"},
  {IDV_HANDLE_RETURN,               "Return"},
  {IDV_HANDLE_DELETE,               "Delete"},
  {IDV_HANDLE_BACKSPACE,            "Backspace"},
  {IDV_HANDLE_BACKSPACE_EX,         "BackspaceEx"},
  {IDV_GOTO_DOC_0,                  "GotoDoc0"},
  {IDV_GOTO_DOC_1,                  "GotoDoc1"},
  {IDV_GOTO_DOC_2,                  "GotoDoc2"},
  {IDV_GOTO_DOC_3,                  "GotoDoc3"},
  {IDV_GOTO_DOC_4,                  "GotoDoc4"},
  {IDV_GOTO_DOC_5,                  "GotoDoc5"},
  {IDV_GOTO_DOC_6,                  "GotoDoc6"},
  {IDV_GOTO_DOC_7,                  "GotoDoc7"},
  {IDV_GOTO_DOC_8,                  "GotoDoc8"},
  {IDV_GOTO_DOC_9,                  "GotoDoc9"},
  {IDV_COPY_CLIP,                   "CopyToClipboard"},
  {IDV_MARK_DOWN,                   "MarkDown"},
  {IDV_MARK_UP,                     "MarkUp"},
  {IDV_MARK_LEFT,                   "MarkLeft"},
  {IDV_MARK_RIGHT,                  "MarkRight"},
  {IDV_PASTE_CLIP,                  "PasteFromClipboard"},
  {IDV_MARK_CUT,                    "MarkCut"},
  {IDV_MARK_DELETE,                 "MarkDelete"},
  {IDV_MDI_MAX,                     "MaximizeWindow"},
  {IDV_MDI_CAS,                     "CascadeWindow"},
  {IDV_FILE_OPEN_IN_LINE,           "OpenFileInLine"},
  {IDV_EDIT_UNDO,                   "Undo"},
  {IDV_INSERT_TOGGLE,               "InsertToggle"},
  {IDV_MOVE_WORD_RIGHT,             "MoveWordRight"},
  {IDV_MOVE_WORD_LEFT,              "MoveWordLeft"},
  {IDV_MARK_WORD_RIGHT,             "MarkWordRight"},
  {IDV_MARK_WORD_LEFT,              "MarkWordLeft"},
  {IDV_MARK_PAGE_UP,                "MarkPageUp"},
  {IDV_MARK_PAGE_DOWN,              "MarkPageDown"},
  {IDV_MARK_END,                    "MarkEnd"},
  {IDV_MARK_HOME,                   "MarkHome"},
  {IDV_MARK_DOC_START,              "MarkDocumentStart"},
  {IDV_MARK_DOC_END,                "MarkDocumentEnd"},

  {IDM_SEARCH,                      "Search"},
  {IDM_REPLACE,                     "Replace"},
  {IDM_PROJECT_REPLACE,             "ProjectReplace"},
  {IDM_GLOBAL_REPLACE,              "GlobalReplace"},
  {IDV_MARK_WORD,                   "MarkWord"},
  {IDV_INSERT_TAB,                  "InsertTab"},
  {IDV_MDI_DUAL,                    "DualWindows"},
  {IDV_SHIFT_BLOCK_RIGHT,           "ShiftBlockRight"},
  {IDV_SHIFT_BLOCK_LEFT,            "ShiftBlockLeft"},
  {IDV_FIND_MATCH_BRACE,            "FindMatchingBrace"},
  {IDV_FIND_MATCH_BLOCK,            "FindMatchingBlock"},
  {IDV_COPY_SPECIAL,                "CopySpecial"},
  {IDV_CUT_SPECIAL,                 "CutSpecial"},
  {IDV_PASTE_SPECIAL,               "PasteSpecial"},
  {IDV_COMP_LIST,                   "CompletionList"},
  {IDV_WORD_COMP,                   "WordCompletion"},
  {IDM_FILE_OPEN,                   "OpenFile"},
  {IDM_FILE_NEW,                    "NewFile"},
  {IDV_RELOAD_FILE,                 "ReloadFile"},
  {IDM_CLOSE_FILE,                  "CloseFile"},
  {ID_WORD_LIST_REDO,               "ListWordsAgain"},
  {ID_WORD_LIST,                    "ListWords"},
  {ID_TAG_LIST,                     "ListTags"},
  {ID_TAG_LIST_CLASS,               "ListTagsClass"},
  {ID_TAG_LIST_DEFINE,              "ListTagsDefine"},
  {ID_TAG_LIST_TYPE,                "ListTagsType"},
  {ID_TAG_LIST_STRUCT,              "ListTagsStruct"},
  {ID_TAG_LIST_FUNCTION,            "ListTagsFunction"},
  {ID_TAG_LIST_PROTOTYPE,           "ListTagsPrototype"},
  {ID_TAG_LIST_ENUM_NAME,           "ListTagsEnumName"},
  {ID_TAG_LIST_ENUM,                "ListTagsEnum"},
  {ID_TAG_LIST_VAR,                 "ListTagsVar"},
  {ID_TAG_LIST_EXTERN,              "ListTagsExtern"},
  {ID_TAG_LIST_MEMBER,              "ListTagsMember"},
  {IDV_COPY_FILE_NAME,              "CopyFileNameToClipboard"},
  {IDM_MACRO_RECORD,                "MacroRecordToggle"},
  {IDM_MACRO_PLAYBACK,              "MacroPlayback"},
  {IDM_APP_EXIT,                    "Exit"},
  {IDV_COLUMN_MARK,                 "ColumnMarkToggle"},
  {IDV_MOVE_DOC_START,              "MoveDocumentStart"},
  {IDV_SWITCH_CPP_H,                "SwitchCppVsH"},
  {IDV_MOVE_DOC_END,                "MoveDocumentEnd"},
  {IDV_GOTO_LINE_NO,                "GotoLineNo"},
  {IDV_INC_SEARCH,                  "IncrementalSearch"},
  {IDV_SEARCH_NEXT,                 "SearchNext"},
  {IDV_SEARCH_PREV,                 "SearchPrevious"},
  {IDM_SET_BOOKMARK_0,              "BookmarkSet0"},
  {IDM_SET_BOOKMARK_1,              "BookmarkSet1"},
  {IDM_SET_BOOKMARK_2,              "BookmarkSet2"},
  {IDM_SET_BOOKMARK_3,              "BookmarkSet3"},
  {IDM_SET_BOOKMARK_4,              "BookmarkSet4"},
  {IDM_SET_BOOKMARK_5,              "BookmarkSet5"},
  {IDM_SET_BOOKMARK_6,              "BookmarkSet6"},
  {IDM_SET_BOOKMARK_7,              "BookmarkSet7"},
  {IDM_SET_BOOKMARK_8,              "BookmarkSet8"},
  {IDM_SET_BOOKMARK_9,              "BookmarkSet9"},
  {IDM_JUMP_TO_BOOKMARK_0,          "BookmarkGoto0"},
  {IDM_JUMP_TO_BOOKMARK_1,          "BookmarkGoto1"},
  {IDM_JUMP_TO_BOOKMARK_2,          "BookmarkGoto2"},
  {IDM_JUMP_TO_BOOKMARK_3,          "BookmarkGoto3"},
  {IDM_JUMP_TO_BOOKMARK_4,          "BookmarkGoto4"},
  {IDM_JUMP_TO_BOOKMARK_5,          "BookmarkGoto5"},
  {IDM_JUMP_TO_BOOKMARK_6,          "BookmarkGoto6"},
  {IDM_JUMP_TO_BOOKMARK_7,          "BookmarkGoto7"},
  {IDM_JUMP_TO_BOOKMARK_8,          "BookmarkGoto8"},
  {IDM_JUMP_TO_BOOKMARK_9,          "BookmarkGoto9"},
  {IDM_BOOKMARK_LIST,               "BookmarkList"},
  {IDM_UNDO_JUMP_TO_TAG,            "UndoJumpToTag"},
  {IDV_SCROLL_LINE_DOWN,            "ScrollLineDown"},
  {IDV_SCROLL_LINE_UP,              "ScrollLineUp"},
  {IDV_DELETE_WORD,                 "DeleteWord"},
  {IDV_INCREASE_MARK_TOP,           "IncreaseMarkTop"},
  {IDV_INCREASE_MARK_BOT,           "IncreaseMarkBot"},
  {IDV_DECREASE_MARK_TOP,           "DecreaseMarkTop"},
  {IDV_DECREASE_MARK_BOT,           "DecreaseMarkBot"},
  {IDV_REMOVE_MARK,                 "RemoveMark"},
  {IDM_GOTO_OTHER_VIEW,             "GotoOtherView"},
  {IDM_TOOL_OUTPUT_NEXT,            "ToolOutputNext"},
  {IDM_TOOL_OUTPUT_PREV,            "ToolOutputPrev"},
  {IDM_ASSOC_LIST,                  "AssociationList"},
  {IDV_EXPAND_TEMPLATE,             "TemplateExpand"},
  {IDV_INDENT_LINE,                 "IndentLine"},
  {IDM_VIEW_POPUP_MENU,             "PopupMenu"},
  {IDM_POPUP_MENU_SETUP,            "PopupMenuSetup"},
  {IDM_SAVE_ALL_DOC,                "SaveAllFiles"},
  {IDV_MARK_TOLOWER,                "ToLower"},
  {IDV_MARK_TOUPPER,                "ToUpper"},
  {IDM_CURR_DOC_PROP_SETUP,         "CurrDocProp"},
  {IDV_MOVE_NEXT_TAB,               "MoveToNextTab"},
  {IDV_MOVE_PREV_TAB,               "MoveToPrevTab"},
  {IDV_INDENT_NEXT_TAB,             "IndentToNextTab"},
  {IDV_INDENT_PREV_TAB,             "IndentToPrevTab"},
  {IDV_FILE_INFO,                   "FileInfo"},
  {IDV_OPEN_AS_DOC,                 "OpenAsNormalDoc"},
  {IDV_TAG_PEEK,                    "TagPeek"},
  {IDV_SCROLL_LOCK,                 "ScrollLock"},
  {IDM_PROJ_MANAGE,                 "ManageProjectFiles"},
  {IDM_LAST_AUTO_TAG,               "JumpToPreviousAutoTag"},
  {IDV_PRINT,                       "Print"},
  {IDV_MOVE_MARK_START,             "MoveToMarkStart"},
  {IDV_MOVE_MARK_END,               "MoveToMarkEnd"},
  {IDM_USER_MENU_SETUP,             "UserMenuSetup"},
  {IDV_HANDLE_DELETE_EX,            "DeleteEx"},
  {IDV_MACRO_REPEAT,                "MacroRepeat"},
  {IDV_MOVE_LEFT_EX,                "MoveLeftEx"},
  {IDV_MOVE_RIGHT_EX,               "MoveRightEx"},
  {IDV_INSERT_DATE,                 "InsertDate"},
  {IDV_INSERT_TIME,                 "InsertTime"},
  {IDV_INSERT_DATE_TIME,            "InsertDateTime"},
  {IDV_INSERT_TIME_DATE,            "InsertTimeDate"},
  {IDV_INSERT_USER_ID,              "InsertUserID"},
  {IDV_REMOVE_TABS,                 "RemoveTabs"},
  {IDV_HANDLE_RETURN_EX,            "ReturnEx"},
  {IDV_GOTO_BLOCK_START,            "GotoBlockStart"},
  {IDV_GOTO_BLOCK_END,              "GotoBlockEnd"},
  {IDV_SHELL_CONTEXT_MENU,          "ShellContextMenu"},
  {IDM_REDO_LAST_TAG,               "RedoTag"},
  {IDV_MOVE_WORD_LEFT_EX,           "MoveWordLeftEx"},
  {IDV_MOVE_WORD_RIGHT_EX,          "MoveWordRightEx"},
  {IDV_SCROLL_LINE_CENTER,          "ScrollLineCenter"},
  {IDV_SCROLL_LINE_TOP,             "ScrollLineTop"},
  {IDV_SCROLL_LINE_BOTTOM,          "ScrollLineBottom"},
  {IDV_NEXT_MATCH_WORD,             "NextMatchWord"},
  {IDV_PREV_MATCH_WORD,             "PrevMatchWord"},
  {IDV_INSERT_COMMENT,              "InsertComment"},
  {IDV_INSERT_COMMENT_DOWN,         "InsertCommentDown"},
  {IDV_MARK_ALL,                    "MarkAll"},
  {IDV_OPEN_DIR_FOR_FILE,           "OpenDirForFile"},
  {IDV_OPEN_PROJECT_FOR_FILE,       "OpenProjectForFile"},
  {IDV_MARK_ALL_WORDS,              "MarkMatchWords"},
  {IDV_MARK_ALL_WORDS_TOGGLE,       "MarkMatchWordsToggle"},
  {IDV_MARK_ALL_WORDS_OFF,          "MarkMatchWordsOff"},
//  {IDM_SEND_TOOL_CMD,               "SendToolCmd"}
};

#define KEY_MAP_ENTRY(id) {VK_##id, #id}
#define KEY_MAP_ENTRY_CHAR(id) {#id[0], #id}
#define NOF_KEY_MAP (sizeof(KeyMap)/sizeof(KeyMap[0]))

typedef struct KeyMapType
{
  WORD m_id;
  const char *m_text;
}KeyMapType;

KeyMapType KeyMap[] =
{
  KEY_MAP_ENTRY(BACK),
  KEY_MAP_ENTRY(TAB),
  KEY_MAP_ENTRY(RETURN),
  KEY_MAP_ENTRY(PAUSE),
  KEY_MAP_ENTRY(CAPITAL),
  KEY_MAP_ENTRY(SPACE),
  KEY_MAP_ENTRY(PRIOR),
  KEY_MAP_ENTRY(NEXT),
  KEY_MAP_ENTRY(END),
  KEY_MAP_ENTRY(HOME),
  KEY_MAP_ENTRY(LEFT),
  KEY_MAP_ENTRY(UP),
  KEY_MAP_ENTRY(RIGHT),
  KEY_MAP_ENTRY(DOWN),
  KEY_MAP_ENTRY(INSERT),
  KEY_MAP_ENTRY(DELETE),
  KEY_MAP_ENTRY(NUMPAD0),
  KEY_MAP_ENTRY(NUMPAD1),
  KEY_MAP_ENTRY(NUMPAD2),
  KEY_MAP_ENTRY(NUMPAD3),
  KEY_MAP_ENTRY(NUMPAD4),
  KEY_MAP_ENTRY(NUMPAD5),
  KEY_MAP_ENTRY(NUMPAD6),
  KEY_MAP_ENTRY(NUMPAD7),
  KEY_MAP_ENTRY(NUMPAD8),
  KEY_MAP_ENTRY(NUMPAD9),
  KEY_MAP_ENTRY(MULTIPLY),
  KEY_MAP_ENTRY(ADD),
  KEY_MAP_ENTRY(SUBTRACT),
  KEY_MAP_ENTRY(DECIMAL),
  KEY_MAP_ENTRY(DIVIDE),
  KEY_MAP_ENTRY(F1),
  KEY_MAP_ENTRY(F2),
  KEY_MAP_ENTRY(F3),
  KEY_MAP_ENTRY(F4),
  KEY_MAP_ENTRY(F5),
  KEY_MAP_ENTRY(F6),
  KEY_MAP_ENTRY(F7),
  KEY_MAP_ENTRY(F8),
  KEY_MAP_ENTRY(F9),
  KEY_MAP_ENTRY(F10),
  KEY_MAP_ENTRY(F11),
  KEY_MAP_ENTRY(F12),
  KEY_MAP_ENTRY(NUMLOCK),
  KEY_MAP_ENTRY(SCROLL),
  KEY_MAP_ENTRY_CHAR(A),
  KEY_MAP_ENTRY_CHAR(B),
  KEY_MAP_ENTRY_CHAR(C),
  KEY_MAP_ENTRY_CHAR(D),
  KEY_MAP_ENTRY_CHAR(E),
  KEY_MAP_ENTRY_CHAR(F),
  KEY_MAP_ENTRY_CHAR(G),
  KEY_MAP_ENTRY_CHAR(H),
  KEY_MAP_ENTRY_CHAR(I),
  KEY_MAP_ENTRY_CHAR(J),
  KEY_MAP_ENTRY_CHAR(K),
  KEY_MAP_ENTRY_CHAR(L),
  KEY_MAP_ENTRY_CHAR(M),
  KEY_MAP_ENTRY_CHAR(N),
  KEY_MAP_ENTRY_CHAR(O),
  KEY_MAP_ENTRY_CHAR(P),
  KEY_MAP_ENTRY_CHAR(Q),
  KEY_MAP_ENTRY_CHAR(R),
  KEY_MAP_ENTRY_CHAR(S),
  KEY_MAP_ENTRY_CHAR(T),
  KEY_MAP_ENTRY_CHAR(U),
  KEY_MAP_ENTRY_CHAR(V),
  KEY_MAP_ENTRY_CHAR(W),
  KEY_MAP_ENTRY_CHAR(X),
  KEY_MAP_ENTRY_CHAR(Y),
  KEY_MAP_ENTRY_CHAR(Z),
  KEY_MAP_ENTRY_CHAR(0),
  KEY_MAP_ENTRY_CHAR(1),
  KEY_MAP_ENTRY_CHAR(2),
  KEY_MAP_ENTRY_CHAR(3),
  KEY_MAP_ENTRY_CHAR(4),
  KEY_MAP_ENTRY_CHAR(5),
  KEY_MAP_ENTRY_CHAR(6),
  KEY_MAP_ENTRY_CHAR(7),
  KEY_MAP_ENTRY_CHAR(8),
  KEY_MAP_ENTRY_CHAR(9),
  KEY_MAP_ENTRY(APPS),
  {0xBC, "COMMA"},
  {0xBE, "PERIOD"}
};

BOOL ReadKeySetupFile(const char *file_name, ACCEL *ac, size_t *nof_entrys)
{
  FILE *f = fopen(file_name, "rt");
  if(f)
  {
    char line[1024];
    char command[256];
    for(*nof_entrys = 0; fgets(line, 1024, f); )
    {
      char *s;
      if((s = strchr(line, '\n')) != NULL)
        *s = '\0';
      if((s = strchr(line, ';')) != NULL)
        *s = '\0';

      if(strlen(line))
      {
        if(line[0] == '\"')
        {
          int i = 0;
          do
          {
            command[i] = line[i + 1];
            i++;
          }
          while(line[i] != '\"');
          command[i - 1] = 0;
          s = strtok(&line[i + 1], "");
        }
        else
        {
          s = strtok(line, " ");
          strcpy(command, s);
        }

        ASSERT(s != NULL);
        if((ac[*nof_entrys].cmd = FuncName2MsgId(command)) == 0)
        {
        }
        else
        {
          WORD key;
          BYTE st;
          if(s)
            while(*s == ' ')
              s++;
          if(!s || !String2KeyState(&key, &st, s))
          {
          }
          else
          {
            ac[*nof_entrys].key = key;
            ac[*nof_entrys].fVirt = st;
            (*nof_entrys)++;
          }
        }
      }
    }
    fclose(f);
    return TRUE;
  }
  return FALSE;
}

BOOL WriteKeySetupFile(const char *file_name, ACCEL *ac, size_t nof_entrys)
{
  FILE *f = fopen(file_name, "wt");
  if(f)
  {
    size_t i;
    char key_text[256];
    for(i = 0; i < nof_entrys; i++)
    {
      KeyState2String(ac[i].key, ac[i].fVirt, key_text);
      fprintf(f, "\"%s\" %s\n", MsgId2FuncName(ac[i].cmd), key_text);
    }
    fclose(f);
    return TRUE;
  }
  return FALSE;
}

const char *MsgId2FuncName(WORD id)
{
  int i;
  for(i = 0; i < NOF_KEY_TEXTS && KeyTextMap[i].m_id != id; i++)
  {/* Nothing */ }
  if(i == NOF_KEY_TEXTS)
  {
    if(id >= IDM_RUN_TOOL_0 && id <= IDM_RUN_TOOL_19)
      return wainApp.gs.m_toolParm[id - IDM_RUN_TOOL_0].m_menuText.c_str();
    else if(id >= IDM_RUN_MACRO_0 &&
            id <= IDM_RUN_MACRO_LAST &&
            size_t(id - IDM_RUN_MACRO_0) < wainApp.gs.m_macroParmList.size())
    {
      return wainApp.gs.m_macroParmList[id - IDM_RUN_MACRO_0].m_menuText.c_str();
    }
  }
  return i < NOF_KEY_TEXTS ? KeyTextMap[i].m_text : NULL;
}

WORD FuncName2MsgId(const char *name)
{
  size_t i;
  for(i = 0; i < NOF_KEY_TEXTS && strcmp(KeyTextMap[i].m_text, name); i++)
  {/* Nothing */ }
  if(i == NOF_KEY_TEXTS)
  {
    size_t j;
    for(j = 0; j < wainApp.gs.m_toolParm.Size(); j++)
      if(wainApp.gs.m_toolParm[j].m_menuText == name)
        return (WORD )(j + IDM_RUN_TOOL_0);
    for(j = 0; j < wainApp.gs.m_macroParmList.size(); j++)
      if(wainApp.gs.m_macroParmList[j].m_menuText == name)
        return (WORD )(j + IDM_RUN_MACRO_0);
  }
  return i < NOF_KEY_TEXTS ? KeyTextMap[i].m_id : (WORD )0;
}

static WORD KeyName2KeyId(const char *name)
{
  int i;
  for(i = 0; i < NOF_KEY_MAP && strcmp(KeyMap[i].m_text, name); i++);
  return i < NOF_KEY_MAP ? KeyMap[i].m_id : (WORD )0;
}

static const char *KeyId2KeyName(WORD id)
{
  int i;
  for(i = 0; i < NOF_KEY_MAP && KeyMap[i].m_id != id; i++);
  return i < NOF_KEY_MAP ? KeyMap[i].m_text : NULL;
}


KeySetupDialogClass::KeySetupDialogClass(CWnd *parent) : CDialog(KeySetupDialogClass::IDD, parent)
{
  m_orgAccel = NULL;
  m_state = FVIRTKEY;
  m_current = CURR_NONE;
}

void KeySetupDialogClass::DoDataExchange(CDataExchange *dx)
{
	CDialog::DoDataExchange(dx);
}

BOOL KeySetupDialogClass::OnInitDialog(void)
{
  CListBox *func = (CListBox *)GetDlgItem(IDC_KEY_FUNC);
  unsigned int i;
  ASSERT(func);
  for(i = 0; i < NOF_KEY_TEXTS; i++)
    func->AddString(KeyTextMap[i].m_text);
  size_t j;
  for(j = 0; j < wainApp.gs.m_toolParm.Size(); j++)
    func->AddString(wainApp.gs.m_toolParm[j].m_menuText.c_str());
  for(j = 0; j < wainApp.gs.m_macroParmList.size(); j++)
    func->AddString(wainApp.gs.m_macroParmList[j].m_menuText.c_str());

  CListBox *key = (CListBox *)GetDlgItem(IDC_KEY_KEY);
  if(key)
  {
    for(i = 0; i < NOF_KEY_MAP; i++)
      key->AddString(KeyMap[i].m_text);
  }
  ASSERT(m_orgAccel);
  memcpy(m_accel, m_orgAccel, sizeof(m_accel));
  m_nofAccel = m_orgNofAccel;
  CStatic *kf = (CStatic *)GetDlgItem(IDC_KEY_FILE);
  if(kf)
  {
    char fn[_MAX_PATH], text[_MAX_PATH + 20];
    MySplitPath(m_currKeyFile, SP_FILE | SP_EXT, fn);
    sprintf(text, "Current File: %s", fn);
    kf->SetWindowText(text);
  }
  return FALSE;
}

void KeySetupDialogClass::KeyChanged(void)
{
  CListBox *lb = (CListBox *)GetDlgItem(IDC_KEY_KEY);
  ASSERT(lb);
  int sel = lb->GetCurSel();
  if(sel != LB_ERR)
  {
    CListBox *cl = (CListBox *)GetDlgItem(IDC_KEY_CURR);
    ASSERT(cl);
    cl->ResetContent();
    char text[256];
    lb->GetText(sel, text);
    WORD id = KeyName2KeyId(text);
    m_current = CURR_KEY;
    if(id != 0)
    {
      size_t i;
      for(i = 0; i < m_nofAccel; i++)
        if(m_accel[i].key == id && m_state == m_accel[i].fVirt)
        {
          const char *s = MsgId2FuncName(m_accel[i].cmd);
          if(s)
            cl->AddString(s);
        }
    }
  }
}

void KeySetupDialogClass::FuncChanged(void)
{
  CListBox *lb = (CListBox *)GetDlgItem(IDC_KEY_FUNC);
  if(lb)
  {
    int sel = lb->GetCurSel();
    if(sel != LB_ERR)
    {
      CListBox *cl = (CListBox *)GetDlgItem(IDC_KEY_CURR);
      ASSERT(cl);
      cl->ResetContent();
      char text[256];
      lb->GetText(sel, text);
      WORD id = FuncName2MsgId(text);
      if(id != 0)
      {
        size_t i;
        m_current = CURR_FUNC;
        for(i = 0; i < m_nofAccel; i++)
          if(m_accel[i].cmd == id)
            if(KeyState2String(m_accel[i].key, m_accel[i].fVirt, text))
              cl->AddString(text);
      }
    }
  }
}

void KeySetupDialogClass::CurrentChanged(void)
{
  if(m_current == CURR_FUNC)
  {
    CListBox *cl = (CListBox *)GetDlgItem(IDC_KEY_CURR);
    CListBox *kl = (CListBox *)GetDlgItem(IDC_KEY_KEY);
    ASSERT(cl);
    ASSERT(kl);
    int sel = cl->GetCurSel();
    if(sel != LB_ERR)
    {
      char text[256];
      WORD key;
      BYTE st;
      cl->GetText(sel, text);
      if(String2KeyState(&key, &st, text))
      {
        char *s = strtok(text, " ");
        if(s)
        {
          char keyS[64];
          strcpy(keyS, s);
          int k = kl->FindString(-1, keyS);
          if(k != LB_ERR)
            kl->SetCurSel(k);
          CButton *b;
          b = (CButton *)GetDlgItem(IDC_KEY_CTRL);
          ASSERT(b);
          b->SetCheck(st & FCONTROL ? 1 : 0);
          b = (CButton *)GetDlgItem(IDC_KEY_ALT);
          ASSERT(b);
          b->SetCheck(st & FALT ? 1 : 0);
          b = (CButton *)GetDlgItem(IDC_KEY_SHIFT);
          ASSERT(b);
          b->SetCheck(st & FSHIFT ? 1 : 0);
          m_state = st;
        }
      }
    }
  }
  else if(m_current == CURR_KEY)
  {
    CListBox *cl = (CListBox *)GetDlgItem(IDC_KEY_CURR);
    CListBox *fl = (CListBox *)GetDlgItem(IDC_KEY_FUNC);
    ASSERT(cl);
    ASSERT(fl);
    int sel = cl->GetCurSel();
    if(sel != LB_ERR)
    {
      char text[256];
      cl->GetText(sel, text);
      int f = fl->FindString(-1, text);
      if(f != LB_ERR)
        fl->SetCurSel(f);
    }
  }
}


void KeySetupDialogClass::AltHit(void)
{
  CButton *b = (CButton *)GetDlgItem(IDC_KEY_ALT);
  ASSERT(b);
  if(b->GetCheck())
  {
    m_state &= ~FALT;
    b->SetCheck(0);
  }
  else
  {
    m_state |= FALT;
    b->SetCheck(1);
  }
}

void KeySetupDialogClass::CtrlHit(void)
{
  CButton *b = (CButton *)GetDlgItem(IDC_KEY_CTRL);
  ASSERT(b);
  if(b->GetCheck())
  {
    m_state &= ~FCONTROL;
    b->SetCheck(0);
  }
  else
  {
    m_state |= FCONTROL;
    b->SetCheck(1);
  }
}

void KeySetupDialogClass::ShiftHit(void)
{
  CButton *b = (CButton *)GetDlgItem(IDC_KEY_SHIFT);
  ASSERT(b);
  if(b->GetCheck())
  {
    m_state &= ~FSHIFT;
    b->SetCheck(0);
  }
  else
  {
    m_state |= FSHIFT;
    b->SetCheck(1);
  }
}

void KeySetupDialogClass::HandleApply(void)
{
  CListBox *fl = (CListBox *)GetDlgItem(IDC_KEY_FUNC);
  ASSERT(fl);
  CListBox *kl = (CListBox *)GetDlgItem(IDC_KEY_KEY);
  ASSERT(kl);
  CListBox *cl = (CListBox *)GetDlgItem(IDC_KEY_CURR);
  ASSERT(cl);
  int cur_func_id = fl->GetCurSel();
  int cur_key_id = kl->GetCurSel();
  if(cur_func_id != LB_ERR && cur_key_id != LB_ERR)
  {
    char func_text[256];
    fl->GetText(cur_func_id, func_text);
    WORD msg_id = FuncName2MsgId(func_text);
    char key_text[256];
    kl->GetText(cur_key_id, key_text);
    WORD key_id = KeyName2KeyId(key_text);
    if(msg_id != 0 && key_id != 0)
    {
      // First check to see if the key is bound to any function, if so remove the binding
      size_t i;
      for(i = 0; i < m_nofAccel; i++)
      {
        if(m_accel[i].key == key_id && m_accel[i].fVirt == m_state)
        {
          memmove(&m_accel[i], &m_accel[i + 1], (m_nofAccel - i)*sizeof(ACCEL));
          m_nofAccel--;
        }
      }
      // Then add a new entry
      m_accel[m_nofAccel].cmd = msg_id;
      m_accel[m_nofAccel].key = key_id;
      m_accel[m_nofAccel].fVirt = m_state;
      m_nofAccel++;
      if(m_current == CURR_FUNC)
      {
        if(m_state & FALT)
          strcat(key_text, " ALT");
        if(m_state & FCONTROL)
          strcat(key_text, " CTRL");
        if(m_state & FSHIFT)
          strcat(key_text, " SHIFT");
        cl->AddString(key_text);
      }
      else if(m_current == CURR_KEY)
      {
        cl->AddString(func_text);
      }
    }
  }
}

void KeySetupDialogClass::HandleRemove(void)
{
  CListBox *cl = (CListBox *)GetDlgItem(IDC_KEY_CURR);
  ASSERT(cl);
  int k = cl->GetCurSel();
  size_t i;
  if(k != LB_ERR)
  {
    char text[256];
    cl->GetText(k, text);
    if(m_current == CURR_KEY)
    {
      WORD msg_id = FuncName2MsgId(text);
      if(msg_id != 0)
      {
        for(i = 0; i < m_nofAccel; i++)
        {
          if(m_accel[i].cmd == msg_id)
          {
            memmove(&m_accel[i], &m_accel[i + 1], (m_nofAccel - i)*sizeof(ACCEL));
            m_nofAccel--;
          }
        }
      }
      cl->DeleteString(k);
    }
    else if(m_current == CURR_FUNC)
    {
      BYTE st;
      WORD key;
      if(String2KeyState(&key, &st, text))
      {
        for(i = 0; i < m_nofAccel; i++)
        {
          if(m_accel[i].key == key && m_accel[i].fVirt == st)
          {
            memmove(&m_accel[i], &m_accel[i + 1], (m_nofAccel - i)*sizeof(ACCEL));
            m_nofAccel--;
          }
        }
        cl->DeleteString(k);
      }
    }
  }
  else
    WainMessageBox(GetMf(), "You have to select the item to be removed\r\nin the \"Current Assignment\" list", IDC_MSG_OK, IDI_INFORMATION_ICO);
}

void KeySetupDialogClass::BrowseForFile(void)
{
  MyFileDialogClass fd(FD_OPEN | FD_CONFIG_PATH, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY, "key", m_currKeyFile, "KeyFiles (*.key)\0*.key\0All files (*.*)\0*.*\0\0", this);
  if(fd.DoModal() == IDOK)
  {
    m_currKeyFile = fd.GetPathName();
    CStatic *kf = (CStatic *)GetDlgItem(IDC_KEY_FILE);
    if(kf)
    {
      CString fn;
      MySplitPath(m_currKeyFile, SP_FILE | SP_EXT, fn);
      kf->SetWindowText(fn);
    }
    m_nofAccel = 0;
    ReadKeySetupFile(m_currKeyFile, (ACCEL *)&m_accel, &m_nofAccel);
  }
}

BEGIN_MESSAGE_MAP(KeySetupDialogClass, CDialog)
  ON_BN_CLICKED(IDC_KEY_ALT,     AltHit)
  ON_BN_CLICKED(IDC_KEY_CTRL,    CtrlHit)
  ON_BN_CLICKED(IDC_KEY_SHIFT,   ShiftHit)
  ON_BN_CLICKED(IDC_KEY_APPLY,   HandleApply)
  ON_BN_CLICKED(IDC_KEY_REMOVE,  HandleRemove)
  ON_BN_CLICKED(IDC_KEY_BROWSE,  BrowseForFile)
  ON_LBN_SELCHANGE(IDC_KEY_KEY,  KeyChanged)
  ON_LBN_SELCHANGE(IDC_KEY_FUNC, FuncChanged)
  ON_LBN_SELCHANGE(IDC_KEY_CURR, CurrentChanged)
END_MESSAGE_MAP();

static BOOL KeyState2String(WORD key, BYTE st, char *text)
{
  const char *s = KeyId2KeyName(key);
  if(s)
  {
    strcpy(text, s);
    if(st & FALT)
      strcat(text, " ALT");
    if(st & FCONTROL)
      strcat(text, " CTRL");
    if(st & FSHIFT)
      strcat(text, " SHIFT");
    return TRUE;
  }
  return FALSE;
}

void KeyState2MenuString(char *text, WORD key, BYTE flags)
{
  if(flags & FALT)
    strcat(text, "Alt+");
  if(flags & FCONTROL)
    strcat(text, "Ctrl+");
  if(flags & FSHIFT)
    strcat(text, "Shift+");
  const char *s = KeyId2KeyName(key);
  char temp[64];
  strcpy(temp, s);
  char *p;
  for(p = &temp[1]; *p; p++)
    *p = (char )tolower(*((unsigned char *)p));
  strcat(text, temp);
}

void KeyState2MenuString(std::string &aText, WORD key, BYTE flags)
{
  if(flags & FALT)
    aText += "Alt+";
  if(flags & FCONTROL)
    aText += "Ctrl+";
  if(flags & FSHIFT)
    aText += "Shift+";
  std::string temp = KeyId2KeyName(key);
  std::string::size_type idx;
  for(idx = 1; idx < temp.size(); idx++)
     temp[idx] = (char )tolower(*(unsigned char *)&(temp[idx]));
  aText += temp;
}

static BOOL String2KeyState(WORD *key, BYTE *st, const char *text)
{
  char temp[256];
  strcpy(temp, text);
  char *s = strtok(temp, " ");
  if(s)
  {
    *key = KeyName2KeyId(s);
    *st = FVIRTKEY;

    while((s = strtok(NULL, " ")) != NULL)
    {
      if(!strcmp(s, "ALT"))
        *st |= FALT;
      else if(!strcmp(s, "CTRL"))
        *st |= FCONTROL;
      else if(!strcmp(s, "SHIFT"))
        *st |= FSHIFT;
      else
        return FALSE;
    }
    return TRUE;
  }
  return FALSE;
}

void MakePopupMenuStrings(std::vector<PopupMenuListClass >&aMenu)
{
  size_t i;
  for(i = 0; i < aMenu.size(); i++)
  {
    const char *text = MsgId2FuncName(aMenu[i].m_id);
    aMenu[i].m_text = text ? text : "----";
  }
  MakePopupMenuAmpersand(aMenu);
  AddPopupMenuAccelerator(aMenu);
}

void RemovePopupMenuAccelerator(std::vector<PopupMenuListClass >&aMenu)
{
   size_t i;
   for(i = 0; i < aMenu.size(); i++)
   {
      std::string::size_type pos = aMenu[i].m_text.find('\t');
      if(pos != std::string::npos)
         aMenu[i].m_text = aMenu[i].m_text.substr(0, pos);
   }
}

void AddPopupMenuAccelerator(std::vector<PopupMenuListClass >&aMenu)
{
  size_t i, j;
  MainFrame *mf = GetMf();
  if(!mf)
    return;
  for(i = 0; i < aMenu.size(); i++)
  {
    BOOL found = FALSE;
    for(j = 0; j < mf->m_nofAccEntrys && !found; j++)
    {
      if(mf->m_accEntry[j].cmd == aMenu[i].m_id)
      {
        found = TRUE;
        aMenu[i].m_text += "\t";
        KeyState2MenuString(aMenu[i].m_text, mf->m_accEntry[j].key, mf->m_accEntry[j].fVirt);
      }
    }
  }
}

void MakePopupMenuAmpersand(std::vector<PopupMenuListClass >&aMenu)
{
   size_t i, j, pos;
   bool in_use['Z' - 'A' + 2];
   #define IN_USE_SIZE (sizeof(in_use)/sizeof(in_use[0]))
   for(i = 0; i < IN_USE_SIZE; i++)
      in_use[i] = false;

   size_t found = 0;
   for(i = 0; i < aMenu.size(); i++)
   {
      pos = aMenu[i].m_text.find('&');
      if(pos != std::string::npos && pos < aMenu[i].m_text.size() - 1)
      {
         j = tolower(*(unsigned char *)&(aMenu[i].m_text[1])) - 'a';
         if(j < IN_USE_SIZE)
         {
            in_use[j] = true;
            found++;
         }
      }
   }

  for(pos = 0; pos < size_t(128) && found < aMenu.size(); pos++)
  {
    for(i = 0; i < aMenu.size(); i++)
    {
      std::string::size_type idx = aMenu[i].m_text.find('&');
      if(idx == std::string::npos)
      {
        if(aMenu[i].m_text.size() > pos)
        {
          j = tolower(*(unsigned char *)&(aMenu[i].m_text[pos])) - 'a';
          if(j < IN_USE_SIZE && !in_use[j])
          {
            in_use[j] = TRUE;
            aMenu[i].m_text.insert(pos, "&");
            found++;
          }
        }
      }
    }
  }
  #undef IN_USE_SIZE
}

BEGIN_MESSAGE_MAP(MenuSetupDialogClass, CDialog)
  ON_BN_CLICKED(IDC_PMS_ADD,        Add)
  ON_BN_CLICKED(IDC_PMS_REMOVE,     Remove)
  ON_BN_CLICKED(IDC_PMS_SEPERATOR,  Seperator)
  ON_BN_CLICKED(IDC_PMS_UP,         Up)
  ON_BN_CLICKED(IDC_PMS_DOWN,       Down)
  ON_BN_CLICKED(IDCANCEL,           OnCancel)
  ON_BN_CLICKED(IDOK,               OnOK)

END_MESSAGE_MAP();

MenuSetupDialogClass::MenuSetupDialogClass(CWnd *parent) : CDialog(MenuSetupDialogClass::IDD, parent)
{
}

PopupMenuSetupDialogClass::PopupMenuSetupDialogClass(CWnd *parent) : MenuSetupDialogClass(parent)
{
   std::copy(wainApp.gs.m_popupMenu.begin(), wainApp.gs.m_popupMenu.end(), std::back_inserter(m_popupMenu));

   RemovePopupMenuAccelerator(m_popupMenu);
}

UserMenuSetupDialogClass::UserMenuSetupDialogClass(CWnd *parent) : MenuSetupDialogClass(parent)
{
   std::copy(wainApp.gs.m_userMenu.begin(), wainApp.gs.m_userMenu.end(), std::back_inserter(m_popupMenu));
   RemovePopupMenuAccelerator(m_popupMenu);
}

void MenuSetupDialogClass::DoDataExchange(CDataExchange *dx)
{
   CDialog::DoDataExchange(dx);
}

BOOL MenuSetupDialogClass::OnInitDialog(void)
{
  size_t i;
  CListBox *lb = (CListBox *)GetDlgItem(IDC_PMS_FUNC_LIST);
  ASSERT(lb);
  for(i = 0; i < NOF_KEY_TEXTS; i++)
    lb->AddString(KeyTextMap[i].m_text);
  for(i = 0; i < wainApp.gs.m_toolParm.Size(); i++)
    lb->AddString(wainApp.gs.m_toolParm[i].m_menuText.c_str());
  for(i = 0; i < wainApp.gs.m_macroParmList.size(); i++)
    lb->AddString(wainApp.gs.m_macroParmList[i].m_menuText.c_str());

  lb = (CListBox *)GetDlgItem(IDC_PMS_MENU_LIST);
  ASSERT(lb);
  for(i = 0; i < m_popupMenu.size(); i++)
  {
    lb->AddString(m_popupMenu[i].m_text.c_str());
  }
  return CDialog::OnInitDialog();
}

void PopupMenuSetupDialogClass::OnOK(void)
{
  MakePopupMenuAmpersand(m_popupMenu);
  AddPopupMenuAccelerator(m_popupMenu);
  wainApp.gs.m_popupMenu.clear();
  std::copy(m_popupMenu.begin(), m_popupMenu.end(), std::back_inserter(wainApp.gs.m_popupMenu));
  EndDialog(IDOK);
}

void UserMenuSetupDialogClass::OnOK(void)
{
   wainApp.gs.m_userMenu.clear();
   std::copy(m_popupMenu.begin(), m_popupMenu.end(), std::back_inserter(wainApp.gs.m_userMenu));
   EndDialog(IDOK);
}

void MenuSetupDialogClass::OnCancel(void)
{
  EndDialog(IDCANCEL);
}

void MenuSetupDialogClass::Add(void)
{
  if(m_popupMenu.size() < MAX_NOF_POPUP_MENU_ITEMS - 1)
  {
    CListBox *lb = (CListBox *)GetDlgItem(IDC_PMS_FUNC_LIST);
    ASSERT(lb);
    int sel = lb->GetCurSel();
    if(sel != LB_ERR)
    {
      char text[128];
      lb->GetText(sel, text);
      WORD id = FuncName2MsgId(text);

      m_popupMenu.push_back(PopupMenuListClass(text, id));
      lb = (CListBox *)GetDlgItem(IDC_PMS_MENU_LIST);
      ASSERT(lb);
      lb->AddString(text);
      lb->SetCurSel(m_popupMenu.size() - 1);
    }
    else
      SetStatusText("You must first select an item in the function list");
  }
  else
    SetStatusText("You have hit the limit on numbers of items in the popupmenu: %d", MAX_NOF_POPUP_MENU_ITEMS);
}

void MenuSetupDialogClass::Remove(void)
{
   CListBox *lb = (CListBox *)GetDlgItem(IDC_PMS_MENU_LIST);
   ASSERT(lb);
   int sel = lb->GetCurSel();
   if(sel != LB_ERR)
   {
      lb->DeleteString(sel);
      m_popupMenu.erase(m_popupMenu.begin() + sel);
   }
   else
   {
      SetStatusText("You must first select an item in the menu list");
   }
}

void MenuSetupDialogClass::Seperator(void)
{
  if(m_popupMenu.size() < MAX_NOF_POPUP_MENU_ITEMS - 1)
  {
    m_popupMenu.push_back(PopupMenuListClass("------", 0));
    CListBox *lb = (CListBox *)GetDlgItem(IDC_PMS_MENU_LIST);
    ASSERT(lb);
    lb->AddString("------");
    lb->SetCurSel(m_popupMenu.size() - 1);
  }
  else
    SetStatusText("You have hit the limit on numbers of items in the popupmenu: %d", MAX_NOF_POPUP_MENU_ITEMS);
}

void MenuSetupDialogClass::Up(void)
{
  CListBox *lb = (CListBox *)GetDlgItem(IDC_PMS_MENU_LIST);
  ASSERT(lb);
  int sel = lb->GetCurSel();
  if(sel != LB_ERR && sel > 0)
  {
    char text[128];
    lb->GetText(sel, text);
    lb->DeleteString(sel);
    lb->InsertString(sel - 1, text);
    lb->SetCurSel(sel - 1);
    std::swap(m_popupMenu[sel], m_popupMenu[sel - 1]);
  }
  else
    SetStatusText("You must first select an item in the menu list");
}

void MenuSetupDialogClass::Down(void)
{
  CListBox *lb = (CListBox *)GetDlgItem(IDC_PMS_MENU_LIST);
  ASSERT(lb);
  size_t sel = lb->GetCurSel();
  if(sel != LB_ERR && sel < m_popupMenu.size() - 1)
  {
    char text[128];
    lb->GetText(sel, text);
    lb->DeleteString(sel);
    lb->InsertString(sel + 1, text);
    lb->SetCurSel(sel + 1);
    std::swap(m_popupMenu[sel], m_popupMenu[sel + 1]);
  }
  else
    SetStatusText("You must first select an item in the menu list");
}

void MainFrame::PopupMenuSetup(void)
{
  PopupMenuSetupDialogClass setup(this);
  setup.DoModal();
}

void MainFrame::UserMenuSetup(void)
{
   UserMenuSetupDialogClass setup(this);
   if(setup.DoModal() == IDOK)
      SetupMenu(TRUE);
}

void MainFrame::KeyboardSetup(void)
{
  KeySetupDialogClass ks;
  ks.m_orgAccel = (ACCEL *)&m_accEntry;
  ks.m_orgNofAccel = m_nofAccEntrys;
  ks.m_currKeyFile = m_keyboardSetupFile;
  if(ks.DoModal() == IDOK)
  {
    if(m_accHandle)
      DestroyAcceleratorTable(m_accHandle);
    memcpy(&m_accEntry, &ks.m_accel, sizeof(m_accEntry));
    m_nofAccEntrys = ks.m_nofAccel;
    if(m_keyboardSetupFile != ks.m_currKeyFile)
    {
      m_keyboardSetupFile = ks.m_currKeyFile;
      wainApp.WriteProfileString("Settings", "KeyFile", m_keyboardSetupFile);
    }
    WriteKeySetupFile(m_keyboardSetupFile, (ACCEL *)&m_accEntry, m_nofAccEntrys);
    m_accHandle = CreateAcceleratorTable((ACCEL *)&m_accEntry, m_nofAccEntrys);

    // Modify the menu
    SetupMenu(TRUE);
  }
}
