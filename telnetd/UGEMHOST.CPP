/***************************************************************************/
/*                                                                         */
/*                      STANDARD NETWORKS, INC.                            */
/*                                                                         */
/*      Filename:  em_host.c                                               */
/*                                                                         */
/*      Description: This file contains that communicate to/from the       */
/*                      host and various emulators.                        */
/*                                                                         */
/*      Revison History                                                    */
/*              Date      By     Description                               */
/*              --------- ---    -----------                               */
/*              20 NOV 91 DAE    Created                                   */
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
#include "ugstuff.h"
#include "mdebug.h"

#define UP      1
#define DOWN    0

extern TypConn *Connection;

extern struct telnet_screen_buffs *tel_screen_buffs;
extern struct telnet_screen **TelScrBuffers;   
extern struct telnet_screen **telclt_buffers;

extern  short     host_audit;

extern  char    UniDebug;
extern int Debug;
extern FILE *stLogFile;

extern  uchar   asc2ebc[256];
extern  uchar   ebc2asc[256];
extern  uchar   defebc2asc[256];

extern BOOL bDontSendBackCANDE;
extern struct struct_ug_globals ug_globals;

//=== Function Prototypes ============================================

void em_init_page(struct telnet_screen *tel_screen);
boolean em_determine_page (int16 conn_index,
				int16 page);
short em_send_input(int conn_index, struct telnet_screen *tel_screen);


#ifdef C1_VT
short tcp_send();
short em_init_buff();
short em_fep_mail();
short prn_err_wind();
#endif

#ifdef UNI_VT

/* =========================================================================
= $BEGDOC  em_init_page CR  Initializes a new page
*/

void em_init_page(struct telnet_screen *tel_screen)
{


/*
=  DESCRIPTION:     This function initializes non-TELNET related fields
=                       of the emulator connection state array.
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

    proc_trace("em_init_page");

    tel_screen->help_count = 0;
    tel_screen->term_mode = TELTERM_DATA;
    tel_screen->telnet_state = TELSTATE_DATA;
    tel_screen->insert_mode = FALSE;

/**** DWN 3/29/94 ***
    for (i = 0; i < STATUS_LEN; i++)
        tel_screen->status_line[i] = SP;
***/
    memset (tel_screen->status_line, SP, STATUS_LEN);

    tel_screen->status_line[STATUS_LEN] = NULL;

    tel_screen->scr_pos = 0;
    tel_screen->sub_opcode = 0;

    // MRR added:
   tel_screen->home_pos = 0;
/**** DWN 3/29/94 ***
    for (i = 0; i < SCRSIZE; i++)
        tel_screen->buffer[i] = SP;
***/
    memset (tel_screen->buffer, SP, SCRSIZE);

}


/*--- function CandeEditOutput -----------------------------------
 *
 *  Perform CANDE editing on a message.
 *
 *  Entry:  inbuff   points to a buffer of ASCII characters to be
 *                   sent to a terminal.
 *          nInChars is the number of characters.
 *          nMaxOut  is the amount of room in inbuff.
 *
 *  Exit:   inbuff   has been replaced with ASCII characters
 *                   that represent the message with CANDE editing
 *                   performed.
 *          nOut     is the number of characters now in the buffer.
 *          Returns zero upon success.
 */
int CandeEditOutput(unsigned char *inbuff, int nInChars, int nMaxOut, 
                    int &nOut)
{
   unsigned char *pin = inbuff, inbyte;
   unsigned char mybuf[2*BUFFSIZE], *pout=mybuf, *pInEnd=pin+nInChars;
   int retval=0, cur_col=0, cur_line=0;
   BOOL bPerformCande=FALSE;

#define STORE_BYTE(x) *(pout++) = x;
#define ADD_ERASE_EOL  STORE_BYTE(ESC); STORE_BYTE('K');

   if(!nInChars) {
      nOut = 0;
      return retval;
   }

   // Special-case the first byte.
   switch(*pin) {
   case DC1:
      memmove(inbuff, inbuff+1, nInChars-1);
      nOut = nInChars-1;
      return 0;
      break;
   case DC4:
   case FF:
   case ESC:
   case BEL:
   case DC3:
      nOut = nInChars;
      return 0;
      break;
   default:
      ADD_ERASE_EOL;
      bPerformCande = TRUE;
      break;
   }

   if(bPerformCande) while(pin < pInEnd) {
      inbyte = *pin;
      switch(inbyte) {
      case DC4:
      case FF:
         cur_col=cur_line=0;
         STORE_BYTE(inbyte);
         break;
      case CR:
         if(*(pin+1) == LF) pin++;
         if(pin < pInEnd) {
            ADD_ERASE_EOL;
            STORE_BYTE(CR);
            cur_col=0;
            cur_line++;
         }
         break;
      case LF:
         if(*(pin+1) == CR) pin++;
         if(pin < pInEnd) {
            ADD_ERASE_EOL;
            STORE_BYTE(LF);
            cur_line++;
         }
         break;
      case ETX:   // End of message
         pin = pInEnd;
         break;
      case ESC:
         STORE_BYTE(ESC);
         pin++;
         // If entering forms mode, just copy rest of buffer;
         if('W' == *pin) {
            while(pin < pInEnd && ETX != *pin) {
               STORE_BYTE(*pin);
               pin++;
            }
         }
         break;
      case NUL:
         STORE_BYTE(NUL);
         break;
      default:
         STORE_BYTE(inbyte);
         cur_col++;
         // Account for wrapping to the next line.
         if(cur_col >= 80) {
            cur_col = 0;
            cur_line++;
            // Ignore CR/LF if we have just wrapped.  
            // Boy, this is stupid.
            if(CR == *(pin+1)) {
               pin++;
               if(LF == *(pin+1)) {
                  pin++;
               }
            }
         }
         break;

      }  // end switch
      pin++;
   } // end while
   // End buffer with Clear EOL unless at bottom.
   if(cur_line < 23 || (cur_line==23 && cur_col<79)) {
      ADD_ERASE_EOL;
      STORE_BYTE(CR);
   }
   // End buffer with ETX.
   if(*(pout-1) != ETX) STORE_BYTE(ETX);
   // Compute # of bytes to copy back into user's buffer.
   nOut = pout - mybuf;
   if(nOut > nMaxOut) nOut = nMaxOut;
   memcpy(inbuff, mybuf, nOut);
   return retval;
}

