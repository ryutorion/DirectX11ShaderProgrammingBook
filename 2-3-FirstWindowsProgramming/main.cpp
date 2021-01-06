#include <cstdint>
#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE,
	_In_ LPSTR,
	_In_ int nShowCmd
)
{
	WNDCLASSEX wcx
	{
		.cbSize = sizeof(wcx),
		.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		.lpfnWndProc = WindowProc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = hInstance,
		.hIcon = LoadIcon(nullptr, IDI_APPLICATION),
		.hCursor = LoadCursor(nullptr, IDC_ARROW),
		.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
		.lpszMenuName = nullptr,
		.lpszClassName = "2-3-FirstWindowsProgramming",
		.hIconSm = LoadIcon(nullptr, IDI_WINLOGO)
	};
	if(!RegisterClassEx(&wcx))
	{
		return 0;
	}

	constexpr int32_t client_width = 1920;
	constexpr int32_t client_height = 1080;

	const int32_t screen_width = GetSystemMetrics(SM_CXSCREEN);
	const int32_t screen_height = GetSystemMetrics(SM_CYSCREEN);

	RECT window_rect
	{
		(screen_width - client_width) / 2,
		(screen_height - client_height) / 2,
		(screen_width + client_width) / 2,
		(screen_height + client_height) / 2
	};

	DWORD window_style = WS_OVERLAPPEDWINDOW ^ WS_SIZEBOX;
	DWORD window_style_ex = WS_EX_ACCEPTFILES;

	AdjustWindowRectEx(&window_rect, window_style, FALSE, window_style_ex);

	HWND hWnd = CreateWindowEx(
		window_style_ex,
		wcx.lpszClassName,
		wcx.lpszClassName,
		window_style,
		window_rect.left,
		window_rect.top,
		window_rect.right - window_rect.left,
		window_rect.bottom - window_rect.top,
		nullptr,
		nullptr,
		wcx.hInstance,
		nullptr
	);
	if(hWnd == nullptr)
	{
		return 0;
	}

	ShowWindow(hWnd, nShowCmd);

	MSG msg;
	while(true)
	{
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				break;
			}

			DispatchMessage(&msg);
		}
	}

	return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}