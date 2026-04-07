@echo off
chcp 65001 > nul
setlocal enabledelayedexpansion
title EasyDarwin Uninstallation tools
color 0A
powershell -Command "Write-Host '***************************** Uninstallation ***********************************' -ForegroundColor Green"

:: 替换原提权代码为以下内容（删除旧的 mshta 那行）
fltmc >nul 2>&1 || (
    echo  Please run the script as an administrator...
    powershell -Command "Start-Process cmd.exe -ArgumentList '/c ""%~s0""' -Verb RunAs"
    exit /b 1
)
cd /d "%~dp0"
endlocal
"%~dp0EasyDarwin.exe" stop
"%~dp0EasyDarwin.exe" uninstall

echo.
powershell -Command "Write-Host '***************************** Uninstallation ***********************************' -ForegroundColor Green"
pause
exit /b 0