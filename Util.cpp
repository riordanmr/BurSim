// Util.cpp - Utilities for BurSim
//
// Mark Riordan  27 Feb 1999

#include "stdafx.h"

//=== Global Variables =============================================
extern int ScheduleTick;
extern char szLogFile[];


/*--- function MakeVersionStringForFile -------------------------------
 *
 *  Return an ASCII version of the Windows VERSION resource in 
 *  a given file.
 *
 *  Entry:  filename          is the name of a file with a version
 *                            resource (usually a .EXE or .DLL).
 *          pszVersionString  points to a buffer that's big enough
 *                            to hold the version number; 17 bytes
 *                            should always be enough.
 *  Exit:   pszVersionString  points to a zero-terminated version
 *                            string of the form w.x.y.z
 *                            If the resource could not be found, a
 *                            zero-length string is returned.
 */
void MakeVersionStringForFile(char *filename, char *pszVersionString)
{
   char * pszSubBlock = TEXT("\\"); 
   void *verbuf;
   DWORD dwhandle;
   DWORD dwvers_size;
   unsigned int uValLen;

   pszVersionString[0] = '\0';
   dwvers_size = GetFileVersionInfoSize(filename,&dwhandle);
   if(dwvers_size) {
      verbuf = malloc(dwvers_size);
      GetFileVersionInfo(filename,dwhandle,dwvers_size,verbuf);
      VS_FIXEDFILEINFO *pinfo;
      if(VerQueryValue(verbuf,pszSubBlock,(void **)&pinfo,&uValLen)) {
         wsprintf(pszVersionString,"%d.%d.%d.%d",
            0xffff & (pinfo->dwFileVersionMS>>16),
            0xffff & pinfo->dwFileVersionMS,
            0xffff & (pinfo->dwFileVersionLS>>16),
            0xffff & pinfo->dwFileVersionLS);
      }
      if(verbuf) free(verbuf);
   }
}

/*--- function MakeVersionString -------------------------------------
 *
 *  Return an ASCII version of the version of this executable,
 *  based on the Windows VERSION resource in the .exe.
 *
 *  Entry:  pszVersionString  points to a buffer that's big enough
 *                            to hold the version number; 17 bytes
 *                            should always be enough.
 *  Exit:   pszVersionString  points to a zero-terminated version
 *                            string of the form w.x.y.z
 *                            If the resource could not be found, a
 *                            zero-length string is returned.
 */
void MakeVersionString(char *pszVersionString)
{
   char filename[_MAX_PATH];
   // Get the application's version info from the executable and
   GetModuleFileName(NULL,filename,_MAX_PATH);
   MakeVersionStringForFile(filename,pszVersionString);
}

/*--- function RunHelp ------------------------------------------------
 *
 *  Bring up the help file for the current application.
 *  Assume the file is located in the same directory as the
 *  .EXE, and that its name ends in .HLP.
 *
 *  Entry:  hwndMain is the window handle for the app's main window.
 *          uCommand is the HELP command: HELP_CONTENTS, HELP_CONTEXT, etc.
 *          contextID is the help context ID
 *
 *  Exit:   Returns non-zero if succeeds, else FALSE.
 */
BOOL RunHelp(HWND hwndMain,UINT uCommand, DWORD contextID)
{
   char szFileName[_MAX_PATH];
   int retval = FALSE;

   GetModuleFileName(NULL, szFileName, _MAX_PATH);
   // szFileName now has full path to the executable.  Change it to
   // the full path to the help file.
   int len = strlen(szFileName);
   if(len>4) {
      strcpy(szFileName+len-3,"HLP");
      retval = ::WinHelp(hwndMain,szFileName,uCommand,contextID);
   }
   return retval;
}

/*--- function MyRand ---------------------------------------
 */
int MyRand(int mymax)
{
   return rand() % mymax;
}

/*--- function BlankPad ---------------------------------------
 */
CString BlankPad(const char *pszstr, int len)
{
   CString cs;
   char psztemp[1024], *cp;
   int mylen = strlen(pszstr);
   if(mylen > len) mylen=len;
   strncpy(psztemp,pszstr,mylen);
   for(cp=psztemp+mylen; cp-psztemp < len; cp++) *cp = ' ';
   psztemp[len] = '\0';
   cs = psztemp;
   return cs;
}

/*--- function ShortTime ------------------------------------------
 *
 *  Return a textual representation of the time.
 *
 *  Entry:  mytime   is the time
 *          
 *  Exit:   time_str points to a 14-byte (plus zero byte) ASCII time.
 */
void
ShortTime(time_t mytime, char *timestr) {
	strncpy(timestr,ctime(&mytime)+4,15);
	timestr[15] = '\0';
}

/*--- function FmtDebug ---------------------------------------
 */
void FmtDebug(const char *fmt,...)
{
   va_list argptr;
   char msg[12000], *ptarg, timestr[32];
   int len;

   static BOOL bInitialized=FALSE;
   CRITICAL_SECTION csDebug;
   if(!bInitialized) InitializeCriticalSection(&csDebug);
   EnterCriticalSection(&csDebug);

   ShortTime(time(NULL),timestr);

   len = wsprintf(msg,"%s%4d:",timestr,GetCurrentThreadId()); // ScheduleTick
   ptarg = msg+len;

   va_start (argptr, fmt);
   len = vsprintf (ptarg, fmt, argptr);
   va_end (argptr);

   FILE *stout = fopen(szLogFile,"a");
   if(stout) {
      fputs(msg,stout);
      fputs("\n",stout);
      fclose(stout);
   }
   LeaveCriticalSection(&csDebug);
}

/*--- function ConstructFilename -----------------------------------
 *  Constructs a full filename from a path and filename.
 *  Is intelligent about whether the given path ends in a 
 *  path separator character.
 */
CString ConstructFilename(CString cspath, CString csfile)
{
   CString csresult=cspath;
   if(csresult.GetLength()>0) {
      if('\\'!=csresult.Right(1) && '/'!= csresult.Right(1)) {
         csresult += '\\';
      }
   }
   csresult += csfile;
   return csresult;
}

BOOL matchbegi(const char *szpattern, const char *szstr_to_search)
{
   return 0==strnicmp(szpattern, szstr_to_search, strlen(szpattern));
}

void TrimTrailingSpaces(char *buf)
{
   char *cp = buf + strlen(buf) - 1;
   while(cp >= buf && ' '==*cp) cp--;
   *(cp+1) = '\0';
}