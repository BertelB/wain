# Microsoft Developer Studio Generated NMAKE File, Based on Wain.dsp
!IF "$(CFG)" == ""
CFG=WAIN - Win32 Debug
!MESSAGE No configuration specified. Defaulting to WAIN - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "WAIN - Win32 Release" && "$(CFG)" != "WAIN - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "WAIN - Win32 Release"

OUTDIR=.\..\Release
INTDIR=.\..\Release
# Begin Custom Macros
OutDir=.\..\Release
# End Custom Macros

ALL : "$(OUTDIR)\Wain.exe"


CLEAN :
	-@erase "$(INTDIR)\CallTree.obj"
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\classview.obj"
	-@erase "$(INTDIR)\DockBar.obj"
	-@erase "$(INTDIR)\DocProp.obj"
	-@erase "$(INTDIR)\FileSelD.obj"
	-@erase "$(INTDIR)\GlobSet.obj"
	-@erase "$(INTDIR)\KeySetup.obj"
	-@erase "$(INTDIR)\Line.obj"
	-@erase "$(INTDIR)\Macro.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\NavigatorDlg.obj"
	-@erase "$(INTDIR)\NavigatorFile.obj"
	-@erase "$(INTDIR)\NavigatorList.obj"
	-@erase "$(INTDIR)\PageDlg.obj"
	-@erase "$(INTDIR)\Print.obj"
	-@erase "$(INTDIR)\Project.obj"
	-@erase "$(INTDIR)\regexp.obj"
	-@erase "$(INTDIR)\Search.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\TabView.obj"
	-@erase "$(INTDIR)\TagList.obj"
	-@erase "$(INTDIR)\Tags.obj"
	-@erase "$(INTDIR)\Tools.obj"
	-@erase "$(INTDIR)\Undo.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\ViewList.obj"
	-@erase "$(INTDIR)\Wain.obj"
	-@erase "$(INTDIR)\Wain.pch"
	-@erase "$(INTDIR)\Wain.res"
	-@erase "$(INTDIR)\WainDoc.obj"
	-@erase "$(INTDIR)\WainEdit.obj"
	-@erase "$(INTDIR)\WainFTP.obj"
	-@erase "$(INTDIR)\WainFunc.obj"
	-@erase "$(INTDIR)\WainMark.obj"
	-@erase "$(INTDIR)\WainView.obj"
	-@erase "$(OUTDIR)\Wain.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W4 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Fp"$(INTDIR)\Wain.pch" /Yu".\..\src\stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x406 /fo"$(INTDIR)\Wain.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Wain.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=MPR.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\Wain.pdb" /machine:I386 /out:"$(OUTDIR)\Wain.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CallTree.obj" \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\classview.obj" \
	"$(INTDIR)\DockBar.obj" \
	"$(INTDIR)\DocProp.obj" \
	"$(INTDIR)\FileSelD.obj" \
	"$(INTDIR)\GlobSet.obj" \
	"$(INTDIR)\KeySetup.obj" \
	"$(INTDIR)\Line.obj" \
	"$(INTDIR)\Macro.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\NavigatorDlg.obj" \
	"$(INTDIR)\NavigatorFile.obj" \
	"$(INTDIR)\NavigatorList.obj" \
	"$(INTDIR)\PageDlg.obj" \
	"$(INTDIR)\Print.obj" \
	"$(INTDIR)\Project.obj" \
	"$(INTDIR)\regexp.obj" \
	"$(INTDIR)\Search.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TabView.obj" \
	"$(INTDIR)\TagList.obj" \
	"$(INTDIR)\Tags.obj" \
	"$(INTDIR)\Tools.obj" \
	"$(INTDIR)\Undo.obj" \
	"$(INTDIR)\ViewList.obj" \
	"$(INTDIR)\Wain.obj" \
	"$(INTDIR)\WainDoc.obj" \
	"$(INTDIR)\WainEdit.obj" \
	"$(INTDIR)\WainFTP.obj" \
	"$(INTDIR)\WainFunc.obj" \
	"$(INTDIR)\WainMark.obj" \
	"$(INTDIR)\WainView.obj" \
	"$(INTDIR)\Wain.res"

