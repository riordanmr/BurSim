// drawwnd.cpp : implementation file
//

#include "stdafx.h"
#include "WndMain.h"
#include "BurSimApp.h"
#include "drawwnd.h"
#include "AXScript.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//=== Global Variables ===================================================
extern CBurSimApp theApp;
extern int Debug;
extern enum enum_wnd_type WndType;
extern int nWindows;
LPCTSTR CDrawWnd::m_lpszClassName = NULL;
LPCTSTR CWndMain::m_lpszClassName = NULL;
int nCharsAcrossConsole = 80;
extern int nMsTimerRowDraw;
extern int FontSize1Saver, FontSize1Windowed;
extern int SecsDelayBetweenRefreshes;
int nJobsToSkip=0;
int nMaxJobsPerScreen=16;
extern char **ScreenTable[];
extern CSPO SPO;
extern CString HostName;
int n80CharLen=0;
extern CArray<CJob, CJob &> Mix;
extern CRITICAL_SECTION critMix;

COLORREF colorBk = RGB(0,0,0);
COLORREF colorGreen = RGB(0,255,0);
//COLORREF colorGreenBright = RGB(128,255,128);
//COLORREF colorUnsure = RGB(0,192,156);
//COLORREF colorUnsureBright = RGB(66,200,200);

// Colors has text colors in this order:
//    colorGreen, colorGreenBright, colorUnsure, colorUnsureBright
#define COLOR_MASK_UNSURE  0x2
#define COLOR_MASK_BRIGHT  0x1
                    //    0,255,0       128,255,128
COLORREF Colors[] = { RGB(0,127,0), RGB(166,255,166), 
   RGB(0,192,156), RGB(128,255,255) };

int nCharWidth=14;  // Initial guess; is computed in Draw.

//=== Function Prototypes ================================================

/////////////////////////////////////////////////////////////////////////////
// CDrawWnd

CDrawWnd::CDrawWnd(BOOL bAutoDelete)
{
	m_bAutoDelete = bAutoDelete;
	m_rgnLast.CreateRectRgn(0,0,0,0);
   m_bUpdateFont = TRUE;
   m_ptLast = CPoint(-1,-1);
   m_nLineThisDraw = 0;
   memset(m_Screen,0, sizeof m_Screen);
   for(int irow=0; irow<SCREENROWS; irow++) {
      memset(m_Screen, ' ', MAXLINELEN);
   }

   m_xmargin = 2;
   m_ymargin = 1;
   m_cursor_col = 0;
   InitJobStructs();
}

CDrawWnd::~CDrawWnd()
{
}


BEGIN_MESSAGE_MAP(CDrawWnd, CWnd)
	//{{AFX_MSG_MAP(CDrawWnd)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_SIZE()

	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_WM_NCACTIVATE()
	ON_WM_ACTIVATE()
	ON_WM_ACTIVATEAPP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
   ON_WM_CHAR()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*--- function CDrawWnd::SetScreenRow -----------------------------------
 * Set one row of screen data in the copy of the screen in m_Screen.
 */
void CDrawWnd::SetScreenRow(int irow, CString strLine)
{
   int len=strLine.GetLength();
   if(len>MAXLINELEN) len = MAXLINELEN;
   memcpy(m_Screen[irow],strLine,len);
   int n_to_set = MAXLINELEN-len;
   if(n_to_set > 0) memset(m_Screen[irow]+len, ' ', n_to_set);
}

/*--- function CDrawWnd::SetConsoleErrorLine -------------------------------
 */
void CDrawWnd::SetConsoleErrorLine(CString strMsg)
{
   SetScreenRow(KEYBOARD_ERROR_MSG_ROW, strMsg);
   SetRefreshTimer();
}

/*--- function CDrawWnd::CopyScreen -----------------------------------
 * Copy a predefined screen to the m_Screen area.
 * This routine is not used much any more, as we are less likely to
 * copy predefined screens & more likely to build them on-the-fly now.
 */
void CDrawWnd::CopyScreen(char *ary[]) {
   char *cp;
   for(int irow=FIRST_ROW_DATA; cp=ary[irow]; irow++) {
      SetScreenRow(irow,ary[irow]);
   }
}

