#pragma once

enum class RenderLayer : int
{
	Opaque = 0,		// ������ü
	Transparent,
	AlphaTested,
	AlphaTestedTreeSprites,
	Count
};