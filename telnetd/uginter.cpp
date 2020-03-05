/*--- UGINTER.CPP -- functions to interface with UniGate code.
 *
 *  Copyright (C) 1997, Standard Networks, Inc.
 *  Mark Riordan   31 March 1996
 */
#include "stdafx.h"
#include "Conn.h"
#include "ugstuff.h"
#include "mdebug.h"
///#include "keyfuncs.h"
///#include "screen.h"
#include "ibmdef.h"
///#include "wrubasic.h"
//extern struct struct_ibm_screen_type *IBMScreenPointers[MAXSTATIONS+1];

extern int Debug;
extern int TD830W;

extern struct telnet_screen **TelScrBuffers;   
extern struct telnet_screen **telclt_buffers;
extern unsigned short telclt_nnormal;  /* telnet_client normal memory buffers */

extern struct telnet_screen_buffs *tel_screen_buffs;

extern unsigned char asc2ebc[];
extern unsigned char ebc2asc[];
///extern Stn_ref_entry *stn_ref;  // was 1+MAXCONNECTIONS
///extern Qwik_entry *qwik;  // originally MAXSTATIONS; was 1+MAXCONNECTIONS

extern int MaxConnections;
extern TypConn *Connection;

struct struct_ug_globals ug_globals;

uchar8  term_type_negot_seq_Any[TELNET_TERM_NEXT_INDEX] = {
  TELNET_TERM_UNKNOWN, TELNET_TERM_TD830,  TELNET_TERM_5250,
  TELNET_TERM_3278,    TELNET_TERM_GIVE_UP,TELNET_TERM_GIVE_UP,
  TELNET_TERM_GIVE_UP, TELNET_TERM_GIVE_UP};

uchar8  term_type_negot_seq_TD830[TELNET_TERM_NEXT_INDEX] = {
  TELNET_TERM_UNKNOWN, TELNET_TERM_TD830,  TELNET_TERM_GIVE_UP,
  TELNET_TERM_GIVE_UP, TELNET_TERM_GIVE_UP,TELNET_TERM_GIVE_UP,
  TELNET_TERM_GIVE_UP, TELNET_TERM_GIVE_UP};

uchar8  term_type_negot_seq_IBM3270[TELNET_TERM_NEXT_INDEX] = {
  TELNET_TERM_UNKNOWN, TELNET_TERM_3278,   TELNET_TERM_GIVE_UP,
  TELNET_TERM_GIVE_UP, TELNET_TERM_GIVE_UP,TELNET_TERM_GIVE_UP,
  TELNET_TERM_GIVE_UP, TELNET_TERM_GIVE_UP};

uchar8  term_type_negot_seq_IBM5250[TELNET_TERM_NEXT_INDEX] = {
  TELNET_TERM_UNKNOWN, TELNET_TERM_5250,   TELNET_TERM_GIVE_UP,
  TELNET_TERM_GIVE_UP, TELNET_TERM_GIVE_UP,TELNET_TERM_GIVE_UP,
  TELNET_TERM_GIVE_UP, TELNET_TERM_GIVE_UP};

/*-------------- Function prototypes -------------------------*/
void em_init_page(struct telnet_screen *tel_screen);
int UGInitializeBuffers(int maxconn);
int Init3270Module(void);
void init_ibm_screen (int ibm_stn_no);
void perform_ibm_key(int conn_index, int key);
void ConnSetLastXmitTime(TypConn *pConn);


/*-------------- Functions -----------------------------------*/

/*--- function InitUnigateStuff ----------------------------------------
 */
int
InitUnigateStuff(void)
{
   int retval=0;
	int conn_index;
	int cur_page;

   if(retval=UGInitializeBuffers(MaxConnections)){
      if(Debug) FmtDebug("InitUnigateStuff: UGInitializeBuffers ret %d",retval);
   }
   //nbuffs = initialize_buffers (start_telnet_screen,nbytes_telnet_screen,
   //                    start_tel_screen_bufs, nbytes_tel_screen_bufs);
   //nbuffs_telclt = initialize_telclt_buffers();
	//memset(tel_screen_buffs,0,nbytes_tel_screen_bufs);
		//sizeof tel_screen_buffs);

	//ConnectionLimit = min(nbuffs, nbuffs_telclt)-1;
	//ConnectionLimit = nbytes_tel_screen_bufs / sizeof(telnet_screen_buffs)-1;
	for(conn_index=0; conn_index<=MaxConnections; // Changed from <= MaxConnections
	 conn_index++) {
		tel_screen_buffs[conn_index].conn_id = conn_index;
		tel_screen_buffs[conn_index].stn_no = conn_index;
		for(cur_page=0; cur_page < 
			sizeof(tel_screen_buffs[0].buf_num) / 
			sizeof(tel_screen_buffs[0].buf_num[0]); cur_page++){
			tel_screen_buffs[conn_index].buf_num[cur_page] = -1;
		}
	}

   ///stn_ref = new Stn_ref_entry[MaxConnections+1];
   ///memset(stn_ref,0,sizeof(stn_ref[0])*(MaxConnections+1));

   ug_globals.vt_start_field = 0x4a;   // EBCDIC [
   ug_globals.vt_end_field = 0x5a;     // EBCDIC ]
	///initqwik();
   Init3270Module();

   return retval;
}

