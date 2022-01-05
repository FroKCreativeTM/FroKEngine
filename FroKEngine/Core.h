#pragma once

#include "Game.h"
#include "Timer.h"
#include "Input.h"

class Core
{
public : 
	static Core* GetInst();
	static void DestroyInst();

	HINSTANCE GetAppInst() const;
	HWND GetMainWnd() const;

	float     AspectRatio()const;

	bool Get4xMsaaState()const;
	void Set4xMsaaState(bool value);

	virtual bool Init(HINSTANCE hInstance, int nWidth = 1280, int nHeight = 720);
	int Run();

public :
	virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected : 
	ATOM MyRegisterClass();

	// 윈도우 생성
	BOOL Create();

	// 게임 로직 돌리기
	void Logic();

	virtual void CreateRtvAndDsvDescriptorHeaps();
	virtual void OnResize();

	// 전부 시간 기반 처리다.
	virtual void Input(float fDeltaTime) = 0;
	virtual int Update(float fDeltaTime) = 0;
	// 업데이트가 끝난 상태로 (충돌 전에) 후처리가 필요한 경우
	virtual int LateUpdate(float fDeltaTime) = 0;
	virtual void Collision(float fDeltaTime) = 0;
	virtual void Render(float fDeltaTime) = 0;

protected : 
	// 여기는 Direct3D 관련 메서드를 넣습니다.
	bool InitDirect3D();
	void CreateCommandObjects();
	void CreateSwapChain();
	void FlushCommandQueue();

	ID3D12Resource* CurrentBackBuffer()const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;

	void CalculateFrameStats();

	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

protected : 
	static Core*	m_pInst;
	static bool		m_bLoop;

	HINSTANCE		m_hInst;
	HWND			m_hWnd;
	RESOLUTION		m_tRS;
	HDC				m_hDC;

	bool			m_AppPaused = false;			// is the application paused?
	bool			m_Minimized = false;			// is the application minimized?
	bool			m_Maximized = false;			// is the application maximized?
	bool			m_Resizing = false;				// are the resize bars being dragged?
	bool			m_FullscreenState = false;		// fullscreen enabled

	bool			m_4xMsaaState = false;			// 4X MSAA enabled
	UINT			m_4xMsaaQuality = 0;			// quality level of 4X MSAA

	Microsoft::WRL::ComPtr<IDXGIFactory4>	m_dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain>	m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D12Device>	m_d3dDevice;

	Microsoft::WRL::ComPtr<ID3D12Fence>		m_Fence;
	UINT64									m_CurrentFence = 0;

	// 이 커맨드 리스트를 이용하면
	// CPU와 GPU의 병렬적인 성능 부분을 보장할 수 있다.
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>			m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		m_DirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	m_CommandList;

	static const int SwapChainBufferCount = 2;
	int											m_CurrBackBuffer = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource>		m_SwapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource>		m_DepthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	D3D12_VIEWPORT	m_ScreenViewport;
	D3D12_RECT		m_ScissorRect;

	UINT m_RtvDescriptorSize = 0;
	UINT m_DsvDescriptorSize = 0;
	UINT m_CbvSrvUavDescriptorSize = 0;

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring		m_MainWndCaption = L"FroK's DirectX 12 Engine";
	D3D_DRIVER_TYPE		m_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT			m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT			m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

protected : 
	Core();
	Core(const Core& rhs) = delete;
	Core& operator=(const Core& rhs) = delete;
	virtual ~Core();
};

