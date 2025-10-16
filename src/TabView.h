#ifndef TABVIEW_H_INC
#define TABVIEW_H_INC

class TabViewClass : public CView
{
   class WainView *m_wainView;
public:
   TabViewClass(class WainView *view);
   virtual void OnDraw(CDC *dc);

   int m_bookmark[NOF_BOOKMARKS];
   void SetBookmark(int nr, int LineNo);
protected:
   afx_msg void OnPaint(void);

   DECLARE_MESSAGE_MAP();
};

#endif



