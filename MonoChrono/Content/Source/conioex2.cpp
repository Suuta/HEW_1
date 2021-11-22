/************************************************
 * @file	conioex2.cpp
 * @brief	�R���\�[�� ���C�u����
 *
 * @author	AT11E ���R�đ�
 ************************************************/

#include "conioex2.h"

#include <string.h>
#include <math.h>
#include <random>


// :::::::::: �O���[�o���ϐ�::::::::::
static conioex2::Engine* GEngine = nullptr;

static bool bIsInit = false;


// :::::::::: Vector2 ::::::::::
Vector2 Vector2::operator-(float scalar)
{
	Vector2 result;
	result.X - scalar;
	result.Y - scalar;
	return result;
}

Vector2 Vector2::operator*(float scalar)
{
	Vector2 result;
	result.X = this->X * scalar;
	result.Y = this->Y * scalar;
	return result;
}

Vector2 Vector2::operator/(float scalar)
{
	Vector2 result;
	result.X = this->X / scalar;
	result.Y = this->Y / scalar;
	return result;
}

Vector2 Vector2::operator/=(float scalar)
{
	*this = *this / scalar;
	return *this;
}

float Vector2::Length(const Vector2& v)
{
	return sqrtf(v.X * v.X + v.Y * v.Y);
}

Vector2 Vector2::Between(const Vector2& v1, const Vector2& v2)
{
	float x = v2.X - v1.X;
	float y = v2.Y - v1.Y;
	return Vector2(x, y);
}

Vector2 Vector2::Normalize(const Vector2& v)
{
	float x = v.X / Length(v);
	float y = v.Y / Length(v);
	return Vector2(x, y);
}

Vector2 Vector2::UnitVector(float degree)
{
	float radian = conioex2::RadianFromDegree(degree);
	float y = sin(radian);
	float x = cos(radian);
	return Vector2(x, y);
}


// :::::::::: XBox�w���p�[ ::::::::::
#define NormalizeStick(value)					(((fabs((value)) / shortMAX)) * ((value) < 0? -1 : 1))
#define NormalizeTrigger(value)					(((fabs((value)) / ByteMAX)))
#define NormarileVibration(value)				((value) / ushortMAX)
#define UnNormarileVibration(value)				((value) * ushortMAX)
#define GetKeyStateFromBitField(field, mask)	(((field) & (mask)) == (mask) ? XBox_Down : XBox_Up)
#define LEFT_STICK_DEAD_ZONE					XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
#define RIGHT_STICK_DEAD_ZONE					XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
#define TRIGGER_DEAD_ZONE						XINPUT_GAMEPAD_TRIGGER_THRESHOLD

void UpdateXBoxButtonState(XBoxInputState* state, XBoxInputState newstate)
{
	switch (*state)
	{
		case XBoxInputState::XBox_Down:
		{
			*state = XBox_Pressed;
			break;
		}
		case XBoxInputState::XBox_Up:
		{
			*state = XBox_Release;
			break;
		}
	}

	if (newstate == XBox_Up)
	{
		if (*state != XBox_Release)
		{
			*state = XBox_Up;
		}
	}
	else if (newstate == XBox_Down)
	{
		if (*state != XBox_Pressed)
		{
			*state = XBox_Down;
		}
	}
}

// :::::::::: �r�b�g�}�b�v�w���p�[ ::::::::::
#define FILEHEADERSIZE 14							// �t�@�C���w�b�_�̃T�C�Y
#define INFOHEADERSIZE 40							// ���w�b�_�̃T�C�Y
#define HEADERSIZE (FILEHEADERSIZE+INFOHEADERSIZE)	// ���v�T�C�Y

struct BMP
{
	unsigned int height;
	unsigned int width;
	Pixel* data;
};

BMP* CreateBMP(int width, int height)
{
	BMP* img = (BMP *)malloc(sizeof(BMP));

	if (img)
	{
		img->data = (Pixel*)malloc(sizeof(Pixel) * width * height);
		img->width = width;
		img->height = height;
		return img;
	}
	else
	{
		return NULL;
	}
}

BMP* LoadBMP(const char* filename)
{
	unsigned int i, j;
	int realWidth;						//�f�[�^���1�s���̃o�C�g��
	unsigned int width, height;			//�摜�̉��Əc�̃s�N�Z����
	unsigned int color;					//��bit��Bitmap�t�@�C���ł��邩
	unsigned char header[HEADERSIZE];	//�w�b�_������荞��
	unsigned char* bmpLineData;			//�摜�f�[�^1�s��
	FILE* fp;
	BMP* img;

	if ((fp = fopen(filename, "rb")) == NULL)
	{
		printf("error: Can't Open file.");
		return NULL;
	}

	//�w�b�_�����S�Ă���荞��
	fread(header, sizeof(unsigned char), HEADERSIZE, fp);

	//�ŏ���2�o�C�g��BM(Bitmap�t�@�C���̈�)�ł��邩
	if (strncmp((const char*)header, "BM", 2))
	{
		printf("error: Not bitmap file.");
		return NULL;
	}

	memcpy(&width,  header + 18, sizeof(width));		//�摜�̌����ڏ�̕����擾
	memcpy(&height, header + 22, sizeof(height));		//�摜�̍������擾
	memcpy(&color,  header + 28, sizeof(unsigned int));	//��bit��Bitmap�ł��邩���擾

	// 24bit����Ȃ�������I������
	if (color != 24)
	{
		printf("error: %s is not 24bit color image.", filename);
		return nullptr;
	}

	//RGB���͉摜��1�s����4byte�̔{���Ŗ�����΂Ȃ�Ȃ����߂���ɍ��킹�Ă���
	realWidth = width * 3 + width % 4;

	//�摜��1�s����RGB��������Ă��邽�߂̃o�b�t�@�𓮓I�Ɏ擾
	bmpLineData = (unsigned char *)malloc(sizeof(unsigned char) * realWidth);
	if (bmpLineData == NULL)
	{
		printf("error: Allocation failed.");
		return NULL;
	}

	//RGB������荞�ނ��߂̃o�b�t�@�𓮓I�Ɏ擾
	img = CreateBMP(width, height);
	if (img == NULL)
	{
		free(bmpLineData);
		fclose(fp);
		return NULL;
	}

	//Bitmap�t�@�C���̏��͍�������E�ցA�������ɕ���ł���
	for (i = 0; i < height; i++)
	{
		fread(bmpLineData, 1, realWidth, fp);
		for (j = 0; j < width; j++)
		{
			img->data[(height - i - 1) * width + j].B = bmpLineData[j * 3];
			img->data[(height - i - 1) * width + j].G = bmpLineData[j * 3 + 1];
			img->data[(height - i - 1) * width + j].R = bmpLineData[j * 3 + 2];
		}
	}

	free(bmpLineData);

	fclose(fp);
	return img;
}