/*--- function CDrawWnd::MyCreateFont ---------------------------------
 */
BOOL CDrawWnd::MyCreateFont(CDC& dc)
{
   CString csFontFace = "Lucida Console";
   CFont *pFormerFont=NULL;
   int nFontSize;

   nFontSize = 8;  // Default, just to play it safe.
   switch(WndType) {
   case WND_TYPE_SAVER:
      nFontSize = FontSize1Saver;
      break;
   case WND_TYPE_NORMAL:
      nFontSize = FontSize1Windowed;
   }
   int nHeight = MulDiv(nFontSize, dc.GetDeviceCaps(LOGPIXELSY), 72);
   int nWeight = FW_DONTCARE;
   int nPitchAndFamily = FF_DONTCARE | FIXED_PITCH;
   BOOL bOK = m_fontConsole.CreateFont(nHeight,0,0,0,nWeight,FALSE,FALSE,FALSE,
      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
      DEFAULT_QUALITY, nPitchAndFamily, csFontFace);
   bOK = m_fontConsoleUnderlined.CreateFont(nHeight,0,0,0,nWeight,FALSE,TRUE,FALSE,
      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
      DEFAULT_QUALITY, nPitchAndFamily, csFontFace);
   return bOK;
}

void CDrawWnd::CalculateFontInfo(CDC& dc)
{
   TEXTMETRIC Metrics;
   dc.GetTextMetrics(&Metrics);
   m_height = Metrics.tmHeight;
   char *pszStrScreenLength = "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE";
   CSize sizestr=dc.GetTextExtent(pszStrScreenLength,MAXLINELEN);
   n80CharLen = sizestr.cx;
   nCharWidth = sizestr.cx/MAXLINELEN;
   // Center the display if in screen saver mode.
   if(WND_TYPE_SAVER==WndType) {
      int width = sizestr.cx;
	   CRect rect;
	   GetClientRect(&rect);
      int totalwidth = rect.right;
      if(totalwidth > width) {
         m_xmargin = (totalwidth-width) / 2;
         if(m_xmargin<0) m_xmargin=0;
      } 
      if(rect.bottom > 28*m_height) {
         m_ymargin = (rect.bottom - 28*m_height)/2;
      }
   }
}

/*--- function CDrawWnd::PutLine ---------------------------------------
 *
 * Draw a line to the window.
 * 
 * The following characters in the input line do not take up a 
 * character position on the display, but rather toggle the
 * font color:
 *  ` turns on bright
 *  ~ turns off bright
 *  { switches to alternate color, meaning I'm not sure about this
 *    part of the display.
 *  } switches back to standard green.
 */
void CDrawWnd::PutLine(int iline,const char *fmt,...)
{
   va_list argptr;
   char msg[12000], *pmsg=msg;
   char *pStartNext;
   int len, thislen, x=m_xmargin;
   int coloridx=0;

   va_start (argptr, fmt);
   len = vsprintf (msg, fmt, argptr);
   va_end (argptr);
   memset(msg+len, ' ', MAXLINELEN-len);

   if(Debug>4) {
      CString strLine(msg);
      strLine.TrimRight();
      FmtDebug("PutLine %d: %s", iline, strLine);
   }

   m_pdc->SetTextColor(colorGreen);
   while(pmsg < msg+nCharsAcrossConsole) {
      pStartNext = strpbrk(pmsg,"`~{}");
      if(!pStartNext) pStartNext = msg+nCharsAcrossConsole;
      thislen = pStartNext - pmsg;
      m_pdc->TextOut(x,m_ymargin + m_height*iline,pmsg,thislen);
      x += thislen * nCharWidth;
      pmsg = pStartNext + 1;
      switch(*pStartNext) {
      case '`':
         coloridx |= COLOR_MASK_BRIGHT;
         break;
      case '~':
         coloridx &= 0xff - COLOR_MASK_BRIGHT;
         break;
      case '{':
         coloridx |= COLOR_MASK_UNSURE;
         break;
      case '}':
         coloridx &= 0xff - COLOR_MASK_UNSURE;
         break;
      }
      m_pdc->SetTextColor(Colors[coloridx]);
   }
   // Underline this if it's the third line of the display.
   if(333 == iline) {
      CPen pen(PS_SOLID,1,colorGreen);
      m_pdc->SelectObject(&pen);

      int x=m_xmargin;
      int y=m_ymargin + 3*m_height-1;
      m_pdc->MoveTo(x,y);
      x += n80CharLen;
      m_pdc->LineTo(x,y);
   }
   //m_pdc->SelectObject(m_fontConsole);
}

