#!/bin/bash

# ANSI escape codes
RED='\033[31m'
GREEN='\033[32m'
YELLOW='\033[1;33m'
NC='\033[0m' # Reset color

echo -e "${GREEN}***************************** Uninstallation ***********************************${NC}"
echo

# Check if the user is root
if [ "$(id -u)" -eq 0 ]; then
    echo -e "  ${GREEN}****** The user is the root user, continue execution. ******${NC}"
else
    # Check if the user has sudo privileges
    if sudo -n true 2>/dev/null; then
        echo -e "  ${GREEN}****** The current user has sudo privileges, continue execution. ******${NC}"
        # Execute the script itself with sudo
        sudo "$0"
        exit 0
    else
        echo -e "  ${GREEN}****** Use the following method to execute the uninstall.sh script. ******${NC}"
        echo -e "  ${GREEN}****** 1.Switch to root user and run ./uninstall.sh ******${NC}"
        echo -e "  ${GREEN}****** 2.Try running sudo ./uninstall.sh command. ******${NC}"
        echo -e "  ${GREEN}****** If a password input box pops up, enter the root password before attempting to execute the script ******${NC}"
        su
        exit 1
    fi
fi


"$PWD/EasyDarwin" stop     
"$PWD/EasyDarwin" uninstall


echo -e "${GREEN}***************************** Uninstallation ***********************************${NC}"
echo