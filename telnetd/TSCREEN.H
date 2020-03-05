/*\
 * Virtual Terminal definitions
\*/

#define SCRSIZE  1920
#define SCREENHDRSIZE 400

#ifdef BIGBUFF
#define SCRBUFSIZE 7500
#else
#define SCRBUFSIZE 10000
#endif


struct telnet_screen
{
    /*  Current status of the TELNET options that are available in UNIGATE.
        The following options are implemented.

        binary;         Option 0,  RFC 856                IN and OUT
        echo;           Option 1,  RFC 857                IN and OUT
        sga;            Option 3,  RFC 858                IN and OUT
        status;         Option 5,  RFC 859                IN only
        send location;  Option 23, RFC 779                IN only
        term_type;      Option 24, RFC 1091               IN only
        eor;            Option 25, RFC 885                IN and OUT
                                                                                                           **/

    uchar8  tel_opts[NTELOPTS]; /* Buffer indicating whether or not     */
                                /*  a TELNET option is supported.       */

    uint16  help_file_fd;       /* help file, file descriptor           */
    int16   help_count;         /* help counter                         */

    uchar8  telnet_state;       /* Current state of this connection.    */
                                /* TELSTATE_DATA, TELSTATE_WILL, etc.   */
    int16   scr_pos;            /* Current cursor position              */
    int16   cur_ptr;            /* Current buffer position              */
    int16   home_pos;           /* home_position                        */
    uchar8  cntl_esc;
    uchar8  term_mode;          /* Specifies FORMS or DATA mode.        */
    uchar8  sub_opcode;
    boolean subesc;
    uint8   status_line[STATUS_LEN+1];
    boolean insert_mode;        /* Indicates if we are in insert mode   */
    uint8   type_ptr;           /* pointer in type_str                  */

                                /* Last type of term out there, VT100,  */
                                /*   TD830, etc.                        */
    uchar8  term_last_type_str[TERM_TYPE_LEN];


                                /* Type of term out there, VT100,       */
                                /*   TD830, etc.                        */
    uchar8  term_type_str[TERM_TYPE_LEN];

                                /* Location of terminal                 */
    uchar8  term_loc[TERM_LOC_LEN];

    uint8   loc_ptr;            /* Pointer term_loc string              */
    boolean sndloc_match;       /* Got a match on SNDLOC                */
    boolean err_msg;            /* Local msg on status line             */
    char    print_name[18];     /* printer name                         */

    char    buffer [SCRBUFSIZE];
    // field_info contains field information; it is an parallel array 
    // to buffer.  It is indexed by screen byte location, just like buffer.
    // An entry is meaningful only if the corresponding entry in buffer
    // meets the condition IS_IBM_DATA_FIELD().
    // The following symbols define bit positions in field_info:
    // FIELDINFO_MSK_LEN  is the length of the field.
    // FIELDINFO_MSK_PROT is On if the field is protected.
#define FIELDINFO_MSK_LEN  0xfff
#define FIELDINFO_MSK_PROT 0x1000
    unsigned short field_info[SCRBUFSIZE]; // Has field information.

    // The following fields were added from SCREEN.H
    // from a struct originally named struct screen_type
    // but renamed by me (for LANClt) to struct struct_ibm_screen_type
    short bgh_stn_no;
    short ibm_stn_no;
    // Previously, IBM code had its own copy of the cursor position.
#define cursor_pos scr_pos
    ///short cursor_pos;
    short buffer_pos;
    short ru_size;
    short saved_len;
    short wsf_data_left;
    short key_error;
    int   line_count;  // for printing??

    char    bind_rcvd;
    char    plu_session;
    char    formatted;
    char    in_help;
    unsigned char    aid;
    char    cf_key;
    char    key_menu;
    char    end_supported;
    char    reply_mode;
    char    msg_continued;
    char    msg_structured;
    char    force_refresh;
    char    saved_type;
    char    last_read_cmd;
    char    as400;
    char    err_message;

    char    saved_char[10];
    char    help_file[16];
    //char    buffer [SCRSIZE];
    char    err_save[80];

    // The following fields were globals in UniGate, but they 
    // can't be in a multithreaded environment.
#define struct_recv_buffer message
   struct struct_recv_buffer *out_buffer;
   char    *outbuf;
   char    *outlimit;
   char    *outpos;

};

// IBM-specific symbols:
#define NONE_SAVED      0
#define TEXT_SAVED      1
#define WSF_SAVED       2


struct telnet_screen_buffs
{
    int16   conn_id;            /* TCB id of this connection            */
    int16   stn_no;             /* Associated Station number            */

    int16   buf_type[MAX_PAGES+1];/* Type of buffer used for screen data        */
                                /*   in fq.c                            */

    int16   buf_num[MAX_PAGES+1]; /* Number of buffer used for screen data*/
    uint8   cur_page;
    int16   timeout_ticks;      /* Number idle time signals accumulated */
    uint8   telnet_mode;        /* Indicates if we are a server or clnt */
    uchar8  term_type_num;      /* Type of term out there, VT100, etc.  */
    uchar8  term_type_negot_seq[TELNET_TERM_NEXT_INDEX];
                                /* Array of successive term types to try*/
                                /* during telnet negotiation.  If we    */
                                /* get to TELNET_TERM_GIVE_UP, then we  */
                                /* failed to agree on a term type       */
    int16   term_type_negot_idx;/* Array index into term_type_negot_seq */
                                /* of termtype last tried               */
    boolean bGotETXThisMsg;     /* TRUE if we hit an ETX in an earlier  */
                                /* TCP packet that was part of this same*/
                                /* message.                             */
                                /* This means we should ignore chars    */
                                /* until the next Telnet EOR.           */
    boolean bEORPending;        /* This packet ends with Telnet EOR, so */
                                /* after processing it, we should clear */
                                /* bGotETXThisMsg.                      */

    boolean status_state;       /* Indicates what we should do about    */
                                /*    the status line                   */
                                /*    0:  Nothing                       */
                                /*    1:  Display Status message only   */
                                /*    2:  Display Row Col Page, etc     */
                                /*    3:  Status message and Row Col Pg */

                                /*    for PTOP                          */
                                /*      1:   PTOP_SRV                   */
                                /*      2:   PTOP_CLT_OPENING           */
                                /*      3:   PTOP_CLT_OPEN              */

#ifdef UNI_VT
    uchar8  term_state;         /* Indicates whether we are in RECV or  */
                                /*   LOCAL mode.  If in receive mode,   */
                                /*   it indicates which page to send    */
                                /*   it to.                             */

    uchar8  aid;                /* ibm: aid char                        */
    boolean des_bs;             /* destructive backspace flag           */
    boolean auto_flip;          /* auto flip local                      */
    boolean line_25;            /* Do we write to line 25?              */
    boolean alt_delim;          /* Indicates in alt. delim. mode.       */
    boolean sound_bell;         /* Ring bell when cursor tabs to home   */

    boolean full_xmit;          /*   FALSE-xmit to cursor in            */
                                /*   forms mode, TRUE-xmit all fields   */
                                /*   in forms mode                      */

    boolean upshift;            /* ibm: Upshift all characters          */
    boolean zero_fill;          /* ibm: Right Justified zero fill       */

    boolean ext_attr;           /* ibm3270   external attribute         */

    uint8 cntl[3];              /* For CNTL [0]: if 0 no CNTL, 1 if CNTL*/
                                /*   [1]: 1st number                    */
                                /*   [2]: 2nd number                    */
#endif

};


