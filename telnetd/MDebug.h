#ifndef MDEBUG_H
#define MDEBUG_H

#include "Conn.h"

void Dout(char *msg,char *buf, int nbytes);
void DoutAsc(char *msg,char *buf, int nbytes);
void PutH(char *str);
void LogMsg(const char *msg);
void MDebug(const char *msg);
void FmtDebug(const char *msg,...);
void CloseLogFile();
void
ReportElapsedTime(const char *msg, DWORD starttime);
void
ShortTime(time_t mytime, char *timestr);
void
LogTransmission(TypConn *pConn,char *buf,int nbytes,char *label);
void 
FormatSession(TypConn *pConn, char *mbuf);
int
SendMF(TypConn *pConn,const char *str);
void GetDebugFlag(CString csAppName);

#endif MDEBUG_H
