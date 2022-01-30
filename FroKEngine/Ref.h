#pragma once

#include "Game.h"

using namespace std;

class Ref
{
public:
	void AddRef() { ++m_nRef; }
	int Release()
	{
		--m_nRef;

		if (m_nRef == 0)
		{
			delete this;
			return 0;
		}
		return m_nRef;
	}

public:
	void SetEnable(bool bEnable)
	{
		this->m_bEnable = bEnable;
	}

	void Die()
	{
		this->m_bLife = false;
	}

	bool GetLife() const
	{
		return m_bLife;
	}

	bool GetEnable() const
	{
		return m_bEnable;
	}

	string GetTag() const
	{
		return m_strTag;
	}

	void SetTag(const string& strTag)
	{
		m_strTag = strTag;
	}

protected:
	Ref();
	virtual ~Ref();

protected:
	// 이 오브젝트를 얼마나 참조하냐를 나타내는 지표
	int			m_nRef;
	bool		m_bEnable;	// 객체 활성화(객체는 있지만, 보이냐 안 보이는가)
	bool		m_bLife;	// 살리는가 죽이는가.
	string		m_strTag;	// 태그 이름
};

