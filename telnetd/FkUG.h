#define LINELEN 200
#define BYTESPERLINE 16

// Function prototypes
void ListenThreadProc(void);
BOOL SetupListen(void);
DWORD ProcessThreadProc(LPDWORD lpdwParam);
BOOL AcceptConnection(SOCKET *skOutput);
void DoSession(SOCKET sock);
void Blocking(SOCKET s);

int
begmatch(char *pattern, char *target);
void
SendBuf(SOCKET sock, unsigned char *buf, int len);
void
SendszASC(SOCKET sock,const char *str);
