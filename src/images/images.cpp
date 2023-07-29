#include "images.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../images/stb_image.h"

Image::Image() {
    
}

Image::~Image() {
    if (pixels == nullptr) {
        free(pixels);
    }
}

bool Image::read(const char* filename) {
    stbi_set_flip_vertically_on_load(1);
    pixels = stbi_load(filename, &width, &height, &channels, 0);
    if (pixels == NULL) {
        printf("Error loading the image\n");
        return false;
    }
    printf("Image: { %s, %d x %d x %d }\n", filename, width, height, channels);
    return true;
}
