BOOL 
match(TypConn *pConn, int row, int col, const char *str);
int GetChars(TypConn *pConn, int col, int row, int nchars, 
             BOOL bPrintOnly, char *errmsg, char *buf);
void
PlaceStr(TypConn *pConn,int irow, int icol, const char *str);
int strcpynz(char *targ, const char *source);
int
PlaceAtCursor(TypConn *pConn,const char *str) ;
CString GetToken(CString cs, int nfield, const char delim[]=" \t");
CString
GetFieldByOrd(TypWRUParm *pWRU,TypConn *pConn,int fieldnum);
int
LoadScreen(TypConn *pConn);
void
RecordScreenImage(TypConn *pConn);
void
RecordScreenText(TypConn *pConn);
CString
TokenOnLine(TypConn *pConn,int row, int nfield);
int
CountTokens(CString cs, const char delim[]=" \t");
int
CountTokensOnLine(TypConn *pConn, int row, const char delim[]=" \t");
CString
GetLineFromScreen(TypConn *pConn,int row);
void
EraseToEndOfField(TypConn *pConn);
void
Tab(TypWRUParm *pWRU,TypConn *pConn);
int GoToField(TypWRUParm *pWRU,TypConn *pConn, int fieldnum);
BOOL
LookForStringOnScreen(TypWRUParm *pWRU,TypConn *pConn,CString searchstr, 
                      int startrow, int endrow,int &foundcol, int &foundrow);
BOOL
LookForStringAtCol(TypWRUParm *pWRU,TypConn *pConn,CString searchstr, 
                  int col,int startrow, int endrow,int &foundrow);
BOOL GetRelativeStringAtCol(TypWRUParm *pWRU,TypConn *pConn,const char *searchstr,
   int col, int relcol,int relrow,int len,char *foundstr);
int GetCursor(TypConn *pConn, int &col, int &row);
void RemoveSpecialChars(LPSTR lpszString, int nchars);
void RemoveSpecialChars(LPSTR lpszString);
void RemoveSpecialChars(CString csString);

int GetFieldChars(TypWRUParm *pWRU, TypConn *pConn, char *pszField,
      char *pszScreen, char *pszDatabase, char *errmsg, char *buf);

