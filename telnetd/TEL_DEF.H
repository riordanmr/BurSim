/***************************************************************************/
/*      								   */
/*			STANDARD NETWORKS, INC.             		   */
/*									   */
/*	Filename:  tel_def.h						   */
/*									   */
/*	Description: Include file for definitions used TELNET processing   */
/*			module.						   */
/*									   */
/*	Revison History							   */
/*		Date	  By	 Description				   */
/*              --------- ---    -----------                               */
/*              31 May 91 DAE	 Created                                   */
/*									   */
/*	Technical Information:                                             */
/*                                        				   */
/***************************************************************************/

/*
 * Definitions for the TELNET protocol.
 */
#ifndef TELNET_DEF_H
#define TELNET_DEF_H

#define         TELNET_TOTAL_CONNS     MAXSTATIONS



#define TELNET_CLIENT              1       /* Code to use if we are a TELNET client */

#define TELNET_CLIENT_OPEN         2       /* Code to use if we are a opening a TELNET client */

#define TELNET_SERVER              3       /* Code to use if we are a TELNET server */


#define 	TERM_TYPE_LEN	       40	/* Length of TERM_TYPE string */
#define         TERM_LOC_LEN           18        /* Locatin of terminal */

#define 	STATUS_LEN	       80

#ifdef UNI_VT
#define 	MAX_PAGES		10
#else

#ifdef UNI_TD
#define         MAX_PAGES               1
#endif

#endif


/* Terminals that we know about */

#define TELNET_TERM_UNKNOWN 0
#define TELNET_TERM_TD830   1
#define TELNET_TERM_VT100   2
#define TELNET_TERM_VT220   3
#define TELNET_TERM_3278    4
#define TELNET_TERM_VT102   5
#define TELNET_TERM_PRT132  6
#define TELNET_TERM_5250    7
#define TELNET_MAX_TERMTYPE 8

/* If we get this far in the negotiation,
 * we can't agree on a terminal type.
 */
#define TELNET_TERM_GIVE_UP 8
#define TELNET_TERM_NEXT_INDEX  8

#define TELNET_TERM_TD830A   20

/* Possible TELNET connection states */

#define TELSTATE_DATA	  (uchar8) 0
#define TELSTATE_IAC	  (uchar8) 1
#define TELSTATE_WILL	  (uchar8) 2
#define TELSTATE_WONT	  (uchar8) 3
#define TELSTATE_DO	  (uchar8) 4
#define TELSTATE_DONT	  (uchar8) 5
#define TELSTATE_SB	  (uchar8) 6
#define TELSTATE_SB_OPCODE (uchar8) 7
#define TELSTATE_SB_IS	  (uchar8) 8
#define TELSTATE_SB_SEND  (uchar8) 9
#define TELSTATE_SB_IAC   (uchar8) 10

#define TELSTATE_ESC	  (uchar8) 11
#define TELSTATE_ESC_CSI  (uchar8) 12
#define TELSTATE_ESCC	  (uchar8) 13
#define TELSTATE_ESCI	  (uchar8) 14
#define TELSTATE_ESCD	  (uchar8) 15
#define TELSTATE_ESCO	  (uchar8) 16
#define TELSTATE_HELP	  (uchar8) 17

#define TELSTATE_ESC1	  (uchar8) 18
#define TELSTATE_ESC2	  (uchar8) 19
#define TELSTATE_ESC3	  (uchar8) 20
#define TELSTATE_ESC4	  (uchar8) 21
#define TELSTATE_ESC5	  (uchar8) 22
#define TELSTATE_ESC6	  (uchar8) 23
#define TELSTATE_ESC7	  (uchar8) 24
#define TELSTATE_ESC8	  (uchar8) 25
#define TELSTATE_ESC9	  (uchar8) 26

#define TELSTATE_ESC16    (uchar8) 29
#define TELSTATE_ESC17    (uchar8) 30
#define TELSTATE_ESC18	  (uchar8) 31
#define TELSTATE_ESC19	  (uchar8) 32
#define TELSTATE_ESC20	  (uchar8) 33
#define TELSTATE_ESC21	  (uchar8) 34
#define TELSTATE_ESC22	  (uchar8) 35
#define TELSTATE_ESC23	  (uchar8) 36
#define TELSTATE_ESC24	  (uchar8) 37

