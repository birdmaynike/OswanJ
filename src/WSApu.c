﻿/*
$Date: 2014-02-06 20:35:44 +0900 (2014/02/06 (木)) $
$Rev: 104 $
*/

#include <windows.h>
#include <mmsystem.h>
#include "../resource.h"
#include "WSHard.h"
#include "WSApu.h"
#include "WSError.h"
#include "WSInput.h"

#define BUFSIZEN    0x10000
#define BPSWAV      12000 // WSのHblankが12KHz
#define BUFSIZEW    64
#define BUFCOUNT    30

unsigned long WaveMap;
SOUND Ch[4];
int VoiceOn;
SWEEP Swp;
NOISE Noise;
int Sound[7] = {1, 1, 1, 1, 1, 1, 1};
int WsWaveVol = 4;

static unsigned char PData[4][32];
static unsigned char PDataN[8][BUFSIZEN];
static unsigned int RandData[BUFSIZEN];

static HWAVEOUT hWaveOut;
static WAVEHDR  *whdr[BUFCOUNT];
static int      rBuf, wBuf, wPos;

extern HWND hWnd;
extern BYTE *Page[16];
extern BYTE IO[0x100];



void CALLBACK apuWaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    switch (uMsg)
    {
    case WOM_OPEN:
        break;
    case WOM_DONE:
        rBuf++;
        if (rBuf >= BUFCOUNT)
        {
            rBuf = 0;
        }
        break;
    case WOM_CLOSE:
        break;
    }
}

int apuWaveCreate(void)
{
    MMRESULT        result;
    WAVEFORMATEX    wfe;
    int             i;
    TCHAR msg[255];

    rBuf = 0;
    wBuf = 0;
    wPos = 0;
    ZeroMemory(&wfe, sizeof(wfe));
    wfe.wFormatTag = WAVE_FORMAT_PCM;
    wfe.nChannels = 2;
    wfe.wBitsPerSample = 16;
    wfe.nBlockAlign = wfe.nChannels * wfe.wBitsPerSample / 8;
    wfe.nSamplesPerSec = BPSWAV;
    wfe.nAvgBytesPerSec = wfe.nSamplesPerSec * wfe.nBlockAlign;
    result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfe, (DWORD_PTR)apuWaveOutProc, (DWORD_PTR)NULL, CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR )
    {
        waveOutGetErrorText(result, msg, sizeof(msg));
        ErrorMsg(ERR_WAVEOUT_OPEN);
    }
    for (i = 0; i < BUFCOUNT; i++)
    {
        whdr[i] = (WAVEHDR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WAVEHDR));
        whdr[i]->lpData = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BUFSIZEW * 4);
        whdr[i]->dwBufferLength = BUFSIZEW * 4;
        whdr[i]->dwFlags = 0;
        whdr[i]->dwLoops = 0;
        result = waveOutPrepareHeader(hWaveOut, whdr[i], sizeof(WAVEHDR));
        if (result != MMSYSERR_NOERROR )
        {
            waveOutGetErrorText(result, msg, sizeof(msg));
            ErrorMsg(ERR_WAVEOUT_PREPAREHEADER);
        }
    }
    return 0;
}

void apuWaveRelease(void)
{
    int i;

    waveOutReset(hWaveOut);
    for(i = 0; i < BUFCOUNT; i++)
    {
        if (whdr[i])
        {
            waveOutUnprepareHeader(hWaveOut,whdr[i],sizeof(WAVEHDR));
            HeapFree(GetProcessHeap(),0,whdr[i]->lpData);
            HeapFree(GetProcessHeap(),0,whdr[i]);
            whdr[i] = NULL;
        }
    }
    waveOutClose(hWaveOut);
    return;
}

void apuWaveClear(void)
{
    int i;

    waveOutReset(hWaveOut);
    for (i = 0; i < BUFCOUNT; i++)
    {
        ZeroMemory(whdr[i]->lpData, BUFSIZEW * 4);
    }
    rBuf = 0;
    wBuf = 0;
    wPos = 0;
}

int apuInit(void)
{
    int i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 32; j++)
        {
            PData[i][j] = 8;
        }
    }
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < BUFSIZEN; j++)
        {
            PDataN[i][j] = ((apuMrand(15 - i) & 1) ? 15 : 0);
        }
    }

    for (i = 0; i < BUFSIZEN; i++)
    {
        RandData[i] = apuMrand(15);
    }
    apuWaveCreate();
    return 0;
}

void apuEnd(void)
{
    apuWaveRelease();
    apuDestroySound();
}

