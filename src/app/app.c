#include "app.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../images/stb_image.h"
#include <stdio.h>

ScreenInfo screenInfo;

int appEntry(int argc, char** argv) {
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
    screenInfo.width = imageWidth;
    screenInfo.height = imageHeight;
    return 1;
}

int appInit(void) {
    
    return 1;
}

int appRender(void) {
    return 1;
}

int appDeinit(void) {
    return 1;
}

