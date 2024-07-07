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
struct TexH  { int idx; };

extern FraH  invFraH;
extern ShaH  invShaH;
extern UniH  invUniH;
extern AttrH invAttrH;
extern MeshH invMeshH;
extern TexH  invTexH;


struct RenderPass {
    FraH frame;
    ShaH shader;
    TexH texture;
    FraH frameIn;
    int textureUnit;
    std::vector<std::pair<UniH,  int>> uniformsInt;
    std::vector<std::pair<UniH,  float>> uniformsFloat;
    std::vector<std::pair<AttrH, MeshH>> attributes;
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
    TexH addTexture(const Image& image);
    void clear();
    void render(const RenderPass& pass);
    
private:
    GraphicsState* state;
};
