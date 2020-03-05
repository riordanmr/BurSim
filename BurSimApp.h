// BurSimApp.h : main header file for the BurSim application
//
#ifndef BURSIMAPP_INCLUDED
#define BURSIMAPP_INCLUDED
#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

enum enum_display_type {DISP_MIX, DISP_DISKS, DISP_MSGS, DISP_COMMAND, DISP_NONE};
const enum_display_type DISP_LAST_REFRESH = DISP_MSGS;

#define TIMER_START_REFRESH   2
#define TIMER_DELAY_REFRESH   3
#define TIMER_DRAW_ROW        4

/////////////////////////////////////////////////////////////////////////////
// CBurSim:
// See BurSim.cpp for the implementation of this class
//

class CBurSimApp : public CWinApp
{
public:
	CBurSimApp();
   void LoadSettings();
   void SaveSettings();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBurSim)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	void DoConfig();

	//{{AFX_MSG(CBurSim)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
#endif