/*--- function CDrawWnd::Draw ---------------------------------------
 */
void CDrawWnd::Draw(CDC& dc, int nLine) 
{
	CRgn rgnNew, rgnBlack;
   CFont *pFormerFont=NULL;

   if(Debug>5)FmtDebug("Entering Draw");
   m_pdc = &dc;

   if(m_bUpdateFont) {
      m_bUpdateFont = FALSE;
      m_fontConsole.DeleteObject();
      m_fontConsoleUnderlined.DeleteObject();
      if(MyCreateFont(dc)) {
         dc.SelectObject(m_fontConsole);
         CalculateFontInfo(dc);
      } else {
         AfxMessageBox("Can't create font");
      }
   } else {
      dc.SelectObject(m_fontConsole);
   }

   dc.SetTextColor(colorGreen);
   dc.SetBkColor(colorBk);

#if 0
   CTime mytime(time(NULL));
   CString str = mytime.Format("%m/%d/%Y %H:%M:%S");
   CString str2;
   str2.Format("%2d ",nLine);
   PutLine(nLine, str2 + str + "  " + CString('=',MyRand(40)));
#else
   int len = strlen(m_Screen[nLine]);
   if(len < 0 || len > MAXLINELEN) {
      AfxMessageBox("Bad line length");
   }
   if(KEYBOARD_ROW == nLine) {
      DrawKeyboardArea(dc);
   } else if(PROCESSOR_STATUS_ROW == nLine) {
      DrawProcessorStatusLine(dc);
   } else {
      PutLine(nLine,"%s",m_Screen[nLine]);
   }
#endif
   if(Debug>6)FmtDebug("Leaving Draw()");
}

void CDrawWnd::DrawKeyboardArea(CDC &dc)
{
   int nchars, x=m_xmargin;
   m_pdc = &dc;
   m_pdc->SetTextColor(colorGreen);
   char *pmsg = m_Screen[KEYBOARD_ROW];
   nchars = m_cursor_col;

   m_pdc->SelectObject(m_fontConsole);
   dc.SetTextColor(colorGreen);
   dc.SetBkColor(colorBk);

   m_pdc->TextOut(x,m_ymargin + m_height*KEYBOARD_ROW,pmsg,nchars);
   x += nchars * nCharWidth;

   m_pdc->SelectObject(m_fontConsoleUnderlined);
   pmsg += nchars;
   m_pdc->TextOut(x,m_ymargin + m_height*KEYBOARD_ROW,pmsg,1);

   x += 1 * nCharWidth;
   pmsg += 1;
   nchars = MAXLINELEN - m_cursor_col - 1;
   m_pdc->SelectObject(m_fontConsole);
   m_pdc->TextOut(x,m_ymargin + m_height*KEYBOARD_ROW,pmsg,nchars);

}

void CDrawWnd::DrawProcessorStatusLine(CDC &dc)
{
   m_pdc = &dc;
   m_pdc->SelectObject(m_fontConsoleUnderlined);
   PutLine(PROCESSOR_STATUS_ROW,"%s",m_Screen[PROCESSOR_STATUS_ROW]);
   m_pdc->SelectObject(m_fontConsole);
}

void CDrawWnd::SetRefreshTimer()
{
   KillTimer(TIMER_START_REFRESH);
	VERIFY(SetTimer(TIMER_START_REFRESH, 1000*SecsDelayBetweenRefreshes, NULL) != 0);
}

void CDrawWnd::DelayRefresh()
{
   KillTimer(TIMER_START_REFRESH);
   SetTimer(TIMER_DELAY_REFRESH, 4*1000*SecsDelayBetweenRefreshes, NULL);
}