#define TELSTATE_CNTL1    (uchar8) 40
#define TELSTATE_CNTL2    (uchar8) 41

#define TELSTATE_ESCE     (uchar8) 42

#define TELSTATE_ESC25    (uchar8) 43
#define TELSTATE_ESC26    (uchar8) 44
#define TELSTATE_ESC27    (uchar8) 45
#define TELSTATE_ESC28    (uchar8) 46
#define TELSTATE_ESC29    (uchar8) 47
#define TELSTATE_ESC30    (uchar8) 48
#define TELSTATE_ESC31    (uchar8) 49
#define TELSTATE_ESC32    (uchar8) 50
#define TELSTATE_ESC33    (uchar8) 51
#define TELSTATE_ESC34    (uchar8) 52
#define TELSTATE_ESC35    (uchar8) 53
#define TELSTATE_ESC36    (uchar8) 54
#define TELSTATE_ESC37    (uchar8) 55
#define TELSTATE_ESC38    (uchar8) 56
#define TELSTATE_ESC39    (uchar8) 57
#define TELSTATE_ESC40    (uchar8) 58
#define TELSTATE_ESC41    (uchar8) 59
#define TELSTATE_ESC42    (uchar8) 60
#define TELSTATE_ESC43    (uchar8) 61
#define TELSTATE_ESC44    (uchar8) 62
#define TELSTATE_ESC45    (uchar8) 63
#define TELSTATE_ESC46    (uchar8) 64
#define TELSTATE_ESC47    (uchar8) 65
#define TELSTATE_ESC48    (uchar8) 66
#define TELSTATE_ESC49    (uchar8) 67
#define TELSTATE_ESC50    (uchar8) 68

#define TELSTATE_ESCA     (uchar8) 69
#define TELSTATE_ESCP	  (uchar8) 70
#define TELSTATE_PRINT_PROMPT	(uchar8) 71


/* TELNET TERM state */
#define TERMSTATE_LOCAL   (uchar8) 0
#define TERMSTATE_RECV1   (uchar8) 1
#define TERMSTATE_RECV2   (uchar8) 2
#define TERMSTATE_RECV3   (uchar8) 3
#define TERMSTATE_RECV4   (uchar8) 4

/*  TELNET option states */
#define TELOPT_SUPPORTED_OUT	(uchar8) 0X01
#define TELOPT_SUPPORTED_IN	(uchar8) 0X02
#define TELOPT_ACTIVE_OUT	(uchar8) 0x04
#define TELOPT_ACTIVE_IN	(uchar8) 0x08
#define TELOPT_DO_ASKING	(uchar8) 0x10
#define TELOPT_WILL_ASKING	(uchar8) 0x20

/* Terminal FORMS or DATA mode */

#define TELTERM_DATA	    (uint32) 0x00000001 /* Data mode. */
#define TELTERM_LEFT_JUST   (uint32) 0x00000002 /* Left justified field */
#define TELTERM_RIGHT_JUST  (uint32) 0x00000004 /* Right justified field */
#define TELTERM_UNUSED1     (uint32) 0x00000008
#define TELTERM_BRIGHT	    (uint32) 0x00000010 /* Bright characters */
#define TELTERM_UNDERLINE   (uint32) 0x00000020 /* Underline */
#define TELTERM_REVERSE     (uint32) 0x00000040 /* Reverse video */
#define TELTERM_BLINK	    (uint32) 0x00000080 /* Blink */
#define TELTERM_SECURE	    (uint32) 0x00000100 /* Secure highlight */

/* DEC VT mode functions */

#define TELNET_VT_CLEAR_EOL     (uint16)  1
#define TELNET_VT_CLEAR_EOP	(uint16)  2
#define TELNET_VT_CLEAR_SCREEN	(uint16)  3
#define TELNET_VT_DEL_CHAR	(uint16)  4
#define TELNET_VT_DEL_LINE	(uint16)  5
#define TELNET_VT_HELP_SCREEN	(uint16)  6
#define TELNET_VT_HOME_CURSOR	(uint16)  7
#define TELNET_VT_INSERT_CHAR	(uint16)  8
#define TELNET_VT_INSERT_LINE	(uint16)  9
#define TELNET_VT_PAGE_DOWN	(uint16) 10
#define TELNET_VT_PAGE_UP	(uint16) 11
#define TELNET_VT_REFRESH	(uint16) 12
#define TELNET_VT_SPECIFY	(uint16) 13
#define TELNET_VT_TOGGLE_FORMS	(uint16) 14
#define TELNET_VT_TOGGLE_STATUS (uint16) 15
#define TELNET_VT_TOGGLE_TERM_MODE (uint16) 16
#define TELNET_VT_XMIT		(uint16) 17
#define TELNET_VT_XMIT_LINE	(uint16) 18

