/*--- FkUG.CPP -- Simple program to emulate (fake) a UniGate
 *  doing TD830 emulation.  Used for testing when no real UniGate
 *  connected to a Unisys mainframe is available.
 *  This program reads logfiles created by TD830W and uses them
 *  as scripts in replying to terminals that connect to us.
 *
 *  Mark Riordan   14 April 1996
 */

#include "stdafx.h"
#include "FkUG.h"
#include "Conn.h"
#include "MDebug.h"
#include "hexbinpr.h"
#include "getoptpr.h"
#include "tel_def.h"
#include "IGWinUtl.h"

//=== Global Variables ================================================
extern int Debug;
int LogToDisk=0;
char *LogFileName=NULL;
extern unsigned char asc2ebc[256];
DWORD SecsBetweenScreens=0;
DWORD dwTimeoutMsFirstScreen = 800;
BOOL bDontSendBackCANDE=FALSE;
HANDLE hSocketNumberCopied;

extern char szListenIP[];
int TCPListenPort = 23;
SOCKET skListen;                        // Wait for connection requests

// Satisfy MDebug.cpp:
size_t MaxLogFileSize=1200000, CurLogFileSize;
char WRUSkeletonsPath[256];

///void HTMLOut::wr(char const * p) { int j=0; }

//=== Function Prototypes  ============================================
void
InitMain(void);
int SetTermTypeToNegotiate(char *szTermType);
void FmtStdout(char *fmt,...);


/*--- function InitTelnetd -----------------------------------
 *  Formerly main() of FkUG.
 */
int InitTelnetd()
{
   int retval =0,retcode;
   WSADATA WSAData;
   extern char    *myoptarg;
   char szVersion[64];
   DWORD dwThreadId, dwParam;
   HANDLE hTh;

   MakeVersionString(szVersion);

   if ((retcode = WSAStartup(MAKEWORD(1,1), &WSAData)) != 0) {
		perror("WSAStartup");
		retval=1;
	}

   if(!retval) {
      InitMain();
      hTh = CreateThread(
		       NULL,                        // no security attributes
		       0,                           // use default stack size
		       (LPTHREAD_START_ROUTINE)ListenThreadProc, // thread function
		       &dwParam,                    // argument to thread function
		       0,                           // use default creation flags
		       &dwThreadId                  // returns the thread identifier
		       );
      CloseHandle(hTh);
   }

   return retval;
}

// ListenThreadProc - Thread listens for connections
//
// Parameters:
//   none
//
// Return value:
//   none
//
// Comments:
//   Blocks waiting for a connection, and spawns a thread in ProcessThread
//   for each connection request.

