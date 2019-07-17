#pragma once

#include "stdafx.h"
#include "Interfaces.h"
#include "resource.h"

struct WavHeader
    {
        // WAV-������ ���������� � RIFF-���������:

        // �������� ������� "RIFF" � ASCII ���������
        // (0x52494646 � big-endian �������������)
        char m_cChunkId[4];

        // 36 + subchunk2Size, ��� ����� �����:
        // 4 + (8 + subchunk1Size) + (8 + subchunk2Size)
        // ��� ���������� ������ �������, ������� � ���� �������.
        // ����� ������, ��� ������ ����� - 8, �� ����,
        // ��������� ���� chunkId � chunkSize.
        DWORD m_dwChunkSize;

        // �������� ������� "WAVE"
        // (0x57415645 � big-endian �������������)
        char m_cFormat[4];

        // ������ "WAVE" ������� �� ���� ����������: "fmt " � "data":
        // ���������� "fmt " ��������� ������ �������� ������:
    
        // �������� ������� "fmt "
        // (0x666d7420 � big-endian �������������)
        char m_cSubchunk1Id[4];

        // 16 ��� ������� PCM.
        // ��� ���������� ������ ����������, ������� � ���� �������.
        DWORD m_dwSubchunk1Size;

        // ����� ������, ������ ������ ����� �������� ����� http://audiocoding.ru/wav_formats.txt
        // ��� PCM = 1 (�� ����, �������� �����������).
        // ��������, ������������ �� 1, ���������� ��������� ������ ������.
        WORD m_wAudioFormat;

        // ���������� �������. ���� = 1, ������ = 2 � �.�.
        WORD m_wNumChannels;

        // ������� �������������. 8000 ��, 44100 �� � �.�.
        DWORD m_dwSampleRate;

        // sampleRate * numChannels * bitsPerSample/8
        DWORD m_dwByteRate;

        // numChannels * bitsPerSample/8
        // ���������� ���� ��� ������ ������, ������� ��� ������.
        WORD m_wBlockAlign;

        // ��� ���������� "��������" ��� �������� ��������. 8 ���, 16 ��� � �.�.
        WORD m_wBitsPerSample;

        // ���������� "data" �������� �����-������ � �� ������.

        // �������� ������� "data"
        // (0x64617461 � big-endian �������������)
        char m_cSubchunk2Id[4];

        // numSamples * numChannels * bitsPerSample/8
        // ���������� ���� � ������� ������.
        DWORD m_dwSubchunk2Size;

        // ����� ������� ��������������� Wav ������.
    };



// ������������� wav ����� � ���������
//  ������ ������ �� ����� ������������.
class Audio: public ISignal
{

private:
	//��������� wav �����
    WAVEFORMATEX g_sDataDesc;

	    //��������� �����
    IXAudio2SourceVoice* g_pSource;

	//������ � ���������� � ����� ������ ��� ����������
    XAUDIO2_BUFFER g_sBuffer;

	    //�������� ����� ���������
    IXAudio2* g_pAudio;

	    //�������� �����
    IXAudio2MasteringVoice* g_pMasteringVoice;

	    //����� �����
    HANDLE v_hFile;

	    //��������� ��������� wav �����
    WavHeader v_sWavHeader;

	    //������ ������ �� �����
    BYTE* g_pBuffer;

	    //�������� �����
    DWORD v_dwReadBytes;

public:

public:
	Audio()
	{
				CoInitializeEx(0,COINIT_MULTITHREADED);
	}

	int Initialize()
	{
		Load(L"C:\conga_2.wav");
		Play();
		return 0;
	}

	int Load(LPCWSTR filename)
	{
                
                if(FAILED(XAudio2Create(&g_pAudio)))
                {
                    MessageBox(NULL, L"Can't create main audio interface.", L"Error", MB_OK);
                    return 1;
                }
                g_pAudio->CreateMasteringVoice(&g_pMasteringVoice);
                		
				HRSRC src = FindResource(NULL,MAKEINTRESOURCE(DefaultWave), RT_RCDATA);
				HGLOBAL gl = LoadResource(NULL, src);
				BYTE* wave = (BYTE*)LockResource(gl);
				
				memcpy(&v_sWavHeader, wave, sizeof(WavHeader));
				g_pBuffer = new BYTE[v_sWavHeader.m_dwChunkSize - 36];
				memcpy(g_pBuffer, wave + 36, v_sWavHeader.m_dwChunkSize - 36);

				
                
     //           //��������� ���� � ��������� ���������� �� ����� �����
     //           v_hFile=CreateFile(filename,
					//GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,0);
     //           ReadFile(v_hFile,(void*)&v_sWavHeader,sizeof(WavHeader),&v_dwReadBytes,NULL);
     //           //������� ������ ��� ������
     //           g_pBuffer=new BYTE[v_sWavHeader.m_dwChunkSize-36];
     //           //�������� ����� ������
     //           ReadFile(v_hFile,(void*)g_pBuffer,v_sWavHeader.m_dwChunkSize-36,&v_dwReadBytes,NULL);
     //           CloseHandle(v_hFile);

                //�������� ��������� ��� ������2
                g_sDataDesc.wFormatTag=1;
                g_sDataDesc.nChannels=v_sWavHeader.m_wNumChannels;
                g_sDataDesc.nSamplesPerSec=v_sWavHeader.m_dwSampleRate;
                g_sDataDesc.nAvgBytesPerSec=v_sWavHeader.m_dwByteRate;
                g_sDataDesc.nBlockAlign=v_sWavHeader.m_wBlockAlign;
                g_sDataDesc.wBitsPerSample=v_sWavHeader.m_wBitsPerSample;
                g_sDataDesc.cbSize=18;
                g_sBuffer.Flags=XAUDIO2_END_OF_STREAM;
                g_sBuffer.AudioBytes=v_sWavHeader.m_dwChunkSize-36;
                g_sBuffer.pAudioData=g_pBuffer;
                g_sBuffer.PlayBegin=0;
                g_sBuffer.PlayLength=0;
                g_sBuffer.LoopBegin=0;
                g_sBuffer.LoopLength=0;
                g_sBuffer.LoopCount=XAUDIO2_LOOP_INFINITE;

                //������ ��������
                g_pAudio->CreateSourceVoice(&g_pSource,&g_sDataDesc);

                //������� ���������� � ������
                g_pSource->SubmitSourceBuffer(&g_sBuffer);
				return 0;
	}
	void Play()
	{
                g_pSource->Start();
	}

