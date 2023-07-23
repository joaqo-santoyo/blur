// Copyright Joaquin Santoyo Lopez
#include <Windows.h>
#include <stdio.h>
#include "glad/glad.h"
#define STB_IMAGE_IMPLEMENTATION
#include "images/stb_image.h"
#include "graphics/graphics.h"



const char* imageVertexSource = R"(
precision mediump float;
attribute vec3 aPosition;
attribute vec2 aTexture;
varying   vec2 vTexture;
void main() {
    gl_Position = vec4(aPosition.x, aPosition.y, aPosition.z, 1.0);
    vTexture = vec2(aTexture.x, aTexture.y);
}
)";
const char* imageFragmentSource = R"(
precision mediump float;
uniform sampler2D uTexture;
varying vec2      vTexture;
void main() {
    gl_FragColor = vec4(texture2D(uTexture, vTexture).rgb, 1.0);
}
)";

enum ShaderName {
    ShaderImage,
    ShaderNameCount
};

enum ShaderImageUniformName {
    ShaderImageUniformNameTexture,
    ShaderImageUniformNameCount
};

enum ShaderImageAttributeName {
    ShaderImageAttributeNamePosition,
    ShaderImageAttributeNameTexture,
    ShaderImageAttributeNameCount
};

















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

    if (argc <= 1) {
        printf("Usage: blur.exe <image_filename>");
        return 1;
    }
    // Create the image first of all so we set the viewport accordingly
    int imageWidth, imageHeight, imageChannels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* pixels = stbi_load(argv[1], &imageWidth, &imageHeight, &imageChannels, 0);
    if (pixels == NULL) {
        printf("Error loading the image\n");
        exit(1);
    }
    printf("Image: { %s, %d x %d x %d }\n", argv[1], imageWidth, imageHeight, imageChannels);

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
    rect.right = imageWidth;
    rect.bottom = imageHeight;
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





    // Configure shaders
    ShaderInfo shaderImage;
    shaderImage.name = "ShaderImage";
    shaderImage.vertexShader = imageVertexSource;
    shaderImage.fragmentShader = imageFragmentSource;
    shaderImage.uniforms.resize(ShaderImageUniformNameCount);
    shaderImage.uniforms[ShaderImageUniformNameTexture] = "uTexture";
    shaderImage.attributes.resize(ShaderImageAttributeNameCount);
    shaderImage.attributes[ShaderImageAttributeNamePosition] = "aPosition";
    shaderImage.attributes[ShaderImageAttributeNameTexture] = "aTexture";

    // Configure graphics
    GraphicsInfo graphicsInfo;
    graphicsInfo.width = imageWidth;
    graphicsInfo.height = imageHeight;
    graphicsInfo.shaders.resize(ShaderNameCount);
    graphicsInfo.shaders[ShaderImage] = shaderImage;
    Graphics graphics(graphicsInfo);

    // Configure graphics resources
    float quadData[] = {
        -1, -1, 0, 0, 0,
         1,  1, 0, 1, 1,
        -1,  1, 0, 0, 1,
        -1, -1, 0, 0, 0,
         1, -1, 0, 1, 0,
         1,  1, 0, 1, 1,
    };
    int quad = graphics.addMesh(quadData, 6 * 5 * sizeof(float));
    int texture = graphics.addTexture(imageWidth, imageHeight, imageChannels, pixels);
    int textureUnit = 0; // Always the same texture unit

    RenderPass pass;
    pass.shaderId = ShaderImage;
    pass.textureId = texture;
    pass.textureUnit = textureUnit;
    pass.uniformsInt = {
        { ShaderImageUniformNameTexture, textureUnit },
    };
    pass.uniformsFloat = { };
    pass.attributes = {
        { ShaderImageAttributeNamePosition, quad, 3, false, 5 * sizeof(float), 0 },
        { ShaderImageAttributeNameTexture,  quad, 2, false, 5 * sizeof(float), 3 * sizeof(float) }
    };
    pass.vertexCount = 6;

    int k = 0;
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
        k++;
        if (k % 2 == 0)
            graphics.addRenderPass(pass);
        graphics.render();
        SwapBuffers(windowDevice);
    }

    // Cleanup
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(windowRenderer);
    ReleaseDC(windowHandle, windowDevice);
    DestroyWindow(windowHandle);
    return 0;
}
