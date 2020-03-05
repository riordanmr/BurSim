/*--- FkUGFunc.cpp -- Functions for FkUG.cpp ---------------------------
 */
#include "stdafx.h"
#include "FkUG.h"
#include "Conn.h"
#include "MDebug.h"
#include "ugstuff.h"
#include "hexbinpr.h"
#include "getoptpr.h"
#include "tel_def.h"
#include "Station.h"

#define RDBUFSIZE 8192

//===  Global Variables  =============================================
extern int Debug;
extern int LogToDisk;
extern char *LogFileName;
extern DWORD dwTimeoutMsFirstScreen;
extern unsigned char orig_ebc2asc[256];
extern unsigned char orig_asc2ebc[256];
unsigned char asc2ebc[256];
unsigned char ebc2asc[256];
extern struct telnet_screen_buffs *tel_screen_buffs;
extern struct telnet_screen **TelScrBuffers;   
extern unsigned short nnormal;  /* normal memory buffers */
CStationTable StationTable;
int MaxConnections = 512;
BOOL bForceNewEntry = TRUE;  // TRUE if every new [xxx] section in the input file
                             // should result in a new entry in the logfile structure.

extern CArray<CJob, CJob &> Mix;
extern CRITICAL_SECTION critMix;
extern BOOL bShowWelcome;

TypConn *Connection;
char *computername;

char LogFile[_MAX_PATH];
FILE *stLogFile;
BOOL bReOpenLogFile=TRUE;
BOOL bLogFileDirty = FALSE;
CRITICAL_SECTION csLogFile;
CRITICAL_SECTION csalloc_screen_buf;
CRITICAL_SECTION critConns;
CRITICAL_SECTION critConsole;

//=== Function Prototypes  ===========================================
int telnet_server
   (int16 id,                   /* TCB id */
       int16 sig,               /* TCP signal */
       uint8 * txt,             /* Data buffer */
       int16 len,               /* Amount of 'good' data in buffer */
       int16 flags);             /* Flags passed with data */
void
tel_server_close_conn(int conn_index);
int InitUnigateStuff(void);
void ug_get_termloc(int conn_index, char *buf);

/*--- function FmtStdout -----------------------------------
 *
 *  Acts like printf(), but prepends date and time and appends newline.
 */
void FmtStdout(char *fmt,...) {
   va_list argptr;
   char msg[12000], timestr[32];

	EnterCriticalSection(&critConsole);
	ShortTime(time(NULL),timestr);
   va_start (argptr, fmt);
   vsprintf (msg, fmt, argptr);
   va_end (argptr);
   fputs(timestr,stdout);
   fputs(":",stdout);
   puts(msg);
	LeaveCriticalSection(&critConsole);
}

void DoutEBC(char *title,unsigned char *buf, int nbytes)
{
   int j, iout;
   char szbuf[2500];

   strcpy(szbuf,title);
   iout = strlen(szbuf);
   szbuf[iout++] = '[';
   for(j=0; j<nbytes; j++) {
      szbuf[iout++] = orig_ebc2asc[buf[j]];
   }
   szbuf[iout++] = ']';
   szbuf[iout] = '\0';
   FmtStdout("%s",szbuf);
}

void DoutASC(unsigned char *buf, int nbytes)
{
   int iout=0, j;
   char szbuf[2500];

   szbuf[iout++] = '\0';
   szbuf[iout++] = '{';
   for(j=0; j<nbytes; j++) {
      szbuf[iout++] = buf[j];
   }
   szbuf[iout++] = '}';
   szbuf[iout] = '\0';
   FmtDebug("%s",szbuf);
}

