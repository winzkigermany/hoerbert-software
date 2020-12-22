@echo off

set nsisexe="c:\Program Files (x86)\NSIS\Bin\makensis.exe"
set signtoolexe="C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.18362.0\x64\signtool"

%nsisexe% /V4 hoerbert.nsi

c:\Windows\System32\timeout.exe /T 2

%signtoolexe% sign ..\Build32\hoerbert-installer.exe
%signtoolexe% timestamp /t http://timestamp.comodoca.com ..\Build32\hoerbert-installer.exe
%signtoolexe% verify /pa /d /v ..\Build32\hoerbert-installer.exe