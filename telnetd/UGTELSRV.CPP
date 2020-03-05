/***************************************************************************/
/*                                                                         */
/*                      STANDARD NETWORKS, INC.                            */
/*                                                                         */
/*      Filename:  telsrv.c                                                */
/*                                                                         */
/*      Description: This file contains functions to handle requests       */
/*                      to use UNIGATE/TCP's TELNET server capabilites.    */
/*                                                                         */
/*      Revison History                                                    */
/*              Date      By     Description                               */
/*              --------- ---    -----------                               */
/*              31 May 91 DAE    Created                                   */
/*              14 Apr 96 MRR    Converted to FkUG                         */
/*                                                                         */
/*      Technical Information:                                             */
/*                                                                         */
/*                                                                         */
/*  THIS IS AN UNPUBLISHED WORK PROTECTED BY FEDERAL COPYRIGHT LAW.        */
/*  ANY UNAUTHORIZED REPRODUCTION IS PROHIBITED.                           */
/*  THE CONTENTS OF THIS WORK ARE CONFIDENTIAL AND MAY NOT BE              */
/*  DISCLOSED OR USED WITHOUT EXPRESS WRITTEN PERMISSION OF                */
/*                                                                         */
/*                      STANDARD NETWORKS, INC.                            */
/*                      4610 UNIVERSITY AVENUE, SUITE 1080                 */
/*                      MADISON, WI 53705                                  */
/*                      (608) 231-3100                                     */
/*                                                                         */
/*   (C) COPYRIGHT STANDARD NETWORKS, INC.                                 */
/*   ALL RIGHTS RESERVED                                                   */
/***************************************************************************/


 /* include files */

#include "stdafx.h"
#include "Conn.h"
#include "MDebug.h"
#include "ugstuff.h"
#include "ibmdef.h"

#define ONLINE     1
#define OFFLINE    0

#define UP      1
#define DOWN    0


//=== Global Variables  ===============================================
extern int Debug;
#if 0
struct
{
    char    *str;
    char    term_type;
}   srv_types [] =
    {
        "td830",        TELNET_TERM_TD830,
        "vt100",        TELNET_TERM_VT100,
        "vt2",          TELNET_TERM_VT220,
        "vt102",        TELNET_TERM_VT100,
        "ansi",         TELNET_TERM_VT100,
        "nvt-printer",  TELNET_TERM_PRT132,
        "ibm-327",      TELNET_TERM_3278,
        "ibm-3179",     TELNET_TERM_5250,
        "ibm-5251-11",  TELNET_TERM_5250,
        "ibm-3196-a1",  TELNET_TERM_5250,
        "",             TELNET_TERM_UNKNOWN
    };
#else 
struct struct_srv_types 
{
    char    *str;
    char    term_type;
};

struct struct_srv_types  srv_types_td830 [] =  {
   "UNISYS-TD830", TELNET_TERM_TD830,
   "A-SERIES TD830 COMPATIBLE", TELNET_TERM_TD830,
   "",             TELNET_TERM_UNKNOWN
};

struct struct_srv_types  srv_types_5250 [] =  {
   "IBM-5251-11 ", TELNET_TERM_5250,
   "",             TELNET_TERM_UNKNOWN
};

struct struct_srv_types  srv_types_3270 [] =  {
   "IBM-3278-2",   TELNET_TERM_3278,
   "",             TELNET_TERM_UNKNOWN
};

struct struct_srv_types *psrv_types = srv_types_td830;
#endif

long total_telnet_sessions = 0;

extern int ConnectionLimit;

extern uchar asc2ebc[256];
extern uchar ebc2asc[256];

extern struct telnet_screen tel_screen_buf;

extern struct telnet_screen_buffs *tel_screen_buffs;
extern struct telnet_screen **TelScrBuffers;   

 short dec_vt_enabled=0;
 short ibm3270_enabled=1;
 short ibm5250_enabled=1;

//=== Function prototypes  =============================================
int telnet_server
   (int16 id,                   /* TCB id */
       int16 sig,               /* TCP signal */
       uint8 * txt,             /* Data buffer */
       int16 len,               /* Amount of 'good' data in buffer */
       int16 flags);             /* Flags passed with data */
void send_opt
   (   int16 id,
       int16 option,
       int16 opcode,
       int16 ask);
void td830_welcome
   (int16 id , int16 conn_index);
void tel_abort
   (int16 conn_index,
       int16 id,
       int16 type);
void send_buff_to_host
   (int16 conn_index,
       boolean mcs,
       unsigned char dest_host,
       uint8 page);
void assign_station
   (int16 conn_index);
void disconnect_station
   (int16 conn_index);
void send_text_message
   (int16 conn_index,
    char  *text);


void em_send_input_to_srv(int conn_index);
void em_init_page(struct telnet_screen *tel_screen);

///===  Functions  ======================================================

/*--- function SetTermTypeToNegotiate ---------------------------
 *
 *  Set the terminal type to negotiate during Telnet negotiations.
 *
 *  Entry:  szTermType  is "td830", or "5250", or "3270".
 *
 *  Exit:   Returns 0 upon success, else error code.
 */
int SetTermTypeToNegotiate(char *szTermType)
{
   int retval = 0;

   if(!stricmp(szTermType,"td830")) {
      psrv_types = srv_types_td830;
   } else if(!stricmp(szTermType,"3270")) {
      psrv_types = srv_types_3270;
   } else if(!stricmp(szTermType,"5250")) {
      psrv_types = srv_types_5250;
   } else {
      retval = 27;
   }
   return retval;
}

/* =========================================================================
= $BEGDOC  telnet_server 2 CR  Telnet server process module
*/

