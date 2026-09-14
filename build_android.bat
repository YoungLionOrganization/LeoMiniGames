@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem LeoMiniGames Android local builder for Windows.
rem Usage: build_android.bat [arm64-v8a^|armeabi-v7a^|x86_64^|x86] [Release^|Debug] [apk^|aab^|both]
rem Optional environment: QT_BASE, QT_VERSION, QT_ANDROID_ROOT, ANDROID_SDK_ROOT,
rem ANDROID_NDK_ROOT, JAVA_HOME, LMG_KEEP_ANDROID_BUILD=1.

set "ROOT=%~dp0"
set "ABI=%~1"
if not defined ABI set "ABI=arm64-v8a"
set "CONFIG=%~2"
if not defined CONFIG set "CONFIG=Release"
set "PACKAGE=%~3"
if not defined PACKAGE set "PACKAGE=apk"

set "QT_ARCH="
if /I "%ABI%"=="arm64-v8a" set "QT_ARCH=android_arm64_v8a"
if /I "%ABI%"=="armeabi-v7a" set "QT_ARCH=android_armv7"
if /I "%ABI%"=="x86_64" set "QT_ARCH=android_x86_64"
if /I "%ABI%"=="x86" set "QT_ARCH=android_x86"
if not defined QT_ARCH (
  echo [ERROR] Unsupported ABI: %ABI%
  exit /b 2
)

if not defined QT_BASE set "QT_BASE=D:\Qt"
if defined QT_ANDROID_ROOT (
  set "QT_KIT=%QT_ANDROID_ROOT%"
) else (
  if defined QT_VERSION (
    set "QT_KIT=%QT_BASE%\%QT_VERSION%\%QT_ARCH%"
  ) else (
    for %%V in (6.11.1 6.11.0 6.10.2 6.10.1) do (
      if not defined QT_KIT if exist "%QT_BASE%\%%V\%QT_ARCH%\bin\qt-cmake.bat" (
        set "QT_KIT=%QT_BASE%\%%V\%QT_ARCH%"
        set "QT_VERSION=%%V"
      )
    )
  )
)
if not defined QT_KIT (
  echo [ERROR] A matching Qt Android kit was not found for %ABI%.
  echo         Install %QT_ARCH% with Qt Maintenance Tool, or set:
  echo         set QT_ANDROID_ROOT=D:\Qt\6.11.1\%QT_ARCH%
  exit /b 3
)
set "QTCMAKE=%QT_KIT%\bin\qt-cmake.bat"
if not exist "%QTCMAKE%" (
  echo [ERROR] qt-cmake.bat not found: %QTCMAKE%
  exit /b 3
)

rem Qt for Android is a cross build and needs a same-version host Qt installation.
set "QT_HOST_FOUND="
for %%H in (llvm-mingw_64 msvc2022_64 mingw_64) do (
  if exist "%QT_BASE%\%QT_VERSION%\%%H\bin\qtpaths.exe" set "QT_HOST_FOUND=%QT_BASE%\%QT_VERSION%\%%H"
)
if not defined QT_HOST_FOUND (
  echo [ERROR] Same-version desktop Qt host tools were not found under %QT_BASE%\%QT_VERSION%.
  echo         Install a desktop kit for Qt %QT_VERSION% as well as the Android kit.
  exit /b 4
)

if not defined ANDROID_SDK_ROOT if defined ANDROID_HOME set "ANDROID_SDK_ROOT=%ANDROID_HOME%"
if not defined ANDROID_SDK_ROOT set "ANDROID_SDK_ROOT=%LOCALAPPDATA%\Android\Sdk"
if not exist "%ANDROID_SDK_ROOT%" (
  echo [ERROR] Android SDK not found: %ANDROID_SDK_ROOT%
  echo         Configure Android in Qt Creator or set ANDROID_SDK_ROOT.
  exit /b 5
)
if not defined ANDROID_NDK_ROOT set "ANDROID_NDK_ROOT=%ANDROID_SDK_ROOT%\ndk\27.2.12479018"
if not exist "%ANDROID_NDK_ROOT%\source.properties" (
  echo [ERROR] Android NDK r27c not found: %ANDROID_NDK_ROOT%
  echo         Expected revision: 27.2.12479018
  exit /b 6
)
if not exist "%ANDROID_SDK_ROOT%\platforms\android-36\android.jar" (
  echo [ERROR] Android SDK Platform 36 is missing.
  echo         Install platforms;android-36 in Qt Creator/Android SDK Manager.
  exit /b 7
)
if not exist "%ANDROID_SDK_ROOT%\build-tools\36.0.0" (
  echo [ERROR] Android Build Tools 36.0.0 are missing.
  echo         Install build-tools;36.0.0 in Qt Creator/Android SDK Manager.
  exit /b 8
)

