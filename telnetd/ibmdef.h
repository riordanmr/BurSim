/***********************************************************************
 ** ibmdef.h                                                          **
 **     IBM Data Declarations                                         **
 **                                                                   **
 ***********************************************************************/

/*\
 * IBM definitions
\*/

  /* 3270 Data stream characters */

#define EW          0xF5
#define W           0xF1

#define SEL	    0x04
#define PT          0x05
#define SBA         0x11
#define EUA         0x12
#define IC          0x13
#define NL          0x15
#define SF          0x1D
#define SA          0x28
#define SFE         0x29
#define MF          0x2C
#define TRN	    0x35
#define RA          0x3C

#define ENP         0x14
#define INP         0x24
#define IRS         0x1E

  /* WCC bit fields */

#define WCC_PRINT   0x08
#define WCC_ALARM   0x04
#define WCC_RESTORE 0x02
#define WCC_RESET   0x01

  /* Attribute bit fields */

#define ATTR_UNPR   0
#define ATTR_PROT   0x20
#define ATTR_NUM    0x10
#define ATTR_DISP   0x0C
#define ATTR_MDT    0x01

#define DISP_NRM    0x00
#define DISP_DET    0x04
#define DISP_BRT    0x08
#define DISP_HID    0x0C

#define ATTR_EDISP  0x42

#define EDISP_NRM   0x00
#define EDISP_BLINK 0x02
#define EDISP_INV   0x40
#define EDISP_UND   0x42

// IS_IBM_NONTEXT is TRUE iff the parameter 
// is a field or attribute byte.
#define IS_IBM_NONTEXT(x) (x & 0x80)
// IS_IBM_DATA_FIELD is TRUE iff the parameter
// is a field byte (not just attribute).
#define IS_IBM_DATA_FIELD(x) ((x & 0xA0) == 0x80)
// IS_IBM_PROT_OR_UNPROT_FIELD is TRUE iff the parameter
// is a field byte--protected or otherwise.  
// This was readded 11 May 99
// to prevent overwrite of protected fields.
#define IS_IBM_PROT_OR_UNPROT_FIELD(x) (x & 0x80) 

  /* AID values */

#define NO_AID      0x60
#define AID_ENTER   0x7D
#define AID_PF1     0xF1
#define AID_PF2     0xF2
#define AID_PF3     0xF3
#define AID_PF4     0xF4
#define AID_PF5     0xF5
#define AID_PF6     0xF6
#define AID_PF7     0xF7
#define AID_PF8     0xF8
#define AID_PF9     0xF9
#define AID_PF10    0x7A
#define AID_PF11    0x7B
#define AID_PF12    0x7C
#define AID_PF13    0xC1
#define AID_PF14    0xC2
#define AID_PF15    0xC3
#define AID_PF16    0xC4
#define AID_PF17    0xC5
#define AID_PF18    0xC6
#define AID_PF19    0xC7
#define AID_PF20    0xC8
#define AID_PF21    0xC9
#define AID_PF22    0x4A
#define AID_PF23    0x4B
#define AID_PF24    0x4C
#define AID_PA1     0x6C
#define AID_PA2     0x6E
#define AID_PA3     0x6B
#define AID_CLEAR   0x6D

  /* Key values */

#define PA1_KEY     25
#define CLR_KEY     28
#define MAX_AID_KEY 28

#define REFRESH_KEY 29
#define ATTN_KEY    30
#define SYSREQ_KEY  31
#define SYSREQ_5250_KEY  71

#define CF1_KEY     41
#define MAX_CF_KEY  52

#define MAX_KEY     52

#define UNIGATE_ROLL_DOWN_KEY    67
#define UNIGATE_ROLL_UP_KEY      68
#define UNIGATE_HELP_KEY         61
#define UNIGATE_PA1_KEY          25
