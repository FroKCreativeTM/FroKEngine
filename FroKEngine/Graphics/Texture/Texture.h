#pragma once

#include "../../Ref.h"

class Texture : public Ref
{ 
public :
	// ��ȸ �� ����� ������ ������ �̸�
	std::string strName;

	// �� �ؽ�ó�� ������ ��� ����Ǿ��ִ� ���� ����
	std::wstring strFileName;

	Microsoft::WRL::ComPtr<ID3D12Resource> pResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> pUploadHeap = nullptr;
};

