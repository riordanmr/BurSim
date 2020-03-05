// Job.cpp

#include "stdafx.h"

//=== Global Variables ===========================================
extern CSPO SPO;
extern CArray<CJob, CJob &> Mix;
extern CRITICAL_SECTION critMix;

CJob::CJob()
{
   m_inuse = FALSE;
   m_hEventODT = NULL;
   m_priority = 6;
   m_threadid = 0;
}

void CJob::CJobCopy(const CJob &other, CJob &cjob_new)
{
   cjob_new.m_inuse = other.m_inuse;
   cjob_new.m_mixnum = other.m_mixnum;
   cjob_new.m_progname = other.m_progname;
   cjob_new.m_scripttext = other.m_scripttext;
   cjob_new.m_priority = other.m_priority;
   cjob_new.m_memory = other.m_memory;
   cjob_new.m_strStatus = other.m_strStatus;
   cjob_new.m_threadid = other.m_threadid;
   cjob_new.m_hthread = other.m_hthread;
   cjob_new.m_time_start = other.m_time_start;
   cjob_new.m_odt_input = other.m_odt_input;
   cjob_new.m_hEventODT = other.m_hEventODT;
   int size = other.m_aryFormFiles.GetSize();
   for(int j=0; j<size; j++) {
      cjob_new.m_aryFormFiles[j] = other.m_aryFormFiles[j];
   }
}

CJob CJob::operator =(const CJob &other)
{
   CJobCopy(other,*this);
   return *this;
}

// Copy constructor
CJob::CJob(const CJob &other)
{
   CJobCopy(other,*this);
}

void CJob::LogSPOMsgFromProgram(CString strMsg)
{
   CString strSPOMsg;
   strSPOMsg.Format("      %s=%-3.3d %s",m_progname, m_mixnum, strMsg);
   SPO.LogSPOMsg("",strSPOMsg);
}

void CJob::ExecuteCommandFromProgram(CString strCmd)
{
   CString strSPOMsg;
   strSPOMsg.Format("ZIP: %s", strCmd);
   LogSPOMsgFromProgram(strSPOMsg);

   ExecuteCommand(strCmd, FALSE);
}

void ShellSortMixArray(CDWordArray & ary, int (__cdecl *compare )(DWORD va11, DWORD val22 ) )
{
#define STRIDE_FACTOR 3
   int c, stride;
   int d;
   int found;
   int nElements = ary.GetSize();
   DWORD dwtemp;

   stride = 1;
   while (stride <= nElements)
      stride = stride * STRIDE_FACTOR + 1;

   while (stride > (STRIDE_FACTOR - 1)) {
      stride = stride / STRIDE_FACTOR;
      for (c = stride; c < nElements; c++) {
         found = 0;
         d = c - stride;
         while ((d >= 0) && !found) {
            if (compare (ary[d + stride],ary[d]) < 0) {
               dwtemp = ary[d];
               ary[d] = ary[d + stride];
               ary[d + stride] = dwtemp;
               d -= stride;
            } else {
               found = 1;
            }
         }
      }
   }
}

/*--- function CompareMix -----------------------------------
 *  Higher priority jobs come before lower priority.
 *  For two jobs at same priority, lower mix number comes first.
 */
int CompareMix(DWORD mix1, DWORD mix2) {
   int retval=0;

   if(Mix[mix1].m_priority > Mix[mix2].m_priority) {
      retval = -1;
   } else if(Mix[mix1].m_priority < Mix[mix2].m_priority) {
      retval = 1;
   } else if(Mix[mix1].m_mixnum < Mix[mix2].m_mixnum) {
      retval = -1;
   } else if(Mix[mix1].m_mixnum < Mix[mix2].m_mixnum) {
      retval = 1;
   } 
   return retval;
}

/*--- function CJob::SortJobs -----------------------------------
 */
void CJob::SortJobs(CDWordArray &arySortedMix)
{
   arySortedMix.RemoveAll();
   int imix;

   for(imix=FIRST_ASSIGNABLE_MIX_NUM; imix<Mix.GetSize(); imix++) {
      if(Mix[imix].m_inuse) {
         arySortedMix.Add((DWORD) Mix[imix].m_mixnum);
      }
   }
   ShellSortMixArray(arySortedMix, CompareMix);
}

/*--- function UnisysToNTPriority -----------------------------------
 */
int UnisysToNTPriority(int unisysPri)
{
   int ntPri=THREAD_PRIORITY_NORMAL;
   if(unisysPri >= 0 && unisysPri < 3) {
      ntPri = THREAD_PRIORITY_LOWEST;
   } else if(unisysPri < 6) {
      ntPri = THREAD_PRIORITY_BELOW_NORMAL;
   } else if(unisysPri > 6) {
      ntPri = THREAD_PRIORITY_ABOVE_NORMAL;
   }
   return ntPri;
}