unsigned int apuMrand(unsigned int Degree)
{
#define BIT(n) (1<<n)
    typedef struct
    {
        unsigned int N;
        int InputBit;
        int Mask;
    } POLYNOMIAL;

    static POLYNOMIAL TblMask[]=
    {
        { 2,BIT(2) ,BIT(0)|BIT(1)},
        { 3,BIT(3) ,BIT(0)|BIT(1)},
        { 4,BIT(4) ,BIT(0)|BIT(1)},
        { 5,BIT(5) ,BIT(0)|BIT(2)},
        { 6,BIT(6) ,BIT(0)|BIT(1)},
        { 7,BIT(7) ,BIT(0)|BIT(1)},
        { 8,BIT(8) ,BIT(0)|BIT(2)|BIT(3)|BIT(4)},
        { 9,BIT(9) ,BIT(0)|BIT(4)},
        {10,BIT(10),BIT(0)|BIT(3)},
        {11,BIT(11),BIT(0)|BIT(2)},
        {12,BIT(12),BIT(0)|BIT(1)|BIT(4)|BIT(6)},
        {13,BIT(13),BIT(0)|BIT(1)|BIT(3)|BIT(4)},
        {14,BIT(14),BIT(0)|BIT(1)|BIT(4)|BIT(5)},
        {15,BIT(15),BIT(0)|BIT(1)},
        { 0,      0,      0},
    };
    static POLYNOMIAL *pTbl = TblMask;
    static int ShiftReg = BIT(2)-1;
    int XorReg = 0;
    int Masked;

    if(pTbl->N != Degree)
    {
        pTbl = TblMask;
        while(pTbl->N) {
            if(pTbl->N == Degree)
            {
                break;
            }
            pTbl++;
        }
        if(!pTbl->N)
        {
            pTbl--;
        }
        ShiftReg &= pTbl->InputBit-1;
        if(!ShiftReg)
        {
            ShiftReg = pTbl->InputBit-1;
        }
    }
    Masked = ShiftReg & pTbl->Mask;
    while(Masked)
    {
        XorReg ^= Masked & 0x01;
        Masked >>= 1;
    }
    if(XorReg)
    {
        ShiftReg |= pTbl->InputBit;
    }
    else
    {
        ShiftReg &= ~pTbl->InputBit;
    }
    ShiftReg >>= 1;
    return ShiftReg;
}

void apuSetPData(int addr, unsigned char val)
{
    int i, j;

    i = (addr & 0x30) >> 4;
    j = (addr & 0x0F) << 1;
    PData[i][j]=(unsigned char)(val & 0x0F);
    PData[i][j + 1]=(unsigned char)((val & 0xF0)>>4);
}

unsigned char apuVoice(void)
{
    static int index = 0, b = 0;
    unsigned char v;

    if ((IO[SDMACTL] & 0x98) == 0x98) { // Hyper voice
        v = Page[IO[SDMASH] + b][*(WORD*)(IO + SDMASL) + index++];
        if ((*(WORD*)(IO + SDMASL) + index) == 0)
        {
            b++;
        }
        if (v < 0x80)
        {
            v += 0x80;
        }
        else
        {
            v -= 0x80;
        }
        if (*(WORD*)(IO+SDMACNT) <= index)
        {
            index = 0;
            b = 0;
        }
        return v;
    }
    else if ((IO[SDMACTL] & 0x88) == 0x80) { // DMA start
        IO[SND2VOL] = Page[IO[SDMASH] + b][*(WORD*)(IO + SDMASL) + index++];
        if ((*(WORD*)(IO + SDMASL) + index) == 0)
        {
            b++;
        }
        if (*(WORD*)(IO + SDMACNT) <= index)
        {
            IO[SDMACTL] &= 0x7F; // DMA end
            *(WORD*)(IO + SDMACNT) = 0;
            index = 0;
            b = 0;
        }
    }
    return ((VoiceOn && Sound[4]) ? IO[SND2VOL] : 0x80);
}

void apuSweep(void)
{
    if ((Swp.step) && Swp.on) // sweep on
    {
        if (Swp.cnt < 0)
        {
            Swp.cnt = Swp.time;
            Ch[2].freq += Swp.step;
            Ch[2].freq &= 0x7ff;
        }
        Swp.cnt--;
    }
}

WORD apuShiftReg(void)
{
    static int nPos = 0;
    // Noise counter
    if (++nPos >= BUFSIZEN)
    {
        nPos = 0;
    }
    return RandData[nPos];
}