int telnet_server
   (int16 id,                   /* TCB id */
       int16 sig,               /* TCP signal */
       uint8 * txt,             /* Data buffer */
       int16 len,               /* Amount of 'good' data in buffer */
       int16 flags)             /* Flags passed with data */
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

   boolean foundit;             /* Indicates if we have found a XMIT */
   boolean disp_pos;
   boolean type_found;
   int send_to_host = FALSE;
   char msg[256];

   int16 conn_index;
   int16 i, j, k;            /* counter */

   uint8 cur_page;
   uint8 opcode;
   unsigned char scratch_buff[BUFFSIZE];

   uchar8 sub_send[] = {TELNET_IAC, TELNET_SB, '\0', TELQUAL_SEND,
    TELNET_IAC, TELNET_SE};

   struct message recv_buffer;
   struct telnet_screen *tel_screen;

   proc_trace ("telnet_server");

   /* Determine which tel_screen structure record is being accessed by this
      TCB id.  If conn_index ends up equal to TELNET_TOTAL_CONNS, we do not
      have any information regarding this TCB id. */

   conn_index = id;
   if(tel_screen_buffs[conn_index].conn_id != id) {
      wsprintf(msg,"** \a\aError: tel_screen_buffs[%d].conn_id = %d",
         conn_index, tel_screen_buffs[conn_index].conn_id);
      puts(msg);
      MDebug(msg);
      return 5;
   }

   /* If we have no knowledge of the TCB id in tel_screen, we will just throw
      away the request, by setting sig to an unreasonable value thereby
      preventing execution of the switch, since we don't know what to do with
      it. */

   if (conn_index == (TELNET_TOTAL_CONNS + 1))
   {
      tcp_abort (id);
      return send_to_host;
   }

   cur_page = tel_screen_buffs[conn_index].cur_page;

   /* buf_num was set to -1 in fep2.c when startup occrued.  It specifies un
      allocated pages */

   if (tel_screen_buffs[conn_index].buf_num[cur_page] == -1)
      alloc_screen_buf (&tel_screen_buffs[conn_index].buf_type[cur_page],
                (unsigned short *)&tel_screen_buffs[conn_index].buf_num[cur_page]);

   load_screen_buf (tel_screen_buffs[conn_index].buf_type[cur_page],
                    tel_screen_buffs[conn_index].buf_num[cur_page]);

   /* Check for idle time signoff */


   switch (sig)
   {
   case SG_DATA:
      /* Reset */
      tel_screen_buffs[conn_index].timeout_ticks = 0;

      foundit = FALSE;
      i = 0;


     while (i < len)
/**
11/3/94  DAE
changed at BACOB, now there can be data after an EOF character.
now works more like ptop
      while ((!foundit) && (i < len))
**/
      {



        /**
            11/3/94  DAE
            changed at BACOB, now there can be data after an EOF character.
            now works more like ptop.

            This must be at the begining of the while loop since the case
                statments to a continue and don't fall to the end.
        **/

        /* Set up packet to send to host */
        if (foundit)
        {
            cur_page = tel_screen_buffs[conn_index].cur_page;
            store_screen_buf (tel_screen_buffs[conn_index].buf_type[cur_page],
                    tel_screen_buffs[conn_index].buf_num[cur_page]);
            if (tel_screen_buffs[conn_index].stn_no == 0) {
                tel_abort (conn_index, id, 5);
            } else {
                //em_send_input_to_srv(conn_index);
                send_to_host = TRUE;
            }
            foundit = FALSE;

            cur_page = tel_screen_buffs[conn_index].cur_page;
            load_screen_buf (tel_screen_buffs[conn_index].buf_type[cur_page],
                    tel_screen_buffs[conn_index].buf_num[cur_page]);

        }

        if (tel_screen->scr_pos > SCRBUFSIZE)
        {
            printf("BIG err: tel_srv scr_pos: %d  stn: %d   ",
                tel_screen->scr_pos, tel_screen_buffs[conn_index].stn_no);
            tel_screen->scr_pos = SCRBUFSIZE;
        }
         disp_pos = FALSE;
         switch (tel_screen->telnet_state)
         {

         case TELSTATE_IAC:
            /* See what follows IAC */
            switch (txt[i])
            {
            case TELNET_WILL:
               i++;
               tel_screen->telnet_state = TELSTATE_WILL;
               continue;

            case TELNET_WONT:
               i++;
               tel_screen->telnet_state = TELSTATE_WONT;
               continue;

            case TELNET_DO:
               i++;
               tel_screen->telnet_state = TELSTATE_DO;
               continue;

            case TELNET_DONT:
               i++;
               tel_screen->telnet_state = TELSTATE_DONT;
               continue;

            case TELNET_SB:
               i++;
               tel_screen->sub_opcode = 0;
               tel_screen->telnet_state = TELSTATE_SB;
               continue;

            case TELNET_SE:
               i++;
               break;

            case TELNET_EL:
            case TELNET_EC:
            case TELNET_IP:
            case TELNET_AO:
            case TELNET_BREAK:
            case TELNET_DM:    /** SYNCH, flush buffers  **/
            case TELNET_NOP:

               sprintf ((char *)scratch_buff, "TEL CDE?:%x STN:%d\0",
                        txt[i], tel_screen_buffs[conn_index].stn_no);
               prn_err_wind (0, scratch_buff);

               /* To support ?BRK on ICC's emulator */

               if (txt[i] == TELNET_AO)
               {
                  sprintf ((char *)recv_buffer.buff, "?BRK\0");
                  send_buff_to_host (conn_index, TRUE, 0, cur_page);

                  sprintf ((char *)scratch_buff, "%c%c\0",
                           TELNET_IAC, TELNET_DM);

                  tcp_send (id, scratch_buff,
                            strlen ((char *)scratch_buff), FL_PSH);
               }

#ifdef IBM
               if (tel_screen_buffs[conn_index].term_type_num == TELNET_TERM_3278)
               {
                  j = 0;
                  scratch_buff[j++] = W;
                  scratch_buff[j++] = 0xC2;
                  scratch_buff[j++] = TELNET_IAC;
                  scratch_buff[j++] = TELNET_EOR;
                  tcp_send(id, scratch_buff, j, FL_PSH);
                  tel_screen->scr_pos = 0;
               }
               else if (tel_screen_buffs[conn_index].term_type_num == TELNET_TERM_5250)

               {
                  j = 0;
                  scratch_buff[j++] = 0;
                  scratch_buff[j++] = 0;
                  scratch_buff[j++] = 0x12;
                  scratch_buff[j++] = 0xA0;
                  scratch_buff[j++] = 0x00;
                  scratch_buff[j++] = 0x00;
                  scratch_buff[j++] = 0x04;
                  scratch_buff[j++] = 0x00;
                  scratch_buff[j++] = 0x00;
                  scratch_buff[j++] = 0x03;
                  scratch_buff[j++] = 0x04;
                  scratch_buff[j++] = 0x11;
                  scratch_buff[j++] = 0x00;
                  scratch_buff[j++] = 0x08;
                  scratch_buff[j++] = 0x04;
                  scratch_buff[j++] = 0x52;
                  scratch_buff[j++] = 0x00;
                  scratch_buff[j++] = 0x00;
                  scratch_buff[0] = (j) >> 8;
                  scratch_buff[1] = (j) & 0xff;
                  scratch_buff[j++] = TELNET_IAC;
                  scratch_buff[j++] = TELNET_EOR;
                  tcp_send(id, scratch_buff, j, FL_PSH);
                  tel_screen->scr_pos = 0;
               }
#endif


               i++;
               break;

            case TELNET_EOR:
               if (tel_screen->tel_opts[TELOPT_EOR] &
                   TELOPT_ACTIVE_IN)
                  foundit = TRUE;
               i++;

#ifdef UNI_VT
#ifdef IBM
               if (tel_screen_buffs[conn_index].term_type_num == TELNET_TERM_3278)
                  foundit = ibm3270_eor (conn_index);
               else if (tel_screen_buffs[conn_index].term_type_num == TELNET_TERM_5250)
                  foundit = ibm5250_eor (conn_index);
#endif
#endif
               break;

            case TELNET_IAC:
               /* IAC followed by an IAC is a byte of IAC data. */
               tel_screen->buffer[tel_screen->scr_pos++] =
                  txt[i];
               i++;
               break;

            case TELNET_AYT:
               /* Respond to an Are-You-There */

               sprintf ((char *)recv_buffer.buff, "?WRU\0");
               send_buff_to_host (conn_index, TRUE, 0, cur_page);
               i++;
               break;

            case TELNET_GA:
               /* Ignore GA in server mode. */
               i++;
               break;

            default:
               break;

            }                   /* END of switch(txt[i]) */

            tel_screen->telnet_state = TELSTATE_DATA;

            continue;           /* END of TELSTATE_IAC case */

         case TELSTATE_WILL:

            opcode = txt[i];

            if (opcode == 0x87)
            {
/**
     3 / 7 /95   DAE
    Kludge to get around GFS Unix ICC problem of 0x87 coming back instead
        of 0x17.  **/
#if 0
                if (tcpip.tel_no_sndloc_action == '0')
                {
                    tel_abort (conn_index, id, 1);
                    i = len;
                }
                else
#endif
                if ( (!ibm3270_enabled &&
                        (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_3278)) ||
                       (!ibm5250_enabled &&
                        (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_5250)) ||
                       (!dec_vt_enabled &&
                         ((tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_VT100) ||
                          (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_VT220))) )
                {
                    sprintf ((char *)scratch_buff,"UNSUPP:%s\0",
                                            tel_screen->term_type_str);
                    prn_err_wind (0, scratch_buff);
                    tel_abort (conn_index, id, 11);
                    i = len;
                }
                else
                    assign_station (conn_index);
            }
            else if ((opcode > NTELOPTS) ||
                !(tel_screen->tel_opts[opcode] &
                  TELOPT_SUPPORTED_IN))
            {
               /* Option is out of the range of what we are supporting, or it
                  is not supported. */
               send_opt (id, TELNET_DONT, opcode, 0);
            }
            else if (tel_screen->tel_opts[opcode] &
                     TELOPT_DO_ASKING)
            {
               /* If we were asking, mark it active. We would have sent a DO
                  opcode. */
               tel_screen->tel_opts[opcode] |= TELOPT_ACTIVE_IN;

               /* Turn off the asking bit */
               tel_screen->tel_opts[opcode] ^= TELOPT_DO_ASKING;

               /* Request subnegotion if they do termtype */
               if (opcode == TELOPT_TTYPE)
               {
                  sub_send[2] = TELOPT_TTYPE;
                  tel_screen->sub_opcode = 0;
                  tel_screen_buffs[conn_index].term_type_num =
                     TELNET_TERM_UNKNOWN;
                  tel_screen->type_ptr = 0;
                  for (k = 0; k < TERM_TYPE_LEN; k++)
                  {
                     tel_screen->term_type_str[k] = 0;
                     tel_screen->term_last_type_str[k] = 0;
                  }
                  tcp_send (id, sub_send, 6, FL_PSH);
               }
            }
            else
            {
               /* The client started the loop. */
               /* Only respond if we are already not doing it */
               if (!(tel_screen->tel_opts[opcode] &
                     TELOPT_ACTIVE_IN))
               {
                  send_opt (id, TELNET_DO, opcode, 0);

                  if (opcode == TELOPT_TTYPE)
                  {
                     sub_send[2] = TELOPT_TTYPE;
                     tel_screen->sub_opcode = 0;
                     tel_screen_buffs[conn_index].term_type_num =
                        TELNET_TERM_UNKNOWN;
                     tel_screen->type_ptr = 0;
                     for (k = 0; k < TERM_TYPE_LEN; k++)
                     {
                        tel_screen->term_type_str[k] = 0;
                        tel_screen->term_last_type_str[k] = 0;
                     }
                     tcp_send (id, sub_send, 6, FL_PSH);
                  }
               }
            }

            i++;
            tel_screen->telnet_state = TELSTATE_DATA;

            continue;           /* End of TELSTAT_WILL case */


         case TELSTATE_WONT:

            opcode = txt[i];

            /* If we were asking to have it active in... */
            if (tel_screen->tel_opts[opcode] & TELOPT_DO_ASKING)
            {
               /* If we were expecting it active in, turn it off. */
               if (tel_screen->tel_opts[opcode] & TELOPT_ACTIVE_IN)
                  tel_screen->tel_opts[opcode] ^= TELOPT_ACTIVE_IN;

               /* Turn the do asking bit off */
               tel_screen->tel_opts[opcode] ^= TELOPT_DO_ASKING;

               if (opcode == TELOPT_TTYPE)
               {
                  tel_screen->telnet_state = TELSTATE_SB_IAC;
                  txt[i] = TELNET_SE;
                  tel_screen->sub_opcode = TELOPT_TTYPE;
                  tel_screen->term_type_str[0] = 'A';
                  tel_screen->term_type_str[1] = 0;
                  tel_screen->term_last_type_str[0] = 'A';
                  tel_screen->term_last_type_str[1] = 0;
                  continue;

               }
               else if (txt[i] == TELOPT_SNDLOC)
               {
#if 0
                  if (tcpip.tel_no_sndloc_action == '0')
                  {
                     tel_abort (conn_index, id, 1);
                     i = len;
                  }
                  else
#endif
                  if ( (!ibm3270_enabled &&
                        (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_3278)) ||
                       (!ibm5250_enabled &&
                        (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_5250)) ||
                       (!dec_vt_enabled &&
                         ((tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_VT100) ||
                          (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_VT220))) )
                  {
                     sprintf ((char *)scratch_buff,"UNSUPP:%s\0",
                                            tel_screen->term_type_str);
                     prn_err_wind (0, scratch_buff);
                     tel_abort (conn_index, id, 11);
                     i = len;
                  }
                  else
                     assign_station (conn_index);
               }
            }
            else
            {
               /* They are telling us that they will not do something.  In
                  this instance, we did not solicit them.  Only respond if
                  the option was active in. */

               if (tel_screen->tel_opts[opcode] & TELOPT_ACTIVE_IN)
                  send_opt (id, TELNET_DONT, opcode, 0);
            }

            i++;
            tel_screen->telnet_state = TELSTATE_DATA;

            continue;           /* End of TELSTAT_WONT case */

         case TELSTATE_DO:

            opcode = txt[i];
            if ((opcode > NTELOPTS) ||
                !(tel_screen->tel_opts[opcode] & TELOPT_SUPPORTED_OUT))
            {
               /* Option is out of the range of what we are supporting, or it
                  is not supported. */
               send_opt (id, TELNET_WONT, opcode, 0);
            }
            else if (tel_screen->tel_opts[opcode] &
                     TELOPT_WILL_ASKING)
            {
               /* If we were asking, mark it active. We would have sent a
                  WILL opcode. Right now, the only will we send is ECHO.  We
                  do not want him to ECHO. */
               tel_screen->tel_opts[opcode] |= TELOPT_ACTIVE_OUT;
               /* Turn off ASKING bit */
               tel_screen->tel_opts[opcode] ^= TELOPT_WILL_ASKING;

#ifdef IBM
               if (tcpip.tel_send_welcome == 'Y')
               {
                   if ((tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_3278) &&
                       (opcode == TELOPT_BINARY))
                   {
                      ibm3270_welcome (id);
                      tel_screen->scr_pos = 0;
                   }
                   else if ((tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_5250) &&
                            (opcode == TELOPT_BINARY))
                   {
                      ibm5250_welcome (id);
                      tel_screen->scr_pos = 0;
                   }
               }
#endif
            }
            else
            {
               /* The client started the loop. */

               /* Only respond if we are already not doing it */

               if (!(tel_screen->tel_opts[opcode] & TELOPT_ACTIVE_OUT))
                  send_opt (id, TELNET_WILL, opcode, 0);

            }

            i++;
            tel_screen->telnet_state = TELSTATE_DATA;

            continue;           /* END ofTELSTATE_DO case */


         case TELSTATE_DONT:
            opcode = txt[i];

            if (tel_screen->tel_opts[opcode] & TELOPT_WILL_ASKING)
            {
               /* If we were asking, make sure the active out bit is off, if
                  it is on. */
               if (tel_screen->tel_opts[opcode] &
                   TELOPT_ACTIVE_OUT)
                  tel_screen->tel_opts[opcode] ^=
                     TELOPT_ACTIVE_OUT;

               /* Turn off the will asking */
               tel_screen->tel_opts[opcode] ^= TELOPT_WILL_ASKING;
            }
            else
            {
               /* Only respond if we are doing it. This would be from an
                  unsolicited DONT. */
               if (tel_screen->tel_opts[opcode] & TELOPT_ACTIVE_OUT)
                  send_opt (id, TELNET_WONT, opcode, 0);
            }

            i++;
            tel_screen->telnet_state = TELSTATE_DATA;

            continue;           /* END of TELSTATE_DONT case */

         case TELSTATE_SB:

/***
    Used to test GFS ICC UNIX problem.
if (txt[i] == TELOPT_SNDLOC)
{
    printf("changed\n");
    txt[i] = 0x87;
}
***/
            if (txt[i] == TELNET_IAC)
               tel_screen->telnet_state = TELSTATE_SB_IAC;
            else if (txt[i] == TELOPT_SNDLOC)
            {
               tel_screen->sub_opcode = txt[i];
               tel_screen->telnet_state = TELSTATE_SB_IS;
            }
            else
            {
               tel_screen->sub_opcode = txt[i];
               tel_screen->telnet_state = TELSTATE_SB_OPCODE;
            }
            i++;

            continue;           /* END of TELSTATE_SB case */


         case TELSTATE_SB_OPCODE:

            if (txt[i] == TELQUAL_SEND)
               tel_screen->telnet_state = TELSTATE_SB_SEND;
            else
               tel_screen->telnet_state = TELSTATE_SB_IS;
            i++;

            continue;           /* END of TELSTATE_SB_OPCODE case */

         case TELSTATE_SB_IS:

            if (txt[i] == TELNET_IAC)
               tel_screen->telnet_state = TELSTATE_SB_IAC;
            else if (tel_screen->sub_opcode == TELOPT_TTYPE)
            {
               if (tel_screen->type_ptr < TERM_TYPE_LEN)
                  tel_screen->term_type_str[tel_screen->type_ptr++] =
                     tolower (txt[i]);
            }
            else if (tel_screen->sub_opcode == TELOPT_SNDLOC)
            {
               if (tel_screen->loc_ptr < TERM_LOC_LEN)
                  tel_screen->term_loc[tel_screen->loc_ptr++] =
                     txt[i];
            }

            i++;

            continue;           /* END of TELSTATE_SB_IS case */

         case TELSTATE_SB_SEND:

            if (txt[i] == TELNET_IAC)
               tel_screen->telnet_state = TELSTATE_SB_IAC;
            i++;

            continue;           /* END of TELSTATE_SB_SEND case */

         case TELSTATE_SB_IAC:

            if (txt[i] == TELNET_SE)
            {
               tel_screen->telnet_state = TELSTATE_DATA;
               if (tel_screen->sub_opcode == TELOPT_TTYPE)
               {
                  tel_screen->sub_opcode = 0;
                  type_found = FALSE;
                  strncpy((char *)scratch_buff,(const char *)tel_screen->term_type_str,sizeof scratch_buff);
                  _strupr((char *)scratch_buff);
                  for (j=0; psrv_types[j].str[0] != 0; j++)
                  {
                     if (strstr((const char *)scratch_buff,
                                psrv_types[j].str) != NULL)
                     {
                        /* Do not accept TD830-NDL or TD830-ASCII *
                        if ( (srv_types[j].term_type == TELNET_TERM_TD830) &&
                             ((strstr(tel_screen->term_type_str, "ndl") ) ||
                              (strstr(tel_screen->term_type_str, "ascii"))) )
                           continue;
                           */

                        /* Do not accept TD830-NDL */
                        if ( (psrv_types[j].term_type == TELNET_TERM_TD830) &&
                             (strstr((const char *)scratch_buff, "ndl") ) )
                            continue;

                        if ( (psrv_types[j].term_type == TELNET_TERM_TD830) &&
                             (strstr((const char *)scratch_buff, "ascii") ) )
                            tel_screen_buffs[conn_index].term_type_num =
                                        TELNET_TERM_TD830A;
                        else
                            tel_screen_buffs[conn_index].term_type_num =
                                                psrv_types[j].term_type;
                        type_found = TRUE;
                        break;
                     }
                  }

                  if (type_found)
                  {
                     /*
                        If term-type is not enabled and is not last chance,
                        keep trying til we get one we like. If this is the
                        last chance, take it now and we will kick him out later.
                     */
                     if (!ibm3270_enabled &&
                         (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_3278) &&
                         (stricmp((const char *)tel_screen->term_type_str,
                                 (char *)tel_screen->term_last_type_str) != 0) )
                        type_found = FALSE;
                     else
                     if (!ibm5250_enabled &&
                         (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_5250) &&
                         (stricmp((const char *)tel_screen->term_type_str,
                                 (char *)tel_screen->term_last_type_str) != 0) )
                        type_found = FALSE;
                     else
                     if (!dec_vt_enabled &&
                         ((tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_VT100) ||
                          (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_VT220)) &&
                         (stricmp((const char *)tel_screen->term_type_str,
                                 (char *)tel_screen->term_last_type_str) != 0) )
                        type_found = FALSE;

                  }

                  if (!type_found)
                  {
                     if (stricmp((const char *)tel_screen->term_type_str,
                                (char *)tel_screen->term_last_type_str) == 0)
                     {
                        /* We have found the end of the list but it is not a
                           valid term type. */

                        sprintf ((char *)scratch_buff,"UNK TTYP:%s\0",
                              (char *)tel_screen->term_type_str);
                        prn_err_wind (0, scratch_buff);

                        if (dec_vt_enabled)
                        {
                            sprintf ((char *)scratch_buff,
                             "\r\n\r\nUnable to determine terminal type\r\n"
                               "Default is VT100 emulation.%c[1;1H\r\n\r\n",
                                 ESC);
                            tcp_send (id, scratch_buff, strlen ((char *)scratch_buff), FL_PSH);
                        }
                        tel_screen_buffs[conn_index].term_type_num =
                                                        TELNET_TERM_VT100;
                     }
                     else
                     {
                        memcpy (tel_screen->term_last_type_str,
                                tel_screen->term_type_str,
                                TERM_TYPE_LEN);
                        sub_send[2] = TELOPT_TTYPE;

                        tel_screen->sub_opcode = 0;
                        tel_screen_buffs[conn_index].term_type_num =
                                                        TELNET_TERM_UNKNOWN;
                        tel_screen->type_ptr = 0;
                        memset (tel_screen->term_type_str, 0, TERM_TYPE_LEN);
                        tcp_send (id, sub_send, 6, FL_PSH);
                     }
                  }

                  if (tel_screen_buffs[conn_index].term_type_num !=
                                                        TELNET_TERM_UNKNOWN)
                  {
                     if ((tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_VT100) ||
                         (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_VT220) )
                     {
#ifdef UNI_VT
                        //if (tcpip.tel_send_welcome == 'Y')
                        //   vt100_welcome (conn_index);
#endif
                        /* Set all DEC VTs to vt100 */
                        tel_screen_buffs[conn_index].term_type_num =
                                                        TELNET_TERM_VT100;
                     }

                     /* Find out what we can and can not do in both
                        directions. */
                     send_opt (id, TELNET_WILL, TELOPT_BINARY,
                                                        TELOPT_WILL_ASKING);

/**** DWN 6/10/94 ****/
                     if (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_VT100)
                        send_opt (id, TELNET_WILL, TELOPT_ECHO,
                                                        TELOPT_WILL_ASKING);
                     else
                        tel_screen->tel_opts[TELOPT_ECHO] = 0;

                     send_opt (id, TELNET_WILL, TELOPT_SGA, TELOPT_WILL_ASKING);
                     send_opt (id, TELNET_DO, TELOPT_STATUS, TELOPT_DO_ASKING);

                     if (!(tel_screen->tel_opts[TELOPT_SNDLOC] & TELOPT_ACTIVE_IN))
                     {
                        tel_screen->sub_opcode = 0;
                        tel_screen->loc_ptr = 0;
                        memset (tel_screen->term_loc, NULL, TERM_LOC_LEN);
                        send_opt (id, TELNET_DO, TELOPT_SNDLOC, TELOPT_DO_ASKING);
                     }

                     if (tel_screen_buffs[conn_index].term_type_num !=
                                                        TELNET_TERM_VT100)
                     {
                        tel_screen->tel_opts[TELOPT_EOR] = TELOPT_SUPPORTED_IN |
                                                           TELOPT_SUPPORTED_OUT;
                        send_opt (id, TELNET_WILL, TELOPT_EOR, TELOPT_WILL_ASKING);
                     }

                     send_opt (id, TELNET_DO, TELOPT_BINARY, TELOPT_DO_ASKING);
                     if ((tel_screen_buffs[conn_index].term_type_num !=
                                                        TELNET_TERM_3278) &&
                         (tel_screen_buffs[conn_index].term_type_num !=
                                                        TELNET_TERM_5250))
                        send_opt (id, TELNET_DO, TELOPT_SGA, TELOPT_DO_ASKING);

                     if (tel_screen_buffs[conn_index].term_type_num !=
                                                        TELNET_TERM_VT100)
                        send_opt (id, TELNET_DO, TELOPT_EOR, TELOPT_DO_ASKING);

#if 0
                     if ( (tcpip.tel_send_welcome == 'Y') &&
/***  dae 2/28/95  */
                          ( (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_TD830) ||
                            (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_TD830A) ) )

                        td830_welcome (id, conn_index);
#endif
                  }
               }
               else if (tel_screen->sub_opcode == TELOPT_SNDLOC)
               {
                  /* Fill out with spaces */
                  memset (tel_screen->term_loc+tel_screen->loc_ptr,
                                SP, TERM_LOC_LEN-tel_screen->loc_ptr);

                  tel_screen->sub_opcode = 0;

                  if ( (!ibm3270_enabled &&
                        (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_3278)) ||
                       (!ibm5250_enabled &&
                        (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_5250)) ||
                       (!dec_vt_enabled &&
                         ((tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_VT100) ||
                          (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_VT220))) )
                  {
                     sprintf ((char *)scratch_buff,"UNSUPP:%s\0",
                                            tel_screen->term_type_str);
                     prn_err_wind (0, scratch_buff);
                     tel_abort (conn_index, id, 11);
                     i = len;
                  }
                  else
                     assign_station (conn_index);
               }
            }
            else
               tel_screen->telnet_state = TELSTATE_SB;
            i++;

            continue;           /* END of TELSTATE_SB_IAC case */

         default:

            if (txt[i] == TELNET_IAC)
            {
               tel_screen->telnet_state = TELSTATE_IAC;
               i++;
            }
#ifdef UNI_VT
#ifdef IBM
            else if (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_3278)
               foundit = ibm3270_parse_input (conn_index, &i, &len,
                                              txt);
            else if (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_5250)
               foundit = ibm5250_parse_input (conn_index, &i, &len, txt);
#endif
            else if (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_3278
                     || tel_screen_buffs[conn_index].term_type_num ==
                     TELNET_TERM_5250) {
               // Added by MRR
               tel_screen->buffer[tel_screen->scr_pos++] =
                  txt[i++];
            } else if (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_VT100)
               foundit = vt100_parse_input (conn_index, &i, &len,
                                            txt);
#endif

#ifdef UNI_TD
            else if ( (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_TD830) ||
                      (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_TD830A) ||
                      (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_PRT132) )
            {
               tel_screen->buffer[tel_screen->scr_pos++] =
                  txt[i++];

               /* Added for BACOB emulation problems (no FF EF) */
               if (tel_screen->scr_pos >= SCRBUFSIZE)
                  foundit = TRUE;
            }
#endif
            else
            {
               sprintf ((char *)scratch_buff,
                        "TEL: UNK TTYP #%d\0",
                        tel_screen_buffs[conn_index].term_type_num);
               prn_err_wind (0, scratch_buff);

               /* Just get rid of the data since we don't know what type it
                  is. 3-22-93 */
               //if (tcpip.tel_no_sndloc_action == '0')
               if (1)
               {
                  tel_abort (conn_index, id, 4);
                  i = len;
               }
               else
                  i++;
            }

            continue;           /* END of TELSTATE_DATA case */

         }                      /* END of switch(tel_screen->telnet_state) */

      }                         /* END of while ((!foundit) && (i < len)) */

      cur_page = tel_screen_buffs[conn_index].cur_page;

/** added by DAE 2/22/94  **/
store_screen_buf (tel_screen_buffs[conn_index].buf_type[cur_page],
        tel_screen_buffs[conn_index].buf_num[cur_page]);

      /* Set up packet to send to host */
      if (foundit)
      {
         if (tel_screen_buffs[conn_index].stn_no == 0)
            tel_abort (conn_index, id, 5);
         else
            send_to_host = TRUE;
            //em_send_input_to_srv (conn_index);
      }

      break;

   case SG_ESTAB:
      telsrv_estab_conn(id);

      break;

   case SG_FIN:
   case SG_CLOSE:
   case SG_REFUSE:
   case SG_DEAD:
      if (Debug>1)
         MDebug ("SG_FIN/SG_CLOSE/SG_DEAD/SG_REFUSE telnet_server\n");

      sprintf ((char *)scratch_buff, "TEL STA %d Discon.\0", tel_screen_buffs[conn_index].stn_no);
      prn_err_wind (0, scratch_buff);

      if (sig == SG_FIN)
         tcp_close (id);
      else if ((sig == SG_DEAD) || (sig == SG_REFUSE))
         tcp_abort (id);

      /* Make sure these things only happen once DAE 3/9/92 */
      if (tel_screen_buffs[conn_index].conn_id != -1)
      {
         tel_screen_buffs[conn_index].conn_id = -1;

#ifdef TELCLT
         ibm_stn = find_stn_link (tel_screen_buffs[conn_index].stn_no, cluster.myid);
         if ( (ibm_stn != 0) &&
              (qwik[ibm_stn].conn_index != 0) &&
              (tel_screen_buffs[qwik[ibm_stn].conn_index].conn_id != -1) )
            tcp_close (tel_screen_buffs[qwik[ibm_stn].conn_index].conn_id);
#endif
      }

      store_screen_buf (tel_screen_buffs[conn_index].buf_type[cur_page],
                        tel_screen_buffs[conn_index].buf_num[cur_page]);

      disconnect_station (conn_index);

      if (tel_screen_buffs[conn_index].stn_no != 0)
      {
         //hermes_status[tel_screen_buffs[conn_index].stn_no] = OFFLINE;
         ///qwik[tel_screen_buffs[conn_index].stn_no].conn_index = 0;
         tel_screen_buffs[conn_index].stn_no = 0;
      }

      break;

   case SG_IDLE:
#if 0
      if (Debug>1)
         printf ("SG_IDLE telnet_server\n");

      /* Don't do idle time out if set to 0 */
      //if (tcpip.telnet_idle_time == 0)
      //   return;

      /* Don't count for printers */
      if (tel_screen_buffs[conn_index].term_type_num == TELNET_TERM_PRT132)
         return;

      /* Incr. counter.  If less than telnet_idle_time, return */

      tel_screen_buffs[conn_index].timeout_ticks++;
      //if (tel_screen_buffs[conn_index].timeout_ticks < tcpip.telnet_idle_time)
        // return;

      if (tel_screen_buffs[conn_index].timeout_ticks == tcpip.telnet_idle_time)
      {
         send_text_message (conn_index, "TELNET idle time signoff");
         //net_stats.tcp_idle_timeout++;
         tcp_close (id);
      }
      else
      if (tel_screen_buffs[conn_index].timeout_ticks > tcpip.telnet_idle_time+5)
      {
         tcp_abort (id);
         sprintf (scratch_buff, "TEL STA %d Discon.\0",
                        tel_screen_buffs[conn_index].stn_no);
         prn_err_wind (0, scratch_buff);

         tel_screen_buffs[conn_index].conn_id = -1;

#ifdef TELCLT
         ibm_stn = find_stn_link (tel_screen_buffs[conn_index].stn_no,
                                cluster.myid);
         if ( (ibm_stn != 0) &&
              (qwik[ibm_stn].conn_index != 0) &&
              (tel_screen_buffs[qwik[ibm_stn].conn_index].conn_id != -1) )
            tcp_close (tel_screen_buffs[qwik[ibm_stn].conn_index].conn_id);
#endif
         store_screen_buf (tel_screen_buffs[conn_index].buf_type[cur_page],
                           tel_screen_buffs[conn_index].buf_num[cur_page]);

         disconnect_station (conn_index);
         if (tel_screen_buffs[conn_index].stn_no != 0)
         {
            //hermes_status[tel_screen_buffs[conn_index].stn_no] = OFFLINE;
            qwik[tel_screen_buffs[conn_index].stn_no].conn_index = 0;
            tel_screen_buffs[conn_index].stn_no = 0;
         }
      }
#endif
      break;

   case SG_URG:
      store_screen_buf (tel_screen_buffs[conn_index].buf_type[cur_page],
                        tel_screen_buffs[conn_index].buf_num[cur_page]);

      break;

   }
   return send_to_host;
}