/* =========================================================================
= $BEGDOC  em_from_host CR  Deal with a data buffer from the host
*/

/*--- function em_from_host -----------------------------------------
 *
 *  Exit:   Returns zero upon success.
 */
short em_from_host(short sn, struct message *textptr)
{


/*
=  DESCRIPTION: This function interprets data received from the host
=                   system for conversion before sending out to a
=                   emulated terminal.
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

    boolean done;
    boolean erase;
    boolean esc_ra;
    boolean found_dc1;
    boolean full_screen;
    boolean forms_mode;
    boolean line_buff[24];
    boolean new_alloc;
    boolean send_back;  /* ESC ( */
    boolean status_state;

    //short     hold_status_state;
    short     i, k, m;
    int     length, length_new;

    int16   conn_index;
    int16   cur_col;
    int16   cur_line;
    int16   end_pos;
    //int16   hold;
    int16   last_line;
    int16   line, col;
    //int16   new_lines;
    int16   scr_pos;
    int16   start_pos;
    int16   status_count;
    int16   status_position;
    int16   swap_line;
    //int16   tcb_id;

    uchar8  *buffptr;
    uchar8  alt_start, alt_end;
    uchar8  save_buff[80];

    uint8   cur_page, new_page, orig_page;

    uint32 forms_field;

    struct  message  *dataptr;

    /* Locals added by MRR */
    uchar8      scratch_buff[BUFFSIZE];
    struct telnet_screen *tel_screen;
    unsigned short firstbyte, bytesleft;
    short end_scr=FALSE;
    TypConn *pConn = &Connection[sn];
        
    length = textptr->msglen;
    bytesleft = textptr->bytesleft;
    firstbyte = length - bytesleft;

   if(pConn->ConnDebug>1) {
      int j;

      FmtDebug("UGEMHOST called with %d bytes:",bytesleft);
      char mybuf[8000];
      int k=0;
      for(j=firstbyte; j<length; j++, k++) {
         mybuf[k] = ebc2asc[textptr->buff[j]];
      }
      mybuf[k] = 0;
      MDebug(mybuf);
   }

#ifdef TELNET_IDLE_KLUDGE
   // This is a kludge to deal with Waukesha State Bank.
   // If this is subsequent processing on a message we've already seen,
   // and the remaining bytes in the message are all tabs, ignore the
   // message.

   if(firstbyte>0) {
      for(k=firstbyte; k<length; k++) {
         if(textptr->buff[k] != HT) goto notHT;
      }
      textptr->bytesleft = 0;
      return end_scr;
notHT: ;
   }
#endif
   /**
    TOOK OUT 12/6/93 DAE
    if (host_audit)
        audit_rec('j',textptr,0);
   **/

    conn_index = sn;

    if (tel_screen_buffs[conn_index].conn_id == -1)
        return(1);

    dataptr = textptr;

    if (tel_screen_buffs[conn_index].line_25)
        last_line = 25;
    else
        last_line = 24;

    cur_page = tel_screen_buffs[conn_index].cur_page;
    /* Save KBC */
    orig_page = cur_page;

    /* Get DCP */
    cur_page = tel_screen_buffs[conn_index].term_state - 1;
   // Added by MRR to take care of emulated TD830 screens here in 
   // EMHOST, rather than telnet screens in TELCLT.
   /* Load the screen.  If it has never been allocated, allocate one. */
   if (tel_screen_buffs[conn_index].buf_num[cur_page] == -1) {
      alloc_screen_buf(&tel_screen_buffs[conn_index].buf_type[cur_page],
          (unsigned short *)&tel_screen_buffs[conn_index].buf_num[cur_page]);
      load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page],
                    tel_screen_buffs[conn_index].buf_num[cur_page]);
      if(tel_screen) {
         em_init_page(tel_screen);
      } 
   }

    load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page],
                    tel_screen_buffs[conn_index].buf_num[cur_page]);
    if(pConn->ConnDebug>3) {
       FmtDebug("in em_from_host, firstbyte=%d length=%d bytesleft=%d",
          firstbyte,length,bytesleft);
    }
    if(!firstbyte) {
       // If necessary (for logging in TD830W), log this buffer receipt
       UGLogFromHost(sn,dataptr->buff,length);

       // If this is the first time we've been called with this
       // message, translate it.
       for(i=0; i < length; i++)
       {
           if ( (i > firstbyte+1) &&
                (dataptr->buff[i-2] == ESC) &&
                (dataptr->buff[i-1] == '"') )
           {
               translate(defebc2asc,&dataptr->buff[i],&dataptr->buff[i]);
               i++;
               translate(defebc2asc,&dataptr->buff[i],&dataptr->buff[i]);
           }
   /*** dwn 2/1/96 ** Handle page-change sequence with dflt translate ***/
           else
           if ( (i > firstbyte+1) &&
                (dataptr->buff[i-2] == ESC) &&
                (dataptr->buff[i-1] == '$') )
           {
               translate(defebc2asc,&dataptr->buff[i],&dataptr->buff[i]);
           }
           else
           if ( (i > firstbyte+0) &&
                (dataptr->buff[i-1] == ESC) )
           {
               translate(defebc2asc,&dataptr->buff[i],&dataptr->buff[i]);
           }
           else
               translate(ebc2asc,&dataptr->buff[i],&dataptr->buff[i]);
       }
       if(pConn->bDoCandeEditing) {
          CandeEditOutput(dataptr->buff, length, sizeof dataptr->buff,
             length_new);
          if(pConn->ConnDebug > 2) {
             FmtDebug("CANDE editing of %d bytes produced %d bytes",
                length, length_new);
             if(pConn->ConnDebug > 3) {
               DoutAsc("CANDE editing resulted in",(char *)dataptr->buff,length_new);
             }
          }
          textptr->msglen = length = length_new;
       }
    }
    status_count = 0;
    buffptr = dataptr->buff;
    scr_pos = tel_screen->scr_pos;
    start_pos = tel_screen->scr_pos;

    if(tel_screen_buffs[conn_index].bGotETXThisMsg) {
       // We should ignore this TCP packet, as there was an ETX 
       // earlier in the message.
       textptr->bytesleft = 0;
       return 0;
    }

/**** DWN 3/29/94 ****
    for (i = 0; i < 24; line_buff[i++] = FALSE);
***/
    memset (line_buff, FALSE, 24);

