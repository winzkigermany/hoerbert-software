@echo off
@rem VCToolsInstallDir=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.23.28105\
@rem VCToolsRedistDir=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.23.27820\

set signtoolexe="C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.18362.0\x64\signtool"
set xcopyexe="C:\\Windows\\System32\\xcopy"
set binarycreatorexe="c:\\Qt\\QtIFW-3.1.1\\bin\\binarycreator.exe"

@rem clear the path, so no clutter gets pulled in from wrong paths
set PATH=""

@echo Running windeployqt for 32 bit build from PATH
call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat" x86

C:\Qt\5.12.5\msvc2017\bin\windeployqt.exe ..\Build32\Build\bin\hoerbert.exe --release --force --no-system-d3d-compiler -core -gui -xml -network --no-compiler-runtime

%signtoolexe% sign ..\Build32\Build\bin\hoerbert.exe
%signtoolexe% timestamp /t http://timestamp.comodoca.com ..\Build32\Build\bin\hoerbert.exe
%signtoolexe% verify /pa /d /v ..\Build32\Build\bin\hoerbert.exe

rmdir qtinstaller-windows\packages32\hoerbert\data /s /q
mkdir qtinstaller-windows\packages32\hoerbert\data
@rem copy VC++ runtime libraries
%xcopyexe% "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Redist\\MSVC\\14.24.28127\\x86\\Microsoft.VC142.CRT" "..\\Build32\\Build\\bin" /s /e /v /y
%xcopyexe% "C:\\Program Files (x86)\\OpenSSL-Win32\\libcrypto-1_1.dll" "..\\Build32\\Build\\bin\\"
%xcopyexe% "C:\\Program Files (x86)\\OpenSSL-Win32\\libssl-1_1.dll" "..\\Build32\\Build\\bin\\"
%xcopyexe% ..\Build32\Build\bin qtinstaller-windows\packages32\hoerbert\data /s /e /v

%binarycreatorexe% -v --offline-only -c qtinstaller-windows\config\config.xml -p qtinstaller-windows\packages32 ../Build32/hoerbert-setup-32bit.exe
%binarycreatorexe% -v --offline-only -c qtinstaller-windows\config\config.xml -p qtinstaller-windows\packages32 --ignore-translations ../Build32/hoerbert-setup-32bit.exe

%signtoolexe% sign ..\Build32\hoerbert-setup-32bit.exe
%signtoolexe% timestamp /t http://timestamp.comodoca.com ..\Build32\hoerbert-setup-32bit.exe
%signtoolexe% verify /pa /d /v ..\Build32\hoerbert-setup-32bit.exe