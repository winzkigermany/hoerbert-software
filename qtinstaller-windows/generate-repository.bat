@echo off
set repogenexe="c:\\Qt\\QtIFW-4.0.1\\bin\\repogen.exe"

@REM %repogenexe% -r --unite-metadata -p packages repository
%repogenexe% --update --unite-metadata -v -p %~dp0/packages %~dp0/repository

