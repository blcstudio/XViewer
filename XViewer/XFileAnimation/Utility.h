#pragma once

#include <string>
#include <sstream>
#include <d3d9.h>
#include <assert.h>

/* 通用工具类 */

template <class T>
std::string ToString(const T & t) {
	std::ostringstream oss;
	oss.clear();
	oss << t;
	return oss.str();
}

class CUtility
{
public:
	CUtility(void){};
	~CUtility(void){};
	
	static bool FailedHr(HRESULT hr);						// 封装FAILED宏，增加输出错误信息
	static void DebugString(const std::string &str);		// 控制台输出文字
	static char* DuplicateCharString(const char* c_str);	// const char* 转 char*
	static bool FindTextureFile(std::string *filename);		// 确认纹理文件存在
	static bool DoesFileExist(const std::string &filename);	// 确认文件存在
	static void SplitPath(const std::string& inputPath, std::string* pathOnly, std::string* filenameOnly);	// 分割文件路径用
	static std::string GetTheCurrentDirectory();			// 当前目录
};