#define TELNET_VT_UP_ARROW	(uint16) 19
#define TELNET_VT_DOWN_ARROW	(uint16) 20
#define TELNET_VT_LEFT_ARROW	(uint16) 21
#define TELNET_VT_RIGHT_ARROW	(uint16) 22

#define TELNET_VT_NORMAL	(uint16) 23

#define TELNET_VT_NO_FUNC	(uint16) 24

#define TELNET_VT_TOGGLE_FORMS_XMIT  (uint16) 25
#define TELNET_VT_CNTL       (uint16) 26
#define TELNET_VT_CLOSE_CONN    (uint16) 27
#define TELNET_VT_TOGGLE_HOME_BELL  (uint16) 28

#define TELNET_VT_BACK_TAB      (uint16) 29

#define TELNET_VT_ROLL_UP       (uint16) 40
#define TELNET_VT_ROLL_DOWN       (uint16) 41

#define TELNET_VT_TOGGLE_ALT_DELIM       (uint16) 42

#define TELNET_VT_EXCHG_BELOW       (uint16) 43
#define TELNET_VT_EXCHG_ABOVE       (uint16) 44

#define TELNET_VT_END_LINE        (uint16) 45
#define TELNET_VT_END_TEXT        (uint16) 46

#define TELNET_VT_DES_BACK        (uint16) 47

#define TELNET_VT_LOCAL           (uint16) 48
#define TELNET_VT_RECV            (uint16) 49

#define TELNET_VT_AUTO_FLIP        (uint16) 50

#define TELNET_VT_TOGGLE_LINE_25    (uint16) 51

#define TELNET_VT_PRINT_PROMPT	    (uint16) 52
#define TELNET_VT_PRINT_SCREEN	    (uint16) 53

#define TELNET_DUMMY            (uint16) 99

/* DEC VT Function Keys */

#define TELNET_VT_KEY_PF1	(uint16)  1
#define TELNET_VT_KEY_PF2	(uint16)  2
#define TELNET_VT_KEY_PF3	(uint16)  3
#define TELNET_VT_KEY_PF4	(uint16)  4

#define TELNET_VT_KEY_F5        (uint16)  5
#define TELNET_VT_KEY_F6        (uint16)  6
#define TELNET_VT_KEY_F7	(uint16)  7
#define TELNET_VT_KEY_F8	(uint16)  8
#define TELNET_VT_KEY_F9	(uint16)  9
#define TELNET_VT_KEY_F10	(uint16) 10
#define TELNET_VT_KEY_F11	(uint16) 11
#define TELNET_VT_KEY_F12	(uint16) 12

#define TELNET_VT_KEY_LEFT_ARROW    (uint16) 16
#define TELNET_VT_KEY_RIGHT_ARROW   (uint16) 17
#define TELNET_VT_KEY_UP_ARROW	    (uint16) 18
#define TELNET_VT_KEY_DOWN_ARROW    (uint16) 19

#define TELNET_VT_EDIT_FIND	(uint16) 21
#define TELNET_VT_EDIT_INSERT	(uint16) 22
#define TELNET_VT_EDIT_REMOVE	(uint16) 23
#define TELNET_VT_EDIT_SELECT	(uint16) 24
#define TELNET_VT_EDIT_PREV	(uint16) 25
#define TELNET_VT_EDIT_NEXT	(uint16) 26

/**
#define TELNET_VT_ESC_A 	(uint16) 31
**/

