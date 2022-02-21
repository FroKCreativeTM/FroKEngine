#pragma once

#include "Component.h"
#include "Frustum.h"

enum class PROJECTION_TYPE
{
	PERSPECTIVE,	// 원근
	ORTHOGRAPHIC,	// 직교
};

class Camera : public Component
{
public : 
	Camera();
	virtual ~Camera();

	virtual void FinalUpdate() override;
	void Render();

	void SetProjectionType(PROJECTION_TYPE type) { _type = type; }
	PROJECTION_TYPE GetProjectionType() { return _type; }

	void SetCullingMaskLayerOnOff(uint8 layer, bool on)
	{
		if (on)
			_cullingMask |= (1 << layer);
		else
			_cullingMask &= ~(1 << layer);
	}

	void SetCullingMaskAll() { SetCullingMask(UINT32_MAX); }
	void SetCullingMask(uint32 mask) { _cullingMask = mask; }
	bool IsCulled(uint8 layer) { return (_cullingMask & (1 << layer)) != 0; }

private : 
	PROJECTION_TYPE _type = PROJECTION_TYPE::PERSPECTIVE;

	float _near = 1.f;
	float _far = 1000.f;
	float _fov = XM_PI / 4.f;
	float _scale = 1.f;

	Matrix _matView = {};
	Matrix _matProjection = {};

	Frustum _frustum;

	// 레이어에 따라서 culling
	// bit flag
	uint32 _cullingMask = 0;

public : 
	// TEMP
	static Matrix S_MatView;
	static Matrix S_MatProjection;
};

