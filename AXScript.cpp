// AXScript.cpp - Interface to Microsoft ActiveX Scripting 
//
// Adapted from MSDN Article ID Q221992
// Mark Riordan  22 Aug 1999

#include "stdafx.h"
// Include ActiveX Script definitions...
//#include <InitGuid.h>
#include <activscp.h>
// Include definition for MyScriptObject...
#include <stdio.h>
#include "BurSimScriptObject.h" 
#include "AxScript.h"
#include "\Lang\MSHosting\activdbg.h"

// I had to add this on 25 Feb 2004 because this became an unsatisfied external.
// Got this from http://www.codeguru.com/activex/actvdbg.shtml
// {51973C11-CB0C-11d0-B5C9-00A0244A0E7A}
DEFINE_GUID(IID_IActiveScriptSiteDebug, 0x51973C11, 0xcb0c, 0x11d0, 0xb5, 0xc9, 0x0, 0xa0, 0x24, 0x4a, 0x0e, 0x7a);


//=== Global Variables ===========================================
extern int Debug;
CArray<CJob, CJob &> Mix;
extern CRITICAL_SECTION critMix;
extern CString strScriptsPath;
extern CSPO SPO;
extern BOOL bRunDebuggerOnRuntimeError;

//=== Function prototypes ========================================
CString MakeODTEventName(CString strProgram);
CString MakeSTOQEventName(CString strSTOQ);
int UnisysToNTPriority(int unisysPri);

//--------------------------------------------------------------------

#if 0
// Incomplete stuff from Q232394
// See also activdbh.h from Q223389.
// This is the IActiveScriptSiteDebug interface, with a minimal implementation: 
{
   STDMETHOD GetDocumentContextFromPosition(
      DWORD dwSourceContext, 
      ULONG uCharacterOffset, 
      ULONG uNumChars, 
      IDebugDocumentContext **ppsc) {return E_NOTIMPL;}

   STDMETHOD GetApplication( IDebugApplication **ppda ) {return E_NOTIMPL;}

   STDMETHOD GetRootApplicationNode( IDebugApplicationNode **ppdanRoot) {return E_NOTIMPL;}
   
   STDMETHOD OnScriptErrorDebug( 
      IActiveScriptErrorDebug *pErrorDebug, 
      BOOL *pfEnterDebugger,
      BOOL *pfCallOnScriptErrorWhenContinuing) {return E_NOTIMPL;} 
}
#endif
EXTERN_C const IID IID_IActiveScriptSiteDebug;

//===  Stock class from Q221992  ==========================================

// Your IActiveScriptSite implementation...
class MyActiveScriptSite : public IActiveScriptSite 
{

private:
   class MyActiveScriptSiteDebug : public IActiveScriptSiteDebug // added by MRR
{
      STDMETHODIMP GetDocumentContextFromPosition(
         DWORD dwSourceContext, 
         ULONG uCharacterOffset, 
         ULONG uNumChars, 
         IDebugDocumentContext **ppsc) {return E_NOTIMPL;}

      STDMETHODIMP GetApplication( IDebugApplication **ppda ) { 
         SPO.LogSPOMsg("", "      Runtime error");
         return E_NOTIMPL;
      }

      STDMETHODIMP GetRootApplicationNode( IDebugApplicationNode **ppdanRoot) {return E_NOTIMPL;}
   
      STDMETHODIMP OnScriptErrorDebug( 
         IActiveScriptErrorDebug *pErrorDebug, 
         BOOL *pfEnterDebugger,
         BOOL *pfCallOnScriptErrorWhenContinuing) 
      {
         SPO.LogSPOMsg("", "      Runtime error (OnScriptErrorDebug)");
         return E_NOTIMPL;
      } 
      // Added by MRR:
   public:
      long __stdcall QueryInterface(const struct _GUID &,void ** ) { return E_NOTIMPL;}
      virtual ULONG _stdcall AddRef(void) {
         return 1; ////++m_dwRef;
      }
      virtual ULONG _stdcall Release(void) {
         return 0;
         ////if(--m_dwRef == 0) return 0;  return m_dwRef;
      }
      // End of added by MRR
   };

private:
   ULONG m_dwRef; // Reference count
   MyActiveScriptSiteDebug *m_MyActiveScriptSiteDebug;
public:
   IUnknown *m_pUnkScriptObject; // Pointer to your object that is exposed
                                 // to the script engine in GetItemInfo().
   
