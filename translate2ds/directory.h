#pragma once
#include <sys/stat.h>
#include <ftw.h>
#include "log.h"

namespace sp
{
	void MakeDirectory(const std::string& name)
	{
#ifdef _WIN32
		std::string n = "mkdir " + name;
		int result = system(n.c_str());
#else
		int result = mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

		if (result < 0)
		{
			g_Log << "Make Directory failed on \"" << name <<"\"\n";
		}
	}

	int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
	{
		int rv = remove(fpath);

		if (rv)
		perror(fpath);

		return rv;
	}

	int rmrf(const char *path)
	{
		return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
	}

	void DeleteDirectory(const std::string& name)
	{

#ifdef _WIN32
		std::string n = "rmdir /s /q " + name;
		int result = system(n.c_str()); //windows delete temp directory
	//	g_Log <<"WIN32\n";
#else
		std::string n = "rm -rf " + name;
		int result = rmrf(name.c_str());
	//	g_Log <<"NO WIN32\n";
#endif

		if (result < 0)
		{
			g_Log << "Delete Directory failed on \"" << name <<"\" result =" << result << "\n";
		}
	}
}
