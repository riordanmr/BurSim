// BurSimScriptObject.cpp : implementation file
//

#include "stdafx.h"
#include <activscp.h>
#include <afxdisp.h>
#include <comdef.h>
#include "DrawWnd.h"
#include "BurSimScriptObject.h"
#include "Conn.h"
#include "fkug.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=== Global Variables ===================================================
extern int Debug;
extern CSPO SPO;
extern CArray<CJob, CJob &> Mix;
extern CRITICAL_SECTION critMix;
extern TypConn *Connection;


/*--- function ReturnVariant -----------------------------------
 *
 *  Set the value of a variant.  Note: there does not appear to be
 *  any direct way to set the value of a variant to anything other
 *  than a string.  If you want the current type to be something 
 *  else, it appears that you must set the string value and then
 *  change the current type.
 *
 *  Entry:  szstring    is a zero-terminated ASCII string, even though
 *                      VARIANTs are internally stored in UNICODE.
 *
 *  Exit:   *myvar      is a variant which now has the value of szstring.
 */
void ReturnVariant(const char *szstring, VARIANT *myvar)
{
   _variant_t myvar_t(szstring);
   *myvar = myvar_t.Detach();
}


/////////////////////////////////////////////////////////////////////////////
// CBurSimScriptObject

IMPLEMENT_DYNCREATE(CBurSimScriptObject, CCmdTarget)

CBurSimScriptObject::CBurSimScriptObject()
{
   if(Debug>1) FmtDebug("CBurSimScriptObject::CBurSimScriptObject");
	EnableAutomation();
}

CBurSimScriptObject::~CBurSimScriptObject()
{
}


