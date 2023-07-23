#pragma once
#include <string>
#include <vector>

// Simple graphics engine
// Only GL_FLOAT atttributes for now
// Only int or float uniforms for now
// Only a single texture and texture unit for now

struct FrameInfo {
    int width;
    int height;
};

struct ShaderInfo {
    std::string name;
    const char* vertexShader;
    const char* fragmentShader;
    std::vector<const char*> uniforms;
    std::vector<const char*> attributes;
};

struct AttributeInfo {
    int attr;
    int bufferId;
    int count;
    bool normalized;
    int stride;
    int offset;
};

struct RenderPass {
    int frame;
    int shaderId;
    int textureId;
    int textureUnit;
    std::vector<std::pair<int, int>> uniformsInt;
    std::vector<std::pair<int, float>> uniformsFloat;
    std::vector<AttributeInfo> attributes;
    int vertexCount;
};

struct GraphicsInfo {
    int width;
    int height;
    std::vector<FrameInfo> frames;
    std::vector<ShaderInfo> shaders;
};

struct Frame {
    unsigned int id;
    unsigned int texture;
    int width;
    int height;
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
    int getFrameTexture(int frame);
    void addRenderPass(const RenderPass& pass);
    void render();

private:
    int width;
    int height;
    std::vector<RenderPass> renderPasses;
    std::vector<Shader> shaders;
    std::vector<Frame> frames;
};