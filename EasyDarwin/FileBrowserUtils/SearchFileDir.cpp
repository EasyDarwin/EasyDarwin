#include "SearchFileDir.h"
#include <string> 
#include <fstream>
#include <algorithm>
#ifdef _WIN32
#include <direct.h>
#define ACCESS _access
#define MKDIR(a) _mkdir((a))
#else
#define ACCESS access
#define MKDIR(a) mkdir((a),0755)
#endif


/*
struct stat {
        mode_t     st_mode;       //Œƒº˛∑√Œ »®œﬁ
        ino_t      st_ino;       //À˜“˝Ω⁄µ„∫≈
        dev_t      st_dev;        //Œƒº˛ π”√µƒ…Ë±∏∫≈
        dev_t      st_rdev;       //…Ë±∏Œƒº˛µƒ…Ë±∏∫≈
        nlink_t    st_nlink;      //Œƒº˛µƒ”≤¡¨Ω” ˝
        uid_t      st_uid;        //À˘”–’ﬂ”√ªß ∂±∫≈
        gid_t      st_gid;        //◊È ∂±∫≈
        off_t      st_size;       //“‘◊÷Ω⁄Œ™µ•ŒªµƒŒƒº˛»›¡ø
        time_t     st_atime;      //◊Ó∫Û“ª¥Œ∑√Œ ∏√Œƒº˛µƒ ±º‰
        time_t     st_mtime;      //◊Ó∫Û“ª¥Œ–ﬁ∏ƒ∏√Œƒº˛µƒ ±º‰
        time_t     st_ctime;      //◊Ó∫Û“ª¥Œ∏ƒ±‰∏√Œƒº˛◊¥Ã¨µƒ ±º‰
        blksize_t st_blksize;    //∞¸∫¨∏√Œƒº˛µƒ¥≈≈ÃøÈµƒ¥Û–°
        blkcnt_t   st_blocks;     //∏√Œƒº˛À˘’ºµƒ¥≈≈ÃøÈ
      };
 */
SearchFileDir *SearchFileDir::m_sSearchFileDir = nullptr;
SearchFileDir::SearchFileDir()
{

}
SearchFileDir::~SearchFileDir()
{

}
SearchFileDir *SearchFileDir::getInstance()
{
	if (!m_sSearchFileDir)
	{
		m_sSearchFileDir = new SearchFileDir();
		//m_sSearchFileDir->initRootPath();
	}
	return m_sSearchFileDir;
}
bool sortByFileName(const FileAttributeInfo& a, const FileAttributeInfo& b)
{
	return a.fileName < b.fileName;

}
void SearchFileDir::initRootPath()
{
#if _WIN32

	struct _finddata64i32_t fileInfo;//±£¥ÊŒƒº˛–≈œ¢µƒΩ·ππÃÂ
	static int counter=0; //º«¬ºŒƒº˛ ˝ƒø
	long handle;//æ‰±˙
	int done;//≤È’“nextfile «∑Ò≥…π¶

	char root = 'A';

	for (int i = 0 ; i < 26 ; i ++)
	{
		char name[7] = "A:/*.*";
		name[0] += i;
	
		handle = _findfirst64i32(name,&fileInfo);
		if(handle == -1)
		{
			continue;
		}
		else
		{
			name[3] = '\0';
			name[5] = '\0';
			name[6] = '\0';
			FileAttributeInfo info;
			info.fileName = name;
			info.type = File_Dir;
			_rootPathList.push_back(info);
		}
		_findclose(handle);
	}
	
	/*
	FileAttributeInfo info;
	info.fileName = "/";
	info.type = File_Dir;
	_rootPathList.push_back(info);*/
#else
	_rootPathList.clear();
	vector<string> mach;
	mach.push_back("");
	searchFileList("/",_rootPathList,mach);
	

#endif // CC_PLATFORM_WIN32 == CC_TARGET_PLATFORM
	sort(_rootPathList.begin(), _rootPathList.end(), sortByFileName);
}

