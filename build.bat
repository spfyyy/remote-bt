@echo off
setlocal
cls

set PROJ_DIR=%~dp0.
cd "%PROJ_DIR%"

REM verify necessary environment variables are present
if "%VCPKG_X64_PATH%" == "" (
	echo "VCPKG_X64_PATH environment variable not defined" >&2
	goto :error
)
if "%VS_PATH%" == "" (
    echo "VS_PATH environment variable not defined" >&2
	goto :error
)
cl >NUL 2>NUL || call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" || goto :error

REM configure build directory
if not exist build (
    mkdir build || goto :error
)
cd build
if not exist ssh.dll (
	copy "%VCPKG_X64_PATH%\bin\ssh.dll" . || goto :error
)
if not exist pthreadVC3.dll (
	copy "%VCPKG_X64_PATH%\bin\pthreadVC3.dll" . || goto :error
)

cl /Zi ^
	/I"%VCPKG_X64_PATH%\include" ^
	"%PROJ_DIR%\src\remote_bt_cli.c" ^
	"%PROJ_DIR%\src\remote_bt.c" ^
	"%PROJ_DIR%\src\bencode.c" ^
	"%PROJ_DIR%\src\torrent.c" ^
	/link /LIBPATH:"%VCPKG_X64_PATH%\lib" ^
	ssh.lib || goto :error
goto :EOF

:error
exit /b 1
