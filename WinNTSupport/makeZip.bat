
IF EXIST DarwinStreamingServer ( RMDIR /Q /S DarwinStreamingServer) 
MKDIR DarwinStreamingServer

IF EXIST build ( RMDIR /Q /S build )
MKDIR build 

DEL /Q dynmodules_disabled\*.dll

IF option%1 EQU optiondebug ( 
 copy /Y Debug\*.exe build\
 copy /Y Debug\*.dll build\
 copy /Y dynmodules_disabled\Debug\*.dll dynmodules_disabled\
) ELSE (
 copy /Y Release\*.exe build\
 copy /Y Release\*.dll build\
 copy /Y dynmodules_disabled\Release\*.dll dynmodules_disabled\
)


copy build\DarwinStreamingServer.exe DarwinStreamingServer\DarwinStreamingServer.exe
copy build\RegistrySystemPathEditor.exe DarwinStreamingServer\RegistrySystemPathEditor.exe
copy build\StreamingLoadTool.exe DarwinStreamingServer\StreamingLoadTool.exe
copy build\MP3Broadcaster.exe DarwinStreamingServer\MP3Broadcaster.exe
copy build\PlaylistBroadcaster.exe DarwinStreamingServer\PlaylistBroadcaster.exe
copy build\qtpasswd.exe DarwinStreamingServer\qtpasswd.exe

copy streamingserver.xml DarwinStreamingServer\streamingserver.xml
copy relayconfig.xml-Sample DarwinStreamingServer\relayconfig.xml-Sample
copy qtusers DarwinStreamingServer\qtusers
copy ..\qtgroups DarwinStreamingServer\qtgroups
copy ..\WebAdmin\src\streamingadminserver.pl DarwinStreamingServer\streamingadminserver.pl
copy ..\StreamingLoadTool\streamingloadtool.cfg DarwinStreamingServer\streamingloadtool.cfg
copy WinPasswdAssistant.pl DarwinStreamingServer\WinPasswdAssistant.pl
copy Install.bat DarwinStreamingServer\Install.bat

copy ..\sample_100kbit.mov DarwinStreamingServer\sample_100kbit.mov
copy ..\sample_300kbit.mov DarwinStreamingServer\sample_300kbit.mov
copy ..\sample_100kbit.mp4 DarwinStreamingServer\sample_100kbit.mp4
copy ..\sample_300kbit.mp4 DarwinStreamingServer\sample_300kbit.mp4
copy ..\sample.mp3 DarwinStreamingServer\sample.mp3
copy ..\sample_50kbit.3gp DarwinStreamingServer\sample_50kbit.3gp
copy ..\sample_h264_1mbit.mp4 DarwinStreamingServer\sample_h264_1mbit.mp4
copy ..\sample_h264_100kbit.mp4 DarwinStreamingServer\sample_h264_100kbit.mp4
copy ..\sample_h264_300kbit.mp43 DarwinStreamingServer\sample_h264_300kbit.mp4

copy dynmodules_disabled\QTSSSpamDefenseModule.dll DarwinStreamingServer\QTSSSpamDefenseModule.dll
copy dynmodules_disabled\QTSSRawFileModule.dll DarwinStreamingServer\QTSSRawFileModule.dll
copy build\QTSSRefMovieModule.dll DarwinStreamingServer\QTSSRefMovieModule.dll

copy ..\Documentation\ReadMe.rtf DarwinStreamingServer\ReadMe.rtf

mkdir DarwinStreamingServer\AdminHtml
copy ..\WebAdmin\WebAdminHtml\*.pl DarwinStreamingServer\AdminHtml
copy ..\WebAdmin\WebAdminHtml\*.cgi DarwinStreamingServer\AdminHtml
copy ..\WebAdmin\WebAdminHtml\*.html DarwinStreamingServer\AdminHtml

mkdir DarwinStreamingServer\AdminHtml\images
copy ..\WebAdmin\WebAdminHtml\images\*.gif DarwinStreamingServer\AdminHtml\images

mkdir DarwinStreamingServer\AdminHtml\includes
copy ..\WebAdmin\WebAdminHtml\includes\*.js DarwinStreamingServer\AdminHtml\includes

mkdir DarwinStreamingServer\AdminHtml\html_en
copy ..\WebAdmin\WebAdminHtml\html_en\messages DarwinStreamingServer\AdminHtml\html_en
copy ..\WebAdmin\WebAdminHtml\html_en\genres DarwinStreamingServer\AdminHtml\html_en