#ifdef TABS_KLUDGE
    // I don't understand why forms mode is arbitrarily being set 
    // to False.  It seems to contradict the manual & the behavior
    // of the Core emulator.  However, setting Forms mode based on 
    // the previous contents of the screen doesn't work, either.
    // So, I save off the old forms mode here and use it for
    // HT processing.  /mrr 7 Jan 97
    old_forms_mode = (tel_screen->term_mode != TELTERM_DATA);
#endif

    /* All messages from host assume non-forms mode */
    forms_mode = FALSE;
    tel_screen->term_mode = TELTERM_DATA;

    full_screen = FALSE;
    esc_ra = FALSE;
    send_back = FALSE;
    found_dc1 = FALSE;


    status_state = 0;
    /* First fill the screen buffer */
    for (i = firstbyte; i < length; i++)
    {
        switch(buffptr[i])
        {
            case    FF:
                if (!forms_mode)
                {
/**** DWN 3/29/94
                    for (k = 0; k < SCRSIZE; k++)
                    {
                        line_buff[k / 80] = TRUE;
                        tel_screen->buffer[k] = SP;
                    }
****/
                    memset (line_buff, TRUE, 24);
                    memset (tel_screen->buffer, SP, SCRSIZE);

                    scr_pos = 0;
                }
                else
                {
                    erase = FALSE;
                    k = 0;
                    scr_pos = 0;
                    while (k < SCRSIZE)
                    {
                        if ( (tel_screen->buffer[k] == US) ||
                             (tel_screen->buffer[k] == GS) )
                        {
                            if (scr_pos == 0)
                                scr_pos = k + 1;
                            erase = TRUE;
                        }
                        else if (tel_screen->buffer[k] == RS)
                            erase = FALSE;
                        else if (erase)
                        {
                            line_buff[k / 80] = TRUE;
                            tel_screen->buffer[k] = SP;
                        }

                        k++;
                    }
                    tel_screen->scr_pos = scr_pos;
                }
                break;

            case    CR:
                scr_pos = (scr_pos / 80 + 1) * 80;
                if (scr_pos >= SCRSIZE)
                    scr_pos = 0;
                if (buffptr[i+1] == LF)
                    i++;

                break;

            case    LF:     /* Line feed */
                if (scr_pos > (SCRSIZE - 81) )
                    scr_pos -= SCRSIZE;

                scr_pos += 80;

                break;

            case    BS:     /* Back Space */
                scr_pos--;
                if(scr_pos < 0)
                    scr_pos = SCRSIZE - 1;
                break;

            case    HT:
                if (!forms_mode)
                {
                    done = FALSE;
                    while (!done)
                    {
/*
    11/23/92
        DAE
                        tel_screen->buffer[scr_pos++] = SP;
*/
                        scr_pos++;
                        line_buff[scr_pos / 80] = TRUE;

                        if ( (scr_pos % 8) == 0)
                            done = TRUE;
                    }
                }
                else
                {
                    k = scr_pos;
                    done = FALSE;
                    while (!done)
                    {
                        if ( (tel_screen->buffer[k] == US) &&
                             !em_check_for_end(k+1) )
                        {
                            k++;
                            scr_pos = k;
                            tel_screen->term_mode = TELTERM_LEFT_JUST;
                            done = TRUE;
                        }
                        else if ( (tel_screen->buffer[k] == GS) &&
                                  !em_check_for_end(k+1) )
                        {
                            k++;
                            while ( (k < SCRSIZE) &&
                                    !em_check_for_end(k) )
                                k++;
                            k--;

                            scr_pos = k;
                            tel_screen->term_mode = TELTERM_RIGHT_JUST;
                            done = TRUE;
                        }
                        else
                        {
                            k++;
                            if (k > SCRSIZE)
                                k = 0;
                        }
                    }
                }
                break;

            case    NULL:
/*** DWN 8/9/95
            case    DEL:
***/
            case    BEL:
                break;

            case    ETX:    /* End of transmission */
#if 0
                // Save number of bytes remaining in buff if is not the
                // last one in the message.
                dataptr->bytesleft = length-i-1; 
#else
                // ETX means ignore the rest of this message.
                dataptr->bytesleft = 0;
#endif
                tel_screen_buffs[conn_index].bGotETXThisMsg = TRUE;
                end_scr = TRUE;
                i = length+2;  // break out of for loop.
                break;

            case    US:     /* Start left justified field */
            case    GS:     /* Start right justified field */
                if (!forms_mode)
                {
                    line_buff[scr_pos / 80] = TRUE;
                    tel_screen->buffer[scr_pos++] = buffptr[i];
                }
                break;

            case    FS:     /* Start protected field */
                if (!forms_mode)
                {
                    line_buff[scr_pos / 80] = TRUE;
                    tel_screen->buffer[scr_pos++] = buffptr[i];
                }
                break;

            case    RS:     /* End forms field and highlight field */
                if (!forms_mode)
                {
                    line_buff[scr_pos / 80] = TRUE;
                    tel_screen->buffer[scr_pos++] = buffptr[i];
                }
                break;
#if 0
            case    ETB:    /* Ends highlight */
                line_buff[scr_pos / 80] = TRUE;
                tel_screen->buffer[scr_pos++] = buffptr[i];
                break;

            case    EM:     /* Starts secure highlight */
                line_buff[scr_pos / 80] = TRUE;
                tel_screen->buffer[scr_pos++] = buffptr[i];
                break;

            case    CAN:    /* Start blinking highlight */
                line_buff[scr_pos / 80] = TRUE;
                tel_screen->buffer[scr_pos++] = buffptr[i];
                break;
#endif
            case    DC1:    /* Clear to end of line */

        /*      2/13/95    DAE
                If set for auto flip, look for DC1, if not present
                set term to local.  By doing it here we will still
                send out a message. */

                found_dc1 = TRUE;
                break;

            case    DC3:    /* Up arrow */
                if (scr_pos < 80)
                    scr_pos += SCRSIZE;
                scr_pos -= 80;
                break;

            case    DC4:    /* Position cursor home */
                        /*Position cursor home */
                if (!forms_mode)
                    scr_pos = 0;
                else
                {
                    done = FALSE;
                    k = 0;
                    while (!done)
                    {
                        if ( (tel_screen->buffer[k] == US) || (k > SCRSIZE) )
                        {
                            done = TRUE;
                            tel_screen->term_mode = TELTERM_LEFT_JUST;
                        }
                        else if (tel_screen->buffer[k] == GS)
                        {
                            if (tel_screen_buffs[conn_index].term_type_num ==
                                                TELNET_TERM_VT100)
                            /* If IBM 3270, we don't position on the right */
                            {
                                while ( (tel_screen->buffer[k] != RS) &&
                                        (k < SCRSIZE))
                                    k++;
                                k -= 2;
                            }
                            done = TRUE;
                            tel_screen->term_mode = TELTERM_RIGHT_JUST;
                        }
                        else
                            k++;
                    }
                    k++;
                    scr_pos = k;
                    line = k / 80;
                    col = k % 80;
                }
                break;
                // The attribute characters apparently should be 
                // treated identically to printing characters, so
                // I commented out these lines.
                // Discovered by a Swiss SNITerm prospect & verified
                // against Core's emulator.
#if 0
            case    SUB:    /* Start bright higlight */
                line_buff[scr_pos / 80] = TRUE;
                tel_screen->buffer[scr_pos++] = buffptr[i];
                break;

            case    SO:     /* Start reverse hightlight */
                line_buff[scr_pos / 80] = TRUE;
                tel_screen->buffer[scr_pos++] = buffptr[i];
                break;

            case    SI:     /* Start underline */
                line_buff[scr_pos / 80] = TRUE;
                tel_screen->buffer[scr_pos++] = buffptr[i];
                break;
#endif
            case    ESC:    /* Escape sequences */
            case    DC2:    /* Toggle forms mode */

                if (buffptr[i] == ESC)
                    i++;

                switch (buffptr[i])
                {
                    case    'R':
                        if (buffptr[i + 1] == 'S')
                            /* Display Message on Status Line */
                        {
                            i +=2;  /* Puts us past the S */

                            /* The value is two character HEX */
                            status_count = (buffptr[i++] - 48) * 16;
                            if (buffptr[i] < 59)
                                status_count += (buffptr[i++] - 48);
                            else
                                status_count += (buffptr[i++] - 55);
/**** DWN 3/29/94
                            for (k = 0; k < STATUS_LEN; k++)
                                tel_screen->status_line[k] = SP;
****/
                            memset (tel_screen->status_line, SP, STATUS_LEN);

                            k = 0;
                            // Added by MRR for bad message from Amalgamated
                            // screen; esc RS50 followed by less than 80 bytes.
                            if(status_count > length-i) status_count = length-i;
                            while (k < status_count)
                            {
#ifndef ETX_END_BUFFER
										  if (buffptr[i] == ETX)
										      break;
#endif
                                /* Only store alphanumerics in the status line */
                                if ( (buffptr[i] >= 0x20) &&
                                     (buffptr[i] <= 0x7E) )
                                    tel_screen->status_line[k++] = buffptr[i++];
                                else
                                {
                                    tel_screen->status_line[k++] = SP;
                                    i++;
                                }
                            }
                            i--;
                            if (status_count > 0)
                                tel_screen->status_line[STATUS_LEN] = NULL;
                        }
                        else if (buffptr[i + 1] == 'A')
                            /* Display Message on Status Line */
/* ESC R A 0310 39   Store ASCII in terminal starting at
    position 0310 (range is 3000 to 3004f,  0310 is position 16 (HEX 10),
       57 bytes long (0x39) */
                        {
                            i +=2;  /* Puts us past the S */
                            i +=2;  /* Puts us past the 03 */
                            status_position = (buffptr[i++] - 48) * 16;
                            if (buffptr[i] < 59)
                                status_position += (buffptr[i++] - 48);
                            else
                                status_position += (buffptr[i++] - 55);

                            status_count = (buffptr[i++] - 48) * 16;
                            if (buffptr[i] < 59)
                                status_count += (buffptr[i++] - 48);
                            else
                                status_count += (buffptr[i++] - 55);

/**** DWN 3/29/94
                            for (k = 0; k < STATUS_LEN; k++)
                                tel_screen->status_line[k] = SP;
****/
                            memset (tel_screen->status_line, SP, STATUS_LEN);

                            k = status_position;
                            if ( (status_count + status_position) > 80)
                                status_count = 80 - status_position;

                            while (k < status_count + status_position)
                            {
#ifndef ETX_END_BUFFER
									     if (buffptr[i] == ETX)
										      break;
#endif
                                /* only handle later if not a space.
                                    (Displaying as an error CSI) */
                                if (buffptr[i] != SP)
                                    esc_ra = TRUE;

                                /* Only store alphanumerics in the status line */
                                if ( (buffptr[i] >= 0x20) &&
                                     (buffptr[i] <= 0x7E) )
                                    tel_screen->status_line[k++] = buffptr[i++];
                                else
                                {
                                    tel_screen->status_line[k++] = SP;
                                    i++;
                                }
                            }
                            i--;

                            if (status_count > 0)
                                tel_screen->status_line[STATUS_LEN] = NULL;

                        }
                        else if (buffptr[i + 1] == 'C')
                        {
                            /* Reconfigure terminal sequence, eat it */
                            i++; /* Bump it to the C */
                        }
                        else if (buffptr[i + 1] == 'T')
                            /** Eat terminal config. enquiry
                                <ESC>RT009C04<DC1>
                                <ESC>RT009201<DC1>  **/
                        {
                            i +=2;  /* Puts us past the T */
                            i +=6;  /* Puts us past the address and len. */
                            i--;
                        }
                        break;

                    case    'W':     /* Enter forms mode */
                    case    DC2:
#if 1
                        if (tel_screen_buffs[conn_index].alt_delim)
                        {
                            translate(ebc2asc, &ug_globals.vt_start_field, &alt_start);
                            translate(ebc2asc, &ug_globals.vt_end_field, &alt_end);

                            for (k = 0; k < 1920; k++)
                            {
                                if (tel_screen->buffer[k] == alt_start)
                                    tel_screen->buffer[k] = US;
                                else if (tel_screen->buffer[k] == alt_end)
                                    tel_screen->buffer[k] = RS;
                            }
                        }
#endif
                        if (!forms_mode)
                        {
                            /* If we are not in FORMS mode, go into it */
                            forms_mode = TRUE;
                            /* If forms_mode was set, see if there are any fields. */

                            done = FALSE;
                            k = 0;
                            while (!done)
                            {
                                if ( (tel_screen->buffer[k] == US) ||
                                     (k > SCRSIZE) )
                                {
                                    done = TRUE;
                                    tel_screen->term_mode = TELTERM_LEFT_JUST;
                                }
                                else if (tel_screen->buffer[k] == GS)
                                {
                                    done = TRUE;
                                    tel_screen->term_mode = TELTERM_RIGHT_JUST;
                                }
                                else
                                    k++;
                            }
                            if (k < SCRSIZE)
                            {
                                k++;
                                scr_pos = k;
                                line = k / 80;
                                col = k % 80;
                            }
                            else
                            {
                                forms_mode = FALSE;
                                // Added by MRR; else previous setting of
                                // TELTERM_LEFT_JUST implied forms mode.
                                tel_screen->term_mode = TELTERM_DATA;
                                scr_pos = 0;
                                line = scr_pos / 80;
                                col = scr_pos % 80;
                            }
                        }
                        else if ( (forms_mode) &&
                                  (buffptr[i] == DC2) )
                        {
                            forms_mode = FALSE;
                            scr_pos = 0;
                        }
                        break;

                    case    'S':    /* Roll page up */
                        memcpy(save_buff, &tel_screen->buffer[0], 80);
                        memcpy (tel_screen->buffer, tel_screen->buffer+80,1840);
                        memcpy(&tel_screen->buffer[1840], save_buff, 80);

                        if (tel_screen_buffs[conn_index].term_type_num ==
                                                            TELNET_TERM_VT100)
                        {
                            memset (line_buff, TRUE, 24);
/*
    For the sake of line 25 VTs just repaint the whole screen (Reilly)
    DAE 5/19/95
                            if (length > 512)
                                memset (line_buff, TRUE, 24);
                            else
                            {
                                tcb_id = tel_screen_buffs[conn_index].conn_id;
                                tcp_send(tcb_id, "\x01b[24;80H\x0d\x0a\0",
                                            10, FL_PSH);
                                sprintf(scratch_buff,
                                        "\x01b[%d;80H\x0d\x0a\0", last_line);
                                tcp_send(tcb_id, scratch_buff,
                                            strlen(scratch_buff), FL_PSH);

                                vt100_refresh(tel_screen_buffs[conn_index].conn_id,
                                            conn_index, 1840, SCRSIZE, FALSE);
                                memset (line_buff, FALSE, 24);
                            }
*/
                        }
                        break;

                    case    'T':    /* Roll page down */
                            memcpy(save_buff, &tel_screen->buffer[1840], 80);
                            for (k = 1839; k >= 0; k--)
                                tel_screen->buffer[k+80] = tel_screen->buffer[k];
                            memcpy(&tel_screen->buffer[0], save_buff, 80);

                            memset (line_buff, TRUE, 24);
/*
    For the sake of line 25 VTs just repaint the whole screen (Reilly)
    DAE 5/19/95

                            tcp_send(tcb_id, "\x01b[1;1H\x1b[1L\0", 10, FL_PSH);
                            line_buff[0] = TRUE;
*/

                        break;

                    case    '<':    /* Exchange line below */
                    case    '>':    /* Exchange line above */

                            cur_line = tel_screen->scr_pos / 80;
                            cur_col =  tel_screen->scr_pos % 80;
                            if (buffptr[i] == '>')
                            {
                                if (cur_line == 0)
                                    swap_line = 23;
                                else
                                    swap_line = cur_line - 1;
                            }
                            else
                            {
                                if (cur_line == 23)
                                    swap_line = 0;
                                else
                                    swap_line = cur_line + 1;
                            }
                            scr_pos = (swap_line * 80) + cur_col;

                            memcpy(save_buff, &tel_screen->buffer[cur_line*80], 80);
                            for (k = 0; k < 80; k++)
                                tel_screen->buffer[(cur_line * 80) + k] =
                                    tel_screen->buffer[(swap_line * 80) + k];
                            memcpy(&tel_screen->buffer[swap_line*80], save_buff, 80);

                            line_buff[swap_line] = TRUE;
                            line_buff[cur_line] = TRUE;

                        break;

                    case    'M':        /* Delete line & move cursor to col 1 */
                        tel_screen->scr_pos = scr_pos;  // update real copy of scr_pos
                        ug_delete_line_tel_screen(conn_index,tel_screen);
                        scr_pos = tel_screen->scr_pos;  // restore working copy.
                        break;

                    case    'X':        /* Enter non-forms mode */
                        if (forms_mode) {
                            forms_mode = FALSE;
                            // We need to set term_mode here because other
                            // esc sequences on the same page may use 
                            // term_mode to determine forms_mode.  /mrr 18Sep1998
                            tel_screen->term_mode = TELTERM_DATA;
                        }
                        scr_pos = 0;
                        break;

                    case    'D':        /* Set mobile home */
                        tel_screen->home_pos = scr_pos;
                        break;

                    case    'J':        /* Erase to end of screen*/
                        tel_screen->scr_pos = scr_pos;
                        ug_erase_to_end_of_page_tel_screen(tel_screen);
                        scr_pos = tel_screen->scr_pos;
                        break;

                    case    'K':        /* Erase to end of line */
/**
                        k = scr_pos;
                        line_buff[scr_pos / 80] = TRUE;
                        if ( (k % 80) == 0)
                            tel_screen->buffer[k++] = SP;
                        while (k % 80)
                            tel_screen->buffer[k++] = SP;
***/

/* 4/10/95
    DAE
    We were only doing this as if we were never in forms mode.
    At State of MI Trans. we got a host.log of clear to end of line within
    a field.
    */
                        if (!forms_mode)

                        {
                            k = scr_pos;
                            line_buff[scr_pos / 80] = TRUE;
                            /* Handle pos in col 1 */
                            if ( (k % 80) == 0)
                                tel_screen->buffer[k++] = SP;

                            while ( (k % 80) != 0)
                                tel_screen->buffer[k++] = SP;
                        }
                        else
                        {
                            /* First find out if we are in a field */
                            k = 0;
                            while (k != scr_pos)
                            {
                                if (tel_screen->buffer[k] == US)
                                    forms_field = TELTERM_LEFT_JUST;
                                else if (tel_screen->buffer[k] == GS)
                                    forms_field = TELTERM_RIGHT_JUST;
                                else if (tel_screen->buffer[k] == RS)
                                    forms_field = FALSE;
                                k++;
                            }


                            /* Current Position */
                            if (tel_screen->buffer[k] == RS)
                                forms_field = FALSE;

/*** DWN 3/21/95
     Make sure we clear the whole field
     if it is right-justified.
***/
                            m = scr_pos;
                            line_buff[scr_pos / 80] = TRUE;
                            if (forms_field != FALSE)
                            {
                                /* If right justified, back up */
                                if (forms_field & TELTERM_RIGHT_JUST)
                                {
                                    while (tel_screen->buffer[--m] != GS);
                                    k = m + 1;
                                }

                                done = FALSE;
                                while (!done)
                                {
                                    if (em_check_for_end(k))
                                    {
                                        done = TRUE;
                                        forms_field = FALSE;
                                    }
                                    else
                                        tel_screen->buffer[k] = SP;

                                    k++;

                                    if ( (k % 80) == 0)
                                        done = TRUE;
                                }
                            }
                        }

                        break;

                    case '"':       /* Position cursor to column, row */
                        /* Position cursor */
                        scr_pos = (buffptr[i+2]-' ')*80 + (buffptr[i+1]-' ');
                        scr_pos = scr_pos % SCRSIZE;  // mrr added to fix blowup on bad cursor positioning
                        i += 2;
                        break;

                    case '&':   /* Align cursor (KBC) and page(DCP) */

                        cur_page = tel_screen_buffs[conn_index].term_state - 1;
                        orig_page = cur_page;
                        tel_screen_buffs[conn_index].cur_page = cur_page;
/**** DWN 3/29/94
                        for (k = 0; k < 24; line_buff[k++] = TRUE);
****/
                        memset (line_buff, TRUE, 24);
                        break;

                    case '6':   /* Go back to original page */
                        cur_page = orig_page;
                        break;

                    case '$':       /* Position new page */
                        new_page = buffptr[i+1] - 31;

                        if (new_page < (MAX_PAGES + 1))
                        /* new_page is 1 relative (i.e. 1, 2, 3, 4) */
                        {
                            memcpy(scratch_buff, tel_screen, SCREENHDRSIZE);
/*** DWN 11/2/94 Remember forms mode on page change ***/
                            if (!forms_mode)
                                tel_screen->term_mode = TELTERM_DATA;
                            store_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page],
                                    tel_screen_buffs[conn_index].buf_num[cur_page]);

                            cur_page = new_page - 1; /* To base on a 0 offset */
                            em_determine_page(conn_index, cur_page);
                            new_alloc = FALSE;
                            if (tel_screen_buffs[conn_index].buf_num[cur_page] == -1)
                            {
                                alloc_screen_buf(&tel_screen_buffs[conn_index].buf_type[cur_page],
                                            (unsigned short *)&tel_screen_buffs[conn_index].buf_num[cur_page]);
                                new_alloc = TRUE;
                            }

                            load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page],
                                tel_screen_buffs[conn_index].buf_num[cur_page]);

                            if ( (new_alloc) || (tel_screen->buffer[0] == NULL))
                            {
                                new_alloc = FALSE;
                                /* We only have to move alitte bit of memory */
                                memcpy(tel_screen, scratch_buff, SCREENHDRSIZE);

                                /* Initialize some variables */
                                em_init_page(tel_screen);
                            }

                            forms_mode = tel_screen->term_mode;
                            tel_screen_buffs[conn_index].cur_page = cur_page;

                            if (!(tel_screen->term_mode & TELTERM_DATA))
                            {
                                /* If forms_mode, see if there are any fields. */
                                done = FALSE;
                                k = 0;
                                while (!done)
                                {
                                    if ( (tel_screen->buffer[k] == US) ||
                                         (k > SCRSIZE) )
                                        done = TRUE;
                                    else if (tel_screen->buffer[k] == GS)
                                    {
                                        if (tel_screen_buffs[conn_index].term_type_num ==
                                                TELNET_TERM_VT100)
                                        /* If IBM 3270, we don't position on the right */
                                        {
                                            while ( (tel_screen->buffer[k] != RS) &&
                                                     (k < SCRSIZE) )
                                            k++;
                                            k -= 2;
                                        }
                                        done = TRUE;
                                    }
                                    else
                                        k++;
                                }
                                if (k >= SCRSIZE)
                                /* Could not find any fields */
                                {
                                    forms_mode = FALSE;
                                    scr_pos = tel_screen->scr_pos = 0;
                                }
                                else
                                {
                                    forms_mode = TRUE;
                                    k++;
                                    scr_pos = tel_screen->scr_pos = k;
                                }
                            }
                            else
                            {
                                forms_mode = FALSE;
                                tel_screen->scr_pos = scr_pos = 0;
                            }
                        }
                        i++;
                        break;

                    case '(':   /* Transmits DCP page to host. */
                        send_back = TRUE;
                        break;

                    default:
                        break;
                }
                break;

            default:
                // This is a displayable byte to be placed on the screen.
                pConn->m_nPrintableBytesRec++;
                if (!forms_mode)
                {
                    line_buff[scr_pos / 80] = TRUE;
                    tel_screen->buffer[scr_pos++] = buffptr[i];
                }
                else
                {
                    /* If we have data to go into a forms mode */
                    if ( (tel_screen->buffer[scr_pos] != RS) &&
                         (tel_screen->buffer[scr_pos] != US) &&
                         (tel_screen->buffer[scr_pos] != GS) &&
                         (buffptr[i] != HT))
                    {
                        line_buff[scr_pos / 80] = TRUE;
                        tel_screen->buffer[scr_pos++] = buffptr[i];
                    }
                    else
                    {
                        /* Look for the next field to start putting data in. */
                        k = scr_pos++;
                        done = FALSE;
                        while (!done)
                        {
                            if ( (tel_screen->buffer[k] == US) ||
                                 (tel_screen->buffer[k] == GS) )
                            {
                                if (tel_screen->buffer[k] == US)
                                    tel_screen->term_mode = TELTERM_LEFT_JUST;
                                else
                                    tel_screen->term_mode = TELTERM_RIGHT_JUST;
                                k++;
                                if (k == SCRSIZE)
                                    k = 0;

                                if (buffptr[i] != HT)
                                {
                                    line_buff[scr_pos / 80] = TRUE;
                                    tel_screen->buffer[k++] = buffptr[i];
                                }

                                scr_pos = k;
                                done = TRUE;
                            }
                            else
                            {
                                k++;
                                if (k == SCRSIZE)
                                    k = 0;
                            }
                        }
                    }
                }
                if (scr_pos == SCRSIZE)
                    scr_pos = 0;
                break;
        }   /* switch */
    }   /* for */

    // If we didn't get an ETX signifying an end of message, we need
    // to set the number of good bytes left (usually 0).
    if(i < length) {
       dataptr->bytesleft = length-i-1;
    } else if(i==length) {
       dataptr->bytesleft = 0;
    }

    /*  2/13/95       DAE */

    if ( (tel_screen_buffs[conn_index].auto_flip) &&
         (!found_dc1) 
         /*&& (tel_screen_buffs[conn_index].term_type_num == TELNET_TERM_VT100) */)
    {
        tel_screen_buffs[conn_index].term_state = TERMSTATE_LOCAL;
    }



    if ( (status_state != 0) &&
         TRUE) /// ( ( (qwik[sn].stationbits & 0x40) != 0) ) )  /* scroll on */
    {
        tel_screen_buffs[conn_index].status_state = status_state;
/***
        vt100_last_line(conn_index);
***/
    }


    /* A safety check */
    if (scr_pos >= SCRSIZE)
        scr_pos = SCRSIZE;

    /* If a right justified forms mode field, put the cursor to the
        right */
    if ( (tel_screen->term_mode == TELTERM_RIGHT_JUST) 
       /*&& (tel_screen_buffs[conn_index].term_type_num == TELNET_TERM_VT100)*/ )
    {
        k = scr_pos;
        while ( (tel_screen->buffer[k] != RS) &&
                (k < SCRSIZE) )
            k++;
        k--;
        scr_pos = k;

        line_buff[scr_pos / 80] = TRUE;

    }

