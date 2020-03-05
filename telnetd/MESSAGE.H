#define HEADERSIZE 48

#if defined (BIGBUFF)

#define BUFFSIZE   8192

#else
#define BUFFSIZE   2508
#endif

struct message
{
   unsigned char opcode[2];
   short fepno;
   short stn;
   unsigned char original_opcode[2];

   unsigned short user_number;
   unsigned short local_trancode;
   unsigned short next;
   unsigned char msg_cnt;
   unsigned char short_trancode;
/***
DWN  8/11/93 for 4 digit trancode
   unsigned long message_number;
   unsigned short next;
   unsigned char msg_cnt;
   unsigned char local_trancode;
****/

   unsigned char station_bits;
   unsigned char termtype;
   unsigned char originating_fep;
   unsigned char installation_flag;
   short formno;
   unsigned short msglen;
   unsigned short bytesleft;
   unsigned char progname[18];
   unsigned short continuator;
   unsigned short originator;
   unsigned char alreadymapped;
   unsigned char hostno;
   unsigned char buff[BUFFSIZE];
};
#define BUFFERSIZE sizeof(struct message)
/* station bits usage */
/* +----+----+                               */
/* | x  | y  |     x= return to sender fep   */
/* +----+----+     y= return to sender flag  */
/* | x  | a  |     a= available              */
/* +----+----|                               */
/* | x  | c  |     c= continuator update     */
/* +----+----+        value 0,1,2            */
/* | x  | c  |                               */
/* +----+----|                               */



typedef struct
{
   unsigned char x;
   unsigned char y;
   unsigned char length;
   unsigned char color;
}  Attr;

/* Macro used by everyone */
// Fixed by MRR 3 April 1999 to deal with signed char inptr.
#define translate(table,inptr,outptr)  *(outptr) = table[*((unsigned char *)(inptr))]

