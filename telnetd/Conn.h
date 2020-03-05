/*--- Conn.h -- Connection-related structures and so on.
 */
#ifndef INCLUDED_CONN_H
#define INCLUDED_CONN_H
///#include "htmlout.h"
///#include "varinput.h"

#define BUFSIZE 200
#define WRUPORT 3344

#define MAXSESSION 100
#ifdef IN_TD830W
#define MAXCONNECTIONS 12
#else 
#define MAXCONNECTIONS 120
#endif

#define MAXCOL 80
#define MAXROW 24
#define SESSIONIDBYTES      6
#define SESSIONIDBYTESALLOC 12
#define MAXHOSTNAME 80
#define MAXTERMLOCATION 20
#define MAXPOOLNAME     32
#define SOCKBUFSIZE     2048
#define MAXSTATIONNAME	24
// enum_buttons_pos describes the position of buttons output in HTML.
enum enum_buttons_pos {BUTTONS_TOP, BUTTONS_BOTTOM};

// enum_inuse describes the state of a connection slot.
// INUSE_NO means that the connection & slot are available.
// INUSE_DYING means that the connection has gone away, but 
//     the session that owns the connection doesn't know it,
//     so we shouldn't reallocate this connection until the
//     owning session goes away or explicitly closes the conn.
// INUSE_YES means that the connection is in use (though not
//     nec. owned by a session) and as far as we know, is
//     still connected.
enum enum_inuse {INUSE_NO, INUSE_DYING, INUSE_YES};
typedef struct struct_conn {
   int         ConnDebug;     // Connection-specific copy of session's Debug flag
   enum enum_inuse   inuse;  // whether this slot is in use.
   char        *screen;  // Pointer to the current screen
   char        *status_line;  // Pointer to screen status line
	char			hostname[MAXHOSTNAME];
   int         wru_termtype;  // WRUTERM_xxxxx constant for terminal type
	int			port;
   int         isess;    // Index of owning session, or 0 if none.
   char        pool[MAXPOOLNAME];
   char        term_loc[MAXTERMLOCATION];
	char        inbuf[SOCKBUFSIZE];
   char        current_program[40];
	int			rembytes;   // bytes remaining in inbuf
   fd_set      readfds;
	fd_set      writefds;
	fd_set      exceptfds;
	struct      timeval seltimeout;
	time_t		timestarted;
	time_t		timelastxmit;
   long int 	sesswaitsec;
   long int    talkwaitms;
	SOCKET      sock;
	int         iord;       // Connection number
   int         connected;
   int         screensgotten;
   int         telnet_negot_done;
	int			lasterr;        // WSAGetLastError
   unsigned long hthread;
   DWORD       threadid;
   OVERLAPPED  Overlapped;
   HWND        hWndToUpdate;
   long int    nscreensrec; // # of screens received on this connection
   long int		bytesrec; // # of bytes received on this connection 
   int         logging;  // TRUE if logging session
   FILE       *stlog;    // stream to log session to TalkToMainframe
   ///struct message ugmsg; // Message structure from UniGate, used to store
                         // decoded input from host, with Telnet escape chars
                         // already interpreted.
   enum enum_buttons_pos  m_AutoformButtonPos;
                           // Position of Submit buttons on HTML returned
                           // by AutoForms.
   BOOL        m_bAutoformForceUpper;  // TRUE if autoform mode should
                                       // force user input to uppercase.
   BOOL        m_bAutoformJavascript;  // TRUE if autoform mode should 
                           // output HTML to cooperate with JavaScript.
   BOOL        m_bAutoformWaitForFirstOnly;// TRUE if autoform mode should 
                           // wait only for the first screen from the host.
   BOOL        m_bAutoformSubmitStaleOK;  // TRUE if it's OK for an AutoForm
                           // user to send a screen from a web page that is
                           // not the most recently-sent.
   BOOL        m_bAutoformCmdLineAlways;  // TRUE if Autoform should always
                           // put out a command line--not just when there
                           // are no fields.
   int         m_AutoformMinPrintableBytesForFirstOnly;  // If we get < this number of
                           // printable bytes from the host when m_bAutoformWaitForFirstOnly 
                           // is True, keep reading from the host--don't count this as a message.
   int         m_nPrintableBytesRec;      // number of displayable bytes received
                           // so far on this connection.  Used by Autoform.
   void       *pScreenDB;  // pointer to current screen database, else 0.
                           // Has type TypScreenDBInfo.
   char        szcurscreen[44];  // name of current screen if using
                                 // screens database.
   char        szcurscreenUpper[44];  // szcurscreen, in upper case.
   BOOL        bUseGlobalIfNotFound;   // When searching for a field in 
                           // screens DB, use _GLOBAL screen if not found.
   BOOL        bDoCandeEditing;  // Edit output to terminal as per CANDE.
} TypConn;

struct BareConnection {
	char		hostname[MAXHOSTNAME];
   int      bare_termtype;  // WRUTERM_xxxxx constant for terminal type
	int 		port;
	char		stnname[MAXSTATIONNAME];
};



//typedef enum enum_type_readret {RSuccess, RNoBytes, RWrongSock,
//  RTimeout,RRecvError,RWaitLocal} TypReadRet;
typedef int TypReadRet;
#define RSuccess		0
#define RNoBytes		1
#define RWrongSock	2
#define RTimeout		3
#define RRecvError	4
#define RWaitLocal	5


/*--- Function prototypes ---------------------------------------------- */
BOOL   WINAPI   
DllMain (HANDLE hInst, 
         ULONG ul_reason_for_call,
         LPVOID lpReserved);
int GetConnectionSlot(int *isess);
void
InitConnectionStruct(TypConn *pConn) ;
//void SignalEndLocalMode(TypConn *pConn,BOOL bReset);
///void SetDebugForAllConnsForSession(TypWSession *pWSess, int newDebug);

TypReadRet
TalkToMainframe(TypConn *pConn, char *buf, int &rembytes);

void
ShortTime(time_t mytime, char *timestr);

void 
FormatConnection(TypConn *pConn, char *mbuf);
int
StartConnection(int isess);
void
CleanupConn(TypConn *pConn, BOOL bRegardless, enum_inuse new_inuse);
void
LogTransmission(TypConn *pConn,char *buf,int nbytes,char *label);
TypReadRet
GetASCIIBuf(TypConn *pConn, int ntimeoutsecs, char *buf, int maxbuf, 
            BOOL bLineDelimit, int &bytes_ret);

#endif