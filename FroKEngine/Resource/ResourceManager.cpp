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
	// 먼저 텍스처를 찾는다
	Texture* texture = FindTexture(strKey);

	// 있다면 반환
	if (texture)
	{
		return texture;
	}

	// 없다면 생성
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

	// 우리가 사용할 텍스처의 레퍼런스 카운터를 증가시킨다.
	iter->second->AddRef();

	// 텍스처 반환.
	return iter->second;
}
