#include "app.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../images/stb_image.h"
#include "../graphics/graphics.h"
#include <stdio.h>


const char* imageVertexSource = R"(
attribute vec3 aPosition;
attribute vec2 aTexture;
varying   vec2 vTexture;
void main() {
    gl_Position = vec4(aPosition.x, aPosition.y, aPosition.z, 1.0);
    vTexture = vec2(aTexture.x, aTexture.y);
}
)";
const char* imageFragmentSource = R"(
uniform sampler2D uTexture;
varying vec2      vTexture;
void main() {
    gl_FragColor = vec4(texture2D(uTexture, vTexture).rgb, 1.0);
}
)";

const char* blurVertexSource = R"(
attribute vec3 aPosition;
attribute vec2 aTexture;
varying   vec2 vTexture;
void main() {
    gl_Position = vec4(aPosition.x, aPosition.y, aPosition.z, 1.0);
    vTexture = vec2(aTexture.x, aTexture.y);
}
)";

const char* blurFragmentSource = R"(
uniform sampler2D uTexture;
uniform int       uWidth;
uniform int       uHeight;
uniform float     uRadius;
varying vec2      vTexture;
void main() {
    vec2 uTextureSize = vec2(uWidth, uHeight);

    // Fixed-valued kernels
    //#if (KERNEL == 5)
    //int n = 5;
    //float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
    //#else
    //int n = 11;
    //float weight[11] = { 0.082607, 0.080977, 0.076276, 0.069041, 0.060049,
    //                     0.050187, 0.040306, 0.031105, 0.023066, 0.016436, 0.011254 };
    //#endif

    // Compute kernel in runtime. Might be too heavy?
    // See https://stackoverflow.com/questions/8204645/implementing-gaussian-blur-how-to-calculate-convolution-matrix-kernel
    float weight[KERNEL];
    float x = 2.0 * pow(uRadius, 2.0);
    float sum = 0;
    for (int i = 0; i < KERNEL; i++) {
        weight[i] = exp(-(pow(float(i), 2.0) / x));
        sum += weight[i];
    }
    // Sum the other wing of the kernel for proper normalization (minus center)
    for (int i = 1; i < KERNEL; i++) {
        sum += weight[i];
    }
    for (int i = 0; i < KERNEL; i++) {
        weight[i] /= sum;
    }


    vec2 texOffset = 1.0 / uTextureSize; // gets size of single texel
    vec3 result = texture2D(uTexture, vTexture).rgb * weight[0]; // current fragment's contribution
    #ifdef HORIZONTAL
    for (int i = 1; i < KERNEL; i++) {
        result += texture2D(uTexture, vTexture + vec2(texOffset.x * float(i), 0.0)).rgb * weight[i];
        result += texture2D(uTexture, vTexture - vec2(texOffset.x * float(i), 0.0)).rgb * weight[i];
    }
    #else
    for (int i = 1; i < KERNEL; i++) {
        result += texture2D(uTexture, vTexture + vec2(0.0, texOffset.y * float(i))).rgb * weight[i];
        result += texture2D(uTexture, vTexture - vec2(0.0, texOffset.y * float(i))).rgb * weight[i];
    }
    #endif

    gl_FragColor = vec4(result, 1.0);
}
)";

