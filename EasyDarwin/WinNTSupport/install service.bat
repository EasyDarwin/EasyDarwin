cd ./
set curPath=%~dp0
set exePath=%~dp0\EasyDarwin.exe
set xmlPath=%~dp0\easydarwin.xml
echo service path£º%curPath%
sc create EasyDarwin binPath= "\"%exePath%\" -c \"%xmlPath%\"" start= auto
sc failure EasyDarwin reset= 0 actions= restart/0
sc config EasyDarwin type= interact type= own
net start EasyDarwin
pause