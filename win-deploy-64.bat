@echo off
@rem VCToolsInstallDir=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.23.28105\
@rem VCToolsRedistDir=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.23.27820\

set BUILDPATH=C:\Users\rainer\documents\hoerbert\Build64

set signtoolexe="C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.18362.0\x64\signtool"
set xcopyexe="C:\\Windows\\System32\\xcopy"

@rem clear the path, so no clutter gets pulled in from wrong paths
set PATH=""

@echo Running windeployqt for 64 bit build from PATH
call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat" amd64
if %errorlevel% neq 0 goto :error

set path="%BUILDPATH%\Build\lib_win_;C:\Qt\5.15.2\msvc2019_64\bin";%PATH%
@rem C:\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe %BUILDPATH%\Build\bin\hoerbert.exe --release --force --no-system-d3d-compiler --no-compiler-runtime --qmldir C:\Qt\5.15.2\msvc2019_64\bin -core -gui -xml -network -concurrent

cd /D C:\Qt\5.15.2\msvc2019_64\bin
if %errorlevel% neq 0 goto :error

windeployqt.exe %BUILDPATH%\Build\bin --release --force --no-system-d3d-compiler --no-compiler-runtime --no-opengl-sw -core -gui -xml -network -concurrent
if %errorlevel% neq 0 goto :error

%signtoolexe% sign %BUILDPATH%\Build\bin\hoerbert.exe
if %errorlevel% neq 0 goto :error

%signtoolexe% timestamp /t http://timestamp.comodoca.com %BUILDPATH%\Build\bin\hoerbert.exe
if %errorlevel% neq 0 goto :error

%signtoolexe% verify /pa /d /v %BUILDPATH%\Build\bin\hoerbert.exe
if %errorlevel% neq 0 goto :error

@rem copy VC++ runtime libraries
%xcopyexe% "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.24.28127\x64\Microsoft.VC142.CRT" "%BUILDPATH%\Build\bin" /s /e /v /Y
if %errorlevel% neq 0 goto :error

%xcopyexe% "C:\Program Files\OpenSSL-Win64\libcrypto-1_1-x64.dll" "%BUILDPATH%\Build\bin\" /Y
if %errorlevel% neq 0 goto :error

%xcopyexe% "C:\Program Files\OpenSSL-Win64\libssl-1_1-x64.dll" "%BUILDPATH%\Build\bin\" /Y
if %errorlevel% neq 0 goto :error

cd /D %BUILDPATH%\..\hoerbert-software

echo.
echo SUCCESS!
exit /b 0

:error
echo.
echo Script failed with error #%errorlevel%.
cd /D %BUILDPATH%\..\hoerbert-software
exit /b %errorlevel%