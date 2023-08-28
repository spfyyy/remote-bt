@echo off
setlocal
cls

set PROJ_DIR=%~dp0
cd "%PROJ_DIR%"

if "%VS_PATH%" == "" (
    echo "VS_PATH environment variable not defined" >&2
)
cl >NUL 2>NUL || call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" || goto :error

REM build depencies
REM build openssl (needed to build libssh)
if not exist src\dependencies\openssl\libcrypto.lib (
    cd src\dependencies\openssl || goto :error
    perl Configure VC-WIN64A no-apps no-docs && nmake || goto :error
	cd "%PROJ_DIR%"
)

REM build libssh
if not exist src\dependencies\libssh\build\src\Release\ssh.lib (
    if not exist src\dependencies\libssh\build (
        mkdir src\dependencies\libssh\build || goto :error
    )
	cd src\dependencies\libssh\build

    cmake -S "%PROJ_DIR%\src\dependencies\libssh" -DCMAKE_BUILD_TYPE=Release -DWITH_ZLIB=OFF -DWITH_EXAMPLES=OFF -DOPENSSL_ROOT_DIR="%PROJ_DIR%\src\dependencies\openssl" -DOPENSSL_CRYPTO_LIBRARY="%PROJ_DIR%\src\dependencies\openssl\libcrypto.lib" -DOPENSSL_INCLUDE_DIR="%PROJ_DIR%\src\dependencies\openssl\include" && cmake --build . --config Release || goto :error
    cd "%PROJ_DIR%"
)

if not exist build (
    mkdir build || goto :error
)
cd build

if not exist ssh.dll (
    copy "%PROJ_DIR%\src\dependencies\libssh\build\src\Release\ssh.dll" . || goto :error
)

if not exist libcrypto-3-x64.dll (
    copy "%PROJ_DIR%\src\dependencies\openssl\libcrypto-3-x64.dll" . || goto :error
)

if not exist pthreadVC3.dll (
    copy "%PROJ_DIR%\src\dependencies\libssh\build\src\Release\pthreadVC3.dll" . || goto :error
)

cl -I"%PROJ_DIR%\src\dependencies\libssh\include" -I"%PROJ_DIR%\src\dependencies\libssh\build\include" "%PROJ_DIR%\src\torrent.c" "%PROJ_DIR%\src\dependencies\libssh\build\src\Release\ssh.lib" || goto :error
goto :EOF

:error
exit /b 1
