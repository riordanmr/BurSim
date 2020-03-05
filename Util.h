// Util.h

//=== Function prototypes ===========================================

void MakeVersionStringForFile(char *filename, char *pszVersionString);
void MakeVersionString(char *pszVersionString);
BOOL RunHelp(HWND hwndMain,UINT uCommand, DWORD contextID);

int MyRand(int mymax);
CString BlankPad(const char *pszstr, int len);
void FmtDebug(const char *fmt,...);

CString ConstructFilename(CString cspath, CString csfile);
BOOL matchbegi(const char *szpattern, const char *szstr_to_search);
void TrimTrailingSpaces(char *buf);

