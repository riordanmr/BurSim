#if !defined(AFX_DLGCONFIG_H__67618952_C9D8_11D2_A092_0000F87ADAFA__INCLUDED_)
#define AFX_DLGCONFIG_H__67618952_C9D8_11D2_A092_0000F87ADAFA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DlgConfig.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgConfig dialog

class CDlgConfig : public CDialog
{
// Construction
public:
	CDlgConfig(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgConfig)
	enum { IDD = IDD_DIALOG_CONFIG };
	int		m_FontsizeSaver1;
	int		m_FontsizeWindowed1;
	int		m_Debug;
	int		m_nSecsDelayBetweenRefreshes;
	BOOL	m_bRunDebugger;
	int		m_TCPListenPort;
	BOOL	m_bShowWelcome;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgConfig)
	afx_msg void OnButtonHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCONFIG_H__67618952_C9D8_11D2_A092_0000F87ADAFA__INCLUDED_)
