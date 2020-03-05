/*--- UGSTUB.CPP -- Stubbedout routines, and misc
 * variable declarations needed by routines taken from
 * UniGate.
 *
 * Copyright (C) 1997, Standard Networks, Inc.
 * Mark Riordan   29 March 1996
 */

#include "stdafx.h"

#include "Conn.h"
#include "ugstuff.h"
#include "mdebug.h"

/*--- Global Documentation ---------------------------
 
	A few words on identifying connections.

  There are quite a few numbers associated with a given
  connection.  The following are used by UniGate:

  -- Station number, often abbreviation stn_no or sn.
   Amoung other things, this is an index into qwik, 
	which has station-specific info.

  -- TCB number, used by UniGate's TCP/IP implementation
   to identify a connection(?).

  -- Buffer number.  This is an index into a "buffers"
   array for a given page on the terminal screen.
	The "buffers array" was designed to hold multipurpose
	buffers, but in the WRU, we use it (so far) only for
   screen pages.  There are two buffers arrays:
   em_buffers, for emulated screens, and telclt_buffers,
   which hold accumulated received bytes that have not yet
   been put on the emulated screen.  I'm not happy about this
   arrangement, but it was the quickest way to get the
   existing Unigate code to do what I needed.

  -- conn_index, an index into the tel_screen_buffs array.
   Of greatest interest in this array is the station number 
	and buffer number for each screen page in the terminal.

  The WRU uses these additional numbers:

  -- Socket number, used by standard TCP/IP implementations,
   including those of Windows NT and UNIX, to identify a connection.

  -- An index into the Connection array.  This array contains
   information like the host and port name for the connection,
	the socket number, info needed by select(), semaphores
	to synchronize between the Web server and worker threads,
	and Web server specific info used to write HTML.

  WRU's port of UniGate code does not need all of the
  numbers used by UniGate.  We use the following mappings:

  -- Station number (stn_no) is simply the Connection index.

  -- conn_index is also simply the Connection index.

*/

//=== Macros =====================================================
#define UG_LOAD_TEL_SCREEN \
   struct telnet_screen *tel_screen; \
   int cur_page = tel_screen_buffs[conn_index].cur_page; \
   load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page], \
                   tel_screen_buffs[conn_index].buf_num[cur_page]);

//=== Global Variables ============================================
extern int Debug;
extern int MaxConnections;
///Qwik_entry *qwik;  // was MAXSTATIONS
///Stn_ref_entry *stn_ref;  // was MAXSTATIONS

extern struct struct_ug_globals ug_globals;

int16 telnet_clt_ports_open = 0; 

char debug_procs=0;
char debug_signals=0;

int16 host_audit=0;

extern struct telnet_screen **TelScrBuffers;   
extern struct telnet_screen_buffs *tel_screen_buffs;
extern TypConn *Connection;

//=== Function prototypes  ==========================================
void Dout(char *msg,char *buf, int nbytes);
BOOL IsUnisysTerm(int stn);
short screen_first_field (struct telnet_screen *ibm_screen);
int ibm_next_unprot_field_with_wrap(struct telnet_screen *ibm_screen,int mypos);
int ibm_screen_current_unprot_field(int conn_index,int mypos);


/*--- function ug_init_tel_screens ----------------------------------
 *
 *  Do initialization that, in UniGate, would be done by FEP3.C.
 *
 *  Mark telnet screens as unallocated.
 */
void
ug_init_tel_screens()
{
   int i,j;

   /* Specify that no screen buffs are allocated. */
   for (i = 1; i <= TELNET_TOTAL_CONNS; i++)
   {
      for (j = 0; j <= MAX_PAGES; j++)
         tel_screen_buffs[i].buf_num[j] = -1;

#ifdef UNI_VT
      tel_screen_buffs[i].term_state = TERMSTATE_RECV1;

      tel_screen_buffs[i].status_state = 0;
      tel_screen_buffs[i].alt_delim = (unsigned char)ug_globals.UseAlternateDelimiters;
      tel_screen_buffs[i].upshift = TRUE;
#endif

   }
}

