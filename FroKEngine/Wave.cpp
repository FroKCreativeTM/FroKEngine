#include "Wave.h"
#include <ppl.h>
#include <algorithm>
#include <vector>
#include <cassert>

using namespace DirectX;

Waves::Waves(int m, int n, float dx, float dt, float speed, float damping)
{
    m_NumRows = m;
    m_NumCols = n;

    m_VertexCount = m * n;
    m_TriangleCount = (m - 1) * (n - 1) * 2;

    m_TimeStep = dt;
    m_SpatialStep = dx;

    float d = damping * dt + 2.0f;
    float e = (speed * speed) * (dt * dt) / (dx * dx);
    m_K1 = (damping * dt - 2.0f) / d;
    m_K2 = (4.0f - 8.0f * e) / d;
    m_K3 = (2.0f * e) / d;

    m_PrevSolution.resize(m * n);
    m_CurrSolution.resize(m * n);
    m_Normals.resize(m * n);
    m_TangentX.resize(m * n);

    // 시스템 메모리에 그리드 정점을 생성합니다.

    float halfWidth = (n - 1) * dx * 0.5f;
    float halfDepth = (m - 1) * dx * 0.5f;
    for (int i = 0; i < m; ++i)
    {
        float z = halfDepth - i * dx;
        for (int j = 0; j < n; ++j)
        {
            float x = -halfWidth + j * dx;

            m_PrevSolution[i * n + j] = XMFLOAT3(x, 0.0f, z);
            m_CurrSolution[i * n + j] = XMFLOAT3(x, 0.0f, z);
            m_Normals[i * n + j] = XMFLOAT3(0.0f, 1.0f, 0.0f);
            m_TangentX[i * n + j] = XMFLOAT3(1.0f, 0.0f, 0.0f);
        }
    }
}

Waves::~Waves()
{
}

int Waves::RowCount() const
{
    return m_NumRows;
}

int Waves::ColumnCount() const
{
    return m_NumCols;
}

int Waves::VertexCount() const
{
    return m_VertexCount;
}

int Waves::TriangleCount() const
{
    return m_TriangleCount;
}

float Waves::Width() const
{
    return m_NumCols * m_SpatialStep;
}

float Waves::Depth() const
{
    return m_NumRows * m_SpatialStep;
}

void Waves::Update(float fDeltaTime)
{
	static float t = 0;

	// 시간을 더한다.
	t += fDeltaTime;

	// 지정된 시간 단계에서만 시뮬레이션을 업데이트한다.
	if (t >= m_TimeStep)
	{
		// 내부 포인트만 업데이트하십시오. 제로 경계 조건을 사용한다.
		concurrency::parallel_for(1, m_NumRows - 1, [this](int i)
			//for(int i = 1; i < mNumRows-1; ++i)
			{
				for (int j = 1; j < m_NumCols - 1; ++j)
				{
					// 이 업데이트 후에는 이전 이전 버퍼를 버릴 것이므로 해당 버퍼를 새 업데이트로 덮어쓴다.
					// prev_ij가 다시 필요하지 않고 할당이 마지막에 발생하기 때문에
					// 이 작업을 제자리에서 수행하는 방법(동일한 요소에 대한 읽기/쓰기)에 주의해야한다.

					// 참고 j 인덱스 x 및 i 인덱스 z: h(x_j, z_i, t_k)
					// 또한 +z 축은 "아래로" 이동합니다. 
					//  이것은 단지 감소하는 행 인덱스와 일관성을 유지해야 한다.

					m_PrevSolution[i * m_NumCols + j].y =
						m_K1 * m_PrevSolution[i * m_NumCols + j].y +
						m_K2 * m_CurrSolution[i * m_NumCols + j].y +
						m_K3 * (m_CurrSolution[(i + 1) * m_NumCols + j].y +
							m_CurrSolution[(i - 1) * m_NumCols + j].y +
							m_CurrSolution[i * m_NumCols + j + 1].y +
							m_CurrSolution[i * m_NumCols + j - 1].y);
				}
			});

		// 이전 버퍼를 새 데이터로 덮어쓰기만 하면
		// 이 데이터가 현재 솔루션이 되어야 하고
		// 이전 현재 솔루션이 새 이전 솔루션이 되어야 한다.
		std::swap(m_PrevSolution, m_CurrSolution);

		t = 0.0f; // reset time

		//
		// 유한 차분 체계를 사용하여 법선을 계산합니다
		//
		concurrency::parallel_for(1, m_NumRows - 1, [this](int i)
			//for(int i = 1; i < mNumRows - 1; ++i)
			{
				for (int j = 1; j < m_NumCols - 1; ++j)
				{
					float l = m_CurrSolution[i * m_NumCols + j - 1].y;
					float r = m_CurrSolution[i * m_NumCols + j + 1].y;
					float t = m_CurrSolution[(i - 1) * m_NumCols + j].y;
					float b = m_CurrSolution[(i + 1) * m_NumCols + j].y;
					m_Normals[i * m_NumCols + j].x = -r + l;
					m_Normals[i * m_NumCols + j].y = 2.0f * m_SpatialStep;
					m_Normals[i * m_NumCols + j].z = b - t;

					XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&m_Normals[i * m_NumCols + j]));
					XMStoreFloat3(&m_Normals[i * m_NumCols + j], n);

					m_TangentX[i * m_NumCols + j] = XMFLOAT3(2.0f * m_SpatialStep, r - l, 0.0f);
					XMVECTOR T = XMVector3Normalize(XMLoadFloat3(&m_TangentX[i * m_NumCols + j]));
					XMStoreFloat3(&m_TangentX[i * m_NumCols + j], T);
				}
			});
	}
}

void Waves::Disturb(int i, int j, float magnitude)
{
	// Don't disturb boundaries.
	assert(i > 1 && i < m_NumRows - 2);
	assert(j > 1 && j < m_NumCols - 2);

	float halfMag = 0.5f * magnitude;

	// Disturb the ijth vertex height and its neighbors.
	m_CurrSolution[i * m_NumCols + j].y += magnitude;
	m_CurrSolution[i * m_NumCols + j + 1].y += halfMag;
	m_CurrSolution[i * m_NumCols + j - 1].y += halfMag;
	m_CurrSolution[(i + 1) * m_NumCols + j].y += halfMag;
	m_CurrSolution[(i - 1) * m_NumCols + j].y += halfMag;
}
