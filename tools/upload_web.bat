@echo off
setlocal enabledelayedexpansion

echo === ESP32-S3 Web Files Upload Tool ===
echo.

:: Check if data directory exists
if not exist "data" (
    echo Error: Data directory not found. Run: npm run web:build-dist && npm run web:sync
    pause
    exit /b 1
)

:: Auto-detect COM port or use default
set "PORT=COM17"
for /f "tokens=1" %%i in ('python -c "import serial.tools.list_ports; ports=serial.tools.list_ports.comports(); [print(p.device) for p in ports if 'USB' in p.description]" 2>nul') do (
    set "PORT=%%i"
    goto :found_port
)
:found_port

echo Using port: %PORT%
echo Data directory: data\
echo.

:: Build and upload LittleFS
python scripts\upload_littlefs.py --port %PORT% --data-dir data

if %errorlevel% equ 0 (
    echo.
    echo === Web Files Upload Complete ===
) else (
    echo.
    echo === Web Files Upload Failed ===
)

pause 