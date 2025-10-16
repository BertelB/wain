#ifndef DIALOGBASE_H_INC
#define DIALOGBASE_H_INC

template <typename T>
std::string ToString(T t)
{
   std::stringstream ss;
   ss << t;
   return ss.str();
}

template <typename T>
T FromString(const std::string &aStr)
{
   std::stringstream ss(aStr);
   T t;
   ss >> t;
   return t;
}

class DialogBaseClass : public CDialog
{
public:
   DialogBaseClass(UINT aTemplateId, CWnd *aParent = 0) : CDialog(aTemplateId, aParent)
   {}
protected:

   virtual void DdxString(CDataExchange* pDX, int nIDC, std::string &value)
   {
      HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
      if (pDX->m_bSaveAndValidate)
      {
         int Len = ::GetWindowTextLength(hWndCtrl);
         char *Temp = new char [Len + 1];
         ::GetWindowText(hWndCtrl, Temp, Len + 1);
         value = Temp;
         delete [] Temp;
      }
      else
      {
         AfxSetWindowText(hWndCtrl, value.c_str());
      }
   }
   virtual void DdxCheck(CDataExchange *pDX, int nIDC, bool &value)
   {
      HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
      if (pDX->m_bSaveAndValidate)
      {
         int v = (int )::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L);
         ASSERT(v >= 0 && v <= 2);
         value = v ? true : false;
      }
      else
      {
         int v = value;
         if(v < 0 || v > 2)
         {
            v = 0;  // default to off
         }
         ::SendMessage(hWndCtrl, BM_SETCHECK, (WPARAM )v, 0L);
      }
   }

   virtual void DdxRadio(CDataExchange *pDX, int nIdcStart, int nIdcEnd, uint32_t &value)
   {
      if (pDX->m_bSaveAndValidate)
      {
         value = 0;
         int nIdc;
         for (nIdc = nIdcStart; nIdc <= nIdcEnd; nIdc++)
         {
            HWND hWndCtrl = pDX->PrepareCtrl(nIdc);
            int v = (int )::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L);
            ASSERT(v >= 0 && v <= 2);
            if (v)
            {
               value = nIdc - nIdcStart;
            }
         }
      }
      else
      {
         HWND hWndCtrl = pDX->PrepareCtrl(nIdcStart + value);
         ::SendMessage(hWndCtrl, BM_SETCHECK, (WPARAM )1, 0L);
      }
   }


   virtual void DdxInt(CDataExchange *aDx, int aId, int &aValue, int aMin = INT_MIN, int aMax = INT_MAX)
   {
      /* HWND hWndCtrl = */ aDx->PrepareCtrl(aId);
      if(aDx->m_bSaveAndValidate)
      {
         std::string S = GetDlgItemString(aId);
         aValue = FromString<int>(S);
         if(aValue < aMin)
            aValue = aMin;
         else if(aValue > aMax)
            aValue = aMax;
      }
      else
      {
         if(aValue < aMin)
            aValue = aMin;
         else if(aValue > aMax)
            aValue = aMax;
         std::string S = ToString(aValue);
         SetDlgItemString(aId, S);
      }
   }
   void SetDlgItemString(int aId, const std::string &aStr)
   {
      SetDlgItemText(aId, aStr.c_str());
   }
   std::string GetDlgItemString(int aItemId)
   {
      HWND HwndCtrl = ::GetDlgItem(m_hWnd, aItemId);
      if(!HwndCtrl)
         return "";
      int Len = ::GetWindowTextLength(HwndCtrl) + 1;
      char *s = new char [Len];
      ::GetWindowText(HwndCtrl, s, Len);
      std::string Text(s);
      delete [] s;
      return Text;
   }

   void GetListboxString(CComboBox* aComboBox, int& aIdx, std::string& aStr)
   {
      int Len = aComboBox->GetLBTextLen(aIdx);
      char *s = new char [Len + 1];
      aComboBox->GetLBText(aIdx, s);
      aStr = s;
      delete [] s;
   }
};

