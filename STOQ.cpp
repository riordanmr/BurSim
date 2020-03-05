// STOQ.cpp
//
// Mark Riordan   6 Sep 1999  Placed into public domain

#include "stdafx.h"

//=== Global Variables ===========================================
extern int Debug;
CMap <CString, CString, CSTOQ *, CSTOQ *> MapSTOQs;
CRITICAL_SECTION critMapSTOQ;

inline UINT AFXAPI HashKey(CString key)
{
	UINT nHash = 0;
      int nchars = key.GetLength();
	while (nchars--)
		nHash = (nHash<<5) + nHash + key.GetAt(nchars);
	return nHash;
}

/*--- function CrackSTOQList -----------------------------------
 */
int CrackSTOQList(CString strSTOQs, CStringArray &arySTOQs)
{
   arySTOQs.RemoveAll();
   CString strArg;
   strSTOQs.TrimRight();
   do {
      strSTOQs.TrimLeft();
      strArg = strSTOQs.SpanExcluding("|");
      if(strArg.GetLength()) {
         arySTOQs.Add(strArg);
         // Strip off token just cracked off.
         strSTOQs = strSTOQs.Mid(strArg.GetLength());
      } else {
         break;
      }
   } while(TRUE);
   return arySTOQs.GetSize();
}

void InitSTOQs()
{
   InitializeCriticalSection(&critMapSTOQ);
}

CSTOQ::CSTOQ()
{
}

CSTOQ::CSTOQ(LPCTSTR szSTOQName)
{
   BOOL bManualReset = TRUE, bInitialState = FALSE;
   m_hEventSTOQ = CreateEvent(NULL, bManualReset, bInitialState, NULL);
   m_pSTOQList = new CStringList; 
   InitializeCriticalSection(&m_critSTOQ);
}

int CSTOQ::AddTail(LPCTSTR Buf) 
{
   int retval=0;
   EnterCriticalSection(&m_critSTOQ);
   m_pSTOQList->AddTail(Buf);
   SetEvent(m_hEventSTOQ);
   LeaveCriticalSection(&m_critSTOQ);

   return retval;
}

/*--- function GetHead -----------------------------------
 *
 *  Exit:   Returns TRUE if we were able to get and remove a list member,
 *          in which case
 *          strBuf   is the contents of the STOQ entry.
 */
BOOL CSTOQ::GetHead(CString &strBuf, int &nRemaining)
{
   BOOL bGot=TRUE;
   EnterCriticalSection(&m_critSTOQ);
   if(!m_pSTOQList->IsEmpty()) {
      strBuf = m_pSTOQList->RemoveHead();
      nRemaining = m_pSTOQList->GetCount();
   } else {
      bGot = FALSE;
      nRemaining = 0;
   }
   LeaveCriticalSection(&m_critSTOQ);
   return bGot;
}

void CSTOQ::RemoveAll()
{
   if(m_pSTOQList) m_pSTOQList->RemoveAll();
}

CSTOQ *CSTOQ::GetSTOQPtr(LPCTSTR szSTOQName)
{
   CSTOQ *p_STOQ;
   EnterCriticalSection(&critMapSTOQ);
   if(!MapSTOQs.Lookup(szSTOQName, p_STOQ)) {
      p_STOQ = new CSTOQ(szSTOQName);
      MapSTOQs.SetAt(szSTOQName, p_STOQ);
   }
   LeaveCriticalSection(&critMapSTOQ);
   return p_STOQ;
}