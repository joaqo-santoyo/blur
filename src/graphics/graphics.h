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
struct MeshH { int idx; };

extern FraH  invFraH;
extern ShaH  invShaH;
extern UniH  invUniH;
extern AttrH invAttrH;
extern MeshH invMeshH;


struct RenderPass {
    FraH frame;
    ShaH shader;
    int textureId;
    int textureUnit;
    std::vector<std::pair<UniH,  int>> uniformsInt;
    std::vector<std::pair<UniH,  float>> uniformsFloat;
    std::vector<std::pair<AttrH, MeshH>> attributes;
    int vertexCount;
};

class GraphicsState;

class Graphics {
public:
    bool initialized = false;
    Graphics();
    ~Graphics();
    bool init(const float windowScaleFactor, const int width, const int height);

    FraH addFrame(const int width, const int height);
    ShaH addShader(
        const std::string& name,
        const char* vertexShader,
        const char* fragmentShader,
        const std::vector<const char*>& defines,
        const std::vector<std::pair<UniH&,  const char*>>& uniInfos,
        const std::vector<std::pair<AttrH&, const char*>>& attrInfos
    );
    MeshH addMesh(int dimensions, int vertexCount, float* data, int size);
    int addTexture(const Image& image);
    int getFrameTexture(FraH frame);
    void clear();
    void render(const RenderPass& pass);
    
private:
    GraphicsState* state;
};