   MyActiveScriptSite::MyActiveScriptSite() {
      m_dwRef = 1;
      m_MyActiveScriptSiteDebug = new MyActiveScriptSiteDebug;
   }
   MyActiveScriptSite::~MyActiveScriptSite() {}
   
   // IUnknown methods...
   virtual HRESULT _stdcall QueryInterface(REFIID riid, void **ppvObject) {
      HRESULT hresult = E_NOTIMPL;
      // If we are being queried for the debugger interface, tell the caller
      // about our dummy IActiveScriptSiteDebug only if we are not configured
      // for debugging.  If we don't tell the caller about our dummy implementation,
      // the script host will call a default *real* debugger.
      if(!bRunDebuggerOnRuntimeError && IID_IActiveScriptSiteDebug == riid) {
         // MRR: See the LOCSERVE MSDN example for what to do here...
         *ppvObject = m_MyActiveScriptSiteDebug; 
         m_MyActiveScriptSiteDebug->AddRef();
         hresult = S_OK;
      } else {
         *ppvObject = NULL;
      }
      return hresult;
   }
   virtual ULONG _stdcall AddRef(void) {
      return ++m_dwRef;
   }
   virtual ULONG _stdcall Release(void) {
      if(--m_dwRef == 0) return 0;
      return m_dwRef;
   }
   
   // IActiveScriptSite methods...
   virtual HRESULT _stdcall GetLCID(LCID *plcid) {
      return S_OK;
   }
   
   virtual HRESULT _stdcall GetItemInfo(LPCOLESTR pstrName,
      DWORD dwReturnMask, IUnknown **ppunkItem, ITypeInfo **ppti) {

      // Is it expecting an ITypeInfo?
      if(ppti) {
         // Default to NULL.
         *ppti = NULL;
         
         // Return if asking about ITypeInfo... 
         if(dwReturnMask & SCRIPTINFO_ITYPEINFO)
            return TYPE_E_ELEMENTNOTFOUND;
      }
      
      // Is the engine passing an IUnknown buffer?
      if(ppunkItem) {
         // Default to NULL.
         *ppunkItem = NULL;
         
         // Is Script Engine looking for an IUnknown for our object?
         if(dwReturnMask & SCRIPTINFO_IUNKNOWN) {
            // Check for our object name...
            if (!_wcsicmp(L"MyObject", pstrName)) {
               // Provide our object.
               *ppunkItem = m_pUnkScriptObject;
               // Addref our object...
               m_pUnkScriptObject->AddRef();
            }
         }
      }
      
      return S_OK;
   }
   
   virtual HRESULT __stdcall GetDocVersionString(BSTR *pbstrVersion) {
      return S_OK;
   }
   
   virtual HRESULT __stdcall OnScriptTerminate(const VARIANT *pvarResult,
      const EXCEPINFO *pexcepInfo) {
      return S_OK;
   }
   
   virtual HRESULT __stdcall OnStateChange(SCRIPTSTATE ssScriptState) {
      return S_OK;
   }
   
   virtual HRESULT __stdcall OnScriptError(
      IActiveScriptError *pscriptError) {
      static BSTR pwcErrorText;
      pscriptError->GetSourceLineText(&pwcErrorText);

      AfxMessageBox(
         CString("IActiveScriptSite::OnScriptError()\n") + 
         CString("Line: ") + 
         CString(pwcErrorText),
         MB_SETFOREGROUND);
      ::SysFreeString(pwcErrorText);
      return S_OK;
   }
   
   virtual HRESULT __stdcall OnEnterScript(void) {
      return S_OK;
   }
   
   virtual HRESULT __stdcall OnLeaveScript(void) {
      return S_OK;
   }
};

// Global instance of our IActiveScriptSite implementation.
//MyActiveScriptSite g_iActiveScriptSite;

// Script Engine CLSIDs...
#include <initguid.h>
DEFINE_GUID(CLSID_VBScript, 0xb54f3741, 0x5b07, 0x11cf, 0xa4, 0xb0, 0x0,
   0xaa, 0x0, 0x4a, 0x55, 0xe8);
DEFINE_GUID(CLSID_JScript, 0xf414c260, 0x6ac0, 0x11cf, 0xb6, 0xd1, 0x00,
   0xaa, 0x00, 0xbb, 0xbb, 0x58);
DEFINE_GUID(CLSID_Python, 0xDF630910,  0x1c1d, 0x11d0, 0xae, 0x36, 0x8c, 
   0x0f, 0x5E, 0x00, 0x00, 0x00);
//PyScript_CLSID = "{DF630910-1C1D-11d0-AE36-8C0F5E000000}"

