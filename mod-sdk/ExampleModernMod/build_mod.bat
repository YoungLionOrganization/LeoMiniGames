@echo off
setlocal
if "%~1"=="" (
  python "%~dp0..\..\tools\sdk\build_package.py" "%~dp0."
) else (
  python "%~dp0..\..\tools\sdk\build_package.py" "%~dp0." --rcc "%~1"
)
exit /b %errorlevel%
