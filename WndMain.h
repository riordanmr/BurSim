#if !defined(AFX_WNDMAIN_H__96E94292_CE5F_11D2_A09A_0000F87ADAFA__INCLUDED_)
#define AFX_WNDMAIN_H__96E94292_CE5F_11D2_A09A_0000F87ADAFA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// WndMain.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWndMain window

class CWndMain : public CWnd
{
// Construction
public:
	CWndMain();

// Attributes
public:
	static LPCTSTR m_lpszClassName;

// Operations
public:
	BOOL Create(enum enum_wnd_type wnd_type);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWndMain)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWndMain();

	// Generated message map functions
protected:
	//{{AFX_MSG(CWndMain)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnQueryEndSession();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WNDMAIN_H__96E94292_CE5F_11D2_A09A_0000F87ADAFA__INCLUDED_)
