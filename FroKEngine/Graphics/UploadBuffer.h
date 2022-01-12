#pragma once

template<typename T>
class UploadBuffer
{
public:
    UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) :
        m_IsConstantBuffer(isConstantBuffer)
    {
        m_ElementByteSize = sizeof(T);

        // ��� ������ ��ҵ��� 256 ����Ʈ�� ��� ������ �����Ѵ�.
        // �̴� �ϵ��� m*256����Ʈ �����°� n*256����Ʈ ������ ��� �����͸� �� �� �ֱ� �����̴�.
        // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
        // UINT64 OffsetInBytes; // multiple of 256
        // UINT   SizeInBytes;   // multiple of 256
        // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
        if (isConstantBuffer) 
        {
            m_ElementByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(T));
        }

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(m_ElementByteSize * elementCount),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_UploadBuffer)));

        ThrowIfFailed(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));

        // ���ҽ��� ���� ������ ������ ������ �ʿ䰡 ����. 
        // �׷��� �츮�� ���� �ּҿ� ���� ���ƾ� �Ѵ�.
        // GPU���� ����ϴ� ���� ���ҽ��� �����Ѵ�. 
        // (���� ����ȭ ����� ����ؾ� �Ѵ�.)
    }

    UploadBuffer(const UploadBuffer& rhs) = delete;
    UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
    ~UploadBuffer()
    {
        if (m_UploadBuffer != nullptr) 
        {
            m_UploadBuffer->Unmap(0, nullptr);
        }

        m_MappedData = nullptr;
    }

    ID3D12Resource* Resource()const
    {
        return m_UploadBuffer.Get();
    }

    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&m_MappedData[elementIndex * m_ElementByteSize], &data, sizeof(T));
    }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
    BYTE* m_MappedData = nullptr;

    UINT m_ElementByteSize = 0;
    bool m_IsConstantBuffer = false;
};