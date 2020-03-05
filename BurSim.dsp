# Microsoft Developer Studio Project File - Name="BurSim" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=BurSim - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "BurSim.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "BurSim.mak" CFG="BurSim - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BurSim - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "BurSim - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "BurSim - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "\mrr\BurSim" /I "\mrr\BurSim\telnetd" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "UNI_VT" /D "UNI_TD" /D "INITGUID" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 version.lib wsock32.lib netapi32.lib winmm.lib C:\Microsoft\MicrosoftPlatformSDK\Lib\Uuid.Lib /nologo /subsystem:windows /debug /machine:I386

!ELSEIF  "$(CFG)" == "BurSim - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "\mrr\BurSim" /I "\mrr\BurSim\telnetd" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "UNI_VT" /D "UNI_TD" /D "INITGUID" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 version.lib wsock32.lib netapi32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386

!ENDIF 

# Begin Target

# Name "BurSim - Win32 Release"
# Name "BurSim - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\telnetd\AscEbc.cpp
# End Source File
# Begin Source File

SOURCE=.\AXScript.cpp
# End Source File
# Begin Source File

SOURCE=.\BurSim.cpp
# End Source File
# Begin Source File

SOURCE=.\BurSim.rc
# End Source File
# Begin Source File

SOURCE=.\BurSimScriptObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Command.cpp
# End Source File
# Begin Source File

SOURCE=.\Displays.cpp

!IF  "$(CFG)" == "BurSim - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "BurSim - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DlgConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\Drawwnd.cpp
# End Source File
# Begin Source File

SOURCE=.\telnetd\FkStubs.cpp
# End Source File
# Begin Source File

SOURCE=.\telnetd\FkUG.cpp
# End Source File
# Begin Source File

SOURCE=.\telnetd\FkUGFunc.cpp
# End Source File
# Begin Source File

SOURCE=.\telnetd\HEXBIN.CPP
# End Source File
# Begin Source File

SOURCE=.\Job.cpp
# End Source File
# Begin Source File

SOURCE=.\telnetd\MDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\MYGETOPT.CPP

!IF  "$(CFG)" == "BurSim - Win32 Release"

# ADD CPP /Zi /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "BurSim - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Screen.cpp
# End Source File
# Begin Source File

SOURCE=.\SPO.cpp
# End Source File
# Begin Source File

SOURCE=.\telnetd\Station.cpp
# End Source File
# Begin Source File

SOURCE=.\Stdafx.cpp
# ADD CPP /Yc"Stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\STOQ.cpp
# End Source File
# Begin Source File

SOURCE=.\telnetd\ug2clt.cpp
# End Source File
# Begin Source File

SOURCE=.\telnetd\UGAlloc.CPP
# End Source File
# Begin Source File

SOURCE=.\telnetd\UGEMHOST.CPP
# End Source File
# Begin Source File

SOURCE=.\telnetd\uginter.cpp
# End Source File
# Begin Source File

SOURCE=.\telnetd\UGStub.cpp
# End Source File
# Begin Source File

SOURCE=.\telnetd\UGTELSRV.CPP
# End Source File
# Begin Source File

SOURCE=.\Util.cpp
# End Source File
# Begin Source File

SOURCE=.\WndMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;inl;fi;fd"
# Begin Source File

SOURCE=.\AXScript.h
# End Source File
# Begin Source File

SOURCE=.\BurSimApp.h
# End Source File
# Begin Source File

SOURCE=.\BurSimdlg.h
# End Source File
# Begin Source File

SOURCE=.\BurSimScriptObject.h
# End Source File
# Begin Source File

SOURCE=.\Command.h
# End Source File
# Begin Source File

SOURCE=.\telnetd\Conn.h
# End Source File
# Begin Source File

SOURCE=.\telnetd\Default.h
# End Source File
# Begin Source File

SOURCE=.\DlgConfig.h
# End Source File
# Begin Source File

SOURCE=.\drawwnd.h
# End Source File
# Begin Source File

SOURCE=.\telnetd\FkUG.h
# End Source File
# Begin Source File

SOURCE=.\GETOPTPR.H
# End Source File
# Begin Source File

SOURCE=.\telnetd\HEXBINPR.H
# End Source File
# Begin Source File

SOURCE=.\telnetd\HTMLOUT.H
# End Source File
# Begin Source File

SOURCE=.\telnetd\IGWinUtl.H
# End Source File
# Begin Source File

SOURCE=.\Job.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Screen.h
# End Source File
# Begin Source File

SOURCE=.\telnetd\scrutils.h
# End Source File
# Begin Source File

SOURCE=.\SPO.h
# End Source File
# Begin Source File

SOURCE=.\telnetd\Station.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\STOQ.h
# End Source File
# Begin Source File

SOURCE=.\Util.h
# End Source File
# Begin Source File

SOURCE=.\telnetd\winutil.h
# End Source File
# Begin Source File

SOURCE=.\WndMain.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\BurSim.ico
# End Source File
# Begin Source File

SOURCE=.\res\BurSim.rc2
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\icon_con.ico
# End Source File
# Begin Source File

SOURCE=.\res\saver.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\Help\BurSim.hh
# End Source File
# End Target
# End Project
