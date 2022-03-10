#include "pch.h"
#include "Audio.h"

DEFINITION_SINGLE(Audio)

Audio::Audio() {}
Audio::~Audio() {}

bool Audio::Init()
{
	System_Create(&m_pSystem);

	m_pSystem->init(10, FMOD_INIT_NORMAL, nullptr);

	return true;
}

bool Audio::LoadSound(const string& strKey, bool bLoop,
	const char* pFileName, const string& strPathKey)
{
	// 중복을 피하자
	if (FindSound(strKey))
	{
		return true;
	}

	string strFullPath;
	strFullPath += strPathKey;
	strFullPath += pFileName;

	FMOD_MODE eMode = FMOD_LOOP_NORMAL;

	//  루프가 아니라면
	if (!bLoop)
	{
		eMode = FMOD_DEFAULT;
	}

	PSOUNDINFO pSoundInfo = new SOUNDINFO;

	pSoundInfo->bLoop = bLoop;
	m_pSystem->createSound(strFullPath.c_str(), eMode, nullptr, &pSoundInfo->pSound);

	m_mapSound.insert(make_pair(strKey, pSoundInfo));

	return true;
}

bool Audio::Play(const string& strKey)
{
	PSOUNDINFO pSound = FindSound(strKey);

	if (!pSound)
	{
		return false;
	}

	m_pSystem->update();

	SOUND_TYPE eType = ST_BGM;

	if (!pSound->bLoop)
	{
		eType = ST_EFFECT;
	}

	m_pSystem->playSound(pSound->pSound, nullptr, false, &m_pChannel[eType]);

	return true;
}

void Audio::Stop(SOUND_TYPE eType)
{
	m_pChannel[eType]->stop();
}

void Audio::Volume(SOUND_TYPE eType, float fVolume)
{
	m_pChannel[eType]->setVolume(fVolume);
}

PSOUNDINFO Audio::FindSound(const string& strKey)
{
	unordered_map<string, PSOUNDINFO>::iterator iter = m_mapSound.find(strKey);

	if (iter == m_mapSound.end())
	{
		return nullptr;
	}

	return iter->second;
}
