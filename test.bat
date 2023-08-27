@echo off
setlocal
cls

set PROJ_DIR=%~dp0
cd "%PROJ_DIR%"

if not exist build (
    mkdir build || goto :error
)

cd build

if "%VS_PATH%" == "" (
    echo "VS_PATH environment variable not defined" >&2
)
cl >NUL 2>NUL || call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" || goto :error

cl "%PROJ_DIR%\src\test\test_suite.c" || goto :error
.\test_suite.exe || goto :error
goto :EOF

:error
exit /b 1