	WORD GetFrequency()
	{
					XAUDIO2_VOICE_STATE state;
					g_pSource->GetState( &state );
					//int s = g_sDataDesc.nBlockAlign;
					int s = v_sWavHeader.m_wBitsPerSample;
					int k = state.SamplesPlayed % g_sBuffer.AudioBytes;
					WORD c = g_pBuffer[k];
					if (s == 16) //16 bit
					{
						c = c<<8;
						c += g_pBuffer[k+1];
					}	
					return c;
	}

	LPWORD GetFrequency(int num)
	{
		LPWORD lw = new WORD[num];
		XAUDIO2_VOICE_STATE state;
		g_pSource->GetState( &state );
		int s = g_sDataDesc.nBlockAlign;
		int k = state.SamplesPlayed % g_sBuffer.AudioBytes;
		//if (k + num > )
		switch(v_sWavHeader.m_wBitsPerSample)
		{
		case 8:
			{
				for (int i = 0; i < num; i++)
					lw[i] = g_pBuffer[k + i];
				break;
			}
		case 16:
			{
				for (int i = 0; i < num; i++)
				{
					lw[i] = g_pBuffer[k + s*i];
					lw[i] <<= 8;
					lw[i] += g_pBuffer[k + s*i + 1];
				}
				break;
			}
		}
		return lw;	
	}

	int* GetSignal(int num, int channel)
	{
		int* t = (int*)GetFrequency256(num);
		return t;
	}

	WORD GetFrequency256()
	{
					XAUDIO2_VOICE_STATE state;
					g_pSource->GetState( &state );
					//int s = g_sDataDesc.nBlockAlign;
					int s = v_sWavHeader.m_wBitsPerSample;
					int k = state.SamplesPlayed % g_sBuffer.AudioBytes;
					WORD c = g_pBuffer[k];	
					return c;
	}
	int* GetFrequency256(int num)
	{
		int i;
		int* lw = new int[num];
		XAUDIO2_VOICE_STATE state;
		g_pSource->GetState( &state );
		int s = g_sDataDesc.nBlockAlign;
		int k = state.SamplesPlayed % g_sBuffer.AudioBytes;
		switch(v_sWavHeader.m_wBitsPerSample)
		{
		case 8:
			for (i = 0; i < num && i < g_sBuffer.AudioBytes; i++)
				lw[i] = g_pBuffer[k - i];
			for (int h = i; i < num; i++)
				lw[i] = g_pBuffer[i-h];
			break;

		case 16:
			for (i = 0; i < num && i < g_sBuffer.AudioBytes; i++)
					lw[i] = g_pBuffer[k - s*i];
			for (int h = i; i < num; i++)
				lw[i] = g_pBuffer[i*s-h];
			break;
		}
		return lw;	
	}

	//int Read()
	//{
 //   
 //               //MessageBox(NULL, L"Stop.", L"Message", MB_OK);
	//	unsigned int s,k,c;
	//			bool con = TRUE;
	//			while(con)
	//			{
	//				//XAUDIO2_VOICE_STATE state;
	//				//g_pSource->GetState( &state );
	//				//s = g_sDataDesc.nBlockAlign;
	//				//k = state.SamplesPlayed % g_sBuffer.AudioBytes;
	//				//c = g_pBuffer[k];
	//				//for (int i=1; i<s; i++)
	//				//{
	//				//	c = c<<8;
	//				//	c += g_pBuffer[k+i];
	//				//}
	//				//for (int i = 0; i < c/10000; i++)
	//					//printf(".");
	//				WORD c = *GetFrequency(1);
	//				printf("%d\n", c);
	//				//printf("%d\n", c);
	//				con = !GetAsyncKeyState( VK_ESCAPE );
	//				Sleep(10);
	//			}

 //               //g_pSource->Stop();
 //               //g_pSource->DestroyVoice();
 //               //g_pMasteringVoice->DestroyVoice();
 //               //g_pAudio->Release();
 //               //CoUninitialize();
 //               return 0;
	//}  

	void Stop()
	{
		g_pSource->Stop();
	}

	int Destroy()
	{
		Release();
		return 0;
	}

	void Release()
	{
		if (g_pSource != NULL)
		{
	            g_pSource->Stop();
                g_pSource->DestroyVoice();
		}
		if (g_pMasteringVoice != NULL)
                g_pMasteringVoice->DestroyVoice();
		if (g_pAudio != NULL)
                g_pAudio->Release();
	}

	~Audio()
	{
		Release();
		CoUninitialize();
	}

};