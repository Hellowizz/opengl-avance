#include "Application.hpp"

#include <iostream>
#include <cstdlib>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/Image2DRGBA.hpp>
#include <glmlv/GlobalWavPlayer.hpp>

#include <glm/gtc/type_ptr.hpp>

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 }; 
    int radioChoice = 0;
    
    glm::vec3 lightDir(1, 1, 1);
    lightDir = glm::normalize(lightDir);

    glm::vec3 lightColor(3, 3, 3);

    float alphaLight;
    float thetaLight;

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();
        
        // vitesse de la camera
        float m_sceneSize = 30.0f;
        m_viewController.setSpeed(m_sceneSize * 0.1f);
        glm::mat4 ProjMatrix = glm::perspective(70.f, m_nWindowWidth/float(m_nWindowHeight), 0.001f * m_sceneSize, m_sceneSize);
        //glm::mat4 ViewMatrix = m_viewController.getViewMatrix();
        
        // update solar system
        // m_cSolarSystem.update(time);
        m_cSolarSystem.update(seconds);
        for(int i = 0; i < m_planetInstances.size(); ++i) {
            m_planetInstances[i].setModelMatrix(m_cSolarSystem.planets[i]->getWorldMatrix());
        }

        if(m_camera.isTimeToChange(seconds))
        {
                m_camera.stareNewRandomPlanet(&m_cSolarSystem);
                m_camera.toggleState(!m_camera.planetFocus);
        }
        m_camera.updatePosition(seconds);
        glm::mat4 ViewMatrix = m_camera.viewMatrix;

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO); // Set FBO as draw framebuffer

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_programGBuffer.use();
        glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);

            // MODELS DRAWING CODE
        for(auto & planetInstance : m_planetInstances) {
            planetInstance.draw(ViewMatrix, ProjMatrix);
        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Restore screen as draw framebuffer

            // SHADING PASS
        m_programShading.use();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightDir.x = cos(glm::radians(alphaLight)) * sin(glm::radians(thetaLight));
        lightDir.y = sin(glm::radians(alphaLight)) * sin(glm::radians(thetaLight));
        lightDir.z = cos(glm::radians(thetaLight));
        glUniform3fv(m_uLightDir_vsGLoc, 1, glm::value_ptr(ViewMatrix * glm::vec4(lightDir, 0)));
        glUniform3fv(m_uLightIntensityGLoc, 1, glm::value_ptr(lightColor));

        glUniform1i(m_uGPositionGLoc, 0);
        glUniform1i(m_uGNormalGLoc, 1);
        glUniform1i(m_uGAmbiantGLoc, 2);
        glUniform1i(m_uGDiffuseGLoc, 3);
        glUniform1i(m_uGlossyShininessGLoc, 4);

        for (int i=0; i<5; i++){
            glActiveTexture( GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
        }

        glBindVertexArray(m_VAOTriangle);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        GBufferTextureType GTextureType;
       
        // TODO remove this
        static float time = 0;

        // GUI code:
        ImGui_ImplGlfwGL3_NewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
            if (ImGui::ColorEdit3("clearColor", clearColor)) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }

            if (ImGui::InputFloat3("lightDir", glm::value_ptr(lightDir)))
            {
                lightDir = glm::normalize(lightDir);
            }

            ImGui::InputFloat3("lightColor", glm::value_ptr(lightColor));

            ImGui::InputFloat("AlphaLight", &alphaLight);
            ImGui::InputFloat("ThetaLight", &thetaLight);

            ImGui::RadioButton("Position", &radioChoice, 0);
            ImGui::RadioButton("Normal", &radioChoice, 1);
            ImGui::RadioButton("Ambient", &radioChoice, 2);
            ImGui::RadioButton("Diffuse", &radioChoice, 3);

            switch(radioChoice){
                case 0 :
                    GTextureType = GPosition;
                    break;
                case 1 :
                    GTextureType = GNormal;
                    break;
                case 2 :
                    GTextureType = GAmbient;
                    break;
                case 3 :
                    GTextureType = GDiffuse;
                    break;
            }
            ImGui::SliderFloat("time", &time, 0, 10);

            ImGui::SliderFloat3("distToPlanet", glm::value_ptr(m_camera.distToPlanet), -1, 1);

            ImGui::End();
        }