/*--- function telsrv_estab_conn ------------------------------------------------
 *
 *  Initialize things for when an inbound connection has been established.
 *  Taken from SG_ESTAB case in above code.
 */
void
telsrv_estab_conn(int id)
{
   int i, conn_index=id;

   struct telnet_screen *tel_screen;
   int cur_page = tel_screen_buffs[conn_index].cur_page;  
   load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page], 
                   tel_screen_buffs[conn_index].buf_num[cur_page]);

      if (Debug > 1) {
         char msg[200];
         sprintf(msg,"telnet_server establishing conn %d",id);
         MDebug(msg);
      }

      /* First clear out tel_opts buffer */
      for (i = 0; i < NTELOPTS; i++)
         tel_screen->tel_opts[i] = 0;

      /* Now signify which options we will support. */
      tel_screen->tel_opts[TELOPT_BINARY] = TELOPT_SUPPORTED_IN | TELOPT_SUPPORTED_OUT;
      tel_screen->tel_opts[TELOPT_ECHO] =   TELOPT_SUPPORTED_OUT;
      tel_screen->tel_opts[TELOPT_SGA] =    TELOPT_SUPPORTED_IN | TELOPT_SUPPORTED_OUT;
      tel_screen->tel_opts[TELOPT_STATUS] = TELOPT_SUPPORTED_IN;
      tel_screen->tel_opts[TELOPT_SNDLOC] = TELOPT_SUPPORTED_IN;
      tel_screen->tel_opts[TELOPT_TTYPE] =  TELOPT_SUPPORTED_IN;
      /* Set if TD830 or 3278 is detected in code above. */
      tel_screen->tel_opts[TELOPT_EOR] = 0;

      /* Set some page specific things. */
      tel_screen_buffs[conn_index].term_type_num = TELNET_TERM_UNKNOWN;
      tel_screen->type_ptr = 0;

      for (i = 0; i < TERM_TYPE_LEN; i++)
      {
         tel_screen->term_last_type_str[i] = 0;
         tel_screen->term_type_str[i] = 0;
      }

      memset (tel_screen->term_loc, NULL, TERM_LOC_LEN);

      /* The following were added for BACOB
            DAE   2/23/94    */
      tel_screen->term_mode = TELTERM_DATA;
      tel_screen->telnet_state = TELSTATE_DATA;
      tel_screen->scr_pos = 0;
      tel_screen->sub_opcode = 0;
      tel_screen->loc_ptr = 0;
      memset (tel_screen->buffer, SP, SCRSIZE);

