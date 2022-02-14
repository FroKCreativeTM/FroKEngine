#pragma once

// ��� Ÿ��
enum SCENE_CREATE
{
	SC_CURRENT,
	SC_NEXT,
	SC_END
};

// ��� ��ȯ ���� �÷���
enum class SCENE_CHANGE : int
{
	SC_NONE,
	SC_CHANGE
};

enum class RenderLayer : int
{
	Opaque = 0,		// ������ü
	Transparent,
	AlphaTested,
	AlphaTestedTreeSprites,
	Count
};