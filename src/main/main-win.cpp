// Copyright Joaquin Santoyo Lopez
#include "app/app.h"
#include "glad/glad.h"
#include <Windows.h>
#include <stdio.h>






LRESULT CALLBACK windowProcedure(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SIZE:
        return 0;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(window, msg, wParam, lParam);
    }
}

int main(int argc, char** argv) {

    if (!appEntry(argc, argv)) {
        return 1;
    }

    // Create win32 window
    const wchar_t* windowName = L"Blur";
    const wchar_t* className = L"Blur";
    HINSTANCE instance = GetModuleHandle(NULL);
    WNDCLASS windowClass;
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = (WNDPROC)windowProcedure;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = instance;
    windowClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = className;
    if (RegisterClass(&windowClass) == 0) {
        printf("Error RegisterClass failed\n");
        return 1;
    }
    DWORD style = WS_OVERLAPPEDWINDOW;
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = windowInfo.width;
    rect.bottom = windowInfo.height;
    AdjustWindowRect(&rect, style, FALSE);
    HWND windowHandle = CreateWindow(
        className,
        windowName,
        style,
        0,
        0,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        instance,
        NULL
    );
    if (windowHandle == NULL) {
        printf("Error: CreateWindow failed\n");
        return 1;
    }
    windowInfo.scaleFactor = 1;

    // Create opengl context (https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_%28WGL%29)
    HDC windowDevice = GetDC(windowHandle);
    if (windowDevice == NULL) {
        printf("Error: GetDC failed\n");
        return 1;
    }
    PIXELFORMATDESCRIPTOR pfd;
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cRedShift = 0;
    pfd.cGreenBits = 8;
    pfd.cGreenShift = 0;
    pfd.cBlueBits = 8;
    pfd.cBlueShift = 0;
    pfd.cAlphaBits = 8;
    pfd.cAlphaShift = 0;
    pfd.cAccumBits = 0;
    pfd.cAccumRedBits = 0;
    pfd.cAccumGreenBits = 0;
    pfd.cAccumBlueBits = 0;
    pfd.cAccumAlphaBits = 0;
    pfd.cDepthBits = 16;
    pfd.cStencilBits = 8;
    pfd.cAuxBuffers = 0;
    pfd.iLayerType = 0;
    pfd.bReserved = 0;
    pfd.dwLayerMask = 0;
    pfd.dwVisibleMask = 0;
    pfd.dwDamageMask = 0;
    int pf = ChoosePixelFormat(windowDevice, &pfd);
    SetPixelFormat(windowDevice, pf, &pfd);
    HGLRC windowRenderer = wglCreateContext(windowDevice);
    if (windowRenderer == NULL) {
        printf("Error: wglCreateContext failed\n");
        return 1;
    }
    wglMakeCurrent(windowDevice, windowRenderer);
    gladLoadGL();

    appInit();

    // Enter window loop
    WPARAM running = 1;
    ShowWindow(windowHandle, SW_SHOWNORMAL);
    while (running) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
            if (msg.message == WM_QUIT) {
                running = msg.wParam; // this is 0
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        appRender();
        SwapBuffers(windowDevice);
    }

    // Cleanup
    appDeinit();
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(windowRenderer);
    ReleaseDC(windowHandle, windowDevice);
    DestroyWindow(windowHandle);
    return 0;
}