#ifdef UNI_VT
      em_init_page (tel_screen);

      tel_screen->home_pos = 0;
      tel_screen->cntl_esc = 0;
      tel_screen->subesc = 0;

      tel_screen_buffs[conn_index].term_state = TERMSTATE_RECV1;
      tel_screen_buffs[conn_index].stn_no = conn_index; // MRR: was 0;
      tel_screen_buffs[conn_index].status_state = 0;//tcpip.vt_status_state & 3;
      tel_screen_buffs[conn_index].auto_flip =  0;//(tcpip.vt_auto_flip== 'Y');
      tel_screen_buffs[conn_index].line_25 =  1;//(tcpip.vt_line_25== 'Y');
      tel_screen_buffs[conn_index].des_bs =  1;//(tcpip.vt_des_bs == 'Y');

      tel_screen_buffs[conn_index].aid = 0;
      tel_screen_buffs[conn_index].upshift =   0;//(tcpip.ibm_upshift == 'Y');
      tel_screen_buffs[conn_index].ext_attr =  0;//(tcpip.ibm_ext_attr == 'Y');
      tel_screen_buffs[conn_index].zero_fill = 0;//(tcpip.ibm_zero_fill == 'Y');
      tel_screen_buffs[conn_index].full_xmit = 0;//(tcpip.vt_full_xmit == 'Y');
      tel_screen_buffs[conn_index].alt_delim = 0;//(tcpip.vt_alt_delims == 'Y');
      tel_screen_buffs[conn_index].sound_bell= 0;//(tcpip.vt_sound_bell == 'Y');
      tel_screen_buffs[conn_index].cntl[0] = 0;
      tel_screen_buffs[conn_index].timeout_ticks = 0;
      store_screen_buf (tel_screen_buffs[conn_index].buf_type[cur_page],
                        tel_screen_buffs[conn_index].buf_num[cur_page]);

      for (i = 1; i < MAX_PAGES; i++)
      {
         if (tel_screen_buffs[conn_index].buf_num[i] != -1)
         {
            /* Set a "flag" indicating that a new user will be using an
               already used an allocated buffer. (New page function.) */
            load_screen_buf (tel_screen_buffs[conn_index].buf_type[i],
                             tel_screen_buffs[conn_index].buf_num[i]);
            tel_screen->buffer[0] = NULL;
            store_screen_buf (tel_screen_buffs[conn_index].buf_type[i],
                              tel_screen_buffs[conn_index].buf_num[i]);
         }
      }

      load_screen_buf (tel_screen_buffs[conn_index].buf_type[cur_page],
                       tel_screen_buffs[conn_index].buf_num[cur_page]);
