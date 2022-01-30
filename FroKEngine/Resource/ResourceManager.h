#pragma once

#include "../Game.h"

using namespace std;

class ResourceManager
{
public:
	bool Init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);	// �ؽ�ó�� �ε��Ϸ��� �ν��Ͻ� �ڵ��� �ʿ��ϴ�.
	class Texture* LoadTexture(const string& strKey,
		const wchar_t* pFileName);
	class Texture* FindTexture(const string& strKey);

private:
	// �ؽ�ó�� �����ϱ� ���� ��
	std::unordered_map<std::string, class Texture*> m_Textures;

	Microsoft::WRL::ComPtr<ID3D12Device>	m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_CommandList;

	DECLARE_SINGLE(ResourceManager)
};

