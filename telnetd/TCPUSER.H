/***************************************************************************/
/*                                                                         */
/*       T C P / I P     C o m m o n     U s e r     I n t e r f a c e     */
/*                                                                         */
/***************************************************************************/


/*\
 * User call flags 
\*/

#define FL_LISTEN	0x01
#define FL_PACE 	0x08

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
 * tcpuser interface procedure definitions
\*/

short tcp_open(   unsigned short    lport,
		unsigned long	fhost,
                unsigned short    fport,
                unsigned short    flags,
                short             (*rcv)() );

short tcp_close(  short             id );

short tcp_send(   short             id,
		unsigned char	*buff,
                unsigned short    len,
                unsigned short    flags );

short tcp_receive(short             id );

short tcp_abort(  short             id );

short tcp_status( short             id,
		TCB		*tcb_buff);

short tcp_control(short             id,
                unsigned short    fcn,
                unsigned short    parm );

/*\
 * Useful externals
\*/

extern unsigned long 	myhost;

