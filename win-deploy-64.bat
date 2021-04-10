@echo off
@rem VCToolsInstallDir=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.23.28105\
@rem VCToolsRedistDir=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.23.27820\

set signtoolexe="C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.18362.0\x64\signtool"
set xcopyexe="C:\\Windows\\System32\\xcopy"
set binarycreatorexe="c:\\Qt\\QtIFW-4.0.1\\bin\\binarycreator.exe"

@rem clear the path, so no clutter gets pulled in from wrong paths
set PATH=""

@echo Running windeployqt for 64 bit build from PATH
call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat" amd64
set path="C:\Users\rainer\Documents\hoerbert\Build64\hoerbert\..\Build\lib_win_;C:\Qt\5.15.2\msvc2017_64\bin";%PATH%
C:\Qt\5.15.2\msvc2017_64\bin\windeployqt.exe ..\Build64\Build\bin\hoerbert.exe --release --force --no-system-d3d-compiler -core -gui -xml -network -concurrent --no-compiler-runtime

%signtoolexe% sign ..\Build64\Build\bin\hoerbert.exe
%signtoolexe% timestamp /t http://timestamp.comodoca.com ..\Build64\Build\bin\hoerbert.exe
%signtoolexe% verify /pa /d /v ..\Build64\Build\bin\hoerbert.exe

rmdir qtinstaller-windows\packages\hoerbert64\data /s /q
mkdir qtinstaller-windows\packages\hoerbert64\data
@rem copy VC++ runtime libraries
%xcopyexe% "C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.24.28127\x64\Microsoft.VC142.CRT" "..\Build64\Build\bin" /s /e /v /Y
%xcopyexe% "C:\Program Files\OpenSSL-Win64\libcrypto-1_1-x64.dll" "..\Build64\Build\bin\" /Y
%xcopyexe% "C:\Program Files\OpenSSL-Win64\libssl-1_1-x64.dll" "..\Build64\Build\bin\" /Y
%xcopyexe% ..\Build64\Build\bin qtinstaller-windows\packages\hoerbert64\data /s /e /v /Y

@REM %binarycreatorexe% -v --repository qtinstaller-windows\repository -c qtinstaller-windows\config\config.xml ../Build64/hoerbert-setup.exe
@REM %binarycreatorexe% -v -c qtinstaller-windows\config\config.xml -p qtinstaller-windows\packages\hoerbert64 ../Build64/hoerbert-setup-64bit.exe
@REM %binarycreatorexe% -v -c qtinstaller-windows\config\config.xml -p qtinstaller-windows\packages\hoerbert64 --ignore-translations ../Build64/hoerbert-setup-64bit.exe
@REM %signtoolexe% sign ..\Build64\hoerbert-setup-64bit.exe
@REM %signtoolexe% timestamp /t http://timestamp.comodoca.com ..\Build64\hoerbert-setup-64bit.exe
@REM %signtoolexe% verify /pa /d /v ..\Build64\hoerbert-setup-64bit.exe
