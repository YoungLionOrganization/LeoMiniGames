@echo off
setlocal
if "%~1"=="" (
  echo Usage: build_mod.bat "D:\Qt\6.8.0\mingw_64\bin\rcc.exe"
  exit /b 2
)
"%~1" -binary mod.qrc -o example_modern-1.0.0.rcc
if errorlevel 1 exit /b %errorlevel%
echo Built: example_modern-1.0.0.rcc
