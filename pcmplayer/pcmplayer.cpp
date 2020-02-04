#include <cstdio>
#include "classdmsplayer2.h"
HWND hwnd = 0;
DSBGMPlayer *Player;
//DSPlayer *SPlayer;
DSCreated *dc;
LPBYTE fileBuffer = NULL;
DmspSound Sound;
OPENFILENAMEW opfn;
WCHAR Str1[1024] = L"APCM Files | *.pcm\0*.pcm\0Wave Files | *.wav\0*.wav\0Game PCM | *.dat\0*.dat\0All Files | *\0*\0\0";
WCHAR Str2[4096] = { 0 };
INT64 fsize;
FILE* fp;
double mtime = 0.0;
void AfterPlay() 
{
	double select;
	INT32 temp;
	mtime = fsize * 1.0 / DSBGMPlayer::nAvgBytesPerSecond;
	printf("音频总时长 Total time %.1lf 秒 Seconds，输入数字可从对应秒数位置开始播放;输入负数将返回主菜单。\nNumber: Jump to (Seconds); Minus: Back to menu\n\n", mtime);
	while (1)
	{
		select = 0.0;
		scanf("%lf", &select);
		if (select < -0.0000001) 
		{

			return;
		}
		temp = (INT32)(select*DSBGMPlayer::nAvgBytesPerSecond);
		temp = min(temp,fsize- DSBGMPlayer::nAvgBytesPerSecond);
		temp = max(temp, 0);
		temp -= temp % 4;
		printf("从 From %.1lf 秒 Seconds 跳转到 Jump to %.1lf 秒 Seconds \n\n", Player->MCursor* 1.0 / DSBGMPlayer::nAvgBytesPerSecond,min(select,mtime-1.0));
		Player->MCursor = temp;

	}

}
void OpenAndPlay(char* cmdpara = NULL) 
{
	memset(&opfn, 0, sizeof(opfn));
	opfn.lStructSize = sizeof(opfn);
	opfn.hwndOwner = hwnd;
	opfn.lpstrFilter = Str1;
	opfn.lpstrFile = Str2;
	opfn.nMaxFile = 4096;
	if (cmdpara ||GetOpenFileNameW(&opfn))
	{
		fp = NULL;
		if (cmdpara)
		{
			printf("%s\n", cmdpara);
			fp = fopen(cmdpara, "rb");
		}
		else
		{
			wprintf(L"%s\n", Str2);
			fp = _wfopen(Str2, L"rb");
		}
		if (!fp) 
		{
			printf("未能打开文件 Failed，错误代码 Error code：%d\n\n", GetLastError());
		}
		if (fileBuffer)
		{
			Player->Stop();
			while (!(Player->IsSourceBufferSafeToRelease(fileBuffer)));
			delete fileBuffer;
			fileBuffer = NULL;
		}
		_fseeki64(fp, 0, SEEK_END);
		fsize = _ftelli64(fp);
		if (fsize > 0x7fffffff) 
		{
			printf("文件过大，只播放前面一段\n\n");
			fsize = 0x7fffffff;
		}
		_fseeki64(fp, 0, SEEK_SET);
		fileBuffer = new BYTE[fsize];
		fread(fileBuffer, 1, fsize, fp);
		fclose(fp);
		fsize -= fsize % 4;
		Sound.base = fileBuffer;
		Sound.loop = 0;
		Sound.offset = 0;
		Sound.size = (INT32)fsize;
		printf("开始播放 Start Playing\n\n");
		Player->ChangeAndPlay(Sound);
		//
		//SPlayer->Play(Sound);
		AfterPlay();
	}
	else
	{
		printf("未能打开文件 Failed，错误代码 Error code：%d\n\n", GetLastError());
	}

}
int main(int argc ,char** argv)
{
	int select;
	BOOL opencmd = (argc >= 2);
	SetConsoleTitleA("AWPSOFT Sound Track Player");
	while (!hwnd)
	{
		hwnd = FindWindowA("ConsoleWindowClass", "AWPSOFT Sound Track Player");
	}
	dc = new DSCreated(hwnd);
	if (!(dc->GetEnableLevel())) 
	{
		printf("调用声卡失败\n");
		return 0;
	}
	Player = new DSBGMPlayer(dc);
	//SPlayer = new DSPlayer(dc);
	printf("AWPSOFT音轨播放器 支持 16位双声道44100Hz .pcm/.wav/.dat 音轨播放。\n");
	printf("AWPSOFT Sound Track Player for playing stereo-16bits-44100Hz sound tracks (.pcm/.wav/.dat) \n\n");
	while (1) 
	{
		select = 0;
		if (opencmd)
		{
			opencmd = FALSE;
			OpenAndPlay(argv[1]);
		}
		else
		{
			printf("请选择 Select：1.打开 Open  2.退出 Exit\n\n");
			scanf("%d", &select);
			switch (select)
			{
			case 1:
				OpenAndPlay();
				break;
			case 2:
				return 0;
			default:
				break;
			}
		}
		
	}
	return 0;
}
