#pragma once

class Image {
public:
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* pixels = nullptr;
    
    Image();
    ~Image();
    bool read(const char* filename);
};