#if 0
         // Blit on screen
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + GTextureType);

        glBlitFramebuffer(0, 0, m_nWindowWidth, m_nWindowHeight, 0, 0,  m_nWindowWidth, m_nWindowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
#endif
        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, viewportSize.x, viewportSize.y);
        ImGui::Render();

        /* Poll for and process events */
        glfwPollEvents();

        /* Swap front and back buffers*/
        m_GLFWHandle.swapBuffers();

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            m_viewController.update(float(ellapsedTime));
        }
    }

    return 0;
}

struct Vertex
{
    glm::vec2 position;
    glm::vec3 color;

    Vertex(glm::vec2 position, glm::vec3 color):
        position(position), color(color)
    {}
};

GLint ModelInstance::uSpecularLoc = -1;
GLint ModelInstance::uAmbiantLoc = -1;
GLint ModelInstance::uDiffuseLoc = -1;
GLint ModelInstance::uShininessLoc = -1;
GLint ModelInstance::uShininessLocSampler = -1;
GLint ModelInstance::uKdLoc = -1;
GLint ModelInstance::uKsLoc = -1;
GLint ModelInstance::uKaLoc = -1;
GLint ModelInstance::uMVPMatrixLoc = -1;
GLint ModelInstance::uMVMatrixLoc = -1;
GLint ModelInstance::uNormalMatrixLoc = -1;

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_AssetsRootPath { m_AppPath.parent_path() / "assets" }

