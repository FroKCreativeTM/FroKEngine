#pragma once
#include "Object.h"

class Mesh;
class Material;
class GameObject;

struct MeshRenderInfo
{
	shared_ptr<Mesh>				mesh;
	vector<shared_ptr<Material>>	materials;
};

class MeshData : public Object
{
public:
	MeshData();
	virtual ~MeshData();

public:
	static shared_ptr<MeshData> LoadFromFBX(const wstring& path);

	// 게임 입장에서 보면 FBX 자체는 너무 무거운 파일이다.
	// 그렇기 때문에 필요한 정보만 따로 빼내서 이를 활용할 필요가 있는 경우가 많다.
	virtual void Load(const wstring& path);
	virtual void Save(const wstring& path);

	vector<shared_ptr<GameObject>> Instantiate();

private:
	shared_ptr<Mesh>				_mesh;
	vector<shared_ptr<Material>>	_materials;

	vector<MeshRenderInfo> _meshRenders;
};
