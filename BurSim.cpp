// BurSim.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "BurSimApp.h"
#include "DrawWnd.h"
#include "WndMain.h"
#include "DlgConfig.h"
#include "GetOptpr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static char *author = "Mark Riordan  riordan@awa.com, riordan@rocketmail.com  Sept 1999";

//=== Global variables ===========================================
enum enum_wnd_type WndType=WND_TYPE_NONE;
int Debug=3;
int ScheduleTick=0;
char szLogFile[_MAX_PATH];
char szListenIP[24] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int nWindows = 1;
int nMsTimerRowDraw=400/24;
enum enum_display_type DisplayLeft=DISP_MIX;
int FontSize1Saver, FontSize1Windowed;
int SecsDelayBetweenRefreshes = 10;
BOOL bRunDebuggerOnRuntimeError=FALSE;
BOOL bShowWelcome=TRUE;
BOOL bInitialMinimize=FALSE;
extern CDrawWnd *pConsole1;
CSPO SPO;
CString HostName;
CRITICAL_SECTION critMix;
CString strScriptsPath;
CString strInitFile;

extern int TCPListenPort;

//=== Function Prototypes ========================================
int InitTelnetd();
BurSimRet StartProgram(const char *szProgram, int priority);
void InitialMsgs();

/////////////////////////////////////////////////////////////////////////////
// CBurSimApp

BEGIN_MESSAGE_MAP(CBurSimApp, CWinApp)
	//{{AFX_MSG_MAP(CBurSimApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

TCHAR szConfig[]=_T("Config");

/////////////////////////////////////////////////////////////////////////////
// CBurSimApp construction

CBurSimApp::CBurSimApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBurSimApp object

CBurSimApp theApp;

// This is the fixed version from Q200440 

BOOL MatchOption(LPTSTR lpsz, TCHAR ch)
{
    if (lpsz[0] == '-' || lpsz[0] == '/')
        lpsz++;

    if (lpsz[0] == ch)
        return TRUE;

    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CBurSimApp initialization

BOOL CBurSimApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
   int ch;
   BOOL bRetval = FALSE;
   extern char    *myoptarg;
   char szbuf[256];
   enum enum_cmd_action {CMDLINE_CONFIG, CMDLINE_PARENT, CMDLINE_WINDOWED,
      CMDLINE_SAVER} CmdlineAction = CMDLINE_WINDOWED;

   strInitFile = "BurSim.INI";
	Enable3dControls();
	//SetRegistryKey(_T("Mark Riordan"));
   InitializeCriticalSection(&critMix);

   // Parse the command line
   while(EOF != (ch=mygetopt(__argc, __argv, "cswpnD:t:d:i:"))) {
      switch(ch) {
      case 'D':
         Debug = atoi(myoptarg);
         break;
      case 'c':
         CmdlineAction = CMDLINE_CONFIG;
         break;
      case 's':
         CmdlineAction = CMDLINE_SAVER;
         break;
      case 'n':
         bInitialMinimize = TRUE;
      case 'w':
         CmdlineAction = CMDLINE_WINDOWED;
         break;
      case 'p':
         CmdlineAction = CMDLINE_PARENT;
         break;
      case 'i':
         strInitFile = myoptarg;
         break;
      }
   }

   LoadSettings();

   switch(CmdlineAction) {
   case CMDLINE_CONFIG:
		DoConfig();
      break;
   case CMDLINE_PARENT:
      {
		CWnd* pParent = CWnd::FromHandle((HWND)atol(__argv[2]));
		ASSERT(pParent != NULL);
		CDrawWnd* pWnd = new CDrawWnd();
		CRect rect;
		pParent->GetClientRect(&rect);
		pWnd->Create(NULL, WS_VISIBLE|WS_CHILD, rect, pParent, NULL);
		m_pMainWnd = pWnd;
		bRetval = TRUE;
      }
      break;
   case CMDLINE_SAVER:
      WndType = WND_TYPE_SAVER;
      break;
   case CMDLINE_WINDOWED:
      WndType = WND_TYPE_NORMAL;
      break;
   }
   if(WND_TYPE_SAVER == WndType || WND_TYPE_NORMAL == WndType) {
		CWndMain* pWnd = new CWndMain; //CBurSimWnd;
		pWnd->Create(WndType);
		m_pMainWnd = pWnd;
		bRetval =  TRUE;
   }

   // Initialize log file
   time_t mytime;
   time(&mytime);
   strcpy(szbuf,ctime(&mytime));
   szbuf[20] = 0;
   if(Debug){
      FILE *stlog = fopen(szLogFile,"w");
      if(stlog) {
         fprintf(stlog,"=== BurSim starting at %s ============================\n",szbuf);
         fclose(stlog);
      }
   }

   InitSTOQs();
   InitTelnetd();
   InitialMsgs();
   StartProgram("INIT", DEFAULT_UNISYS_PRIORITY);

	return bRetval;
}

