#pragma once
#include <string>
#include <vector>
#include "../images/images.h"

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
    std::vector<const char*> defines;
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
    float windowScaleFactor;
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
    Graphics();
    ~Graphics();
    bool init(const GraphicsInfo& info);
    int addMesh(float* data, int size);
    int addTexture(const Image& image);
    int getFrameTexture(int frame);
    void addRenderPass(const RenderPass& pass);
    void render();
    
private:
    int width;
    int height;
    int defaultFrameBufferWidth;
    int defaultFrameBufferHeight;
    std::vector<RenderPass> renderPasses;
    std::vector<Shader> shaders;
    std::vector<Frame> frames;
};