/*--- function InitOnePage ----------------------------------
 */
void InitOnePage(struct telnet_screen *tel_screen)
{
   memset(tel_screen,0,sizeof *tel_screen);
   memset(tel_screen->buffer,' ',sizeof tel_screen->buffer);
   memset(tel_screen->status_line,' ',sizeof tel_screen->status_line);
	em_init_page(tel_screen);
}


/*--- function LoadIBMScreen ----------------------------------
 *
 *  This function does for IBM emulation what the macros
 *  LOAD_TEL_SCREEN and load_screen_buf(type,num) do for TD830.
 */
struct telnet_screen *LoadIBMScreen(int conn_index)
{
   struct telnet_screen *ibm_screen;
   int cur_page = tel_screen_buffs[conn_index].cur_page; 
   int buf_num = tel_screen_buffs[conn_index].buf_num[cur_page];
   ibm_screen = TelScrBuffers[buf_num];
   return ibm_screen;
}

/*--- function UGInitConnection ------------------------------------------
 */
int
UGInitConnection(int conn_index, int termtype)
{
   int i;
   struct telnet_screen *tel_screen=0;
   int retval=0;
   int stsstruct = sizeof (struct telnet_screen);
   int sstructmsg = sizeof (struct message);
   int cur_page = 0;  // formerly MAX_PAGES;
   TypConn *pConn = &Connection[conn_index];

   if(pConn->ConnDebug>1)FmtDebug("UGInitConnection called for conn %d",conn_index);
   tel_screen_buffs[conn_index].cur_page = cur_page; // formerly 0;
   tel_screen_buffs[conn_index].full_xmit = TRUE;
   tel_screen_buffs[conn_index].term_state = TERMSTATE_RECV1;
   tel_screen_buffs[conn_index].bGotETXThisMsg = FALSE;
   tel_screen_buffs[conn_index].bEORPending = FALSE;
   tel_screen_buffs[conn_index].alt_delim = (unsigned char)ug_globals.UseAlternateDelimiters;

	// Make sure we've got at least the default screen page allocated.
	if(tel_screen_buffs[conn_index].buf_num[0] == -1) {
		if(!alloc_screen_buf (&tel_screen_buffs[conn_index].buf_type[0],
				(unsigned short *)&tel_screen_buffs[conn_index].buf_num[0])){
         if(pConn->ConnDebug)FmtDebug("UGInitConnection: cannot alloc_screen_buf for conn %d",conn_index);
      }
	}

   if (tel_screen_buffs[conn_index].buf_num[cur_page] == -1) {
      if(!telclt_alloc_screen_buf (&tel_screen_buffs[conn_index].buf_type[cur_page],
               (unsigned short *)&tel_screen_buffs[conn_index].buf_num[cur_page])) {
         if(pConn->ConnDebug)FmtDebug("UGInitConnection: cannot telclt_alloc_screen_buf for conn %d",conn_index);
         retval = 2;
      }
   }

   if(!retval) {
      telclt_load_screen_buf (tel_screen_buffs[conn_index].buf_type[cur_page],
                       tel_screen_buffs[conn_index].buf_num[cur_page]);
      if(!tel_screen) {
         retval = 3;
      } else {
         // memset(tel_screen,0,sizeof *tel_screen);

         /* First clear out tel_opts buffer */
         for (i = 0; i < NTELOPTS; i++)
            tel_screen->tel_opts[i] = 0;

         /* Now signify which options we will support. */
         tel_screen->tel_opts[TELOPT_BINARY] =
            TELOPT_SUPPORTED_IN | TELOPT_SUPPORTED_OUT;
         tel_screen->tel_opts[TELOPT_ECHO] =
            TELOPT_SUPPORTED_IN | TELOPT_SUPPORTED_OUT;
         tel_screen->tel_opts[TELOPT_SGA] =
            TELOPT_SUPPORTED_IN | TELOPT_SUPPORTED_OUT;
         tel_screen->tel_opts[TELOPT_SNDLOC] =
            TELOPT_SUPPORTED_OUT;
         tel_screen->tel_opts[TELOPT_TTYPE] =
            TELOPT_SUPPORTED_OUT;
         tel_screen->tel_opts[TELOPT_EOR] =
            TELOPT_SUPPORTED_IN | TELOPT_SUPPORTED_OUT;
         tel_screen->telnet_state = TELSTATE_DATA;

         /* Preset term_type */
         tel_screen_buffs[conn_index].term_type_num = TELNET_TERM_UNKNOWN;

         tel_screen_buffs[conn_index].term_type_negot_idx = 0;
         void *pary = tel_screen_buffs[conn_index].term_type_negot_seq;
         switch(termtype) {
         case WRUTERMTYPE_TD830:
            memcpy(pary,term_type_negot_seq_TD830, sizeof term_type_negot_seq_TD830);
            break;
         case WRUTERMTYPE_3278:
            memcpy(pary,term_type_negot_seq_IBM3270, sizeof term_type_negot_seq_IBM3270);
            break;
         case WRUTERMTYPE_5250:
            memcpy(pary,term_type_negot_seq_IBM5250, sizeof term_type_negot_seq_IBM5250);
            break;
         default:
            memcpy(pary,term_type_negot_seq_Any, sizeof term_type_negot_seq_Any);
            break;
         }

          tel_screen->help_file_fd = 0;

         store_screen_buf (tel_screen_buffs[conn_index].buf_type[cur_page],
                           tel_screen_buffs[conn_index].buf_num[cur_page]);
      }
   }
   init_ibm_screen(conn_index);
   return retval;
}

