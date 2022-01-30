#include "Core.h"

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return Core::GetInst()->WndProc(hwnd, msg, wParam, lParam);
}

// nullptr 선언은 여기서 가능하다.
// 왜냐면 얘는 프로그램 시작과 생기는 인스턴스고
// 전역변수니까(static)
bool Core::m_bLoop = true;

Core* Core::m_pInst = nullptr;
Core* Core::GetInst()
{
	return m_pInst;
}

void Core::DestroyInst()
{
	SAFE_DELETE(m_pInst)
}

HINSTANCE Core::GetAppInst() const
{
    return m_hInst;
}

HWND Core::GetMainWnd() const
{
    return m_hWnd;
}

// 화면비 구하기
float Core::AspectRatio() const
{
    return static_cast<float>(m_tRS.nWidth) / m_tRS.nHeight;
}

bool Core::Get4xMsaaState() const
{
    return m_4xMsaaState;
}

void Core::Set4xMsaaState(bool value)
{
    if (m_4xMsaaState != value)
    {
        m_4xMsaaState = value;

        // Recreate the swapchain and buffers with new multisample settings.
        CreateSwapChain();
        OnResize();
    }
}

bool Core::Init(HINSTANCE hInstance, int nWidth, int nHeight)
{
	// 인스턴스 정보를 코어에 저장합니다.
	m_hInst = hInstance;

	MyRegisterClass();

    // 해상도 설정
    m_tRS.nWidth = nWidth;
    m_tRS.nHeight = nHeight;

    Create();

    // 그래픽에 대한 설정을 한다.
    if (!InitDirect3D()) 
    {
        return false;
    }

    // 초기화 된 코드를 이용해서 리사이즈를 한다.
    OnResize();

    // 여기에 게임에서 사용을 하는 매니저들을 생성한다.
    // 타이머(FPS, 델타타임) 초기화
    if (!GET_SINGLE(Timer)->Init(m_hWnd))
    {
        return false;
    }

    // 인풋 매니저 초기화
    if (!GET_SINGLE(Input)->Init(m_hWnd, false))
    {
        return false;
    }

    // 경로관리자 초기화
    if (!GET_SINGLE(PathManager)->Init())
    {
        return false;
    }

    // 리소스 매니저 초기화
    if (!GET_SINGLE(ResourceManager)->Init(m_d3dDevice.Get(), m_CommandList.Get()))
    {
        return false;
    }

    return true;
}

int Core::Run()
{
    MSG msg;

    GET_SINGLE(Timer)->Reset();

    ::ZeroMemory(&msg, sizeof(MSG));

    while (m_bLoop)
    {
        // 윈도우 메시지 큐를 처리하는 함수입니다.
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 윈도우 데드타임
        else
        {
            if (!m_AppPaused)
            {
                // 게임의 로직이 실행된다.
                Logic();
            }
            else
            {
                Sleep(100);
            }
        }
    }

    return (int)msg.wParam;
}

ATOM Core::MyRegisterClass()
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_hInst;
    wcex.hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_FROKENGINE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = m_MainWndCaption.c_str();
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

