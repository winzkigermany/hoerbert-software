@echo off
@rem VCToolsInstallDir=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.23.28105\
@rem VCToolsRedistDir=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.23.27820\

set signtoolexe="C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.18362.0\x64\signtool"
set xcopyexe="C:\\Windows\\System32\\xcopy"
set binarycreatorexe="c:\\Qt\\QtIFW-3.1.1\\bin\\binarycreator.exe"

@rem clear the path, so no clutter gets pulled in from wrong paths
set PATH=""

@echo Running windeployqt for 64 bit build from PATH
call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat" amd64
set path="C:\Users\rainer\Documents\hoerbert\Build64\hoerbert\..\Build\lib_win_;C:\Qt\5.12.5\msvc2017_64\bin";%PATH%
C:\Qt\5.12.5\msvc2017_64\bin\windeployqt.exe ..\Build64\Build\bin\hoerbert.exe --release --force --no-system-d3d-compiler -core -gui -xml -network --no-compiler-runtime

%signtoolexe% sign ..\Build64\Build\bin\hoerbert.exe
%signtoolexe% timestamp /t http://timestamp.comodoca.com ..\Build64\Build\bin\hoerbert.exe
%signtoolexe% verify /pa /d /v ..\Build64\Build\bin\hoerbert.exe

rmdir qtinstaller-windows\packages64\hoerbert\data /s /q
mkdir qtinstaller-windows\packages64\hoerbert\data
@rem copy VC++ runtime libraries
%xcopyexe% "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Redist\\MSVC\\14.24.28127\\x64\\Microsoft.VC142.CRT" "..\\Build64\\Build\\bin" /s /e /v /y
%xcopyexe% "C:\\Program Files\\OpenSSL-Win64\\libcrypto-1_1-x64.dll" "..\\Build64\\Build\\bin\\"
%xcopyexe% "C:\\Program Files\\OpenSSL-Win64\\libssl-1_1-x64.dll" "..\\Build64\\Build\\bin\\"
%xcopyexe% ..\Build64\Build\bin qtinstaller-windows\packages64\hoerbert\data /s /e /v

%binarycreatorexe% -v --offline-only -c qtinstaller-windows\config\config.xml -p qtinstaller-windows\packages64 ../Build64/hoerbert-setup-64bit.exe
%binarycreatorexe% -v --offline-only -c qtinstaller-windows\config\config.xml -p qtinstaller-windows\packages64 --ignore-translations ../Build64/hoerbert-setup-64bit.exe

%signtoolexe% sign ..\Build64\hoerbert-setup-64bit.exe
%signtoolexe% timestamp /t http://timestamp.comodoca.com ..\Build64\hoerbert-setup-64bit.exe
%signtoolexe% verify /pa /d /v ..\Build64\hoerbert-setup-64bit.exe