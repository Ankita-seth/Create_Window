#include "framework.h"

bool DirectXExample::InitializeWindow(
	HINSTANCE hInstance,
	int ShowWND,
	bool fullScreen)
{
	if (fullScreen)
	{
		HMONITOR hmon = MonitorFromWindow(hwnd,
			MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		Width = mi.rcMonitor.right - mi.rcMonitor.left;
		Height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}

	WNDCLASSEX wndcls;

	wndcls.cbSize = sizeof(WNDCLASSEX);
	wndcls.style = NULL;
	wndcls.cbClsExtra = 0;
	wndcls.cbWndExtra = 0;
	wndcls.hInstance = hInstance;
	wndcls.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);
	wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndcls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndcls.lpfnWndProc = WndProc;
	wndcls.lpszClassName = WindowName;
	wndcls.lpszMenuName = 0;
	wndcls.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if (!RegisterClassEx(&wndcls))
	{
		MessageBox(0, L"Error to Register Class", L"Error", MB_OK);
		return false;
	}

	hwnd = CreateWindowExW(
		NULL,
		WindowName,
		WindowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		Width,
		Height,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (!hwnd)
	{
		MessageBox(NULL, L"Fail to create Window", L"Error", MB_OK);
		return false;
	}

	if (fullScreen)
	{
		SetWindowLong(hwnd, GWL_STYLE, 0);
	}
	ShowWindow(hwnd, ShowWND);
	UpdateWindow(hwnd);
	return true;
}

void DirectXExample::mainloop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (true)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else
		{
			//
		}
	}
}

bool DirectXExample::InitializeDevice()
{
	HRESULT hr;
	IDXGIFactory4* dxgiFactory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
	{
		return false;
	}
	IDXGIAdapter1* adapter;
	int adapterIndex = 0;
	bool adapterFound = false;

	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			adapterIndex++;
			continue;
		}
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}
		adapterIndex++;
	}

	if (!adapterFound)
	{
		return false;
	}

	//create a device
	//hr = SwDeviceCreate();
	hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
	if (FAILED(hr))
	{
		return false;
	}


	//create a command queue
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue));
	if (FAILED(hr))
	{
		return false;
	}
	//create a SwapChain 

		//filling BufferDesc structure
	DXGI_MODE_DESC	backBufferDesc = {};
	backBufferDesc.Width = Width;
	backBufferDesc.Height = Height;
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	backBufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	backBufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//filling Sample Desc structure
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;

	//filling swapchain desc structure
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameBufferCount;
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.Windowed = !FullScreen;
	swapChainDesc.SampleDesc = sampleDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Flags = 0;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	UINT64 fenceValue[frameBufferCount];
	//CreateSwapChain Function
	IDXGISwapChain* dxSwapChain;
	dxgiFactory->CreateSwapChain(
		commandQueue,
		&swapChainDesc,
		&dxSwapChain
	);

	SwapChain = static_cast<IDXGISwapChain3*>(dxSwapChain);
	frameIndex = SwapChain->GetCurrentBackBufferIndex();


	//create a descriptor heap

		//filling the DESCRIPTOR_HEAP_DESC Structure
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NumDescriptors = 3;
	rtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDescriptorHeapDesc.NodeMask = 0;

	//CreateDescriptorHeap
	hr = device->CreateDescriptorHeap(
		&rtvDescriptorHeapDesc,
		IID_PPV_ARGS(&DescriptorHeap)
	);
	if (FAILED(hr))
	{
		return FALSE;
	}

	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < frameBufferCount; i++)
	{
		hr = SwapChain->GetBuffer(i, IID_PPV_ARGS(&RenderTarget[i]));
		if (FAILED(hr))
		{
			return false;
		}
		device->CreateRenderTargetView(RenderTarget[i], nullptr, rtvHandle);
		rtvHandle.Offset(1, rtvDescriptorSize);
	}


	//create a command allocator
	for (int i = 0; i < frameBufferCount; i++)
	{
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
		if (FAILED(hr))
		{
			return false;
		}
	}

	//create a command list
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[0], nullptr, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
	{
		return false;
	}
	commandList->Close();

	//create a fence
	for (int i = 0; i < frameBufferCount; i++)
	{
		hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		if (FAILED(hr))
		{
			return FALSE;
		}
	}

	//fenceEvent to handle the fence
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		return false;
	}

	return true;
}

void DirectXExample::Update()
{
}

void DirectXExample::UpdatePipeline()
{
	HRESULT hr;

	// We have to wait for the gpu to finish with the command allocator before we reset it
	DirectXExample::WaitForPreviousFrame();

	// we can only reset an allocator once the gpu is done with it resetting an allocator frees the memory that the command list was stored in
	hr = commandAllocator[frameIndex]->Reset();
	if (FAILED(hr))
	{
		Running = false;
	}
	hr = commandList->Reset(commandAllocator[frameIndex], NULL);
	if (FAILED(hr))
	{
		Running = false;
	}

	// here we start recording commands into the commandList (which all the commands will be stored in the commandAllocator)

	// transition the "frameIndex" render target from the present state to the render target state so the command list draws to it starting from here
	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(RenderTarget[frameIndex],  D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// here we again get the handle to our current render target view so we can set it as the render target in the output merger stage of the pipeline
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(DescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);

	// set the render target for the output merger stage (the output of the pipeline)
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Clear the render target by using the ClearRenderTargetView command
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// transition the "frameIndex" render target from the render target state to the present state. If the debug layer is enabled, you will receive a warning if present is called on the render target when it's not in the present state
	//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(RenderTarget[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	hr = commandList->Close();
	if (FAILED(hr))
	{
		Running = FALSE;
	}
}
void DirectXExample::Render()
{
	HRESULT hr;

	// update the pipeline by sending commands to the commandqueue
	DirectXExample::UpdatePipeline();
	// create an array of command lists (only one command list here)
	ID3D12CommandList* ppCommandLists[] = { commandList };

	// execute the array of command lists
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr))
	{
		Running = false;
	}

	// present the current backbuffer
	hr = SwapChain->Present(0, 0);
	if (FAILED(hr))
	{
		Running = false;
	}
}

void DirectXExample::Cleanup()
{
	// wait for the gpu to finish all frames
	for (int i = 0; i < frameBufferCount; ++i)
	{
		frameIndex = i;
		WaitForPreviousFrame();
	}

	// get swapchain out of full screen before exiting
	BOOL fs = false;
	if (SwapChain->GetFullscreenState(&fs, NULL))
		SwapChain->SetFullscreenState(false, NULL);

	SAFE_RELEASE(device);
	SAFE_RELEASE(SwapChain);
	SAFE_RELEASE(commandQueue);
	SAFE_RELEASE(DescriptorHeap);
	SAFE_RELEASE(commandList);

	for (int i = 0; i < frameBufferCount; ++i)
	{
		SAFE_RELEASE(RenderTarget[i]);
		SAFE_RELEASE(commandAllocator[i]);
		SAFE_RELEASE(fence[i]);
	};
}

void DirectXExample::WaitForPreviousFrame()
{
	HRESULT hr;

	// swap the current rtv buffer index so we draw on the correct buffer
	frameIndex = SwapChain->GetCurrentBackBufferIndex();

	// if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
	{
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
		if (FAILED(hr))
		{
			Running = false;
		}

		// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value has reached "fenceValue", we know the command queue has finished executing
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	// increment fenceValue for next frame
	fenceValue[frameIndex]++;
}

LRESULT CALLBACK WndProc(HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			if (MessageBox(0, L"Are you sure you want to exit?",
				L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hwnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,
		msg,
		wParam,
		lParam);
}


