#pragma once

struct MacroEntryInfoType
{
   int m_code;
   char *m_functionName;
   int m_info;
};

class MacroListEntryClass
{
   friend class MacroListClass;
   friend class MainFrame;
   MacroEntryInfoType m_info;
   MacroListEntryClass *m_prev;
   MacroListEntryClass *m_next;
   MacroListEntryClass();
   ~MacroListEntryClass();
};

class MacroListClass
{
public:
   MacroListEntryClass m_list;
   MacroListEntryClass *m_current;
   bool m_recording;
   MacroListClass();
   ~MacroListClass();
   void CleanUp(void);
};