LRESULT Core::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // ImGui(예정)

    // 윈도우 메시지(키 매니저)
    switch (message)
    {
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            m_AppPaused = true;
            GET_SINGLE(Timer)->Stop();
        }
        else
        {
            m_AppPaused = false;
            GET_SINGLE(Timer)->Start();
        }
        return 0;

        // 사용자가 창의 크기를 조정할 때 WM_SIZE가 전송됩니다.
    case WM_SIZE:
        // 새 클라이언트 영역 차원을 저장합니다.
        m_tRS.nWidth = LOWORD(lParam);
        m_tRS.nHeight = HIWORD(lParam);
        if (m_d3dDevice)
        {
            if (wParam == SIZE_MINIMIZED)
            {
                m_AppPaused = true;
                m_Minimized = true;
                m_Maximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                m_AppPaused = false;
                m_Minimized = false;
                m_Maximized = true;
                OnResize();
            }
            else if (wParam == SIZE_RESTORED)
            {

                // 최소화 상태로 복원?
                if (m_Minimized)
                {
                    m_AppPaused = false;
                    m_Minimized = false;
                    OnResize();
                }

                // 최대화 상태로 복원?
                else if (m_Maximized)
                {
                    m_AppPaused = false;
                    m_Maximized = false;
                    OnResize();
                }
                else if (m_Resizing)
                {
                    // 사용자가 크기 조정 막대를 드래그하는 경우
                    // 사용자가 크기 조정 막대를 계속 끌면 WM_SIZE 메시지 스트림이 창으로 전송되고
                    // 각 WM_SIZE에 대해 크기를 조정하는 것이 
                    // 무의미하고 느려지기 때문에 여기에서 버퍼 크기를 조정하지 않습니다.
                    // 따라서 대신 사용자가 창 크기 조정을 완료한 후 재설정하고 크기 조정 막대를 해제하여 
                    // WM_EXITSIZEMOVE 메시지를 보냅니다.
                }
                else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
                {
                    OnResize();
                }
            }
        }
        return 0;

    // WM_EXITSIZEMOVE 메시지는 사용자가 크기 조정 막대를 잡을 때 전송됩니다.
    case WM_ENTERSIZEMOVE:
        m_AppPaused = true;
        m_Resizing = true;
        GET_SINGLE(Timer)->Stop();
        return 0;

    // WM_EXITSIZEMOVE 사용자가 크기 조정 막대를 놓을 때 전송됩니다.
    // 여기서 우리는 새 창 크기를 기반으로 모든 것을 재설정합니다.
    case WM_EXITSIZEMOVE:
        m_AppPaused = false;
        m_Resizing = false;
        GET_SINGLE(Timer)->Start();
        OnResize();
        return 0;

    case WM_DESTROY:
        m_bLoop = false;
        PostQuitMessage(0);
        return 0;

    // WM_MENUCHAR 메시지는 메뉴가 활성 상태이고 사용자가 mnemonic 또는
    // 가속 키에 해당하지 않는 키를 눌렀을 때 전송됩니다.
    case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

    // 창이 너무 작아지지 않도록 이 메세지를 통해 잡습니다.
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
        return 0;

    case WM_KEYDOWN: case WM_SYSKEYDOWN:
        GET_SINGLE(Input)->KeyDown(wParam);
        return 0;
    case WM_KEYUP: case WM_SYSKEYUP:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        else if ((int)wParam == VK_F2)
            // 4X MSAA
            Set4xMsaaState(!m_4xMsaaState);

        GET_SINGLE(Input)->KeyUp(wParam);
        return 0;
    case WM_CHAR:
        GET_SINGLE(Input)->KeyIn(wParam);
        return 0;
    case WM_MOUSEMOVE:
        GET_SINGLE(Input)->MouseIn(lParam);
        OnMouseMove(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_INPUT:
        GET_SINGLE(Input)->MouseRawIn(lParam);
        return 0;
    case WM_LBUTTONDOWN:
        GET_SINGLE(Input)->SetMouseLButton(true);
        GET_SINGLE(Input)->MouseIn(lParam);
        OnMouseDown(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_LBUTTONUP:
        GET_SINGLE(Input)->SetMouseLButton(false);
        GET_SINGLE(Input)->MouseUp();
        OnMouseUp(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_MBUTTONDOWN:
        GET_SINGLE(Input)->SetMouseMButton(true);
        GET_SINGLE(Input)->MouseIn(lParam);
        OnMouseDown(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_MBUTTONUP:
        GET_SINGLE(Input)->SetMouseMButton(false);
        GET_SINGLE(Input)->MouseUp();
        OnMouseUp(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_RBUTTONDOWN:
        GET_SINGLE(Input)->SetMouseRButton(true);
        GET_SINGLE(Input)->MouseIn(lParam);
        OnMouseDown(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_RBUTTONUP:
        GET_SINGLE(Input)->SetMouseRButton(false);
        GET_SINGLE(Input)->MouseUp();
        OnMouseUp(GET_SINGLE(Input)->GetMouseX(), GET_SINGLE(Input)->GetMouseY());
        return 0;
    case WM_XBUTTONDOWN: case WM_XBUTTONUP:
        GET_SINGLE(Input)->SetMouseXButton(wParam);
        GET_SINGLE(Input)->MouseUp();
        return 0;
    case WM_DEVICECHANGE:
        // 컨트롤러
        GET_SINGLE(Input)->CheckControllers();
        return 0;
    }

    // 기본 값!
    return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL Core::Create()
{
    m_hWnd = CreateWindowW(
        m_MainWndCaption.c_str(),
        m_MainWndCaption.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 
        0, 
        CW_USEDEFAULT, 
        0, 
        nullptr, nullptr, 
        m_hInst, nullptr);

    if (!m_hWnd)
    {
        return FALSE;
    }

    // 타이틀바나 윈도우를 포함한 크기를 구함
    RECT rc = { 0, 0, m_tRS.nWidth, m_tRS.nHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    // 위에서 구해준 크기로 윈도우 클라이언트 크기를 원하는 크기로 맞춰줘야 한다.
    SetWindowPos(m_hWnd, HWND_TOPMOST, 100, 100, rc.right - rc.left,
        rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

    // 윈도우를 실제로 띄운다.
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);

    return TRUE;
}

void Core::Logic()
{
    // 제일 먼저 업데이트 해야되는 물건
    GET_SINGLE(Timer)->Update();
    // 우리가 함수를 만들고 그 델타타임에 이것을 전달하면 된다.
    float fDeltaTime = GET_SINGLE(Timer)->GetDeltaTime();
    CalculateFrameStats();

    // 입력을 받는다.
    Input(fDeltaTime);
    Update(fDeltaTime);
    LateUpdate(fDeltaTime);
    Collision(fDeltaTime);

    Render(fDeltaTime);
}

void Core::CreateRtvAndDsvDescriptorHeaps()
{
    // Render Targer View에 대한 서술자를 생성한다.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf())));

    // Depth/Stencil View에 대한 서술자를 생성하고 작성한다.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));
}

void Core::OnResize()
{
    // 하나라도 완성되지 않았다면 에러를 발생할 것이다.
    assert(m_d3dDevice);
    assert(m_SwapChain);
    assert(m_DirectCmdListAlloc);

    // 자원을 변화시키기 전에는 flush가 필요하다.
    FlushCommandQueue();

    // 커맨드 리스트를 먼저 리셋시킨다.
    ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

    // 우리가 다시 생성할 이전 자원을 전부 해제한다.
    for (int i = 0; i < SwapChainBufferCount; ++i)
    {
        m_SwapChainBuffer[i].Reset();
    }
    // Depth/Stencil 버퍼를 리셋한다.
    m_DepthStencilBuffer.Reset();

    // 스왑 체인의 크기를 다시 지정한다.
    ThrowIfFailed(m_SwapChain->ResizeBuffers(
        SwapChainBufferCount,
        m_tRS.nWidth, m_tRS.nHeight,
        m_BackBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    // 현재 백 버퍼를 0으로 초기화 한다.
    m_CurrBackBuffer = 0;

    // Render targer view 서술자 힙을 가져와서 핸들에 저장한다.
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < SwapChainBufferCount; i++)
    {
        // Swap chain의 i번째 버퍼를 가져온다.
        ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
        // 그 버퍼에 대한 Render Target View를 생성한다.
        m_d3dDevice->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
        // 힙의 다음 항목으로 넘어간다.
        rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
    }

    // depth/stencil 버퍼와 뷰를 생성한다.
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_tRS.nWidth;
    depthStencilDesc.Height = m_tRS.nHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = m_DepthStencilFormat;
    depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    // D3D12_CLEAR_VALUE는 특정 자원을 지우기 위해(즉 완전 초기화)서 
    // 이를 서술하기 위한 타입이다.
    D3D12_CLEAR_VALUE optClear;
    optClear.Format = m_DepthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())));

    // 밉 레벨이 0인 전체 리소스 형식을 사용하여 서술자 생성
    m_d3dDevice->CreateDepthStencilView(m_DepthStencilBuffer.Get(), nullptr, DepthStencilView());

    // 리소스가 초기화 된 상태에서 Depth 버퍼를 사용하도록 전환한다.
    // CD3DX12_RESOURCE_BARRIER는 GPU가 자원을 다 기록하지 않았거나
    // 기록조차 시작하지 않았을 때 이에 접근하는 것 즉 Resource Hazard를 방지한다.
    m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthStencilBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

    // 리사이즈 명령을 실행한다.
    ThrowIfFailed(m_CommandList->Close());
    ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
    // _countof(cmdsLists) : 배열에 있는 명령 목록 수
    // cmdsLists : 명령 목록들의 배열의 첫 원소를 가리키는 포인터
    m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // 리사이징이 완료될 때까지 기다린다.
    FlushCommandQueue();

    // 뷰포트 변환을 클라이언트 영역을 포함하도록  업데이트 한다.
    m_ScreenViewport.TopLeftX = 0;
    m_ScreenViewport.TopLeftY = 0;
    m_ScreenViewport.Width = static_cast<float>(m_tRS.nWidth);
    m_ScreenViewport.Height = static_cast<float>(m_tRS.nHeight);
    m_ScreenViewport.MinDepth = 0.0f;
    m_ScreenViewport.MaxDepth = 1.0f;

    m_ScissorRect = { 0, 0, static_cast<long>(m_tRS.nWidth), static_cast<long>(m_tRS.nHeight) };
    // m_CommandList->RSSetScissorRects(1, &m_ScissorRect);
}

bool Core::InitDirect3D()
{
#if defined(DEBUG) || defined(_DEBUG) 
    // D3D12 디버깅 컨트롤러를 켠다.
    {
        ComPtr<ID3D12Debug> debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    }
#endif

    // IID_PPV_ARGS는 COM의 ID 즉 ppType인 RIID를 void**로 치환한다.
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

    // Direct3D 12 하드웨어 디바이스를 생성한다.
    // 하위 최소 지원 레벨은 Direct3D 11로 한다.
    HRESULT hardwareResult = D3D12CreateDevice(
        nullptr,             // default adapter
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_d3dDevice));

    // 만약 하드웨어 디바이스가 없다면 WARP(윈도우 고급 래스터화 플랫폼) 디바이스 즉 소프트웨어로 교체한다.
    if (FAILED(hardwareResult))
    {
        ComPtr<IDXGIAdapter> pWarpAdapter;
        ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(
            pWarpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_d3dDevice)));
    }


    ThrowIfFailed(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&m_Fence)));

    /// <summary>
    /// 서술자는 서술자 힙에 들어간다.
    /// 이 서술자들에 대한 계산을 진행할 때는 이 서술자 크기를 가져와서 계산을 진행해야한다.
    /// </summary>
    m_RtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_DsvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_CbvSrvUavDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    ///////////////////////////////////////
    //              기능 점검             //
    ///////////////////////////////////////
     
    // 백버퍼의 4X MSAA 퀄리티 지원을 체크합니다.
    // Direct3D 11이 가능한 장비들은 4X MSAA를 모든 렌터 타켓 포맷에 지원합니다.
    // 그렇기 때문에 퀄리티 지원 부분만 체크를 합니다.
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format = m_BackBufferFormat;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    ThrowIfFailed(m_d3dDevice->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualityLevels,
        sizeof(msQualityLevels)));

    m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
    assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
    // 디버깅 모드라면 어뎁터(GPU)에 대한 로그를 남깁니다.
    LogAdapters();
