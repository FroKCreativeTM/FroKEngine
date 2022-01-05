#pragma once

#include "Core.h"
#include "resource.h"

using namespace DirectX;

class FroKEngine : public Core
{
public:
	FroKEngine();
	~FroKEngine();

	virtual bool Init(HINSTANCE hInstance, int nWidth = 1280, int nHeight = 720) override;

private:

	// ���� �ð� ��� ó����.
	virtual void Input(float fDeltaTime) override;
	virtual int Update(float fDeltaTime) override;
	// ������Ʈ�� ���� ���·� (�浹 ����) ��ó���� �ʿ��� ���
	virtual int LateUpdate(float fDeltaTime) override;
	virtual void Collision(float fDeltaTime) override;
	virtual void Render(float fDeltaTime) override;

	virtual void OnResize()override;
};

FroKEngine::FroKEngine() : 
	Core() { }
FroKEngine::~FroKEngine() { }

bool FroKEngine::Init(HINSTANCE hInstance, int nWidth, int nHeight)
{
	if (!Core::Init(hInstance, nWidth, nHeight))
	{
		return false;
	}
	return true;
}

void FroKEngine::OnResize()
{
	Core::OnResize();
}

void FroKEngine::Input(float fDeltaTime)
{
	if (GET_SINGLE(Input)->IsKeyDown(VK_ESCAPE))
	{
		m_bLoop = false;
		PostQuitMessage(0);
	}
}

int FroKEngine::Update(float fDeltaTime)
{
	return 0;
}

int FroKEngine::LateUpdate(float fDeltaTime)
{
	return 0;
}

void FroKEngine::Collision(float fDeltaTime)
{
}

void FroKEngine::Render(float fDeltaTime)
{
	// Ŀ�ǵ� ��ϰ� ���õ� �޸𸮸� �����մϴ�.
	// ����� Ŀ�ǵ� ����Ʈ�� GPU���� ������ �Ϸ��� ��쿡�� �缳���� �� �ֽ��ϴ�.
	ThrowIfFailed(m_DirectCmdListAlloc->Reset());

	// Ŀ�ǵ� ����Ʈ�� ExecuteCommandList�� ���� ��� ��⿭�� �߰��� �� �缳���� �� �ֽ��ϴ�.
	// Ŀ�ǵ� ����Ʈ�� �����ϸ� �޸𸮰� ����˴ϴ�.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	// ���ҽ� ��뷮�� ���� ���� ��ȯ�� ��Ÿ���ϴ�.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// ����Ʈ�� Scissor Rect�� �����մϴ�. �̰��� Ŀ�ǵ� ����Ʈ�� �缳���� ������ �缳���Ǿ�� �մϴ�.
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	// �� ���ۿ� ���� ���۸� ����ϴ�.
	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// �������� ���۸� �����մϴ�.
	m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	// ���ҽ� ��뷮�� ���� ���� ��ȯ�� ��Ÿ���ϴ�.
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// ���ڵ� Ŀ�ǵ带 �մϴ�.
	ThrowIfFailed(m_CommandList->Close());

	// ������ ��⿭�� Ŀ�ǵ� ����Ʈ�� �߰��մϴ�.
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// back/front ���۸� �ٲߴϴ�.
	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;

	// ������ ����� �Ϸ�� ������ ��ٸ��ϴ�. 
	// �� ���� ��ȿ�����̸� �ܼ����� ���ؼ� �ִ� �ڵ��Դϴ�. ���߿� ������ �ڵ带 �����ϴ� ����� �����ݴϴ�.
	// ���� �����Ӵ� ��ٸ� �ʿ䰡 �����ϴ�.
	FlushCommandQueue();
}