/*--- function GetUniPageAddr ----------------------------------
 *
 *  Return the screen buffer address for a connection.
 *
 *  Entry:  pConn    points to a session struct
 *
 *  Exit:   *scr_addr   points to the address of a 80*24
 *                      screen buffer in ASCII
 *          *status_line points to an 80-char status line
 *          Returns 0 if successful.
 */
int GetUniPageAddr(TypConn *pConn, char **scr_addr, char **status_addr)
{
   struct telnet_screen *tel_screen;
   int retval;
   int conn_index = pConn->iord;

   int cur_page = tel_screen_buffs[conn_index].cur_page;
   load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page],
                   tel_screen_buffs[conn_index].buf_num[cur_page]);

   *scr_addr = tel_screen->buffer;
   *status_addr = (char *)tel_screen->status_line;

   retval = !(*scr_addr && *status_addr);

   return retval;
}

/*--- function SendTCPPlusEOR ------------------------------------------
 */
void
SendTCPPlusEOR(int id, unsigned char *buf, int len)
{
#define LOCALBUFSIZE 12000
   char localbuf[LOCALBUFSIZE];
   int cpylen, done=FALSE;
   unsigned char telnet_eor[2] = {(unsigned char)TELNET_IAC, 
      (unsigned char)TELNET_EOR};
	SOCKET mysock = Connection[id].sock;

   do {
      cpylen = len > LOCALBUFSIZE ? LOCALBUFSIZE : len;
      len -= cpylen;
      memcpy(localbuf,buf,cpylen);
      buf += cpylen;
      if(cpylen < LOCALBUFSIZE-2) {
         memcpy(localbuf+cpylen,telnet_eor,sizeof telnet_eor);
         cpylen += sizeof telnet_eor;
         done = TRUE;
      }
   	send(mysock, localbuf, (int) cpylen, 0);
   } while(!done);
}

void
UGLogFromHost(short sn, unsigned char *buff, int len)
{
   TypConn *pConn = Connection+sn;
   if(pConn->logging) {
      LogTransmission(pConn,(char *)buff,(int) len,"[UGToEm]");
   }
}

/*--- function UGSetLocation -----------------------------------
 *
 *  Set the terminal location (station name) to be used by
 *  a connection.
 */
void UGSetLocation(int iconn, const char *location)
{
   ///strncpy((char *)qwik[iconn].stn_name,location,17);
}

/*--- function GetStationName ----------------------------------
 *
 *  Return the station name (location) for a connection number.
 */
char *GetStationName(int iconn)
{
   return Connection[iconn].term_loc;
}

/*--- function GetTermType -----------------------------------------
 */
int GetTermType(int stn) 
{
   int termtype = TELNET_TERM_UNKNOWN;
   if(stn) {
      termtype =  tel_screen_buffs[stn].term_type_num;
   }
   return termtype;
}

/*--- function IsUnisysTerm ----------------------------------
 */
BOOL IsUnisysTerm(int stn)
{
   return (TELNET_TERM_TD830 == GetTermType(stn));
}