#endif

    // 커맨드 오브젝트 
    CreateCommandObjects();
    CreateSwapChain();
    CreateRtvAndDsvDescriptorHeaps();

    return true;
}

void Core::CreateCommandObjects()
{
    // 커맨드 큐에 대한 서술자를 채우고 생성한다.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue)));

    // 커맨드 할당자를 생성한다.
    // 이 할당자는 커맨드 리스트의 메모리를 할당하기 위해 존재한다.
    // 명령 목록에 추가된 명령들은 이 할당자의 메모리에 저장된다.
    ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, // GPU가 직접 실행하는 명령 목록
        IID_PPV_ARGS(m_DirectCmdListAlloc.GetAddressOf())));    // RIID와 void** 타입의 커맨드 할당자 COM 객체를 넣어준다.

    // 커맨드 큐에 넣을 커맨드 리스트를 생성한다.
    ThrowIfFailed(m_d3dDevice->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_DirectCmdListAlloc.Get(),     // 관련된 커맨드 할당자를 넣는다.
        nullptr,                        // PipelineStateObject를 초기화한다.
        IID_PPV_ARGS(m_CommandList.GetAddressOf())));

    // 닫힌 상태로 시작을 한다.
    // 처음 커맨드 리스트를 참조할 때 이를 리셋할 것이다.
    // 그리고 커맨드 리스트는 리셋하기 전에는 닫혀있어야 한다.
    m_CommandList->Close();
}

