#pragma once

#define SAFE_DELETE(p) if(p) { delete p; p = NULL; }

// Ŭ������ �̱������� �ڵ� �����Ѵ�.
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

// �̱��� ��ü�� NULL�� �ʱ�ȭ �����Ѵ�.
#define DEFINITION_SINGLE(Type) Type* Type::m_pInst = NULL; 
#define GET_SINGLE(Type) Type::GetInst()
#define DESTROY_SINGLE(Type) Type::DestroyInst()

inline std::wstring AnsiToWString(const std::string & str)
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

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif