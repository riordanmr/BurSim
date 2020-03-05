// DlgConfig.cpp : implementation file
//

#include "stdafx.h"
#include "BurSimApp.h"
#include "DlgConfig.h"
#include "Util.h"
#include "Help/BurSim.hh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=== Global Variables ============================================
extern CBurSimApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDlgConfig dialog


CDlgConfig::CDlgConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgConfig)
	m_FontsizeSaver1 = 0;
	m_FontsizeWindowed1 = 0;
	m_Debug = 0;
	m_nSecsDelayBetweenRefreshes = 0;
	m_bRunDebugger = FALSE;
	m_TCPListenPort = 0;
	m_bShowWelcome = FALSE;
	//}}AFX_DATA_INIT
}


void CDlgConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgConfig)
	DDX_Text(pDX, IDC_EDIT_FONT_SAVER_1, m_FontsizeSaver1);
	DDX_Text(pDX, IDC_EDIT_FONT_WINDOWED_1, m_FontsizeWindowed1);
	DDX_Text(pDX, IDC_EDIT_DEBUG, m_Debug);
	DDX_Text(pDX, IDC_EDIT_SECS_REFRESH, m_nSecsDelayBetweenRefreshes);
	DDX_Check(pDX, IDC_CHECK_RUN_DEBUGGER, m_bRunDebugger);
	DDX_Text(pDX, IDC_EDIT_TCP_LISTEN_PORT, m_TCPListenPort);
	DDX_Check(pDX, IDC_CHECK_SHOW_WELCOME, m_bShowWelcome);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgConfig, CDialog)
	//{{AFX_MSG_MAP(CDlgConfig)
	ON_BN_CLICKED(IDC_BUTTON_HELP, OnButtonHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgConfig message handlers

void CDlgConfig::OnButtonHelp() 
{
   // Using the next line causes Help to be brought up 
   // behind other windows.  Weird.
   // Wait--there's something very fishy going on here.
   // It happens with FTPSNI.HLP, too--sometimes, on MRRDEC.
   // Maybe has something to do with RoboHELP running???
   //MessageBox("OnButtonHelp here");
   AfxGetApp()->WinHelp(Configuring_BurSim);
   //AfxGetApp()->WinHelp(0,HELP_CONTENTS);
   // Same is true if we call RunHelp with theApp.m_pMainWnd->m_hWnd
   // RunHelp(theApp.m_pMainWnd->m_hWnd,HELP_CONTEXT,Configuring_BurSim);
}
