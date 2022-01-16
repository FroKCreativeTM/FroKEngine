#include "GeometryGenerator.h"

using namespace DirectX;

GeometryGenerator::MeshData GeometryGenerator::CreateBox(float width, float height, float depth, uint32 numSubdivisions)
{
    return MeshData();
}

/// <summary>
/// 정이십면체에서 시작한 구입니다. 
/// 이는 내려오면서 subdivision을 통해서 삼각형을 더 나눠서 원에 가깝게 계산합니다.
/// </summary>
/// <param name="radius"></param>
/// <param name="sliceCount"></param>
/// <param name="stackCount"></param>
/// <returns></returns>
GeometryGenerator::MeshData GeometryGenerator::CreateSphere(float radius, uint32 sliceCount, uint32 stackCount)
{
    MeshData meshData;

    //
    // 위 극점부터 아래로 스택이 내려오면서 정점들을 계산합니다.
    //

    // 극점: ​​직사각형 텍스처를 구에 매핑할 때
    // 극점에 할당할 고유한 점이 텍스처 맵에 없기 때문에
    // 텍스처 좌표 왜곡이 발생합니다.
    Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    meshData.Vertices.push_back(topVertex);

    float phiStep = XM_PI / stackCount;
    float thetaStep = 2.0f * XM_PI / sliceCount;

    // 각각의 스택 링의 정점을 계산합니다. (고리로써 극점을 카운팅하지 않습니다.)
    for (uint32 i = 1; i <= stackCount - 1; ++i)
    {
        float phi = i * phiStep;

        // 고리의 정점들 
        for (uint32 j = 0; j <= sliceCount; ++j)
        {
            float theta = j * thetaStep;

            Vertex v;

            // 구형에서 직교로
            v.Position.x = radius * sinf(phi) * cosf(theta);
            v.Position.y = radius * cosf(phi);
            v.Position.z = radius * sinf(phi) * sinf(theta);

            // 세타에 대한 P의 편도함수
            v.TangentU.x = -radius * sinf(phi) * sinf(theta);
            v.TangentU.y = 0.0f;
            v.TangentU.z = +radius * sinf(phi) * cosf(theta);

            XMVECTOR T = XMLoadFloat3(&v.TangentU);
            XMStoreFloat3(&v.TangentU, XMVector3Normalize(T));

            XMVECTOR p = XMLoadFloat3(&v.Position);
            XMStoreFloat3(&v.Normal, XMVector3Normalize(p));

            v.TexC.x = theta / XM_2PI;
            v.TexC.y = phi / XM_PI;

            meshData.Vertices.push_back(v);
        }
    }

    meshData.Vertices.push_back(bottomVertex);

    //
    // 최상위 스택에 대한 인덱스를 계산합니다.
    // 맨 위 스택은 정점 버퍼에 먼저 쓰여지고 맨 위 폴을 첫 번째 링에 연결합니다.
    //

    for (uint32 i = 1; i <= sliceCount; ++i)
    {
        meshData.Indices32.push_back(0);
        meshData.Indices32.push_back(i + 1);
        meshData.Indices32.push_back(i);
    }

    //
    // 내부 스택(극점에 연결되지 않음)에 대한 인덱스를 계산합니다.
    //

    // 인덱스를 첫 번째 링의 첫 번째 꼭짓점 인덱스로 오프셋합니다.
    // 이것은 상단 극 정점을 건너 뛰는 것입니다.
    uint32 baseIndex = 1;
    uint32 ringVertexCount = sliceCount + 1;
    for (uint32 i = 0; i < stackCount - 2; ++i)
    {
        for (uint32 j = 0; j < sliceCount; ++j)
        {
            meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j);
            meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
            meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);

            meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);
            meshData.Indices32.push_back(baseIndex + i * ringVertexCount + j + 1);
            meshData.Indices32.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
        }
    }

    //
    // 하단 스택에 대한 인덱스를 계산합니다.
    // 하단 스택은 정점 버퍼에 마지막으로 작성되었으며 하단 폴을 하단 링에 연결합니다.
    //

    // 아래 정점이 마지막에 추가되었습니다.
    uint32 southPoleIndex = (uint32)meshData.Vertices.size() - 1;

    // 인덱스를 마지막 링의 첫 번째 꼭짓점 인덱스로 오프셋합니다.
    baseIndex = southPoleIndex - ringVertexCount;

    for (uint32 i = 0; i < sliceCount; ++i)
    {
        meshData.Indices32.push_back(southPoleIndex);
        meshData.Indices32.push_back(baseIndex + i);
        meshData.Indices32.push_back(baseIndex + i + 1);
    }

    return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateGeosphere(float radius, uint32 numSubdivisions)
{
    MeshData meshData;

    // 세분화 수에 상한선을 둔다.
    numSubdivisions = std::min<uint32>(numSubdivisions, 6u);

    // 정이십면체를 테셀레이션하여 구를 근사화합니다.

    const float X = 0.525731f;
    const float Z = 0.850651f;

    XMFLOAT3 pos[12] =
    {
        XMFLOAT3(-X, 0.0f, Z),  XMFLOAT3(X, 0.0f, Z),
        XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z),
        XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X),
        XMFLOAT3(0.0f, -Z, X),  XMFLOAT3(0.0f, -Z, -X),
        XMFLOAT3(Z, X, 0.0f),   XMFLOAT3(-Z, X, 0.0f),
        XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)
    };

    uint32 k[60] =
    {
        1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
        1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
        3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
        10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
    };

    meshData.Vertices.resize(12);
    meshData.Indices32.assign(&k[0], &k[60]);

    for (uint32 i = 0; i < 12; ++i) 
    {
        meshData.Vertices[i].Position = pos[i];
    }

    for (uint32 i = 0; i < numSubdivisions; ++i) 
    {
        Subdivide(meshData);
    }

    // 구와 스케일에 정점을 투영합니다.
    for (uint32 i = 0; i < meshData.Vertices.size(); ++i)
    {
        // 단위 구에 투영합니다.
        XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&meshData.Vertices[i].Position));

        // 구체에 투영합니다.
        XMVECTOR p = radius * n;

        XMStoreFloat3(&meshData.Vertices[i].Position, p);
        XMStoreFloat3(&meshData.Vertices[i].Normal, n);

        // 구형 좌표에서 텍스처 좌표를 파생합니다.
        float theta = atan2f(meshData.Vertices[i].Position.z, meshData.Vertices[i].Position.x);

        // Put in [0, 2pi].
        if (theta < 0.0f)
            theta += XM_2PI;

        float phi = acosf(meshData.Vertices[i].Position.y / radius);

        meshData.Vertices[i].TexC.x = theta / XM_2PI;
        meshData.Vertices[i].TexC.y = phi / XM_PI;

        // 세타에 대한 P의 편도함수
        meshData.Vertices[i].TangentU.x = -radius * sinf(phi) * sinf(theta);
        meshData.Vertices[i].TangentU.y = 0.0f;
        meshData.Vertices[i].TangentU.z = +radius * sinf(phi) * cosf(theta);

        XMVECTOR T = XMLoadFloat3(&meshData.Vertices[i].TangentU);
        XMStoreFloat3(&meshData.Vertices[i].TangentU, XMVector3Normalize(T));
    }

    return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, 
    uint32 sliceCount, uint32 stackCount)
{
    MeshData meshData;

    // 더미들을 만든다.
    float stackHeight = height / stackCount;

    // 한 층 위의 더미로 올라갈 때의 반지름 변화량을 구한다.
    float radiusStep = (topRadius - bottomRadius) / stackCount;

    uint32 ringCount = stackCount++;

    // 최하단 고리에서 최상단의 고리로 올라가면서
    // 각 고리의 정점들을 계산한다.
    // 여기서 i가 가로축 고리, j가 y축 조각이다
    for (uint32 i = 0; i < ringCount; ++i)
    {
        float y = -0.5f * height + i * stackHeight;
        float r = bottomRadius + i * radiusStep;

        // 고리의 정점들
        float dTheta = 2.0f * XM_PI / sliceCount;

        for (uint32 j = 0; j <= sliceCount; ++j)
        {
            Vertex vertex;

            float c = cosf(j * dTheta);
            float s = sinf(j * dTheta);

            vertex.Position = XMFLOAT3(r * c, y, r * s);

            vertex.TexC.x = (float)j / sliceCount;
            vertex.TexC.y = 1.0f - (float)i / stackCount;

            // 원기둥을 다음과 같이 매개변수화 할 수 있다.
            // 이를 위해 텍스처 좌표 v 성분과 동일한 방향으로 나아가는 매개변수 v를 도입했다.
            // 이렇게 하면 종접선(bitangent)이 텍스처 좌표 v 성분과 같은 방향이 된다.
            // 밑면 반지름 r0, 윗면 반지름이 r1이라고 할 때, 
            // [0,1] 구간의 v에 대해 
			//   r(v) = r1 + (r0-r1)v
			//
			//   x(t, v) = r(v)*cos(t)
			//   y(t, v) = h - hv
			//   z(t, v) = r(v)*sin(t)
			// 
			//  dx/dt = -r(v)*sin(t)
			//  dy/dt = 0
			//  dz/dt = +r(v)*cos(t)
			//
			//  dx/dv = (r0-r1)*cos(t)
			//  dy/dv = -h
			//  dz/dv = (r0-r1)*sin(t)

            // TangentU는 단위 길이 벡터이다
            vertex.TangentU = XMFLOAT3(-s, 0.0f, c);

            float dr = bottomRadius - topRadius;
            XMFLOAT3 bitangent(dr * c, -height, dr * s);

            XMVECTOR T = XMLoadFloat3(&vertex.TangentU);
            XMVECTOR B = XMLoadFloat3(&bitangent);
            XMVECTOR N = XMVector3Normalize(XMVector3Cross(T,B));
            XMStoreFloat3(&vertex.Normal, N);

            meshData.Vertices.push_back(vertex);
        }
    }

    // 한 고리의 첫 정점과 마지막 정점은 위치가 같지만 텍스처 좌표들이
    // 다르므로 서로 다른 정점으로 간주해야 한다. 이를 위해서 고리의 정점의 갯수에 1을 더한다.
    uint32 ringVertexCount = sliceCount + 1;

    // 각 더미들의 인덱스를 계산한다.
    for (uint32 i = 0; i < stackCount; ++i)
    {
        for (uint32 j = 0; j < sliceCount; ++j)
        {
            meshData.Indices32.push_back(i * ringVertexCount + j);
            meshData.Indices32.push_back((i + 1) * ringVertexCount + j);
            meshData.Indices32.push_back((i + 1) * ringVertexCount + j + 1);

            meshData.Indices32.push_back(i * ringVertexCount + j);
            meshData.Indices32.push_back((i + 1) * ringVertexCount + j + 1);
            meshData.Indices32.push_back(i * ringVertexCount + j + 1);
        }
    }

    // 위 아래 실린더 마개들을 만든다.
    BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
    BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);

    return meshData;
}

