#if !defined(AFX_BURSIMSCRIPTOBJECT_H__DD38DD26_58A3_11D3_A152_0000F87ADAFA__INCLUDED_)
#define AFX_BURSIMSCRIPTOBJECT_H__DD38DD26_58A3_11D3_A152_0000F87ADAFA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BurSimScriptObject.h : header file
//

#define WAITRET_TIMEOUT    1
#define WAITRET_ODT        2
#define WAITRET_STOQ       3
 

/////////////////////////////////////////////////////////////////////////////
// CBurSimScriptObject command target

class CBurSimScriptObject : public CCmdTarget
{
	DECLARE_DYNCREATE(CBurSimScriptObject)

	CBurSimScriptObject();           // protected constructor used by dynamic creation

// Attributes
public:
   int m_so_mixnum;

// Operations
public:

	virtual ~CBurSimScriptObject();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBurSimScriptObject)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementation
   CrackFormID(LPCTSTR szFormID, int &nFormFile, int &nForm);
protected:

	// Generated message map functions
	//{{AFX_MSG(CBurSimScriptObject)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CBurSimScriptObject)
	afx_msg void SPOMsg(LPCTSTR Text);
	afx_msg long Wait(long Secs, LPCTSTR STOQsToWaitFor, VARIANT FAR* STOQGotten);
	afx_msg void Accept(VARIANT FAR* Text);
	afx_msg long GetSTOQMsg(LPCTSTR STOQName, VARIANT FAR* Header, VARIANT FAR* Buf);
	afx_msg long SendMCSBuf(LPCTSTR Header, LPCTSTR Buf);
	afx_msg void ZIP(LPCTSTR Instruction);
	afx_msg void SendSTOQMsg(LPCTSTR STOQName, LPCTSTR Buf);
	afx_msg BSTR OpenFormFile(LPCTSTR Filename);
	afx_msg void FillField(LPCTSTR FormID, LPCTSTR FieldName, VARIANT FAR* ScrnBuf, LPCTSTR Chars);
	afx_msg BSTR OpenForm(LPCTSTR FormFileID, LPCTSTR FormName);
	afx_msg BSTR InitFormBuf(LPCTSTR FormID);
	afx_msg BSTR GetInputField(LPCTSTR FormID, LPCTSTR FieldName, LPCTSTR Buf);
	afx_msg void SuppressField(LPCTSTR FormID, LPCTSTR FieldName, VARIANT FAR* Buf);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BURSIMSCRIPTOBJECT_H__DD38DD26_58A3_11D3_A152_0000F87ADAFA__INCLUDED_)
