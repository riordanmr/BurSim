#ifndef HTMLOUT_H
#define HTMLOUT_H

#define HTML_BUFSIZE 8192

class TypWSession;

class HTMLOut {
public:
   int         HTMLDebug;     // Copy of Debug flag from pWRU
   int         n_calls;
   BOOL        m_bKeepAlive;  // Whether browser asked us to keep conn alive.
   BOOL        m_bSentHeader; // Whether HTTP header has been sent yet.
   BOOL        m_bSentLength; // Whether we sent the Content-Length header.
   BOOL        m_bUseKeepAlive; // Whether we should send KeepAlive header.

   int         m_nHTTPCode;
   CString     m_csHTTPCode;
	TypWSession	*session;
#ifndef AUTHTOOL
	char			htmlbuf[HTML_BUFSIZE];
	int			hidx;
	LPEXTENSION_CONTROL_BLOCK  lpECB;
#else 
   CString     m_csHTML;
   void       *m_pIDE;  // pointer to CDbgFrame debugger window
   CString     m_csBrowserFilename;
   unsigned int m_nBrowserOpenFails;
   BOOL        m_update_browser;
#endif

public:
	HTMLOut(void);
   void init(void);
	void wr(const char *str);
	void HTMLOut::flush(void);
	void SetServerInfo(LPEXTENSION_CONTROL_BLOCK  newlpEcb);
   int SendHTTPHeader(BOOL bSendLength);
   void SetHTTPCode(int code, char *pszresp_code);
#ifdef AUTHTOOL
   void InitBrowser(CString csName);
   void UpdateSeeEdit(BOOL bForceDisplay);
   void UpdateBrowser(void);
#endif
};
#endif
