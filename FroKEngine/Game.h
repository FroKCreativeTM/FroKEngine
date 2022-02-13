// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>
#include <WindowsX.h>

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <wrl/client.h>

// C/C++ 런타임 헤더 파일입니다.
#include <iostream>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include <functional>
#include <typeinfo>


#include <cassert>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <cstdint>
#include <float.h>
#include <cmath>

//////////////////////////////////////////////////////////////
//					DirectX 12								//
//////////////////////////////////////////////////////////////
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
// #include <d3dcommon.h>

#include "Graphics/d3dx12.h"
#include "Graphics/Texture/DDSTextureLoader.h"

// 필요한 DirectX12 라이브러리를 링크한다.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")


// 인풋 관련 헤더
#include <XInput.h>
#pragma comment(lib, "Xinput.lib")

// 매크로 선언된 헤더를 여기다가 넣는다.
#include "Macro.h"

// 직접 타입에 관련된 것을 선언합니다.
#include "Type.h"
#include "Resource.h"

// 에러 핸들 
#include "ErrorLogger.h"
#include "DxException.h"

// 게임 개발에 전반적으로 필요한 유틸 관련 클래스
#include "Util.h"
#include "MathHelper.h"

// 경로 관련 상수
#include "Path/PathConst.h"

// 씬/레이어/매니저 관련 상수
#include "Scene/SceneConst.h"
#include "Scene/LayerConst.h"

// 충돌 관련 플래그
#include "Collision/CollisionFlag.h"

// 프레임 자원 3개를 담는 벡터를 생성하기 위한 도구
// 이렇게 하면 n~n+2까지의 프레임을 미리 생성할 수 있다.
static const int gNumFrameResource = 3;
