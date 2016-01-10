#include "Utility.h"
#include "Dxerr.h"
#include <io.h>
#include <algorithm>

/* 通用工具类 */

// 输出信息到控制台
void CUtility::DebugString(const std::string &str) {
	OutputDebugString(str.c_str());
}

// 封装FAILED宏
bool CUtility::FailedHr(HRESULT hr) {
	if (FAILED(hr)) {
		DebugString("DirectX Reported Error: "+ToString(DXGetErrorString(hr))+" - " + 
			ToString(DXGetErrorDescription(hr))+"\n");

		return true;
	}

	return false;
}

// const char* 转 char*
char* CUtility::DuplicateCharString(const char* charString) {
    if (!charString)
		return 0;

	size_t len = strlen(charString) + 1;
	char *newString = new char[len];
	memcpy(newString, charString, len*sizeof(char));

	return newString;
}

// 确认纹理文件存在
bool CUtility::FindTextureFile(std::string *filename) {
	if (!filename)
		return false;

	// 直接判断文件存在
	if (DoesFileExist(*filename))
		return true;

	// 去掉路径，只留下文件名
	std::string pathOnly;
	std::string filenameOnly;
	SplitPath(*filename,&pathOnly,&filenameOnly);
	// 再次判断文件存在
	if (DoesFileExist(filenameOnly)) {
		*filename = filenameOnly;
		return true;
	}

	// 不存在
	return false;
}

// 封装_access，判断文件存在
bool CUtility::DoesFileExist(const std::string &filename) {
	return (_access(filename.c_str(), 0) != -1);
}

// 分割路径为文件名和文件夹路径，用于查找纹理文件
void CUtility::SplitPath(const std::string& inputPath, std::string* pathOnly, std::string* filenameOnly) {
	std::string fullPath(inputPath);
	std::replace(fullPath.begin(),fullPath.end(),'\\','/');

	std::string::size_type lastSlashPos = fullPath.find_last_of('/');

	if (lastSlashPos == std::string::npos) {
		*pathOnly = "";
		*filenameOnly = fullPath;
	}
	else // do the split
	{
		if (pathOnly)
			*pathOnly = fullPath.substr(0,lastSlashPos);

		if (filenameOnly)
			*filenameOnly = fullPath.substr(lastSlashPos+1,fullPath.size()-lastSlashPos-1);
	}
}

// 获取当前路径
std::string CUtility::GetTheCurrentDirectory() {
	int bufferSize = GetCurrentDirectory(0,NULL);
	char *buffer = new char[bufferSize];

	GetCurrentDirectory(bufferSize,buffer);
	std::string directory(buffer);
	delete []buffer;

	return directory;
}