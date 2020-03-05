// AXScript.h 

// Include ActiveX Script definitions...
#include <activscp.h>
// Include definition for MyScriptObject...
#include "BurSimScriptObject.h" 

enum enum_languages {LANG_UNKNOWN, LANG_VBSCRIPT, LANG_JSCRIPT, LANG_PYTHON};

class TypScriptInfo {
public:
   char                *si_pszScriptText;
   CString              si_script_filename;
   int                  si_mixnum;
   int                  si_priority;
   struct _stat         si_file_stat;
   enum enum_languages  si_lang;

   TypScriptInfo() { si_pszScriptText=0; si_mixnum=0; si_lang=LANG_UNKNOWN; }
};

class CMRRAXScript {
public:
   CBurSimScriptObject m_myScriptObject;
   IActiveScript *m_iActiveScript;
   IActiveScriptParse *m_iActiveScriptParse; 
   //int      m_script_mixnum;

   BurSimRet RunScript(TypScriptInfo *psi);
};

void DoRunScript(TypScriptInfo *psi);
BurSimRet GetScriptFilename(const char *szScript, CString &strFullScriptName, enum_languages &lang);
BurSimRet StartScriptByFile(TypScriptInfo *psi);
BurSimRet StartProgram(const char *szProgram, int priority);
void InitJobStructs();
