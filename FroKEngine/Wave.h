#pragma once

#include <vector>
#include <DirectXMath.h>

class Waves
{
public:
    Waves(int m, int n, float dx, float dt, float speed, float damping);
    Waves(const Waves& rhs) = delete;
    Waves& operator=(const Waves& rhs) = delete;
    ~Waves();

    int RowCount()const;
    int ColumnCount()const;
    int VertexCount()const;
    int TriangleCount()const;
    float Width()const;
    float Depth()const;

    // i번째 그리드 포인트의 솔루션 반환
    const DirectX::XMFLOAT3& Position(int i)const { return m_CurrSolution[i]; }

    // i번째 그리드 포인트에서 법선 솔루션을 반환합니다.
    const DirectX::XMFLOAT3& Normal(int i)const { return m_Normals[i]; }

    // 로컬 x축 방향의 i번째 그리드 점에서 단위 접선 벡터를 반환합니다.
    const DirectX::XMFLOAT3& TangentX(int i)const { return m_TangentX[i]; }

    void Update(float fDeltaTime);
    void Disturb(int i, int j, float magnitude);

private:
    int m_NumRows = 0;
    int m_NumCols = 0;

    int m_VertexCount = 0;
    int m_TriangleCount = 0;

    // 미리 계산할 수 있는 시뮬레이션 상수.
    float m_K1 = 0.0f;
    float m_K2 = 0.0f;
    float m_K3 = 0.0f;

    float m_TimeStep = 0.0f;
    float m_SpatialStep = 0.0f;

    std::vector<DirectX::XMFLOAT3> m_PrevSolution;
    std::vector<DirectX::XMFLOAT3> m_CurrSolution;
    std::vector<DirectX::XMFLOAT3> m_Normals;
    std::vector<DirectX::XMFLOAT3> m_TangentX;
};

