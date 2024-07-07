#include "graphics.h"
#ifdef _WIN32
    #include "glad/glad.h"
#else
    #include <OpenGL/gl.h>
    #include <OpenGL/OpenGL.h>
#endif
#include <stdexcept>


FraH  invFraH  = { -1 };
ShaH  invShaH  = { -1 };
UniH  invUniH  = { -1 };
AttrH invAttrH = { -1 };
MeshH invMeshH = { -1 };



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

struct Mesh {
    unsigned int id;
    int dimensions;
    int vertexCount;
    int size;
};

static int compileShader(const std::vector<const char*>& defines, const char* source, int* shader, GLenum type) {
    int s = glCreateShader(type);
    std::vector<const char*> sources = defines;
    sources.push_back(source);
    glShaderSource(s, static_cast<int>(sources.size()), sources.data(), NULL);
    glCompileShader(s);
    GLint params;
    glGetShaderiv(s, GL_COMPILE_STATUS, &params);
    if (!params) {
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &params);
        if (params > 0) {
            GLchar* log = (GLchar*)malloc(params);
            glGetShaderInfoLog(s, params, &params, log);
            printf("Shader compile log:\n%s", log);
            free(log);
        }
        return 0;
    }
    *shader = s;
    return 1;
}

static int createProgram(int vertexShader, int fragmentShader, int* program) {
    int p = glCreateProgram();
    glAttachShader(p, vertexShader);
    glAttachShader(p, fragmentShader);
    glLinkProgram(p);
    GLint params;
    glGetProgramiv(p, GL_LINK_STATUS, &params);
    if (!params) {
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &params);
        if (params > 0) {
            GLchar* log = (GLchar*)malloc(params);
            glGetProgramInfoLog(p, params, &params, log);
            printf("Program link log: %s", log);
            free(log);
        }
        return 0;
    }
    *program = p;
    return 1;
}

class GraphicsState{
public:
    int width;
    int height;
    int defaultFrameBufferWidth;
    int defaultFrameBufferHeight;
    std::vector<Shader> shaders;
    std::vector<Frame>  frames;
    std::vector<Mesh>   meshes;
};


Graphics::Graphics(){
    state = new GraphicsState();
}

Graphics::~Graphics() {
    delete state;
}

bool Graphics::init(const float windowScaleFactor, const int width, const int height) {
    state->width = width;
    state->height = height;
    state->defaultFrameBufferWidth = width * windowScaleFactor;
    state->defaultFrameBufferHeight = height * windowScaleFactor;
    glViewport(0, 0, width, height);
    initialized = true;
    return true;
}

FraH Graphics::addFrame(const int width, const int height) {
    Frame frame;
    frame.width = width;
    frame.height = height;
    glGenFramebuffers(1, &frame.id);
    glBindFramebuffer(GL_FRAMEBUFFER, frame.id);
    printf("FrameBuffer: %d %d %d\n", frame.id, frame.width, frame.height);
    glGenTextures(1, &frame.texture);
    glBindTexture(GL_TEXTURE_2D, frame.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.width, frame.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame.texture, 0);
    GLint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer error! %d\n", status);
        return invFraH;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    int idx = static_cast<int>(state->frames.size());
    state->frames.push_back(std::move(frame));
    return FraH { idx };
}

