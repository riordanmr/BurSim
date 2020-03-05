void *
UGMapMemory(char *lpMappingName, DWORD dwNumberOfBytesToMap);
int CreateNullSecurity(SECURITY_ATTRIBUTES *psa);
BOOL WRUFormatSystemErrorMessage(int errcode, char * lpBuffer, int maxbytes);
int GetRegistryMachineDword(const char *keyname, const char *valname, DWORD &value);
int SetRegistryMachineDword(char *keyname, char *valname, DWORD val);
int GetRegistryMachineString(char *keyname, char *valname, char *val, int maxbytes);
int SetRegistryMachineString(char *keyname, char *valname, CString val);
HANDLE WRUNTOpenFileRead(LPCTSTR lpFileName);
int
FSend(SOCKET sock,char *fmt...);
DWORD timeGetTimeHighRes(void);
BOOL SNITryEnterCriticalSection(LPCRITICAL_SECTION lpcrit);
void MakeVersionString(char *pszVersionString);