#endif

      send_opt (id, TELNET_DO, TELOPT_TTYPE, TELOPT_DO_ASKING);

      total_telnet_sessions++;

      store_screen_buf (tel_screen_buffs[conn_index].buf_type[cur_page],
                        tel_screen_buffs[conn_index].buf_num[cur_page]);
}

/* =========================================================================
= $BEGDOC  send_opt 2 CR  Send an option negotiation sequence
*/

void send_opt
   (   int16 id,
       int16 option,
       int16 opcode,
       int16 ask)
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
 /* Strings that get filled and sent out. */
   uchar8 doopt[] = {TELNET_IAC, TELNET_DO, '\0', '\0'};
   uchar8 dont[] = {TELNET_IAC, TELNET_DONT, '\0', '\0'};
   uchar8 will[] = {TELNET_IAC, TELNET_WILL, '\0', '\0'};
   uchar8 wont[] = {TELNET_IAC, TELNET_WONT, '\0', '\0'};

   int conn_index=id;

   struct telnet_screen *tel_screen;
   int cur_page = tel_screen_buffs[conn_index].cur_page;  
   load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page], 
                   tel_screen_buffs[conn_index].buf_num[cur_page]);
    switch (option)
    {
        case TELNET_WILL:
            if ( !(tel_screen->tel_opts[opcode] & TELOPT_ACTIVE_OUT) )
            {
                will[2] = (uchar8)opcode;
                tel_screen->tel_opts[opcode] |= TELOPT_ACTIVE_OUT | ask;
                tcp_send (id, will, 3, FL_PSH);
            }
            break;

        case TELNET_WONT:
            wont[2] = (uchar8)opcode;
            if (opcode <= NTELOPTS)
                tel_screen->tel_opts[opcode] ^= TELOPT_ACTIVE_OUT;
            tcp_send (id, wont, 3, FL_PSH);
            break;

        case TELNET_DO:
            if ( !(tel_screen->tel_opts[opcode] & TELOPT_ACTIVE_IN) )
            {
                doopt[2] = (uchar8)opcode;
                tel_screen->tel_opts[opcode] |= TELOPT_ACTIVE_IN | ask;
                tcp_send (id, doopt, 3, FL_PSH);
            }
            break;

        case TELNET_DONT:
            dont[2] = (uchar8)opcode;
            if (opcode <= NTELOPTS)
                tel_screen->tel_opts[opcode] ^= TELOPT_ACTIVE_IN;
            tcp_send (id, dont, 3, FL_PSH);
            break;
    }

}