#define TELNET_VT_ESC_B 	(uint16) 32
/***	#define TELNET_VT_ESC_C 	(uint16) 33
        #define TELNET_VT_ESC_D         (uint16) 34
#define TELNET_VT_ESC_E         (uint16) 35 ***/
#define TELNET_VT_ESC_F 	(uint16) 36
#define TELNET_VT_ESC_G 	(uint16) 37
#define TELNET_VT_ESC_H 	(uint16) 38
/***	#define TELNET_VT_ESC_I 	(uint16) 39    ***/
#define TELNET_VT_ESC_J 	(uint16) 40
#define TELNET_VT_ESC_K 	(uint16) 41
#define TELNET_VT_ESC_L 	(uint16) 42
#define TELNET_VT_ESC_M 	(uint16) 43
#define TELNET_VT_ESC_N 	(uint16) 44
#define TELNET_VT_ESC_O 	(uint16) 45
/**
#define TELNET_VT_ESC_P 	(uint16) 46
**/
#define TELNET_VT_ESC_Q 	(uint16) 47
#define TELNET_VT_ESC_R 	(uint16) 48
#define TELNET_VT_ESC_S 	(uint16) 49
#define TELNET_VT_ESC_T 	(uint16) 50
#define TELNET_VT_ESC_U 	(uint16) 51
#define TELNET_VT_ESC_V 	(uint16) 52
#define TELNET_VT_ESC_W 	(uint16) 53
/***    #define TELNET_VT_ESC_X         (uint16) 54     ***/
#define TELNET_VT_ESC_Y 	(uint16) 55
#define TELNET_VT_ESC_Z 	(uint16) 56

#define TELNET_VT_ESC_PF1	(uint16) 61
#define TELNET_VT_ESC_PF2	(uint16) 62
#define TELNET_VT_ESC_PF3	(uint16) 63
#define TELNET_VT_ESC_PF4	(uint16) 64

#define TELNET_VT_ESC_F6	(uint16) 66
#define TELNET_VT_ESC_F7	(uint16) 67
#define TELNET_VT_ESC_F8	(uint16) 68
#define TELNET_VT_ESC_F9	(uint16) 69
#define TELNET_VT_ESC_F10	(uint16) 70
#define TELNET_VT_ESC_F11	(uint16) 71
#define TELNET_VT_ESC_F12	(uint16) 72

#define TELNET_VT_ESC_LEFT_ARROW    (uint16) 76
#define TELNET_VT_ESC_RIGHT_ARROW   (uint16) 77
#define TELNET_VT_ESC_UP_ARROW	    (uint16) 78
#define TELNET_VT_ESC_DOWN_ARROW    (uint16) 79

#define TELNET_VT_ESC_CL        (uint16) 80
#define TELNET_VT_ESC_CP	(uint16) 81
#define TELNET_VT_ESC_CS	(uint16) 82
#define TELNET_VT_ESC_DC	(uint16) 83
#define TELNET_VT_ESC_DL	(uint16) 84
#define TELNET_VT_ESC_IC	(uint16) 85
#define TELNET_VT_ESC_IL	(uint16) 86

#define TELNET_VT_ESC_X         (uint16) 87

#define TELNET_VT_ESC_CB        (uint16) 88

#define TELNET_VT_ESC_TAB       (uint16) 89

#define TELNET_VT_ESC_CD        (uint16) 90

#define TELNET_VT_ESC_EL        (uint16) 91
#define TELNET_VT_ESC_ET        (uint16) 92

#define TELNET_VT_ESC_DB        (uint16) 93

#define TELNET_VT_KEY_F13       (uint16) 94
#define TELNET_VT_KEY_F14       (uint16) 95
#define TELNET_VT_KEY_F15       (uint16) 96
#define TELNET_VT_KEY_F16       (uint16) 97
#define TELNET_VT_KEY_F17       (uint16) 98
#define TELNET_VT_KEY_F18       (uint16) 99
#define TELNET_VT_KEY_F19       (uint16) 100
#define TELNET_VT_KEY_F20       (uint16) 101

#define TELNET_VT_ESC_F13       (uint16) 102
#define TELNET_VT_ESC_F14       (uint16) 103
#define TELNET_VT_ESC_F15       (uint16) 104
#define TELNET_VT_ESC_F16       (uint16) 105
#define TELNET_VT_ESC_F17       (uint16) 106
#define TELNET_VT_ESC_F18       (uint16) 107
#define TELNET_VT_ESC_F19       (uint16) 108
#define TELNET_VT_ESC_F20       (uint16) 109

