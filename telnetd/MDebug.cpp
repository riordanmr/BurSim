/*--- MDebug.cpp -- Debug routines 
 *
 * Copyright (C) 1997, Standard Networks, Inc.
 * Written by Mark Riordan
 */
#include "stdafx.h"
#include <mmsystem.h>
#include "hexbinpr.h"
//#include "FldAttr.h"
#include "IGWinUtl.h"
#include "conn.h"

//=== Global Variables ==============================================
extern int Debug;
extern char computername[];
extern struct telnet_screen_buffs *tel_screen_buffs;
extern struct telnet_screen **TelScrBuffers;   
extern TypConn *Connection;
extern unsigned char ebc2asc[256];
extern char LogFile[_MAX_PATH];
extern FILE *stLogFile;
extern BOOL bReOpenLogFile;
extern BOOL bLogFileDirty;
extern CRITICAL_SECTION csLogFile;
extern size_t MaxLogFileSize, CurLogFileSize;
extern char WRUSkeletonsPath[];


//=== Function prototypes ===========================================



/*--- function MDebug  --------------------------------------------
 *  
 *  Write a message to a log file if debugging is enabled.
 */
void
MDebug(const char *msg)
{
   if(Debug) {
      FmtDebug("%s",msg);
   }
}

/*--- function CloseLogFile -----------------------------------
 *
 *  Close the logfile.  This flushes the file.  It also helps
 *  programs (like text editors that have the file open) notice
 *  that the file has changed.
 *
 *  Entry:  csLogFile      is the critical section that guards the file.
 *          bLogFileDirty
 */
void CloseLogFile()
{
   EnterCriticalSection(&csLogFile);
   bLogFileDirty=FALSE;
   bReOpenLogFile=TRUE;
   if(stLogFile) fclose(stLogFile);
   stLogFile = NULL;
   LeaveCriticalSection(&csLogFile);
}

/*--- function RenameLogFile ---------------------------------------
 *
 *  Close and rename the current logfile.
 * 
 *  Exit:	csnewname	is the new name of the file, which we
 *								have thought up.
 *				Returns 0 upon success.
 */
int RenameLogFile(CString &csnewname)
{
	int retval=0;
	char *lpLast, *p=LogFile;
	char mbuf[_MAX_PATH];

	EnterCriticalSection(&csLogFile);

	// Create the new logfile name by taking the old logfile name,
	// stripping the extension, if any, and adding the date and time
	// plus .Log.
	// We want to set p to the addr of the last '.' in the filename.

   /* find the last directory separator */
   lpLast = NULL;
   while (*p) {
      if (*p == '\\' || *p == '/') lpLast = p;
      p++;
   }

   /* find the last period in the filename portion */
   p = lpLast ? lpLast : LogFile;
   while (*p && *p != '.') p++;
	// If no period, then set p to the end of the filename.
	if(p == lpLast) p = LogFile +strlen(LogFile);

	strncpy(mbuf,LogFile,p-LogFile);
	mbuf[p-LogFile] = '\0';
	CTime mytime = CTime::GetCurrentTime();
	csnewname = mbuf + mytime.Format("-%Y%m%d-%H%M%S.Log");
   CloseLogFile();
	retval=rename(LogFile,csnewname);
	bReOpenLogFile = TRUE;
	LeaveCriticalSection(&csLogFile);

	return retval;
}

/*--- function DoutHex -----------------------------------
 *
 *  Log a buffer to the debug file in hex.
 *
 *  Entry:  msg      is a descriptive string.
 *          buf      is the buffer.
 *          nbytes   is the number of bytes.
 */
void DoutHex(char *msg, char *buf, int nbytes)
{
   int j;
   char mybuf[200],*bptr=mybuf;

   FmtDebug("%s %d bytes:",msg,nbytes);
   bptr = mybuf;
   for(j=0; j<nbytes; j++) {
      sprintf(bptr,"%-2.2x ",0xff&buf[j]);
      bptr += strlen(bptr);
      if((j+1)%16 == 0) {
         MDebug(mybuf);
         bptr = mybuf;
         mybuf[0] = '\0';
      }
   }
   if(mybuf[0]) MDebug(mybuf);
}