GeometryGenerator::MeshData GeometryGenerator::CreateGrid(float width, float depth, uint32 m, uint32 n)
{
    return MeshData();
}

GeometryGenerator::MeshData GeometryGenerator::CreateQuad(float x, float y, float w, float h, float depth)
{
    return MeshData();
}

void GeometryGenerator::Subdivide(MeshData& meshData)
{
    // 입력 지오메트리의 복사본을 저장합니다.
    MeshData inputCopy = meshData;

    meshData.Vertices.resize(0);
    meshData.Indices32.resize(0);

    //       v1
    //       *
    //      / \
	//     /   \
	//  m0*-----*m1
    //   / \   / \
	//  /   \ /   \
	// *-----*-----*
    // v0    m2     v2

    uint32 numTris = (uint32)inputCopy.Indices32.size() / 3;
    for (uint32 i = 0; i < numTris; ++i)
    {
        Vertex v0 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 0]];
        Vertex v1 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 1]];
        Vertex v2 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 2]];

        //
        // 중간점을 생성합니다.
        //

        Vertex m0 = MidPoint(v0, v1);
        Vertex m1 = MidPoint(v1, v2);
        Vertex m2 = MidPoint(v0, v2);

        //
        // 새 지오메트리를 추가합니다.
        //

        meshData.Vertices.push_back(v0); // 0
        meshData.Vertices.push_back(v1); // 1
        meshData.Vertices.push_back(v2); // 2
        meshData.Vertices.push_back(m0); // 3
        meshData.Vertices.push_back(m1); // 4
        meshData.Vertices.push_back(m2); // 5

        meshData.Indices32.push_back(i * 6 + 0);
        meshData.Indices32.push_back(i * 6 + 3);
        meshData.Indices32.push_back(i * 6 + 5);

        meshData.Indices32.push_back(i * 6 + 3);
        meshData.Indices32.push_back(i * 6 + 4);
        meshData.Indices32.push_back(i * 6 + 5);

        meshData.Indices32.push_back(i * 6 + 5);
        meshData.Indices32.push_back(i * 6 + 4);
        meshData.Indices32.push_back(i * 6 + 2);

        meshData.Indices32.push_back(i * 6 + 3);
        meshData.Indices32.push_back(i * 6 + 1);
        meshData.Indices32.push_back(i * 6 + 4);
    }
}