bool SearchFileDir::isDir(string path)
{
	
#if _WIN32
	struct _finddata64i32_t fileInfo;//±£¥ÊŒƒº˛–≈œ¢µƒΩ·ππÃÂ
	long handle;//æ‰±˙
	string fileName = path+"/*.*"; //“™À—À˜µƒŒƒº˛√˚
	//≤È’“µ⁄“ª∏ˆŒƒº˛£¨∑µªÿæ‰±˙
	handle = _findfirst64i32(fileName.c_str(),&fileInfo);
	if(handle == -1)
	{
		return false;
	}
	
	return true;
#else
	

	DIR              *pDir ;

	pDir = opendir(path.c_str());
	if (pDir)
	{
		return true;
	}
	closedir(pDir);
	return false;
#endif // CC_PLATFORM_WIN32 == CC_TARGET_PLATFORM

}
bool SearchFileDir::searchFileList(const string &dirName,vector<FileAttributeInfo> &list,vector<string> machList,bool onlyDir)
{
#if _WIN32
	struct _finddata64i32_t fileInfo;//±£¥ÊŒƒº˛–≈œ¢µƒΩ·ππÃÂ
	static int counter=0; //º«¬ºŒƒº˛ ˝ƒø
	long handle;//æ‰±˙
	int done;//≤È’“nextfile «∑Ò≥…π¶
	string fileName = dirName +"/*.*"; //“™À—À˜µƒŒƒº˛√˚
	//≤È’“µ⁄“ª∏ˆŒƒº˛£¨∑µªÿæ‰±˙
	handle = _findfirst64i32(fileName.c_str(),&fileInfo);
	if(handle == -1)
	{
		//MESSAGEBOX("∏√ƒø¬ºŒ™ø’!◊Û”“ª¨∂Ø∆¡ƒª∑µªÿ");
		//cout<<"∏√ƒø¬ºŒ™ø’!"<<endl;
		//cin.get();
		return false;
	}
	do
	{
		// cout<<"≤È’“≥…π¶"<<endl;
		// cin.get();
		// cout<<fileInfo.name<<endl;
		//»Áπ˚ «Œƒº˛º–".",ªÚ’ﬂ".."£¨‘ÚΩ¯––≈–∂œœ¬“ª∏ˆŒƒº˛
		if((strcmp(fileInfo.name,".")==0)|(strcmp(fileInfo.name,"..")==0))
		{
			//cout<<"∂™∆˙£°"<<endl;
			//cin.get();
			continue;
		}
		else
		{
			if (isDir(dirName + "/" + fileInfo.name))
			{
				FileAttributeInfo info;
				info.fileName = fileInfo.name;
				info.type = File_Dir;
				info.file_info.st_atime = fileInfo.time_access;
				info.file_info.st_mtime = fileInfo.time_write;
				info.file_info.st_ctime = fileInfo.time_create;
				list.push_back(info);
			}
			else if (onlyDir)
			{

			}
			else
			{
				string name = fileInfo.name;

				for (auto iter = machList.begin(); iter != machList.end();iter++)
				{
					int r_count = name.rfind(*iter);
					if (r_count != -1 && r_count == name.size() - iter->size())
					{
						FileAttributeInfo info;
						info.fileName = fileInfo.name;
						info.type = File_Text;
						info.file_info.st_atime = fileInfo.time_access;
						info.file_info.st_mtime = fileInfo.time_write;
						info.file_info.st_ctime = fileInfo.time_create;
						list.push_back(info);
						break;
					}
				}
			}
			
		}
	}while(!(done=_findnext64i32(handle,&fileInfo)));
	_findclose(handle);
	
#else

	DIR              *pDir ;
	struct dirent    *ent  ;
	int               i=0  ;

	pDir = opendir(dirName.c_str());
	if (!pDir)
	{
		return false;
	}

	while((ent = readdir(pDir)) != nullptr)
	{
		if(ent->d_type & DT_DIR)
		{
			if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
				continue;
			
			FileAttributeInfo info;
			info.fileName = ent->d_name;
			info.type = File_Dir;
			stat(ent->d_name,&(info.file_info));
			list.push_back(info);
			
		}
		else if(!onlyDir)
		{
			for (auto iter = machList.begin(); iter != machList.end();iter++)
			{
				if (find_Last_mach(ent->d_name,*iter))
				{
					FileAttributeInfo info;
					info.fileName = ent->d_name;
					info.type = File_Text;
					stat((dirName + ent->d_name).c_str(),&(info.file_info));
					list.push_back(info);
					break;
				}
			}
			
			//cout<<ent->d_name<<endl;
		}
	}
	closedir(pDir);


	
	
#endif // CC_PLATFORM_WIN32 == CC_TARGET_PLATFORM
	sort(list.begin(), list.end(), sortByFileName);
	return true;
}

