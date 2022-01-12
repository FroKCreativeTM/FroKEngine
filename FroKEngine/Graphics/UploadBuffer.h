#pragma once

template<typename T>
class UploadBuffer
{
public:
    UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) :
        m_IsConstantBuffer(isConstantBuffer)
    {
        m_ElementByteSize = sizeof(T);

        // 상수 버퍼의 요소들은 256 바이트의 배수 단위로 생성한다.
        // 이는 하드웨어가 m*256바이트 오프셋과 n*256바이트 길이의 상수 데이터만 볼 수 있기 때문이다.
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

        // 리소스가 끝날 때까지 매핑을 해제할 필요가 없다. 
        // 그러나 우리는 다음 주소에 쓰지 말아야 한다.
        // GPU에서 사용하는 동안 리소스를 제거한다. 
        // (따라서 동기화 기술을 사용해야 한다.)
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