// Job.h 

#define FIRST_ASSIGNABLE_MIX_NUM 2
#define DEFAULT_UNISYS_PRIORITY  6
#define MIX_STATUS_EXECUTING     "EXECUTING"
#define MIX_STATUS_COMPLEX_WAIT  "WTG COMPLEX WAIT"
#define MIX_STATUS_WAIT_ACCEPT   "WTG ACCEPT"

class CJob {
public:
   BOOL     m_inuse;
   int      m_mixnum;
   CString  m_progname;
   CString  m_scripttext;
   int      m_priority;
   int      m_memory;  // I guess this is in kilodigits?
   CString  m_strStatus;
   DWORD    m_threadid;
   HANDLE   m_hthread;
   CTime    m_time_start;
   CString  m_odt_input;
   HANDLE   m_hEventODT;
   CArray <CBSFormFile, CBSFormFile &> m_aryFormFiles;
public:
   CJob();
   void CJobCopy(const CJob &other, CJob &cjob_new);
   CJob operator =(const CJob &other);
   CJob(const CJob &other);

   void ExecuteCommandFromProgram(CString strCmd);
   void LogSPOMsgFromProgram(CString strMsg);
   static void SortJobs(CDWordArray &arySortedMix);
};