/* =========================================================================
= $BEGDOC  td830_welcome 2 CR  send welcome screen to TD830
*/

void td830_welcome
   (int16 id , int16 conn_index)
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

   unsigned char scratch_buff[BUFFSIZE];
    short   j;

    j = 0;
    j = j + sprintf ((char *)&scratch_buff[j],
                     "%c%cX%c%c\"0)\0",
                     DC1, ESC, FF, ESC);

    j = j + sprintf ((char *)&scratch_buff[j],
       "S T A N D A R D   N E T W O R K S ,   I N C .");

    j = j + sprintf ((char *)&scratch_buff[j],
                     "%c\"C+T D 8 3 0\0", ESC);

    j = j + sprintf ((char *)&scratch_buff[j],
           "%c\"8-F a k e   T E R M I N A L   S U P P O R T",
                     ESC);

    //j = j + sprintf (&scratch_buff[j],
    //                 "%c\"@/Version: %s%c\"  \0",
    //                 ESC, version, ESC);

    j = 0;


    if (tel_screen_buffs[conn_index].term_type_num == TELNET_TERM_TD830)
        while (scratch_buff[j] != 0)
        {
            translate (asc2ebc, &scratch_buff[j], &scratch_buff[j]);
            j++;
        }
    else
        j = strlen((char *)scratch_buff);

    scratch_buff[j++] = TELNET_IAC;
    scratch_buff[j++] = TELNET_EOR;

    tcp_send (id, scratch_buff, j, FL_PSH);


}

void
tel_server_close_conn(int conn_index)
{
   tel_screen_buffs[conn_index].conn_id = -1;
   tel_screen_buffs[conn_index].stn_no = 0;
}
/* =========================================================================
= $BEGDOC  tel_abort 2 CR  code to clean up a TELNET conn after an abort
*/

void tel_abort
   (int16 conn_index,
       int16 id,
       int16 type)
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

#if 0
   short         stn_no;
   short         i;
   unsigned long fhost;
   char          qmsg[55];

   TCB tcb_buff;

   tcp_status (tel_screen_buffs[conn_index].conn_id, &tcb_buff);
   fhost = tcb_buff.fhost;

   stn_no = tel_screen_buffs[conn_index].stn_no;

   if (tcpip.tel_quarantine == 'Y')
      strcpy (qmsg, "Quarantine.");
   else
      strcpy (qmsg, "Disconn.");
   if (stn_no == 0)
   {
      strcpy (scratch_buff, "TEL STA ");
      strcat (scratch_buff, qmsg);
      prn_err_wind (0, scratch_buff);
      memset (scratch_buff, ' ', 21);
      scratch_buff[21] = 0;
      i = 2;
      if (tel_screen->term_loc[0] != NULL)
      {
         memcpy (&scratch_buff[2], tel_screen->term_loc, TERM_LOC_LEN);
         i = TERM_LOC_LEN+2;
         while ( (i>2) && (scratch_buff[i-1] == ' ') )
            i--;
      }
      scratch_buff[i] = '@';
      get_hostname (fhost, &scratch_buff[i+1]);
      if (strlen(scratch_buff) > 21)
      {
         memcpy (scratch_buff+100, scratch_buff+i, 21);
         memset (scratch_buff+i, ' ', 21);
         prn_err_wind (0, scratch_buff);
         memcpy (scratch_buff+2, scratch_buff+100, 21);
      }
      prn_err_wind (0, scratch_buff);
   }
   else
   {
      sprintf (scratch_buff, "TEL STA %d %s", stn_no, qmsg);
      prn_err_wind (0, scratch_buff);
      qwik[stn_no].conn_index = 0;
      tel_screen_buffs[conn_index].stn_no = 0;
      //hermes_status[stn_no] = OFFLINE;
   }

   switch (type)
   {
     case 1:
        strcpy (qmsg, "  Clt wont do SNDLOC");
        break;

     case 2:
        strcpy (qmsg, "  Dup Station Name");
        break;

     case 3:
        strcpy (qmsg, "  No Name match");
        break;

     case 4:
        strcpy (qmsg, "  Unknown Term Type");
        break;

     case 5:
        strcpy (qmsg, "  Unassigned Station");
        break;

     case 6:
        strcpy (qmsg, "  No Sessions match");
        break;

     case 7:
        strcpy (qmsg, "  Station in use");
        break;

     case 8:
        strcpy (qmsg, "  Term/Printer match");
        break;

     case 9:
        strcpy (qmsg, "  IP match required");
        break;

     case 10:
        strcpy (qmsg, "  Station is not up");
        break;

     case 11:
        strcpy (qmsg, "  Type not Supported");

   }


   prn_err_wind (0, qmsg);
   strcat (qmsg, "-  Unable to Attach to Station.");
   send_text_message (conn_index, qmsg);

   if (tcpip.tel_quarantine != 'Y')
   {
       /*** Note: This is here to ensure the session aborts when ***
        ***       after the message goes out.                    ***/

       tel_screen_buffs[conn_index].conn_id = -1;

       tcp_close (id);
   }
#else
       tel_screen_buffs[conn_index].conn_id = -1;
#endif
}



/* =========================================================================
= $BEGDOC  send_buff_to_host 2 CR  send data to the host
 *   Not used in FkUG
*/

void send_buff_to_host
   (int16 conn_index,
       boolean mcs,
       unsigned char dest_host,
       uint8 page)
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

#if 0
   int16    i;
   int16    prog_ind;
   int16    stn_no;
   struct message recv_buffer;
   char msg[200];

   proc_trace ("send_buff_to_host");

   stn_no = tel_screen_buffs[conn_index].stn_no;

   if (page != MAX_PAGES+1)
      store_screen_buf (tel_screen_buffs[conn_index].buf_type[page],
                        tel_screen_buffs[conn_index].buf_num[page]);

   recv_buffer.msglen = strlen (recv_buffer.buff);
    if (recv_buffer.msglen > (BUFFSIZE - 8))
    {
        //poscurs(2, 24);
        sprintf(msg,"BIG err: send_buff_to_host len: %d  stn: %d   ",
                recv_buffer.msglen, stn_no);
        MDebug(msg);
        recv_buffer.msglen = (BUFFSIZE - 8);
    }

   for (i = 0; i < recv_buffer.msglen; i++)
      translate (asc2ebc, &recv_buffer.buff[i],
                 &recv_buffer.buff[i]);

   if (dest_host == 0)
   {
      //recv_buffer.hostno = cluster.myhost;

      prog_ind = qwik[stn_no].currentprogram_index;

      if (prog_ind != 0)
      {
         memcpy (recv_buffer.progname, pt[prog_ind].name, 18);
         recv_buffer.hostno = pt[prog_ind].host_num;
      }
   }

   recv_buffer.fepno = cluster.myid;
   recv_buffer.stn = stn_no;
   recv_buffer.termtype = qwik[stn_no].ndltype;
   recv_buffer.station_bits = 0;
   recv_buffer.buff[recv_buffer.msglen] = ETX;
   recv_buffer.opcode[0] = MESSAGE_OP;
   recv_buffer.opcode[1] = MESSAGE_FROM_STATION;
   recv_buffer.original_opcode[0] = 0;
   recv_buffer.original_opcode[1] = 0;
   ///stats[stn_no].recvcount++;

   if (mcs)
      send_2_host (&recv_buffer);

   if (page != MAX_PAGES+1)
      load_screen_buf (tel_screen_buffs[conn_index].buf_type[page],
                        tel_screen_buffs[conn_index].buf_num[page]);
#endif
}


/* =========================================================================
= $BEGDOC  assign_station 2 CR  Pick a station to assign to tel_screen
*/

