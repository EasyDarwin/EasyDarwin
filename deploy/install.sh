#!/bin/bash
clear

# ================================ 配置 ================================
FILE_PATH="configs/config.toml"
SERVICE_NAME="EasyDarwin"
INSTALL_CMD="./EasyDarwin install"
START_CMD="./EasyDarwin start"

testNum=0
passedPorts=()

# ======================== 颜色定义 ========================
RED="\033[1;31m"
GREEN="\033[1;32m"
YELLOW="\033[1;33m"
CLEAR="\033[0m"

# ======================== 输出函数 ========================
ok() {
    echo -e "${GREEN}[OK] $1${CLEAR}"
}

no() {
    echo -e "${RED}[ERROR] $1${CLEAR}"
}

warn() {
    echo -e "${YELLOW}[WARNING] $1${CLEAR}"
}

green() {
    echo -e "${GREEN}$1${CLEAR}"
}

red() {
    echo -e "${RED}$1${CLEAR}"
}

# ================================ 检查 root 权限 ================================
if [ $UID -ne 0 ]; then
    warn "Please run this script as root!"
    exec sudo "$0" "$@"
    exit 1
fi

cd "$(dirname "$0")" || exit 1

# ================================ 检查服务是否已运行 ================================
if systemctl is-active --quiet "$SERVICE_NAME"; then
    echo ""
    warn "$SERVICE_NAME Service is already running."
    echo ""
    read -p "Press Enter to exit..."
    exit 1
fi

# ================================ 环境检查标题 ================================
echo ""
green " ================================ Environment Checking ================================"
echo ""

if [ ! -f "$FILE_PATH" ]; then
    warn "File $FILE_PATH not found. Will check default ports..."
    echo ""
fi

green "+------------------------------------------------+---------------------------------+"
green " [Port Status]                              [Port Number]      [PID]      [Name]"
green "+------------------------------------------------+---------------------------------+"

# ======================== 端口检测函数（带颜色） ========================
check_port() {
    local port="$1"
    local desc="$2"
    local pid=$(lsof -i :"$port" -sTCP:LISTEN -t 2>/dev/null)

    if [ -n "$pid" ]; then
        local pname=$(ps -p "$pid" -o comm= 2>/dev/null)
        echo -e "|  ${RED}[ NO ]${CLEAR}  |  Checking $desc |port:$port|     |$pid| $pname"
        testNum=$((testNum + 1))

        echo -ne "${RED}This port is occupied. Force kill? (Y/N) ${CLEAR}"
        read -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            kill -9 "$pid" >/dev/null 2>&1
            sleep 0.5
            green "Killed PID $pid"
            check_port "$port" "$desc"
            return
        fi
    else
        echo -e "|  ${GREEN}[ OK ]${CLEAR}  |  Checking $desc |port:$port|"
        passedPorts+=("$port")
    fi
}

check_udp_port() {
    local port="$1"
    local desc="$2"
    local pid=$(ss -uln sport = :"$port" | grep -oP 'pid=\K\d+' 2>/dev/null | head -n1)

    if [ -n "$pid" ]; then
        local pname=$(ps -p "$pid" -o comm= 2>/dev/null)
        echo -e "|  ${RED}[ NO ]${CLEAR}  |  Checking $desc |port:$port|     |$pid| $pname"
        testNum=$((testNum + 1))

        echo -ne "${RED}This UDP port is occupied. Force kill? (Y/N) ${CLEAR}"
        read -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            kill -9 "$pid" >/dev/null 2>&1
            sleep 0.5
            green "Killed PID $pid"
            check_udp_port "$port" "$desc"
            return
        fi
    else
        echo -e "|  ${GREEN}[ OK ]${CLEAR}  |  Checking $desc |port:$port|"
        passedPorts+=("$port")
    fi
}