class PropertyPageBaseClass : public CPropertyPage
{
public:
   PropertyPageBaseClass(UINT aTemplateId) : CPropertyPage(aTemplateId)
   {}
protected:

   virtual void DdxString(CDataExchange* pDX, int nIDC, std::string &value)
   {
      HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
      if (pDX->m_bSaveAndValidate)
      {
         int Len = ::GetWindowTextLength(hWndCtrl);
         char *Temp = new char [Len + 1];
         ::GetWindowText(hWndCtrl, Temp, Len + 1);
         value = Temp;
         delete [] Temp;
      }
      else
      {
         AfxSetWindowText(hWndCtrl, value.c_str());
      }
   }
   virtual void DdxCheck(CDataExchange *pDX, int nIDC, bool &value)
   {
      HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
      if (pDX->m_bSaveAndValidate)
      {
         int v = (int )::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L);
         ASSERT(v >= 0 && v <= 2);
         value = v ? true : false;
      }
      else
      {
         int v = value;
         if(v < 0 || v > 2)
         {
            v = 0;  // default to off
         }
         ::SendMessage(hWndCtrl, BM_SETCHECK, (WPARAM )v, 0L);
      }
   }
   void SetDlgItemString(int aId, const std::string &aStr)
   {
      SetDlgItemText(aId, aStr.c_str());
   }
};

inline bool GetWindowString(CWnd *aWnd, std::string &aStr)
{
   ASSERT(aWnd);
   size_t Size = aWnd->GetWindowTextLength();
   aStr.resize(Size);
   aWnd->GetWindowText((char *)aStr.c_str(), Size + 1);
   return true;
}

inline std::string StrTok(std::string &aSrc, const char *aDelims)
{
   std::string::size_type Pos = 0;

   for(Pos = 0; Pos < aSrc.size() && strchr(aDelims, aSrc[Pos]); Pos++)
      ;
   if(Pos == aSrc.size())
     return "";
   std::string::size_type Start = Pos;
   for(; Pos < aSrc.size() && !strchr(aDelims, aSrc[Pos]); Pos++)
      ;

   std::string Ret = aSrc.substr(Start, Pos - Start);
   while(strchr(aDelims, aSrc[Pos]) && Pos < aSrc.size())
      Pos++;
   aSrc = aSrc.substr(Pos);

   return Ret;
}

class DirListCompareClass
{
public:
   DirListCompareClass() {}
   bool operator () (const std::string &lhs, const std::string &rhs)
   {
      if(lhs[0] == '[' && rhs[0] != '[')
         return true;
      if(lhs[0] != '[' && rhs[0] == '[')
         return false;
      return stricmp(lhs.c_str(), rhs.c_str()) < 0;
   }
};

class StrSplitterClass
{
public:
   StrSplitterClass(const std::string &aSrc) :
      m_src(aSrc),
      m_startPos(0)
   {}
   bool Get(std::string &aDest, const char *aDelim)
   {
      if(m_startPos == std::string::npos)
         return false;
      m_startPos = m_src.find_first_not_of(aDelim, m_startPos);
      if(m_startPos != std::string::npos)
      {
         std::string::size_type EndPos;
         EndPos = m_src.find_first_of(aDelim, m_startPos);
         aDest = m_src.substr(m_startPos, EndPos - m_startPos);
         m_startPos = EndPos;
         return true;
      }
      return false;
   }

   void Set(const std::string aSrc)
   {
      m_src = aSrc;
      m_startPos = 0;
   }
private:
   StrSplitterClass(); // Not used
   std::string m_src;
   std::string::size_type m_startPos;
};

inline int StrICmp(const std::string &lhs, const std::string &rhs)
{
   return stricmp(lhs.c_str(), rhs.c_str());
}

inline int StrCmp(const std::string &lhs, const std::string &rhs, bool aIgnoreCase)
{
   return aIgnoreCase ? stricmp(lhs.c_str(), rhs.c_str()) : strcmp(lhs.c_str(), rhs.c_str());
}

#endif