void CDrawWnd::StartScreenPaint()
{
   if(Debug>4) FmtDebug("StartScreenPaint m_display_type=%d", m_display_type);
   m_nLineThisDraw = FIRST_ROW_REGULAR_REFRESH;
   SetTimer(TIMER_DRAW_ROW, nMsTimerRowDraw, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CDrawWnd message handlers

void CDrawWnd::OnTimer(UINT nIDEvent) 
{
   int j;

   switch(nIDEvent) {
   case TIMER_DELAY_REFRESH:
      if(Debug>4) FmtDebug("TIMER_DELAY_REFRESH");
      KillTimer(TIMER_DELAY_REFRESH);
      SetRefreshTimer();
      // fallthrough...

   case TIMER_START_REFRESH:
      if(Debug>4) FmtDebug("TIMER_START_REFRESH old m_display_type=%d",m_display_type);
      j = (int) m_display_type;  j++; m_display_type = (enum enum_display_type)j;
      if(m_display_type>DISP_LAST_REFRESH) m_display_type = DISP_MIX;
      m_spo_lines_back = SPO_MSGS_DISPLAYED;
      UpdateScreen();
      StartScreenPaint();
      break;

   case TIMER_DRAW_ROW:
      {
         if(Debug>4) FmtDebug("TIMER_DRAW_ROW m_display_type=%d",m_display_type);
         CClientDC dc(this);
		   Draw(dc, m_nLineThisDraw);
         // After error message line has been drawn, clear it so it won't
         // be displayed next time.
         if(KEYBOARD_ERROR_MSG_ROW==m_nLineThisDraw) {
            SetConsoleErrorLine(" ");
         }
         if(++m_nLineThisDraw >= SCREENROWS) {
            KillTimer(TIMER_DRAW_ROW);
         }
      }
      break;
   default:	
		CWnd::OnTimer(nIDEvent);
      break;
   }
}

void CDrawWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	m_rgnLast.DeleteObject();
	m_rgnLast.CreateRectRgn(0,0,0,0);
	CBrush brush(RGB(0,0,0));
	CRect rect;
	GetClientRect(rect);
	dc.FillRect(&rect, &brush);
	//int nWidth = m_nWidth * rect.Width() + ::GetSystemMetrics(SM_CXSCREEN)/2;
	//nWidth = nWidth/::GetSystemMetrics(SM_CXSCREEN);

   for(int iline=0; iline<SCREENROWS; iline++) {
      Draw(dc,iline);
   }

	// Do not call CWnd::OnPaint() for painting messages
}

/*--- function CDrawWnd::SetFixedScreenAreas --------------------------
 */
void CDrawWnd::SetFixedScreenAreas()
{
   char line[MAXLINELEN+4];

   //memset(m_Screen[0],' ',MAXLINELEN);
   memset(m_Screen[1],' ',MAXLINELEN);
   time_t mytime;
   time(&mytime);
   m_memory = 7000000 + 1000*MyRand(4000);
   double f_memory = m_memory / 1000000.0;
   m_pct_cpu_avail = (600 + MyRand(399)) / 10.0;

   int n=sprintf(line," %-19.19s%10.3f MD Mem,%6.1f%% CPU Avail      ",
      HostName, f_memory, m_pct_cpu_avail);
   strftime(line+n,24,"%m/%d/%Y %H:%M:%S",localtime(&mytime));
   memcpy(m_Screen[PROCESSOR_STATUS_ROW],line,MAXLINELEN);
   memset(m_Screen[3],' ',MAXLINELEN);
   memset(m_Screen[4],' ',MAXLINELEN);
}

/*--- function CDrawWnd::UpdateScreen ---------------------------------
 *
 *  Updates the m_Screen screen image from data that's elsewhere.
 */
void CDrawWnd::UpdateScreen() 
{
   int startln, iline=0, msgline, imix, j;
   CDWordArray arySortedMix;
   CStringArray aryMixLines;
   CString strline;

   switch (m_display_type) {
   case DISP_MSGS:
      // Compute the index of the first line to display.
      // Skip backwards from the m_spo_next_idx index--
      // but if we have never wrapped, don't go back prior to the beg
      // of the buffer.
      // The amount by which we skip back is m_spo_lines_back.
      startln = SPO.m_spo_next_idx - m_spo_lines_back;
      if(startln < 0) {
         if(SPO.m_n_wrapped) {
            startln += MAX_SPO_MSGS;
            if(startln < 0) startln = 0;
         } else {
            startln = 0;
         }
      }
      strcpy(m_Screen[SPO_IDX_FIRST],"------------------------------[  System Messages  ]-----------------------------");
      
      for(iline=0, msgline=startln; iline < SPO_MSGS_DISPLAYED; iline++, msgline++) {
         if(msgline >= MAX_SPO_MSGS) msgline = 0;
         memcpy(m_Screen[iline+SPO_IDX_FIRST+1],SPO.m_spo_msgs[msgline].msg, MAXLINELEN);
      }
      break;
   case DISP_MIX:
      for(iline=SPO_IDX_FIRST; iline<SCREENROWS; iline++) {
         memset(m_Screen[iline], ' ', MAXLINELEN);
      }
      EnterCriticalSection(&critMix);
      CJob::SortJobs(arySortedMix);
      strline = "  Mix#  Active    Mix         P M  Mem";
      aryMixLines.Add(strline);
      for(j=0; j<arySortedMix.GetSize(); j++) {
         imix = (int) arySortedMix[j];
         strline.Format("%7d %-21.21s%2d%2d%5d",Mix[imix].m_mixnum, 
            Mix[imix].m_progname, Mix[imix].m_priority, Mix[imix].m_priority,
            Mix[imix].m_memory);
         aryMixLines.Add(strline);
      }
      aryMixLines.Add(" ");
      aryMixLines.Add("  Mix#  Waiting   Mix         P M  Mem");
      aryMixLines.Add(" ");
      aryMixLines.Add("Null Schedule");
      // Set up the left side of the screen.
      for(iline=SPO_IDX_FIRST, imix=0; iline<SCREENROWS &&
         imix<aryMixLines.GetSize(); iline++, imix++ ) {
         strline = BlankPad(aryMixLines[imix], 39);
         // Put a '.' in the middle of the screen for header lines
         // and mix lines.
         if(TRUE || isdigit(strline.GetAt(6)) || isalpha(strline.GetAt(2))) {
            strline = strline + ".";
         }
         memcpy(m_Screen[iline], strline, 40);
      }
      // Set up the right side of the screen.
      if(imix < aryMixLines.GetSize()) {
         // If the next line in the output array is blank, skip it rather
         // than put it at the top of the screen.
         strline = aryMixLines[imix];
         strline.TrimRight();
         if(!strline.GetLength()) {
            imix++;
         }
      }
      for(iline=SPO_IDX_FIRST; iline<SCREENROWS &&
         imix<aryMixLines.GetSize(); iline++, imix++ ) {
         j = aryMixLines[imix].GetLength();
         memcpy(m_Screen[iline]+40, aryMixLines[imix], j);
      }
      LeaveCriticalSection(&critMix);
      break;
   case DISP_DISKS:
      CopyScreen(ScreenTable[m_display_type]);
      break;
   }
   SetFixedScreenAreas();
}

void CDrawWnd::UpdateKeyboardArea()
{
   CClientDC dc(this);
   Draw(dc, KEYBOARD_ROW);
}

//================================================================================

void CDrawWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	//m_nScale = cx/29;
	m_nHeight = cy;
}

int CDrawWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
   SetRefreshTimer();
   // Cause the timer to go off immediately.
	PostMessage(WM_TIMER, TIMER_START_REFRESH, 0);
   SetFocus();

	return 0;
}

BOOL CDrawWnd::Create(DWORD dwExStyle, DWORD dwStyle, const RECT& rect, 
	CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	BOOL bOK = CreateEx(dwExStyle, m_lpszClassName, _T(""), dwStyle, 
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 
		pParentWnd->GetSafeHwnd(), NULL, NULL );
   return bOK;
}

void CDrawWnd::PostNcDestroy() 
{
	if (m_bAutoDelete)
		delete this;
}

void CDrawWnd::OnDestroy() 
{
	PostQuitMessage(0);
}

BOOL CDrawWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
   if(WND_TYPE_SAVER == WndType) {
   	SetCursor(NULL);
   } else {
      SetCursor(::LoadCursor(NULL,IDC_ARROW));
   }
	return TRUE;
}

BOOL CDrawWnd::OnNcActivate(BOOL bActive) 
{
	if (!bActive)
		return FALSE;
	return CWnd::OnNcActivate(bActive);
}

void CDrawWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
   if(Debug>2) FmtDebug("OnActivate");
   if (nState == WA_INACTIVE) {
#if 01
      if(Debug>1) FmtDebug("Quitting on OnActivate");
		PostMessage(WM_CLOSE);
#endif
   }
	CWnd::OnActivate(nState, pWndOther, bMinimized);
}