bool CompareColor(COLORREF table, Pixel pixel)
{
	return table == RGB(pixel.R, pixel.G, pixel.B);
}


// :::::::::: �T�E���h�w���p�[ ::::::::::

bool LoadWav(const char* fileName, WavData* data)
{
	HMMIO    hMmio = NULL;
	MMIOINFO mmioInfo;
	MMRESULT mmRes;

	// WAVE�t�@�C���I�[�v��
	memset(&mmioInfo, 0, sizeof(MMIOINFO));
	hMmio = mmioOpenA(const_cast<char*>(fileName), &mmioInfo, MMIO_READ);
	if (hMmio == NULL)
	{
		return false;
	}

	// RIFF�`�����N����
	MMCKINFO riffChunk;
	riffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	mmRes = mmioDescend(hMmio, &riffChunk, NULL, MMIO_FINDRIFF);
	if (mmRes != MMSYSERR_NOERROR)
	{
		mmioClose(hMmio, 0);
		return false;
	}

	// �t�H�[�}�b�g�`�����N����
	MMCKINFO formatChunk;
	formatChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	mmRes = mmioDescend(hMmio, &formatChunk, &riffChunk, MMIO_FINDCHUNK);
	if (mmRes != MMSYSERR_NOERROR)
	{
		mmioClose(hMmio, 0);
		return false;
	}

	// �t�H�[�}�b�g�擾
	DWORD formatSize = formatChunk.cksize;
	DWORD size = mmioRead(hMmio, reinterpret_cast<HPSTR>(&data->Format), formatSize);
	if (size != formatSize)
	{
		mmioClose(hMmio, 0);
		return false;
	}

	// RIFF�`�����N�Ɉړ�
	mmioAscend(hMmio, &formatChunk, 0);

	// �f�[�^�`�����N����
	MMCKINFO dataChunk;
	dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	mmRes = mmioDescend(hMmio, &dataChunk, &riffChunk, MMIO_FINDCHUNK);
	if (mmRes != MMSYSERR_NOERROR)
	{
		mmioClose(hMmio, 0);
		return false;
	}

	// �f�[�^�擾
	data->BufferSize = dataChunk.cksize;
	data->Buffer = new BYTE[data->BufferSize];
	size = mmioRead(hMmio, reinterpret_cast<HPSTR>(data->Buffer), data->BufferSize);
	if (size != dataChunk.cksize)
	{
		data->BufferSize = 0;
		if (data->Buffer != NULL)
		{
			delete[] data->Buffer;
			data->Buffer;
		}
		return false;
	}

	mmioClose(hMmio, 0);
	return true;
}


//inline float SemitonesToFrequencyRatio(float Semitones)
//{
//	// FrequencyRatio = 2 ^ Octaves
//	//                = 2 ^ (Semitones / 12)
//	return powf(2.0f, Semitones / 12.0f);
//}


// :::::::::: conioex2 �֐�::::::::::

namespace conioex2
{
    void* GetOutputHandle()
    {
	    return GEngine->console.hOutput;
    }

    void* GetInputHandle()
    {
	    return GEngine->console.hInput;
    }

    const char* Title()
    {
	    return GEngine->console.Title;
    }

    int Width()
    {
	    return GEngine->console.WindowSize.X;
    }

    int Height()
    {
	    return GEngine->console.WindowSize.Y;
    }

