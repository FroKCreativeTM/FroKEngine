#pragma once

// �������� ���� ������ Ÿ���� �����մϴ�.

// �ػ� ���� ������ �����մϴ�.
typedef struct _tagResolution
{
	unsigned int nWidth;
	unsigned int nHeight;

	_tagResolution() :
		nWidth(0), nHeight(0) {}

	_tagResolution(int x, int y) :
		nWidth(x), nHeight(y) {}
}RESOLUTION, * PRESOLUTION;