# Microsoft Developer Studio Project File - Name="WAIN" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=WAIN - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Wain.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Wain.mak" CFG="WAIN - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WAIN - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "WAIN - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WAIN - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\..\Release"
# PROP Intermediate_Dir ".\..\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu".\..\src\stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu".\..\src\stdafx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x406 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x406 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 MPR.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "WAIN - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\..\pcobj"
# PROP BASE Intermediate_Dir ".\..\pcobj"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\..\Debug"
# PROP Intermediate_Dir ".\..\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu".\..\src\stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /O2 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /Yu".\..\src\stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x406 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x406 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 MPR.LIB /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "WAIN - Win32 Release"
# Name "WAIN - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\..\src\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\classview.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\DockBar.cpp
# End Source File
# Begin Source File

SOURCE=.\..\Src\DocProp.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\FileSelD.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\FileSelD.h
# End Source File
# Begin Source File

SOURCE=.\..\SRC\GlobSet.cpp
# End Source File
# Begin Source File

SOURCE=.\..\SRC\KeySetup.cpp
# End Source File
# Begin Source File

SOURCE=..\SRC\Line.cpp
# End Source File
# Begin Source File

SOURCE=.\..\SRC\Macro.cpp
# End Source File
# Begin Source File

SOURCE=.\..\src\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\NavigatorDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\NavigatorFile.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\NavigatorList.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nrex.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nrex.h
# End Source File
# Begin Source File

SOURCE=..\Src\PageDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Print.cpp
# End Source File
# Begin Source File

SOURCE=.\..\SRC\Project.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\regexp.cpp
# End Source File
# Begin Source File

SOURCE=.\..\SRC\Search.cpp
# End Source File
# Begin Source File

SOURCE=..\src\SimpleDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\..\src\StdAfx.cpp
# ADD CPP /Yc".\..\src\stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\Src\TabView.cpp
# End Source File
# Begin Source File

SOURCE=.\..\SRC\TagList.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\Tags.cpp
# End Source File
# Begin Source File

SOURCE=..\SRC\Tools.cpp
# End Source File
# Begin Source File

SOURCE=.\..\SRC\Undo.cpp
# End Source File
# Begin Source File

SOURCE=..\SRC\ViewList.cpp
# End Source File
# Begin Source File

SOURCE=.\..\src\Wain.cpp
# End Source File
# Begin Source File

SOURCE=.\..\src\Wain.rc
# End Source File
# Begin Source File

SOURCE=.\..\src\WainDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\..\SRC\WainEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\WainFTP.cpp
# End Source File
# Begin Source File

SOURCE=.\..\SRC\WainFunc.cpp
# End Source File
# Begin Source File

SOURCE=..\Src\WainMark.cpp
# End Source File
# Begin Source File

SOURCE=.\..\src\WainView.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\..\src\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=..\src\ClassView.h
# End Source File
# Begin Source File

SOURCE=..\src\DialogBase.h
# End Source File
# Begin Source File

SOURCE=..\src\DockBar.h
# End Source File
# Begin Source File

SOURCE=..\src\DocProp.h
# End Source File
# Begin Source File

SOURCE=..\src\GlobSet.h
# End Source File
# Begin Source File

SOURCE=.\..\SRC\KeySetup.h
# End Source File
# Begin Source File

SOURCE=.\..\src\MainFrm.h
# End Source File
# Begin Source File

SOURCE=..\Src\NavigatorDlg.h
# End Source File
# Begin Source File

SOURCE=..\Src\NavigatorList.h
# End Source File
# Begin Source File

SOURCE=..\src\PageDlg.h
# End Source File
# Begin Source File

SOURCE=.\..\SRC\Project.h
# End Source File
# Begin Source File

SOURCE=..\src\RegExp.h
# End Source File
# Begin Source File

SOURCE=.\..\src\Resource.h
# End Source File
# Begin Source File

SOURCE=..\src\SimpleDialog.h
# End Source File
# Begin Source File

SOURCE=.\..\src\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\src\TabView.h
# End Source File
# Begin Source File

SOURCE=..\src\TagList.h
# End Source File
# Begin Source File

SOURCE=..\src\Tags.h
# End Source File
# Begin Source File

SOURCE=..\src\Tools.h
# End Source File
# Begin Source File

SOURCE=.\..\src\Wain.h
# End Source File
# Begin Source File

SOURCE=.\..\src\WainDoc.h
# End Source File
# Begin Source File

SOURCE=..\src\WainSearch.h
# End Source File
# Begin Source File

SOURCE=.\..\src\WainView.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\res\smallico.bmp
# End Source File
# Begin Source File

SOURCE=..\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=..\res\Wain.ico
# End Source File
# Begin Source File

SOURCE=..\res\Wain.rc2
# End Source File
# Begin Source File

SOURCE=..\res\WainDoc.ico
# End Source File
# End Group
# Begin Group "Help Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\..\hlp\AfxCore.rtf
# End Source File
# Begin Source File

SOURCE=.\..\hlp\AfxPrint.rtf
# End Source File
# Begin Source File

SOURCE=.\..\hlp\AppExit.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\Bullet.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\CurArw2.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\CurArw4.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\CurHelp.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\EditCopy.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\EditCut.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\EditPast.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\EditUndo.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\FileNew.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\FileOpen.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\FilePrnt.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\FileSave.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\HlpSBar.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\HlpTBar.bmp
# End Source File
# Begin Source File

SOURCE=.\..\bat\MakeHelp.bat
# End Source File
# Begin Source File

SOURCE=.\..\hlp\RecFirst.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\RecLast.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\RecNext.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\RecPrev.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\Scmax.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\ScMenu.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\Scmin.bmp
# End Source File
# Begin Source File

SOURCE=.\..\hlp\Wain.cnt
# End Source File
# End Group
# Begin Source File

SOURCE=.\..\ReadMe.txt
# End Source File
# End Target
# End Project
