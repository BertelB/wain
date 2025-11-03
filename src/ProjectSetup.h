#ifndef PROJECTSETUP_H_INC
#define PROJECTSETUP_H_INC

#include "../src/ProjectOptions.h"
// The list that show prog, arg, path on Project->Setup
class ProjectListCtrl : public CListCtrl
{
   DECLARE_DYNAMIC(ProjectListCtrl);
public:
   ProjectListCtrl(const ProjectExecParam* _param, uint32_t _default, const char* _title) : CListCtrl(), m_title(_title)
   {
      for (uint32_t i = 0; i < 4; i++)
      {
         m_param[i] = _param[i];
      }
      m_default = _default;
   }
   ProjectExecParam m_param[4];
   uint32_t m_default = 0;
   void UpdateData();
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnLButtonDblClk(UINT flags, CPoint point);
   // afx_msg void OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult);
   std::string m_title;
   DECLARE_MESSAGE_MAP();
};

// The dialog that pop up when the user clicks on one of the lists in Project -> Setup
class ProjectMakeExecParamDialog : public DialogBaseClass
{
   DECLARE_DYNAMIC(ProjectMakeExecParamDialog);
public:
   ProjectMakeExecParamDialog(CWnd *parent, ProjectExecParam& _param);
   ~ProjectMakeExecParamDialog(void);
   enum {IDD = IDC_PROJECT_EDIT_DIALOG};
   const ProjectExecParam& GetParam() const { return m_param; }
protected:
   virtual BOOL OnInitDialog(void);
   afx_msg void OnOk(void);

   ProjectExecParam m_param;
   DECLARE_MESSAGE_MAP();
   virtual void DoDataExchange(CDataExchange *dx);
   afx_msg void ProgBrowse(void);
   afx_msg void PathBrowse(void);
};

// The Project -> Setup dialog
class ProjectSetupDialogClass : public DialogBaseClass
{
  DECLARE_DYNAMIC(ProjectSetupDialogClass);
public:
   ProjectOptions m_projectOptions;
   ProjectListCtrl* m_makeList = 0;
   ProjectListCtrl* m_execList = 0;
   ProjectSetupDialogClass(CWnd *aParent = 0);
   virtual ~ProjectSetupDialogClass(void);
   enum {IDD = IDD_PROJ_SETUP_DIALOG};
   bool m_inCheck = false;
protected:
  virtual BOOL OnInitDialog();
  void DoDataExchange(CDataExchange *dx);
  afx_msg void ProjBrowse(void);
  afx_msg void IncPathBrowse(void);
  afx_msg void TagSetup(void);
  afx_msg void OnOk(void);
  afx_msg void OnItemChangingExec(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnItemChangingMake(NMHDR* pNMHDR, LRESULT* pResult);
  DECLARE_MESSAGE_MAP();
private:
};

#endif