// Ole-initialization class.
class OleInitClass {
public:
   OleInitClass() {
      OleInitialize(NULL);
   }
   ~OleInitClass() {
      OleUninitialize();
   }
};

// This global class calls OleInitialize() at
// application startup, and calls OleUninitialize()
// at application exit...
// This is now local, to allow multiple simultaneous scripts.
//OleInitClass g_OleInitClass;

void HRVERIFY(HRESULT hr, char * msg)
{
   if(FAILED(hr)) {
      CString str;
      str.Format("Error: 0x%08lx (%s)", hr, msg);
      AfxMessageBox(str, 0x10000);
      _exit(0);
   }

} 

//===  End of Stock class from Q221992  ============================


BurSimRet CMRRAXScript::RunScript(TypScriptInfo *psi)
{
   BurSimRet retval=BE_NO_ERROR;
   MyActiveScriptSite local_iActiveScriptSite;

   // Initialize our IActiveScriptSite implementation with your
   // script object's IUnknown interface...
   local_iActiveScriptSite.m_pUnkScriptObject = 
      m_myScriptObject.GetInterface(&IID_IUnknown);

   m_myScriptObject.m_so_mixnum = psi->si_mixnum;

   // Start inproc script engine, VBSCRIPT.DLL or one of the others
   CLSID clsid_lang;
   switch(psi->si_lang) {
   case LANG_JSCRIPT:
      clsid_lang = CLSID_JScript;
      break;
   case LANG_PYTHON:
      clsid_lang = CLSID_Python;
      break;
   default:
      clsid_lang = CLSID_VBScript;
      break;
   }
   HRVERIFY(CoCreateInstance(clsid_lang, NULL, CLSCTX_INPROC_SERVER, 
      IID_IActiveScript, (void **)&m_iActiveScript), 
      "CoCreateInstance() for language-specific scripting engine");

   // Get engine's IActiveScriptParse interface.
   HRVERIFY(m_iActiveScript->QueryInterface(IID_IActiveScriptParse, 
      (void **)&m_iActiveScriptParse), 
      "QueryInterface() for IID_IActiveScriptParse");

   // Give engine our IActiveScriptSite interface...
   HRVERIFY(m_iActiveScript->SetScriptSite(&local_iActiveScriptSite), 
      "IActiveScript::SetScriptSite()");

   // Give the engine a chance to initialize itself...
   HRVERIFY(m_iActiveScriptParse->InitNew(),
      "IActiveScriptParse::InitNew()");

   // Add a root-level item to the engine's name space...
   HRVERIFY(m_iActiveScript->AddNamedItem(L"MyObject", 
      SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE),
      "IActiveScript::AddNamedItem()");	

   // Parse the code scriptlet...
   EXCEPINFO ei;
   CString strScriptText = psi->si_pszScriptText;
   BSTR pParseText = strScriptText.AllocSysString();
   // Does not blow up if I put return retval; here.
   // HRESULT ParseScriptText(
   //   LPCOLESTR pstrCode,          // address of scriptlet text
   //   LPCOLESTR pstrItemName,      // address of item name
   //   IUnknown *punkContext,       // address of debugging context
   //   LPCOLESTR pstrDelimiter,     // address of end-of-scriptlet delimiter
   //   DWORD dwSourceContextCookie, // application-defined value for debugging
   //   ULONG ulStartingLineNumber,  // starting line of the script
   //   DWORD dwFlags,               // scriptlet flags
   //   VARIANT *pvarResult,         // address of buffer for results
   //   EXCEPINFO *pexcepinfo        // address of buffer for error data
   //  );

   m_iActiveScriptParse->ParseScriptText(pParseText, L"MyObject", NULL,
      NULL, 0, 0, 0L, NULL, &ei);

   // Set the engine state. This line actually triggers the execution
   // of the script.
   // blows up even if I put return retval; here
   m_iActiveScript->SetScriptState(SCRIPTSTATE_CONNECTED);

   // Release engine...
   m_iActiveScriptParse->Release();
   m_iActiveScript->Release(); 

   return retval;
}

