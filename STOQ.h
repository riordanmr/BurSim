// STOQ.h

#define MSG_HEADER_LEN     450
#define MSG_HEADER_PREFIX  "!!MCSHeader "

#define MSG_HEADER_CONNID_OFFSET 12
#define MSG_HEADER_CONNID_LEN    6

int CrackSTOQList(CString strSTOQs, CStringArray &arySTOQs);
void InitSTOQs();

class CSTOQ {
public:
   HANDLE   m_hEventSTOQ;
   CStringList *m_pSTOQList;
   CRITICAL_SECTION m_critSTOQ;

public:
   CSTOQ();
   CSTOQ(LPCTSTR szSTOQName);
   void RemoveAll();
   int AddTail(LPCTSTR Buf);
   BOOL GetHead(CString &strBuf, int &nRemaining);

   static CSTOQ *GetSTOQPtr(LPCTSTR szSTOQName);
};