/**
    if (send_back)
    {
        tel_screen->scr_pos = scr_pos;
        em_send_input (conn_index);
    }
***/

    if (cur_page != orig_page)
        /* KBC was not aligned to DCP, so set DCP back to KBC */
    {
        tel_screen->scr_pos = scr_pos;
        store_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page],
                         tel_screen_buffs[conn_index].buf_num[cur_page]);
        cur_page = orig_page;
        tel_screen_buffs[conn_index].cur_page = orig_page;
        load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page],
                        tel_screen_buffs[conn_index].buf_num[cur_page]);

        scr_pos = tel_screen->scr_pos;
        if (tel_screen->term_mode == TELTERM_DATA)
            forms_mode = FALSE;
        memset (line_buff, TRUE, 24);
    }

    tel_screen->scr_pos = scr_pos;
    end_pos = tel_screen->scr_pos;
   if (send_back)
    {
      // MRR added code to check the first two positions on the
      // screen.  If they are a CANDE editing sequence & 
      // bDontSendBackCANDE is true, don't send the screen.
      if(bDontSendBackCANDE && tel_screen->buffer[0] == '?' && 
        (tel_screen->buffer[1]== '+' || tel_screen->buffer[1]=='-') ){
         send_back = FALSE;
      }
      if(send_back) {
         tel_screen->scr_pos = scr_pos;
         em_send_input (conn_index,tel_screen);
      }
    }

    //mrr added term_mode to struct
    //tel_screen->tforms_mode = forms_mode;

    store_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page],
                tel_screen_buffs[conn_index].buf_num[cur_page]);

    return(end_scr);
}

