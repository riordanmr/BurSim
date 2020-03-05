// Command.cpp -- execute MCP commands.

#include "stdafx.h"

//=== Function prototypes =========================================
BurSimRet StartProgram(const char *szProgram, int priority);
int UnisysToNTPriority(int unisysPri);

//=== Global variables ============================================
extern int Debug;
extern CDrawWnd *pConsole1;
extern CArray<CJob, CJob &> Mix;
extern CRITICAL_SECTION critMix;
char *aryMixCommands[] = {"AX", "PR", NULL};
extern CSPO SPO;


/*--- function ParseCommand -----------------------------------
 *
 *  Exit:   mixnum   is the mix number from the beginning of the
 *                   command; zero if none.
 *          strCmd   is the command (such as AX)
 *          strAllArgs  is all arguments.  E.g., for 2AX HI THERE,
 *                   strAllArgs would be "HI THERE".
 *          aryArgs  is the array of arguments.  For the above command,
 *                   aryArgs[0] would be "HI".
 */
void ParseCommand(CString strCmdLine, int &cmd_mixnum, CString &strCmd, 
                  CString &strAllArgs, CStringArray &aryArgs)
{
   CString strMix = strCmdLine.SpanIncluding("0123456789");
   CString strArg;
   int len, thislen;
   strCmdLine.TrimLeft();
   strCmdLine.TrimRight();
   // Crack off leading mix number, if any.
   cmd_mixnum = 0;
   if(strMix.GetLength()) {
      cmd_mixnum = atoi(strMix);
      strCmdLine = strCmdLine.Mid(strMix.GetLength());
   }

   // First two letters are command.

   len = strCmdLine.GetLength();
   thislen = len>=2 ? 2 : len;
   strCmd = strCmdLine.Left(thislen);
   strCmdLine = strCmdLine.Mid(thislen);
   strCmdLine.TrimLeft();
   strAllArgs = strCmdLine;

   do {
      strCmdLine.TrimLeft();
      strArg = strCmdLine.SpanExcluding(" ");
      if(strArg.GetLength()) {
         aryArgs.Add(strArg);
         // Strip off token just cracked off.
         strCmdLine = strCmdLine.Mid(strArg.GetLength());
      } else {
         break;
      }
   } while(TRUE);
}

void EchoCommandToSPO(CString strCmdLine)
{
   CString strMsg = "==> " + strCmdLine;
   SPO.LogSPOMsg("",strMsg);
}

int ExecuteCommand(CString strCmdLine, BOOL bFromConsole)
{
   int retval=0;
   if(Debug>1) {
      FmtDebug("Executing cmd: '%s'", strCmdLine);
   }
   CString strCmd, strAllArgs, strline;
   CStringArray aryArgs;
   CDWordArray arySortedMix;
   int cmd_mixnum, nArgs, j, iarg, imix, iline;
   BOOL bMixCmdOK=FALSE, bCmdError=FALSE;
   BurSimRet ret;

   ParseCommand(strCmdLine, cmd_mixnum, strCmd, strAllArgs, aryArgs);
   nArgs = aryArgs.GetSize();

   if(!bFromConsole) {
   }

   if("EX" == strCmd) {
      int priority=DEFAULT_UNISYS_PRIORITY;
      for(iarg=1; iarg<nArgs-1; iarg++) {
         if("PR" == aryArgs[iarg]) {
            priority = atoi(aryArgs[iarg+1]);
            iarg++;
         } else {
            bCmdError = TRUE;
         }
      }
      EchoCommandToSPO(strCmdLine);
      if(bCmdError) pConsole1->SetConsoleErrorLine("  ** KEYBOARD IGNORED: MISSING KEY WORD");
      if(BE_NO_ERROR != (ret=StartProgram(aryArgs[0],priority))) {
         // Error--probably no such file.
         if(BE_CANT_OPEN_FILE==ret) {
            pConsole1->SetConsoleErrorLine("  ** KEYBOARD IGNORED: PROG NOT ON CODE PATH");
         }
      }
   } else if("DQ" == strCmd) {
      if(nArgs) {
         pConsole1->m_spo_lines_back = atoi(aryArgs[0]);
         if(pConsole1->m_spo_lines_back < SPO_MSGS_DISPLAYED) {
            pConsole1->m_spo_lines_back = SPO_MSGS_DISPLAYED;
         }
         if(pConsole1->m_spo_lines_back > MAX_SPO_MSGS) {
            pConsole1->m_spo_lines_back = MAX_SPO_MSGS;
         }
      } else {
         pConsole1->m_spo_lines_back = SPO_MSGS_DISPLAYED;
      }
      pConsole1->m_display_type = DISP_MSGS;
      pConsole1->DelayRefresh();
      pConsole1->UpdateScreen();
      pConsole1->StartScreenPaint();
   } else if("WY" == strCmd) {
      EnterCriticalSection(&critMix);
      CJob::SortJobs(arySortedMix);
      for(iline=SPO_IDX_FIRST; iline<SCREENROWS; iline++) {
         memset(pConsole1->m_Screen[iline], ' ', MAXLINELEN);
      }
      for(iline=SPO_IDX_FIRST, j=0; j<arySortedMix.GetSize() && iline<SCREENROWS; j++, iline++) {
         imix = (int) arySortedMix[j];
         strline.Format("P=%d M=%d %s=%-3.3d %s",
            Mix[imix].m_priority, Mix[imix].m_priority, 
            Mix[imix].m_progname, Mix[imix].m_mixnum, 
            Mix[imix].m_strStatus);
         memcpy(pConsole1->m_Screen[iline], strline, strline.GetLength());
      }
      LeaveCriticalSection(&critMix);
      pConsole1->m_display_type = DISP_COMMAND;
      pConsole1->DelayRefresh();
      pConsole1->UpdateScreen();
      pConsole1->StartScreenPaint();
   } else if(cmd_mixnum) {
      EchoCommandToSPO(strCmdLine);
      for(j=0; aryMixCommands[j]; j++) {
         if(aryMixCommands[j] == strCmd) {
            if(cmd_mixnum < Mix.GetSize() && Mix[cmd_mixnum].m_inuse) {
               bMixCmdOK = TRUE;
            }
         }
      }
      if(bMixCmdOK) {
         if("AX" == strCmd) {
            Mix[cmd_mixnum].m_odt_input = strAllArgs; 
            SetEvent(Mix[cmd_mixnum].m_hEventODT);
         } else if("PR" == strCmd) {
            Mix[cmd_mixnum].m_priority = atoi(aryArgs[0]);
            SetThreadPriority(Mix[cmd_mixnum].m_hthread, 
               UnisysToNTPriority(Mix[cmd_mixnum].m_priority));
         } else {
            EchoCommandToSPO(strCmdLine);
            pConsole1->SetConsoleErrorLine("  ** KEYBOARD IGNORED: MISSING KEY WORD");
         }
      } else {
         if(Debug) FmtDebug("No such mix number in '%s", strCmdLine);
         pConsole1->SetConsoleErrorLine("  ** KEYBOARD IGNORED: JOB INACTIVE");
      }
   } else {
      EchoCommandToSPO(strCmdLine);
      pConsole1->SetConsoleErrorLine("  ** KEYBOARD IGNORED: MISSING KEY WORD");
   }
   return retval;
}