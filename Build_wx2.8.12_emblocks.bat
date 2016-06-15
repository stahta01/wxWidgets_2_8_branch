SET PATH=C:\Apps32\MinGW-4.7.1-use\bin;%SystemRoot%\system32

ECHO %PATH%

CD build\msw
REM PAUSE

SET _CPPFLAGS="-D_WIN32_IE=0x0601"
SET _CXXFLAGS="-fno-strict-aliasing -fpermissive -fno-keep-inline-dllexport -Wno-unused-local-typedefs"

REM clean
REM mingw32-make -f makefile.gcc CFG=_wxeb CPPFLAGS=%_CPPFLAGS% CXXFLAGS=%_CXXFLAGS% USE_XRC=1 SHARED=1 MONOLITHIC=1 BUILD=release UNICODE=1 VENDOR=eb clean
REM
REM setup_h
    mingw32-make -f makefile.gcc CFG=_wxeb CPPFLAGS=%_CPPFLAGS% CXXFLAGS=%_CXXFLAGS% USE_XRC=1 SHARED=1 MONOLITHIC=1 BUILD=release UNICODE=1 VENDOR=eb setup_h
    PAUSE
REM build_cfg_file
    mingw32-make -f makefile.gcc CFG=_wxeb CPPFLAGS=%_CPPFLAGS% CXXFLAGS=%_CXXFLAGS% USE_XRC=1 SHARED=1 MONOLITHIC=1 BUILD=release UNICODE=1 VENDOR=eb build_cfg_file
REM PAUSE
    mingw32-make -f makefile.gcc CFG=_wxeb CPPFLAGS=%_CPPFLAGS% CXXFLAGS=%_CXXFLAGS% USE_XRC=1 SHARED=1 MONOLITHIC=1 BUILD=release UNICODE=1 VENDOR=eb
REM BUILD=debug
REM >log.txt 2>&1
REM 
REM USE_OPENGL=1

PAUSE