/*--- function Dout -----------------------------------
 *
 *  Log a buffer to the debug file.  The buffer is logged
 *  in both hex and EBCDIC-translated-to-ASCII.
 *
 *  Entry:  msg      is a descriptive string.
 *          buf      is the buffer.
 *          nbytes   is the number of bytes.
 */
void Dout(char *msg,char *buf, int nbytes)
{
   char mybuf[20000],*bptr=mybuf;
   int j;

   DoutHex(msg,buf,nbytes);

   if(nbytes >= sizeof(mybuf)-4) nbytes = sizeof(mybuf)-4;

   *(bptr++) = '[';
   for(j=0; j<nbytes; j++,bptr++) {
      *(bptr) = (char)ebc2asc[(unsigned char)buf[j]];
      if(!*bptr) *bptr = ' ';
   }
   *(bptr++) = ']';
   *(bptr++)='\0';
   MDebug(mybuf);
}

/*--- function DoutAsc -----------------------------------
 *
 *  Log a buffer to the debug file.  The buffer is logged
 *  in both hex ASCII.
 *
 *  Entry:  msg      is a descriptive string.
 *          buf      is the buffer.
 *          nbytes   is the number of bytes.
 */
void DoutAsc(char *msg,char *buf, int nbytes)
{
   char mybuf[20000],*bptr=mybuf;
   int j;

   DoutHex(msg,buf,nbytes);

   if(nbytes >= sizeof(mybuf)-4) nbytes = sizeof(mybuf)-4;

   *(bptr++) = '[';
   for(j=0; j<nbytes; j++,bptr++) {
      *(bptr) = buf[j];
      if(!*bptr) *bptr = ' ';
   }
   *(bptr++) = ']';
   *(bptr++)='\0';
   MDebug(mybuf);
}

/*--- function WriteToDebugFile -----------------------------------
 *
 *  Write a message to a separate debug file.
 *
 *  Entry:  szline1  is the first line of the message.
 *          sztext   is subsequent lines.
 *          len      is the lengt hof sztext.
 *
 *  Exit:   The message has been written to c:\HEATDbg.txt
 */
void WriteToDebugFile(const char *szline1, const char *sztext, int len)
{
   static char *filename = "C:\\HEATDbg.TXT";
   FILE *stdebug = fopen(filename,"a");
   time_t mytime;

   if(stdebug) {
      time(&mytime);
      fprintf(stdebug,"Debug trace taken at %s",ctime(&mytime));
      fprintf(stdebug,"%s\n",szline1);
      fwrite(sztext,len,1,stdebug);
      fprintf(stdebug,"*** End ***\n\n");
      fclose(stdebug);
   }
}

/*--- function LogTransmission ---------------------------------------------
 *
 * Log the receipt or sending of a block of data.
 *
 *  Entry:  pConn    points to a TypConn structure
 *          buf         points to a buffer of bytes
 *          nbytes      is the number of bytes to log
 *          label       points to a zero-terminated string that
 *                      describes this transmission.
 *                      "UGToEm" indicates Unigate to emulator
 *                      "EmToUG" indicates emulator to Unigate
 */
void
LogTransmission(TypConn *pConn,char *buf,int nbytes,char *label)
{
#define BYTESPERLINE 16
   FILE *logstream=pConn->stlog;
   char line[200], *cp, ch;
   int bytes_in_line,j;

   if(logstream) {
      fwrite(label,strlen(label),1,logstream);
      fputc('\n',logstream);
      fprintf(logstream,"Len=%d\n",nbytes);
      fprintf(logstream,"Data=\n");
      while(nbytes) {
         bytes_in_line = nbytes < BYTESPERLINE ? nbytes : BYTESPERLINE;
         BinToHex((unsigned char *)buf,bytes_in_line," ",line);
         cp = line +strlen(line);
         *(cp++) = ' ';
         *(cp++) = '|';
         for(j=0; j<bytes_in_line; j++) {
            ch = ebc2asc[(unsigned char)buf[j]];
            if(ch < 0x20 || ch > 0x7f) ch = '.';
            *(cp++) = ch;
         }
         *(cp++) = '|';
         *(cp++) = '\n';
         *(cp++) = '\0';
         fwrite(line,strlen(line),1,logstream);
         nbytes -= bytes_in_line;
         buf += bytes_in_line;
      }
   }
}

