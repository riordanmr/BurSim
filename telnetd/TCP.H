/***************************************************************************/
/*                                                                         */
/*                     T C P       D e f i n i t i o n s                   */
/*                                                                         */
/***************************************************************************/
/*\
 *	tcp.h
\*/

/*\
 * System Constants
\*/

    /* approximate time in SECONDS!!! */

#ifndef TELNET_UNIGATE
#define NTCB	1
#endif

#define IDLE_TIMEOUT    60    /* Number of seconds */


/*\
 * transmission control block
\*/

typedef struct tcb
{
        unsigned short  id;
	unsigned char	state;
	unsigned char	flags;

	unsigned long	lhost;
        unsigned short  lport;
	unsigned long	fhost;
        unsigned short  fport;

                 short  (*rcv_rtn)();

        unsigned short    rcvq;
        unsigned short    sndq;
        unsigned short    rtxq;
	unsigned short	  oopq;

	unsigned long	  oop_seq;

        unsigned short    rtxt;
        unsigned short    ackt;
        unsigned short    deadt;
        unsigned short    idlet;

        unsigned short    pktsize;

	/*\
	 * Send Sequence Variables
	\*/

	struct
	{
		unsigned long	una;
		unsigned long	nxt;
		unsigned short	wnd;
		unsigned long	up;
		unsigned long	wl1;
		unsigned long	wl2;
		unsigned long	cnt;
		unsigned short	offset;
	} snd;
	unsigned long	iss;

	/*\
	 * Receive Sequence Variables
	\*/

	struct
	{
		unsigned long	nxt;
		unsigned short	wnd;
		unsigned long	up;
	} rcv;
	unsigned long	irs;

} TCB;

extern TCB tcb_vec[NTCB];
extern TCB *tcb;

/*\
 * tcb connection states
\*/

#define	CLOSED		0
#define LISTEN		1
#define SYNRCVD		2
#define SYNSENT		3
#define ESTAB		4
#define CLSWAIT		5
#define FINWAIT1	6
#define FINWAIT2	7
#define CLOSING		8
#define TIMEWAIT	9

/*\
 * tcb flags
\*/

#define FL_LISTEN	0x01
#define FL_UH		0x02
#define FL_UP		0x04
#define FL_PACE 	0x08

#define FL_ACK		0x10
#define FL_RTX		0x20

/*\
 * User call flags 
\*/

#define FL_PSH		0x01
#define FL_URG		0x02
#define FL_FIN		0x04

/*\
 * User Signals
\*/

#define SG_DATA		0
#define SG_REFUSE	1
#define SG_ESTAB	2
#define SG_CLOSE	3
#define SG_FIN		4
#define SG_URG		5
#define SG_DEAD		6
#define SG_IDLE 	7

/*\
 * Current Segment variables
\*/

typedef struct seg
{
	unsigned long	shost;
	unsigned long	dhost;
	unsigned short	sport;
	unsigned short	dport;
	unsigned long	seq;
	unsigned long	ack;
	unsigned char	ctl;
	unsigned short	len;
	unsigned short	wnd;
	unsigned long	up;
} SEG;

/*\
 * tcp packet header definition
\*/

typedef struct tcphdr
{
	unsigned short	t_sport;
	unsigned short	t_dport;
	unsigned long	t_seq;
	unsigned long	t_ack;
	unsigned char	t_offset;
	unsigned char	t_ctl;
	unsigned short	t_window;
	unsigned short	t_chksum;
	unsigned short	t_urgent;
} TCPHDR;

extern TCPHDR	*pkt;
extern char	*txt;
extern short	 txt_len;

#define TCPHDRSIZE sizeof(TCPHDR)

/*\
 * Control field bits
\*/

#define	PC_FIN	0x01
#define PC_SYN	0x02
#define PC_RST	0x04
#define PC_PSH	0x08
#define PC_ACK	0x10
#define PC_URG	0x20

/*\
 * Pseudo-header for tcp-checksum
\*/

typedef struct pseudo_hdr
{
	unsigned long	ps_shost;
	unsigned long	ps_dhost;
	unsigned char	ps_zero;
	unsigned char	ps_proto;
	unsigned short	ps_len;
} PSEUDO;

extern PSEUDO	pseudo;

/*\
 * External Procedures
\*/

extern long tod();

