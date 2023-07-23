#include "graphics.h"
#include "../glad/glad.h"




static int compileShader(const char* source, int* shader, GLenum type) {
    int s = glCreateShader(type);
    glShaderSource(s, 1, &source, NULL);
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




Graphics::Graphics(const GraphicsInfo& info) {
    width = info.width;
    height = info.height;

    // Shaders
    shaders.resize(info.shaders.size());
    for (size_t i = 0; i < info.shaders.size(); i++) {
        const ShaderInfo& shaderInfo = info.shaders[i];
        int v, f, p;
        if (!compileShader(shaderInfo.vertexShader, &v, GL_VERTEX_SHADER)) {
            printf("Vertex shader compilation failed: %s", shaderInfo.name.c_str());
            return;
        };
        if (!compileShader(shaderInfo.fragmentShader, &f, GL_FRAGMENT_SHADER)) {
            printf("Fragment shader compilation failed: %s", shaderInfo.name.c_str());
            return;
        };
        if (!createProgram(v, f, &p)) {
            printf("Create program failed: %s", shaderInfo.name.c_str());
            return;
        };
        shaders[i].program = p;
    }
    for (size_t i = 0; i < shaders.size(); i++) {
        const ShaderInfo& shaderInfo = info.shaders[i];
        Shader& shader = shaders[i];
        shader.uniforms.resize(shaderInfo.uniforms.size());
        shader.attributes.resize(shaderInfo.attributes.size());
        glUseProgram(shader.program);
        for (int j = 0; j < shader.uniforms.size(); j++) {
            shader.uniforms[j] = glGetUniformLocation(shader.program, shaderInfo.uniforms[j]);
        }
        for (int j = 0; j < shader.attributes.size(); j++) {
            shader.attributes[j] = glGetAttribLocation(shader.program, shaderInfo.attributes[j]);
        }
    }

    // Frames
    frames.resize(info.frames.size());
    for (size_t i = 0; i < frames.size(); i++) {
        const FrameInfo& frameInfo = info.frames[i];
        Frame& frame = frames[i];
        frame.width = frameInfo.width;
        frame.height = frameInfo.height;
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
        }
        else {
            printf("FrameBuffer texture good\n");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glViewport(0, 0, width, height);
    initialized = true;
}

Graphics::~Graphics() {

}

int Graphics::addMesh(float* data, int size) {
    int mesh;
    glGenBuffers(1, (GLuint*)&mesh);
    glBindBuffer(GL_ARRAY_BUFFER, mesh);
    glBufferData(GL_ARRAY_BUFFER, 6 * 5 * sizeof(float), data, GL_STATIC_DRAW);
    return mesh;
}

int Graphics::addTexture(int width, int height, int channels, unsigned char* pixels) {
    int texture;
    glGenTextures(1, (GLuint*)&texture);
    int glTextureType;
    switch (channels) {
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
        width,
        height,
        0,
        glTextureType,
        GL_UNSIGNED_BYTE,
        pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    return texture;
}

int Graphics::getFrameTexture(int frame) {
    return frames[frame].texture;
}

void Graphics::addRenderPass(const RenderPass& pass) {
    renderPasses.push_back(pass);
}

void Graphics::render() {
    glClearColor(.1f, .1f, .1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (const RenderPass& renderPass : renderPasses) {
        Shader& shader = shaders[renderPass.shaderId];
        glUseProgram(shader.program);

        if (renderPass.frame >= 0) {
            const Frame& frame = frames[renderPass.frame];
            glViewport(0, 0, frame.width, frame.height);
            glBindFramebuffer(GL_FRAMEBUFFER, frame.id);
        } else { // Binding to the default FrameBuffer
            glViewport(0, 0, width, height);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glActiveTexture(GL_TEXTURE0 + renderPass.textureUnit);
        glBindTexture(GL_TEXTURE_2D, renderPass.textureId);

        for (const std::pair<int, int>& uniformInt : renderPass.uniformsInt) {
            int id = uniformInt.first;
            int value = uniformInt.second;
            glUniform1i(shader.uniforms[id], value);
        }

        for (const std::pair<int, float>& uniformFloat : renderPass.uniformsFloat) {
            int id = uniformFloat.first;
            float value = uniformFloat.second;
            glUniform1f(shader.uniforms[id], value);
        }

        for (const AttributeInfo& attrInfo : renderPass.attributes) {
            int attr = shader.attributes[attrInfo.attr];
            GLboolean normalized = attrInfo.normalized ? GL_TRUE : GL_FALSE;
            glBindBuffer(GL_ARRAY_BUFFER, attrInfo.bufferId);
            glEnableVertexAttribArray(attr);
            glVertexAttribPointer(attr, attrInfo.count, GL_FLOAT, normalized, attrInfo.stride, reinterpret_cast<void*>(attrInfo.offset));
        }

        glDrawArrays(GL_TRIANGLES, 0, renderPass.vertexCount);

        for (int attr : shader.attributes) {
            glDisableVertexAttribArray(attr);
        }
    }
    renderPasses.clear();
}
