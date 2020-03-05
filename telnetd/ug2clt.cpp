/*--- ug2clt.cpp -- Functions for talking to telnet client.
 * Used by FkUG.
 *
 * Copyright (C) 1997, Standard Networks, Inc.
 * Written by Mark Riordan
 */
#include "stdafx.h"
#include "Conn.h"
///#include "MDebug.h"
#include "ugstuff.h"

extern int Debug;
extern struct telnet_screen tel_screen_buf;

extern struct telnet_screen_buffs *tel_screen_buffs;
extern struct telnet_screen **TelScrBuffers;   
extern int MaxConnections;

/*--- function UGInitInboundConnection  -----------------------------------
 */
int
UGInitInboundConnection(int conn_index)
{
   int retval=0;

   if(Debug>2) FmtDebug("In UGInitInboundConnect, conn_index=%d",conn_index);
   tel_screen_buffs[conn_index].conn_id = conn_index;
   tel_screen_buffs[conn_index].cur_page = 0;
   tel_screen_buffs[conn_index].timeout_ticks = 0;
   tel_screen_buffs[conn_index].telnet_mode = TELNET_SERVER;

   telsrv_estab_conn(conn_index);

   return retval;
}

/*--- function UGInitAllInbound ----------------------------------------
 */
void
UGInitAllInbound(void)
{
   int conn_index;

   for(conn_index=0; conn_index<MaxConnections; conn_index++) {
      tel_screen_buffs[conn_index].stn_no = 0;
      tel_screen_buffs[conn_index].conn_id = -1;
   }
}
