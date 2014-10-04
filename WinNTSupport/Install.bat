echo off

echo -----
echo Backing up old config, qtusers, and qtgroups files
move "c:\Program Files\Darwin Streaming Server\qtusers" "c:\Program Files\Darwin Streaming Server\qtusers.backup"
move "c:\Program Files\Darwin Streaming Server\qtgroups" "c:\Program Files\Darwin Streaming Server\qtgroups.backup"
move "c:\Program Files\Darwin Streaming Server\streamingserver.xml" "c:\Program Files\Darwin Streaming Server\streamingserver.xml.backup"
move "c:\Program Files\Darwin Streaming Server\relayconfig.xml" "c:\Program Files\Darwin Streaming Server\relayconfig.xml.backup"
move "c:\Program Files\Darwin Streaming Server\streamingadminserver.pem" "c:\Program Files\Darwin Streaming Server\streamingadminserver.pem.backup"
move "c:\Program Files\Darwin Streaming Server\streamingadminserver.conf" "c:\Program Files\Darwin Streaming Server\streamingadminserver.conf.backup"
move "c:\Program Files\Darwin Streaming Server\streamingloadtool.cfg" "c:\Program Files\Darwin Streaming Server\streamingloadtool.cfg.backup"

echo -----
echo Creating c:\Program Files\Darwin Streaming Server\ directory
mkdir "c:\Program Files\Darwin Streaming Server\"

echo -----
echo Copying files
copy DarwinStreamingServer.exe "c:\Program Files\Darwin Streaming Server\DarwinStreamingServer.exe"
copy streamingserver.xml "c:\Program Files\Darwin Streaming Server\streamingserver.xml-sample"
REM copy qtusers "c:\Program Files\Darwin Streaming Server\qtusers"
REM copy qtgroups "c:\Program Files\Darwin Streaming Server\qtgroups"
copy relayconfig.xml-Sample  "c:\Program Files\Darwin Streaming Server\relayconfig.xml-Sample"
copy PlaylistBroadcaster.exe  "c:\Program Files\Darwin Streaming Server\PlaylistBroadcaster.exe"
copy MP3Broadcaster.exe  "c:\Program Files\Darwin Streaming Server\MP3Broadcaster.exe"
copy streamingadminserver.pl "c:\Program Files\Darwin Streaming Server\streamingadminserver.pl"
REM copy streamingadminserver.pem "c:\Program Files\Darwin Streaming Server\streamingadminserver.pem"
copy qtpasswd.exe  "c:\Program Files\Darwin Streaming Server\qtpasswd.exe"
copy StreamingLoadTool.exe  "c:\Program Files\Darwin Streaming Server\StreamingLoadTool.exe"
copy streamingloadtool.cfg  "c:\Program Files\Darwin Streaming Server\streamingloadtool.cfg"
copy WinPasswdAssistant.pl "c:\Program Files\Darwin Streaming Server\WinPasswdAssistant.pl"

mkdir  "c:\Program Files\Darwin Streaming Server\QTSSModules\"
copy QTSSRefMovieModule.dll "c:\Program Files\Darwin Streaming Server\QTSSModules\QTSSRefMovieModule.dll"
REM copy QTSSRawFileModule.dll "c:\Program Files\Darwin Streaming Server\QTSSModules\QTSSRawFileModule.dll"
REM copy QTSSSpamDefenseModule.dll "c:\Program Files\Darwin Streaming Server\QTSSModules\QTSSSpamDefenseModule.dll"

mkdir  "c:\Program Files\Darwin Streaming Server\Playlists\"

mkdir  "c:\Program Files\Darwin Streaming Server\Movies\"
copy sample_100kbit.mov "c:\Program Files\Darwin Streaming Server\Movies\sample_100kbit.mov"
copy sample_300kbit.mov "c:\Program Files\Darwin Streaming Server\Movies\sample_300kbit.mov"
copy sample_100kbit.mp4 "c:\Program Files\Darwin Streaming Server\Movies\sample_100kbit.mp4"
copy sample_300kbit.mp4 "c:\Program Files\Darwin Streaming Server\Movies\sample_300kbit.mp4"
copy sample.mp3 "c:\Program Files\Darwin Streaming Server\Movies\sample.mp3"
copy sample_50kbit.3gp "c:\Program Files\Darwin Streaming Server\Movies\sample_50kbit.3gp"
copy sample_h264_1mbit.mp4 "c:\Program Files\Darwin Streaming Server\Movies\sample_h264_1mbit.mp4"
copy sample_h264_100kbit.mp4 "c:\Program Files\Darwin Streaming Server\Movies\sample_h264_100kbit.mp4"
copy sample_h264_300kbit.mp4 "c:\Program Files\Darwin Streaming Server\Movies\sample_h264_300kbit.mp4"

mkdir  "c:\Program Files\Darwin Streaming Server\Logs\"

rmdir /S /Q "c:\Program Files\Darwin Streaming Server\Documentation"
mkdir "c:\Program Files\Darwin Streaming Server\Documentation\"
copy readme.pdf "c:\Program Files\Darwin Streaming Server\Documentation"

rmdir /S /Q "c:\Program Files\Darwin Streaming Server\AdminHtml"
mkdir "c:\Program Files\Darwin Streaming Server\AdminHtml\"
copy AdminHtml\*.pl "c:\Program Files\Darwin Streaming Server\AdminHtml"
copy AdminHtml\*.cgi "c:\Program Files\Darwin Streaming Server\AdminHtml"
copy AdminHtml\*.html "c:\Program Files\Darwin Streaming Server\AdminHtml"

mkdir "c:\Program Files\Darwin Streaming Server\AdminHtml\images\"
copy AdminHtml\images\*.gif "c:\Program Files\Darwin Streaming Server\AdminHtml\images"

mkdir "c:\Program Files\Darwin Streaming Server\AdminHtml\includes\"
copy AdminHtml\includes\*.js "c:\Program Files\Darwin Streaming Server\AdminHtml\includes"

mkdir "c:\Program Files\Darwin Streaming Server\AdminHtml\html_en\"
copy AdminHtml\html_en\* "c:\Program Files\Darwin Streaming Server\AdminHtml\html_en"

"c:\Program Files\Darwin Streaming Server\DarwinStreamingServer" -i

echo -----
echo Adding the install dir to the system path
RegistrySystemPathEditor.exe

echo -----
perl "c:\Program Files\Darwin Streaming Server\WinPasswdAssistant.pl"
cls

echo -----------------------------------------------------------------
echo The Darwin Streaming Server has been installed.
echo -----------------------------------------------------------------
echo Launching the web based Streaming Admin Server
echo *** Closing this window will stop the Streaming Admin Server. ***
perl "c:\Program Files\Darwin Streaming Server\streamingadminserver.pl"
echo -----------------------------------------------------------------
echo Perl is not properly installed on your system.
echo This installer requires Perl to run the Streaming Admin Server.
echo:
echo If you wish to use the web admin user interface to configure Darwin 
echo Streaming Server, you will need to perform the following steps.
echo:
echo See the readme file on installing Perl.
echo Please make sure the path to the Perl interpreter is in your system path 
echo and run 
echo:
echo c:\Program Files\Darwin Streaming Server\streamingadminserver.pl
echo:
echo from the command prompt.

pause
