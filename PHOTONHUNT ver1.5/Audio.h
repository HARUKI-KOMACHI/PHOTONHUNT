#pragma once

#include <xaudio2.h>

void InitAudio();
void UninitAudio();

int LoadAudio(const wchar_t* FileName);
void UnloadAudio(int Index);
void PlayAudio(int Index, bool Loop = false);

void PauseAudio(int Index);

void ResumeAudio(int Index);

void StopAudio(int Index);

void SetAudioPan(int Index, float leftVolume, float rightVolume);

void UpdateAudioCircle(int Index, float time, float speed);