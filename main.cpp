#include "framework.h"

//Main Function
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpcmdLine,
	int nShowCmd)
{
	DirectXExample* ex = new DirectXExample();
	if (ex)
	{
		if (!ex->InitializeWindow(hInstance, nShowCmd, ex->FullScreen))
		{
			MessageBox(0, L"Failed to CreateWindow", L"ERROR", MB_OK);
			return 0;
		}
		if (!ex->InitializeDevice())
		{
			MessageBox(0, L"Failed to initialize the d3device", L"Error", MB_OK);
			return 1;
		}
		ex->mainloop();
		ex->WaitForPreviousFrame();
		ex->UpdatePipeline();
		CloseHandle(ex->fenceEvent);
		ex->Cleanup();
	}
	delete ex;
	ex = NULL;
	return 0;
}