void apuWaveSet(void)
{
    static  int point[] = {0, 0, 0, 0};
    static  int preindex[] = {0, 0, 0, 0};
    int     channel, index;
    short   value, lVol[4], rVol[4];
    short   LL, RR, vVol;
    static int wait = 0;
    short   *dataAdr;

    if (WsInputGetNowait())
    {
        wait = 1;
    }
    else if (wait)
    {
        wait = 0;
        apuWaveClear();
    }
    apuSweep();
    for (channel = 0; channel < 4; channel++)
    {
        lVol[channel] = 0;
        rVol[channel] = 0;
        if (Ch[channel].on)
        {
            if (channel == 1 && VoiceOn && Sound[4])
            {
                continue;
            }
            else if (channel == 2 && Swp.on && !Sound[5])
            {
                continue;
            }
            else if (channel == 3 && Noise.on && Sound[6])
            {
                index = (3072000 / BPSWAV) * point[3] / (2048 - Ch[3].freq);
                if ((index %= BUFSIZEN) == 0 && preindex[3])
                {
                    point[3] = 0;
                }
                value = (short)PDataN[Noise.pattern][index] - 8;
            }
            else if (Sound[channel] == 0)
            {
                continue;
            }
            else {
                index = (3072000 / BPSWAV) * point[channel] / (2048 - Ch[channel].freq);
                if ((index %= 32) == 0 && preindex[channel])
                {
                    point[channel] = 0;
                }
                value = (short)PData[channel][index] - 8;
            }
            preindex[channel] = index;
            point[channel]++;
            lVol[channel] = value * Ch[channel].volL; // -8*15=-120, 7*15=105
            rVol[channel] = value * Ch[channel].volR;
        }
    }
    vVol = ((short)apuVoice() - 0x80);
    // mix 16bits wave -32768 ～ +32767 32768/120 = 273
    LL = (lVol[0] + lVol[1] + lVol[2] + lVol[3] + vVol) * WsWaveVol;
    RR = (rVol[0] + rVol[1] + rVol[2] + rVol[3] + vVol) * WsWaveVol;
    dataAdr = (short*)(whdr[wBuf]->lpData);
    dataAdr[wPos * 2] = LL;
    dataAdr[wPos * 2 + 1] = RR;
    if (++wPos >= BUFSIZEW)
    {
        waveOutWrite(hWaveOut, whdr[wBuf], sizeof(WAVEHDR));
        wPos = 0;
        if (++wBuf >= BUFCOUNT)
        {
            wBuf = 0;
        }
        if (wait == 0)
        {
            while (wBuf == rBuf) // 書き込みが再生に追いついた
            {
                Sleep(1); // WaveOutのコールバック関数でrBufが更新されるまで待つ
            }
        }
    }
}

/*
PlaySound()は音量調節が出来ないのでいったんメモリに読み込んで
プレイ時にデータを加工する
*/
static void* SSMasterBuf;
static void* SSPlayBuf;
static DWORD SSDataLen, SSHeadLen;

void apuLoadSound(void)
{
    HINSTANCE hInst;
    HRSRC hRsrc;
    HGLOBAL hG;
    BYTE* wave;
    BYTE* tmp;
    DWORD wfesize;

    hInst = (HINSTANCE)GetWindowLong(hWnd, GWLP_HINSTANCE);
    hRsrc = FindResource(hInst, MAKEINTRESOURCE(IDR_WAVE1), TEXT("WAVE"));
    hG = LoadResource(hInst, hRsrc);
    wave = (BYTE*)LockResource(hG); //RIFFヘッダの確認
    tmp = wave;
    if (*(LPDWORD)wave != *(LPDWORD)"RIFF")
    {
        ErrorMsg(ERR_WAVERESOURCE);
        return;
    }
    wave += 4; //ポインタは次を指しておく
    wave += 4; //ポインタは次を指しておく
    //WAVEヘッダの確認
    if (*(LPDWORD)wave != *(LPDWORD)"WAVE")
    {
        ErrorMsg(ERR_WAVERESOURCE);
        return;
    }
    wave += 4; //ポインタは次を指しておく
    //fmtチャンクの確認
    if (*(LPDWORD)wave != *(LPDWORD)"fmt ")
    {
        ErrorMsg(ERR_WAVERESOURCE);
        return;
    }
    wave += 4; //ポインタは次を指しておく
    //fmtチャンクサイズの取得
    wfesize = *(DWORD *)wave;
    wave += 4; //ポインタは次を指しておく
    wave += wfesize; //WAVEFORMATEX構造体分だけポインタを進めておく
    //dataチャンクの確認
    if (*(LPDWORD)wave != *(LPDWORD)"data")
    {
        ErrorMsg(ERR_WAVERESOURCE);
        return;
    }
    wave += 4; //ポインタは次を指しておく
    //波形データのバイト数の取得
    SSDataLen = *(DWORD *)wave;
    wave += 4; //ポインタは次を指しておく
    SSHeadLen = wave - tmp;
    SSMasterBuf = malloc(SSDataLen);
    SSPlayBuf = malloc(SSHeadLen + SSDataLen);
    if (SSMasterBuf != NULL && SSPlayBuf != NULL)
    {
        memcpy(SSMasterBuf, wave, SSDataLen);
        memcpy(SSPlayBuf, tmp, SSHeadLen);
    }
    FreeResource(hG);
}

void apuDestroySound(void)
{
    free(SSMasterBuf);
    SSMasterBuf = NULL;
    free(SSPlayBuf);
    SSPlayBuf = NULL;
}

void apuStartupSound(void)
{
    short *src, *dst;
    int i;
    int size = SSDataLen / sizeof(short);

    if (SSMasterBuf == NULL || SSPlayBuf == NULL)
    {
        return;
    }
    src = (short*)SSMasterBuf;
    dst = (short*)((BYTE*)SSPlayBuf + SSHeadLen);
    // マスターの音量を変更してプレイバッファーにコピー
    for (i = 0; i < size; i++)
    {
        *dst++ = (*src++) * WsWaveVol / 32;
    }
    PlaySound((LPCTSTR)SSPlayBuf, NULL, SND_MEMORY | SND_ASYNC);
}
