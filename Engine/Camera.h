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

private : 
	PROJECTION_TYPE _type = PROJECTION_TYPE::PERSPECTIVE;

	float _near = 1.f;
	float _far = 1000.f;
	float _fov = XM_PI / 4.f;
	float _scale = 1.f;

	Matrix _matView = {};
	Matrix _matProjection = {};

	Frustum _frustum;

public : 
	// TEMP
	static Matrix S_MatView;
	static Matrix S_MatProjection;
};

