#include "GSystem.h"
#if PLATFORM_WINDOWS

static int	_WIDTH					= 1024;
static int	_HEIGHT					= 768;
static int	_FPS					= 60;
static bool _EXIT					= false;
static HWND _WINDOW					= NULL;
static LPDIRECT3D9 _DIRECTX			= NULL;
LPDIRECT3DDEVICE9 _DEVICE			= NULL; // Non-static to allow for extern access

static LRESULT CALLBACK _WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CLOSE:
		_EXIT = true;
		return 0;
	case WM_KEYDOWN:
		if ((lParam & (1 << 30)) == 0)
			GSystem::RunKeyCallbacks((vkey_t)wParam);
		return 0;
	case WM_KEYUP:
		GSystem::RunKeyUpCallbacks((vkey_t)wParam);
		return 0;
	case WM_CHAR:
		if ((int)wParam <= 127 && isprint((int)wParam) && (lParam & (1 << 30)) == 0)
			GSystem::RunASCIICallbacks((char)wParam);
		return 0;
	case WM_SYSKEYDOWN:
		if ((lParam & (1 << 30)) == 0)
			GSystem::RunKeyCallbacks((vkey_t)wParam);
		return 0;
	case WM_SYSKEYUP:
		GSystem::RunKeyUpCallbacks((vkey_t)wParam);
		return 0;
	case WM_MOUSEMOVE:
		GSystem::RunMouseMoveCallbacks((int_t)GET_X_LPARAM(lParam), (int_t)GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONDOWN:
		GSystem::RunMouseCallbacks((int_t)GET_X_LPARAM(lParam), (int_t)GET_Y_LPARAM(lParam), 0);
		return 0;
	case WM_LBUTTONUP:
		GSystem::RunMouseUpCallbacks((int_t)GET_X_LPARAM(lParam), (int_t)GET_Y_LPARAM(lParam), 0);
		return 0;
	case WM_LBUTTONDBLCLK:
		GSystem::RunMouseCallbacks((int_t)GET_X_LPARAM(lParam), (int_t)GET_Y_LPARAM(lParam), 0);
		return 0;
	case WM_RBUTTONDOWN:
		GSystem::RunMouseCallbacks((int_t)GET_X_LPARAM(lParam), (int_t)GET_Y_LPARAM(lParam), 1);
		return 0;
	case WM_RBUTTONUP:
		GSystem::RunMouseUpCallbacks((int_t)GET_X_LPARAM(lParam), (int_t)GET_Y_LPARAM(lParam), 1);
		return 0;
	case WM_RBUTTONDBLCLK:
		GSystem::RunMouseCallbacks((int_t)GET_X_LPARAM(lParam), (int_t)GET_Y_LPARAM(lParam), 1);
		return 0;
	case WM_MBUTTONDOWN:
		GSystem::RunMouseCallbacks((int_t)GET_X_LPARAM(lParam), (int_t)GET_Y_LPARAM(lParam), 2);
		return 0;
	case WM_MBUTTONUP:
		GSystem::RunMouseUpCallbacks((int_t)GET_X_LPARAM(lParam), (int_t)GET_Y_LPARAM(lParam), 2);
		return 0;
	case WM_MBUTTONDBLCLK:
		GSystem::RunMouseCallbacks((int_t)GET_X_LPARAM(lParam), (int_t)GET_Y_LPARAM(lParam), 2);
		return 0;
	case WM_MOUSEWHEEL:
		GSystem::RunMouseWheelCallbacks((float)0, (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
		return 0;
	case WM_XBUTTONDOWN:
		GSystem::RunMouseCallbacks((int_t)GET_X_LPARAM(lParam), (int_t)GET_Y_LPARAM(lParam), (int_t)GET_XBUTTON_WPARAM(wParam) - (int_t)XBUTTON1 + 3);
		return 0;
	case WM_XBUTTONUP:
		GSystem::RunMouseUpCallbacks((int_t)GET_X_LPARAM(lParam), (int_t)GET_Y_LPARAM(lParam), (int_t)GET_XBUTTON_WPARAM(wParam) - (int_t)XBUTTON1 + 3);
		return 0;
	case WM_XBUTTONDBLCLK:
		GSystem::RunMouseCallbacks((int_t)GET_X_LPARAM(lParam), (int_t)GET_Y_LPARAM(lParam), (int_t)GET_XBUTTON_WPARAM(wParam) - (int_t)XBUTTON1 + 3);
		return 0;
	case WM_MOUSEHWHEEL:
		GSystem::RunMouseWheelCallbacks((float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, (float)0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static WCHAR* _GetTitle() {
	static WCHAR* TITLE = NULL;
	static WCHAR NAME_BUFFER[MAX_PATH];

	// No need to find the title more than once
	if (TITLE)
		return TITLE;

	// Get the full path;
	GetModuleFileName(NULL, NAME_BUFFER, MAX_PATH);

	// Remove the path
	TITLE = wcsrchr(NAME_BUFFER, L'\\');
	if (TITLE == NULL) {
		wcscpy(NAME_BUFFER, L"Game");
		TITLE = NAME_BUFFER;
		return TITLE;
	}
	TITLE++;

	// Remvoe the extension
	WCHAR* end = wcsrchr(NAME_BUFFER, L'.');
	if (end != NULL)
		*end = 0;

	return TITLE;
}

void GSystem::Print(const char* message, ...) {
	if (message) {
		va_list args;
		va_start(args, message);
#if PLATFORM_WINDOWS && DEBUG
		int size = vsnprintf(NULL, 0, message, args);
		if (size > 0) {
			char* string = new char[size + 1];
			vsnprintf(string, size + 1, message, args);
			OutputDebugStringA(string);
			delete[] string;
		}
#endif
		vprintf(message, args);
		va_end(args);
	}
}

void GSystem::Debug(const char* message, ...) {
#if DEBUG
	if (message) {
		va_list args;
		va_start(args, message);
#if PLATFORM_WINDOWS
		int size = vsnprintf(NULL, 0, message, args);
		if (size > 0) {
			char* string = new char[size + 1];
			vsnprintf(string, size + 1, message, args);
			OutputDebugStringA(string);
			delete[] string;
		}
#else
		vprintf(message, args);
#endif
		va_end(args);
	}
#endif
}

int_t GSystem::GetWidth() {
	return _WIDTH;
}

int_t GSystem::GetHeight() {
	return _HEIGHT;
}

int_t GSystem::GetFPS() {
	return _FPS;
}

int_t GSystem::GetUniqueRef() {
	static int_t REF = 1;
	return REF++;
}

uint64 GSystem::GetMilliseconds() {
	return (uint64)GetTickCount64();
}

uint64 GSystem::GetMicroseconds() {
	LARGE_INTEGER frequency;
	LARGE_INTEGER counter;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);
	return (uint64)(counter.QuadPart * 1000000 / frequency.QuadPart);
}

uint64 GSystem::GetNanoseconds() {
	LARGE_INTEGER frequency;
	LARGE_INTEGER counter;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);
	return (uint64)(counter.QuadPart * 1000000000 / frequency.QuadPart);
}

void GSystem::SetDefaultWD() {
	SetCurrentDirectory(L"Resources");
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	GSystem::SetDefaultWD();

#if DEBUG
	char wd[MAX_PATH];
	GetCurrentDirectoryA(MAX_PATH, wd);
	GSystem::Print("WD: %s\n", wd);
#endif
	
	// Create the window class
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_CLASSDC;
	wc.lpfnWndProc = _WndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(wc.hInstance, (LPCTSTR)IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = _GetTitle();
	wc.hIconSm = LoadIcon(wc.hInstance, (LPCTSTR)IDI_APPLICATION);
	HRESULT result = RegisterClassEx(&wc);
	if (FAILED(result))
		return (int)result;
	
	// Adjust the window rect for the flags
	RECT rect;
	SetRect(&rect, 0, 0, _WIDTH, _HEIGHT);
	const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_CLIPCHILDREN | WS_SYSMENU | WS_MINIMIZEBOX;
	const DWORD exStyle = 0;
	AdjustWindowRectEx(&rect, style, FALSE, exStyle);

	// Create the window
	_WINDOW = CreateWindowEx(exStyle, _GetTitle(), _GetTitle(), style, 0, 0, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, GetModuleHandle(NULL), NULL);
	if (_WINDOW == NULL)
		return (int)GetLastError();

	// Move the window to be in the center of the screen
	RECT system;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &system, 0);
	MoveWindow(_WINDOW, (system.right - system.left) / 2 - (rect.right - rect.left) / 2, (system.bottom - system.top) / 2 - (rect.bottom - rect.top) / 2, rect.right - rect.left, rect.bottom - rect.top, TRUE);

	// Create a DirectX reference
	_DIRECTX = Direct3DCreate9(D3D_SDK_VERSION);
	if (_DIRECTX == NULL)
		return (int)GetLastError();

	// Set the DirectX options
	D3DPRESENT_PARAMETERS params;
	ZeroMemory(&params, sizeof(params));
	params.BackBufferWidth = _WIDTH;
	params.BackBufferHeight = _HEIGHT;
	params.BackBufferFormat = D3DFMT_UNKNOWN; // Use D3DFMT_A8R8G8B8 for fullscreen mode
	params.BackBufferCount = 1;
	params.SwapEffect = D3DSWAPEFFECT_COPY;
	params.hDeviceWindow = _WINDOW;
	params.Windowed = TRUE;
	params.EnableAutoDepthStencil = TRUE;
	params.AutoDepthStencilFormat = D3DFMT_D24X8;
	
	// Create the device
	result = _DIRECTX->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, _WINDOW, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &_DEVICE);
	if (FAILED(result))
		result = _DIRECTX->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, _WINDOW, D3DCREATE_MIXED_VERTEXPROCESSING, &params, &_DEVICE);
	if (FAILED(result))
		result = _DIRECTX->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, _WINDOW, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &_DEVICE);
	if (FAILED(result))
		return (int)result;

	// Our startup callback
	GSystem::RunStartupCallbacks();

	// Show the window
	ShowWindow(_WINDOW, SW_SHOWNORMAL);

	// Run the event loop
	MSG msg;
	ULONGLONG timer = GetTickCount64();
	while (!_EXIT) {
		
		// Handle normal system events
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Run the timer at the framerate
		if (timer + 1000 / _FPS < GetTickCount64()) {
			timer = GetTickCount64();
			
#if DEBUG
			static const int RECORDS = 60;
			static uint64 timeRecord[RECORDS];
			static int_t timeIndex = 0;
			uint64 time = GSystem::GetMicroseconds();
#endif

			// Set render states
			_DEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			_DEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			_DEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			_DEVICE->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE); // Needed for textures
			_DEVICE->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			_DEVICE->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			_DEVICE->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			_DEVICE->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

			// Clear the buffer
			_DEVICE->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0x00, 0x00, 0x00), 1.0f, NULL);
			
			// Reset the world matrix
			//D3DXMATRIXA16 world;
			//D3DXMatrixIdentity(&world);
			//_DEVICE->SetTransform(D3DTS_WORLD, &world);

			// Reset the view matrix (camera)
			//D3DXMATRIXA16 view;
			//D3DXMatrixLookAtLH(&view, &_CAMERA_EYE, &_CAMERA_AT, &_CAMERA_UP);
			//_DEVICE->SetTransform(D3DTS_VIEW, &view);

			// Reset the projection matrix
			//D3DXMATRIXA16 projection;
			//D3DXMatrixPerspectiveFovLH(&projection, D3DXToRadian(45.0f), (FLOAT)_WIDTH / (FLOAT)_HEIGHT, 1.0f, D3DX_16F_MAX);
			//_DEVICE->SetTransform(D3DTS_PROJECTION, &projection);

			// Begin the current scene
			_DEVICE->BeginScene();

			// Our render callback
			GSystem::RunDrawCallbacks();

			// End the current scene
			_DEVICE->EndScene();

			// Render to screen
			_DEVICE->Present(NULL, NULL, NULL, NULL);

#if DEBUG
			time = GSystem::GetMicroseconds() - time;
			timeRecord[timeIndex++] = time;
			if (timeIndex >= RECORDS) timeIndex = 0;
			uint64 average = 0;
			for (int_t i = 0; i < RECORDS; i++)
				average += timeRecord[i];
			average /= RECORDS;
			if (timeIndex % RECORDS == 0)
				GSystem::Print("%d FPS\n", (int)(1000000 / average));
#endif

		}
	}

	// Hide the window to avoid flicker
	ShowWindow(_WINDOW, SW_HIDE);

	// Our shutdown callback
	GSystem::RunShutdownCallbacks();

	// Delete all the callbacks
	GSystem::DeleteAllCallbacks();

	// Shutdown Direct X
	if (_DEVICE) {
		_DEVICE->Release();
		_DEVICE = NULL;
	}
	if (_DIRECTX) {
		_DIRECTX->Release();
		_DIRECTX = NULL;
	}

	// Cleanup and destroy the window
	if (_WINDOW != NULL) {
		DestroyWindow(_WINDOW);
		UnregisterClass(_GetTitle(), GetModuleHandle(NULL));
		_WINDOW = NULL;
	}

	return 0;
}

#endif // PLATFORM_WINDOWS
