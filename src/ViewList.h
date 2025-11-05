#pragma once

class ViewListElem
{
public:
  ViewListElem* m_rankNext;
  ViewListElem* m_rankPrev;
  ViewListElem();
  ~ViewListElem();
  std::string m_name;
  std::string m_shortName;
  class WainView *m_myView;
  int m_nr;
  int GetPosition() const { return m_position; }
  void SetPosition(int _position) { m_position = _position; }
  int m_position;
  int m_iconIndex;
  void SetName(const char* _fileName, bool _isFtpFile);
  void UpdateStatus(void);
  bool m_ftpFile;
  bool m_ignore;
  bool m_skip;
  bool m_reload;
  bool m_readOnly;
  time_t m_modifiedTime;
  long m_size;
};

typedef std::vector<const ViewListElem*> BrowseListType;

class ViewList
{
public:
  ViewList();
  ~ViewList();
  ViewListElem m_list;
  ViewListElem *m_currentView;
  ViewListElem *m_topView[2];
  int AddView(WainView *view, const char *file_name, bool IsFtpFile);
  BOOL RemoveView(int nr);
  void SetViewName(int nr, const char *name, bool IsFtpFile);
  ViewListElem *GetRankNext(ViewListElem *);
  ViewListElem *GetRankPrev(ViewListElem *);
  const ViewListElem *GetAbsNr(int nr);
  ViewListElem *PutInRankTop(int nr);
  int FindUniqueNr(void);
  ViewListElem *GetViewNr(int nr);

  // The List on the Browse bar, sorted by name.
  BrowseListType m_browseList;
};
