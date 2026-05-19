
#include <d3d11.h>
#include <DirectXMath.h>
#include "shader.h"
#include "sprite.h"
#include "keyboard.h"

#include "audio.h"






static IXAudio2* g_Xaudio{};
static IXAudio2MasteringVoice* g_MasteringVoice{};


void InitAudio()
{
	// XAudio生成
	XAudio2Create(&g_Xaudio, 0);

	// マスタリングボイス生成
	g_Xaudio->CreateMasteringVoice(&g_MasteringVoice);
}


void UninitAudio()
{
	g_MasteringVoice->DestroyVoice();
	g_Xaudio->Release();
}









struct AUDIO
{
	IXAudio2SourceVoice*	SourceVoice{};
	BYTE*					SoundData{};

	int						Length{};
	int						PlayLength{};
};

#define AUDIO_MAX 100
static AUDIO g_Audio[AUDIO_MAX]{};
int oldIndex;



int LoadAudio(const wchar_t *FileName)
{
	int index = -1;

	for (int i = 0; i < AUDIO_MAX; i++)
	{
		if (g_Audio[i].SourceVoice == nullptr)
		{
			index = i;
			break;
		}
	}

	if (index == -1)
		return -1;

	// サウンドデータ読込
	WAVEFORMATEX wfx = { 0 };

	{
		HMMIO hmmio = NULL;
		MMIOINFO mmioinfo = { 0 };
		MMCKINFO riffchunkinfo = { 0 };
		MMCKINFO datachunkinfo = { 0 };
		MMCKINFO mmckinfo = { 0 };
		UINT32 buflen;
		LONG readlen;


		hmmio = mmioOpen(const_cast<LPWSTR>(FileName), &mmioinfo, MMIO_READ);
		assert(hmmio);

		riffchunkinfo.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		mmioDescend(hmmio, &riffchunkinfo, NULL, MMIO_FINDRIFF);

		mmckinfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
		mmioDescend(hmmio, &mmckinfo, &riffchunkinfo, MMIO_FINDCHUNK);

		if (mmckinfo.cksize >= sizeof(WAVEFORMATEX))
		{
			mmioRead(hmmio, (HPSTR)&wfx, sizeof(wfx));
		}
		else
		{
			PCMWAVEFORMAT pcmwf = { 0 };
			mmioRead(hmmio, (HPSTR)&pcmwf, sizeof(pcmwf));
			memset(&wfx, 0x00, sizeof(wfx));
			memcpy(&wfx, &pcmwf, sizeof(pcmwf));
			wfx.cbSize = 0;
		}
		mmioAscend(hmmio, &mmckinfo, 0);

		datachunkinfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
		mmioDescend(hmmio, &datachunkinfo, &riffchunkinfo, MMIO_FINDCHUNK);



		buflen = datachunkinfo.cksize;
		g_Audio[index].SoundData = new unsigned char[buflen];
		readlen = mmioRead(hmmio, (HPSTR)g_Audio[index].SoundData, buflen);


		g_Audio[index].Length = readlen;
		g_Audio[index].PlayLength = readlen / wfx.nBlockAlign;


		mmioClose(hmmio, 0);
	}


	// サウンドソース生成
	g_Xaudio->CreateSourceVoice(&g_Audio[index].SourceVoice, &wfx);
	assert(g_Audio[index].SourceVoice);


	return index;
}




void UnloadAudio(int Index)
{
	g_Audio[Index].SourceVoice->Stop();
	g_Audio[Index].SourceVoice->DestroyVoice();

	delete[] g_Audio[Index].SoundData;
	g_Audio[Index].SoundData = nullptr;
}





void PlayAudio(int Index, bool Loop)
{
	g_Audio[Index].SourceVoice->Stop();
	g_Audio[Index].SourceVoice->FlushSourceBuffers();

	// バッファ設定
	XAUDIO2_BUFFER bufinfo;

	memset(&bufinfo, 0x00, sizeof(bufinfo));
	bufinfo.AudioBytes = g_Audio[Index].Length;
	bufinfo.pAudioData = g_Audio[Index].SoundData;
	bufinfo.PlayBegin = 0;
	bufinfo.PlayLength = g_Audio[Index].PlayLength;

	// ループ設定
	if (Loop)
	{
		bufinfo.LoopBegin = 0;
		bufinfo.LoopLength = g_Audio[Index].PlayLength;
		bufinfo.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	g_Audio[Index].SourceVoice->SubmitSourceBuffer(&bufinfo, NULL);

	// 再生
	g_Audio[Index].SourceVoice->Start();
}


void PauseAudio(int Index)
{
	if (g_Audio[Index].SourceVoice)
	{
		g_Audio[Index].SourceVoice->Stop(0); // 参数0表示立即停止
	}
}

void ResumeAudio(int Index)
{
	if (g_Audio[Index].SourceVoice)
	{
		g_Audio[Index].SourceVoice->Start(0); // 继续播放
	}
}

void StopAudio(int Index)
{
	if (g_Audio[Index].SourceVoice)
	{
		g_Audio[Index].SourceVoice->Stop(0);
		g_Audio[Index].SourceVoice->FlushSourceBuffers(); // 清空缓冲，下次重新从头播放
	}
}

void SetAudioPan(int Index, float leftVolume, float rightVolume)
{
	if (g_Audio[Index].SourceVoice)
	{
		// 限制在 0.0f ~ 1.0f
		if (leftVolume < 0.0f) leftVolume = 0.0f;
		if (leftVolume > 1.0f) leftVolume = 1.0f;
		if (rightVolume < 0.0f) rightVolume = 0.0f;
		if (rightVolume > 1.0f) rightVolume = 1.0f;

		float matrix[2] = { leftVolume, rightVolume };

		// InputChannels = 1 (单声道音源) 或 2 (立体声音源)
		// OutputChannels = 2 (通常耳机/音响左右声道)
		g_Audio[Index].SourceVoice->SetOutputMatrix(
			nullptr,            // 默认输出到主混音器
			1,                  // 输入声道数，这里假设音频是单声道
			2,                  // 输出声道数（立体声）
			matrix              // 声道音量矩阵
		);
	}
}



// 在 update 循环里每帧调用
void UpdateAudioCircle(int Index, float time, float speed)
{
	if (g_Audio[Index].SourceVoice)
	{
		// time = 运行的总时间（秒）
		// speed = 转圈的速度（频率），比如 1.0f = 每秒转一圈

		float angle = time * speed * 2.0f * 3.14159265f;

		// 左右音量在 0~1 范围内波动
		float left = (sinf(angle) + 1.0f) * 0.5f;
		float right = (cosf(angle) + 1.0f) * 0.5f;

		float matrix[2] = { left, right };
		g_Audio[Index].SourceVoice->SetOutputMatrix(
			nullptr, 1, 2, matrix
		);
	}
}
