# Microsoft Developer Studio Generated NMAKE File, Based on SimpleBrowser.dsp
!IF "$(CFG)" == ""
CFG=TimeStampWin - Win32 Debug
!MESSAGE No configuration specified. Defaulting to TimeStampWin - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "TimeStampWin - Win32 Release" && "$(CFG)" != "TimeStampWin - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SimpleBrowser.mak" CFG="TimeStampWin - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TimeStampWin - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TimeStampWin - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "TimeStampWin - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\SimpleBrowser.exe"


CLEAN :
	-@erase "$(INTDIR)\dlgtxtctrl.obj"
	-@erase "$(INTDIR)\SimpleBrowser.obj"
	-@erase "$(INTDIR)\SimpleBrowser.res"
	-@erase "$(INTDIR)\TCPClient.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\SimpleBrowser.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\SimpleBrowser.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SimpleBrowser.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib comctl32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\SimpleBrowser.pdb" /machine:I386 /out:"$(OUTDIR)\SimpleBrowser.exe" 
LINK32_OBJS= \
	"$(INTDIR)\dlgtxtctrl.obj" \
	"$(INTDIR)\SimpleBrowser.obj" \
	"$(INTDIR)\TCPClient.obj" \
	"$(INTDIR)\SimpleBrowser.res"

"$(OUTDIR)\SimpleBrowser.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "TimeStampWin - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\SimpleBrowser.exe" "$(OUTDIR)\SimpleBrowser.bsc"


CLEAN :
	-@erase "$(INTDIR)\dlgtxtctrl.obj"
	-@erase "$(INTDIR)\dlgtxtctrl.sbr"
	-@erase "$(INTDIR)\SimpleBrowser.obj"
	-@erase "$(INTDIR)\SimpleBrowser.res"
	-@erase "$(INTDIR)\SimpleBrowser.sbr"
	-@erase "$(INTDIR)\TCPClient.obj"
	-@erase "$(INTDIR)\TCPClient.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\SimpleBrowser.bsc"
	-@erase "$(OUTDIR)\SimpleBrowser.exe"
	-@erase "$(OUTDIR)\SimpleBrowser.ilk"
	-@erase "$(OUTDIR)\SimpleBrowser.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\SimpleBrowser.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SimpleBrowser.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dlgtxtctrl.sbr" \
	"$(INTDIR)\SimpleBrowser.sbr" \
	"$(INTDIR)\TCPClient.sbr"

"$(OUTDIR)\SimpleBrowser.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib comctl32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\SimpleBrowser.pdb" /debug /machine:I386 /out:"$(OUTDIR)\SimpleBrowser.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\dlgtxtctrl.obj" \
	"$(INTDIR)\SimpleBrowser.obj" \
	"$(INTDIR)\TCPClient.obj" \
	"$(INTDIR)\SimpleBrowser.res"

"$(OUTDIR)\SimpleBrowser.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("SimpleBrowser.dep")
!INCLUDE "SimpleBrowser.dep"
!ELSE 
!MESSAGE Warning: cannot find "SimpleBrowser.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "TimeStampWin - Win32 Release" || "$(CFG)" == "TimeStampWin - Win32 Debug"
SOURCE=.\dlgtxtctrl.cpp

!IF  "$(CFG)" == "TimeStampWin - Win32 Release"


"$(INTDIR)\dlgtxtctrl.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "TimeStampWin - Win32 Debug"


"$(INTDIR)\dlgtxtctrl.obj"	"$(INTDIR)\dlgtxtctrl.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\SimpleBrowser.cpp

!IF  "$(CFG)" == "TimeStampWin - Win32 Release"


"$(INTDIR)\SimpleBrowser.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "TimeStampWin - Win32 Debug"


"$(INTDIR)\SimpleBrowser.obj"	"$(INTDIR)\SimpleBrowser.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\TCPClient.cpp

!IF  "$(CFG)" == "TimeStampWin - Win32 Release"


"$(INTDIR)\TCPClient.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "TimeStampWin - Win32 Debug"


"$(INTDIR)\TCPClient.obj"	"$(INTDIR)\TCPClient.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\SimpleBrowser.rc

"$(INTDIR)\SimpleBrowser.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