void ScriptThread(void *pvoid)
{
   OleInitClass local_OleInitClass;

   TypScriptInfo *psi = (TypScriptInfo *)pvoid;
   CString strSPO;
   int mixnum = psi->si_mixnum;
   BOOL bManualReset = TRUE, bInitialState = FALSE;
   Mix[mixnum].m_hEventODT = CreateEvent(NULL, bManualReset, bInitialState, NULL);
   Mix[mixnum].m_memory = psi->si_file_stat.st_size;
   Mix[mixnum].m_strStatus = MIX_STATUS_EXECUTING;
   Mix[mixnum].m_threadid = GetCurrentThreadId();
   Mix[mixnum].m_priority = psi->si_priority;
   SetThreadPriority(GetCurrentThread(), UnisysToNTPriority(psi->si_priority));
   if(Debug) FmtDebug("BOJ %s",Mix[mixnum].m_progname);
   CMRRAXScript script;
#if 0
   try {
      script.RunScript(psi);
   } catch (...) {
      if(Debug) FmtDebug("** Caught exception in mix %d program %s",mixnum, Mix[mixnum].m_progname);
      strSPO.Format("       -- %s=%-3.3d EXCEPTION",Mix[mixnum].m_progname,mixnum);
      SPO.LogSPOMsg("",strSPO);
   }
#else 
      script.RunScript(psi);
#endif

   // Blows up in OleUninitialize even if I put a return here.  
   // Does not blow up if return before RunScript().
   free(psi->si_pszScriptText);

   // Job has completed.  Output EOJ message.
   CTime timeEnd = CTime::GetCurrentTime();
   CString strNowTime = timeEnd.Format("%H:%M");
   CTimeSpan spanElapsed = timeEnd - Mix[mixnum].m_time_start;
   CString strElapsed;
   strElapsed.Format("%-2.2d:%-2.2d",spanElapsed.GetTotalMinutes(), spanElapsed.GetSeconds());

   // Calculate the charge time--the number of CPU seconds this thread used.
   // We'll use the sum of the kernel and user times.
   FILETIME CreationTime, ExitTime, KernelTime, UserTime; 
   GetThreadTimes(GetCurrentThread(), &CreationTime, &ExitTime, &KernelTime, &UserTime);
   ULARGE_INTEGER ulKernel, ulUser;
   memcpy(&ulKernel, &KernelTime, sizeof ulKernel);
   memcpy(&ulUser, &UserTime, sizeof ulUser);
   // Times are in 100 ns units; convert to seconds.
   time_t KernelSec = (time_t)(ulKernel.QuadPart / 10000000);
   time_t UserSec =   (time_t)(ulUser.QuadPart / 10000000);
   CTimeSpan spanCharge((time_t) (KernelSec+UserSec));
   CString strCharge;
   strCharge.Format("%-2.2d:%-2.2d",spanCharge.GetTotalMinutes(), spanCharge.GetSeconds());
      
   strSPO.Format("      EOJ %s=%-3.3d %s IN %s CHG %s",Mix[mixnum].m_progname, 
      mixnum,strNowTime, strElapsed, strCharge);
   SPO.LogSPOMsg("",strSPO);

   _int64 Kernel64 = ulKernel.QuadPart, User64 = ulUser.QuadPart;
   if(Debug) FmtDebug("EOJ %s; user=%8.5f sec  kernel=%8.5f sec", Mix[mixnum].m_progname,
      User64 / 10000000.0, Kernel64 / 10000000.0);
   Mix[mixnum].m_inuse = FALSE;

   delete psi;
}

static char *szScript = 
  "Const ForReading = 1, ForWriting = 2, ForAppending = 8\r\n"
  "\r\n"
   "Set fso = CreateObject(\"Scripting.FileSystemObject\")\r\n"
   "Set logfile = fso.OpenTextFile(\"\\BurSimAx.Log\", ForAppending, True)\r\n"
   "line = Time & \" \" & Date & \" BurSim script here. \" \r\n"
   "logfile.WriteLine(line)\r\n"
   "logfile.Close\r\n"
   "Dim MyMsg\r\n"
   "For j= 1 to 15\r\n"
   "  SPOMsg j & \"=\" & line\r\n"
   "  waittype = WaitForMsg(2,MyMsg)\r\n"
   "Next\r\n"
   ;

void DoRunScript(TypScriptInfo *psi)
{
   Mix[psi->si_mixnum].m_hthread = (HANDLE) _beginthread(ScriptThread,0,psi);
}

void InitJobStructs()
{
   CJob job;
   job.m_mixnum = 0;
   Mix.Add(job);
   job.m_mixnum = 1;
   Mix.Add(job);
}

/*--- function StartScriptByFile -----------------------------------
 */