void Core::CreateSwapChain()
{
    // 재생성 하기 전에 리셋을 진행한다.
    m_SwapChain.Reset();

    // 스왑 체인에 대한 설명을 적는다.
    // 스왑 체인은 DXGI의 인터페이스이다.
    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = m_tRS.nWidth;
    sd.BufferDesc.Height = m_tRS.nHeight;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = m_BackBufferFormat;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = SwapChainBufferCount;
    sd.OutputWindow = m_hWnd;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // 주의 : 스왑 체인은 플러싱 하기 위해서 커맨드 큐를 사용합니다.
    ThrowIfFailed(m_dxgiFactory->CreateSwapChain(
        m_CommandQueue.Get(),
        &sd,
        m_SwapChain.GetAddressOf()));
}

/// <summary>
/// 커맨드 큐를 플러시합니다.
/// </summary>
void Core::FlushCommandQueue()
{
    // 이 펜스 포인트까지 명령을 표시하기 위해 펜스값을 증가시킨다.
    m_CurrentFence++;

    // 명령 대기열에 명령을 추가하여 새 펜스 포인트를 설정한다.
    // 왜냐하면 우리는 GPU 타임라인에 있는 경우 GPU의 연산이 완료될 때까지 새 펜스 포인트가 설정되지 않습니다. (큐가 처리되는 동안!)
    // 이 Signal() 이전의 모든 명령을 처리합니다. (큐 안의 연산이 전부 끝나야 GPU가 다음 명령 목록을 실행한다!)
    ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

    // GPU가 이 울타리 지점까지 명령을 완료할 때까지 기다린다.
    if (m_Fence->GetCompletedValue() < m_CurrentFence)
    {
        // 이벤트를 핸들하기 위해 핸들 객체를 하나 생성한다.
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

        // GPU가 현재 펜스에 도달하면 그에 맞는 이벤트를 발생시킨다.
        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

        // CPU가 현재 펜스의 이벤트가 진행되는 동안 기다린다.
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

ID3D12Resource* Core::CurrentBackBuffer() const
{
    return m_SwapChainBuffer[m_CurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Core::CurrentBackBufferView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_CurrBackBuffer,
        m_RtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE Core::DepthStencilView() const
{
    return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void Core::CalculateFrameStats()
{
    // 코드는 초당 평균 프레임을 계산하고
    // 한 프레임을 렌더링하는 데 걸리는 평균 시간. 
    // 이러한 통계는 윈도우 캡션 표시줄에 추가됩니다.

    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    // Compute averages over one second period.
    if ((GET_SINGLE(Timer)->GetTotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        wstring fpsStr = to_wstring(fps);
        wstring mspfStr = to_wstring(mspf);

        std::wcout << L"fps: " + fpsStr + L"   mspf: " + mspfStr << std::endl;

        wstring windowText = m_MainWndCaption +
            L"    fps: " + fpsStr +
            L"   mspf: " + mspfStr;

        SetWindowText(m_hWnd, windowText.c_str());

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

void Core::LogAdapters()
{
    UINT i = 0;
    IDXGIAdapter* adapter = nullptr;
    std::vector<IDXGIAdapter*> adapterList;
    while (m_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        std::wstring text = L"***Adapter: ";
        text += desc.Description;
        text += L"\n";

        OutputDebugString(text.c_str());

        adapterList.push_back(adapter);

        ++i;
    }

    for (size_t i = 0; i < adapterList.size(); ++i)
    {
        LogAdapterOutputs(adapterList[i]);
        ReleaseCom(adapterList[i]);
    }
}

void Core::LogAdapterOutputs(IDXGIAdapter* adapter)
{
    UINT i = 0;
    IDXGIOutput* output = nullptr;
    while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        std::wstring text = L"***Output: ";
        text += desc.DeviceName;
        text += L"\n";
        OutputDebugString(text.c_str());

        LogOutputDisplayModes(output, m_BackBufferFormat);

        ReleaseCom(output);

        ++i;
    }
}

void Core::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
    UINT count = 0;
    UINT flags = 0;

    // 리스트 안의 갯수를 가져오기 위해 nullptr를 넣는다.
    output->GetDisplayModeList(format, flags, &count, nullptr);

    std::vector<DXGI_MODE_DESC> modeList(count);
    output->GetDisplayModeList(format, flags, &count, &modeList[0]);

    for (auto& x : modeList)
    {
        UINT n = x.RefreshRate.Numerator;
        UINT d = x.RefreshRate.Denominator;
        std::wstring text =
            L"Width = " + std::to_wstring(x.Width) + L" " +
            L"Height = " + std::to_wstring(x.Height) + L" " +
            L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
            L"\n";

        ::OutputDebugString(text.c_str());
    }
}

Core::Core()
{
    // Only one D3DApp can be constructed.
    assert(m_pInst == nullptr);
    m_pInst = this;

    // 메모리 누수를 체크하기 위한 설정
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    // 메모리 릭이 있을 때만 쓴다.
    // 매개변수로 그 메모리의 블록 번호를 넣어주면,
    // 메모리 릭 부분을 바로 이동한다. (호출 스택을 잘 볼 수 있어야..)
    // _CrtSetBreakAlloc(313);

    // 컴파일 시간에 체크해서 이걸 동작시킬지 말지 결정한다.
#ifdef _DEBUG   
    // 콘솔창을 생성시켜주는 함수
    if (AllocConsole())
    {
        FILE* nfp[3];
        freopen_s(nfp + 0, "CONOUT$", "rb", stdin);
        freopen_s(nfp + 1, "CONOUT$", "wb", stdout);
        freopen_s(nfp + 2, "CONOUT$", "wb", stderr);
        std::ios::sync_with_stdio();
    }
#endif
}

Core::~Core()
{
    DESTROY_SINGLE(Input);
    DESTROY_SINGLE(Timer);
    DESTROY_SINGLE(PathManager);
    DESTROY_SINGLE(ResourceManager);

    if (m_d3dDevice != nullptr)
    {
        FlushCommandQueue();
    }

    // 콘솔창 해제
#ifdef _DEBUG
    FreeConsole();
#endif // _DEBUG
}