void assign_station
   (int16 conn_index)
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
#if 0
    unsigned short stn_no;
    unsigned long  fhost;
    unsigned short lport;
    unsigned char  locat[20];
    unsigned short progindex;
    unsigned short i;
    unsigned short id;
    unsigned char  found;
    unsigned char  found_ip;
    unsigned char *p;
    TCB tcb_buff;

   struct telnet_screen *tel_screen;
   int cur_page = tel_screen_buffs[conn_index].cur_page;  
   load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page], 
                   tel_screen_buffs[conn_index].buf_num[cur_page]);

   id = tel_screen_buffs[conn_index].conn_id;
    tcp_status (id, &tcb_buff);
    fhost = tcb_buff.fhost;
    lport = tcb_buff.lport;

    /** First look for at Station name match **/

    found = FALSE;
    if (tel_screen->term_loc[0] != NULL)
    {
        for (stn_no = 1; stn_no <= sessionlimit; stn_no++)
        {
/** DWN 11/13/95 Allow "blah/STNNAME" to match STNNAME. */
            if (qwik[stn_no].status == 0xff)
                continue;
#ifdef DCOM
            if (qwik[stn_no].lineno != 0)
                continue;
#endif

            p = NULL;
            if ( (memcmp(qwik[stn_no].stn_name,tel_screen->term_loc,18) == 0) ||
                 ( ((p=(unsigned char *)memchr(qwik[stn_no].stn_name, '/', 18)) != NULL) &&
                   (memcmp(p+1, tel_screen->term_loc,
                                    18-(p-qwik[stn_no].stn_name)-1) == 0) ) )
            {
                if (qwik[stn_no].status != UP)
                {
                    if (p != NULL)
                        continue;
                    tel_abort (conn_index, id, 10);
                    return;
                }

                if ( (qwik[stn_no].fhost != 0)       &&
                     (qwik[stn_no].fhost != fhost) )
                {
                    if (p != NULL)
                        continue;
                    //if (tcpip.tel_need_ip_match != 'Y')
                    if(1)
                        break;
                    tel_abort (conn_index, id, 9);
                    return;
                }

                if (qwik[stn_no].conn_index != 0)
                {
                    if (qwik[stn_no].fhost != fhost)
                    {
                        tel_abort (conn_index, id, 7);
                        return;
                    }
/***  DWN 10/25/94 For BACOB: if name & IP match, clear the old session
                   and give it to the new session
***/
                    i = qwik[stn_no].conn_index;
/***  DWN 12/6/94 *** consistancy check before killing session ***/
                    if (tel_screen_buffs[i].stn_no == stn_no)
                    {
                        if (tel_screen_buffs[i].conn_id != -1)
                            tcp_abort (tel_screen_buffs[i].conn_id);
                        tel_screen_buffs[i].conn_id = -1;
                        tel_screen_buffs[i].stn_no = 0;

                        sprintf ((char *)scratch_buff, "TEL STA %d Re-con.\0", stn_no);
                        prn_err_wind (0, scratch_buff);

                    }
                    //hermes_status[stn_no] = OFFLINE;
                    qwik[stn_no].conn_index = 0;
                }

                if ( (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_PRT132) &&
                     (qwik[stn_no].term_type != 4) )
                {
                    tel_abort (conn_index, id, 8);
                    return;
                }
                if ( (tel_screen_buffs[conn_index].term_type_num !=
                                                        TELNET_TERM_PRT132) &&
                     (qwik[stn_no].term_type != 1) &&
                     (qwik[stn_no].term_type != 10) )
                {
                    tel_abort (conn_index, id, 8);
                    return;
                }
                found = TRUE;
                break;
            }
        }

#if 0
        if (!found && (tcpip.tel_no_match_action == '0') )
        {
            tel_abort (conn_index, id, 3);
            return;
        }
#endif
    }

    /** Next look for an IP match **/

    if (!found)
    {
        found_ip = FALSE;
        for (stn_no = 1; stn_no <= sessionlimit; stn_no++)
        {
            if ( (qwik[stn_no].status != 0xff) &&
#ifdef DCOM
                 (qwik[stn_no].lineno == 0) &&
#endif
                 (qwik[stn_no].fport == lport) &&
                 (qwik[stn_no].fhost == fhost) )
            {
/**
    9/26/94 DAE
                found_ip = TRUE;
**/

                if ( (qwik[stn_no].status != UP)    ||
                     (qwik[stn_no].conn_index != 0) ||
                     ( (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_PRT132) &&
                       (qwik[stn_no].term_type != 4) ) ||
                     ( (tel_screen_buffs[conn_index].term_type_num !=
                                                        TELNET_TERM_PRT132) &&
                       (qwik[stn_no].term_type != 1) &&
                       (qwik[stn_no].term_type != 10) ) )
                    continue;
                found = TRUE;
                break;
            }
        }

        /* 9/26/94  DAE
                Changed found_ip to !found_ip   ***/
        if (!found && !found_ip && (tcpip.tel_need_ip_match == 'Y') )
        {
            tel_abort (conn_index, id, 9);
            return;
        }
    }

    if (!found)
    {
        for (stn_no = 1; stn_no <= sessionlimit; stn_no++)
        {
            if ( (qwik[stn_no].status != 0xff) &&
#ifdef DCOM
                 (qwik[stn_no].lineno == 0) &&
#endif
                 (qwik[stn_no].fport == lport) &&
                 (qwik[stn_no].fhost == 0) )
            {
                if ( (qwik[stn_no].status != UP)    ||
                     (qwik[stn_no].conn_index != 0) ||
                     ( (tel_screen_buffs[conn_index].term_type_num ==
                                                        TELNET_TERM_PRT132) &&
                       (qwik[stn_no].term_type != 4) ) ||
                     ( (tel_screen_buffs[conn_index].term_type_num !=
                                                        TELNET_TERM_PRT132) &&
                       (qwik[stn_no].term_type != 1) &&
                       (qwik[stn_no].term_type != 10) ) )
                    continue;
                found = TRUE;
                break;
            }
        }

        if (!found)
        {
            tel_abort (conn_index, id, 6);
            return;
        }
    }

    //load_station_entry(stn_no, 2);

    if ( ( (tel_screen->term_loc[0] == NULL) &&
           (tcpip.tel_no_sndloc_action == '2') ) ||
         ( (tel_screen->term_loc[0] != NULL) &&
           (memcmp(station_table_ex.name,tel_screen->term_loc,18) != 0) &&
           (tcpip.tel_no_match_action == '2') ) )
    {
        /* Create new name either from IP address or SND LOC response. */
        memset(locat, NULL, 20);
        if (tel_screen->term_loc[0] == NULL)
            sprintf (locat, "IP%3.3d%3.3d%3.3d%3.3d    ",
                (fhost & 0xff000000) >> 24,
                (fhost & 0x00ff0000) >> 16,
                (fhost & 0x0000ff00) >> 8,
                (fhost & 0x000000ff) );
/**
    This was above.
    DAE
    3/15/95
                (qwik[stn_no].fhost & 0xff000000) >> 24,
                (qwik[stn_no].fhost & 0x00ff0000) >> 16,
                (qwik[stn_no].fhost & 0x0000ff00) >> 8,
                (qwik[stn_no].fhost & 0x000000ff) );
**/
        else
            memcpy (locat, tel_screen->term_loc, TERM_LOC_LEN);

        /* Check and see if this name is in use, if it is abort the
            session. */
        for (i=1; i < sessionlimit; i++)
        {
            /* Don't look at our station, or stations not in use. */
            if ( (i != stn_no) &&
                 (tel_screen_buffs[i].stn_no != 0) )
            {
                load_screen_buf (tel_screen_buffs[i].buf_type[0],
                                 tel_screen_buffs[i].buf_num[0]);
                if (memcmp(tel_screen->term_loc, locat, TERM_LOC_LEN) == 0)

                {
                    tel_abort (conn_index, id, 2);
                    return;
                }
            }
        }

        /* Now update things in memory and to the host. */
        load_screen_buf (tel_screen_buffs[conn_index].buf_type[tel_screen_buffs[conn_index].cur_page],
                         tel_screen_buffs[conn_index].buf_num[tel_screen_buffs[conn_index].cur_page]);

        if (tel_screen->term_loc[0] == NULL)
            memcpy (tel_screen->term_loc, locat, TERM_LOC_LEN);

        /* Store in memory, but not disk. */
        memcpy (qwik[stn_no].stn_name, locat, TERM_LOC_LEN);

        store_screen_buf (tel_screen_buffs[conn_index].buf_type[tel_screen_buffs[conn_index].cur_page],
                         tel_screen_buffs[conn_index].buf_num[tel_screen_buffs[conn_index].cur_page]);

/**
    memcpy(stn_entry->name, qwik[which_station].stn_name, 18); was added to
    update_station_on_host just before the upload to the host. for this to work.
**/
        update_station_on_host (stn_no, 1, 2);

        /* We did this above, but need to do it again. */
        load_station_entry(stn_no, 2);
    }

    tel_screen_buffs[conn_index].stn_no = stn_no;
    qwik[stn_no].conn_index = conn_index;
    //hermes_status[stn_no] = ONLINE;

    qwik[stn_no].stationbits = 0;
    progindex = tranfind(station_table_ex.tranattach);
    qwik[stn_no].currentprogram_index = progindex;
    qwik[stn_no].current_host = pt[progindex].host_num;
    qwik[stn_no].local_trancode = tranlocaltran(station_table_ex.tranattach);
    qwik[stn_no].parse_mode = get_number(station_table_ex.boj_parse, 1);


    if (tel_screen->term_loc[0] == NULL)
    {
        memset (locat, NULL, 20);
        get_hostname(fhost, locat);
        if (strlen(locat) > 12)
            sprintf ((char *)scratch_buff, "%d@%s\0", stn_no, locat);
        else
            sprintf ((char *)scratch_buff, "STN:%d@%s\0", stn_no, locat);
    }
    else
        sprintf ((char *)scratch_buff, "STN:%d@%s\0", stn_no, tel_screen->term_loc);

    scratch_buff[20] = NULL;
    prn_err_wind (0, scratch_buff);

    if ( (qwik[stn_no].term_type == 4) &&
         (qwik[stn_no].boj == 2) )
    {
        scratch_buff[0] = 0x00;
        scratch_buff[1] = TELNET_IAC;
        scratch_buff[2] = TELNET_EOR;
        tcp_send(id, scratch_buff, 3, FL_PSH);
        return;
    }

    if ( (qwik[stn_no].term_type == 4) &&
         (qwik[stn_no].boj == 3) )
        return;

    switch (tcpip.tel_connect_action)
    {
        case '0':
            return;

        case '1':
            strcpy (recv_buffer.buff, "?MENU");
            break;

        case '2':
            sprintf (recv_buffer.buff, "?READY %18.18s", qwik[stn_no].stn_name);
            break;

        case '3':
            if (qwik[stn_no].term_type == 4)
                return;
            recv_buffer.fepno = cluster.myid;
            recv_buffer.stn = stn_no;
            recv_buffer.station_bits = 0;
            recv_buffer.opcode[0] = MESSAGE_OP;
            recv_buffer.opcode[1] = MESSAGE_FROM_STATION;
            recv_buffer.original_opcode[0] = 0;
            recv_buffer.original_opcode[1] = 0;
            send_whoami (stn_no, &recv_buffer);
            return;

        case '4':
            if (qwik[stn_no].term_type == 4)
                return;
            //cmdline[0] = NULL;
            recv_buffer.stn = stn_no;
            handle_signon (stn_no);
            return;

        default:
            return;
    }

    if ( ((cluster.environment == '2') || (cluster.environment == '3')) &&
         (progindex > 0) &&
         (namecmp (pt[progindex].name, "ROUTER    ") == 0) )