void CDrawWnd::OnActivateApp(BOOL bActive, HTASK hTask) 
{
   if(Debug>2) FmtDebug("OnActivateApp");
   if (!bActive) {
      if(Debug>1) FmtDebug("Quitting on OnActivateApp");
		PostMessage(WM_CLOSE);
   }
	CWnd::OnActivateApp(bActive, hTask);
}

void CDrawWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
   if(WND_TYPE_SAVER==WndType) {
      if (m_ptLast == CPoint(-1,-1)) {
		   m_ptLast = point;
      } else if (m_ptLast != point) {
         if(Debug>1) FmtDebug("Quitting on OnMouseMove");
		   PostMessage(WM_CLOSE);
      }
   }
}

void CDrawWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
   if(WND_TYPE_SAVER==WndType) {
      if(Debug>1) FmtDebug("Quitting on OnLButtonDown");
   	PostMessage(WM_CLOSE);
   }
	CWnd::OnLButtonDown(nFlags, point);
}

void CDrawWnd::OnMButtonDown(UINT nFlags, CPoint point) 
{
   if(WND_TYPE_SAVER==WndType) {
      if(Debug>1) FmtDebug("Quitting on OnMButtonDown");
   	PostMessage(WM_CLOSE);
   }
	CWnd::OnMButtonDown(nFlags, point);
}

void CDrawWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
   if(WND_TYPE_SAVER==WndType) {
      if(Debug>1) FmtDebug("Quitting on OnRButtonDown");
   	PostMessage(WM_CLOSE);
   }
	CWnd::OnRButtonDown(nFlags, point);
}

void CDrawWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
   BOOL bDrawKeyboard = TRUE;
   if(WND_TYPE_SAVER==WndType) {
      if(Debug>1) FmtDebug("Quitting on OnKeyDown");
   	PostMessage(WM_CLOSE);
      bDrawKeyboard = FALSE;
   }
   if(VK_LEFT == nChar || VK_BACK == nChar) {
      if(--m_cursor_col < 0) m_cursor_col = MAXLINELEN-1;
   } else if(VK_RIGHT == nChar) {
      m_cursor_col++;
      if(m_cursor_col >= MAXLINELEN) m_cursor_col=0;
   } else if(VK_HOME == nChar) {
      m_cursor_col = 0;
   } else if(VK_TAB == nChar) {
      m_cursor_col = 8 + 8*(m_cursor_col/8);
      if(m_cursor_col >= MAXLINELEN) m_cursor_col=0;
   } else if(VK_RETURN == nChar) {
      ExecuteCommandFromConsole();
   } else {
   	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
      bDrawKeyboard = FALSE;
   }
   if(bDrawKeyboard) {
      UpdateKeyboardArea();
   }
}

void CDrawWnd::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
   if(WND_TYPE_SAVER==WndType) {
      if(Debug>1) FmtDebug("Quitting on OnSysKeyDown");
   	PostMessage(WM_CLOSE);
   }
	CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void CDrawWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
   if(isprint(nChar)) {
      m_Screen[KEYBOARD_ROW][m_cursor_col] = toupper(nChar);
      m_cursor_col++;
      if(m_cursor_col >= MAXLINELEN) m_cursor_col=0;
      UpdateKeyboardArea();
   } else {
      CWnd::OnChar(nChar, nRepCnt, nFlags);
   }
}

//================================================================================

void CDrawWnd::ExecuteCommandFromConsole()
{
   CString strCmd = CString(m_Screen[KEYBOARD_ROW],m_cursor_col);
   m_cursor_col = 0;
   UpdateKeyboardArea();
   ExecuteCommand(strCmd, TRUE);
   StartScreenPaint();
}
