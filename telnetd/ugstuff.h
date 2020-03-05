/*--- ugstuff.h -- Defines, macros, etc., needed by code I took
 *  from UniGate.
 *
 *  Mark Riordan  29 March 1996
 */

#ifndef UGSTUFF_H
#define UGSTUFF_H

struct struct_ug_globals {
   DWORD UseAlternateDelimiters;
   unsigned char vt_start_field;  // EBCDIC alternate form field start char
   unsigned char vt_end_field;    // EBCDIC alternate form field end char
};

short alloc_screen_buf (short *type, unsigned short *num);
short store_screen_buf (short type, short num);
void ug_init_tel_screens(void);
short telclt_alloc_screen_buf (short *type, unsigned short *num);

boolean vt100_parse_input(  int16 conn_index,
			    int16 *counter,
			    int16 *len,
			    uint8 *txt);
void msg_fr_ibm_host (int sn, struct message *bufptr);
void msg_fr_5250_host (int sn, struct message *bufptr);
short send_2_bgh (struct message *bufptr, int ibm_stn_no);
void bgh_send (unsigned char *c, int conn_index);
short tcp_send_log (short id,
          unsigned char *buff,
          unsigned short len,
          unsigned short flags);
void
UGLogFromHost(short sn, unsigned char *buff, int len);

short uni_proc_trace (char *c);
short proc_trace(char *display);
short prn_err_wind(
                short             i,
                unsigned char   *buff);
short check_trancode (unsigned short sn);
short send_2_host (struct message * buffptr);
short em_send_input(int conn_index, struct telnet_screen *tel_screen);
void
telsrv_estab_conn(int id);

int
UGInitConnection(int conn_index, int termtype);
int
UGInitInboundConnection(int conn_index);
void
UGInitAllInbound(void);
void
SendTCPPlusEOR(int id, unsigned char *buf, int len);
void
UGSelectPage(int conn_index, int cur_page);
void
UGSendSpecify(TypConn *pConn);
int
UGSendScreen(TypConn *pConn);
int
UGTransmitLine(TypConn *pConn);
void UGSetLocation(int iconn, const char *location);

short initqwik(void);

int telnet_client
   (int16 id,                   /* TCB id */
       int16 sig,               /* TCP signal */
       uint8 * txt,             /* Data buffer */
       int16 len,               /* Amount of 'good' data in buffer */
       int16 flags,             /* Flags passed with data */
       struct message *msgptr,
       int   &rembytes);        /* Return: unprocessed bytes remaining */

short ug_first_field (int conn_index);
short ug_home_cursor (int conn_index);
short ug_next_field (int conn_index,
                  int j);
int
ug_get_curpos(int conn_index);
void
ug_set_curpos(int conn_index,int curpos);
boolean emf_check_for_end (struct telnet_screen *tel_screen,
                          int16 pos);
boolean is_end_of_field(char ichar);
int
ug_get_forms_mode(int conn_index);

void ug_free_buffers(void);
int
ug_tab(TypConn *pConn);
void
ug_erase_to_end_of_field(TypConn *pConn);
void ug_erase_to_end_of_page_tel_screen(struct telnet_screen *tel_screen);
void ug_erase_to_end_of_page(TypConn *pConn);
int ug_type_char(TypConn *pConn, char mychar);
int ug_insert_line(TypConn *pConn);
int ug_delete_line_tel_screen(int conn_index,struct telnet_screen *tel_screen);
int ug_del_line(TypConn *pConn);
int ug_del_char(TypConn *pConn);
int ug_clear_screen(TypConn *pConn);
int ug_blank_screen(TypConn *pConn);

void ug_set_forms_mode(TypConn *pConn, int mode);
void ug_set_insert_mode(TypConn *pConn, int mode);
void ug_set_form_xmit_to_cursor_mode(TypConn *pConn, int mode);
void ug_set_auto_flip(int conn_index, int newval);
void ug_set_term_state(int conn_index, BOOL bReceive);
int
ug_get_forms_mode(int conn_index);
int ug_get_insert_mode(int conn_index);
int ug_get_auto_flip(int conn_index);
int ug_get_term_state(int conn_index);
BOOL ug_next_nonfield(TypConn *pConn, int begpos, int &nextbeg, int &len);


/*--- macro load_screen_buf -----------------------------------------------
 *
 * Converted from function version in FQ.C--necessary because use of 
 * tel_screen as a global is not thread-safe.  
 */
#if 0
#define load_screen_buf(type,num)  tel_screen = em_buffers+num;
#endif
#if 0
#define load_screen_buf(type,num) \
   tel_screen = 0; \
   if(num <= nnormal) { \
      tel_screen = em_buffers+num; \
   }
#endif

#define load_screen_buf(type,num)  tel_screen = TelScrBuffers[num];

/*--- macro telclt_load_screen_buf -----------------------------------------------
 *
 * Converted from function version in FQ.C--necessary because use of 
 * tel_screen as a global is not thread-safe.  
 */
#if 0
#define telclt_load_screen_buf(type,num) \
	tel_screen = NULL; \
   if(num <= telclt_nnormal) { \
      tel_screen =  telclt_buffers[num]; \
   }
#else

#define telclt_load_screen_buf(type,num) tel_screen = telclt_buffers[num];

#endif

/*--- macro em_check_for_end ---------------------------------------------
 *
 *  Converted from function em_check_for_end in EM_VT.C--necessary
 *  again because of prior use of tel_screen as a global.
 */
#define em_check_for_end(pos) \
   ((tel_screen->buffer[pos] == RS) || \
	 (tel_screen->buffer[pos] == US) || \
	 (tel_screen->buffer[pos] == GS) || \
	 (tel_screen->buffer[pos] == FS) )


/*--- macro load_tel_screen  -----------------------------------------------
 *
 *  Does the work normally associated with calling load_screen_buf
 */
#define LOAD_TEL_SCREEN \
   struct telnet_screen *tel_screen;  \
   int conn_index = pConn->iord;   \
   int cur_page = tel_screen_buffs[conn_index].cur_page;  \
   load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page], \
                   tel_screen_buffs[conn_index].buf_num[cur_page]);

#define UG_LOAD_TEL_SCREEN \
   struct telnet_screen *tel_screen; \
   int cur_page = tel_screen_buffs[conn_index].cur_page; \
   load_screen_buf(tel_screen_buffs[conn_index].buf_type[cur_page], \
                   tel_screen_buffs[conn_index].buf_num[cur_page]);

#endif UGSTUFF_H