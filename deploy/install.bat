@echo off
chcp 65001 > nul
mode con: cols=110 lines=40
setlocal enabledelayedexpansion
title  EasyDarwin Installation tools
color 0A

:: 管理员权限自动提权
fltmc >nul 2>&1 || (
    echo  Please run the script as an administrator...
    powershell -Command "Start-Process cmd.exe -ArgumentList '/c ""%~s0""' -Verb RunAs"
    exit /b 1
)
cd /d "%~dp0"

:: 配置文件路径
set "FILE_PATH=configs\config.toml"

:: 检查服务是否已运行
sc query EasyDarwin >nul 2>&1
if %errorlevel% equ 0 (
    sc query EasyDarwin | find "RUNNING" >nul
    if %errorlevel% equ 0 (
        echo.
        echo [WARNING] EasyDarwin Service is already installed and running.
        echo.
        pause
        exit /b 1
    )
)

echo.
echo  ================================ Environment Checking ================================
echo.

if not exist "%FILE_PATH%" (
    echo   file %FILE_PATH% not found.Will be checking the default port status..
    echo.
)

echo +------------------------------------------------+---------------------------------+
echo [Port Status]                              [Port Number]      [PID]      [name]
echo +------------------------------------------------+---------------------------------+

set "FILE_EXTENSION=1"
if not exist "%FILE_PATH%" set "FILE_EXTENSION=0"

:: 初始化变量
set "valueCount=0"
set "testNum=0"
set "portNum=0"
set "portsNum=0"
set "addrNum=0"
set "passedPorts="

:: 默认端口列表
set "valueCount=5"
set "values[1]=10086"
set "values[2]=24434"
set "values[3]=21935"
set "values[4]=15544"
set "values[5]=24888"

if !FILE_EXTENSION! equ 1 (
    set "valueCount=0"
    for /f "delims=" %%L in (%FILE_PATH%) do (
        set "line=%%L"
        set "line=!line: =!"
        if "!line:~0,1!"=="#" (
            rem 注释行跳过
        ) else (
            echo !line! | findstr /i "httplistenaddr httpslistenaddr addr icetcpmuxport" >nul 2>&1
            if not errorlevel 1 (
                for /f "tokens=1,2 delims==" %%A in ("!line!") do (
                    set "key=%%A"
                    set "value=%%B"
                    :: 清理键值
                    set "key=!key: =!"
                    set "key=!key:'=!"
                    set "value=!value: =!"
                    set "value=!value:'=!"
                    set "value=!value::=!"
                    :: 清理首尾空白，确保纯数字
                    for /f "tokens=* delims= " %%v in ("!value!") do set "value=%%v"

                    :: ================== 端口校验 ==================
                    if "!key!"=="httplistenaddr" (
                        set /a portNum+=1
                        if !portNum! equ 1 (
                            if "!value!"=="0" (
                                echo HTTP Port config error: This value cannot be 0!
                                goto :NoImage
                            )
                            echo !value!|findstr /r "^[0-9][0-9]*$" >nul
                            if errorlevel 1 (
                                echo HTTP Port config error: This value should be number---^(!value!^)
                                goto :NoImage
                            )
                            call :check_port "!value!" "HTTP(TCP)           "
                        )
                    ) else if "!key!"=="httpslistenaddr" (
                        set /a portsNum+=1
                        if !portsNum! equ 1 (
                            if "!value!"=="0" (
                                echo HTTPS Port config error: This value cannot be 0!
                                goto :NoImage
                            )
                            echo !value!|findstr /r "^[0-9][0-9]*$" >nul
                            if errorlevel 1 (
                                echo HTTPS Port config error: This value should be number---^(!value!^)
                                goto :NoImage
                            )
                            call :check_port "!value!" "HTTPS(TCP)          "
                        )
                    ) else if "!key!"=="icetcpmuxport" (
                        if "!value!"=="0" (
                            echo WebRTC Port config error: This value cannot be 0!
                            goto :NoImage
                        )
                        echo !value!|findstr /r "^[0-9][0-9]*$" >nul
                        if errorlevel 1 (
                            echo WebRTC Port config error: This value should be number---^(!value!^)
                            goto :NoImage
                        )
                        call :check_port "!value!" "WebRTC(TCP)         "
                        call :check_udp_port "!value!" "WebRTC(UDP)         "
                    ) else if "!key!"=="addr" (
                        set /a addrNum+=1
                        if !addrNum! equ 1 (
                            if "!value!"=="0" (
                                echo RTMP Port config error: This value cannot be 0!
                                goto :NoImage
                            )
                            echo !value!|findstr /r "^[0-9][0-9]*$" >nul
                            if errorlevel 1 (
                                echo RTMP Port config error: This value should be number---^(!value!^)
                                goto :NoImage
                            )
                            call :check_port "!value!" "RTMP(TCP)           "
                        )
                        if !addrNum! equ 2 (
                            if "!value!"=="0" (
                                echo RTSP Port config error: This value cannot be 0!
                                goto :NoImage
                            )
                            echo !value!|findstr /r "^[0-9][0-9]*$" >nul
                            if errorlevel 1 (
                                echo RTSP Port config error: This value should be number---^(!value!^)
                                goto :NoImage
                            )
                            call :check_port "!value!" "RTSP(TCP)           "
                        )
                    )
                )
            )
        )
    )
)

