#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/load_obj.hpp>

class Application
{
public:
    Application(int argc, char** argv);

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

    std::vector<GLuint> m_textures;

    GLuint m_VBO = 0;
    GLuint m_IBO = 0;
    GLuint m_VAO = 0;
    GLuint m_FBO = 0;

    GLuint m_VAOTriangle;
    GLuint m_VBOTriangle;

    GLint uMVPMatrixLoc = -1;
    GLint MVMatrixLoc = -1;
    GLint NormalMatrixLoc = -1;
    GLint uKdLoc = -1;
    GLint uKsLoc = -1;
    GLint uKaLoc = -1;
    GLint uShininessLoc = -1;
    GLint uLightDir_vsLoc = -1;
    GLint uLightIntensityLoc = -1;
    GLint uTextureLoc = -1;
    GLint m_uSpecularLoc = -1;
    GLint m_uAmbiantLoc = -1;
    GLint m_uDiffuseLoc = -1;
    GLint m_uShininessLoc = -1;
    GLint m_uLightDir_vsGLoc = -1;
    GLint m_uLightIntensityGLoc = -1;
    GLint m_uGPositionGLoc = -1;
    GLint m_uGNormalGLoc = -1;
    GLint m_uGAmbiantGLoc = -1;
    GLint m_uGDiffuseGLoc = -1;
    GLint m_uGlossyShininessGLoc = -1;
    GLint m_uDirLightViewProjMatrixLoc = -1;
    GLint m_uDirLightShadowMapLoc = -1;
    GLint m_uDirLightShadowMapBiasLoc = -1;
    GLint m_uGDepthSamplerLocation = -1;
    GLint m_uDirLightViewProjMatrixLocation = -1;

    GLuint m_directionalSMTexture;
    GLuint m_directionalSMFBO;
    GLuint m_directionalSMSampler;
    int32_t m_nDirectionalSMResolution = 2048;
    glm::vec3 m_SceneSize;
    float m_SceneSizeLength;
    glm::vec3 m_SceneCenter;

        // Light
    float m_DirLightPhiAngleDegrees = 100.f;
    float m_DirLightThetaAngleDegrees = 30.f;
    glm::vec3 m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
    glm::vec3 m_DirLightColor = glm::vec3(1, 1, 1);
    float m_DirLightIntensity = 2.f;
    float m_DirLightSMBias = 0.01f;
    bool m_directional_light_change;

        // Scene data in GPU:
    GLuint m_SceneVBO = 0;
    GLuint m_SceneIBO = 0;
    GLuint m_SceneVAO = 0;

    // Required data about the scene in CPU in order to send draw calls
    struct ShapeInfo
    {
        uint32_t indexCount; // Number of indices
        uint32_t indexOffset; // Offset in GPU index buffer
        int materialID = -1;
    };
    std::vector<ShapeInfo> m_shapes; // For each shape of the scene, its number of indices

    bool directionalSMDirty = true;

    glmlv::GLProgram m_directionalSMProgram;
    GLint m_uDirLightViewProjMatrix;

    GLuint m_cubeIndexBuffer;
    GLuint m_sphereIndexBuffer;
    glmlv::GLProgram m_programGBuffer;
    glmlv::GLProgram m_programShading;
    glmlv::GLProgram m_displayDepthProgram;
    glmlv::ViewController m_viewController = glmlv::ViewController(m_GLFWHandle.window());

    glmlv::ObjData m_crytekSponzaObj;

    GLuint m_GBufferTextures[GBufferTextureCount];

    static glm::vec3 computeDirectionVectorUp(float phiRadians, float thetaRadians)
    {
        const auto cosPhi = glm::cos(phiRadians);
        const auto sinPhi = glm::sin(phiRadians);
        const auto cosTheta = glm::cos(thetaRadians);
        return -glm::normalize(glm::vec3(sinPhi * cosTheta, -glm::sin(thetaRadians), cosPhi * cosTheta));
    }

    static glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians)
    {
        const auto cosPhi = glm::cos(phiRadians);
        const auto sinPhi = glm::sin(phiRadians);
        const auto sinTheta = glm::sin(thetaRadians);
        return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
    }
};
