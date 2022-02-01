#pragma once

#include "../Game.h"
#include "fmod.hpp"

using namespace FMOD;
using namespace std;

#pragma comment(lib, "fmod64_vc")

typedef struct _tagSoundInfo
{
	Sound* pSound;
	bool		bLoop;
}SOUNDINFO, * PSOUNDINFO;

enum SOUND_TYPE
{
	ST_BGM,
	ST_EFFECT,
	ST_END
};

class SoundManager
{
public:
	bool Init();
	bool LoadSound(const string& strKey,
		bool bLoop,
		const char* pFileName,
		const string& strPathKey = SOUND_PATH);
	bool Play(const string& strKey);
	void Stop(SOUND_TYPE eType);
	void Volume(SOUND_TYPE eType, float fVolume);

private:
	PSOUNDINFO FindSound(const string& strKey);

private:
	System* m_pSystem;
	Channel* m_pChannel[ST_END];
	unordered_map<string, PSOUNDINFO>		m_mapSound;

	DECLARE_SINGLE(SoundManager)
};