{
    glEnable(GL_DEPTH_TEST);

    // DECLARATION OF ALL THE .OBJ
    std::vector< glmlv::fs::path > modelPaths = {
        m_AssetsRootPath / "projet" / "soleil" / "planeterouge.obj",
        m_AssetsRootPath / "projet" / "planet_rouge" / "planeterouge.obj",
        m_AssetsRootPath / "projet" / "planet_2" / "planeterouge.obj",
        m_AssetsRootPath / "projet" / "planet_3" / "planeterouge.obj",
        m_AssetsRootPath / "projet" / "planet_4" / "planeterouge.obj",
        m_AssetsRootPath / "projet" / "planet_5" / "planeterouge.obj",
        m_AssetsRootPath / "projet" / "asteroide" / "planeterouge.obj"
    };

    for(auto & path : modelPaths) {
        ModelObj * modelObj = new ModelObj();
        modelObj->load(path);
        m_planetModels.push_back(modelObj);
    }

    m_cSolarSystem.generate();
    for(int i = 0; i < m_cSolarSystem.planets.size(); ++i) {
        ModelInstance modelInstance;
        int indexObj;
        if(i==0)
            indexObj = 0;
        else
            indexObj = m_cSolarSystem.planets[i]->asteroide ?
                    m_planetModels.size() - 1: 1 + rand() % (m_planetModels.size() - 2);
        modelInstance.setModel(m_planetModels[indexObj]);
        m_planetInstances.push_back(modelInstance);
    }

    // MUSIQUE

    glmlv::GlobalWavPlayer::playWav(m_AssetsRootPath / "projet" / "music" / "musique.wav");

    // CAMERA
    m_camera.setCamera(glm::vec3(0.009,0.1,-0.06), glm::vec3(0.8, 2, 1.3), m_cSolarSystem.planets[0]);

    // Here we load and compile shaders from the library
    m_programGBuffer = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "geometryPass.vs.glsl", m_ShadersRootPath / m_AppName / "geometryPass.fs.glsl" });
    m_programShading = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl", m_ShadersRootPath / m_AppName / "shadingPass.fs.glsl" });
    m_directionalSMProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "directionalSM.vs.glsl", m_ShadersRootPath / m_AppName / "directionalSM.fs.glsl" });

    ModelInstance::uMVPMatrixLoc = glGetUniformLocation(m_programGBuffer.glId() , "uModelViewProjMatrix");
    ModelInstance::uMVMatrixLoc = glGetUniformLocation(m_programGBuffer.glId() , "uModelViewMatrix");
    ModelInstance::uNormalMatrixLoc = glGetUniformLocation(m_programGBuffer.glId() , "uNormalMatrix");

    ModelInstance::uKdLoc = glGetUniformLocation(m_programGBuffer.glId(), "uKd");
    ModelInstance::uKsLoc = glGetUniformLocation(m_programGBuffer.glId(), "uKs");
    ModelInstance::uKaLoc = glGetUniformLocation(m_programGBuffer.glId(), "uKa");
    ModelInstance::uShininessLoc = glGetUniformLocation(m_programGBuffer.glId(), "uShininess");
    uLightDir_vsLoc = glGetUniformLocation(m_programGBuffer.glId(), "uLightDir_vs");
    uLightIntensityLoc = glGetUniformLocation(m_programGBuffer.glId(), "uLightIntensity");

    ModelInstance::uDiffuseLoc = glGetUniformLocation(m_programGBuffer.glId(), "uKdSampler");
    ModelInstance::uAmbiantLoc = glGetUniformLocation(m_programGBuffer.glId(), "uKaSampler");
    ModelInstance::uSpecularLoc = glGetUniformLocation(m_programGBuffer.glId(), "uKsSampler");
    ModelInstance::uShininessLocSampler = glGetUniformLocation(m_programGBuffer.glId(), "uShininessSampler");

    m_uLightDir_vsGLoc = glGetUniformLocation(m_programShading.glId(), "uLightDir_vs");
    m_uLightIntensityGLoc = glGetUniformLocation(m_programShading.glId(), "uLightIntensity");

    m_uGPositionGLoc = glGetUniformLocation(m_programShading.glId(), "uGPosition");
    m_uGNormalGLoc = glGetUniformLocation(m_programShading.glId(), "uGNormal");
    m_uGAmbiantGLoc = glGetUniformLocation(m_programShading.glId(), "uGAmbiant");
    m_uGDiffuseGLoc = glGetUniformLocation(m_programShading.glId(), "uGDiffuse");

    m_uDirLightViewProjMatrixLoc = glGetUniformLocation(m_directionalSMProgram.glId() , "uDirLightViewProjMatrix");


    glGenTextures(GBufferTextureCount, m_GBufferTextures);
    for(int i=0; i < GBufferTextureCount; i++){
        glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
        //glTexImage2D(GL_TEXTURE_2D, 0, m_GBufferTextureFormat[i], m_nWindowWidth, m_nWindowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexStorage2D(GL_TEXTURE_2D, 1, m_GBufferTextureFormat[i], m_nWindowWidth, m_nWindowHeight);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

    for(int i=0; i < GBufferTextureCount; i++) {
        if(m_GBufferTextureFormat[i] == GL_DEPTH_COMPONENT32F) {
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBufferTextures[i], 0);
        } else {
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_GBufferTextures[i], 0);
        }
    }

        // END TEXTURES //

     // we will write into 5 textures from the fragment shader
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
    glDrawBuffers(5, drawBuffers);

    auto val = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if(val != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERREUR ?? : " << val << std::endl;
        throw std::runtime_error("");
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);


        // TRIANGLE //

    glGenBuffers(1, &m_VBOTriangle);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBOTriangle);

    GLfloat data[] = { -1, -1, 3, -1, -1, 3 };
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(data), data, 0);

    glGenVertexArrays(1, &m_VAOTriangle);
    glBindVertexArray(m_VAOTriangle);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

        // SM TEXTURE //

    glGenTextures(1, &m_directionalSMTexture);
    glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, 512, 512);

    glGenFramebuffers(1, &m_FBOSM);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBOSM);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_directionalSMTexture, 0);
    GLenum drawBufferDepth[] = { GL_DEPTH_ATTACHMENT };
    
    glDrawBuffers(1, drawBufferDepth);

    glGenSamplers(1, &m_directionalSMSampler);
    glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file
}

Application::~Application() {
    // free all ModelObj
    for(auto & model: m_planetModels) {
        delete(model);
    }
}
