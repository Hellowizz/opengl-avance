#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/load_obj.hpp>
#include "glm/ext.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "SolarSystem.hpp"

struct ModelObj
{
    void load(const glmlv::fs::path objPath)
    {
        glmlv::loadObj(objPath, data);

            // VBO

        glGenBuffers(1, &VBO);  // gen vbo
        glBindBuffer(GL_ARRAY_BUFFER, VBO);  // biding vbo
        glBufferStorage(GL_ARRAY_BUFFER, data.vertexBuffer.size() * sizeof(data.vertexBuffer[0]), data.vertexBuffer.data(), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);  // debind vbo

            // IBO

        glGenBuffers(1, &IBO);  // gen ibo
        glBindBuffer(GL_ARRAY_BUFFER, IBO);  // biding ibo
        //glBufferStorage(GL_ARRAY_BUFFER, cube.indexBuffer.size() * sizeof(cube.indexBuffer.data()[0]), cube.indexBuffer.data(), 0);
        glBufferStorage(GL_ARRAY_BUFFER, data.indexBuffer.size() * sizeof(data.indexBuffer.data()[0]), data.indexBuffer.data(), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);  // debind ibo

            // VAO

        const GLint positionAttrLocation = 0;
        const GLint normalAttrLocation = 1;
        const GLint texCoordsAttrLocation = 2;

        glGenVertexArrays(1, &VAO);  // gen vao

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glEnableVertexAttribArray(positionAttrLocation);
        glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, position));

        glEnableVertexAttribArray(normalAttrLocation);
        glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, normal));

        glEnableVertexAttribArray(texCoordsAttrLocation);
        glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, texCoords));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

            // TEXTURES

        textures.resize(data.textures.size());
        glGenTextures(data.textures.size(), textures.data());

        for(int i=0; i < data.textures.size(); i++){
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, data.textures[i].width(), data.textures[i].height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, data.textures[i].data());

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glmlv::ObjData data;

    std::vector<GLuint> textures;
    GLuint VBO;
    GLuint IBO;
    GLuint VAO;
};

struct ModelInstance
{
    // share uniforms in all models.
    // Models can however have different
    // material parameters.
    static GLint uSpecularLoc;
    static GLint uAmbiantLoc;
    static GLint uDiffuseLoc;
    static GLint uShininessLoc;
    static GLint uShininessLocSampler;
    static GLint uKdLoc;
    static GLint uKsLoc;
    static GLint uKaLoc;
    static GLint uMVPMatrixLoc;
    static GLint uMVMatrixLoc;
    static GLint uNormalMatrixLoc;

    ModelObj * model;
    glm::mat4 modelMatrix;

    void setModel(ModelObj * _model)
    {
        model = _model;
    }

    void setModelMatrix(glm::mat4 _modelMatrix)
    {
        modelMatrix = _modelMatrix;
    }

    void draw(const glm::mat4 & ViewMatrix, const glm::mat4 & ProjMatrix)
    {
        // build matrices here
        glm::mat4 MVMatrix(ViewMatrix * modelMatrix);
        glm::mat4 MVPMatrix(ProjMatrix * MVMatrix);
        glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));

        // set the view matrix
        glUniformMatrix4fv(uMVPMatrixLoc, 1, GL_FALSE, glm::value_ptr(MVPMatrix));
        glUniformMatrix4fv(uMVMatrixLoc, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(uNormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

        // Put here rendering code
        glBindVertexArray(model->VAO);

        // Drawing

        auto indexOffset = 0;
        for (int i=0; i<model->data.shapeCount; i++) {
            auto indexCount = model->data.indexCountPerShape[i];

            auto idMaterial = model->data.materialIDPerShape[i];
            const auto &material = model->data.materials[idMaterial];

            glActiveTexture(GL_TEXTURE0);
            if(material.KaTextureId != -1)
                glBindTexture(GL_TEXTURE_2D, model->textures[material.KaTextureId]);
            else
                glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE1);
            if(material.KsTextureId != -1)
                glBindTexture(GL_TEXTURE_2D, model->textures[material.KsTextureId]);
            else
                glBindTexture(GL_TEXTURE_2D, 0);

            glActiveTexture(GL_TEXTURE2);
            if(material.KdTextureId != -1)
                glBindTexture(GL_TEXTURE_2D, model->textures[material.KdTextureId]);
            else
                glBindTexture(GL_TEXTURE_2D, 0);

                // correspond aux unité de textures (GL_TEXTURE0)
            glUniform1i(uSpecularLoc, 1);
            glUniform1i(uAmbiantLoc, 0);
            glUniform1i(uDiffuseLoc, 2);

            glUniform3fv(uKdLoc, 1, glm::value_ptr(material.Kd));
            glUniform3fv(uKaLoc, 1, glm::value_ptr(material.Ka));
            glUniform3fv(uKsLoc, 1, glm::value_ptr(material.Ks));
            glUniform1f(uShininessLoc, material.shininess);

            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (const GLvoid*) (indexOffset * sizeof(GLuint)));
            indexOffset += indexCount;
        }

        glBindVertexArray(0);
    }
};

class Application
{
public:
    Application(int argc, char** argv);
    ~Application();

    int run();

    enum GBufferTextureType
    {
        GPosition = 0,
        GNormal,
        GAmbient,
        GDiffuse,
        GGlossyShininess,
        GDepth, // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
        GBufferTextureCount
    };

    const GLenum m_GBufferTextureFormat[6] = { GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F };

private:
    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_AssetsRootPath;

    // SCENE CONTENT

    SolarSystem m_cSolarSystem;
    std::vector<ModelObj*> m_planetModels;
    std::vector<ModelInstance> m_planetInstances;

    // relative to rendering

    std::vector<GLuint> m_textures;

    GLuint m_FBO = 0;

    GLuint m_VAOTriangle;
    GLuint m_VBOTriangle;

    GLuint m_FBOSM;

    GLint uLightDir_vsLoc = -1;
    GLint uLightIntensityLoc = -1;
    GLint uTextureLoc = -1;
    GLint m_uLightDir_vsGLoc = -1;
    GLint m_uLightIntensityGLoc = -1;
    GLint m_uGPositionGLoc = -1;
    GLint m_uGNormalGLoc = -1;
    GLint m_uGAmbiantGLoc = -1;
    GLint m_uGDiffuseGLoc = -1;
    GLint m_uGlossyShininessGLoc = -1;

    GLuint m_directionalSMTexture;
    GLuint m_directionalSMFBO;
    GLuint m_directionalSMSampler;
    int32_t m_nDirectionalSMResolution = 512;

    glmlv::GLProgram m_directionalSMProgram;
    GLint m_uDirLightViewProjMatrix;
    GLint m_uDirLightViewProjMatrixLoc = -1;

    GLuint m_cubeIndexBuffer;
    GLuint m_sphereIndexBuffer;
    glmlv::GLProgram m_programGBuffer;
    glmlv::GLProgram m_programShading;
    glmlv::ViewController m_viewController = glmlv::ViewController(m_GLFWHandle.window());

    GLuint m_GBufferTextures[GBufferTextureCount];
};
