#include "PathManager.h"

DEFINITION_SINGLE(PathManager)

PathManager::PathManager()
{

}

PathManager::~PathManager()
{

}

bool PathManager::Init()
{
	// MAX_PATH : winapi�� ���(260)
	wchar_t strPath[MAX_PATH] = {};

	// ���� ����ǰ� �ִ� ��θ� �̾ƿ´�.
	// ������ �����/�������� ��ΰ� �ٸ���.
	// ���� ���� ���߿� ���丮 �̸��� �޶��� �� �ִ�.
	// GetCurrentDirectory()

	// �� �Լ��� ���������� �̸����� ����Ѵ�.(../test.exe)
	// �̰��� �������...
	GetModuleFileName(NULL, strPath, MAX_PATH);

	for (int i = lstrlen(strPath) - 1; i >= 0; --i)
	{
		// ������ ������ / �Ǵ� \�� �߰��� ���
		if (strPath[i] == '/' ||
			strPath[i] == '\\')
		{
			memset(strPath + (i + 1), 0,
				sizeof(wchar_t) * (MAX_PATH - (i + 1)));
			break;
		}
	}

	// ���� ��Ʈ �н��� ��������.
	m_mapPath.insert(make_pair(ROOT_PATH, strPath));

	// �ؽ�ó ��� ����
	if (!CreatePath(TEXTURE_PATH, L"Graphics\\Texture\\Datas\\"))
	{
		return false;
	}

	// ������ ��� ����
	if (!CreatePath(DATA_PATH, L"Data\\"))
	{
		return false;
	}

	// ������ ��� ����
	if (!CreatePath(SOUND_PATH, L"Sound\\"))
	{
		return false;
	}

	return true;
}

// ����Ʈ �н�(ex. �ؽ�ó �н������� ���)�� ����
// 1. Ű��
// 2. �츮�� ���� path
// 3. �⺻ �н� ���
bool PathManager::CreatePath(const string& strKey,
	const wchar_t* pPath, const string& strBaseKey)
{
	const wchar_t* pBasePath = FindPath(strBaseKey);

	wstring strPath;

	// ���� ��쿡��!
	if (pBasePath)
	{
		strPath = pBasePath;
	}

	strPath += pPath;

	// �ʿ� �־��ش�.
	m_mapPath.insert(make_pair(strKey, strPath));

	return true;
}

const wchar_t* PathManager::FindPath(const string& strKey)
{
	unordered_map<string, wstring>::iterator iter =
		m_mapPath.find(strKey);

	if (iter == m_mapPath.end())
	{
		return nullptr;
	}

	return iter->second.c_str();
}

const char* PathManager::FindPathMultiByte(const string& strKey)
{
	// �ϴ� ���� �����ڵ� �н��� ã�´�.
	const wchar_t* pPath = FindPath(strKey);
	if (!pPath)
	{
		return nullptr;
	}

	// �����ڵ带 ��Ƽ����Ʈ��
	memset(m_strPath, 0, MAX_PATH);
	WideCharToMultiByte(CP_ACP, 0, pPath, -1, m_strPath, lstrlen(pPath), 0, 0);

	return m_strPath;
}