GeometryGenerator::Vertex GeometryGenerator::MidPoint(const Vertex& v0, const Vertex& v1)
{
    return Vertex();
}

// 최 상단의 원을 근사해서 구한다.
void GeometryGenerator::BuildCylinderTopCap(float bottomRadius, float topRadius, float height, 
    uint32 sliceCount, uint32 stackCount, MeshData& meshData)
{
    uint32 baseIndex = (uint32)meshData.Vertices.size();

    float y = 0.5f * height;
    float dTheta = 2.0f * XM_PI / sliceCount;

    // 위 마개 정점들은 최상단 고리 정점들과 위치가 같지만
    // 텍스처 좌표와 법선이 다르므로 따로 추가 해야한다.
    for (uint32 i = 0; i <= sliceCount; ++i)
    {
        float x = topRadius * cosf(i * dTheta);
        float z = topRadius * sinf(i * dTheta);

        // 위 마개 정점들은 최상단 고리 정점들과 위치가 같지만 
        // 텍스처 좌표와 법선이 다르기 때문에 추가가 필요하다.
        float u = x / height + 0.5f;
        float v = z / height + 0.5f;

        meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
    }

    // 마개의 중심 정점
    meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

    // 중심 정점의 인덱스
    uint32 centerIndex = (uint32)meshData.Vertices.size() - 1;

    for (uint32 i = 0; i < sliceCount; ++i)
    {
        meshData.Indices32.push_back(centerIndex);
        meshData.Indices32.push_back(baseIndex + i + 1);
        meshData.Indices32.push_back(baseIndex + i);
    }
}

// 최 하단의 원을 근사해서 구한다.
void GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, 
    uint32 sliceCount, uint32 stackCount, MeshData& meshData)
{
    uint32 baseIndex = (uint32)meshData.Vertices.size();
    float y = -0.5f * height;

    float dTheta = 2.0f * XM_PI / sliceCount;
    for (uint32 i = 0; i <= sliceCount; ++i)
    {
        float x = bottomRadius * cosf(i * dTheta);
        float z = bottomRadius * sinf(i * dTheta);

        // 위 마개 정점들은 최상단 고리 정점들과 위치가 같지만 
        // 텍스처 좌표와 법선이 다르기 때문에 추가가 필요하다.
        float u = x / height + 0.5f;
        float v = z / height + 0.5f;

        meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
    }

    // 마개의 중심 정점
    meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

    // 중심 정점의 인덱스
    uint32 centerIndex = (uint32)meshData.Vertices.size() - 1;

    for (uint32 i = 0; i < sliceCount; ++i)
    {
        meshData.Indices32.push_back(centerIndex);
        meshData.Indices32.push_back(baseIndex + i);
        meshData.Indices32.push_back(baseIndex + i + 1);
    }
}