BurSimRet StartScriptByFile(TypScriptInfo *psi)
{
   BurSimRet retval=BE_NO_ERROR;
   FILE *stfile;
   int mixnum = psi->si_mixnum;

   stfile = fopen(psi->si_script_filename,"rb");
   if(stfile) {
      if(!_stat(psi->si_script_filename,&psi->si_file_stat)) {
         int len = psi->si_file_stat.st_size;
         psi->si_pszScriptText = (char *) malloc(len+8);
         fread(psi->si_pszScriptText,1, len, stfile);
         fclose(stfile);
         psi->si_pszScriptText[len] = '\0';

         CTime timefile(psi->si_file_stat.st_mtime);
         CString strModifyDate = timefile.Format("%m%d%y");
         Mix[mixnum].m_time_start = CTime::GetCurrentTime();
         CString strNowTime = Mix[mixnum].m_time_start.Format("%H:%M");
         
         CString strSPO;
         strSPO.Format("      BOJ %s=%-3.3d %s %s",Mix[mixnum].m_progname, 
            mixnum,strModifyDate,strNowTime);
         SPO.LogSPOMsg("",strSPO);

         DoRunScript(psi);
      }
   } else {
      retval=BE_CANT_OPEN_FILE;
   }
   // The called thread is responsible for freeing the memory.
   //if(pszScript) free(pszScript);
   return retval;
}

/*--- function GetScriptFilename -----------------------------------
 */
BurSimRet GetScriptFilename(const char *szScript, CString &strFullScriptName, enum_languages &lang)
{
   BurSimRet retval=BE_NO_ERROR;
   BOOL bFound=FALSE;

   strFullScriptName.Empty();
   CString strFullPath = ConstructFilename(strScriptsPath, szScript);
   CString strFullName = strFullPath;
   struct _stat mystat;

   if(!_stat(strFullName,&mystat)) {
      bFound = TRUE;
   }
   if(!bFound) {
      strFullName += ".js";
      if(!_stat(strFullName,&mystat)) {
         bFound = TRUE;
         lang = LANG_JSCRIPT;
      }
   }
   if(!bFound) {
      strFullName = strFullPath + ".vbs";
      if(!_stat(strFullName,&mystat)) {
         lang = LANG_VBSCRIPT;
         bFound = TRUE;
      }
   }
   if(!bFound) {
      strFullName = strFullPath + ".py";
      if(!_stat(strFullName,&mystat)) {
         lang = LANG_PYTHON;
         bFound = TRUE;
      }
   }

   if(!bFound) {
      retval = BE_CANT_OPEN_FILE;
   } else {
      strFullScriptName = strFullName;
   }
   if(retval) {
      if(Debug) FmtDebug("Can't open script %s",strFullPath);
   } else if(Debug>1) FmtDebug("Opened script %s",strFullName);
   return retval;
}

/*--- function AllocateMixNumber -----------------------------------
 */
int AllocateMixNumber() {
   int mixnum;
   BOOL bFound=FALSE;

   EnterCriticalSection(&critMix);
   int mixsize = Mix.GetSize();
   for(mixnum=FIRST_ASSIGNABLE_MIX_NUM; mixnum < mixsize; mixnum++) {
      if(!Mix[mixnum].m_inuse) {
         bFound = TRUE;
         if(Mix[mixnum].m_hEventODT) {
            CloseHandle(Mix[mixnum].m_hEventODT);
            Mix[mixnum].m_hEventODT = NULL;
         }
         break;
      }
   }

   if(!bFound) {
      CJob *pJob = new CJob;
      Mix.Add(*pJob);
      mixnum = Mix.GetUpperBound();
   }
   Mix[mixnum].m_inuse = TRUE;
   Mix[mixnum].m_mixnum = mixnum;
   Mix[mixnum].m_progname.Empty();
   Mix[mixnum].m_scripttext.Empty();

   LeaveCriticalSection(&critMix);
   return mixnum;
}

/*--- function StartProgram -----------------------------------
 *
 *  Entry:  szProgram   is the program name, such as FDPSPO.
 *                      It is used as the basis for a script filename,
 *                      but does not include a path or extension.
 *          priority    is the Unisys priority.
 */
BurSimRet StartProgram(const char *szProgram, int priority)
{
   BurSimRet retval=BE_NO_ERROR;
   TypScriptInfo *psi = new TypScriptInfo;

   CString strFullScriptName;
   retval = GetScriptFilename(szProgram, psi->si_script_filename, psi->si_lang);
   if(!retval) {
      psi->si_mixnum = AllocateMixNumber();
      psi->si_priority = priority;
      Mix[psi->si_mixnum].m_progname = szProgram;
      retval = StartScriptByFile(psi);
   }
   return retval;
}