void DoutHex(char *msg,unsigned char *buf, int nbytes)
{
   int j;
   char mybuf[20000],*bptr=mybuf;

      sprintf(mybuf,"%s %d bytes:",msg,nbytes);
      bptr += strlen(bptr);
      for(j=0; j<nbytes; j++) {
         sprintf(bptr,"%-2.2x ",0xff&buf[j]);
         bptr += strlen(bptr);
         if((j+1)%20 == 0) {
            FmtDebug("%s",mybuf);
            bptr = mybuf;
            mybuf[0] = '\0';
         }
      }
      if(mybuf[0]) FmtDebug("%s",mybuf);
      bptr = mybuf;
      *(bptr++) = '[';
      for(j=0; j<nbytes; j++,bptr++) {
         //*(bptr) = (char)ebc2asc[(unsigned char)buf[j]];
         *(bptr) = buf[j];
         if(!*bptr) *bptr = ' ';
      }
      *(bptr++) = ']';
      *(bptr++)='\0';
      FmtDebug("%s",mybuf);
}


/*--- function InitCritSecs -----------------------------------
 */
void InitCritSecs()
{
   InitializeCriticalSection(&csLogFile);
   InitializeCriticalSection(&csalloc_screen_buf);
   InitializeCriticalSection(&critConns);
   InitializeCriticalSection(&critConsole);
}

/*--- function InitMain ---------------------------------------------
 */
void
InitMain() {
   int j;
#define CNAMESIZE 80
   computername = (char *)malloc(CNAMESIZE);
   DWORD csize=CNAMESIZE;
   GetComputerName(computername,&csize);

	strcpy(LogFile,"\\FkUG.Log");
   InitCritSecs();
   InitUnigateStuff();
   for(j=0; j<256; j++) {
      asc2ebc[j] = orig_asc2ebc[j];
      ebc2asc[j] = orig_ebc2asc[j];
   }

   int nConnectionBytes = (sizeof(TypConn)+8)*MaxConnections;
   Connection= (struct struct_conn *) malloc(nConnectionBytes);
   memset(Connection,0,nConnectionBytes);

   UGInitAllInbound();
   StationTable.Init("Stations.csv");
}

/*--- function DoInitSession  ----------------------------------------
 */
int
DoInitSession(int &conn_index,SOCKET sock)
{
   int retval=0;
   TypConn *pConn;
   char msg[256];

   EnterCriticalSection(&critConns);
   for (conn_index=1; conn_index<MaxConnections; conn_index++)
	    if ( (tel_screen_buffs[conn_index].stn_no == 0) &&
	    (tel_screen_buffs[conn_index].conn_id == -1) )
	   break;

   if (conn_index >= MaxConnections) {
      wsprintf(msg,"Exceeded session limit");
      FmtDebug(msg);
	   if(LogToDisk)MDebug(msg);
      retval=1;
   } else {
      pConn = Connection + conn_index;
      pConn->iord = conn_index;
      pConn->sock = sock;
      pConn->ConnDebug = Debug;
      pConn->current_program[0] = '\0';

      tel_screen_buffs[conn_index].stn_no = conn_index;
      tel_screen_buffs[conn_index].conn_id = conn_index;
   }
   LeaveCriticalSection(&critConns);
   if(!retval) {
      UGInitConnection(conn_index,WRUTERMTYPE_TD830);
      UGInitInboundConnection(conn_index);
      // Send initial TELNET_IAC DO TERMINAL_TYPE
      //unsigned char do_type[] = {TELNET_IAC, TELNET_DO, TELOPT_TTYPE};
      //tcp_send(conn_index,do_type,sizeof do_type,0); 
   }
   return retval;
}

/*--- function TransToProgram -----------------------------------
 */
CString TransToProgram(CString strTrans)
{
   return strTrans;
}

/*--- function FindMixForProgram -----------------------------------
 */
int FindMixForProgram(CString strProgram)
{
   int mix=0, j;
   BOOL bFound=FALSE;
   for(j=FIRST_ASSIGNABLE_MIX_NUM; !bFound && j<Mix.GetSize(); j++) {
      if(Mix[j].m_inuse && Mix[j].m_progname == strProgram) {
         mix = j;
         bFound=TRUE;
      }
   }
   return mix;
}

/*--- function DoTransaction  --------------------------------------------------
 *
 *  Send a transaction to a program, for a given station.
 *
 *  
 *	 Entry:	pConn    points to a connection structure.
 *          sock		is a socket
 *				asciibuf is a buffer containing the transaction, in ASCII..
 *				nbytes	is the number of bytes in the buffer.
 */
