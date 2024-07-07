#include "app.h"
#include "../graphics/graphics.h"
#include "../images/images.h"
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


enum FrameName {
    FrameA,
    FrameNameCount
};

WindowInfo windowInfo;
struct App {
    Graphics graphics;
    Image image;
    int texture;
    int textureUnit;
    int quad;
    float radius;
    RenderPass pass0;
    RenderPass pass1;

    ShaH  shaderImage;
    UniH  shaderImage_uTexture;
    AttrH shaderImage_aPosition;
    AttrH shaderImage_aTexture;

    ShaH  shaderHoriBlur;
    UniH  shaderHoriBlur_uTexture;
    UniH  shaderHoriBlur_uWidth;
    UniH  shaderHoriBlur_uHeight;
    UniH  shaderHoriBlur_uRadius;
    AttrH shaderHoriBlur_aPosition;
    AttrH shaderHoriBlur_aTexture;

    ShaH  shaderVertBlur;
    UniH  shaderVertBlur_uTexture;
    UniH  shaderVertBlur_uWidth;
    UniH  shaderVertBlur_uHeight;
    UniH  shaderVertBlur_uRadius;
    AttrH shaderVertBlur_aPosition;
    AttrH shaderVertBlur_aTexture;

} app;

extern "C" int appEntry(int argc, char** argv) {
    if (argc <= 1) {
        printf("Usage: blur <image_filename>");
        return 0;
    }
    if (!app.image.read(argv[1])) {
        return 0;
    }
    windowInfo.width = app.image.width;
    windowInfo.height = app.image.height;
    return 1;
}

extern "C" int appInit() {
    
    FrameInfo frameA;
    frameA.width = windowInfo.width;
    frameA.height = windowInfo.height;

    app.shaderImage = app.graphics.addShader(
        "ShaderImage",
        imageVertexSource,
        imageFragmentSource,
        {
            "#version 120\n",
        },
        {
            { app.shaderImage_uTexture, "uTexture" },
        },
        {
            { app.shaderImage_aPosition, "aPosition" },
            { app.shaderImage_aTexture,  "aTexture"  },
        }
    );

    app.shaderHoriBlur = app.graphics.addShader(
        "HorizontalBlur",
        blurVertexSource,
        blurFragmentSource,
        {
            "#version 120\n",
            "#define HORIZONTAL\n",
            "#define KERNEL 11\n",
        },
        {
            { app.shaderHoriBlur_uTexture, "uTexture" },
            { app.shaderHoriBlur_uWidth,   "uWidth" },
            { app.shaderHoriBlur_uHeight,  "uHeight" },
            { app.shaderHoriBlur_uRadius,  "uRadius" },
        },
        {
            { app.shaderHoriBlur_aPosition, "aPosition" },
            { app.shaderHoriBlur_aTexture,  "aTexture"  },
        }
    );

    app.shaderVertBlur = app.graphics.addShader(
        "VerticalBlur",
        blurVertexSource,
        blurFragmentSource,
        {
            "#version 120\n",
            "#define VERTICAL\n",
            "#define KERNEL 11\n",
        },
        {
            { app.shaderVertBlur_uTexture, "uTexture" },
            { app.shaderVertBlur_uWidth,   "uWidth" },
            { app.shaderVertBlur_uHeight,  "uHeight" },
            { app.shaderVertBlur_uRadius,  "uRadius" },
        },
        {
            { app.shaderVertBlur_aPosition, "aPosition" },
            { app.shaderVertBlur_aTexture,  "aTexture"  },
        }
    );


    GraphicsInfo graphicsInfo;
    graphicsInfo.windowScaleFactor = windowInfo.scaleFactor;
    graphicsInfo.width = windowInfo.width;
    graphicsInfo.height = windowInfo.height;
    graphicsInfo.frames.resize(FrameNameCount);
    graphicsInfo.frames[FrameA] = frameA;
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
    app.texture = app.graphics.addTexture(app.image);
    app.textureUnit = 0; // Always the same texture unit
    app.radius = 5.0f;

    // Horizontal blur pass
    app.pass0.frame = FrameA;
    app.pass0.shaderH = app.shaderHoriBlur;
    app.pass0.textureId = app.texture;
    app.pass0.textureUnit = app.textureUnit;
    app.pass0.uniformsInt = {
        { app.shaderHoriBlur_uTexture, app.textureUnit },
        { app.shaderHoriBlur_uWidth,   windowInfo.width       },
        { app.shaderHoriBlur_uHeight,  windowInfo.height      },
    };
    app.pass0.uniformsFloat = {
        { app.shaderHoriBlur_uRadius,  app.radius }
    };
    app.pass0.attributes = {
        { app.shaderHoriBlur_aPosition, app.quad, 3, false, 5 * sizeof(float), 0 },
        { app.shaderHoriBlur_aTexture,  app.quad, 2, false, 5 * sizeof(float), 3 * sizeof(float) }
    };
    app.pass0.vertexCount = 6;

    // Vertical blur pass
    app.pass1.frame = -1;
    app.pass1.shaderH = app.shaderVertBlur;
    app.pass1.textureId = app.graphics.getFrameTexture(FrameA);
    app.pass1.textureUnit = app.textureUnit;
    app.pass1.uniformsInt = {
        { app.shaderVertBlur_uTexture, app.textureUnit },
        { app.shaderVertBlur_uWidth,   windowInfo.width       },
        { app.shaderVertBlur_uHeight,  windowInfo.height      },
    };
    app.pass1.uniformsFloat = {
        { app.shaderVertBlur_uRadius,  app.radius }
    };
    app.pass1.attributes = {
        { app.shaderVertBlur_aPosition, app.quad, 3, false, 5 * sizeof(float), 0 },
        { app.shaderVertBlur_aTexture,  app.quad, 2, false, 5 * sizeof(float), 3 * sizeof(float) }
    };
    app.pass1.vertexCount = 6;

    // Uncomment to render the original image
//    app.pass1.frame = -1;
//    app.pass1.shaderH = app.shaderImage;
//    app.pass1.textureId = app.texture;
//    app.pass1.textureUnit = app.textureUnit;
//    app.pass1.uniformsInt = {
//        { app.shaderImage_uTexture, app.textureUnit },
//    };
//    app.pass1.uniformsFloat = { };
//    app.pass1.attributes = {
//        { app.shaderImage_aPosition, app.quad, 3, false, 5 * sizeof(float), 0 },
//        { app.shaderImage_aTexture,  app.quad, 2, false, 5 * sizeof(float), 3 * sizeof(float) }
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
