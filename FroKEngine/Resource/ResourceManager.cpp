#include "ResourceManager.h"
#include "../Graphics/Texture/Texture.h"
#include "../Graphics/Material.h"

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

void ResourceManager::LoadShader(const string& strKey, const wchar_t* pFileName, 
	const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
{
	m_shaders[strKey] = CompileShader(pFileName, defines, entrypoint, target);
}

ComPtr<ID3DBlob> ResourceManager::FindShader(const string& strKey)
{
	std::unordered_map<std::string, ComPtr<ID3DBlob>>::iterator iter =
		m_shaders.find(strKey);

	if (iter == m_shaders.end())
	{
		return nullptr;
	}

	// 텍스처 반환.
	return iter->second;
}

Material* ResourceManager::BuildMaterial(string key, int nMatCBIdx, int nDiffuseSrvHeapIdx, 
	XMFLOAT4 diffuseAlbedo, XMFLOAT3 fresnelR0, float fRoughness)
{
	auto material = new Material();
	material->Name = key;
	material->nMatCBIdx = nMatCBIdx;
	material->nDiffuseSrvHeapIdx = nDiffuseSrvHeapIdx;
	material->DiffuseAlbedo = diffuseAlbedo;
	material->FresnelR0 = fresnelR0;
	material->fRoughness = fRoughness;

	return material;
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

Microsoft::WRL::ComPtr<ID3DBlob> ResourceManager::CompileShader(const std::wstring& filename, 
	const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors;
	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	ThrowIfFailed(hr);

	return byteCode;
}
