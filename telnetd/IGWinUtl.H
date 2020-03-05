#ifndef IGWINUTL_INCLUDED
#define IGWINUTL_INCLUDED

#define BAD_INTEGER 999999999

void *
UGMapMemory(char *lpMappingName, DWORD dwNumberOfBytesToMap);
int CreateNullSecurity(SECURITY_ATTRIBUTES *psa);
BOOL WRUFormatSystemErrorMessage(int errcode, char * lpBuffer, int maxbytes);
int GetRegistryMachineDword(const char *keyname, const char *valname, DWORD &value);
int SetRegistryMachineDword(char *keyname, char *valname, DWORD val);
int GetRegistryMachineString(char *keyname, char *valname, char *val, int maxbytes);
int SetRegistryMachineString(char *keyname, char *valname, CString val);
HANDLE WRUNTOpenFileRead(LPCTSTR lpFileName);
int FSend(SOCKET sock,char *fmt...);
DWORD timeGetTimeHighRes(void);
int AsciiToBinary(const char *szstr);
void MakeVersionString(char *pszVersionString);
BOOL IPAddressMatch(unsigned long *pip_to_try, unsigned long *pip_match_against);
int RenameToOld(char *szFilename);
int StripFileExtension(char *szFileName);
int strncasecmp(const char *str1,const char *str2,int len);
char *strcasestr(const char *s, const char *find);
int ReadFileIntoString(CString csFilename, CString &csContents);
CString ExtractDirectory(CString csname);
CString ConstructFilename(CString cspath, CString csfile);
int strncasecmp(const char *str1,const char *str2,int len);
char *strcasestr(const char *s, const char *find);
void StripLeadingAndTrailingSpaces(CString &str);
BOOL CrackStringArg(CString strIn, CString &strOut);
int CheckNTGroupMembership(const char *pszServerOrig, const char *pszDomainOrig, 
     const char *pszUsernameOrig, const char *pszPasswordOrig, const char *pszGroup, 
     char *pszErrmsg, int MsgLevel);

class CSubstStrings {
public:
   CStringArray csPatternsAry;
   CStringArray csSubstituteWithAry;
public:
   void AddMatchString(CString csLookFor, CString csSubstituteWith);
   void SubstituteAll(CString &csInOut);
};

#define ERR_NTAUTH_NO_SUCH_USER     12741
#define ERR_NTAUTH_CANT_LOGON       12742
#define ERR_NTAUTH_NOT_MEMBER       12743

#endif