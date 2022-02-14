#pragma once

// 장면 타입
enum SCENE_CREATE
{
	SC_CURRENT,
	SC_NEXT,
	SC_END
};

// 장면 전환 관련 플래그
enum class SCENE_CHANGE : int
{
	SC_NONE,
	SC_CHANGE
};

enum class RenderLayer : int
{
	Opaque = 0,		// 불투명체
	Transparent,
	AlphaTested,
	AlphaTestedTreeSprites,
	Count
};