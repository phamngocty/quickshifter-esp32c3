@echo off
chcp 65001 >nul
echo ========================================================
echo   TOOL NẠP NHANH FIRMWARE & GIAO DIỆN QUICKSHIFTER PRO
echo                 ESP32-C3 SUPERMINI
echo ========================================================
echo.
set /p COM_PORT="Nhap cong COM (vi du: COM3, COM4): "
if "%COM_PORT%"=="" set COM_PORT=COM3

echo Dang nap Firmware va Giao Dien vao ESP32-C3 tren %COM_PORT%...
python -m esptool --chip esp32c3 --port %COM_PORT% --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 binaries\bootloader.bin 0x8000 binaries\partitions.bin 0x10000 binaries\firmware.bin 0x270000 binaries\littlefs.bin

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [THANH CONG] Da nap xong toan bo Firmware va Giao Dien Web LittleFS!
) else (
    echo.
    echo [THAT BAI] Vui long kiem tra lai cong COM va ket noi day USB!
)
pause