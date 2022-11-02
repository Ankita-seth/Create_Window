#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include<string>
#include <shellapi.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include <d3d12sdklayers.h>
#include <combaseapi.h>
//C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <SDKDDKVer.h>
//Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>
using namespace Microsoft::WRL;

#if defined(CreateWindow)
#undef CreateWindow
#endif

//Liniking the libraries
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
class DirectXExample
{
public:
	DirectXExample()
	{

	}
	~DirectXExample()
	{

	}

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
	//handle for window
	HWND hwnd = NULL;

	//mname of the window
	LPCTSTR WindowName = L"DirectX12";

	//title of the window
	LPCTSTR WindowTitle = L"Welcome to My First Window";

	//width of the window
	int Width = 1000;

	//height of the window
	int Height = 900;

	//fullScreen function
	bool FullScreen = false;

	bool Running = true;

	//to create window
	bool InitializeWindow(
		HINSTANCE hInstance,
		int ShowWND,
		bool fullScreen);

	//Create a Device
	bool InitializeDevice();
	//most important part of the program where we will check for windows messages, update our game logic, and render our frame
	void mainloop();
	void Update();
	void UpdatePipeline();
	void WaitForPreviousFrame();
	void Render();
	void Cleanup();

	const static int frameBufferCount = 3;

	ID3D12Device1* device;
	ID3D12CommandQueue* commandQueue;
	ID3D12Resource* RenderTarget[frameBufferCount];
	ID3D12GraphicsCommandList* commandList;
	ID3D12CommandAllocator* commandAllocator[frameBufferCount];
	IDXGISwapChain3* SwapChain;
	ID3D12DescriptorHeap* DescriptorHeap;
	ID3D12Fence* fence[frameBufferCount];
	HANDLE fenceEvent;
	int rtvDescriptorSize;
	int frameIndex;
	UINT64 fenceValue[frameBufferCount];


};
LRESULT CALLBACK WndProc(HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);