enum ShaderName {
    ShaderImage,
    ShaderHoriBlur,
    ShaderVertBlur,
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

enum ShaderHoriBlurUniformName {
    ShaderHoriBlurUniformNameTexture,
    ShaderHoriBlurUniformNameWidth,
    ShaderHoriBlurUniformNameHeight,
    ShaderHoriBlurUniformNameRadius,
    ShaderHoriBlurUniformNameCount
};

enum ShaderHoriBlurAttributeName {
    ShaderHoriBlurAttributeNamePosition,
    ShaderHoriBlurAttributeNameTexture,
    ShaderHoriBlurAttributeNameCount
};

enum ShaderVertBlurUniformName {
    ShaderVertBlurUniformNameTexture,
    ShaderVertBlurUniformNameWidth,
    ShaderVertBlurUniformNameHeight,
    ShaderVertBlurUniformNameRadius,
    ShaderVertBlurUniformNameCount
};

enum ShaderVertBlurAttributeName {
    ShaderVertBlurAttributeNamePosition,
    ShaderVertBlurAttributeNameTexture,
    ShaderVertBlurAttributeNameCount
};

enum FrameName {
    FrameA,
    FrameNameCount
};

WindowInfo windowInfo;
struct App {
    Graphics graphics;
    int channels;
    unsigned char* pixels;
    int texture;
    int textureUnit;
    int quad;
    float radius;
    RenderPass pass0;
    RenderPass pass1;
} app;

extern "C" int appEntry(int argc, char** argv) {
    if (argc <= 1) {
        printf("Usage: blur.exe <image_filename>");
        return 0;
    }
    int imageWidth, imageHeight, imageChannels;
    stbi_set_flip_vertically_on_load(1);
    app.pixels = stbi_load(argv[1], &imageWidth, &imageHeight, &imageChannels, 0);
    if (app.pixels == NULL) {
        printf("Error loading the image\n");
        exit(1);
    }
    printf("Image: { %s, %d x %d x %d }\n", argv[1], imageWidth, imageHeight, imageChannels);
    windowInfo.width = imageWidth;
    windowInfo.height = imageHeight;
    app.channels = imageChannels;
    return 1;
}

extern "C" int appInit() {
    
    FrameInfo frameA;
    frameA.width = windowInfo.width;
    frameA.height = windowInfo.height;

    ShaderInfo shaderImage;
    shaderImage.name = "ShaderImage";
    shaderImage.defines = {
        "#version 120\n",
    };
    shaderImage.vertexShader = imageVertexSource;
    shaderImage.fragmentShader = imageFragmentSource;
    shaderImage.uniforms.resize(ShaderImageUniformNameCount);
    shaderImage.uniforms[ShaderImageUniformNameTexture] = "uTexture";
    shaderImage.attributes.resize(ShaderImageAttributeNameCount);
    shaderImage.attributes[ShaderImageAttributeNamePosition] = "aPosition";
    shaderImage.attributes[ShaderImageAttributeNameTexture] = "aTexture";

    ShaderInfo shaderHoriBlur;
    shaderHoriBlur.name = "HorizontalBlur";
    shaderHoriBlur.defines = {
        "#version 120\n",
        "#define HORIZONTAL\n",
        "#define KERNEL 11\n",
    };
    shaderHoriBlur.vertexShader = blurVertexSource;
    shaderHoriBlur.fragmentShader = blurFragmentSource;
    shaderHoriBlur.uniforms.resize(ShaderHoriBlurUniformNameCount);
    shaderHoriBlur.uniforms[ShaderHoriBlurUniformNameTexture] = "uTexture";
    shaderHoriBlur.uniforms[ShaderHoriBlurUniformNameWidth] = "uWidth";
    shaderHoriBlur.uniforms[ShaderHoriBlurUniformNameHeight] = "uHeight";
    shaderHoriBlur.uniforms[ShaderHoriBlurUniformNameRadius] = "uRadius";
    shaderHoriBlur.attributes.resize(ShaderHoriBlurAttributeNameCount);
    shaderHoriBlur.attributes[ShaderHoriBlurAttributeNamePosition] = "aPosition";
    shaderHoriBlur.attributes[ShaderHoriBlurAttributeNameTexture] = "aTexture";

    ShaderInfo shaderVertBlur;
    shaderVertBlur.name = "VerticalBlur";
    shaderVertBlur.defines = {
        "#version 120\n",
        "#define VERTICAL\n",
        "#define KERNEL 11\n",
    };
    shaderVertBlur.vertexShader = blurVertexSource;
    shaderVertBlur.fragmentShader = blurFragmentSource;
    shaderVertBlur.uniforms.resize(ShaderVertBlurUniformNameCount);
    shaderVertBlur.uniforms[ShaderVertBlurUniformNameTexture] = "uTexture";
    shaderVertBlur.uniforms[ShaderVertBlurUniformNameWidth] = "uWidth";
    shaderVertBlur.uniforms[ShaderVertBlurUniformNameHeight] = "uHeight";
    shaderVertBlur.uniforms[ShaderVertBlurUniformNameRadius] = "uRadius";
    shaderVertBlur.attributes.resize(ShaderVertBlurAttributeNameCount);
    shaderVertBlur.attributes[ShaderVertBlurAttributeNamePosition] = "aPosition";
    shaderVertBlur.attributes[ShaderVertBlurAttributeNameTexture] = "aTexture";

    GraphicsInfo graphicsInfo;
    graphicsInfo.windowScaleFactor = windowInfo.scaleFactor;
    graphicsInfo.width = windowInfo.width;
    graphicsInfo.height = windowInfo.height;
    graphicsInfo.frames.resize(FrameNameCount);
    graphicsInfo.frames[FrameA] = frameA;
    graphicsInfo.shaders.resize(ShaderNameCount);
    graphicsInfo.shaders[ShaderImage] = shaderImage;
    graphicsInfo.shaders[ShaderHoriBlur] = shaderHoriBlur;
    graphicsInfo.shaders[ShaderVertBlur] = shaderVertBlur;
    if (!app.graphics.init(graphicsInfo)) {
        return 0;
    }

    float quadData[] = {
        -1, -1, 0, 0, 0,
         1,  1, 0, 1, 1,
        -1,  1, 0, 0, 1,
        -1, -1, 0, 0, 0,
         1, -1, 0, 1, 0,
         1,  1, 0, 1, 1,
    };
    app.quad = app.graphics.addMesh(quadData, 6 * 5 * sizeof(float));
    app.texture = app.graphics.addTexture(windowInfo.width, windowInfo.height, app.channels, app.pixels);
    app.textureUnit = 0; // Always the same texture unit
    app.radius = 5.0f;

    // Horizontal blur pass
    app.pass0.frame = FrameA;
    app.pass0.shaderId = ShaderHoriBlur;
    app.pass0.textureId = app.texture;
    app.pass0.textureUnit = app.textureUnit;
    app.pass0.uniformsInt = {
        { ShaderHoriBlurUniformNameTexture, app.textureUnit },
        { ShaderHoriBlurUniformNameWidth,   windowInfo.width       },
        { ShaderHoriBlurUniformNameHeight,  windowInfo.height      },
    };
    app.pass0.uniformsFloat = {
        { ShaderHoriBlurUniformNameRadius,  app.radius }
    };
    app.pass0.attributes = {
        { ShaderHoriBlurAttributeNamePosition, app.quad, 3, false, 5 * sizeof(float), 0 },
        { ShaderHoriBlurAttributeNameTexture,  app.quad, 2, false, 5 * sizeof(float), 3 * sizeof(float) }
    };
    app.pass0.vertexCount = 6;

    // Vertical blur pass
    app.pass1.frame = -1;
    app.pass1.shaderId = ShaderVertBlur;
    app.pass1.textureId = app.graphics.getFrameTexture(FrameA);
    app.pass1.textureUnit = app.textureUnit;
    app.pass1.uniformsInt = {
        { ShaderVertBlurUniformNameTexture, app.textureUnit },
        { ShaderVertBlurUniformNameWidth,   windowInfo.width       },
        { ShaderVertBlurUniformNameHeight,  windowInfo.height      },
    };
    app.pass1.uniformsFloat = {
        { ShaderVertBlurUniformNameRadius,  app.radius }
    };
    app.pass1.attributes = {
        { ShaderVertBlurAttributeNamePosition, app.quad, 3, false, 5 * sizeof(float), 0 },
        { ShaderVertBlurAttributeNameTexture,  app.quad, 2, false, 5 * sizeof(float), 3 * sizeof(float) }
    };
    app.pass1.vertexCount = 6;

    // Uncomment to render the original image
//    app.pass1.frame = -1;
//    app.pass1.shaderId = ShaderImage;
//    app.pass1.textureId = texture;
//    app.pass1.textureUnit = textureUnit;
//    app.pass1.uniformsInt = {
//        { ShaderImageUniformNameTexture, textureUnit },
//    };
//    app.pass1.uniformsFloat = { };
//    app.pass1.attributes = {
//        { ShaderImageAttributeNamePosition, quad, 3, false, 5 * sizeof(float), 0 },
//        { ShaderImageAttributeNameTexture,  quad, 2, false, 5 * sizeof(float), 3 * sizeof(float) }
//    };
//    app.pass1.vertexCount = 6;

    return 1;
}

extern "C" int appRender(void) {
    app.graphics.addRenderPass(app.pass0);
    app.graphics.addRenderPass(app.pass1);
    app.graphics.render();
    return 1;
}

extern "C" int appDeinit(void) {
    return 1;
}
