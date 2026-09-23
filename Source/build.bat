@echo off
setlocal EnableExtensions
REM ---- Compila deus_translate.dll (Release, Win32) usando MSBuild + el proyecto VS ----

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo [ERROR] No se encontro vswhere. Instala Visual Studio con el workload "Desarrollo para el escritorio con C++".
  exit /b 1
)

set "VSINSTALL="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%i"
if "%VSINSTALL%"=="" (
  echo [ERROR] No hay una instalacion de VS con herramientas de C++ ^(x86^).
  exit /b 1
)

set "MSBUILD=%VSINSTALL%\MSBuild\Current\Bin\MSBuild.exe"
if not exist "%MSBUILD%" (
  echo [ERROR] No se encontro MSBuild en: %MSBUILD%
  exit /b 1
)

"%MSBUILD%" "%~dp0deus_translate.vcxproj" /p:Configuration=Release /p:Platform=Win32 /nologo /v:minimal
if errorlevel 1 (
  echo.
  echo [ERROR] Fallo la compilacion.
  exit /b 1
)

echo.
echo [OK] Compilado: %~dp0bin\Release\deus_translate.dll
endlocal