    void InitializeConsole(int width, int height, const char * titleName, int fontWidth, int fontHeight, const WCHAR* fontname)
    {
	    LOG("��conioex2::InitializeConsole().\n");
	    
	    if(bIsInit)
	    {
		    LOG("�E���łɏ���������Ă��܂�.\n");
		    return;
	    }

	    
	    GEngine = new Engine;

	    LOG("�Enew GEngine.\n");

	    GEngine->console.hInput = GetStdHandle(STD_INPUT_HANDLE);
	    GEngine->console.hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	    GEngine->console.WindowSize.X = width;
	    GEngine->console.WindowSize.Y = height;
	    GEngine->console.WindowRect = { 0, 0, static_cast<short>(GEngine->console.WindowSize.X - 1), static_cast<short>(GEngine->console.WindowSize.Y - 1) };

	    // �o�b�t�@�[�̏��
	    GetConsoleScreenBufferInfoEx(GEngine->console.hOutput, &GEngine->console.ScreenBufferInfo);
	    GEngine->console.ScreenBufferInfo.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
	    GEngine->console.ScreenBufferInfo.dwSize = { GEngine->console.WindowSize.X, GEngine->console.WindowSize.Y };
	    GEngine->console.ScreenBufferInfo.srWindow = { 0, 0, (short)GEngine->console.WindowSize.X, (short)GEngine->console.WindowSize.Y };

	    GEngine->console.ScreenBufferInfo.ColorTable[0]  = RGB( 12,  12,  12);
	    GEngine->console.ScreenBufferInfo.ColorTable[1]  = RGB(  0,  55, 218);
	    GEngine->console.ScreenBufferInfo.ColorTable[2]  = RGB( 19, 161,  14);
	    GEngine->console.ScreenBufferInfo.ColorTable[3]  = RGB( 58, 150, 221);
	    GEngine->console.ScreenBufferInfo.ColorTable[4]  = RGB(197,  15,  31);
	    GEngine->console.ScreenBufferInfo.ColorTable[5]  = RGB(136,  23, 152);
	    GEngine->console.ScreenBufferInfo.ColorTable[6]  = RGB(193, 156,   0);
	    GEngine->console.ScreenBufferInfo.ColorTable[7]  = RGB(204, 204, 204);
	    GEngine->console.ScreenBufferInfo.ColorTable[8]  = RGB(118, 118, 118);
	    GEngine->console.ScreenBufferInfo.ColorTable[9]  = RGB( 59, 120, 255);
	    GEngine->console.ScreenBufferInfo.ColorTable[10] = RGB( 22, 198,  12);
	    GEngine->console.ScreenBufferInfo.ColorTable[11] = RGB( 97, 214, 214);
	    GEngine->console.ScreenBufferInfo.ColorTable[12] = RGB(231,  72,  86);
	    GEngine->console.ScreenBufferInfo.ColorTable[13] = RGB(180,   0, 158);
	    GEngine->console.ScreenBufferInfo.ColorTable[14] = RGB(249, 241, 165);
	    GEngine->console.ScreenBufferInfo.ColorTable[15] = RGB(242, 242, 242);

	    // �I�[�o�[�T�C�Y���ɏI��
	    /*if (GEngine->console.ScreenBufferInfo.dwMaximumWindowSize.X < width)
	    {
		    conioex2::Print("�w�肳�ꂽ�R���\�[���T�C�Y�͑傫�����܂��B");
		    return false;
	    }
	    else if (GEngine->console.ScreenBufferInfo.dwMaximumWindowSize.Y < height)
	    {
		    conioex2::Print("�w�肳�ꂽ�R���\�[���T�C�Y�͑傫�����܂��B");
		    return false;
	    }*/


	    strcpy(GEngine->console.Title, titleName);
	    conioex2::SetTitle(titleName);

	    // �T�C�Y�ύX�\�ɂ��邽�߂Ɉ�x�ŏ�������
	    SMALL_RECT rect = { 0, 0, 1, 1 };
	    SetConsoleWindowInfo(GetOutputHandle(), TRUE, &rect);

	    // �J�[�\��
	    GEngine->console.CursorInfo.bVisible = TRUE;
	    GEngine->console.CursorInfo.dwSize = 25;
	    SetConsoleCursorInfo(GetOutputHandle(), &GEngine->console.CursorInfo);

	    // �t�H���g
	    wcscpy(GEngine->console.FontInfo.FaceName, fontname);
	    GEngine->console.FontInfo.cbSize = sizeof(GEngine->console.FontInfo);
	    GEngine->console.FontInfo.nFont = 0;
	    GEngine->console.FontInfo.dwFontSize.X = fontWidth;
	    GEngine->console.FontInfo.dwFontSize.Y = fontHeight;
      //GEngine->console.FontInfo.FontFamily = FF_MODERN << 4 | TMPF_VECTOR | TMPF_TRUETYPE;
	    GEngine->console.FontInfo.FontFamily = FF_DONTCARE;
	    GEngine->console.FontInfo.FontWeight = FW_NORMAL;
	    SetCurrentConsoleFontEx(GetOutputHandle(), false, &GEngine->console.FontInfo);

	    // �o�b�t�@�[�̐ݒ�
	    SetConsoleScreenBufferInfoEx(GetOutputHandle(), &GEngine->console.ScreenBufferInfo);

	    // �o�b�t�@�[�̃T�C�Y
	    SetConsoleScreenBufferSize(GetOutputHandle(), COORD{ (short)GEngine->console.WindowSize.X, (short)GEngine->console.WindowSize.Y });

	    // �E�B���h�E�̏��
	    SetConsoleWindowInfo(GetOutputHandle(), TRUE, &GEngine->console.WindowRect);

	    // ���݂̃o�b�t�@�[��ݒ�
	    SetConsoleActiveScreenBuffer(GetOutputHandle());

    #if 0
	    // �o�b�t�@�[�T�C�Y�̃������m�ہi�m�ۍς݂Ȃ������Ă���m�ہj
	    if (GEngine->console.ScreenBuffer == nullptr)
	    {
		    GEngine->console.ScreenBuffer = new CHAR_INFO[GEngine->console.WindowSize.X * GEngine->console.WindowSize.Y];
		    ::LOG("�Enew GEngine->console.ScreenBuffer.\n");
		    ZeroMemory(GEngine->console.ScreenBuffer, sizeof(CHAR_INFO) * (GEngine->console.WindowSize.X * GEngine->console.WindowSize.Y));
	    }
	    else
	    {
		    delete GEngine->console.ScreenBuffer;
		    GEngine->console.ScreenBuffer = nullptr;
		    ::LOG("�Edelete GEngine->console.ScreenBuffer.\n");

		    GEngine->console.ScreenBuffer = new CHAR_INFO[GEngine->console.WindowSize.X * GEngine->console.WindowSize.Y];
		    ::LOG("�Enew GEngine->console.ScreenBuffer.\n");
		    ZeroMemory(GEngine->console.ScreenBuffer, sizeof(CHAR_INFO) * (GEngine->console.WindowSize.X * GEngine->console.WindowSize.Y));
	    }
    #endif

	    // �o�b�t�@�[�T�C�Y�̃������m��
	    GEngine->console.ScreenBuffer = new CHAR_INFO[GEngine->console.WindowSize.X * GEngine->console.WindowSize.Y];
	    LOG("�Enew GEngine->console.ScreenBuffer.\n");
	    ZeroMemory(GEngine->console.ScreenBuffer, sizeof(CHAR_INFO) * (GEngine->console.WindowSize.X * GEngine->console.WindowSize.Y));


	    // �E�B���h�E�̃��T�C�Y���֎~����
	    HWND consoleWindow = GetConsoleWindow();
	    SetWindowLongPtr(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_SYSMENU);


	    // �R���\�[�����͌`����ݒ�
	    conioex2::ResetConsoleMode();

	    // ���Ԃ̏�����
	    conioex2::InitTime();

	    //�T�E���h�̏�����
	    // conioex2::InitSound();
		AudioManager::Init();

	    // �����F�̏�����
	    conioex2::SetTextAttribute(Color::FONT_WHITE, BACK_BLACK);

	    bIsInit = true;
    }

    void FinalizeConsole()
    {	
	    LOG("��conioex2::FinalizeConsole().\n");
	    if(bIsInit)
	    {
		    
		    if (GEngine)
		    {
			    // conioex2::DestroySound();
				AudioManager::Fin();

			    LOG("��conioex2::DestroySound().\n");

			    if (GEngine->console.ScreenBuffer)
			    {
				    delete GEngine->console.ScreenBuffer;
				    GEngine->console.ScreenBuffer = nullptr;
				    LOG("�Edelete GEngine->console.ScreenBuffer.\n");

				    delete GEngine;
				    GEngine = nullptr;
				    LOG("�Edelete GEngine.\n");
			    }

			    bIsInit = false;
		    }
	    }
	    else
	    {
		    LOG("�E���łɏI���������Ă΂�Ă��܂��B\n");
	    }
    }

    void SetCursorVisibility(bool visibility)
    {
	    visibility ? GEngine->console.CursorInfo.bVisible = true : GEngine->console.CursorInfo.bVisible = false;
	    SetConsoleCursorInfo(GetOutputHandle(), &GEngine->console.CursorInfo);
    }

    void SetCursorLocation(short x, short y)
    {
	    SetConsoleCursorPosition(GetOutputHandle(), COORD{x, y});
    }

    void SetTitle(const char* title)
    {
	    SetConsoleTitleA(title);
    }

    void SetTextAttribute(Color font, Color background)
    {
	    SetConsoleTextAttribute(GetOutputHandle(), static_cast<int>(font) + static_cast<int>(background));
    }

