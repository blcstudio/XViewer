#pragma once

#include <string>
#include <sstream>
#include <d3d9.h>
#include <assert.h>

/* ͨ�ù����� */

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
	
	static bool FailedHr(HRESULT hr);						// ��װFAILED�꣬�������������Ϣ
	static void DebugString(const std::string &str);		// ����̨�������
	static char* DuplicateCharString(const char* c_str);	// const char* ת char*
	static bool FindTextureFile(std::string *filename);		// ȷ�������ļ�����
	static bool DoesFileExist(const std::string &filename);	// ȷ���ļ�����
	static void SplitPath(const std::string& inputPath, std::string* pathOnly, std::string* filenameOnly);	// �ָ��ļ�·����
	static std::string GetTheCurrentDirectory();			// ��ǰĿ¼
};
