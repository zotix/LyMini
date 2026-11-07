@echo off
setlocal
echo ============================================================
echo Compiling and Uploading Spotpear ESP32-C3 MiniGames Arcade
echo Target: ESP32-C3 Dev Module (ST7735 128x128 LCD)
echo ============================================================

set ARDUINO_CLI="C:\Users\billg\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
set FQBN=esp32:esp32:esp32c3:CDCOnBoot=cdc,PartitionScheme=huge_app
set SKETCH_DIR=%~dp0MiniGames.ino
set PORT=%1
if "%PORT%"=="" set PORT=COM7

echo [1/2] Compiling Sketch...
%ARDUINO_CLI% compile --fqbn %FQBN% "%SKETCH_DIR%"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [2/2] Uploading to %PORT%...
%ARDUINO_CLI% upload -p %PORT% --fqbn %FQBN% "%SKETCH_DIR%"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Upload failed! Make sure device is connected at %PORT%
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [SUCCESS] Firmware uploaded successfully!
echo Access Web Portal at: http://mini.local or connect to AP "Spotpear-Mini"
pause