    void SetColorTable(int index, COLORREF color)
    {
	    if (index < 0)  index = 0;
	    if (index > 15) index = 15;

	    GetConsoleScreenBufferInfoEx(GetOutputHandle(), &GEngine->console.ScreenBufferInfo);
	    GEngine->console.ScreenBufferInfo.ColorTable[index] = color;

	    // �ĂԂ��тɃE�B���h�E�T�C�Y�ς�����Ⴄ����Ή�
	    GEngine->console.ScreenBufferInfo.srWindow.Bottom++;
	    GEngine->console.ScreenBufferInfo.srWindow.Right++;

	    SetConsoleScreenBufferInfoEx(GetOutputHandle(), &GEngine->console.ScreenBufferInfo);
    }

    void ResetColorTable()
    {
	    GetConsoleScreenBufferInfoEx(GetOutputHandle(), &GEngine->console.ScreenBufferInfo);

	    GEngine->console.ScreenBufferInfo.ColorTable[0]  = RGB( 12,  12,  12);
	    GEngine->console.ScreenBufferInfo.ColorTable[1]  = RGB(  0,  55, 218);
	    GEngine->console.ScreenBufferInfo.ColorTable[2]  = RGB( 19, 161,  14);
	    GEngine->console.ScreenBufferInfo.ColorTable[3]  = RGB( 58, 150, 221);
	    GEngine->console.ScreenBufferInfo.ColorTable[4]  = RGB(197,  15,  31);
	    GEngine->console.ScreenBufferInfo.ColorTable[5]  = RGB(136,  23, 152);
	    GEngine->console.ScreenBufferInfo.ColorTable[6]  = RGB(193, 156,   0);
	    GEngine->console.ScreenBufferInfo.ColorTable[7]  = RGB(204, 204, 204);
	    GEngine->console.ScreenBufferInfo.ColorTable[8]  = RGB(118, 118, 118);
	    GEngine->console.ScreenBufferInfo.ColorTable[9]  = RGB( 59, 120, 255);
	    GEngine->console.ScreenBufferInfo.ColorTable[10] = RGB( 22, 198,  12);
	    GEngine->console.ScreenBufferInfo.ColorTable[11] = RGB( 97, 214, 214);
	    GEngine->console.ScreenBufferInfo.ColorTable[12] = RGB(231,  72,  86);
	    GEngine->console.ScreenBufferInfo.ColorTable[13] = RGB(180,   0, 158);
	    GEngine->console.ScreenBufferInfo.ColorTable[14] = RGB(249, 241, 165);
	    GEngine->console.ScreenBufferInfo.ColorTable[15] = RGB(242, 242, 242);

	    // �ĂԂ��тɃE�B���h�E�T�C�Y�ς�����Ⴄ����Ή�
	    GEngine->console.ScreenBufferInfo.srWindow.Bottom++;
	    GEngine->console.ScreenBufferInfo.srWindow.Right++;

	    SetConsoleScreenBufferInfoEx(GetOutputHandle(), &GEngine->console.ScreenBufferInfo);
    }

    void SetInputMode(InputMode mode)
    {
	    switch (mode)
	    {
		    case InputMode::Input:
		    {
			    SetConsoleMode(
				    GetInputHandle(),
				    ENABLE_ECHO_INPUT |
				    ENABLE_INSERT_MODE |
				    ENABLE_LINE_INPUT |
				    ENABLE_MOUSE_INPUT |
				    ENABLE_PROCESSED_INPUT |
				    ENABLE_EXTENDED_FLAGS |
				    ENABLE_QUICK_EDIT_MODE |
				    ENABLE_WINDOW_INPUT |
				    ENABLE_VIRTUAL_TERMINAL_INPUT);
			    break;
		    }
		    case InputMode::GameInput:
		    {
			    SetConsoleMode(
				    GetInputHandle(),
				    ENABLE_EXTENDED_FLAGS |
				    ENABLE_MOUSE_INPUT);
			    break;
		    }
	    }
    }

    void ResetConsoleMode()
    {
	    SetConsoleMode(
		    GetOutputHandle(),
		    ENABLE_PROCESSED_OUTPUT |
		    ENABLE_WRAP_AT_EOL_OUTPUT |
		    ENABLE_VIRTUAL_TERMINAL_PROCESSING |
		    DISABLE_NEWLINE_AUTO_RETURN |
		    ENABLE_LVB_GRID_WORLDWIDE);

	    SetConsoleMode(
		    GetInputHandle,
		    ENABLE_ECHO_INPUT |
		    ENABLE_INSERT_MODE |
		    ENABLE_LINE_INPUT |
		    ENABLE_MOUSE_INPUT |
		    ENABLE_PROCESSED_INPUT |
		    ENABLE_EXTENDED_FLAGS |
		    ENABLE_QUICK_EDIT_MODE |
		    //ENABLE_WINDOW_INPUT |
		    ENABLE_VIRTUAL_TERMINAL_INPUT);
    }

    void Draw(int x, int y, const char* string, Color font, Color background)
    {
	    unsigned short color = static_cast<unsigned short>(font) + static_cast<unsigned short>(background);
	    for (size_t i = 0; i < strlen(string); i++)
	    {
		    GEngine->console.ScreenBuffer[y * GEngine->console.WindowSize.X + x + i].Char.UnicodeChar = string[i];
		    GEngine->console.ScreenBuffer[y * GEngine->console.WindowSize.X + x + i].Attributes = color;
	    }
    }

    void Draw(int x, int y, const char * string, uint16 font, uint16 background)
    {
	    unsigned short color = static_cast<unsigned short>(font) + static_cast<unsigned short>(background);
	    for (size_t i = 0; i < strlen(string); i++)
	    {
		    GEngine->console.ScreenBuffer[y * GEngine->console.WindowSize.X + x + i].Char.UnicodeChar = string[i];
		    GEngine->console.ScreenBuffer[y * GEngine->console.WindowSize.X + x + i].Attributes = color;
	    }
    }

    void Draw24(int x, int y, int index, const char* string)
    {
	    for (size_t i = 0; i < strlen(string); i++)
	    {
		    GEngine->console.ScreenBuffer[y * GEngine->console.WindowSize.X + x + i].Char.UnicodeChar = string[i];
		    GEngine->console.ScreenBuffer[y * GEngine->console.WindowSize.X + x + i].Attributes = index << 4;
	    }
    }