void ListenThreadProc()
{
    DWORD dwThreadId, dwParam;
    SOCKET skOut;
    HANDLE hTh;
    
    hSocketNumberCopied = CreateEvent(NULL,  // pointer to security attributes
      FALSE,  // flag for manual-reset event
      FALSE, // flag for initial state
      NULL);     // pointer to event-object name
    if (!SetupListen()) {
       FmtDebug("Error setting up listening socket: %d\n",WSAGetLastError());
        ExitThread(0);              // Error, shutdown
    }
#ifdef USING_LISTEN_SELECT    
    fd_set fds;
	 int retcode;
    FD_ZERO(&fds);
    FD_SET(skListen, &fds);
    while ((retcode = select(0, &fds, NULL, NULL, NULL)) != 0) {
#else
    while(TRUE) {
#endif
		  // Someone has connected to us.
        // Create process thread to handle the session
       if(!AcceptConnection(&skOut)) {
          FmtDebug("Error: accept returned %d\n",WSAGetLastError());
       } else {
          dwParam = skOut;
#ifdef USING_LISTEN_SELECT
		    if(retcode != SOCKET_ERROR) {
#endif
			     hTh = CreateThread(
						    NULL,                        // no security attributes
						    0,                           // use default stack size
						    (LPTHREAD_START_ROUTINE)ProcessThreadProc, // thread function
						    &dwParam,                    // argument to thread function
						    0,                           // use default creation flags
						    &dwThreadId                  // returns the thread identifier
						    );
              // Wait until the thread has made its own copy of
              // the socket number.  Otherwise, we can wind up
              // overwriting it with a new one before the thread
              // makes a copy.
              WaitForSingleObject(hSocketNumberCopied,INFINITE);
              CloseHandle(hTh);
#ifdef USING_LISTEN_SELECT
			     FD_ZERO(&fds);
			     FD_SET(skListen, &fds);
		    } else {
			    break;  // Why break???
		    }
#endif
       }
     }  // while
    return;
}  // ListenThreadProc


// SetupListen - set a socket to listen for client connection requests
//
// Returns TRUE if all OK.

BOOL SetupListen()
{
    SOCKADDR_IN sin;
   
    skListen = INVALID_SOCKET;
    if ((skListen = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        //LogWSError();
        return(FALSE);
    }

    sin.sin_family = AF_INET;
    // If -a was specified, listen only on specified IP address.
    if(szListenIP[0]) {
       sin.sin_addr.s_addr = inet_addr(szListenIP);
    } else {
       sin.sin_addr.s_addr = INADDR_ANY;
    }
    sin.sin_port = htons(TCPListenPort);

    if (bind(skListen, (LPSOCKADDR) &sin, sizeof(sin))) {
        //LogWSError();
        return(FALSE);
    }

    if (listen(skListen, 100)) {
        //LogWSError();
        return(FALSE);
    }
    if(Debug) {
       char *pszaddr = "all IP addresses";
       if(szListenIP[0]) pszaddr = szListenIP;
       FmtDebug("Listening on %s at port %d", pszaddr, TCPListenPort);
    }
    return(TRUE);
}  // SetupListen


// ProcessThreadProc - Thread processes FkScrape requests
//
// Parameters:
//   lpdwParam - ???
//
// Return value:
//   none
//
// Comments:
//   ProcessThreadProc is spawned when a connection request is received.

DWORD ProcessThreadProc(LPDWORD lpdwParam)
{
    SOCKET skOut=*lpdwParam;            // Send output to client

    SetEvent(hSocketNumberCopied);
    //printf("ProcessThreadProc sock %d\n",skOut);
    if (skOut != INVALID_SOCKET) {
       Blocking(skOut);                // going synchronous now
       DoSession(skOut);
       closesocket(skOut);
    }
    return(1);
}  // ProcessThread


// AcceptConnection opens a connection to the client
//
// Parameters:
//   skOut - an open socket
//
// Return value:
//   TRUE upon successful accept, FALSE otherwise

BOOL AcceptConnection(SOCKET *skOut)
{
    struct sockaddr sad;
    int len = sizeof(sad);
	 BOOL isOK=TRUE;

    // Get connection request
    *skOut = accept(skListen, &sad, &len);
    if (*skOut == INVALID_SOCKET) {
        //LogWSError();
        isOK=FALSE;
    }

	 return isOK;
}  // AcceptConnection



// Blocking -- sets socket to block.
void Blocking(SOCKET s)
{
   u_long nonblock = FALSE;               // yes, we have no bananas

//   WSAAsyncSelect(s, hFrame, 0, 0);       // turn off message notifications
   ioctlsocket(s, FIONBIO, &nonblock);    // set socket to blocking
}  // Blocking


void
SendszASC(SOCKET sock,const char *str)
{
   int len =strlen(str), j;
   unsigned char *streb = (unsigned char *)malloc(len);

   for(j=0; j<len; j++) {
      streb[j] = asc2ebc[(unsigned char)str[j]];
   }
   SendBuf(sock,streb,len);
   free(streb);
}

void
SendBuf(SOCKET sock, unsigned char *buf, int len)
{
   int flags=0;
   unsigned char mybuf[12000];

   if(SecsBetweenScreens) {
      Sleep(1000*SecsBetweenScreens);
   }
   if(len > sizeof mybuf) {
      FmtDebug("Error: len of %d too long in SendBuf\n",len);
   }
   memcpy(mybuf,buf,len);
   mybuf[len] = TELNET_IAC;
   mybuf[len+1] = TELNET_EOR;
   if(Debug>1) FmtDebug("Sending %d bytes",len);
	if(Debug>2)Dout("Sending",(char *)mybuf,len+2);
   send(sock,(char *)mybuf,len+2,flags);
}

/*--- function begmatch --------------------------------------------
 *
 *  Check whether a string begins with a given initial substring
 *
 *  Entry:  pattern  points to the desired initial substring
 *          target   is the string we want to check.
 *
 *  Exit:   Returns True if a case-sensitive match, else FALSE.
 */
int
begmatch(char *pattern, char *target)
{
   while(*pattern && *pattern == *target) {
      pattern++; target++;
   }
   return (*pattern == 0);
}

