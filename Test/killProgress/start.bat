@echo off
set num=100
for /l %%a in (1,1,%num%) do (
kill EasyRTSPClient.exe
)

pause