#pragma once

#define SAFE_DELETE(p) if(p) { delete p; p = nullptr; }
#define SAFE_DELETE_ARRAY(p) if(p) { delete p; p = nullptr; }
#define SAFE_RELEASE(p) if(p) { p->Release(); p = nullptr; }

// 클래스를 싱글톤으로 자동 생성한다.
#define DECLARE_SINGLE(Type) \
private : \
	static Type*	m_pInst;\
public: \
	static Type* GetInst()\
	{\
		if(!m_pInst)\
		{\
			m_pInst = new Type;\
		}\
		return m_pInst;\
	}\
static void DestroyInst()\
{\
	SAFE_DELETE(m_pInst);\
}\
private : \
	Type();\
	~Type();

// 싱글톤 객체를 NULL로 초기화 정의한다.
#define DEFINITION_SINGLE(Type) Type* Type::m_pInst = NULL; 
#define GET_SINGLE(Type) Type::GetInst()
#define DESTROY_SINGLE(Type) Type::DestroyInst()

#define DEVICE							GEngine->GetDevice()->GetDevice()
#define GRAPHICS_CMD_LIST				GEngine->GetGraphicsCmdQueue()->GetGraphicsCmdList()
#define COMPUTE_CMD_LIST				GEngine->GetComputeCmdQueue()->GetComputeCmdList()
#define RESOURCE_CMD_LIST				GEngine->GetGraphicsCmdQueue()->GetResourceCmdList()
#define GRAPHICS_ROOT_SIGNATURE			GEngine->GetRootSignature()->GetGraphicsRootSignature()
#define COMPUTE_ROOT_SIGNATURE			GEngine->GetRootSignature()->GetComputeRootSignature()

#define INPUT							GET_SINGLE(Input)
#define DELTA_TIME						GET_SINGLE(Timer)->GetDeltaTime()

#define SAFE_DELETE(p) if(p) { delete p; p = nullptr; }

#define CONST_BUFFER(type)		GEngine->GetConstantBuffer(type)

#define PHYSICS							GET_SINGLE(Physics)->GetPhysics()
#define PHYSICS_SCENE					GET_SINGLE(Physics)->GetScene()

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif


// STL을 날리기 위한 함수
// 타입을 잡아줄 필요가 없기 때문에, 재사용성이 높다.
template <typename T>
void Safe_Delete_VecList(T& ref)
{
	typename T::iterator iter;
	typename T::iterator iterEnd = ref.end();

	for (iter = ref.begin(); iter != iterEnd; ++iter)
	{
		SAFE_DELETE((*iter));
	}

	ref.clear();
}

// 레이어 소멸자에서
// 모든 오브젝트를 지운다.
template <typename T>
void Safe_Release_VecList(T& ref)
{
	typename T::iterator iter;
	typename T::iterator iterEnd = ref.end();

	for (iter = ref.begin(); iter != iterEnd; ++iter)
	{
		SAFE_RELEASE((*iter));
	}

	ref.clear();
}

template <typename T>
void Safe_Delete_Map(T& ref)
{
	typename T::iterator iter;
	typename T::iterator iterEnd = ref.end();

	for (iter = ref.begin(); iter != iterEnd; ++iter)
	{
		SAFE_DELETE(iter->second);
	}

	ref.clear();
}

template <typename T>
void Safe_Release_Map(T& ref)
{
	typename T::iterator iter;
	typename T::iterator iterEnd = ref.end();

	for (iter = ref.begin(); iter != iterEnd; ++iter)
	{
		SAFE_RELEASE(iter->second);
	}

	ref.clear();
}