    void DrawImage(Engine::Image* sprite, int startX, int startY)
    {
	    int p = 0;
	    int color = 0;

	    for (int y = sprite->Height; y > 0; y--)
	    {
		    for (int x = 0; x < sprite->Width; x++)
		    {
			    color = RGB(sprite->Sprite[p], sprite->Sprite[p + 1], sprite->Sprite[p + 2]);

			    if		(color == GEngine->console.ScreenBufferInfo.ColorTable[0])	conioex2::Draw24(startX + x, startY + y - 1, 0, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[1])	conioex2::Draw24(startX + x, startY + y - 1, 1, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[2])	conioex2::Draw24(startX + x, startY + y - 1, 2, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[3])	conioex2::Draw24(startX + x, startY + y - 1, 3, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[4])	conioex2::Draw24(startX + x, startY + y - 1, 4, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[5])	conioex2::Draw24(startX + x, startY + y - 1, 5, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[6])	conioex2::Draw24(startX + x, startY + y - 1, 6, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[7])	conioex2::Draw24(startX + x, startY + y - 1, 7, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[8])	conioex2::Draw24(startX + x, startY + y - 1, 8, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[9])	conioex2::Draw24(startX + x, startY + y - 1, 9, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[10])	conioex2::Draw24(startX + x, startY + y - 1, 10, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[11])	conioex2::Draw24(startX + x, startY + y - 1, 11, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[12])	conioex2::Draw24(startX + x, startY + y - 1, 12, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[13])	conioex2::Draw24(startX + x, startY + y - 1, 13, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[14])	conioex2::Draw24(startX + x, startY + y - 1, 14, " ");
			    else if (color == GEngine->console.ScreenBufferInfo.ColorTable[15])	conioex2::Draw24(startX + x, startY + y - 1, 15, " ");
			    p += 3;
		    }
	    }
    }

    void DrawImageGS(Engine::Image* sprite, int startX, int startY)
    {
	    int p = 0;
	    int color = 0;

	    for (int y = sprite->Height; y > 0; y--)
	    {
		    for (int x = 0; x < sprite->Width; x++)
		    {
			    color = RGB(sprite->Sprite[p] - 1, sprite->Sprite[p + 1] - 1, sprite->Sprite[p + 2] - 1);

			    switch (color)
			    {
				    case RGB(  0,   0,   0): conioex2::Draw24(startX + x, startY + y - 1, 15, " "); break;
				    case RGB( 16,  16,  16): conioex2::Draw24(startX + x, startY + y - 1, 14, " "); break;
				    case RGB( 32,  32,  32): conioex2::Draw24(startX + x, startY + y - 1, 13, " "); break;
				    case RGB( 48,  48,  48): conioex2::Draw24(startX + x, startY + y - 1, 12, " "); break;
				    case RGB( 64,  64,  64): conioex2::Draw24(startX + x, startY + y - 1, 11, " "); break;
				    case RGB( 80,  80,  80): conioex2::Draw24(startX + x, startY + y - 1, 10, " "); break;
				    case RGB( 96,  96,  96): conioex2::Draw24(startX + x, startY + y - 1,  9, " "); break;
				    case RGB(112, 112, 112): conioex2::Draw24(startX + x, startY + y - 1,  8, " "); break;
				    case RGB(128, 128, 128): conioex2::Draw24(startX + x, startY + y - 1,  7, " "); break;
				    case RGB(144, 144, 144): conioex2::Draw24(startX + x, startY + y - 1,  6, " "); break;
				    case RGB(160, 160, 160): conioex2::Draw24(startX + x, startY + y - 1,  5, " "); break;
				    case RGB(176, 176, 176): conioex2::Draw24(startX + x, startY + y - 1,  4, " "); break;
				    case RGB(192, 192, 192): conioex2::Draw24(startX + x, startY + y - 1,  3, " "); break;
				    case RGB(208, 208, 208): conioex2::Draw24(startX + x, startY + y - 1,  2, " "); break;
				    case RGB(224, 224, 224): conioex2::Draw24(startX + x, startY + y - 1,  1, " "); break;
				    case RGB(240, 240, 240): conioex2::Draw24(startX + x, startY + y - 1,  0, " "); break;
			    }
			    p += 3;
		    }
	    }
    }

    void DumpRGB(Engine::Image * sprite)
    {
	    int p = 0;
	    int color = 0;

	    for (int y = sprite->Height; y > 0; y--)
	    {
		    for (int x = 0; x < sprite->Width; x++)
		    {
			    LOG("R %d G %d B %d\n", sprite->Sprite[p], sprite->Sprite[p + 1], sprite->Sprite[p + 2]);
			    p += 3;
		    }
	    }
    }

    void Render()
    {
	    WriteConsoleOutputA(GEngine->console.hOutput, GEngine->console.ScreenBuffer, GEngine->console.WindowSize, COORD{0, 0}, &GEngine->console.WindowRect);
    }

    void ClearScreen()
    {
	    for (int y = 0; y < GEngine->console.WindowSize.Y; y++)
	    {
		    for (int x = 0; x < GEngine->console.WindowSize.X; x++)
		    {
			    Draw(x, y, " ", Color::FONT_BLACK, Color::BACK_BLACK);
		    }
	    }

	    Render();
    }

    void ClearBuffer()
    {
	    for (int y = 0; y < GEngine->console.WindowSize.Y; y++)
	    {
		    for (int x = 0; x < GEngine->console.WindowSize.X; x++)
		    {
			    Draw(x, y, " ", Color::FONT_BLACK, Color::BACK_BLACK);
		    }
	    }
    }

    void ClearScreen(Color font, Color background)
    {
	    for (int y = 0; y < GEngine->console.WindowSize.Y; y++)
	    {
		    for (int x = 0; x < GEngine->console.WindowSize.X; x++)
		    {
			    Draw(x, y, " ", font, background);
		    }
	    }

	    Render();
    }

    void ClearBuffer(Color font, Color background)
    {
	    for (int y = 0; y < GEngine->console.WindowSize.Y; y++)
	    {
		    for (int x = 0; x < GEngine->console.WindowSize.X; x++)
		    {
			    Draw(x, y, " ", font, background);
		    }
	    }
    }

    void ClearInputBuffer()
    {
	    FlushConsoleInputBuffer(GEngine->console.hInput);
    }

    void Wait(unsigned long milliseconds)
    {
	    Sleep(milliseconds);
    }

    void UpdateKeyInput()
    {
	    if (::GetKeyboardState(GEngine->Keyboard.New))
	    {
		    FOR(256)
		    {
			    GEngine->Keyboard.Keys[i].Pressed = false;
			    GEngine->Keyboard.Keys[i].Released = false;

			    if (GEngine->Keyboard.New[i] != GEngine->Keyboard.Old[i])
			    {
				    if (GEngine->Keyboard.New[i] & 0x80)
				    {
					    GEngine->Keyboard.Keys[i].Pressed = !GEngine->Keyboard.Keys[i].Hold;
					    GEngine->Keyboard.Keys[i].Hold = true;
				    }
				    else
				    {
					    GEngine->Keyboard.Keys[i].Released = true;
					    GEngine->Keyboard.Keys[i].Hold = false;
				    }
			    }
			    GEngine->Keyboard.Old[i] = GEngine->Keyboard.New[i];
		    }
	    }
    }