#endif

/* =========================================================================
= $BEGDOC  em_send_input CR  Send input to host, if we have an XMIT
 *
 *  Exit:   Returns 0 if success.
*/

short em_send_input(int conn_index, struct telnet_screen *tel_screen)
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

    boolean  done;
    boolean  done1;
    boolean  keep_data;
    int retval=0;

    //int16    cur_last;
    int16    i,k;         /* counter */
    int16    line, col;
    int16    tcb_id;
    int16    stn_no;

    int32    first_field;        /* A bit field */
    int32    last_field;        /* A bit field */

    uint8 cur_page;
    //uint8 new_page;

    /* Locals added by MRR */
    int16 cur_ptr;          /* Current position in the screen */
    uchar8      scratch_buff[BUFFSIZE];
    struct  message recv_buffer;

    proc_trace("em_send_input");

    stn_no = tel_screen_buffs[conn_index].stn_no;
    cur_page = tel_screen_buffs[conn_index].cur_page;
    tcb_id = tel_screen_buffs[conn_index].conn_id;
    line = tel_screen->scr_pos / 80;
    col = tel_screen->scr_pos % 80;

    //mrr  forced code below to execute
    if(FALSE) {
      col = col;
    }

#ifdef UNI_VT
    else
    {

        if (tel_screen_buffs[conn_index].cntl[0] == 0)
        {
            /***    IF tel_screen->subesc IS TRUE WE HAVE A TRANSMIT LINE. */
            if (tel_screen->term_mode & TELTERM_DATA)
            {
                cur_ptr = tel_screen->scr_pos;
                k = 0;

                if (tel_screen->home_pos != 0)
                {
                    if (cur_ptr <= tel_screen->home_pos)
                    {
                        for (i = tel_screen->home_pos; i < SCRSIZE; i++)
                            translate(asc2ebc, &tel_screen->buffer[i],
                                    &recv_buffer.buff[k++]);
                        cur_ptr = k;
                    }
                    else
                    {
                        for (i = tel_screen->home_pos; i < tel_screen->scr_pos; i++)
                            translate(asc2ebc, &tel_screen->buffer[i],
                                    &recv_buffer.buff[k++]);
                        cur_ptr = k;
                    }
                }


                else if ((cur_ptr == 0) && (!tel_screen->subesc))
                {
                    for (i = 0; i < SCRSIZE; i++)
                        translate(asc2ebc, &tel_screen->buffer[i],
                                &recv_buffer.buff[i]);
                    cur_ptr = SCRSIZE;
                }
                else
                {
                    if (!tel_screen->subesc)
                    {
                        for (i = 0; i < tel_screen->scr_pos; i++)
                        {
                            translate(asc2ebc, &tel_screen->buffer[i],
                                    &recv_buffer.buff[i]);
                        }
                    }
                    else
                    {
                        if (col == 0)
                        {
                            for (i = (line*80); i < ((line+1)*80); i++)
                                translate(asc2ebc, &tel_screen->buffer[i],
                                &recv_buffer.buff[k++]);
                            cur_ptr = k;
                        }
                        else
                        {
                            for (i = (line*80); i < (line*80+col); i++)
                                    translate(asc2ebc,
                                    &tel_screen->buffer[i],
                                    &recv_buffer.buff[k++]);
                            cur_ptr = k;
                        }
                        tel_screen->subesc = FALSE;
                    }
                }
            }
            else
            {
                first_field = 0;

/**** DWN 2/21/92 ****/
                done = FALSE;
                i = 0;

                /* Find the first field */
                while (!done)
                {
                    if ( ( (tel_screen->buffer[i] == US) ||
                           (tel_screen->buffer[i] == GS) ) &&
                         !em_check_for_end(i+1) )
                    {
                        if (tel_screen->buffer[i] == US)
                            first_field = TELTERM_LEFT_JUST;
                        else if (tel_screen->buffer[i] == GS)
                            first_field = TELTERM_RIGHT_JUST;
                        done = TRUE;
                    }
                    else if (i >= SCRSIZE)
                        done = TRUE;
                    else
                        i++;
                }

                /* Now find out if we are in the 'HOME' position.  If
                    we are, the whole screen gets transmitted. */
                if (tel_screen_buffs[conn_index].full_xmit)
                    cur_ptr = SCRSIZE - 1;
                // Try it again:  If there are no forms fields and the
                // cursor is in the upper left, do the same thing.
                // I'm not sure this is right, but it wasn't working
                // the other way.  /mrr
                //else if(!first_field && tel_screen->scr_pos==0)
                //   cur_ptr = SCRSIZE - 1;
                else if ( (first_field & TELTERM_LEFT_JUST) &&
                          (tel_screen->scr_pos <= (i + 1) ) )
                    cur_ptr = SCRSIZE - 1;
                else if (first_field & TELTERM_RIGHT_JUST)
                {
                    done = FALSE;
                    while (!done)
                    {
                        if (em_check_for_end(i+1) )
                            done = TRUE;
                        else
                            i++;
                    }

                    if (tel_screen->scr_pos <= i)
                        cur_ptr = SCRSIZE - 1;
                    else
                        cur_ptr = tel_screen->scr_pos;
                }
                else
                    cur_ptr = tel_screen->scr_pos;

                keep_data = FALSE;
                k = 0;
                for (i = 0; i <= cur_ptr; i++)
                {
                    if (keep_data)
                    {
                        if (!tel_screen->subesc)
                            recv_buffer.buff[k++] = tel_screen->buffer[i];
                        else if ( (i / 80) == line)
                            recv_buffer.buff[k++] = tel_screen->buffer[i];

                        if ( (i < cur_ptr) &&
                              (em_check_for_end(i + 1) ) )
                            keep_data = FALSE;

                    }
                    else if ( ( (tel_screen->buffer[i] == GS) ||
                                (tel_screen->buffer[i] == US) ||
                                (tel_screen->buffer[i] == FS) ) &&
                                !em_check_for_end(i + 1) )
                    {
                        if (tel_screen->buffer[i] == US)
                            last_field = TELTERM_LEFT_JUST;
                        else if (tel_screen->buffer[i] == GS)
                            last_field = TELTERM_RIGHT_JUST;

                        keep_data = TRUE;
                    }
                }

                if ( (last_field & TELTERM_LEFT_JUST) &&
                     keep_data)
                    recv_buffer.buff[k - 1] = NULL;
                else
                    recv_buffer.buff[k] = NULL;

                tel_screen->subesc = FALSE;
                cur_ptr = strlen((char *)recv_buffer.buff);

                for (i = 0; i < cur_ptr; i++)
                    translate(asc2ebc, &recv_buffer.buff[i],
                            &recv_buffer.buff[i]);
            }

            /* Go through and see if an ETX is in the data, if so readjust things */
            done1 = FALSE;
            i = 0;
            while (!done1)
            {
                if (i == cur_ptr) {
                    done1 = TRUE;
                } else if (recv_buffer.buff[i] == ETX)
                {
                    done1 = TRUE;
                    cur_ptr = i;
                }
                i++;
            }
        }
        else
        {
            cur_ptr = 3;
            recv_buffer.buff[0] = ESC;
            recv_buffer.buff[1] = tel_screen_buffs[conn_index].cntl[1];
            recv_buffer.buff[2] = tel_screen_buffs[conn_index].cntl[2];
            tel_screen_buffs[conn_index].cntl[0] = 0;
            //vt100_refresh(tcb_id, conn_index, 1840, SCRSIZE, FALSE);

            for (i = 0; i < cur_ptr; i++)
                translate(asc2ebc, &recv_buffer.buff[i],
                        &recv_buffer.buff[i]);
        }
    }
