@echo off
echo === Setting up ESP32-S3 Upload Tools ===
echo.

echo Installing Python dependencies...
pip install esptool pyserial
if %errorlevel% neq 0 (
    echo Error: Failed to install Python packages
    pause
    exit /b 1
)

echo.
echo Checking for mklittlefs.exe...
if exist "mklittlefs.exe" (
    echo Found mklittlefs.exe in current directory
) else (
    echo mklittlefs.exe not found in current directory
    echo.
    echo Options to get mklittlefs.exe:
    echo 1. Install PlatformIO: pip install platformio
    echo 2. Download from: https://github.com/earlephilhower/mklittlefs/releases
    echo 3. Copy from PlatformIO packages if already installed
    echo.
    
    if exist "%USERPROFILE%\.platformio\packages\tool-mklittlefs\mklittlefs.exe" (
        echo Found PlatformIO version, copying...
        copy "%USERPROFILE%\.platformio\packages\tool-mklittlefs\mklittlefs.exe" .
        echo Copied mklittlefs.exe
    ) else (
        echo PlatformIO version not found
    )
)

echo.
echo Testing esptool installation...
python -m esptool version
if %errorlevel% neq 0 (
    echo Error: esptool test failed
    pause
    exit /b 1
)

echo.
echo === Setup Complete ===
echo.
echo Available upload scripts:
echo - scripts\upload.bat          : Upload firmware
echo - scripts\upload_web.bat      : Upload web files (LittleFS)
echo - scripts\upload_firmware.py  : Advanced firmware upload
echo - scripts\upload_littlefs.py  : Advanced LittleFS upload
echo.
echo See scripts\README-upload.md for detailed documentation

pause 