int SendSocket(LPSTR msg, int Length, int stn)
{
#if 0
   int nbytes=0;
   nbytes = send(Connection[stn].sock, msg, Length,0);
   return nbytes;
#else 
   int nbytes =  tcp_send_log (stn, (unsigned char *)msg, Length,0);
   return nbytes;
#endif
}

short send_2_bgh (struct message *bufptr, int ibm_stn_no)
{
   SendSocket((char *)bufptr->buff,(int)bufptr->msglen,ibm_stn_no);
   return 1;
}

void bgh_send (unsigned char *c, int conn_index)
{
   SendSocket((char *)c,strlen((char *)c), conn_index);
}

/*--- Dummy functions from EM3270.CPP ----------------------------
 */
void bgh_move_cursor(struct telnet_screen *ibm_screen,short last_pos, short pos) 
{
}
void bgh_put_trailer(void) {}
void bgh_put_str(unsigned char *ctl_str, short pos, short last_pos ) {}
void bgh_get_attr(short attr, short pos, short prev_attr, short prev_pos,
		   char **ctl_str, char **left_over) {}
void bgh_refresh_screen(void) {}
void bgh_put_asc(unsigned char *c) {}
void bgh_put_key_menu (short pos) {}


boolean vt100_parse_input(  int16 conn_index,
			    int16 *counter,
			    int16 *len,
			    uint8 *txt)
{
   char *mybuf="Sorry, FkUG can't emulate VT terminal.  \r\n";
   int nchars = strlen(mybuf);

   tcp_send(conn_index,(unsigned char *)mybuf,nchars,0);
   return FALSE;
}

short uni_proc_trace (char *c)
{
   return 1;
}

short
proc_trace(char *msg)
{
   return 1;
}
/* =========================================================================
= $BEGDOC  prn_err_wind CR
*/

short prn_err_wind(
                short             i,
                unsigned char   *buff)
{
   MDebug((char *)buff);

   return 0;
}


short check_trancode (unsigned short sn)
{
   return 1;
}

/*--- function send_2_host -------------------------------------
 *
 *  Exit:   Returns 0 if success
 */
short send_2_host (struct message * buffptr)
{
   short id = buffptr->stn;
   unsigned char *bufst = buffptr->buff;
   unsigned short len = buffptr->msglen;

   // Tack on TELNET end-of-record to message before sending.
   bufst[len++] = TELNET_IAC;
   bufst[len++] = TELNET_EOR;
   return tcp_send_log(id,bufst,len,0);
}

/*--- function tcp_send_log -------------------------------------
 *
 *  Exit:   Returns 0 if success
 */
short tcp_send_log (short id,
          unsigned char *buff,
          unsigned short len,
          unsigned short flags)
{
   // Log this transmission.  Ignore the last two bytes, which
   // are just telnet EOR.
   if(Connection[id].logging) {
      LogTransmission(&Connection[id],(char *)buff,(int) len-2,"[em2ug]");
   }
   return tcp_send(id,buff,len,flags);
}

/*--- function tcp_send -------------------------------------
 *
 *  Exit:   Returns 0 if success
 */
short tcp_send (short id,
          unsigned char *buff,
          unsigned short len,
          unsigned short flags)
{
	SOCKET mysock = Connection[id].sock;
   TypConn *pConn = &Connection[id];
   int retcode = send((SOCKET)mysock, (char *)buff, (int) len, 0);
   if(SOCKET_ERROR == retcode || retcode != len) {
      return WSAGetLastError()+20000;
   }
   char mymsg[80];
   if(pConn->ConnDebug>1) {
      FmtDebug("Sending %d bytes:",len);
   }
   if(pConn->ConnDebug>1) {
      wsprintf(mymsg,"Snd to id %d: ",id);
      Dout(mymsg,(char *)buff,(int)len);
   }

	return 0;
}