    void UpdateAsyncKeyInput()
    {
	    for (int i = 0; i < 256; i++)
	    {
		    GEngine->Keyboard.NewKey[i] = GetAsyncKeyState(i);

		    GEngine->Keyboard.Keys[i].Pressed = false;
		    GEngine->Keyboard.Keys[i].Released = false;

		    if (GEngine->Keyboard.NewKey[i] != GEngine->Keyboard.OldKey[i])
		    {
			    if (GEngine->Keyboard.NewKey[i] & 0x8000)
			    {
				    GEngine->Keyboard.Keys[i].Pressed = !GEngine->Keyboard.Keys[i].Hold;
				    GEngine->Keyboard.Keys[i].Hold = true;
			    }
			    else
			    {
				    GEngine->Keyboard.Keys[i].Released = true;
				    GEngine->Keyboard.Keys[i].Hold = false;
			    }
		    }

		    GEngine->Keyboard.OldKey[i] = GEngine->Keyboard.NewKey[i];
	    }
    }

    bool GetInputKey(Key key, InputState state)
    {
	    switch (state)
	    {
		    case InputState::Pressed: return GEngine->Keyboard.Keys[key].Pressed;  break;
		    case InputState::Release: return GEngine->Keyboard.Keys[key].Released; break;
		    case InputState::Hold:	  return GEngine->Keyboard.Keys[key].Hold;     break;
		    default:				  return false;
	    }
    }

    void UpdateMouseInput()
    {
	    GetNumberOfConsoleInputEvents(GetInputHandle(), &GEngine->Mouse.Event);
	    if (GEngine->Mouse.Event > 0)
	    {
		    ReadConsoleInput(GetInputHandle(), GEngine->Mouse.InputRecord, GEngine->Mouse.Event, &GEngine->Mouse.Event);
	    }

	    for (DWORD i = 0; i < GEngine->Mouse.Event; i++)
	    {
		    switch (GEngine->Mouse.InputRecord[i].EventType)
		    {
			    case FOCUS_EVENT:
			    {
				    GEngine->Mouse.IsFocusInConsole = GEngine->Mouse.InputRecord[i].Event.FocusEvent.bSetFocus;
				    break;
			    }
			    case MOUSE_EVENT:
			    {
				    switch (GEngine->Mouse.InputRecord[i].Event.MouseEvent.dwEventFlags)
				    {
					    case MOUSE_MOVED:
					    {
						    GEngine->Mouse.Location.X = GEngine->Mouse.InputRecord[i].Event.MouseEvent.dwMousePosition.X;
						    GEngine->Mouse.Location.Y = GEngine->Mouse.InputRecord[i].Event.MouseEvent.dwMousePosition.Y;
						    break;
					    }
					    case 0:
					    {
						    for (int j = 0; j < 5; j++)
						    {
							    GEngine->Mouse.NewButton[j] = (GEngine->Mouse.InputRecord[i].Event.MouseEvent.dwButtonState & (1 << j)) > 0;
						    }
						    break;
					    }
				    }
				    break;
			    }

			    default: break;
		    }
	    }

	    for (int i = 0; i < 5; i++)
	    {
		    GEngine->Mouse.Button[i].Pressed  = false;
		    GEngine->Mouse.Button[i].Released = false;

		    if (GEngine->Mouse.NewButton[i] != GEngine->Mouse.OldButton[i])
		    {
			    if (GEngine->Mouse.NewButton[i])
			    {		   
				    GEngine->Mouse.Button[i].Pressed = true;
				    GEngine->Mouse.Button[i].Hold    = true;
			    }		   
			    else	   
			    {		   
				    GEngine->Mouse.Button[i].Released = true;
				    GEngine->Mouse.Button[i].Hold     = false;
			    }
		    }

		    GEngine->Mouse.OldButton[i] = GEngine->Mouse.NewButton[i];
	    }
    }

    bool GetInputMouseInput(MouseButton button, InputState state)
    {
	    switch (state)
	    {
		    case InputState::Pressed: return GEngine->Mouse.Button[button].Pressed;  break;
		    case InputState::Hold:	  return GEngine->Mouse.Button[button].Hold;     break;
		    case InputState::Release: return GEngine->Mouse.Button[button].Released; break;
	    }
	    return false;
    }

    int GetMouseX()
    {
	    return GEngine->Mouse.Location.X;
    }

    int GetMouseY()
    {
	    return GEngine->Mouse.Location.Y;
    }

    bool IsFocusInConsole()
    {
	    return GEngine->Mouse.IsFocusInConsole;
    }

    bool IsInViewport(Vector2 location)
    {
	    return (location.X < static_cast<float>(conioex2::Width())) && (location.X > 0.0f) 
											      &&
		       (location.Y < static_cast<float>(conioex2::Height()) - 1.0f) && (location.Y > 0.0f);
    }

    bool IsHit(AABB self, AABB other)
    {
	    if (self.Min.X > other.Max.X) return false;
	    if (self.Max.X < other.Min.X) return false;
	    if (self.Min.Y > other.Max.Y) return false;
	    if (self.Max.Y < other.Min.Y) return false;
	    return true;
    }

    float RadianFromDegree(float degree)
    {
	    return degree * (PI / 180.f);
    }

