@echo off
setlocal enabledelayedexpansion

echo === ESP32-S3 Firmware Upload Tool ===
echo.

:: Check if firmware file exists
if not exist ".pio\build\waveshare\firmware.bin" (
    echo Error: Firmware not found. Build first with: npm run build
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
echo Firmware: .pio\build\waveshare\firmware.bin
echo.

:: Upload firmware with auto-detection
python scripts\upload_firmware.py --port %PORT% --auto

if %errorlevel% equ 0 (
    echo.
    echo === Upload Complete ===
) else (
    echo.
    echo === Upload Failed ===
)

pause 