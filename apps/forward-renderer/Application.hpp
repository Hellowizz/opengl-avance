#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>

class Application
{
public:
    Application(int argc, char** argv);

    int run();
private:
    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;

    GLuint m_textures[2];

    GLuint m_VBO = 0;
    GLuint m_IBO = 0;
    GLuint m_VAO = 0;

    GLint uMVPMatrixLoc = -1;
    GLint MVMatrixLoc = -1;
    GLint NormalMatrixLoc = -1;
    GLint uKdLoc = -1;
    GLint uKsLoc = -1;
    GLint uShininessLoc = -1;
    GLint uLightDir_vsLoc = -1;
    GLint uLightIntensityLoc = -1;
    GLint uTextureLoc = -1;

    GLuint m_cubeIndexBuffer;
    GLuint m_sphereIndexBuffer;
    glmlv::GLProgram m_program;
    glmlv::ViewController m_viewController = glmlv::ViewController(m_GLFWHandle.window());
};