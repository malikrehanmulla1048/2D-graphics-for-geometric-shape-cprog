@echo off
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

REM Close any existing instance so the executable can be rebuilt.
taskkill /F /IM graphics_editor.exe >nul 2>&1

where gcc >nul 2>nul
if errorlevel 1 (
    echo.
    echo GCC was not found on PATH.
    echo Install MinGW/MSYS2 GCC or add gcc to your PATH, then rerun this file.
    pause
    exit /b 1
)

gcc graphics_editor.c -lpdcurses -o graphics_editor.exe
if errorlevel 1 (
    echo.
    echo Build failed. Make sure MinGW/MSYS2 GCC and PDCurses are installed.
    pause
    exit /b %errorlevel%
)

echo.
echo Starting graphics_editor.exe...
"%~dp0graphics_editor.exe"
if errorlevel 1 (
    echo.
    echo The program exited with an error.
    pause
)

endlocal
