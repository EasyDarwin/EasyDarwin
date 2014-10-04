Making the win32 installer

1. Open WinNTSupport/StreamingServer.dsw in VC++.
2. Do a batch build. This builds all our targets.
3. Run WinNTSupport/makeZip.bat. This copies all install targets (including admin html and help files) into a new directory, WinNTSupport/DarwinStreamingServer.
4. Run WinZip, make a new archive called "DarwinStreamingSrvrversion.Windows.zip", where "version" is the server version. For example: "DarwinStreamingSrvr3_Preview.Windows.zip".
5. Select "Add", navigate into WinNTSupport/DarwinStreamingServer. Make sure that "Include subfolders" is checked, and hit "Add with wildcards". This makes the zip archive.
6. Under "Actions" select "Make .exe file". This makes the self-extracting zip archive, called "DarwinStreamingSrvrversion.Windows.exe". For example: The above
zip archive generates ""DarwinStreamingSrvr3_Preview.Windows.exe". Make sure to set the unstuff to directory to be c:\Darwin Streaming Server3\