"$(OUTDIR)\Wain.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "WAIN - Win32 Debug"

OUTDIR=.\..\Debug
INTDIR=.\..\Debug
# Begin Custom Macros
OutDir=.\..\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Wain.exe"


CLEAN :
	-@erase "$(INTDIR)\CallTree.obj"
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\classview.obj"
	-@erase "$(INTDIR)\DockBar.obj"
	-@erase "$(INTDIR)\DocProp.obj"
	-@erase "$(INTDIR)\FileSelD.obj"
	-@erase "$(INTDIR)\GlobSet.obj"
	-@erase "$(INTDIR)\KeySetup.obj"
	-@erase "$(INTDIR)\Line.obj"
	-@erase "$(INTDIR)\Macro.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\NavigatorDlg.obj"
	-@erase "$(INTDIR)\NavigatorFile.obj"
	-@erase "$(INTDIR)\NavigatorList.obj"
	-@erase "$(INTDIR)\PageDlg.obj"
	-@erase "$(INTDIR)\Print.obj"
	-@erase "$(INTDIR)\Project.obj"
	-@erase "$(INTDIR)\regexp.obj"
	-@erase "$(INTDIR)\Search.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\TabView.obj"
	-@erase "$(INTDIR)\TagList.obj"
	-@erase "$(INTDIR)\Tags.obj"
	-@erase "$(INTDIR)\Tools.obj"
	-@erase "$(INTDIR)\Undo.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\ViewList.obj"
	-@erase "$(INTDIR)\Wain.obj"
	-@erase "$(INTDIR)\Wain.pch"
	-@erase "$(INTDIR)\Wain.res"
	-@erase "$(INTDIR)\WainDoc.obj"
	-@erase "$(INTDIR)\WainEdit.obj"
	-@erase "$(INTDIR)\WainFTP.obj"
	-@erase "$(INTDIR)\WainFunc.obj"
	-@erase "$(INTDIR)\WainMark.obj"
	-@erase "$(INTDIR)\WainView.obj"
	-@erase "$(OUTDIR)\Wain.exe"
	-@erase "$(OUTDIR)\Wain.ilk"
	-@erase "$(OUTDIR)\Wain.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /O2 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Fp"$(INTDIR)\Wain.pch" /Yu".\..\src\stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x406 /fo"$(INTDIR)\Wain.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Wain.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=MPR.LIB /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\Wain.pdb" /debug /machine:I386 /out:"$(OUTDIR)\Wain.exe" 
LINK32_OBJS= \
	"$(INTDIR)\CallTree.obj" \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\classview.obj" \
	"$(INTDIR)\DockBar.obj" \
	"$(INTDIR)\DocProp.obj" \
	"$(INTDIR)\FileSelD.obj" \
	"$(INTDIR)\GlobSet.obj" \
	"$(INTDIR)\KeySetup.obj" \
	"$(INTDIR)\Line.obj" \
	"$(INTDIR)\Macro.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\NavigatorDlg.obj" \
	"$(INTDIR)\NavigatorFile.obj" \
	"$(INTDIR)\NavigatorList.obj" \
	"$(INTDIR)\PageDlg.obj" \
	"$(INTDIR)\Print.obj" \
	"$(INTDIR)\Project.obj" \
	"$(INTDIR)\regexp.obj" \
	"$(INTDIR)\Search.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TabView.obj" \
	"$(INTDIR)\TagList.obj" \
	"$(INTDIR)\Tags.obj" \
	"$(INTDIR)\Tools.obj" \
	"$(INTDIR)\Undo.obj" \
	"$(INTDIR)\ViewList.obj" \
	"$(INTDIR)\Wain.obj" \
	"$(INTDIR)\WainDoc.obj" \
	"$(INTDIR)\WainEdit.obj" \
	"$(INTDIR)\WainFTP.obj" \
	"$(INTDIR)\WainFunc.obj" \
	"$(INTDIR)\WainMark.obj" \
	"$(INTDIR)\WainView.obj" \
	"$(INTDIR)\Wain.res"

