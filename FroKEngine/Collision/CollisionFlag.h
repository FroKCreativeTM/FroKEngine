#pragma once


enum COLLIDER_TYPE
{
	CT_PIXEL,
	CT_PLANE,
	CT_POINT,
	CT_RECT,
	CT_SPHERE,
	CT_RAY,
	CT_END
};

// ?浹 ????
enum COLLISION_STATE
{
	CS_ENTER,
	CS_STAY,
	CS_LEAVE,
	CS_END
};