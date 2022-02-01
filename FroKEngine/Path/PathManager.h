#pragma once

#include "../Game.h"

using namespace std;

class PathManager
{
public:
	bool Init();
	bool CreatePath(const string& strKey,
		const wchar_t* pPath,
		const string& strBaseKey = ROOT_PATH);
	const wchar_t* FindPath(const string& strKey);
	const char* FindPathMultiByte(const string& strKey);

private:
	// wstring는 유니코드 string이다.
	unordered_map<string, wstring>	m_mapPath;
	// 멀티바이트 저장용
	char							m_strPath[MAX_PATH];

	DECLARE_SINGLE(PathManager)
};