void CBurSimScriptObject::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CCmdTarget::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CBurSimScriptObject, CCmdTarget)
	//{{AFX_MSG_MAP(CBurSimScriptObject)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CBurSimScriptObject, CCmdTarget)
	//{{AFX_DISPATCH_MAP(CBurSimScriptObject)
	DISP_FUNCTION(CBurSimScriptObject, "SPOMsg", SPOMsg, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CBurSimScriptObject, "Wait", Wait, VT_I4, VTS_I4 VTS_BSTR VTS_PVARIANT)
	DISP_FUNCTION(CBurSimScriptObject, "Accept", Accept, VT_EMPTY, VTS_PVARIANT)
	DISP_FUNCTION(CBurSimScriptObject, "GetSTOQMsg", GetSTOQMsg, VT_I4, VTS_BSTR VTS_PVARIANT VTS_PVARIANT)
	DISP_FUNCTION(CBurSimScriptObject, "SendMCSBuf", SendMCSBuf, VT_I4, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CBurSimScriptObject, "ZIP", ZIP, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CBurSimScriptObject, "SendSTOQMsg", SendSTOQMsg, VT_EMPTY, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CBurSimScriptObject, "OpenFormFile", OpenFormFile, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION(CBurSimScriptObject, "FillField", FillField, VT_EMPTY, VTS_BSTR VTS_BSTR VTS_PVARIANT VTS_BSTR)
	DISP_FUNCTION(CBurSimScriptObject, "OpenForm", OpenForm, VT_BSTR, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CBurSimScriptObject, "InitFormBuf", InitFormBuf, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION(CBurSimScriptObject, "GetInputField", GetInputField, VT_BSTR, VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CBurSimScriptObject, "SuppressField", SuppressField, VT_EMPTY, VTS_BSTR VTS_BSTR VTS_PVARIANT)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IBurSimScriptObject to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {DD38DD25-58A3-11D3-A152-0000F87ADAFA}
static const IID IID_IBurSimScriptObject =
{ 0xdd38dd25, 0x58a3, 0x11d3, { 0xa1, 0x52, 0x0, 0x0, 0xf8, 0x7a, 0xda, 0xfa } };

BEGIN_INTERFACE_MAP(CBurSimScriptObject, CCmdTarget)
	INTERFACE_PART(CBurSimScriptObject, IID_IBurSimScriptObject, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBurSimScriptObject message handlers

void CBurSimScriptObject::SPOMsg(LPCTSTR Text) 
{
   CJob *pJob = &Mix[m_so_mixnum];
   pJob->LogSPOMsgFromProgram(Text);
}

long CBurSimScriptObject::Wait(long Secs, LPCTSTR STOQsToWaitFor, VARIANT FAR* STOQGotten) 
{
   int retval=0, j;
   CStringArray arySTOQs;
#define MAX_STOQS_TO_WAIT_FOR    64
   HANDLE aryHandles[MAX_STOQS_TO_WAIT_FOR];
   BOOL bCreated[MAX_STOQS_TO_WAIT_FOR];
   BOOL bManualReset = TRUE, bInitialState = FALSE;
   CString strProgram = Mix[m_so_mixnum].m_progname;
   CString strSTOQGotten;

   Mix[m_so_mixnum].m_strStatus = MIX_STATUS_COMPLEX_WAIT;
   memset(bCreated, FALSE, sizeof bCreated);
   aryHandles[0] = Mix[m_so_mixnum].m_hEventODT;
   int nstoqs = CrackSTOQList(STOQsToWaitFor, arySTOQs);
   if(nstoqs > MAX_STOQS_TO_WAIT_FOR) nstoqs = MAX_STOQS_TO_WAIT_FOR;
   for(j=0; j<nstoqs; j++) {
      CSTOQ *pSTOQ = CSTOQ::GetSTOQPtr(arySTOQs[j]);
      aryHandles[j+1] = pSTOQ->m_hEventSTOQ;
   }
   DWORD waitres = WaitForMultipleObjects(nstoqs+1, aryHandles, FALSE, 1000*Secs);
   if(WAIT_TIMEOUT == waitres) {
      retval = WAITRET_TIMEOUT;
   } else if (WAIT_OBJECT_0 == waitres) {
      retval = WAITRET_ODT;
   } else if (WAIT_FAILED == waitres) {
      FmtDebug("Wait: wait failed");
   } else {
      retval = WAITRET_STOQ;
      strSTOQGotten = arySTOQs[waitres - WAIT_OBJECT_0 - 1];
   }
   Mix[m_so_mixnum].m_strStatus = MIX_STATUS_EXECUTING;

   // Set the returned name of STOQ gotten.
   ReturnVariant(strSTOQGotten, STOQGotten);
	return retval;
}

void CBurSimScriptObject::Accept(VARIANT FAR* Text) 
{
   Mix[m_so_mixnum].m_strStatus = MIX_STATUS_WAIT_ACCEPT;
   WaitForSingleObject(Mix[m_so_mixnum].m_hEventODT,INFINITE);
   ResetEvent(Mix[m_so_mixnum].m_hEventODT);
   Mix[m_so_mixnum].m_strStatus = MIX_STATUS_EXECUTING;

   ReturnVariant(Mix[m_so_mixnum].m_odt_input, Text);
}

long CBurSimScriptObject::GetSTOQMsg(LPCTSTR STOQName, VARIANT FAR *Header, VARIANT FAR* Buf) 
{
   long retval=0;
   int nRemaining;
   CString strHeader, strData, strBuf;

   Mix[m_so_mixnum].m_strStatus = "WTG STOQ ENTRY ";
   Mix[m_so_mixnum].m_strStatus += "STOQName";

   CSTOQ *pSTOQ = CSTOQ::GetSTOQPtr(STOQName);
   WaitForSingleObject(pSTOQ->m_hEventSTOQ,INFINITE);

   pSTOQ->GetHead(strBuf, nRemaining);
   if(!nRemaining) ResetEvent(pSTOQ->m_hEventSTOQ);
   if(!strncmp(MSG_HEADER_PREFIX,strBuf,strlen(MSG_HEADER_PREFIX))) {
      strHeader = strBuf.Left(MSG_HEADER_LEN);
      strData = strBuf.Mid(MSG_HEADER_LEN);
   } else {
      strData = strBuf;
   }
   Mix[m_so_mixnum].m_strStatus = MIX_STATUS_EXECUTING;
   ReturnVariant(strHeader, Header);
   ReturnVariant(strData, Buf);
	return retval;
}

long CBurSimScriptObject::SendMCSBuf(LPCTSTR Header, LPCTSTR Buf) 
{
   long retval=0;
   CString strTermID = CString(Header+MSG_HEADER_CONNID_OFFSET,MSG_HEADER_CONNID_LEN);
   int conn_index = atoi(strTermID);
   TypConn *pConn = &Connection[conn_index];
   SOCKET sock = pConn->sock;

   if(Debug>1) FmtDebug("SendMCSBuf stn %d sock %d sending %d bytes",
      conn_index, sock, strlen(Buf));

   SendszASC(sock, Buf);

	return retval;
}

void CBurSimScriptObject::ZIP(LPCTSTR Instruction) 
{
   Mix[m_so_mixnum].ExecuteCommandFromProgram(Instruction);
}

void CBurSimScriptObject::SendSTOQMsg(LPCTSTR STOQName, LPCTSTR Buf) 
{
   CSTOQ *pSTOQ = CSTOQ::GetSTOQPtr(STOQName);
   pSTOQ->AddTail(Buf);
}

// Form stuff

/*--- function CrackFormID -----------------------------------
 *
 *  Given a form ID, returns the index of the form file, 
 *  and of the form within that file.
 *
 *  Entry:  szFormID    is the Form ID of form FormFileIndex|FormIndexWithinFile
 *
 *  Exit:   Returns 0 if success, else non-zero if bad formid.
 */
int CBurSimScriptObject::CrackFormID(LPCTSTR szFormID, int &nFormFile, int &nForm)
{
   nFormFile = -1, nForm = -1;
   sscanf(szFormID,"%d|%d",&nFormFile, &nForm);
   if(nFormFile<0 || nForm<0 || nFormFile>Mix[m_so_mixnum].m_aryFormFiles.GetSize()
      || nForm>Mix[m_so_mixnum].m_aryFormFiles[nFormFile].m_aryForms.GetSize()) {
      return 1;
   }
   return 0;
}

BSTR CBurSimScriptObject::OpenFormFile(LPCTSTR Filename) 
{
	CString strResult;
   CBSFormFile FormFile;
   int retval = FormFile.ReadScreenFile(Filename);
   if(!retval) {
      Mix[m_so_mixnum].m_aryFormFiles.Add(FormFile);
      int nFormFile = Mix[m_so_mixnum].m_aryFormFiles.GetUpperBound();
      strResult.Format("%d",nFormFile);
      if(Debug>2) Mix[m_so_mixnum].m_aryFormFiles[nFormFile].Dump();
   }
   
	return strResult.AllocSysString();
}

BSTR CBurSimScriptObject::OpenForm(LPCTSTR FormFileID, LPCTSTR FormName) 
{
	CString strFormID;
   int nFormFile = atoi(FormFileID);
   if(nFormFile>=0 && nFormFile < Mix[m_so_mixnum].m_aryFormFiles.GetSize()) {
      CBSFormFile &FormFile = Mix[m_so_mixnum].m_aryFormFiles[nFormFile];
      int iform;
      BOOL bFound=FALSE;
      for(iform=0; !bFound && iform<FormFile.m_aryForms.GetSize(); iform++) {
         if(0==FormFile.m_aryForms[iform].m_strFormName.CompareNoCase(FormName)) {
            bFound = TRUE;
            strFormID.Format("%d|%d",nFormFile,iform);
         }
      }
   }

	return strFormID.AllocSysString();
}

BSTR CBurSimScriptObject::InitFormBuf(LPCTSTR FormID) 
{
	CString strBuffer;
   int nFormFile,nForm;
   if(!CrackFormID(FormID,nFormFile,nForm)) {
      CBSForm &Form=Mix[m_so_mixnum].m_aryFormFiles[nFormFile].m_aryForms[nForm];
      strBuffer = T27HEADER;
      strBuffer += Form.m_strFormContents + T27TRAILER;
   }

	return strBuffer.AllocSysString();
}

/*--- function CBurSimScriptObject::FillField -----------------------------------
 */
void CBurSimScriptObject::FillField(LPCTSTR FormID, LPCTSTR FieldName, 
                                    VARIANT FAR* ScrnBuf, LPCTSTR Chars) 
{
   int nFormFile, nForm;
   if(CrackFormID(FormID,nFormFile,nForm)) return;
   int ifld;
   BOOL bFound=FALSE;
   CBSForm &Form=Mix[m_so_mixnum].m_aryFormFiles[nFormFile].m_aryForms[nForm];
   // Scan the array of fields in this database, looking for this field.
   for(ifld=0; !bFound && ifld<Form.m_aryFields.GetSize(); ifld++) {
      CBSField &Field = Form.m_aryFields[ifld];
      if(0==Field.m_strFieldName.CompareNoCase(FieldName)) {
         // This is the field.
         bFound = TRUE;
         int offset = Field.m_field_start + strlen(T27HEADER);
         CString strFieldContents = Chars;
         // Make strFieldContents the exact size of the field.
         if(strFieldContents.GetLength() < Field.m_field_len) {
            strFieldContents += CString(' ',Field.m_field_len-strFieldContents.GetLength());
         } else if(strFieldContents.GetLength() > Field.m_field_len) {
            strFieldContents = strFieldContents.Left(Field.m_field_len);
         }
         // Go through some contortions to safely access the buffer.
         // There must be a better way to do this, but I'm not sure how
         // to better do it and still be conformant with ASCII vs. UNICODE.
         _variant_t myvar_t(ScrnBuf);
         CString strBuf = (char *)(_bstr_t)myvar_t;
         int len = strBuf.GetLength();
         LPTSTR pbuf = strBuf.GetBuffer(len);
         // Replace the appropriate area of the buffer with the new contents.
         memcpy(pbuf+offset,strFieldContents,Field.m_field_len);
         strBuf.ReleaseBuffer(len);

         _variant_t returned_var_t(strBuf);
         *ScrnBuf = returned_var_t.Detach();
      }
   }
}

/*--- function CBurSimScriptObject::GetInputField -----------------------------------
 */
BSTR CBurSimScriptObject::GetInputField(LPCTSTR FormID, LPCTSTR FieldName, LPCTSTR Buf) 
{
	CString strResult;
   int nFormFile, nForm;
   if(!CrackFormID(FormID,nFormFile,nForm)) {
      int ifld;
      BOOL bFound=FALSE;
      CBSForm &Form=Mix[m_so_mixnum].m_aryFormFiles[nFormFile].m_aryForms[nForm];
      // Scan the array of fields in this database, looking for this field.
      for(ifld=0; !bFound && ifld<Form.m_aryFields.GetSize(); ifld++) {
         CBSField &Field = Form.m_aryFields[ifld];
         if(0==Field.m_strFieldName.CompareNoCase(FieldName)) {
            // This is the field.
            bFound = TRUE;
            int nMinSize = Field.m_field_input_offset + Field.m_field_len;
            int inp_len = strlen(Buf);
            if(inp_len >= nMinSize) {
               strResult = CString(Buf+Field.m_field_input_offset, Field.m_field_len);
            } else {
               strResult = (CString(Buf) + CString(' ',nMinSize)).Mid(Field.m_field_input_offset, Field.m_field_len);
            }
         }
      }
   }
	return strResult.AllocSysString();
}

void CBurSimScriptObject::SuppressField(LPCTSTR FormID, LPCTSTR FieldName, VARIANT FAR* Buf) 
{
   int nFormFile, nForm;
   if(!CrackFormID(FormID,nFormFile,nForm)) {
      int ifld;
      BOOL bFound=FALSE;
      CBSForm &Form=Mix[m_so_mixnum].m_aryFormFiles[nFormFile].m_aryForms[nForm];
      // Scan the array of fields in this database, looking for this field.
      for(ifld=0; !bFound && ifld<Form.m_aryFields.GetSize(); ifld++) {
         CBSField &Field = Form.m_aryFields[ifld];
         if(0==Field.m_strFieldName.CompareNoCase(FieldName)) {
            // This is the field.
            bFound = TRUE;


            // Go through some contortions to safely access the buffer.
            // There must be a better way to do this, but I'm not sure how
            // to better do it and still be conformant with ASCII vs. UNICODE.
            _variant_t myvar_t(Buf);
            CString strBuf = (char *)(_bstr_t)myvar_t;
            int len = strBuf.GetLength();
            LPTSTR pbuf = strBuf.GetBuffer(len);

            // Replace the special characters just before and after the field
            // (FS, US, or RS) with spaces).
            int offset = Field.m_field_start + strlen(T27HEADER);
            pbuf[offset-1] = ' ';
            pbuf[offset+Field.m_field_len] = ' ';

            strBuf.ReleaseBuffer(len);

            _variant_t returned_var_t(strBuf);
            *Buf = returned_var_t.Detach();
         }
      }
   }

}
