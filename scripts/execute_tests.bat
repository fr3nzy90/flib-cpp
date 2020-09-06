@echo off
setlocal

if [%~1%]==[] (
   set /a count = 1
) else (
   set /a count = %~1%
)

for %%i in (
   "../out/build/Windows-x86-Debug"
   "../out/build/Windows-x86-Release"
   "../out/build/Windows-x64-Debug"
   "../out/build/Windows-x64-Release"
) do (
   echo Folder: %%i
   for /l %%x in (1, 1, %count%) do "%%~i/tests.exe"
)
endlocal