#endif

    //recv_buffer.fepno = cluster.myid;
    recv_buffer.stn = stn_no;
    recv_buffer.hostno = 0;
    recv_buffer.station_bits = 0;
    recv_buffer.termtype = 0; ///qwik[stn_no].ndltype;
    recv_buffer.msglen = cur_ptr;
    recv_buffer.buff[cur_ptr] = ETX;
    recv_buffer.opcode[0] = MESSAGE_OP;
    recv_buffer.opcode[1] = MESSAGE_FROM_STATION;
    recv_buffer.original_opcode[0] = 0;
    recv_buffer.original_opcode[1] = 0;
    //stats[stn_no].recvcount++;
    //update_signoff_table(stn_no);


/*
    TOOK OUT 12/6/93 BY   DAE
    if (host_audit)
        audit_rec('J',&recv_buffer,0);
*/


    store_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page],
            tel_screen_buffs[conn_index].buf_num[cur_page]);

    if (check_trancode(stn_no) != 0)
    {
        if ((retval=send_2_host(&recv_buffer)) != 0)
        {
            // Error in sending to socket.
           col = col;
        }
#if 0
        else if (qwik[stn_no].oneforone == 1)
            qwik[stn_no].oneforone = 2;
#endif
    }

    load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page],
            tel_screen_buffs[conn_index].buf_num[cur_page]);

    tel_screen->telnet_state = TELSTATE_DATA;
    cur_ptr = 0;

///#ifdef TELNET_UNIGATE
    // Need to load from telclt_ variables, because the GoAhead
    // processing is based on this telnet parameter.
    telclt_load_screen_buf(tel_screen_buffs[conn_index].buf_type[MAX_PAGES],
            tel_screen_buffs[conn_index].buf_num[MAX_PAGES]);
    if ( !(tel_screen->tel_opts[TELOPT_SGA] & TELOPT_ACTIVE_OUT) )
    {
        scratch_buff[0] = TELNET_IAC;
        scratch_buff[1] = TELNET_GA;
        scratch_buff[2] = NULL;
        tcp_send(tcb_id, scratch_buff, 2, FL_PSH);
    }
///#endif

    return(retval);

}




/*********************************** eof ***********************************/