short tcp_abort(short id)
{
   return 0;
}

short tcp_close(short id)
{
   return 0;
}

void PutH(char *msg)
{
	fputs(msg,stdout);
}

#if 0
short initqwik(void)
{
	int j;

   qwik = new Qwik_entry[MaxConnections+1];
	memset(qwik,0,sizeof(Qwik_entry)*(MaxConnections+1));
	for(j=0; j<=MaxConnections; j++) {
		qwik[j].conn_index = j;
	}

	return 1;
}
#endif
/* =========================================================================
= $BEGDOC  em_determine_page CR  Sets current page in term_state
*/

boolean em_determine_page (int16 conn_index,
				int16 page)

{

/*
=  DESCRIPTION:
=
=  RETURNS:  NONE
=
=  SEE ALSO:
=
=  FILES:
=
=  BUGS/NOTES:
=
= $ENDDOC
========================================================================== */
    proc_trace("em_determine_page");

#if 0
    if (page == 0)
	tel_screen_buffs[conn_index].term_state = TERMSTATE_RECV1;
    else if (page == 1)
	tel_screen_buffs[conn_index].term_state = TERMSTATE_RECV2;
    else if (page == 2)
	tel_screen_buffs[conn_index].term_state = TERMSTATE_RECV3;
    else if (page == 3)
	tel_screen_buffs[conn_index].term_state = TERMSTATE_RECV4;
#else
   tel_screen_buffs[conn_index].term_state = page + 1;
#endif

    return(1);
}

boolean emf_check_for_end (struct telnet_screen *tel_screen,
                          int16 pos)

{

/*
=  DESCRIPTION:     See if we are at the end of a field.
=
=  RETURNS:  NONE
= $ENDDOC
========================================================================== */

    proc_trace("em_check_for_end");

    return is_end_of_field(tel_screen->buffer[pos]);
}

boolean is_end_of_field(char ichar)
{
   return ( (ichar == RS) ||
         (ichar == US) ||
         (ichar == GS) ||
         (ichar == FS) );
}
/*--- function ug_first_field ---------------------------
 *
 *  Position the cursor at the first field on the screen,
 *  or upper left if no unprotected fields are on the screen.
 *
 *  Entry:  conn_index  is an index into the Connection array
 *
 *  Exit:   Returns a screen position (0-1919)
 */

short ug_first_field (int conn_index)
{
   int newpos = 0;
   UG_LOAD_TEL_SCREEN
   tel_screen->scr_pos = 0;  // Start at upper left
   if(IsUnisysTerm(conn_index)) {
      if(ug_get_forms_mode(conn_index)) {
         newpos = ug_next_field(conn_index,0);
      }
   } else {
      newpos = screen_first_field(tel_screen) + 1;
      if(newpos<0) newpos=0;
      tel_screen->scr_pos = newpos;
   }
   return newpos;
}

/*--- function ug_home_cursor -----------------------------------
 *
 *  Position the cursor at the first field on the screen,
 *  or upper left if no unprotected fields are on the screen.
 *  Set terminal to LOCAL mode.
 *
 *  Entry:  conn_index  is an index into the Connection array
 *
 *  Exit:   Returns a screen position (0-1919)
 */
short ug_home_cursor(int conn_index)
{
   int retval = ug_first_field(conn_index);
   // Homing the cursor puts terminal in LOCAL state.
   if(IsUnisysTerm(conn_index))ug_set_term_state(conn_index,FALSE);

   return retval;
}

/*--- function ug_next_field  -----------------------------------
 *
 *  Move the current position to the first char of the next field.
 * 
 *  Entry:  conn_index  is the connection index.
 *          j           is the character position (0=top left)
 *                      from which to start.
 *
 *  Exit:   Returns the new current position.
 */
