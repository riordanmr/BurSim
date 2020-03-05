// SPO.cpp -- Implementation of SPO messages.

#include "stdafx.h"
#include "drawwnd.h"

extern CSPO SPO;
extern CString HostName;

CSPO::CSPO()
{
   InitializeCriticalSection(&m_critSPO);

   memset(m_spo_msgs,0,sizeof m_spo_msgs);
   for(int j=0; j<MAX_SPO_MSGS; j++) {
      memset(m_spo_msgs[j].msg,' ', MAXLINELEN);
   }
   m_spo_next_idx = 0;
}

void CSPO::LogSPOMsg(const char *flags, const char *text)
{
   EnterCriticalSection(&m_critSPO);

   strncpy(m_spo_msgs[m_spo_next_idx].msg, text, MAXLINELEN);
   BlankPad(m_spo_msgs[m_spo_next_idx].msg,MAXLINELEN);
   if(++m_spo_next_idx >= MAX_SPO_MSGS) {
      m_spo_next_idx = 0;
      m_n_wrapped++;
   }
   LeaveCriticalSection(&m_critSPO);
}

void InitialMsgs()
{
   CString strmsg;

   CTime mytime(time(NULL));
   SPO.LogSPOMsg("",mytime.Format("      ************************* Halt/Load **************** %m/%d/%Y %H:%M"));
   SPO.LogSPOMsg("","      MCPVS 3.30.R541A (DCOM STOQ DMS2) 13095");
   strmsg.Format("      System #: 0, Hostname: %s , Memory: 021MD, QwkMem: 000KD",
      HostName);
   SPO.LogSPOMsg("",strmsg);
}