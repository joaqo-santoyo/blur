#pragma once
#include <string>
#include <vector>
#include "../images/images.h"

// Simple graphics engine
// Only GL_FLOAT atttributes for now
// Only int or float uniforms for now
// Only a single texture and texture unit for now


struct FraH  { int idx; };
struct ShaH  { int idx; };
struct UniH  { int idx; };
struct AttrH { int idx; };

extern FraH  invFraH;
extern ShaH  invShaH;
extern UniH  invUniH;
extern AttrH invAttrH;



struct AttributeInfo {
    AttrH attrH;
    int bufferId;
    int count;
    bool normalized;
    int stride;
    int offset;
};

struct RenderPass {
    FraH frame;
    ShaH shader;
    int textureId;
    int textureUnit;
    std::vector<std::pair<UniH, int>> uniformsInt;
    std::vector<std::pair<UniH, float>> uniformsFloat;
    std::vector<AttributeInfo> attributes;
    int vertexCount;
};

struct GraphicsInfo {
    float windowScaleFactor;
    int width;
    int height;
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

class GraphicsState;

class Graphics {
public:
    bool initialized = false;
    Graphics();
    ~Graphics();
    bool init(const GraphicsInfo& info);

    FraH addFrame(const int width, const int height);
    ShaH addShader(
        const std::string& name,
        const char* vertexShader,
        const char* fragmentShader,
        const std::vector<const char*>& defines,
        const std::vector<std::pair<UniH&,  const char*>>& uniInfos,
        const std::vector<std::pair<AttrH&, const char*>>& attrInfos
    );

    int addMesh(float* data, int size);
    int addTexture(const Image& image);
    int getFrameTexture(FraH frame);
    void addRenderPass(const RenderPass& pass);
    void render();
    
private:
    GraphicsState* state;
    int width;
    int height;
    int defaultFrameBufferWidth;
    int defaultFrameBufferHeight;
    std::vector<RenderPass> renderPasses;
    std::vector<Shader> shaders;
    std::vector<Frame> frames;
};