# ======================== 读取配置文件 ========================
if [ -f "$FILE_PATH" ]; then
    portNum=0
    portsNum=0
    addrNum=0

    while IFS= read -r line || [ -n "$line" ]; do
        line=$(echo "$line" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
        if [[ -z "$line" || "$line" == \#* ]]; then
            continue
        fi

        key=$(echo "$line" | cut -d'=' -f1 | sed 's/ //g')
        value=$(echo "$line" | cut -d'=' -f2- | sed 's/[^0-9]//g')

        if ! [[ "$value" =~ ^[0-9]+$ ]]; then
            continue
        fi

        if [[ "$key" == "httplistenaddr" ]]; then
            portNum=$((portNum + 1))
            if [ "$portNum" -eq 1 ]; then
                if [ "$value" -eq 0 ] 2>/dev/null; then
                    no "HTTP Port config error: cannot be 0!"
                    exit 1
                fi
                check_port "$value" "HTTP(TCP)           "
            fi

        elif [[ "$key" == "httpslistenaddr" ]]; then
            portsNum=$((portsNum + 1))
            if [ "$portsNum" -eq 1 ]; then
                if [ "$value" -eq 0 ] 2>/dev/null; then
                    no "HTTPS Port config error: cannot be 0!"
                    exit 1
                fi
                check_port "$value" "HTTPS(TCP)          "
            fi

        elif [[ "$key" == "icetcpmuxport" ]]; then
            if [ "$value" -eq 0 ] 2>/dev/null; then
                no "WebRTC Port config error: cannot be 0!"
                exit 1
            fi
            check_port "$value" "WebRTC(TCP)         "
            check_udp_port "$value" "WebRTC(UDP)         "

        elif [[ "$key" == "addr" ]]; then
            addrNum=$((addrNum + 1))
            if [ "$addrNum" -eq 1 ]; then
                if [ "$value" -eq 0 ] 2>/dev/null; then
                    no "RTMP Port config error: cannot be 0!"
                    exit 1
                fi
                check_port "$value" "RTMP(TCP)           "
            elif [ "$addrNum" -eq 2 ]; then
                if [ "$value" -eq 0 ] 2>/dev/null; then
                    no "RTSP Port config error: cannot be 0!"
                    exit 1
                fi
                check_port "$value" "RTSP(TCP)           "
            fi
        fi

    done < "$FILE_PATH"
else
    check_port 10086   "HTTP(TCP)           "
    check_port 24434   "HTTPS(TCP)          "
    check_port 21935   "RTMP(TCP)           "
    check_port 15544   "RTSP(TCP)           "
    check_port 24888   "WebRTC(TCP)         "
    check_udp_port 24888 "WebRTC(UDP)         "
fi

green "+------------------------------------------------+---------------------------------+"
echo ""

# ================================ 端口判断 ================================
if [ "$testNum" -gt 0 ]; then
    echo ""
    red "  One or more ports are not available. Modify config.toml and try again."
    echo ""

    red " -----------------------"
    red " |   _   _  ___    _   |"
    red " |  | \ | |/ _ \  | |  |"
    red " |  |  \| | | | | | |  |"
    red " |  | |\  | |_| | |_|  |"
    red " |  |_| \_|\___/  (_)  |"
    red " |                     |"
    red " -----------------------"
    echo ""
    red " ------------------- Program startup failed -------------------"
    echo ""
    read -p "Press Enter to exit..."
    exit 1
fi

# ================================ 安装服务 ================================
green "Installing $SERVICE_NAME..."
$INSTALL_CMD >/dev/null 2>&1

if systemctl is-enabled --quiet "$SERVICE_NAME"; then
    ok "$SERVICE_NAME installed successfully."
else
    no "$SERVICE_NAME install failed."
    exit 1
fi

# ================================ 启动服务 ================================
green "Starting $SERVICE_NAME..."
$START_CMD >/dev/null 2>&1
sleep 3

if systemctl is-active --quiet "$SERVICE_NAME"; then
    ok "$SERVICE_NAME started successfully."
else
    no "$SERVICE_NAME start failed."
    exit 1
fi

echo ""

# ================================ 成功界面 ================================
green " ================================ Environment Checking ================================"
echo ""
green " -----------------------------"
green " |  __   _______ ____    _   |"
green " |  \ \ / / ____/ ___|  | |  |"
green " |   \ V /|  _| \___ \  | |  |"
green " |    | | | |___ ___) | |_|  |"
green " |    |_| |_____|____/  (_)  |"
green " |                           |"
green " -----------------------------"
echo ""
green " ------------------- Program startup SUCCESS -------------------"
echo ""
read -p "Press Enter to exit..."
exit 0