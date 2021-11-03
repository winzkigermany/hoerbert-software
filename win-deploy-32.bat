@echo off
@rem VCToolsInstallDir=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.23.28105\
@rem VCToolsRedistDir=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.23.27820\

set signtoolexe="C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.18362.0\x64\signtool"
set xcopyexe="C:\\Windows\\System32\\xcopy"
set binarycreatorexe="c:\\Qt\\QtIFW-4.0.1\\bin\\binarycreator.exe"

@rem clear the path, so no clutter gets pulled in from wrong paths
set PATH=""

@echo Running windeployqt for 32 bit build from PATH
call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat" x86
set path="C:\Users\rainer\Documents\hoerbert\Build32\hoerbert\..\Build\lib_win_;C:\Qt\5.15.2\msvc2019\bin";%PATH%
C:\Qt\5.15.2\msvc2019\bin\windeployqt.exe ..\Build32\Build\bin\hoerbert.exe --release --force --no-system-d3d-compiler -core -gui -xml -network -concurrent --no-compiler-runtime

%signtoolexe% sign ..\Build32\Build\bin\hoerbert.exe
%signtoolexe% timestamp /t http://timestamp.comodoca.com ..\Build32\Build\bin\hoerbert.exe
%signtoolexe% verify /pa /d /v ..\Build32\Build\bin\hoerbert.exe

rmdir qtinstaller-windows\packages\hoerbert32\data /s /q
mkdir qtinstaller-windows\packages\hoerbert32\data
@rem copy VC++ runtime libraries
%xcopyexe% "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.24.28127\x86\Microsoft.VC142.CRT" "..\Build32\Build\bin" /s /e /v /Y
%xcopyexe% "C:\Program Files (x86)\OpenSSL-Win32\libcrypto-1_1.dll" "..\Build32\Build\bin\" /Y
%xcopyexe% "C:\Program Files (x86)\OpenSSL-Win32\libssl-1_1.dll" "..\Build32\Build\bin\" /Y
%xcopyexe% ..\Build32\Build\bin qtinstaller-windows\packages\hoerbert32\data /s /e /v /Y

@REM %binarycreatorexe% -v --repository qtinstaller-windows\repository -c qtinstaller-windows\config\config.xml ../Build32/hoerbert-setup.exe
@REM %binarycreatorexe% -v -c qtinstaller-windows\config\config.xml -p qtinstaller-windows\packages ../Build32/hoerbert-setup.exe
@REM %binarycreatorexe% -v -c qtinstaller-windows\config\config.xml -p qtinstaller-windows\packages --ignore-translations ../Build32/hoerbert-setup.exe
@REM %signtoolexe% sign ..\Build32\hoerbert-setup.exe
@REM %signtoolexe% timestamp /t http://timestamp.comodoca.com ..\Build32\hoerbert-setup.exe
@REM %signtoolexe% verify /pa /d /v ..\Build32\hoerbert-setup.exe
