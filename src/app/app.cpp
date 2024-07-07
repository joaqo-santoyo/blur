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



WindowInfo windowInfo;
struct App {
    Graphics graphics;

    FraH  frameA;
    
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
    
    RenderPass pass0;
    RenderPass pass1;
    
    Image image;
    float radius;
    int texture;
    int textureUnit;
    MeshH quadPos;
    MeshH quadTex;

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
    
    app.frameA = app.graphics.addFrame(windowInfo.width, windowInfo.height);

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


    if (!app.graphics.init(windowInfo.scaleFactor, windowInfo.width, windowInfo.height)) {
        return 0;
    }

    float quadPosData[] = {
        -1, -1, 0,
         1,  1, 0,
        -1,  1, 0,
        -1, -1, 0,
         1, -1, 0,
         1,  1, 0,
    };
    
    float quadTexData[] = {
        0, 0,
        1, 1,
        0, 1,
        0, 0,
        1, 0,
        1, 1,
    };
    app.quadPos = app.graphics.addMesh(3, 6, quadPosData, sizeof(quadPosData));
    app.quadTex = app.graphics.addMesh(2, 6, quadTexData, sizeof(quadTexData));
    app.texture = app.graphics.addTexture(app.image);
    app.textureUnit = 0; // Always the same texture unit
    app.radius = 5.0f;

    // Horizontal blur pass
    app.pass0 = {
        app.frameA,
        app.shaderHoriBlur,
        app.texture,
        app.textureUnit,
        {
            { app.shaderHoriBlur_uTexture, app.textureUnit },
            { app.shaderHoriBlur_uWidth,   windowInfo.width       },
            { app.shaderHoriBlur_uHeight,  windowInfo.height      },
        },
        {
            { app.shaderHoriBlur_uRadius,  app.radius }
        },
        {
            { app.shaderHoriBlur_aPosition, app.quadPos },
            { app.shaderHoriBlur_aTexture,  app.quadTex }
        },
        6
    };

    // Vertical blur pass
    app.pass1 = {
        invFraH,
        app.shaderVertBlur,
        app.graphics.getFrameTexture(app.frameA),
        app.textureUnit,
        {
            { app.shaderVertBlur_uTexture, app.textureUnit },
            { app.shaderVertBlur_uWidth,   windowInfo.width       },
            { app.shaderVertBlur_uHeight,  windowInfo.height      },
        },
        {
            { app.shaderVertBlur_uRadius,  app.radius }
        },
        {
            { app.shaderVertBlur_aPosition, app.quadPos },
            { app.shaderVertBlur_aTexture,  app.quadTex }
        },
        6
    };

    // Uncomment to render the original image
//    app.pass1 = {
//        invFraH,
//        app.shaderImage,
//        app.texture,
//        app.textureUnit,
//        {
//            { app.shaderImage_uTexture, app.textureUnit },
//        },
//        { },
//        {
//            { app.shaderImage_aPosition, app.quadPos },
//            { app.shaderImage_aTexture,  app.quadTex }
//        },
//        6
//    };

    return 1;
}

extern "C" int appRender(void) {
    app.graphics.clear();
    app.graphics.render(app.pass0);
    app.graphics.render(app.pass1);
    return 1;
}

extern "C" int appDeinit(void) {
    return 1;
}
