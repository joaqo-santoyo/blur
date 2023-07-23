#pragma once
#include <string>
#include <vector>

// Simple graphics engine

struct ShaderInfo {
    std::string name;
    const char* vertexShader;
    const char* fragmentShader;
    std::vector<const char*> uniforms;
    std::vector<const char*> attributes;
};

struct AttributeInfo {
    int attr;
    int buffer;
    int count;
    bool normalized;
    int stride;
    int offset;
};

struct GraphicsInfo {
    int width;
    int height;
    std::vector<ShaderInfo> shaders;
};

struct Shader {
    int program;
    std::vector<int> uniforms;
    std::vector<int> attributes;
};

class Graphics {
public:
    bool initialized = false;
    Graphics(const GraphicsInfo& info);
    ~Graphics();
    int addMesh(float* data, int size);
    int addTexture(int width, int height, int channels, unsigned char* pixels);
    void render(
        int shaderId,
        int texture,
        int textureUnit,
        std::vector<std::pair<int, int>> uniformsInt,
        std::vector<std::pair<int, float>> uniformsFloat,
        std::vector<AttributeInfo> attributes,
        int vertexCount
    );

private:
    int width;
    int height;
    std::vector<Shader> shaders;
};