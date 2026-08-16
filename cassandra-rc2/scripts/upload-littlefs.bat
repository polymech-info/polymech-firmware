@echo off
setlocal enabledelayedexpansion

:: ESP32 LittleFS Upload Script (Windows)
:: Usage: scripts\upload-littlefs.bat [--env ENV] [--port PORT] [--auto]

:: Default values
set "ENV=waveshare"
set "PORT="
set "AUTO=false"
set "HELP=false"

:: Parse command line arguments
:parse_args
if "%~1"=="" goto :done_parsing
if "%~1"=="-e" (
    set "ENV=%~2"
    shift /1
    shift /1
    goto :parse_args
)
if "%~1"=="--env" (
    set "ENV=%~2"
    shift /1
    shift /1
    goto :parse_args
)
if "%~1"=="-p" (
    set "PORT=%~2"
    shift /1
    shift /1
    goto :parse_args
)
if "%~1"=="--port" (
    set "PORT=%~2"
    shift /1
    shift /1
    goto :parse_args
)
if "%~1"=="-a" (
    set "AUTO=true"
    shift /1
    goto :parse_args
)
if "%~1"=="--auto" (
    set "AUTO=true"
    shift /1
    goto :parse_args
)
if "%~1"=="-h" (
    set "HELP=true"
    shift /1
    goto :parse_args
)
if "%~1"=="--help" (
    set "HELP=true"
    shift /1
    goto :parse_args
)
echo Unknown option: %~1
echo Use --help for usage information
exit /b 1

:done_parsing

:: Show help
if "%HELP%"=="true" (
    echo ESP32 LittleFS Upload Script
    echo.
    echo Usage: %0 [OPTIONS]
    echo.
    echo Options:
    echo   -e, --env ENV     PlatformIO environment ^(default: waveshare^)
    echo   -p, --port PORT   Serial port ^(default: auto-detect^)
    echo   -a, --auto        Use PlatformIO-built LittleFS image
    echo   -h, --help        Show this help message
    echo.
    echo Examples:
    echo   %0 --auto                         # Upload PlatformIO-built LittleFS
    echo   %0 --auto --env waveshare         # Upload from specific environment
    echo   %0 --auto --port COM17            # Upload to specific port
    echo.
    exit /b 0
)

echo === ESP32 LittleFS Upload ===
echo Environment: %ENV%
if "%PORT%"=="" (
    echo Port: auto-detect
) else (
    echo Port: %PORT%
)
echo.

:: Build command
set "CMD=python scripts\upload_littlefs.py --env "%ENV%""

:: Add auto flag
if "%AUTO%"=="true" (
    set "CMD=!CMD! --auto"
)

:: Add port if specified
if not "%PORT%"=="" (
    set "CMD=!CMD! --port "%PORT%""
)

:: Execute upload
echo Command: !CMD!
echo.

!CMD!

if %errorlevel% equ 0 (
    echo.
    echo ✅ LittleFS upload completed successfully!
) else (
    echo.
    echo ❌ LittleFS upload failed!
    exit /b 1
) 