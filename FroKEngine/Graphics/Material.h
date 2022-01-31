#pragma once

class Material
{
public : 
	// ������ ���� �̸�
	std::string Name;

	// �� ������ �ش��ϴ� ��� ������ ����
	int nMatCBIdx = -1;

	// SRV ������ �� ������ �ش��ϴ� �л� �ؽ�ó�� �ε���
	int nDiffuseSrvHeapIdx = -1;

	// ��ü�� �ڷᰡ ���ؼ� ��� ���۸� �����ؾ� �ϴ� ���� ���θ� ���ϴ� ��Ƽ �÷����̴�.
	// FrameResource���� ��ü�� cbuffer�� �����Ƿ�, FrameResource���� ������ �����ؾ� �Ѵ�.
	// ���� ��ü�� �ڷḦ ������ ������ �ݵ��
	// NumFrameDirty = gNumFrameResources�� �����ؾ��Ѵ�.
	// �׷��� ������ ������ �ڿ��� ���ŵȴ�.
	int nFramesDirty = gNumFrameResource;

	// ���̵��� ���̴� ���� ��� ���� �ڷ�
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f,1.0f,1.0f,1.0f };		// �л� �ݻ�(�ݼӰ��� ���� 0�̴�.)
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f ,0.01f };			// ������ ���ε�� ���� ���� 0.02, 0.02 0.02�̴�. 
																	// �ݻ籤�� ���� ������ �� ���� ������ ������ ����(Rf(0��)�� �����Ѵ�.
	float fRoughness = 0.25f;
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

struct MaterialConstants
{
	// ���̵��� ���̴� ���� ��� ���� �ڷ�
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f,1.0f,1.0f,1.0f };		// �л� �ݻ�(�ݼӰ��� ���� 0�̴�.)
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f ,0.01f };			// ������ ���ε�� ���� ���� 0.02, 0.02 0.02�̴�. 
																	// �ݻ籤�� ���� ������ �� ���� ������ ������ ����(Rf(0��)�� �����Ѵ�.
	float fRoughness = 0.25f;
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};