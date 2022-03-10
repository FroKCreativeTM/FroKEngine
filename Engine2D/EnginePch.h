#pragma once

// std::byte ������� ����
#define _HAS_STD_BYTE 0

// ���� Include
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <list>
#include <map>

#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
using namespace std;

// C++17���� ���� file system�̴�.
#include <filesystem>
namespace fs = std::filesystem;

#include "d3dx12.h"				// DX Util
#include "SimpleMath.h"
#include <d3d12.h>
#include <wrl.h>				// For COM
#include <d3dcompiler.h>		
#include <dxgi.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <dxgi1_4.h>
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace Microsoft::WRL;

// ���� ��ũ��
#include "Macro.h"

// �ؽ�ó ����
#include <DirectXTex/DirectXTex.h>
#include <DirectXTex/DirectXTex.inl>

#ifdef _DEBUG
#pragma comment(lib, "DirectXTex\\DirectXTex_debug.lib")
#else
#pragma comment(lib, "DirectXTex\\DirectXTex.lib")
#endif

// �޽� ������ ����
#include "FBX/fbxsdk.h"

// ���� ����
#include "FMOD/fmod.hpp"
using namespace FMOD;
using namespace std;

// UI�� ����
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx12.h"
#include "fontawesome/IconsFontAwesome5.h"

// ���� ����
#include <PxPhysicsAPI.h>

#ifdef _DEBUG
//#pragma comment(lib, "Physx\\Debug\\LowLevel_static_64.lib")
//#pragma comment(lib, "Physx\\Debug\\LowLevelAABB_static_64.lib")
//#pragma comment(lib, "Physx\\Debug\\LowLevelDynamics_static_64.lib")
#pragma comment(lib, "Physx\\Debug\\PhysX_64.lib")
//#pragma comment(lib, "Physx\\Debug\\PhysXCharacterKinematic_static_64.lib")
#pragma comment(lib, "Physx\\Debug\\PhysXCommon_64.lib")
//#pragma comment(lib, "Physx\\Debug\\PhysXCooking_64.lib")
#pragma comment(lib, "Physx\\Debug\\PhysXExtensions_static_64.lib")
#pragma comment(lib, "Physx\\Debug\\PhysXFoundation_64.lib")
#pragma comment(lib, "Physx\\Debug\\PhysXPvdSDK_static_64.lib")
//#pragma comment(lib, "Physx\\Debug\\PhysXTask_static_64.lib")
//#pragma comment(lib, "Physx\\Debug\\PhysXVehicle_static_64.lib")
//#pragma comment(lib, "Physx\\Debug\\SampleBase_static_64.lib")
//#pragma comment(lib, "Physx\\Debug\\SampleFramework_static_64.lib")
//#pragma comment(lib, "Physx\\Debug\\SamplePlatform_static_64.lib")
//#pragma comment(lib, "Physx\\Debug\\SampleRenderer_static_64.lib")
//#pragma comment(lib, "Physx\\Debug\\Samples_64.lib")
//#pragma comment(lib, "Physx\\Debug\\SamplesToolkit_static_64.lib")
//#pragma comment(lib, "Physx\\Debug\\SceneQuery_static_64.lib")
//#pragma comment(lib, "Physx\\Debug\\SimulationController_static_64.lib")
//#pragma comment(lib, "Physx\\Debug\\SnippetRender_static_64.lib")
//#pragma comment(lib, "Physx\\Debug\\SnippetUtils_static_64.lib")
#else
#pragma comment(lib, "Physx\\Release\\LowLevel_static_64.lib")
#pragma comment(lib, "Physx\\Release\\LowLevelAABB_static_64.lib")
#pragma comment(lib, "Physx\\Release\\LowLevelDynamics_static_64.lib")
#pragma comment(lib, "Physx\\Release\\PhysX_64.lib")
#pragma comment(lib, "Physx\\Release\\PhysXCharacterKinematic_static_64.lib")
#pragma comment(lib, "Physx\\Release\\PhysXCommon_64.lib")
#pragma comment(lib, "Physx\\Release\\PhysXCooking_64.lib")
#pragma comment(lib, "Physx\\Release\\PhysXExtensions_static_64.lib")
#pragma comment(lib, "Physx\\Release\\PhysXFoundation_64.lib")
#pragma comment(lib, "Physx\\Release\\PhysXPvdSDK_static_64.lib")
#pragma comment(lib, "Physx\\Release\\PhysXTask_static_64.lib")
#pragma comment(lib, "Physx\\Release\\PhysXVehicle_static_64.lib")
#pragma comment(lib, "Physx\\Release\\SampleBase_static_64.lib")
#pragma comment(lib, "Physx\\Release\\SampleFramework_static_64.lib")
#pragma comment(lib, "Physx\\Release\\SamplePlatform_static_64.lib")
#pragma comment(lib, "Physx\\Release\\SampleRenderer_static_64.lib")
#pragma comment(lib, "Physx\\Release\\Samples_64.lib")
#pragma comment(lib, "Physx\\Release\\SamplesToolkit_static_64.lib")
#pragma comment(lib, "Physx\\Release\\SceneQuery_static_64.lib")
#pragma comment(lib, "Physx\\Release\\SimulationController_static_64.lib")
#pragma comment(lib, "Physx\\Release\\SnippetRender_static_64.lib")
#pragma comment(lib, "Physx\\Release\\SnippetUtils_static_64.lib")
#endif