"$(OUTDIR)\Wain.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Wain.dep")
!INCLUDE "Wain.dep"
!ELSE 
!MESSAGE Warning: cannot find "Wain.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "WAIN - Win32 Release" || "$(CFG)" == "WAIN - Win32 Debug"
SOURCE=..\Src\CallTree.cpp

"$(INTDIR)\CallTree.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\src\ChildFrm.cpp

"$(INTDIR)\ChildFrm.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Src\classview.cpp

"$(INTDIR)\classview.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Src\DockBar.cpp

"$(INTDIR)\DockBar.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\Src\DocProp.cpp

"$(INTDIR)\DocProp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Src\FileSelD.cpp

"$(INTDIR)\FileSelD.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\SRC\GlobSet.cpp

"$(INTDIR)\GlobSet.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\SRC\KeySetup.cpp

"$(INTDIR)\KeySetup.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SRC\Line.cpp

"$(INTDIR)\Line.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\SRC\Macro.cpp

"$(INTDIR)\Macro.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\src\MainFrm.cpp

"$(INTDIR)\MainFrm.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Src\NavigatorDlg.cpp

"$(INTDIR)\NavigatorDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Src\NavigatorFile.cpp

"$(INTDIR)\NavigatorFile.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Src\NavigatorList.cpp

"$(INTDIR)\NavigatorList.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Src\PageDlg.cpp

"$(INTDIR)\PageDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Src\Print.cpp

"$(INTDIR)\Print.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\SRC\Project.cpp

"$(INTDIR)\Project.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Src\regexp.cpp

"$(INTDIR)\regexp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\SRC\Search.cpp

"$(INTDIR)\Search.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\src\StdAfx.cpp

!IF  "$(CFG)" == "WAIN - Win32 Release"

CPP_SWITCHES=/nologo /MD /W4 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Fp"$(INTDIR)\Wain.pch" /Yc".\..\src\stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\Wain.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "WAIN - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /Zi /O2 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Fp"$(INTDIR)\Wain.pch" /Yc".\..\src\stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\Wain.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\Src\TabView.cpp

"$(INTDIR)\TabView.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\SRC\TagList.cpp

"$(INTDIR)\TagList.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SRC\Tags.cpp

"$(INTDIR)\Tags.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SRC\Tools.cpp

"$(INTDIR)\Tools.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\SRC\Undo.cpp

"$(INTDIR)\Undo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SRC\ViewList.cpp

"$(INTDIR)\ViewList.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\src\Wain.cpp

"$(INTDIR)\Wain.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\src\Wain.rc

!IF  "$(CFG)" == "WAIN - Win32 Release"


"$(INTDIR)\Wain.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x406 /fo"$(INTDIR)\Wain.res" /i "\Program\Wain\Wain.1.0\src" /d "NDEBUG" /d "_AFXDLL" $(SOURCE)


!ELSEIF  "$(CFG)" == "WAIN - Win32 Debug"


"$(INTDIR)\Wain.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x406 /fo"$(INTDIR)\Wain.res" /i "\Program\Wain\Wain.1.0\src" /d "_DEBUG" /d "_AFXDLL" $(SOURCE)


!ENDIF 

SOURCE=.\..\src\WainDoc.cpp

"$(INTDIR)\WainDoc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\SRC\WainEdit.cpp

"$(INTDIR)\WainEdit.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Src\WainFTP.cpp

"$(INTDIR)\WainFTP.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\SRC\WainFunc.cpp

"$(INTDIR)\WainFunc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Src\WainMark.cpp

"$(INTDIR)\WainMark.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\..\src\WainView.cpp

"$(INTDIR)\WainView.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wain.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

