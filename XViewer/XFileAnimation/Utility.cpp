#include "Utility.h"
#include "Dxerr.h"
#include <io.h>
#include <algorithm>

/* ͨ�ù����� */

// �����Ϣ������̨
void CUtility::DebugString(const std::string &str) {
	OutputDebugString(str.c_str());
}

// ��װFAILED��
bool CUtility::FailedHr(HRESULT hr) {
	if (FAILED(hr)) {
		DebugString("DirectX Reported Error: "+ToString(DXGetErrorString(hr))+" - " + 
			ToString(DXGetErrorDescription(hr))+"\n");

		return true;
	}

	return false;
}

// const char* ת char*
char* CUtility::DuplicateCharString(const char* charString) {
    if (!charString)
		return 0;

	size_t len = strlen(charString) + 1;
	char *newString = new char[len];
	memcpy(newString, charString, len*sizeof(char));

	return newString;
}

// ȷ�������ļ�����
bool CUtility::FindTextureFile(std::string *filename) {
	if (!filename)
		return false;

	// ֱ���ж��ļ�����
	if (DoesFileExist(*filename))
		return true;

	// ȥ��·����ֻ�����ļ���
	std::string pathOnly;
	std::string filenameOnly;
	SplitPath(*filename,&pathOnly,&filenameOnly);
	// �ٴ��ж��ļ�����
	if (DoesFileExist(filenameOnly)) {
		*filename = filenameOnly;
		return true;
	}

	// ������
	return false;
}

// ��װ_access���ж��ļ�����
bool CUtility::DoesFileExist(const std::string &filename) {
	return (_access(filename.c_str(), 0) != -1);
}

// �ָ�·��Ϊ�ļ������ļ���·�������ڲ��������ļ�
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

// ��ȡ��ǰ·��
std::string CUtility::GetTheCurrentDirectory() {
	int bufferSize = GetCurrentDirectory(0,NULL);
	char *buffer = new char[bufferSize];

	GetCurrentDirectory(bufferSize,buffer);
	std::string directory(buffer);
	delete []buffer;

	return directory;
}