:: 检测默认端口
for /l %%i in (1,1,%valueCount%) do (
    if %%i equ 1 (
        call :check_port "!values[%%i]!" "HTTP(TCP)           "
    ) else if %%i equ 2 (
        call :check_port "!values[%%i]!" "HTTPS(TCP)          "
    ) else if %%i equ 3 (
        call :check_port "!values[%%i]!" "RTMP(TCP)           "
    ) else if %%i equ 4 (
        call :check_port "!values[%%i]!" "RTSP(TCP)           "
    ) else if %%i equ 5 (
        call :check_port "!values[%%i]!" "WebRTC(TCP)         "
        call :check_udp_port "!values[%%i]!" "WebRTC(UDP)         "
    )
)

echo +------------------------------------------------+---------------------------------+
echo.

:: 最终端口校验
set "allPortsFree=1"
for %%p in (%passedPorts%) do (
    set "testFlag="
    call :check_port "%%p" "Final check" >nul 2>&1
    if !testNum! gtr 0 set "allPortsFree=0"
)

if %testNum% gtr 0 (
    echo.
    echo   One or more ports are not available. Modify config.toml to change ports and start again...
    echo.
    goto :NoImage
)

:: 安装服务
"%~dp0EasyDarwin.exe" install >nul 2>&1
sc query EasyDarwin >nul 2>&1
if !errorlevel! equ 0 (
    echo [OK] EasyDarwin Service is installed.
) else (
    echo [FALSE] EasyDarwin Service install failed.
    goto :NoImage
)

:: 启动服务
"%~dp0EasyDarwin.exe" start >nul 2>&1
timeout /t 3 >nul
sc query EasyDarwin | find "RUNNING" >nul
if !errorlevel! equ 0 (
    echo [OK] EasyDarwin Service successfully started.
) else (
    echo [FALSE] EasyDarwin Service failed to start.
    pause
    exit /b 1
)
echo.

goto :YesImage

:: ======================== 端口检测函数 ========================
:check_port
set "port=%~1"
set "desc=%~2"
:check_port_loop
for /f "tokens=5 delims= " %%P in ('netstat -ano ^| findstr ":!port! "') do (
    set "pid=%%P"
    if not "!pid!"=="" if not "!pid!"=="0" (
        for /f "skip=2 tokens=2 delims=," %%a in ('tasklist /fi "PID eq !pid!" /fo csv /nh') do (
            set "processName=%%~a"
        )
        echo ^|  [ NO ]  ^|  Checking !desc! ^|port:!port!^|     ^|!pid!^| !processName!
        set /a testNum+=1

        set /p="This port is occupied. Force to kill? (Y/N) "
        choice /C YN /N >nul
        if errorlevel 2 (
            echo [NO]
            goto :eof
        ) else (
            taskkill /F /pid !pid! >nul 2>&1
            echo [Done]
            set /a testNum=0
            goto :check_port_loop
        )
    )
)
echo ^|  [ OK ]  ^|  Checking %desc% ^|port:%port%^|
set "passedPorts=!passedPorts! %port%"
goto :eof

:check_udp_port
set "port=%~1"
set "desc=%~2"
:check_udp_port_loop
for /f "tokens=4 delims= " %%P in ('netstat -ano -p UDP ^| findstr ":!port! "') do (
    set "pid=%%P"
    if not "!pid!"=="" if not "!pid!"=="0" (
        for /f "skip=2 tokens=2 delims=," %%a in ('tasklist /fi "PID eq !pid!" /fo csv /nh') do (
            set "processName=%%~a"
        )
        echo ^|  [ NO ]  ^|  Checking !desc! ^|port:!port!^|     ^|!pid!^| !processName!
        set /a testNum+=1

        set /p="This UDP port is occupied. Force to kill? (Y/N) "
        choice /C YN /N >nul
        if errorlevel 2 (
            echo [NO]
            goto :eof
        ) else (
            taskkill /F /pid !pid! >nul 2>&1
            echo [Done]
            set /a testNum=0
            goto :check_udp_port_loop
        )
    )
)
echo ^|  [ OK ]  ^|  Checking %desc% ^|port:%port%^|
set "passedPorts=!passedPorts! %port%"
goto :eof

:: ======================== 失败界面 ========================
:NoImage
echo  ================================ Environment Checking ================================
echo.
echo                                -----------------------
echo                                ^|   _   _  ___    _   ^|
echo                                ^|  ^| \ ^| ^|/ _ \  ^| ^|  ^|
echo                                ^|  ^|  \^| ^| ^| ^| ^| ^| ^|  ^|
echo                                ^|  ^| ^|\  ^| ^|_^| ^| ^|_^|  ^|
echo                                ^|  ^|_^| \_^|\___/  (_^)  ^|
echo                                ^|                     ^|
echo                                -----------------------
echo.
echo  -------------------------------- Program startup failed -------------------------------
echo.
pause
exit /b 1

:: ======================== 成功界面 ========================
:YesImage
echo  ================================ Environment Checking ================================
echo.
echo                                -----------------------------
echo                                ^|  __   _______ ____    _   ^|
echo                                ^|  \ \ / / ____/ ___^|  ^| ^|  ^|
echo                                ^|   \ V /^|  _^| \___ \  ^| ^|  ^|
echo                                ^|    ^| ^| ^| ^|___ ___) ^| ^|_^|  ^|
echo                                ^|    ^|_^| ^|_____^|____/  (_)  ^|
echo                                ^|                           ^|
echo                                -----------------------------
echo.
echo  -------------------------------- Program startup SUCCESS -------------------------------
echo.
pause
exit /b 0