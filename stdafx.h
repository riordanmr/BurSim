// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#define VC_EXTRALEAN

#include <afxwin.h>		 // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxtempl.h>
#include <winsock.h>

#include <process.h>    /* _beginthread, _endthread */
#include <sys/types.h>
#include <sys/stat.h>

#include "BurSimApp.h"
#include "BurSimRet.h"
#include "STOQ.h"
#include "util.h"
#include "DrawWnd.h"
#include "spo.h"
#include "Command.h"
#include "Screen.h"
#include "Job.h"

// FkUG Stuff:
#include "ascii.h"
#include "default.h"
#include "message.h"
#include "ops.h"
#include "tcp.h"
#include "tcpuser.h"
#include "ss_def.h"
#include "ss_type.h"
#include "tel_def.h"
#include "tscreen.h"