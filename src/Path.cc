#include	<Path.h>

#if defined(_WIN32)
#	include		<windows.h>
#	include		<direct.h>
#	include		<io.h>
#else
#	include		<limits.h>
#	include		<dirent.h>
#	include		<unistd.h>
#	include		<sys/stat.h>
#endif

#include	<cstring>
#include	<vector>

using namespace std;

bool Path::Exists(const string & sPath) {
#if defined(_WIN32)
	return _access(sPath.c_str(), 0) == 0;
#else
	return access(sPath.c_str(), 0) == 0;
#endif
}

bool Path::Create(const string & sPath) {
#if defined(_WIN32)
	return _mkdir(sPath.c_str()) == 0;
#else
	return mkdir(sPath.c_str(), 755) == 0;
#endif
}

string Path::Current() {
	std::string sPath(512, '0');
#if defined(_WIN32)
	_getcwd((char *)sPath.c_str(), 512);
#else
	getcwd((char *)sPath.c_str(), 512);
#endif
	return std::move(sPath);
}

bool Path::Change(const string & sPath) {
#if defined(_WIN32)
	return (0 == _chdir(sPath.c_str()));
#else
	return (0 == chdir(sPath.c_str()));
#endif
}

string Path::FullPath(const string & sPath) {
	string s(512, '0');
#if defined(_WIN32)
	if (0 >= GetFullPathNameA(sPath.c_str(), 512, (char *)s.c_str(), NULL)) return sPath;
#else
	if (!realpath(sPath.c_str(), (char *)s.c_str())) return sPath;
#endif
	return std::move(s);
}

string Path::PurePath(const string & sPath) {
	if (sPath.empty() || sPath.length() <= 0) return "";
	for (int nIdx = sPath.length() - 1; nIdx >= 0; --nIdx) {
		if (sPath[nIdx] == '/' || sPath[nIdx] == '\\') {
			if (nIdx != 0) return sPath.substr(0, nIdx);
		}
	}
	return "./";
}

string Path::PureFile(const string & sPath) {
	if (sPath.empty() || sPath.length() <= 0) return "";
	for (int nIdx = sPath.length() - 1; nIdx >= 0; --nIdx) {
		if (sPath[nIdx] == '/' || sPath[nIdx] == '\\') {
			return sPath.substr(nIdx + 1);
		}
	}
	return sPath;
}

void Path::Traverse(const string & sPath, function<void (const string &)> fOpt, bool bRecursive) {
	vector<string> vSubDir;

#if defined(_WIN32)
	WIN32_FIND_DATAA iFind;
	HANDLE hResult;

	std::string sFinder(sPath);
	sFinder.append("\\*.*");

	hResult = ::FindFirstFileA(sFinder.c_str(), &iFind);
	if (hResult == INVALID_HANDLE_VALUE)
		return;
	
	while (::FindNextFileA(hResult, &iFind)) {
		std::string sData(sPath);
		sData.append("\\");
		sData.append(iFind.cFileName);
		if (iFind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (!bRecursive)
				continue;
			else if (strcmp(iFind.cFileName, ".") == 0 || strcmp(iFind.cFileName, "..") == 0)
				continue;
			else
				vSubDir.push_back(sData);
		} else {
			fOpt(sData);
		}
	}

	::FindClose(hResult);
#else
	DIR *			pDir;
	struct dirent *	pFile;
	struct stat		iStat;

	if (!(pDir = ::opendir(sPath.c_str()))) return;

	while (NULL != (pFile = ::readdir(pDir))) {
		std::string sData(sPath);
		sData.append("/");
		sData.append(pFile->d_name);

		if (strncmp(pFile->d_name, ".", 1) == 0)
			continue;
		else if (::stat(sData.c_str(), &iStat) < 0)
			continue;			

		if (S_ISDIR(iStat.st_mode)) {
			if (bRecursive) vSubDir.push_back(sData);
		} else if (S_ISREG(iStat.st_mode)) {
			fOpt(sData);
		}
	}

	::closedir(pDir);
#endif

	for (size_t nIdx = 0; nIdx < vSubDir.size(); ++nIdx)
		Traverse(vSubDir[nIdx], fOpt);
}
