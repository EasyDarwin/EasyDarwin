cd ./
set curPath=%~dp0
set exePath=%~dp0\EasyCMS.exe
set xmlPath=%~dp0\easycms.xml
echo service path£º%curPath%
sc create EasyCMS binPath= "\"%exePath%\" -c \"%xmlPath%\"" start= auto
sc failure EasyCMS reset= 0 actions= restart/0
sc config EasyCMS type= interact type= own
net start EasyCMS
pause