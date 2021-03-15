
#define GLEW_STATIC

#include <windows.h>
#include <dwmapi.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <thread>
#include <WinUser.h>

#include <gl/GL.h>

LRESULT CALLBACK wnd_proc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp);


LRESULT CALLBACK WndProc(HWND const window,
    UINT const message,
    WPARAM const wparam,
    LPARAM const lparam) {
    return DefWindowProc(window, message, wparam, lparam);
}


const wchar_t* registerWindowClass(const wchar_t* name) {
    WNDCLASS window_class{};
    window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    window_class.lpszClassName = name;
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = GetModuleHandle(nullptr);
    window_class.hIcon =
        LoadIcon(window_class.hInstance, MAKEINTRESOURCE(101));
    window_class.hbrBackground = 0;
    window_class.lpszMenuName = nullptr;
    window_class.lpfnWndProc = WndProc;
    RegisterClass(&window_class);

    return name;
}


RECT GetClientArea(HWND window) {
    RECT frame;
    GetClientRect(window, &frame);
    return frame;
}


HWND createMainWindow() {
    const wchar_t* window_class = registerWindowClass(L"test_main_window");
    HWND window = CreateWindow(
        window_class, L"hello", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        0, 0,
        800, 800,
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

    LONG Style = GetWindowLong(window, GWL_EXSTYLE);
    SetWindowLong(window, GWL_EXSTYLE, Style | WS_EX_LAYERED);
    SetLayeredWindowAttributes(window, 0, 150, LWA_ALPHA);

    return window;
}

void setChildWindow(HWND parent, HWND child, int offset_x, int offset_y) {
    SetParent(child, parent);
    RECT frame = GetClientArea(parent);

    MoveWindow(child, frame.left + offset_x, frame.top + offset_y, frame.right - frame.left,
        frame.bottom - frame.top, true);

    SetFocus(child);
}

HWND createWindow(LPCWSTR title, bool pop=false, HBRUSH bk=0) {
    if (pop) {
        WNDCLASSEX wcx = {};
        wcx.cbSize = sizeof(WNDCLASSEX);
        wcx.lpszClassName = title;
        wcx.lpfnWndProc = wnd_proc;
        RegisterClassEx(&wcx);
    }
    else {
        WNDCLASS window_class{};
        window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
        window_class.lpszClassName = title;
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.cbClsExtra = 0;
        window_class.cbWndExtra = 0;
        window_class.hInstance = GetModuleHandle(nullptr);
        window_class.hIcon = nullptr;
        window_class.hbrBackground = bk;
        window_class.lpszMenuName = nullptr;
        window_class.lpfnWndProc = WndProc;
        RegisterClass(&window_class);
    }


    HWND hWnd;
    if (pop)
        hWnd = CreateWindowEx(WS_EX_APPWINDOW, title, title,
            WS_POPUP, 100, 100, 800, 400, NULL, NULL, NULL, NULL);
    else
        hWnd = CreateWindowEx(0,title, title,
            WS_CHILD | WS_VISIBLE, 100, 100, 800, 400, HWND_MESSAGE, NULL, NULL, NULL);

    HDC device_context = GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;

    int pixel_format = ChoosePixelFormat(device_context, &pfd);
    SetPixelFormat(device_context, pixel_format, &pfd);

    LONG Style = GetWindowLong(hWnd, GWL_EXSTYLE);
    SetWindowLong(hWnd, GWL_EXSTYLE, Style | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hWnd, 0, 0, LWA_ALPHA);

    return hWnd;
}

int rendererWindow(HWND window, float r, float g, float b, float a) {
    HDC device_context = GetDC(window);
    HGLRC rendering_context = wglCreateContext(device_context);
    wglMakeCurrent(device_context, rendering_context);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(r, g, b, a);

    glClear(GL_COLOR_BUFFER_BIT);
    wglSwapLayerBuffers(device_context, WGL_SWAP_MAIN_PLANE);

 /*   while (true)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        wglSwapLayerBuffers(device_context, WGL_SWAP_MAIN_PLANE);
    }*/
    

    return 0;
}

void runRendererWorker(HWND window, float r, float g, float b, float a) {
    //rendererWindow(window);
    auto thr = std::thread(rendererWindow, window, r, g, b, a);

    thr.detach();
}

int main(void) {
    HWND main_window = createMainWindow();

    auto child_window = createWindow(L"child_window_1", false, CreateSolidBrush(RGB(255, 0, 0)));
    setChildWindow(main_window, child_window, 100, 100);
    //runRendererWorker(child_window, 0, 1, 0, 0.3);

    auto child_window2 = createWindow(L"child_window_2", false, CreateSolidBrush(RGB(0, 255, 0)));
    setChildWindow(main_window, child_window2, 0, 200);
    //runRendererWorker(child_window2, 0, 0, 0.2, 0.2);

    //auto child_window3 = createWindow(L"child_window_3", true);
    //ShowWindow(child_window3, 1);
    //runRendererWorker(child_window3, 0, 0, 0.2, 0.2);

    
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
        
    }
}

LRESULT CALLBACK wnd_proc(HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp) {
    switch (uMsg) {
    case WM_CREATE:
    {
        
    }
    return 0;
    //case WM_PAINT:
    //{
    //    PAINTSTRUCT ps;
    //    HDC hdc = BeginPaint(hWnd, &ps);

    //    RECT rect;
    //    GetClientRect(hWnd, &rect);
    //    SetBkMode(hdc, TRANSPARENT);
    //    DrawText(hdc, L"Hello World!", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    //    EndPaint(hWnd, &ps);
    //}
    //return 0;
    case WM_NCHITTEST: return HTCAPTION;
    default:           return DefWindowProc(hWnd, uMsg, wp, lp);
    }
}