    void UpdateXBoxInput()
    {
	    DWORD dwResult;
	    XINPUT_STATE state;
	    ZeroMemory(&state, sizeof(state));

	    conioex2::Engine::XBox_* gamepad = &GEngine->XBox;

	    dwResult = XInputGetState(0, &state);
	    if (dwResult == ERROR_SUCCESS)
	    {
		    gamepad->enabled = true;
		    gamepad->left_trigger  = NormalizeTrigger(state.Gamepad.bLeftTrigger);
		    gamepad->right_trigger = NormalizeTrigger(state.Gamepad.bRightTrigger);

		    Vector2 zero{ 0.f, 0.f };
		    Vector2 one{ 1.f, 1.f };

		    Vector2 left_stick;
		    left_stick.X = NormalizeStick(state.Gamepad.sThumbLX);
		    left_stick.Y = NormalizeStick(state.Gamepad.sThumbLY);

		    Vector2 right_stick;
		    right_stick.X = NormalizeStick(state.Gamepad.sThumbRX);
		    right_stick.Y = NormalizeStick(state.Gamepad.sThumbRY);

		    gamepad->left_stick  = left_stick;
		    gamepad->right_stick = right_stick;

		    int buttons_field = state.Gamepad.wButtons;
		    UpdateXBoxButtonState(&gamepad->buttons[0],  GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_DPAD_UP));
		    UpdateXBoxButtonState(&gamepad->buttons[1],  GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_DPAD_DOWN));
		    UpdateXBoxButtonState(&gamepad->buttons[2],  GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_DPAD_RIGHT));
		    UpdateXBoxButtonState(&gamepad->buttons[3],  GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_DPAD_LEFT));
													     
		    UpdateXBoxButtonState(&gamepad->buttons[4],  GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_START));
		    UpdateXBoxButtonState(&gamepad->buttons[5],  GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_BACK));
													     
		    UpdateXBoxButtonState(&gamepad->buttons[6],  GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_LEFT_THUMB));
		    UpdateXBoxButtonState(&gamepad->buttons[7],  GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_RIGHT_THUMB));
													     
		    UpdateXBoxButtonState(&gamepad->buttons[8],  GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_SHOULDER_RIGHT));
		    UpdateXBoxButtonState(&gamepad->buttons[9],  GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_SHOULDER_LEFT));
							    
		    UpdateXBoxButtonState(&gamepad->buttons[10], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_A));
		    UpdateXBoxButtonState(&gamepad->buttons[11], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_B));
		    UpdateXBoxButtonState(&gamepad->buttons[12], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_X));
		    UpdateXBoxButtonState(&gamepad->buttons[13], GetKeyStateFromBitField(buttons_field, GAMEPAD_BUTTON_Y));
	    }
	    else
	    {
		    gamepad->enabled = false;
	    }
    }

    bool GetInputXBoxButton(Button button, XBoxInputState state)
    {
	    return GEngine->XBox.buttons[button] == state;
    }

    float GetInputXBoxTrigger(Trigger axis)
    {
	    switch (axis)
	    {
		    case Trigger::Trigger_L:
		    {
			    return GEngine->XBox.left_trigger;
		    }
		    case Trigger::Trigger_R:
		    {
			    return GEngine->XBox.right_trigger;
		    }
		    default: return 0.0f;
	    }
    }

    Vector2 GetInputXBoxAxis(Stick axis)
    {
	    switch (axis)
	    {
		    case Stick::Stick_L:
		    {
			    return GEngine->XBox.left_stick;
		    }
		    case Stick::Stick_R:
		    {
			    return GEngine->XBox.right_stick;
		    }
	    }
    }

    void PlayVibrate()
    {
	    XINPUT_VIBRATION vibration;
	    vibration.wLeftMotorSpeed  = UnNormarileVibration(1.f);
	    vibration.wRightMotorSpeed = UnNormarileVibration(1.f);

	    XInputSetState(0, &vibration);
    }

    void StopVibrate()
    {
	    XINPUT_VIBRATION vibration;
	    vibration.wLeftMotorSpeed  = UnNormarileVibration(0.f);
	    vibration.wRightMotorSpeed = UnNormarileVibration(0.f);

	    XInputSetState(0, &vibration);
    }

    Engine::Image* CreateImage(const char* filename)
    {
	    BMP* img = LoadBMP(filename);
	    if (!img) return nullptr;

	    Engine::Image* sprite = new Engine::Image;
	    if (!sprite) return nullptr;

	    // RBG�̎O�v�f���i�[���邽�߂�3�{�̗̈�m�ۂ���
	    sprite->Sprite = new unsigned char[(img->height * img->width) * 3];
	    sprite->Height = img->height;
	    sprite->Width = img->width;

	    int elm = 0;
	    for (unsigned int i = 0; i < img->height; i++)
	    {
		    for (unsigned int j = 0; j < img->width; j++)
		    {
			    unsigned char r = (img->data[(img->height - i - 1) * img->width + j].R) + 1;
			    unsigned char g = (img->data[(img->height - i - 1) * img->width + j].G) + 1;
			    unsigned char b = (img->data[(img->height - i - 1) * img->width + j].B) + 1;
			    sprite->Sprite[elm    ] = r;
			    sprite->Sprite[elm + 1] = g;
			    sprite->Sprite[elm + 2] = b;
			    elm += 3;
		    }
	    }
	    return sprite;
    }

    void CreateColorTableFromImage(Engine::Image* sprite)
    {
	    COLORREF table[16] = {};	// �J���[�e�[�u���̃o�b�t�@
	    int elem = 0;				// �摜�f�[�^�̗v�f��
	    int index = 0;				// �V�����F�̎��ɂǂ̏ꏊ�ɑ}�����邩
	    bool isExitZero = false;	// RGB(0, 0, 0)�����͂��ꂽ���ǂ���
	    int  match = 0;				// �}�b�`�����F�̐�

	    // �摜�T�C�Y���i�c�~���j�������[�v����
	    for (int n = 0; n < (sprite->Height * sprite->Width); n++)
	    {
		    // �摜�f�[�^����s�N�Z���f�[�^�����
		    if (index > 15) break;
		    Pixel pixel(sprite->Sprite[elem], sprite->Sprite[elem + 1], sprite->Sprite[elem + 2]);

		    // ���͒l��RGB(0, 0, 0)���ǂ���
		    if ((pixel.R + pixel.G + pixel.B) == 0)
		    {
			    // ���ł�0����������X�L�b�v
			    if (isExitZero)
			    {
				    elem += 3;
				    continue;
			    }
			    else
			    {
				    // ���߂�0�����͂��ꂽ�ꍇ�̓}�b�`�A�ȍ~��0�̓X�L�b�v
				    table[index] = RGB(pixel.R, pixel.G, pixel.B);
				    //Debug::LOG("[%2d]: R %3d, G %3d, B %3d\n", index, pixel.R, pixel.G, pixel.B);
				    index++;
				    isExitZero = true;
			    }
		    }
		    else
		    {
			    // index�񂾂����[�v������(�q�b�g�E�m�[�q�b�g�ւ�炸�S����������)�����̗v�f�Ɉ�v����l������ꍇ�����邩��
			    for (int i = 0; i < index; i++)
			    {
				    if (CompareColor(table[i], pixel))
				    {
					    match++;
				    }
			    }
			    // ��v�����l��1��������΃e�[�u���ɒǉ�����
			    if (match == 0)
			    {
				    table[index] = RGB(pixel.R, pixel.G, pixel.B);
				    //Debug::LOG("[%2d]: R %3d, G %3d, B %3d\n", index, pixel.R, pixel.G, pixel.B);
				    index++;
			    }
		    }

		    match = 0;
		    elem += 3;
	    }

	    FOR(16)
	    {
		    conioex2::SetColorTable(i, table[i] - RGB(1, 1, 1));
	    }
    }

    void Print24(int x, int y, const char* string, Pixel font, Pixel background)
    {
	    SetCursorLocation(x, y);
	    printf("\x1b[48;2;%d;%d;%dm ", background.R, background.G, background.B);
	    printf("\x1b[38;2;%d;%d;%dm%s", font.R, font.G, font.B, string);
    }

    void ShowImage(Engine::Image* sprite, int lx, int ly)
    {
	    int i = 0;
	    for (int y = sprite->Height; y > 0; y--)
	    {
		    for (int x = 0; x < sprite->Width; x++)
		    {
			    conioex2::Print24(lx + 2 * x, ly + y - 1, "  ", Pixel(255, 255, 255), Pixel(sprite->Sprite[i], sprite->Sprite[i + 1], sprite->Sprite[i + 2]));
			    i += 3;
		    }
	    }
    }

    bool ReleaseImage(Engine::Image* object)
    {
	    if (object)
	    {
		    if (object->Sprite)
		    {
			    delete object->Sprite; // delete[]����ˁH
			    delete object;
			    return true;
		    }
	    }
	    return false;
    }

    void InitTime()
    {
	    QueryPerformanceFrequency(&GEngine->Time.frequency);
	    QueryPerformanceCounter(&GEngine->Time.start);
	    QueryPerformanceCounter(&GEngine->Time.end);
    }

    double GetDeltaTime()
    {
	    return GEngine->Time.DeltaTime;
    }

    void UpdateTime()
    {
	    QueryPerformanceCounter(&GEngine->Time.end);
	    GEngine->Time.DeltaTime = static_cast<double>((GEngine->Time.end.QuadPart - GEngine->Time.start.QuadPart) / (GEngine->Time.frequency.QuadPart));
	    GEngine->Time.start.QuadPart = GEngine->Time.end.QuadPart;
    }

    uint64 RangeRand(uint64 min, uint64 max)
    {
	    static std::mt19937_64 mt64(0);
	    std::uniform_int_distribution<uint64_t> rand(min, max);
	    return rand(mt64);
    }


