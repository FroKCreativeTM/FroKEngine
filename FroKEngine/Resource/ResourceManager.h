#pragma once

#include "../Game.h"

using namespace std;

class ResourceManager
{
public:
	bool Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);	// 텍스처를 로딩하려면 인스턴스 핸들이 필요하다.
	class Texture* LoadTexture(const string& strKey, const wchar_t* pFileName);
	class Texture* FindTexture(const string& strKey);

	void LoadShader(const string& strKey, 
		const wchar_t* pFileName,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);

	ComPtr<ID3DBlob> FindShader(const string& strKey);

	class Material* BuildMaterial(string key, int nMatCBIdx,
		int nDiffuseSrvHeapIdx, XMFLOAT4 diffuseAlbedo, XMFLOAT3 fresnelR0,
		float fRoughness);

private : 
	Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);

private:
	// 텍스처를 저장하기 위한 맵
	std::unordered_map<std::string, class Texture*> m_Textures;

	// 셰이더를 저장하기 위한 맵
	std::unordered_map<std::string, ComPtr<ID3DBlob>> m_shaders;

	Microsoft::WRL::ComPtr<ID3D12Device>	m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_CommandList;

	DECLARE_SINGLE(ResourceManager)
};