void DoTransaction(TypConn *pConn, SOCKET sock, char *asciibuf, int nbytes)
{
   CString strProgram;
   int mixnum=0;
   BOOL bExplictTrans=FALSE;
   CString strBuf = CString(asciibuf,nbytes);
   CString strHead, strTrans;
   strHead.Format("%s%-6.6d %-18.18s",MSG_HEADER_PREFIX,pConn->iord,pConn->term_loc);
   strHead += CString(' ',MSG_HEADER_LEN-strHead.GetLength());

   if('/'==asciibuf[0]) {
      bExplictTrans = TRUE;
      strTrans = strBuf.Mid(1).SpanIncluding("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_");
      strProgram = TransToProgram(strTrans);
   } else {
      strProgram = pConn->current_program;
   }
   if(strProgram.GetLength() && (mixnum = FindMixForProgram(strProgram)) ) {
      strncpy(pConn->current_program, strProgram, sizeof(pConn->current_program)-1);
      // Put the message in the queue to the program.
      if(Debug>1) FmtDebug("Queuing %d bytes to mix %d, program '%s'", 
         nbytes, mixnum, strProgram);
      // By convention, use a STOQ name the same as the program 
      CSTOQ *pSTOQ = CSTOQ::GetSTOQPtr(strProgram);
      pSTOQ->AddTail(strHead + strBuf);
   } else {
      // Error--no transaction
      pConn->current_program[0] = '\0';
      CString strErrmsg = "Error--there is no current transaction.  Enter a transaction starting with /";
      if(bExplictTrans) strErrmsg.Format("Error--there is no transaction \"%s\".",strTrans);
      SendszASC(sock, "\x1bW\x1b\" \"" + strErrmsg + T27TRAILER);
   }
}

/*--- function ProcReadBuf -----------------------------------------------------
 *
 *  Respond to a buffer of bytes just received from the net.
 *  
 *	 Entry:	conn_index  is the connection index.
 *          sock		is a socket
 *				buf		is the buffer we just received, presumably in EBCDIC.
 *				nbytes	is the number of bytes we just received
 *  
 *  Exit:	
 */

int ProcReadBuf(int conn_index, SOCKET sock, char *buf, int nbytes)
{
   int retval=0,j;
   char asciibuf[8192];
   TypConn *pConn = &Connection[conn_index];

   if(Debug>2) Dout("ProcReadBuf rec:",(char *)buf,nbytes);
   for(j=0; j<nbytes; j++) translate(ebc2asc, buf+j, asciibuf+j);
   asciibuf[nbytes] = '\0';

   DoTransaction(pConn, sock, asciibuf, nbytes);

   return retval;
}


/*--- function DoSession ---------------------------------------------
 *  
 *  Someone has just connected to us.  This function runs the resulting session 
 *  based on the logfile and the responses from the client who is connected.
 */
