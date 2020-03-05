/*--- UGAlloc.CPP -- Buffer management for IntraGate.
 *  
 *  Copyright (C) 1997, Standard Networks, Inc.
 *  Written by Mark Riordan, 31 Jan 1997
 *  
 *  This module is used to allocate
 *  buffers used for screen data and other purposes.
 *
 *  Currently, IntraGate never frees buffers, though it
 *  does reuse them.  Therefore, these routines are pretty
 *  simple.  They can be beefed up later.
 *  The use of arrays as opposed to linked lists or some such
 *  dates to Unigate code.  This is a little crude, but probably
 *  harmless since any large chunks of memory are allocated
 *  as needed anyway.
 */

#include "stdafx.h"
#include "Conn.h"
#include "ugstuff.h"
///#include "mdebug.h"


//=== Global Variables =============================================
extern int Debug;

// tel_screen_buffs keeps track of certain connection parameters.
// It is indexed by connection number (1-MaxConnections).
// Each entry is rather small, so we can allocate the entire
// array once at startup.
struct telnet_screen_buffs *tel_screen_buffs;

// TelScrBuffers is an array of pointers to structures which
// contain additional connection information, plus an image
// of the screen.
// Each telnet_screen is rather large, so we allocate
// the entries dynamically rather than all at once at startup.
// However, the array itself (containing only pointers)
// is allocated all at once, at startup.
// A NULL pointer indicates that the buffer has not yet
// been allocated.
// scr_buffers takes the place of the older em_buffers.
struct telnet_screen **TelScrBuffers;   
int NAllocated_TelScrBuffers=0;

// telclt_buffers added by MRR for use by telnet_client.  Previously,
// it and em_from_host were using the same buffers.
// telclt_buffers is an array very similar to TelScrBuffers.
struct telnet_screen **telclt_buffers;
int NAllocated_telclt_buffers=0;
int Max_telclt_buffers=0;
int Max_TelScrBuffers = 0;


extern CRITICAL_SECTION csalloc_screen_buf;
extern CRITICAL_SECTION cstelclt_alloc_screen_buf;

//=== Function Prototypes ========================================
void InitOnePage(struct telnet_screen *tel_screen);


/*--- function UGInitializeBuffers --------------------------------
 *
 *  Initialize the buffers managed by this module.
 *
 *  Entry:  maxconn  is the maximum number of connections we want.
 *
 *  Exit:   returns 0 upon success.
 */
int UGInitializeBuffers(int maxconn)
{
   int retval=0;
#define SCREENS_PER_CONN 10
   int nbytes_TelScrBuffers = (sizeof(telnet_screen *))*(1+maxconn)*SCREENS_PER_CONN;
   int nbytes_telclt_buffers = (sizeof(telnet_screen *))*(1+maxconn);
   int nbytes_tel_screen_buffs = (sizeof(struct telnet_screen_buffs)+4)*(1+maxconn);

   TelScrBuffers = (struct telnet_screen **) malloc(nbytes_TelScrBuffers);
   if(!TelScrBuffers) {
      retval = 1;
   } 
   if(!retval) {
      memset(TelScrBuffers,0,nbytes_TelScrBuffers);
      Max_TelScrBuffers = SCREENS_PER_CONN*maxconn;
      tel_screen_buffs = (struct telnet_screen_buffs *) malloc(nbytes_tel_screen_buffs);
      if(!tel_screen_buffs) {
         retval = 2;
      } else {
         memset(tel_screen_buffs,0,nbytes_tel_screen_buffs);
      }
   }
   if(!retval) {
      telclt_buffers = (struct telnet_screen **) malloc(nbytes_telclt_buffers);
      Max_telclt_buffers = maxconn;
      if(!telclt_buffers) {
         retval = 1;
      } else {
         memset(telclt_buffers,0,nbytes_telclt_buffers);
      }
   }
   return retval;
}