#if 0
    bool InitSound()
    {
	    ZeroMemory(&GEngine->Sound, sizeof(GEngine->Sound));
	    CoInitializeEx(NULL, tagCOINIT::COINIT_APARTMENTTHREADED);
	    if (FAILED(XAudio2Create(&GEngine->Sound.XAudio2, 0)))
	    {
		    CoUninitialize();
		    return false;
	    }

	    if (FAILED(GEngine->Sound.XAudio2->CreateMasteringVoice(&GEngine->Sound.MasteringVoice)))
	    {
		    CoUninitialize();
		    return false;
	    }
	    return true;
    }

    bool DestroySound()
    {
	    for (int i = 0; i < 10; i++)
	    {
		    if (GEngine->Sound.SourceVoice[i]) GEngine->Sound.SourceVoice[i]->DestroyVoice();
	    }
	    for (int i = 0; i < 10; i++)
	    {
		    if (GEngine->Sound.WavBuffer[i])
		    {
			    delete GEngine->Sound.WavBuffer[i];
			    GEngine->Sound.WavBuffer[i] = nullptr;
		    }
	    }

	    if (GEngine->Sound.XAudio2)
	    {
		    GEngine->Sound.XAudio2->Release();
		    GEngine->Sound.XAudio2 = nullptr;
	    }

	    return true;
    }

    int LoadSound(const char* fileName)
    {
	    static int iIndex = -1;
	    iIndex++;
	    HMMIO hMmio = NULL;
	    DWORD dwWavSize = 0;
	    WAVEFORMATEX* pwfex;
	    MMCKINFO ckInfo;
	    MMCKINFO riffckInfo;
	    PCMWAVEFORMAT pcmWaveForm;

	    hMmio = mmioOpenA(const_cast<LPSTR>(fileName), NULL, MMIO_ALLOCBUF | MMIO_READ);
	    mmioDescend(hMmio, &riffckInfo, NULL, 0);

	    ckInfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
	    mmioDescend(hMmio, &ckInfo, &riffckInfo, MMIO_FINDCHUNK);

	    mmioRead(hMmio, (HPSTR)&pcmWaveForm, sizeof(pcmWaveForm));
	    pwfex = (WAVEFORMATEX*)new CHAR[sizeof(WAVEFORMATEX)];
	    memcpy(pwfex, &pcmWaveForm, sizeof(pcmWaveForm));
	    pwfex->cbSize = 0;
	    mmioAscend(hMmio, &ckInfo, 0);

	    ckInfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
	    mmioDescend(hMmio, &ckInfo, &riffckInfo, MMIO_FINDCHUNK);
	    dwWavSize = ckInfo.cksize;
	    GEngine->Sound.WavBuffer[iIndex] = new BYTE[dwWavSize];
	    DWORD dwOffset = ckInfo.dwDataOffset;
	    mmioRead(hMmio, (HPSTR)GEngine->Sound.WavBuffer[iIndex], dwWavSize);

	    if (FAILED(GEngine->Sound.XAudio2->CreateSourceVoice(&GEngine->Sound.SourceVoice[iIndex], pwfex)))
	    {
		    MessageBox(0, "FAILED: Create SourceVoice.", 0, MB_OK);
		    return E_FAIL;
	    }

	    GEngine->Sound.WavSize[iIndex] = dwWavSize;

	    return iIndex;
    }

    void PlaytSound(int soundIndex, bool doLoop)
    {
	    XAUDIO2_VOICE_STATE state;
	    GEngine->Sound.SourceVoice[soundIndex]->GetState(&state);

	    if (state.BuffersQueued > 0)
	    {
		    return;
	    }

	    XAUDIO2_BUFFER buffer = { 0 };
	    buffer.pAudioData = GEngine->Sound.WavBuffer[soundIndex];
	    buffer.Flags = XAUDIO2_END_OF_STREAM;
	    buffer.AudioBytes = GEngine->Sound.WavSize[soundIndex];

	    if (doLoop)
	    {
		    buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	    }

	    if (FAILED(GEngine->Sound.SourceVoice[soundIndex]->SubmitSourceBuffer(&buffer)))
	    {
		    MessageBox(0, "FAILED: Submit to SourceBuffer.", 0, MB_OK);
		    return;
	    }

	    GEngine->Sound.SourceVoice[soundIndex]->Start(0, XAUDIO2_COMMIT_NOW);
    }

    void StopSound(int soundIndex)
    {
	    GEngine->Sound.SourceVoice[soundIndex]->Stop(0, XAUDIO2_COMMIT_NOW);
    }

    void SetVolume(int soundIndex, float volume)
    {
	    GEngine->Sound.SourceVoice[soundIndex]->SetVolume(volume, XAUDIO2_COMMIT_NOW);
    }

    void SetPitch(int soundIndex, float pitch)
    {
	    if (GEngine->Sound.SourceVoice[soundIndex])
	    {
		    float freqRatio = SemitonesToFrequencyRatio(pitch * 12.f);
		    GEngine->Sound.SourceVoice[soundIndex]->SetFrequencyRatio(freqRatio);
	    }
    }
#endif

}