#define TELNET_VT_SHFT_F8      (uint16) 110
#define TELNET_VT_SHFT_F9      (uint16) 111
#define TELNET_VT_SHFT_F11     (uint16) 112
#define TELNET_VT_SHFT_F12     (uint16) 113
#define TELNET_VT_SHFT_F16     (uint16) 114
#define TELNET_VT_SHFT_TAB     (uint16) 115

#define TELNET_VT_ESC_AF        (uint16) 116
#define TELNET_VT_ESC_AS        (uint16) 117

#define TELNET_VT_ESC_PP	(uint16) 118
#define TELNET_VT_ESC_PS	(uint16) 119

/* TELNET Commands */

#define TELNET_IAC      255             /* interpret as command: */
#define	TELNET_DONT	254		/* you are not to use option */
#define	TELNET_DO	253		/* please, you use option */
#define	TELNET_WONT	252		/* I won't use option */
#define	TELNET_WILL	251		/* I will use option */
#define	TELNET_SB	250		/* interpret as subnegotiation */
#define	TELNET_GA	249		/* you may reverse the line */
#define	TELNET_EL	248		/* erase the current line */
#define	TELNET_EC	247		/* erase the current character */
#define	TELNET_AYT	246		/* are you there */
#define	TELNET_AO	245		/* abort output--but let prog finish */
#define	TELNET_IP	244		/* interrupt process--permanently */
#define	TELNET_BREAK	243		/* break */
#define	TELNET_DM	242		/* data mark--for connect. cleaning */
#define	TELNET_NOP	241		/* nop */
#define	TELNET_SE	240		/* end sub negotiation */
#define TELNET_EOR      239             /* end of record (transparent mode) */

#define TELNET_SYNCH	242		/* for telfunc calls */

/* telnet options */
#define TELOPT_BINARY	(uint8) 0	/* 8-bit data path */
#define TELOPT_ECHO	1	/* echo */
#define	TELOPT_RCP	2	/* prepare to reconnect */
#define	TELOPT_SGA	3	/* suppress go ahead */
#define	TELOPT_NAMS	4	/* approximate message size */
#define	TELOPT_STATUS	5	/* give status */
#define	TELOPT_TM	6	/* timing mark */
#define	TELOPT_RCTE	7	/* remote controlled transmission and echo */
#define TELOPT_NAOL 	8	/* negotiate about output line width */
#define TELOPT_NAOP 	9	/* negotiate about output page size */
#define TELOPT_NAOCRD	10	/* negotiate about CR disposition */
#define TELOPT_NAOHTS	11	/* negotiate about horizontal tabstops */
#define TELOPT_NAOHTD	12	/* negotiate about horizontal tab disposition */
#define TELOPT_NAOFFD	13	/* negotiate about formfeed disposition */
#define TELOPT_NAOVTS	14	/* negotiate about vertical tab stops */
#define TELOPT_NAOVTD	15	/* negotiate about vertical tab disposition */
#define TELOPT_NAOLFD	16	/* negotiate about output LF disposition */
#define TELOPT_XASCII	17	/* extended ascic character set */
#define	TELOPT_LOGOUT	18	/* force logout */
#define	TELOPT_BM	19	/* byte macro */
#define	TELOPT_DET	20	/* data entry terminal */
#define	TELOPT_SUPDUP	21	/* supdup protocol */
#define	TELOPT_SUPDUPOUTPUT 22	/* supdup output */
#define	TELOPT_SNDLOC	23	/* send location */
#define	TELOPT_TTYPE	24	/* terminal type */
#define	TELOPT_EOR	25	/* end or record */
#define TELOPT_EXOPL	255	/* extended-options-list */

#define NTELOPTS	(TELOPT_EOR+1)

/* sub-option qualifiers */
#define	TELQUAL_IS	0	/* option is... */
#define	TELQUAL_SEND	1	/* send option */

// HEAT terminal type definitions.
#define WRUTERMTYPE_TD830     1
#define WRUTERMTYPE_RAW       2
#define WRUTERMTYPE_3278      4
#define WRUTERMTYPE_5250      7
#define WRUTERMTYPE_ANY       127

#endif /* TELNET_DEF_H */

/*********************************** eof ***********************************/
