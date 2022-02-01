#pragma once

enum class RenderLayer : int
{
	Opaque = 0,		// 불투명체
	Transparent,
	AlphaTested,
	AlphaTestedTreeSprites,
	Count
};