void DoSession(SOCKET sock)
{

   fd_set rfds;
   struct timeval timeout;
   int flags=0, nbytesread, totbytes=0, selcode, done=FALSE, bufpos=0;
   int retcode=0;
	unsigned char buf[RDBUFSIZE];
   char msg[200];
   int conn_index, sent_first_screen=FALSE;
   CStation station;
   DWORD threadID = GetCurrentThreadId();

   if(DoInitSession(conn_index,sock)) {
      wsprintf(msg,"** Can't initialize session");
      FmtDebug(msg);
		return;
	}
   TypConn *pConn = Connection + conn_index;
   if(retcode) {
	   SendszASC(sock,"Sorry, can't find script file.\xff\xef");
	}
   if(Debug) {
      FmtDebug("Accepted conn %d on socket %d",conn_index,sock);
   }

	// Loop, reading input from the net and responding appropriately based
	// on the logfile
	do {
      FD_ZERO(&rfds);
      FD_SET(sock,&rfds);
		if(sent_first_screen) {
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;
		} else {
         // Don't wait too long if we haven't sent the first screen yet.
         // For a while, this code was disabled by ORing TRUE into above if.
         timeout.tv_sec = dwTimeoutMsFirstScreen/1000;   // ms to seconds
         timeout.tv_usec = dwTimeoutMsFirstScreen*1000;  // ms to microseconds
		}
		selcode = select(1,&rfds, NULL, NULL, &timeout);
      if(conn_index >= MaxConnections) {
         FmtDebug("** Error: conn_index is %d\n",conn_index);
         done = TRUE;
      }
		if(!selcode) {
			// timeout
         if(!sent_first_screen) {
            ug_get_termloc(conn_index, pConn->term_loc);
            if(bShowWelcome) {
	            // Start by sending the welcome screen
               char msg[1000];
               char time_str[32];
               time_t mytime;
               time(&mytime);
               memcpy(time_str, ctime(&mytime), 24);
               time_str[24] = '\0';
               wsprintf(msg,"\x1bX\x0c"
                  "                                                                                "
                  "                                                                                "
                  "Welcome to BurSim!                                                              "
                  "                                                                                "
                  "Connected at %s                                           "
                  "                                                                                "
                  "You have been assigned station %-3.3d"
                  "\x1bW\x03",
                   time_str,conn_index);
               SendszASC(sock,msg);
            }
            if(StationTable.FindStation(pConn->term_loc, station)) {
               // We found a station entry for this station.
               DoTransaction(pConn,sock,station.st_initial_tran,strlen(station.st_initial_tran));
            }
            sent_first_screen = TRUE;
         }
		} 
		if(FD_ISSET(sock,&rfds)) {
			// Read what the client has sent
			nbytesread = recv(sock, (char *)buf+totbytes, RDBUFSIZE-totbytes, flags);
			if(nbytesread <= 0) { 
            if(Debug) {
               if(nbytesread) {
                  wsprintf(msg,"FkUG: recv on conn %d sock %d GetLastError=%d",
                  conn_index,sock,GetLastError());
                  FmtDebug("%s",msg);
               }
            }
				done=TRUE;
			} else {
            if(Debug>2) {
               wsprintf(msg,"Recv %d bytes: ",nbytesread);
               DoutEBC(msg,buf+totbytes,nbytesread);
               if(Debug>3) {
                  DoutHex("In hex",buf+totbytes,nbytesread);
                  DoutASC(buf+totbytes,nbytesread);
               }
            }
				// Process (respond appropriately to) the buffer of bytes we just got.
            totbytes = 0;
				if(Debug>2)Dout("Rec",(char *)buf+totbytes,nbytesread);
				totbytes += nbytesread;
            if(telnet_server(conn_index,SG_DATA,buf,totbytes,0)) {
               struct telnet_screen *tel_screen;
               int cur_page = tel_screen_buffs[conn_index].cur_page;  
               load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page], 
                               tel_screen_buffs[conn_index].buf_num[cur_page]);
               char *bptr = tel_screen->buffer;
               int nbytes = tel_screen->scr_pos;
				   ProcReadBuf(conn_index,sock,bptr,nbytes);
               tel_screen->scr_pos=0;
            }
            totbytes = 0;
			}
      }
   } while(!done);

   // Mark entry as available.  Socket is closed by our caller.
   // I don't think we really need a critical section here.
   EnterCriticalSection(&critConns);
   if(Debug) {
      FmtDebug("Closing conn %d sock %d", conn_index, sock);
   }
   tel_server_close_conn(conn_index);
   LeaveCriticalSection(&critConns);
}  

/*--- function void em_send_input_to_srv ----------------------------------
 *
 *  Process a transmission received from a terminal, usually from the
 *  user hitting XMIT.
 *  We return an appropriate response, usually a screen image.
 *
 *  Entry:  conn_index  is an index into tel_screen_buffs giving
 *                      the connection whose accumulated input characters
 *                      are to be processed.  
 *
 */
void em_send_input_to_srv(int conn_index)
{
   struct telnet_screen *tel_screen;  
   
   int cur_page = tel_screen_buffs[conn_index].cur_page;  
   load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page], 
                   tel_screen_buffs[conn_index].buf_num[cur_page]);
}

/*--- function ConnSetLastXmitTime -----------------------------------
 */
void ConnSetLastXmitTime(struct struct_conn *)
{
}
