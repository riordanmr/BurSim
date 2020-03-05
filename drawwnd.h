#ifndef DRAWWND_INCLUDED
#define DRAWWND_INCLUDED
// drawwnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDrawWnd window

enum enum_wnd_type {WND_TYPE_NONE, WND_TYPE_SAVER, WND_TYPE_NORMAL};

// Physical characteristics of the display
#define MAXLINELEN         80
#define SCREENROWS         24
// Number of screen rows left over after system info at top.
#define ROWS_DISPLAYED     19
// The top line is reserved for keyboard entry.
#define KEYBOARD_ROW                0
// Screen row of first line to refresh in normal way.  (0-based).
#define FIRST_ROW_REGULAR_REFRESH   1
// Screen row on which error messages from the keyboard are
// displayed.
#define KEYBOARD_ERROR_MSG_ROW      1
// Screen row which contains hostname, memory & CPU avail, date, time.
#define PROCESSOR_STATUS_ROW        2
// First row of regular data (not dedicated to keyboard or errmsg).
#define FIRST_ROW_DATA              2
// In SPO display, number of SPO messages that can be shown.
// ("-----[  System Messages  ]-----" takes up one line)
#define SPO_MSGS_DISPLAYED 18

class CDrawWnd : public CWnd
{
// Construction
public:
	CDrawWnd(BOOL bAutoDelete = TRUE);

// Attributes
public:
	CPoint m_ptLast;

	CRgn m_rgnLast;
	int m_nWidth;
	int m_nHeight;
	static LPCTSTR m_lpszClassName;
   int m_xmargin;
   int m_ymargin;
   int m_height;  // Height of font
   int m_y;       // Place to put next line
   CDC *m_pdc;

   enum enum_display_type m_display_type;

   int  m_nLineThisDraw;
   char m_Screen[SCREENROWS][MAXLINELEN+4];  // This is the characters to actually display 
                                             // in the window.
   char m_szsystem_name[40];
   int  m_memory;
   double m_pct_cpu_avail;
   int  m_cursor_col;   // zero-based
   int  m_spo_lines_back;     // how many lines back to start displaying
                              // the SPO.  Default should be SPO_MSGS_DISPLAYED;
                              // other values are used for DQnn

// Operations
public:
	void SetSpeed(int nSpeed);
   void PutLine(int iline,const char *fmt,...);
	void Draw(CDC& dc, int nWidth);
   void DrawPoolDisplay(CDC& dc);
   BOOL MyCreateFont(CDC& dc);
   void SetScreenRow(int irow, CString strLine);
   void SetConsoleErrorLine(CString strMsg);
   void CopyScreen(char *ary[]);
   void UpdateScreen();
   void SetFixedScreenAreas();
   void CalculateFontInfo(CDC& dc);
   void DrawKeyboardArea(CDC &dc);
   void DrawProcessorStatusLine(CDC &dc);
   void UpdateKeyboardArea();
   void ExecuteCommandFromConsole();
   void SetRefreshTimer();
   void StartScreenPaint();
   void DelayRefresh();

// Overrides
	public:
	virtual BOOL Create(DWORD dwExStyle, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual void PostNcDestroy();
	protected:
	// ClassWizard generated virtual function overrides
	//   Used to be: //{{AFX_VIRTUAL(CDrawWnd)
	//{{AFX_MSG(CDrawWnd)
	afx_msg void OnDestroy();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
   afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	//  Used to be:  //}}AFX_VIRTUAL
	DECLARE_MESSAGE_MAP()

// Implementation
public:
	virtual ~CDrawWnd();
   BOOL m_bUpdateFont;

protected:
	BOOL m_bAutoDelete;
   CFont m_fontConsole;
   CFont m_fontConsoleUnderlined;

};

/////////////////////////////////////////////////////////////////////////////
#endif