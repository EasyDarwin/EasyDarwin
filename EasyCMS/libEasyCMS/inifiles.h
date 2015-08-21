/*******************************************************

inifiles.h
ini file read/write functions
used by linux programs

********************************************************/
#ifndef INIFILES_H
#define INIFILES_H

#include <string>
using namespace std;

string IniReadString (const char *section, const char *indent, const char *defaultresult, const char *inifilename);
int IniWriteString (const char *section, const char *indent, const char *value, const char *inifilename);
int IniReadInteger (const char *section, const char *indent, int defaultresult, const char *inifilename);
int IniWriteInteger (const char *section, const char *indent, int value, const char *inifilename);

#endif /*  */
