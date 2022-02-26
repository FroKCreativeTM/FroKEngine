#include "pch.h"
#include "MeshRenderer.h"
#include "Mesh.h"
#include "Material.h"
#include "Transform.h"
#include "InstancingManager.h"
#include "Resources.h"
#include "Animator.h"

MeshRenderer::MeshRenderer() : Component(COMPONENT_TYPE::MESH_RENDERER)
{

}

MeshRenderer::~MeshRenderer()
{

}

void MeshRenderer::SetMaterial(shared_ptr<Material> material, uint32 idx)
{
	if (_materials.size() <= static_cast<size_t>(idx))
		_materials.resize(static_cast<size_t>(idx + 1));

	_materials[idx] = material;
}

void MeshRenderer::Render()
{
	for (uint32 i = 0; i < _materials.size(); i++)
	{
		shared_ptr<Material>& material = _materials[i];

		if (material == nullptr || material->GetShader() == nullptr)
			continue;

		GetTransform()->PushData();

		// 만약 애니메이터가 있다면
		if (GetAnimator())
		{
			GetAnimator()->PushData();
			material->SetInt(1, 1);
		}

		material->PushGraphicsData();
		_mesh->Render(1, i);
	}
}

void MeshRenderer::Render(shared_ptr<InstancingBuffer>& buffer)
{
	for (uint32 i = 0; i < _materials.size(); i++)
	{
		shared_ptr<Material>& material = _materials[i];

		if (material == nullptr || material->GetShader() == nullptr)
			continue;

		buffer->PushData();

		// 만약 애니메이터가 있다면
		if (GetAnimator())
		{
			GetAnimator()->PushData();
			material->SetInt(1, 1);
		}

		material->PushGraphicsData();
		_mesh->Render(1, i);
	}
}

uint64 MeshRenderer::GetInstanceID()
{
	if (_mesh == nullptr || _materials.empty())
		return 0;

	InstanceID instanceID{ _mesh->GetID(), _materials[0]->GetID()};
	return instanceID.id;
}

// 한 물체당 그림자를 그린다.
void MeshRenderer::RenderShadow()
{
	GetTransform()->PushData();
	GET_SINGLE(Resources)->Get<Material>(L"Shadow")->PushGraphicsData();
	_mesh->Render();
}