short ug_next_field (int conn_index,
                  int j)
{

/*
=  DESCRIPTION:     
=
=  RETURNS:  NONE
=
=  SEE ALSO:
=
=  FILES:
=
=  BUGS/NOTES:
=
= $ENDDOC
========================================================================== */

    boolean beep;
    boolean done;

    int16   col;
    int16   line;
    int16   start_pos;

   UG_LOAD_TEL_SCREEN

    done = FALSE;
    beep = FALSE;
    start_pos = j;
    if(IsUnisysTerm(conn_index)) {
    while (!done)
    {
        if ( (tel_screen->buffer[j] == US) &&
             !emf_check_for_end(tel_screen,j+1) )
        {
            j++;
            line = j / 80;
            col = j % 80;
            tel_screen->scr_pos = j;
            //vt100_scr_pos(line + 1, col + 1, tcb_id);
            done = TRUE;
            tel_screen->term_mode = TELTERM_LEFT_JUST;
        }
        else if ( (tel_screen->buffer[j] == GS) &&
                  !emf_check_for_end(tel_screen,j+1) )
        {
            j++;
            while ( (j < SCRSIZE) &&
                    !emf_check_for_end(tel_screen,j) )
                j++;
            j--;

            line = j / 80;
            col = j % 80;
            tel_screen->scr_pos = j;
            //vt100_scr_pos(line + 1, col + 1, tcb_id);
            done = TRUE;
            tel_screen->term_mode = TELTERM_RIGHT_JUST;
        }
        else
        {
            j++;
            if (j >= SCRSIZE)
            {
                j = 0;
                beep = TRUE;
            }
            /* So that we don't get hung up
                DAE 8/2/93    found at GM*/
            if (j == start_pos)
                done = TRUE;
        }
    }
    } else {
      // IBM terminal
      int fieldpos = ibm_next_unprot_field_with_wrap(tel_screen,j);
      tel_screen->scr_pos = fieldpos+1;

    }

    return(tel_screen->scr_pos);
}


/*--- function ug_current_unprot_field -----------------------------------
 *  
 *  Give information about current field (if any) in T27 mode.
 *
 *  Entry:  conn_index  is the connection index.
 *          mypos       is the character position (0-1919) to test
 *
 *  Exit:   mode        is the type of field the cursor was in:
 *                      FALSE if not in a field (or not forms mode),
 *                      or TELTERM_LEFT_JUST, or TELTERM_RIGHT_JUST.
 *          Returns the cursor position of the first char in this
 *          field, or -1 if none.
 */
int ug_current_unprot_field(int conn_index, int mypos, int &mode)
{
   UG_LOAD_TEL_SCREEN;
   int bFormsMode = ug_get_forms_mode(conn_index);
   int retval=-1, k, forms_field=FALSE, fld_pos;

   if(bFormsMode) {
      /* First find out if we are in a field */
      k = 0;
      while (k != mypos) {
         if (tel_screen->buffer[k] == US) {
            forms_field = TELTERM_LEFT_JUST;
            fld_pos = k;
         } else if (tel_screen->buffer[k] == GS) {
            forms_field = TELTERM_RIGHT_JUST;
            fld_pos = k;
         } else if (tel_screen->buffer[k] == RS) {
            forms_field = FALSE;
         }
         k++;
      }
      if (tel_screen->buffer[k] == RS) forms_field = FALSE;

      if(TELTERM_LEFT_JUST==forms_field) {
         retval = fld_pos+1;
      } else if(TELTERM_RIGHT_JUST == forms_field) {
         // Right-justified field--position at one char before
         // the end-of-field marker.
         retval = mypos;
         while ( (retval < SCRSIZE) &&
                 !emf_check_for_end(tel_screen,retval) )
             retval++;
         retval--;
      }
   }
   mode = forms_field;
   return retval;
}

