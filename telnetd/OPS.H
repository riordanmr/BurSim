#define INVALID    	0

#define HOST_OP         1

#define PROGRAM_OP      2

#define STATION_OP      3

#define LINE_OP         4

#define TRANS_OP        5

#define USERS_OP        6

#define FORMS_OP        7

#define SECURITY_OP     8

#define MESSAGE_OP      9
#define MESSAGE_FROM_STATION     1
#define MESSAGE_TO_STATION       2
#define GOOD_RESULTS_WRITE       4
#define GOOD_RESULTS_RESP        3
#define GOOD_RESULTS_LARGE       6
#define NULL_MESSAGE             7

#define NETWORK_OP      10

#define CLUSTER_OP      11
#define MAP_OP          12
#define CLUSTER_STATUS           1
#define CLUSTER_ACK_MSG          2
#define CLUSTER_YOU_THERE        3
#define CLUSTER_I_AM_HERE        4
#define CLUSTER_NETWORK_START    5
#define CLUSTER_NETWORK_STOP     6
#define CLUSTER_NETWORK_OK       7
#define CLUSTER_NETWORK_DENY     8
#define CLUSTER_REMOTE_REQUEST   9
#define CLUSTER_REMOTE_OK       10
#define CLUSTER_REMOTE_DENY     11
#define CLUSTER_REMOTE_STOP     12
#define CLUSTER_REMOTE_CHAR     13
#define CLUSTER_REMOTE_SCREEN1  14
#define CLUSTER_REMOTE_SCREEN2  15
#define CLUSTER_NAK_MSG          0
#define CLUSTER_TEST            99

#define UNIGATE_OP      51

#define TERM_OP_LOGON		1
#define TERM_OP_LOGOFF		2
#define TERM_OP_REFRESH		3
#define TERM_OP_MESSAGE		4

#define RJE_OP_OPEN		10
#define RJE_OP_CLOSE		11
#define RJE_OP_READ		12
#define RJE_OP_WRITE		13
#define RJE_OP_LABEL		14
#define RJE_OP_DATA		15
#define RJE_OP_ABORT		16

#define FTP_OP_CTL		20
#define FTP_OP_DATA 		21
#define FTP_OP_DATA_ACK 	22
#define FTP_OP_DATA_EOF 	23
#define FTP_OP_ACK		24
#define FTP_OP_CONSOLE		25   /* Message from host */
#define FTP_OP_DISPLAY		26   /* Display data in .buff field */
#define FTP_OP_DISP_ASK 	27   /* Display data in .buff field and get host
                                   response */
#define FTP_OP_EXIT 		28      /* Exit client FTP on host */

#define SOCKET_OP		32


#define RESPONSE_OP     255
#define OK                       1
#define GENERAL_EXCEPTION        0

#define RETRIEVE_OP              6
#define STATUS_CHANGE           15
#define TABLE_UPDATE            40


#define SWITCH_OP       13
#define PROGRAM_TO_PROGRAM 14
#define SYS_OP          15

#define DEAD_MAN_OP     222

#define SENDALL_OP               7

#define MANUAL    0
#define AUTOMATIC 1

#define VIEWSCREEN 22
#define MODSCREEN  32
#define ADDSCREEN  33

#define MODIFY  5
#define UNI_DELETE	4
#define ADD     3

#define FROZEN  99


#define STAT_RETREIVE		52
#define STAT_RETREIVE_NOMORE    53

#define BREAK      0
#define CONT       1

/** Main Menu Catagory symbols **/

#define CAT_CLUSTER	0
#define CAT_TCPIP	1
#define CAT_LINE	2
#define CAT_STATION	3
#define CAT_PROGRAM	4
#define CAT_USER	5
#define CAT_TRAN	6
#define CAT_FORM	7
#define CAT_MAP 	8
#define CAT_SMASK	9
#define CAT_UTILS	10







