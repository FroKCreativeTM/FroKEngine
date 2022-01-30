#include "ResourceManager.h"
#include "../Graphics/Texture/Texture.h"

DEFINITION_SINGLE(ResourceManager)

ResourceManager::ResourceManager()
{

}

ResourceManager::~ResourceManager()
{
	Safe_Release_Map(m_Textures);
}

bool ResourceManager::Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList)
{
	m_d3dDevice = pDevice;
	m_CommandList = pCommandList;

    return true;
}

Texture* ResourceManager::LoadTexture(const string& strKey,
    const wchar_t* pFileName)
{
	// ���� �ؽ�ó�� ã�´�
	Texture* texture = FindTexture(strKey);

	// �ִٸ� ��ȯ
	if (texture)
	{
		return texture;
	}

	// ���ٸ� ����
	texture = new Texture;

	texture->strName = strKey;
	texture->strFileName = pFileName;
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_CommandList.Get(), texture->strFileName.c_str(),
		texture->pResource, texture->pUploadHeap));

	texture->AddRef();

	m_Textures[texture->strName] = std::move(texture);

	return texture;
}

Texture* ResourceManager::FindTexture(const string& strKey)
{
	std::unordered_map<std::string, Texture*>::iterator iter =
		m_Textures.find(strKey);

	if (iter == m_Textures.end())
	{
		return nullptr;
	}

	// �츮�� ����� �ؽ�ó�� ���۷��� ī���͸� ������Ų��.
	iter->second->AddRef();

	// �ؽ�ó ��ȯ.
	return iter->second;
}