static TCHAR szSectionConfig[] = _T("Config");
static TCHAR szDescDisplays[] = _T("Displays");
static TCHAR szDescFontSize1Saver[] = _T("FontSize1Saver");
static TCHAR szDescFontSize1Windowed[] = _T("FontSize1Windowed");
static TCHAR szDescSecsDelayBetweenRefreshes[] = _T("SecsDelayBetweenRefreshes");
static TCHAR szDescDebug[] = _T("Debug");
static TCHAR szDescHostName[] = _T("Hostname");
static TCHAR szDescScriptsPath[] = _T("ScriptsPath");
static TCHAR szDescLogFile[] = _T("LogFile");
static TCHAR szDescRunDebuggerOnRuntimeError[] = _T("RunDebuggerOnRuntimeError");
static TCHAR szDescTCPListenPort[] = _T("TCPListenPort");
static TCHAR szDescListenIP[] = _T("ListenIP");
static TCHAR szDescShowWelcome[] = _T("ShowWelcome");
static TCHAR szDesc[] = _T("");

void CBurSimApp::LoadSettings()
{
   char szbuf[512];
   GetPrivateProfileString(szSectionConfig,szDescLogFile, "BurSim.Log", szLogFile, sizeof szLogFile, strInitFile);
   FontSize1Saver = GetPrivateProfileInt(szSectionConfig,szDescFontSize1Saver, 15, strInitFile);
   FontSize1Windowed = GetPrivateProfileInt(szSectionConfig,szDescFontSize1Windowed, 10, strInitFile);
   SecsDelayBetweenRefreshes = GetPrivateProfileInt(szSectionConfig,szDescSecsDelayBetweenRefreshes, 10, strInitFile);
   bRunDebuggerOnRuntimeError = GetPrivateProfileInt(szSectionConfig,szDescRunDebuggerOnRuntimeError, FALSE, strInitFile);
   bShowWelcome = GetPrivateProfileInt(szSectionConfig,szDescShowWelcome, TRUE, strInitFile);
   TCPListenPort = GetPrivateProfileInt(szSectionConfig,szDescTCPListenPort, 23, strInitFile);
   GetPrivateProfileString(szSectionConfig, szDescListenIP, "", szListenIP, sizeof szListenIP, strInitFile);
   Debug = GetPrivateProfileInt(szSectionConfig,szDescDebug, 0, strInitFile);
   // Use the Windows computer name of this computer as the default host name.
   char szhostname[64];
   DWORD dwlen = sizeof(szhostname);
   GetComputerName(szhostname, &dwlen);
   GetPrivateProfileString(szSectionConfig,szDescHostName, szhostname, szbuf, sizeof szbuf, strInitFile);
   HostName = szbuf;

   GetPrivateProfileString(szSectionConfig, szDescScriptsPath, "DISK", szbuf, sizeof szbuf, strInitFile);
   strScriptsPath = szbuf;
}

void CBurSimApp::SaveSettings()
{
   char szbuf[256];
   WritePrivateProfileString(szSectionConfig,szDescFontSize1Saver,itoa(FontSize1Saver, szbuf, 10), strInitFile);
   WritePrivateProfileString(szSectionConfig,szDescFontSize1Windowed,itoa(FontSize1Windowed, szbuf, 10), strInitFile);
   WritePrivateProfileString(szSectionConfig,szDescSecsDelayBetweenRefreshes, itoa(SecsDelayBetweenRefreshes, szbuf, 10), strInitFile);
   WritePrivateProfileString(szSectionConfig,szDescRunDebuggerOnRuntimeError, itoa(bRunDebuggerOnRuntimeError, szbuf, 10), strInitFile);
   WritePrivateProfileString(szSectionConfig,szDescShowWelcome, itoa(bShowWelcome, szbuf, 10), strInitFile);
   WritePrivateProfileString(szSectionConfig,szDescTCPListenPort, itoa(TCPListenPort, szbuf, 10), strInitFile);
   WritePrivateProfileString(szSectionConfig,szDescDebug,itoa(Debug, szbuf, 10), strInitFile);
   WritePrivateProfileString(szSectionConfig,szDescHostName,HostName, strInitFile);
   WritePrivateProfileString(szSectionConfig,szDescScriptsPath, strScriptsPath, strInitFile);
}

void CBurSimApp::DoConfig()
{
   CDlgConfig dlg;
   dlg.m_FontsizeSaver1 = FontSize1Saver;
	dlg.m_FontsizeWindowed1 = FontSize1Windowed;
   dlg.m_nSecsDelayBetweenRefreshes = SecsDelayBetweenRefreshes;
   dlg.m_Debug = Debug;
   dlg.m_bRunDebugger = bRunDebuggerOnRuntimeError;
   dlg.m_bShowWelcome = bShowWelcome;
   dlg.m_TCPListenPort = TCPListenPort;

   if (dlg.DoModal() == IDOK) {
      FontSize1Saver = dlg.m_FontsizeSaver1;
	   FontSize1Windowed = dlg.m_FontsizeWindowed1;
      SecsDelayBetweenRefreshes = dlg.m_nSecsDelayBetweenRefreshes;
      if(SecsDelayBetweenRefreshes <= 0) SecsDelayBetweenRefreshes = 1;
      Debug = dlg.m_Debug;
      bRunDebuggerOnRuntimeError = dlg.m_bRunDebugger;
      bShowWelcome = dlg.m_bShowWelcome;
      TCPListenPort = dlg.m_TCPListenPort;

      SaveSettings();
      if(pConsole1) pConsole1->SetRefreshTimer();
      if(pConsole1) pConsole1->m_bUpdateFont = TRUE;
   }
}
