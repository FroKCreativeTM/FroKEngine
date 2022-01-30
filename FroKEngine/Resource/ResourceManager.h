#pragma once

#include "../Game.h"

using namespace std;

class ResourceManager
{
public:
	bool Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);	// 텍스처를 로딩하려면 인스턴스 핸들이 필요하다.
	class Texture* LoadTexture(const string& strKey,
		const wchar_t* pFileName);
	class Texture* FindTexture(const string& strKey);

private:
	// 텍스처를 저장하기 위한 맵
	std::unordered_map<std::string, class Texture*> m_Textures;

	Microsoft::WRL::ComPtr<ID3D12Device>	m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_CommandList;

	DECLARE_SINGLE(ResourceManager)
};

