#pragma once

#include "../../Ref.h"

class Texture : public Ref
{ 
public :
	// 조회 시 사용할 고유한 재질의 이름
	std::string strName;

	// 이 텍스처의 파일이 어디 저장되어있는 지를 저장
	std::wstring strFileName;

	Microsoft::WRL::ComPtr<ID3D12Resource> pResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> pUploadHeap = nullptr;
};