bool SearchFileDir::isRootPath(string path)
{
	path = getRealPath(path);
	for (auto iter = _rootPathList.begin() ; iter != _rootPathList.end() ; iter ++)
	{
		if (iter->fileName == path)
		{
			return true;
		}
	}
	return false;
}

string SearchFileDir::getRealPath(string path)//»•µÙ∑µªÿ…œ“ªº∂£¨µ±«∞ƒø¬ºµ»◊÷∑˚
{
	if (path.find("..") == -1)
	{
		return path;
	}

	string new_path = path.substr(0,path.find(".."));
	if (new_path.size() - 1 == getLastSeparatorPos(new_path))
	{
		new_path = new_path.substr(0,new_path.size() - 1);
		if (getLastSeparatorPos(new_path) != -1)
		{
			new_path = new_path.substr(0,getLastSeparatorPos(new_path) - 1);
		}
		return new_path;
	}
	else
	{
		if (path.size() - new_path.size() == 2)
		{
			return new_path;
		}
		else
		{
			return path;
		}
	}
}
int SearchFileDir::getSeparatorPos(string path)
{
	int num = path.find('\\');
	if (num == -1)
	{
		num = path.find('/');
	}
	return num;
}
int SearchFileDir::getLastSeparatorPos(string path)
{
	int num = path.rfind('\\');
	if (num == -1)
	{
		num = path.rfind('/');
	}
	return num;
}

const vector<FileAttributeInfo>* SearchFileDir::getRootPath()
{
	return &_rootPathList;
}
bool SearchFileDir::find_Last_mach(string str,string mach)
{
    if (mach == "*.*") {
        return true;
    }
	int r_count = str.rfind(mach);
	return r_count > 0 && r_count == (str.size() - mach.size());
}
void SearchFileDir::getRoot()
{

}
string SearchFileDir::cmd_system(string command)
{
	string result = "";

	return result;
}

bool SearchFileDir::isHaveThisFile(const string &file)
{
	if (!ACCESS(file.c_str(),0))
	{
		return true;
	}
	return false;
}

void SearchFileDir::buildPath(const string &path)
{
    if (isHaveThisFile(path)) {
        return;
    }
    MKDIR(path.c_str());
}

void SearchFileDir::clearPath(const string &path)
{
    rmdir(path.c_str());
    
    vector<FileAttributeInfo> list;
    vector<string> machList;
    machList.push_back("");
    SearchFileDir::getInstance()->searchFileList(path, list, machList);
    
    for (auto iter = list.begin(); iter != list.end(); iter ++) {
        string name = path + "/" + iter->fileName;
        remove(name.c_str());
    }
    
    buildPath(path);
}
long SearchFileDir::getPathSize(const string &path)
{
    long size = 0;
#if _WIN32
#else
    struct stat statbuff;
    if(stat(path.c_str(), &statbuff) < 0){
        
    }else{
        size = statbuff.st_size;
    }
#endif
    return size;
}