ShaH Graphics::addShader(
    const std::string& name,
    const char* vertexShader,
    const char* fragmentShader,
    const std::vector<const char*>& defines,
    const std::vector<std::pair<UniH&,  const char*>>& uniformPairings,
    const std::vector<std::pair<AttrH&, const char*>>& attributePairings
) {
    Shader shader;
    int v, f, p;
    if (!compileShader(defines, vertexShader, &v, GL_VERTEX_SHADER)) {
        //printf("Vertex shader compilation failed: %s", name.c_str());
        throw std::runtime_error("Vertex shader compilation failed " + name);
    };
    if (!compileShader(defines, fragmentShader, &f, GL_FRAGMENT_SHADER)) {
        //printf("Fragment shader compilation failed: %s", name.c_str());
        throw std::runtime_error("Fragment shader compilation failed " + name);
    };
    if (!createProgram(v, f, &p)) {
        //printf("Create program failed: %s", name.c_str());
        throw std::runtime_error("Create shader program failed " + name);
    };
    shader.program = p;
    glUseProgram(shader.program);
    shader.uniforms.resize(uniformPairings.size());
    shader.attributes.resize(attributePairings.size());
    for (int i = 0; i < uniformPairings.size(); i++) {
        const auto& pairing = uniformPairings[i];
        shader.uniforms[i] = glGetUniformLocation(shader.program, pairing.second);
        pairing.first.idx = i;
    }
    for (int i = 0; i < attributePairings.size(); i++) {
        const auto& pairing = attributePairings[i];
        shader.attributes[i] = glGetAttribLocation(shader.program, pairing.second);
        pairing.first.idx = i;
    }
    int idx = static_cast<int>(state->shaders.size());
    state->shaders.push_back(std::move(shader));
    return ShaH{ idx };
}

MeshH Graphics::addMesh(int dimensions, int vertexCount, float* data, int size) {
    Mesh mesh;
    glGenBuffers(1, (GLuint*)&mesh.id);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.id);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * dimensions * sizeof(float), data, GL_STATIC_DRAW);
    mesh.dimensions = dimensions;
    mesh.vertexCount = vertexCount;
    int idx = static_cast<int>(state->meshes.size());
    state->meshes.push_back(std::move(mesh));
    return MeshH{ idx };
}

int Graphics::addTexture(const Image& image) {
    int texture;
    glGenTextures(1, (GLuint*)&texture);
    int glTextureType;
    switch (image.channels) {
    case 3:     glTextureType = GL_RGB;  break;
    case 4:     glTextureType = GL_RGBA; break;
    default:    glTextureType = GL_RGB;  break;
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // See https://stackoverflow.com/questions/58925604/glteximage2d-crashing-program
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        glTextureType,
        image.width,
        image.height,
        0,
        glTextureType,
        GL_UNSIGNED_BYTE,
        image.pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    return texture;
}

int Graphics::getFrameTexture(FraH frame) {
    return state->frames[frame.idx].texture;
}

void Graphics::clear() {
    glClearColor(.1f, .1f, .1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Graphics::render(const RenderPass& pass) {
    Shader& shader = state->shaders[pass.shader.idx];
    glUseProgram(shader.program);

    if (pass.frame.idx != -1) {
        const Frame& frame = state->frames[pass.frame.idx];
        glViewport(0, 0, frame.width, frame.height);
        glBindFramebuffer(GL_FRAMEBUFFER, frame.id);
    } else { // Binding to the default FrameBuffer
        glViewport(0, 0, state->defaultFrameBufferWidth, state->defaultFrameBufferHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glActiveTexture(GL_TEXTURE0 + pass.textureUnit);
    glBindTexture(GL_TEXTURE_2D, pass.textureId);

    for (const auto& uniformInt : pass.uniformsInt) {
        int id = uniformInt.first.idx;
        int value = uniformInt.second;
        glUniform1i(shader.uniforms[id], value);
    }

    for (const auto& uniformFloat : pass.uniformsFloat) {
        int id = uniformFloat.first.idx;
        float value = uniformFloat.second;
        glUniform1f(shader.uniforms[id], value);
    }

    for (const auto& attribute : pass.attributes) {
        int attr = shader.attributes[attribute.first.idx];
        const Mesh& mesh = state->meshes[attribute.second.idx];
        glBindBuffer(GL_ARRAY_BUFFER, mesh.id);
        glEnableVertexAttribArray(attr);
        glVertexAttribPointer(
            attr,
            mesh.dimensions,
            GL_FLOAT,
            GL_FALSE,
            mesh.dimensions * sizeof(float),
            reinterpret_cast<void*>(0)
        );
    }

    glDrawArrays(GL_TRIANGLES, 0, pass.vertexCount);

    for (int attr : shader.attributes) {
        glDisableVertexAttribArray(attr);
    }
}