// ���� lib
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")


#ifdef _DEBUG
#pragma comment(lib, "FBX\\debug\\libfbxsdk-mt.lib")
#pragma comment(lib, "FBX\\debug\\libxml2-mt.lib")
#pragma comment(lib, "FBX\\debug\\zlib-mt.lib")
#else
#pragma comment(lib, "FBX\\release\libfbxsdk-md.lib")
#pragma comment(lib, "FBX\\release\libxml2-md.lib")
#pragma comment(lib, "FBX\\release\\zlib-md.lib")
#endif

// #pragma comment(lib, "FMOD/fmod64_vc")

// ���� typedef
// �⺻ Ÿ���� �ƴ϶� ���� ��Ī�� �� ��
using int8		= __int8;
using int16		= __int16;
using int32		= __int32;
using int64		= __int64;
using uint8		= unsigned __int8;
using uint16	= unsigned __int16;
using uint32	= unsigned __int32;
using uint64	= unsigned __int64;
using Vec2		= DirectX::SimpleMath::Vector2;
using Vec3		= DirectX::SimpleMath::Vector3;
using Vec4		= DirectX::SimpleMath::Vector4;
using Matrix	= DirectX::SimpleMath::Matrix;

enum class CBV_REGISTER : uint8
{
	b0,
	b1,
	b2,
	b3,
	b4,

	END
};

enum class SRV_REGISTER : uint8
{
	t0 = static_cast<uint8>(CBV_REGISTER::END),
	t1,
	t2,
	t3,
	t4,
	t5,
	t6,
	t7,
	t8,
	t9,

	END
};

enum class UAV_REGISTER : uint8
{
	u0 = static_cast<uint8>(SRV_REGISTER::END),
	u1,
	u2,
	u3,
	u4,

	END
};

enum
{
	SWAP_CHAIN_BUFFER_COUNT = 2,
	CBV_REGISTER_COUNT = CBV_REGISTER::END,
	SRV_REGISTER_COUNT = static_cast<uint8>(SRV_REGISTER::END) - CBV_REGISTER_COUNT,
	CBV_SRV_REGISTER_COUNT = CBV_REGISTER_COUNT + SRV_REGISTER_COUNT,
	UAV_REGISTER_COUNT = static_cast<uint8>(UAV_REGISTER::END) - CBV_SRV_REGISTER_COUNT,
	TOTAL_REGISTER_COUNT = CBV_SRV_REGISTER_COUNT + UAV_REGISTER_COUNT
};

struct WindowInfo
{
	HWND hwnd;		// ��� ������
	int32 width;	// �ʺ�
	int32 height;	// ����
	bool windowed;	// â����ΰ�.
};

struct Vertex
{
	Vertex() {}

	Vertex(Vec3 p, Vec2 u, Vec3 n, Vec3 t)
		: pos(p), uv(u), normal(n), tangent(t)
	{
	}

	Vec3 pos;
	Vec2 uv;
	Vec3 normal;
	Vec3 tangent;
	Vec4 weights;
	Vec4 indices;
};


struct TransformParams
{
	Matrix matWorld;
	Matrix matView;
	Matrix matProjection;
	Matrix matWV;
	Matrix matWVP;
	Matrix matViewInv;
};

struct AnimFrameParams
{
	Vec4	scale;
	Vec4	rotation; // Quaternion
	Vec4	translation;
};

// �������� ������ ��������.
extern unique_ptr<class Engine> GEngine;


// Utils
wstring s2ws(const string& s);
string ws2s(const wstring& s);