/**
   Took out 7/13/94  DAE
         (qwik[stn_no].iotype != 1) )
**/

    {
        send_buff_to_host (conn_index, TRUE, 0,
                                tel_screen_buffs[conn_index].cur_page);
    }
#endif
}

/* =========================================================================
= $BEGDOC  disconnect_station 2 CR  Break connection for a station
*/

void disconnect_station
   (int16 conn_index)
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

#if 0
    unsigned short stn_no;
    unsigned short l;

    stn_no = tel_screen_buffs[conn_index].stn_no;

    if (stn_no == 0)
        return;

    if ( (tcpip.tel_no_sndloc_action == '2') ||
           (tcpip.tel_no_match_action == '2') )
    {
        /* See if we have to change a station name back.
            DAE     3/15/95    */
        load_station_entry(stn_no, 2);
        if (memcmp (qwik[stn_no].stn_name, station_table_ex.name,
                                TERM_LOC_LEN) != 0)
        {
            memcpy (qwik[stn_no].stn_name, station_table_ex.name, TERM_LOC_LEN);
            update_station_on_host (stn_no, 1, 2);
        }
    }


    if ( (qwik[stn_no].term_type == 4) &&
         ( (qwik[stn_no].boj == 2) ||
           (qwik[stn_no].boj == 3) ) )
        return;

    switch (tcpip.tel_discon_action)
    {
        case '0':
            return;

        case '1':
            strcpy (scratch_buff, "?MENU");
            break;

        case '2':
            sprintf (scratch_buff, "?CLEAR %18.18s", qwik[stn_no].stn_name);
            break;

        case '3':
            strcpy (scratch_buff, "?FREE");
            break;

        case '4':
            if (qwik[stn_no].user != 0)
            {
                recv_buffer.stn = stn_no;
    /* added 4/8/94   DAE
        to hopefully prevent improper routing (to wrong FEP) of signoff
            messages at COUTTS */
                recv_buffer.fepno = cluster.myid;
                handle_signoff ();
            }
            return;

        default:
            return;
    }

    memcpy (recv_buffer.progname, "ROUTER            ", 18);
    for (l = 1; l < programs_defined+1; l++)
    {
        if ( (namecmp (pt[l].name, recv_buffer.progname) == 0) &&
             (pt[l].status != 0xff) )
        {
            recv_buffer.hostno = pt[l].host_num;
            strcpy(recv_buffer.buff, scratch_buff);
            send_buff_to_host (conn_index, TRUE, 1,
                                tel_screen_buffs[conn_index].cur_page);
        }
    }

    if (qwik[stn_no].user != 0)
    {
        recv_buffer.stn = stn_no;
        recv_buffer.fepno = cluster.myid;
        handle_signoff ();
    }
#endif
}

/* =========================================================================
= $BEGDOC  send_text_message 2 CR  Send a text message to device
*/

void send_text_message
   (int16 conn_index,
    char  *text)
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
    unsigned short  i;
    unsigned short  j;
    unsigned char scratch_buff[BUFFSIZE];

    j = 0;
    switch (tel_screen_buffs[conn_index].term_type_num)
    {
        case TELNET_TERM_VT100:
            j = j + sprintf ((char *)&scratch_buff[j],
                             "%c%c%c%c[1;1H%c[2J%c[0m%c[24;1H\0",
                             BEL, BEL, BEL, ESC, ESC, ESC, ESC);

            j = j + sprintf ((char *)&scratch_buff[j],
                             "%s%c%c%c\0",
                             text,
                             BEL, BEL, BEL);
            break;

        case TELNET_TERM_PRT132:
            break;              /* Suppress messages to printers */

        case TELNET_TERM_TD830:
        case TELNET_TERM_TD830A:
            scratch_buff[j++] = eDC1;

            j = j + sprintf ((char *)&scratch_buff[j],
                             "%s%c%c\0",
                             text,
                             BEL, BEL);

            if (tel_screen_buffs[conn_index].term_type_num == TELNET_TERM_TD830)
                for (i = 0; i < j; i++)
                   translate (asc2ebc, &scratch_buff[i], &scratch_buff[i]);
            scratch_buff[j++] = TELNET_IAC;
            scratch_buff[j++] = TELNET_EOR;
            break;

        case TELNET_TERM_3278:
            scratch_buff[j++] = EW;
            scratch_buff[j++] = 0xC6;
            scratch_buff[j++] = 0x11;
            scratch_buff[j++] = 0x40;
            scratch_buff[j++] = 0x40;
            scratch_buff[j++] = 0x13;

            while (*text != NULL)
            {
                translate (asc2ebc, text, &scratch_buff[j]);
                j++;
                text++;
            }

            scratch_buff[j++] = TELNET_IAC;
            scratch_buff[j++] = TELNET_EOR;
            break;

        case TELNET_TERM_5250:
            scratch_buff[j++] = 0;
            scratch_buff[j++] = 0;
            scratch_buff[j++] = 0x12;
            scratch_buff[j++] = 0xA0;
            scratch_buff[j++] = 0x00;
            scratch_buff[j++] = 0x00;
            scratch_buff[j++] = 0x04;
            scratch_buff[j++] = 0x00;
            scratch_buff[j++] = 0x00;
            scratch_buff[j++] = 0x03;
            scratch_buff[j++] = 0x04;
            scratch_buff[j++] = 0x40;
            scratch_buff[j++] = 0x04;
            scratch_buff[j++] = 0x11;
            scratch_buff[j++] = 0x00;
            scratch_buff[j++] = 0x08;

            scratch_buff[j++] = 0x11;
            scratch_buff[j++] = 1;
            scratch_buff[j++] = 0;
            scratch_buff[j++] = 0x1D;
            scratch_buff[j++] = 0x40;
            scratch_buff[j++] = 0x00;
            scratch_buff[j++] = 0x20;
            scratch_buff[j++] = 1919 / 256;
            scratch_buff[j++] = 1919 % 256;

            while (*text != NULL)
            {
                translate (asc2ebc, text, &scratch_buff[j]);
                j++;
                text++;
            }

            scratch_buff[j++] = 0x04;
            scratch_buff[j++] = 0x52;
            scratch_buff[j++] = 0x00;
            scratch_buff[j++] = 0x00;
            scratch_buff[0] = (j) >> 8;
            scratch_buff[1] = (j) & 0xff;
            scratch_buff[j++] = TELNET_IAC;
            scratch_buff[j++] = TELNET_EOR;
            break;

        default:
            j = j + sprintf ((char *)&scratch_buff[j],
                             "%s\r\n\r\n\0",
                             text);

    }

    tcp_send (tel_screen_buffs[conn_index].conn_id, scratch_buff, j, FL_PSH);
}
/*********************************** eof ***********************************/