/*--- function alloc_buf_common --------------------------------
 *
 *  Allocate a buffer.
 *
 *  Entry:  bufbase  is the array of pointers to buffers that we
 *                   will update
 *          nlargest_so_far   is the biggest buffer number we've
 *                   allocated so far.
 *
 *  Exit:   nlargest_so_far has been updated.
 *          bufbase  has a new entry in it.
 *          *num     is the array index of the buffer.
 *          Returns 0 upon failure, 1 upon success.
 */

short alloc_buf_common(struct telnet_screen **bufbase,
                       int &nlargest_so_far, int mymax, unsigned short *num)
{
   int retval=0,ibuf;
   int nbytes_to_alloc = sizeof(struct telnet_screen);
   BOOL found= FALSE;

   EnterCriticalSection(&csalloc_screen_buf);
   for(ibuf=1; !found && ibuf <=nlargest_so_far; ibuf++) {
      if(!bufbase[ibuf]) {
         *num = ibuf;
         found = TRUE;
      }
   }

   if(!found) {
      if(nlargest_so_far >= mymax) {
         retval = 0;
         if(Debug) FmtDebug("Tried to allocate buffer %d in alloc_buf_common",nlargest_so_far);
         *num = 0;
      } else {
         *num = 1+nlargest_so_far;
         found = TRUE;
      }
   }
   if(found) {
      bufbase[*num] = (struct telnet_screen *)malloc(nbytes_to_alloc); 
      if(!bufbase[*num]) {
         if(Debug) FmtDebug("Could not allocate buffer in alloc_buf_common");
      } else {
         memset(bufbase[*num],0,nbytes_to_alloc);
         nlargest_so_far++;
         retval = 1;
      }
   }
   LeaveCriticalSection(&csalloc_screen_buf);
   return retval;
}

/*--- function alloc_screen_buf -----------------------------------
 *  Allocates a buffer from TelScrBuffers for TD830 screens.
 *
 *
 *  Exit:   *num  is the number of the buffer allocated.
 *	         *type is set to 1, but this is ignored.
 *          Returns 1, else 0 if failure.
 */

short alloc_screen_buf (short *type, unsigned short *num)
{
   int retval = alloc_buf_common(TelScrBuffers, NAllocated_TelScrBuffers,
      Max_TelScrBuffers,num);
   struct telnet_screen *tel_screen;  

   if(retval) {
      tel_screen = TelScrBuffers[*num];
      InitOnePage(tel_screen);
   }

   return retval;
}

/*--- function free_screen_buf ----------------------------------
 */
int free_screen_buf(struct telnet_screen **bufbase,int num)
{
   int retval=0;

   if(bufbase[num]) free(bufbase[num]);
   bufbase[num] = 0;
   return retval;
}

/*--- function FreeScreenBuffsForConn ----------------------------------
 */
int FreeScreenBuffsForConn(int conn_index)
{
   int retval=0,i;
   
   for(i=0; i<MAX_PAGES; i++) {
		if(tel_screen_buffs[conn_index].buf_num[i] != -1) {
         free_screen_buf(TelScrBuffers,tel_screen_buffs[conn_index].buf_num[i]);
         tel_screen_buffs[conn_index].buf_num[i] = -1;
      }
   }

   return retval;
}

/*--- function telclt_alloc_screen_buf ---------------------------------
 *
 *  Allocate a telnet_screen style buffer for use within telnet_client;
 *  Allocates it from telclt_buffers.
 *  
 */
short telclt_alloc_screen_buf (short *type, unsigned short *num)
{
   return alloc_buf_common(telclt_buffers,NAllocated_telclt_buffers,
      Max_telclt_buffers,num);
}

/*--- function store_screen_buf -------------------------------------
 *
 *  In UniGate, this routine originally was used to save a screen buffer
 *  possibly to disk, but now it does nothing.
 */
short store_screen_buf (short type, short num)
{
   return 1;
}

/*--- function ug_free_buffers --------------------------------------
 *
 *  Free allocated buffers.  Typically called at program
 *  termination to prevent debuggers from complaining.
 */
void ug_free_buffers(void)
{
   free(TelScrBuffers);
   free(telclt_buffers);
}

