// WndMain.cpp : implementation file
//

#include "stdafx.h"
#include "WndMain.h"
#include "BurSimApp.h"
#include "drawwnd.h"
#include "BurSimScriptObject.h"
#include "AXScript.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=== Global Variables ===================================
extern CBurSimApp theApp;
extern enum enum_wnd_type WndType;
extern int nWindows;
CDrawWnd *pConsole1=NULL;
extern enum enum_display_type DisplayLeft, DisplayRight;
extern BOOL bInitialMinimize;

//=== Function prototypes ================================
void DoScheduleEvents();
void MakeVersionString(char *pszVersionString);

/////////////////////////////////////////////////////////////////////////////
// CWndMain

CWndMain::CWndMain()
{
}

CWndMain::~CWndMain()
{
}


BEGIN_MESSAGE_MAP(CWndMain, CWnd)
	//{{AFX_MSG_MAP(CWndMain)
	ON_WM_CREATE()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_SYSCOMMAND()
	ON_WM_SETFOCUS()
	ON_WM_QUERYENDSESSION()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWndMain message handlers

BOOL CWndMain::Create(enum enum_wnd_type wnd_type) 
{
   BOOL bRetval;

   HCURSOR hcursor;
   if(WND_TYPE_SAVER==WndType) {
      hcursor = ::LoadCursor(AfxGetResourceHandle(),MAKEINTRESOURCE(IDC_NULLCURSOR));
   } else {
      hcursor = ::LoadCursor(NULL,IDC_ARROW);
   }
    // Register a class with no cursor
	if (m_lpszClassName == NULL)
	{
      HBRUSH hbrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
    	m_lpszClassName = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW,hcursor,hbrush,
         theApp.LoadIcon(IDI_ICON_CONSOLE) );
	}

   pConsole1 = new CDrawWnd;
   int top=20,left=20,bottom=500,right=980;
   int xscreen=::GetSystemMetrics(SM_CXSCREEN);
   int yscreen=::GetSystemMetrics(SM_CYSCREEN);
   if(WND_TYPE_SAVER == wnd_type) {
      // Create screen-saver style window
      right = xscreen;
      bottom = yscreen;
	   CRect rect(0, 0, right, bottom);
      CWnd::CreateEx(WS_EX_TOPMOST, m_lpszClassName, "BurSim",
      WS_VISIBLE|WS_POPUP, rect, NULL, 0, NULL);
      if(right>xscreen) right=xscreen-10;
      bRetval = pConsole1->Create(0, WS_VISIBLE|WS_CHILD,
         rect, this, 0, NULL);
      pConsole1->m_display_type = DisplayLeft;

   } else {
      CRect rect(left,top,right,bottom);
      DWORD dwExStyle=0;  
      DWORD dwStyle=WS_VISIBLE|WS_POPUP|WS_THICKFRAME|WS_CAPTION|WS_SYSMENU
         |WS_MINIMIZEBOX|WS_MAXIMIZEBOX; 
      if(bInitialMinimize) {
         dwStyle |= WS_MINIMIZE;
      }
      CWnd::CreateEx(dwExStyle, m_lpszClassName, "BurSim", 
         dwStyle, rect, NULL, 0, NULL);

      dwStyle = WS_VISIBLE | WS_CHILD;
      bRetval = pConsole1->Create(dwExStyle,dwStyle,rect, this, 0, NULL);
   }

   pConsole1->m_display_type = DisplayLeft;

   return bRetval;
}

int CWndMain::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
   if(WND_TYPE_SAVER!=WndType) {
      CMenu *pmenu = GetSystemMenu(FALSE);
      if(bInitialMinimize) {
         pmenu->EnableMenuItem(SC_RESTORE, FALSE);
      }
      pmenu->AppendMenu(MF_SEPARATOR, 0, "" ); 
      pmenu->AppendMenu(MF_ENABLED|MF_STRING, IDD_DIALOG_CONFIG, "&Configure...");
      //pmenu->AppendMenu(MF_ENABLED|MF_STRING, IDC_RUN_SCRIPT, "&Run script");
      pmenu->AppendMenu(MF_ENABLED|MF_STRING, ID_HELP, "&Help...");
      pmenu->AppendMenu(MF_ENABLED|MF_STRING, IDD_ABOUTBOX, "&About...");
   }	
   ModifyStyle(0,WS_SYSMENU|WS_CAPTION|WS_MINIMIZEBOX);

   SetFocus();
	
	return 0;
}

void CWndMain::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
   if('+'==nChar || '-'==nChar) {
      //ProcessKeyboardPlusOrMinus(nChar);
   } else if(WND_TYPE_SAVER==WndType) {
   	PostMessage(WM_CLOSE);
   } else {
   	CWnd::OnChar(nChar, nRepCnt, nFlags);
   }
}

void CWndMain::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
   if(WND_TYPE_SAVER==WndType) {
      if(VK_CONTROL==nChar) {
      	PostMessage(WM_CLOSE);
         return;
      } else if(VK_ADD==nChar) {
         //ProcessKeyboardPlusOrMinus('+');
         return;
      } else if(VK_SUBTRACT==nChar) {
         //ProcessKeyboardPlusOrMinus('-');
         return;
      }
   }
 	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CWndMain::OnSysCommand(UINT nID, LPARAM lParam) 
{
	// TODO: Add your message handler code here and/or call default
   if(WND_TYPE_SAVER == WndType) {
   	if ((nID == SC_SCREENSAVE) || (nID == SC_CLOSE)) return;
   }
   if(IDD_ABOUTBOX == nID) {
      char msg[2048], version[40];
      strcpy(msg,
       "The BurSim screen saver simulates the console \r\n"
       "of a Burroughs mid-range (Unisys V-Series) computer.\r\n"
       "\r\n"
       //"See http://www.awa.com/softlock/mrr\r\n\r\n"
       "Mark Riordan   riordan@awa.com  or  riordan@rocketmail.com\r\n\r\n"
       "Version ");
      MakeVersionString(version);
      strcat(msg,version);
      strcat(msg,", compiled ");
      strcat(msg,__DATE__);
      strcat(msg," ");
      strcat(msg,__TIME__);
      ::MessageBox(NULL,msg,"BurSim screen saver",MB_OK);
   } else if(ID_HELP == nID) {
      theApp.WinHelp(0,HELP_CONTENTS);
   } else if(IDD_DIALOG_CONFIG == nID) {
      theApp.DoConfig();
   } else if(bInitialMinimize && SC_RESTORE == nID) {
      char *pmsg = "BurSim's main window is disabled. \r\n"
         "To enable BurSim's main window, run it without the -n option.";
      ::MessageBox(NULL,pmsg,"BurSim", MB_OK);
      // Do not restore window if -n was specified on command line.
   } else {
   	CWnd::OnSysCommand(nID, lParam);  // Was CDrawWnd
   }
}

void CWndMain::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	// TODO: Add your message handler code here
   if(pConsole1 && pConsole1->m_hWnd) pConsole1->SetFocus();	
}

BOOL CWndMain::OnQueryEndSession() 
{
	if (!CWnd::OnQueryEndSession())
		return FALSE;
	
   PostQuitMessage(0);
   
	return TRUE;
}
