#pragma once

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

class Audio
{
public:
	bool Init();
	bool LoadSound(const string& strKey,
		bool bLoop,
		const char* pFileName,
		const string& strPathKey = "Sound\\");
	bool Play(const string& strKey);
	void Stop(SOUND_TYPE eType);
	void Volume(SOUND_TYPE eType, float fVolume);

private:
	PSOUNDINFO FindSound(const string& strKey);

private:
	System* m_pSystem;
	Channel* m_pChannel[ST_END];
	unordered_map<string, PSOUNDINFO>		m_mapSound;

	DECLARE_SINGLE(Audio)
};