/*--- function UnprotectedStart -----------------------------------
 *
 *  Says whether a given position is in an unprotected field,
 *  and if it is, says where that field starts.
 *
 *  Entry:  pConn    points to a connection structure.
 *          col      is a 1-based column number.
 *          row      is a 1-based row number.
 *
 *  Exit:   Returns -1 if the position is not unprotected, 
 *          else the position (0-1919) if the beg of that field.
 */
int UnprotectedStart(TypConn *pConn, int col, int row) 
{
   int goto_pos, mypos=80*(row-1)+col-1, mode;
   if(IsUnisysTerm(pConn->iord)) {
      goto_pos = ug_current_unprot_field(pConn->iord,mypos,mode);
   } else {
      goto_pos = ibm_screen_current_unprot_field(pConn->iord,mypos);
   }
   return goto_pos;
}

/*--- function ug_get_forms_mode ---------------------------------
 *
 *  Returns TRUE if in forms mode.
 */
int
ug_get_forms_mode(int conn_index)
{
   struct telnet_screen *tel_screen; 
   int cur_page = tel_screen_buffs[conn_index].cur_page; 
   if(tel_screen_buffs[conn_index].buf_num[cur_page]<0) return FALSE;
   load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page], 
                   tel_screen_buffs[conn_index].buf_num[cur_page]);

   return (TELTERM_DATA != tel_screen->term_mode);
}

/*--- function ug_get_insert_mode ----------------------------------
 */
int 
ug_get_insert_mode(int conn_index)
{
   struct telnet_screen *tel_screen; 
   int cur_page = tel_screen_buffs[conn_index].cur_page; 
   if(tel_screen_buffs[conn_index].buf_num[cur_page]<0) return FALSE;
   load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page], 
                   tel_screen_buffs[conn_index].buf_num[cur_page]);
   return (tel_screen->insert_mode);
}

/*--- function ug_get_term_state ----------------------------------
 */
int 
ug_get_term_state(int conn_index)
{
   return (tel_screen_buffs[conn_index].term_state);
}

/*--- function ug_set_term_state -------------------------------
 *
 *  Set terminal state to Local or Receive.
 *
 *  Entry:  conn_index  is the connection index.  Smallest allowed
 *                      value is 1.
 *          bReceive    is TRUE for Receive, else FALSE for Local.
 *                      TRUE is mapped to the appropriate internal
 *                      representation for the current page.
 */
void 
ug_set_term_state(int conn_index, BOOL bReceive)
{
   if(!bReceive) {
      tel_screen_buffs[conn_index].term_state = TERMSTATE_LOCAL;
   } else {
      em_determine_page(conn_index,tel_screen_buffs[conn_index].cur_page);
   }
}

int 
ug_get_auto_flip(int conn_index)
{
   return (tel_screen_buffs[conn_index].auto_flip);
}

/*--- function ug_set_auto_flip -----------------------------------
 * 
 *  Set the terminal characteristic "Auto Flip To Local" for
 *  the connection.
 *
 *  Entry:  conn_index  is the connection number (1-n).
 *          newval      is true or false.  If True, the terminal will
 *                      switch to Local mode after receiving any
 *                      subsequent screens.
 */
void
ug_set_auto_flip(int conn_index, int newval)
{
   tel_screen_buffs[conn_index].auto_flip = newval;
}

int
ug_get_curpos(int conn_index)
{
   UG_LOAD_TEL_SCREEN
   ///if(tel_screen_buffs[conn_index].term_type_num == TELNET_TERM_TD830) {
      return tel_screen->scr_pos;
   ///} else {
   ///   return tel_screen->cursor_pos;
   ///}
}

void
ug_set_curpos(int conn_index,int curpos)
{
   UG_LOAD_TEL_SCREEN

   tel_screen->scr_pos = curpos;
}

void ug_get_termloc(int conn_index, char *buf)
{
   UG_LOAD_TEL_SCREEN

   memcpy(buf,tel_screen->term_loc,TERM_LOC_LEN);
   buf[TERM_LOC_LEN] = '\0';
}