set "CMAKE_EXE="
if exist "%QT_BASE%\Tools\CMake_64\bin\cmake.exe" set "CMAKE_EXE=%QT_BASE%\Tools\CMake_64\bin\cmake.exe"
if not defined CMAKE_EXE for /f "delims=" %%I in ('where cmake.exe 2^>nul') do if not defined CMAKE_EXE set "CMAKE_EXE=%%I"
if not defined CMAKE_EXE (
  echo [ERROR] cmake.exe not found.
  exit /b 9
)
set "NINJA_EXE="
if exist "%QT_BASE%\Tools\Ninja\ninja.exe" set "NINJA_EXE=%QT_BASE%\Tools\Ninja\ninja.exe"
if not defined NINJA_EXE for /f "delims=" %%I in ('where ninja.exe 2^>nul') do if not defined NINJA_EXE set "NINJA_EXE=%%I"
if not defined NINJA_EXE (
  echo [ERROR] ninja.exe not found.
  exit /b 10
)

rem Reuse Qt/Android Studio JDK automatically when JAVA_HOME/PATH is not configured.
where java.exe >nul 2>nul
if errorlevel 1 (
  if defined JAVA_HOME if exist "%JAVA_HOME%\bin\java.exe" set "PATH=%JAVA_HOME%\bin;%PATH%"
)
where java.exe >nul 2>nul
if errorlevel 1 if exist "%QT_BASE%\Tools\OpenJDK\bin\java.exe" (
  set "JAVA_HOME=%QT_BASE%\Tools\OpenJDK"
  set "PATH=%JAVA_HOME%\bin;%PATH%"
)
where java.exe >nul 2>nul
if errorlevel 1 if exist "%ProgramFiles%\Android\Android Studio\jbr\bin\java.exe" (
  set "JAVA_HOME=%ProgramFiles%\Android\Android Studio\jbr"
  set "PATH=%JAVA_HOME%\bin;%PATH%"
)
where java.exe >nul 2>nul
if errorlevel 1 (
  echo [ERROR] Java/JDK not found. Qt 6.10 uses JDK 17+; Qt 6.11 supported configuration uses JDK 21.
  exit /b 11
)

set "BUILD_DIR=%ROOT%build\android-%ABI%-%CONFIG%"
if /I not "%LMG_KEEP_ANDROID_BUILD%"=="1" if exist "%BUILD_DIR%" (
  echo [INFO] Removing stale Android build directory: %BUILD_DIR%
  rmdir /s /q "%BUILD_DIR%"
  if exist "%BUILD_DIR%" (
    echo [ERROR] Could not remove stale build directory.
    exit /b 12
  )
)

if /I not "%PACKAGE%"=="apk" if /I not "%PACKAGE%"=="aab" if /I not "%PACKAGE%"=="both" (
  echo [ERROR] Third argument must be apk, aab or both.
  exit /b 13
)

echo ============================================================
echo   LeoMiniGames Android Build
echo ============================================================
echo Qt kit : %QT_KIT%
echo Qt host: %QT_HOST_FOUND%
echo ABI    : %ABI%
echo Config : %CONFIG%
echo SDK    : %ANDROID_SDK_ROOT%
echo NDK    : %ANDROID_NDK_ROOT%
echo CMake  : %CMAKE_EXE%
echo Ninja  : %NINJA_EXE%
echo Java   :
java -version 2^>^&1
echo Output : %BUILD_DIR%
echo.

call "%QTCMAKE%" -S "%ROOT%." -B "%BUILD_DIR%" -G Ninja ^
  -DCMAKE_MAKE_PROGRAM="%NINJA_EXE%" ^
  -DCMAKE_BUILD_TYPE=%CONFIG% ^
  -DBUILD_TESTING=OFF ^
  -DANDROID_ABI=%ABI% ^
  -DANDROID_SDK_ROOT="%ANDROID_SDK_ROOT%" ^
  -DANDROID_NDK_ROOT="%ANDROID_NDK_ROOT%"
if errorlevel 1 (
  echo [ERROR] Qt/CMake configure failed. The build directory was fresh; check the first CMake error above.
  exit /b !errorlevel!
)

if /I "%PACKAGE%"=="apk" goto build_apk
if /I "%PACKAGE%"=="aab" goto build_aab

goto build_both

:build_apk
"%CMAKE_EXE%" --build "%BUILD_DIR%" --target apk --parallel
if errorlevel 1 exit /b !errorlevel!
goto done

:build_aab
"%CMAKE_EXE%" --build "%BUILD_DIR%" --target aab --parallel
if errorlevel 1 exit /b !errorlevel!
goto done

:build_both
"%CMAKE_EXE%" --build "%BUILD_DIR%" --target apk --parallel
if errorlevel 1 exit /b !errorlevel!
"%CMAKE_EXE%" --build "%BUILD_DIR%" --target aab --parallel
if errorlevel 1 exit /b !errorlevel!

:done
echo.
echo [OK] Android build completed.
echo Generated packages:
for /r "%BUILD_DIR%" %%F in (*.apk *.aab) do echo   %%F
exit /b 0
