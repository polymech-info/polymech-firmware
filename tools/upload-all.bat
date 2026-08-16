@echo off
setlocal enabledelayedexpansion

:: ESP32 Firmware & LittleFS Upload Script (Windows)
:: Usage: scripts\upload-all.bat [--env ENV] [--port PORT] [--fs]

:: Default values
set "ENV=waveshare"
set "PORT="
set "UPLOAD_FS=true"
set "FIRMWARE_ONLY=false"
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
if "%~1"=="-f" (
    set "FIRMWARE_ONLY=true"
    set "UPLOAD_FS=false"
    shift /1
    goto :parse_args
)
if "%~1"=="--firmware-only" (
    set "FIRMWARE_ONLY=true"
    set "UPLOAD_FS=false"
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
    echo ESP32 Complete Upload Script
    echo.
    echo Usage: %0 [OPTIONS]
    echo.
    echo Options:
    echo   -e, --env ENV         PlatformIO environment ^(default: waveshare^)
    echo   -p, --port PORT       Serial port ^(default: auto-detect^)
    echo   -f, --firmware-only   Upload firmware only ^(skip LittleFS^)
    echo   -h, --help            Show this help message
    echo.
    echo Examples:
    echo   %0                                # Upload firmware + LittleFS ^(default^)
    echo   %0 --firmware-only                # Upload firmware only
    echo   %0 --env waveshare-release        # Upload release build + LittleFS
    echo   %0 --port COM17                   # Upload both to specific port
    echo   %0 --env waveshare-release --port COM17  # All options
    echo.
    exit /b 0
)

echo === ESP32 Complete Upload Tool ===
echo Environment: %ENV%
if "%PORT%"=="" (
    echo Port: auto-detect
) else (
    echo Port: %PORT%
)
if "%FIRMWARE_ONLY%"=="true" (
    echo Upload: Firmware only
) else (
    echo Upload: Firmware + LittleFS ^(complete^)
)
echo.

:: Build file paths
set "BUILD_DIR=.pio\build\%ENV%"
set "FIRMWARE=%BUILD_DIR%\firmware.bin"
set "BOOTLOADER=%BUILD_DIR%\bootloader.bin"
set "PARTITIONS=%BUILD_DIR%\partitions.bin"

:: Check if firmware exists
if not exist "%FIRMWARE%" (
    echo ❌ Firmware not found: %FIRMWARE%
    echo Run: npm run build ^(or pio run -e %ENV%^)
    exit /b 1
)

echo ✅ Found firmware files in %BUILD_DIR%

:: Build command
set "CMD=python scripts\upload_firmware.py --firmware "%FIRMWARE%""

:: Add optional files if they exist
if exist "%BOOTLOADER%" (
    set "CMD=!CMD! --bootloader "%BOOTLOADER%""
)

if exist "%PARTITIONS%" (
    set "CMD=!CMD! --partitions "%PARTITIONS%""
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
    echo ✅ Firmware upload completed successfully!
    
    :: Upload LittleFS if requested
    if "%UPLOAD_FS%"=="true" (
        echo.
        echo === Uploading LittleFS ===
        
        :: Build LittleFS command
        set "FS_CMD=python scripts\upload_littlefs.py --auto --env "%ENV%""
        
        :: Add port if specified
        if not "%PORT%"=="" (
            set "FS_CMD=!FS_CMD! --port "%PORT%""
        )
        
        echo Command: !FS_CMD!
        echo.
        
        !FS_CMD!
        
        if !errorlevel! equ 0 (
            echo.
            echo ✅ LittleFS upload completed successfully!
            echo 🎉 All uploads completed!
        ) else (
            echo.
            echo ❌ LittleFS upload failed!
            exit /b 1
        )
    )
) else (
    echo.
    echo ❌ Firmware upload failed!
    exit /b 1
) 