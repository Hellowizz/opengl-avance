    #include "Application.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glmlv/Image2DRGBA.hpp>

#include <glm/gtc/type_ptr.hpp>

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 }; 
    int radioChoice = 0;
    
    glm::vec3 lightDir(1, 1, 1);
    lightDir = glm::normalize(lightDir);

    glm::vec3 lightColor(1, 1, 1);

    float alphaLight;
    float thetaLight;

    // Realocate texture
//    glGenTextures(1, &m_directionalSMTexture);
//    glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);
//    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, m_nDirectionalSMResolution, m_nDirectionalSMResolution);
//    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach new texture to FBO
//    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_directionalSMFBO);
//    glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);
//    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_directionalSMTexture, 0);
//    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();
        
                          // INITIALISATION //
        m_viewController.setSpeed(m_SceneSizeLength * 0.1f);
        glm::mat4 ProjMatrix = glm::perspective(70.f, m_nWindowWidth/float(m_nWindowHeight), 0.01f * m_SceneSizeLength, m_SceneSizeLength); // near = 1% de la taille de la scene, far = 100%
        glm::mat4 ModelMatrix(1);
        glm::vec3 posCamera = glm::vec3(5,5,5);
        glm::vec3 cible = glm::vec3(0, 0, 0);
        glm::mat4 ViewMatrix = m_viewController.getViewMatrix();
        glm::mat4 MVMatrix(ViewMatrix * ModelMatrix);
        glm::mat4 uMVPMatrix(ProjMatrix * MVMatrix);
        glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));

        const auto sceneCenter = 0.5f * (m_crytekSponzaObj.bboxMin + m_crytekSponzaObj.bboxMax);
        const float sceneRadius = m_SceneSizeLength * 0.5f;

        const auto dirLightUpVector = computeDirectionVectorUp(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
        const auto dirLightViewMatrix = glm::lookAt(sceneCenter + lightDir * sceneRadius, sceneCenter, dirLightUpVector); // Will not work if m_DirLightDirection is colinear to lightUpVector
        const auto dirLightProjMatrix = glm::ortho(-sceneRadius, sceneRadius, -sceneRadius, sceneRadius, 0.01f * sceneRadius, 2.f * sceneRadius);


        if (directionalSMDirty)
        {
            // Calcul de la shadow map (**)
            m_directionalSMProgram.use();

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_directionalSMFBO);
            glViewport(0, 0, m_nDirectionalSMResolution, m_nDirectionalSMResolution);
            glClearColor(0,0,0,0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUniformMatrix4fv(m_uDirLightViewProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(dirLightProjMatrix*dirLightViewMatrix));

            glBindVertexArray(m_VAO);

            auto indexOffset = 0;
            for (const auto indexCount: m_crytekSponzaObj.indexCountPerShape)
            {
                glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT,
                               (const GLvoid*) (indexOffset * sizeof(GLuint)));
                indexOffset += indexCount;
            }

            glBindVertexArray(0);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            directionalSMDirty = false; // Pas de calcul au prochain tour
        }

                          // FIN INITIALISATION //

        m_programGBuffer.use();
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO); // Set FBO as draw framebuffer

        glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glUniformMatrix4fv(uMVPMatrixLoc, 1, GL_FALSE, glm::value_ptr(uMVPMatrix));
        glUniformMatrix4fv(MVMatrixLoc, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

        // Put here rendering code
        glBindVertexArray(m_VAO);

        // Drawing code

        auto indexOffset = 0;
        for (int i=0; i<m_crytekSponzaObj.shapeCount; i++) {
            auto indexCount = m_crytekSponzaObj.indexCountPerShape[i];

            auto idMaterial = m_crytekSponzaObj.materialIDPerShape[i];
            const auto &material = m_crytekSponzaObj.materials[idMaterial]; 

            glActiveTexture(GL_TEXTURE0);
            if(material.KaTextureId != -1)
                glBindTexture(GL_TEXTURE_2D, m_textures[material.KaTextureId]);
            else
                glBindTexture(GL_TEXTURE_2D, 0);

            glActiveTexture(GL_TEXTURE1);
            if(material.KsTextureId != -1)
                glBindTexture(GL_TEXTURE_2D, m_textures[material.KsTextureId]);
            else
                glBindTexture(GL_TEXTURE_2D, 0);

            glActiveTexture(GL_TEXTURE2);
            if(material.KdTextureId != -1)
                glBindTexture(GL_TEXTURE_2D, m_textures[material.KdTextureId]);
            else
                glBindTexture(GL_TEXTURE_2D, 0);

                // correspond aux unité de textures (GL_TEXTURE0)
            glUniform1i(m_uSpecularLoc, 1); 
            glUniform1i(m_uAmbiantLoc, 0);
            glUniform1i(m_uDiffuseLoc, 2);

            glUniform3fv(uKdLoc, 1, glm::value_ptr(material.Kd));
            glUniform3fv(uKaLoc, 1, glm::value_ptr(material.Ka));
            glUniform3fv(uKsLoc, 1, glm::value_ptr(material.Ks));
            glUniform1f(uShininessLoc, material.shininess);

            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (const GLvoid*) (indexOffset * sizeof(GLuint)));
            indexOffset += indexCount;
        }

        glBindVertexArray(0);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Restore screen as draw framebuffer

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // SHADING PASS
        m_programShading.use();
        //m_displayDepthProgram.use();

        const auto rcpViewMatrix = m_viewController.getRcpViewMatrix(); // Inverse de la view matrix de la caméra
        glUniformMatrix4fv(m_uDirLightViewProjMatrixLoc, 1, GL_FALSE, glm::value_ptr(dirLightProjMatrix * dirLightViewMatrix * rcpViewMatrix));
        glUniform1fv(m_uDirLightShadowMapBiasLoc, 1, &m_DirLightSMBias);

        lightDir.x = cos(glm::radians(alphaLight)) * sin(glm::radians(thetaLight));
        lightDir.y = sin(glm::radians(alphaLight)) * sin(glm::radians(thetaLight));
        lightDir.z = cos(glm::radians(thetaLight));
        glUniform3fv(m_uLightDir_vsGLoc, 1, glm::value_ptr(ViewMatrix * glm::vec4(lightDir, 0)));
        glUniform3fv(m_uLightIntensityGLoc, 1, glm::value_ptr(lightColor));

        glProgramUniform1i(m_programShading.glId(), m_uGPositionGLoc, 0);
        glProgramUniform1i(m_programShading.glId(), m_uGNormalGLoc, 1);
        glProgramUniform1i(m_programShading.glId(), m_uGAmbiantGLoc, 2);
        glProgramUniform1i(m_programShading.glId(), m_uGDiffuseGLoc, 3);
        glProgramUniform1i(m_programShading.glId(), m_uGlossyShininessGLoc, 4);
        glProgramUniform1i(m_programShading.glId(), m_uDirLightShadowMapLoc, 5);

        for (int i=0; i<5; i++){
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
        }

        /*glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);

        GBufferTextureType GTextureType;*/

        m_displayDepthProgram.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);

        glUniform1i(m_uGDepthSamplerLocation, 0);

        glBindVertexArray(m_VAOTriangle);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
       
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

            if (ImGui::CollapsingHeader("Directional Light"))
            {
                ImGui::ColorEdit3("DirLightColor", glm::value_ptr(m_DirLightColor));
                ImGui::DragFloat("DirLightIntensity", &m_DirLightIntensity, 0.1f, 0.f, 100.f);
                bool angleChanged = ImGui::DragFloat("Phi Angle", &m_DirLightPhiAngleDegrees, 1.0f, 0.0f, 360.f);
                angleChanged = ImGui::DragFloat("Theta Angle", &m_DirLightThetaAngleDegrees, 1.0f, 0.0f, 180.f) || angleChanged;

                if (angleChanged) {
                    m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
                    directionalSMDirty = true;
                }
                ImGui::InputFloat("DirShadowMap Bias", &m_DirLightSMBias);
            }
            ImGui::End();
        }

        /*glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        glBlitFramebuffer(0, 0, m_nWindowWidth, m_nWindowHeight, 0, 0,  m_nWindowWidth, m_nWindowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);*/

        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, viewportSize.x, viewportSize.y);
        ImGui::Render();

        /* Poll for and process events */
        glfwPollEvents();

        /* Swap front and back buffers*/
        m_GLFWHandle.swapBuffers();

        auto ellapsedTime = glfwGetTime()-seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
            m_directional_light_change = m_viewController.update(float(ellapsedTime));
        }

        // Pseudo code dans le dessin de la GUI:
        if (m_directional_light_change)
        {
            directionalSMDirty = true; // Il faut recalculer la shadow map
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

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_AssetsRootPath { m_AppPath.parent_path() / "assets" }

{
    glEnable(GL_DEPTH_TEST);

    // MODELE OBJ
    glmlv::loadObj(m_AssetsRootPath / "glmlv" / "models" / "crytek-sponza" / "sponza.obj" , m_crytekSponzaObj);
    m_SceneSize =  m_crytekSponzaObj.bboxMax - m_crytekSponzaObj.bboxMin;
    m_SceneSizeLength = glm::length(m_SceneSize);
    m_SceneCenter = 0.5f * (m_crytekSponzaObj.bboxMax + m_crytekSponzaObj.bboxMin);

    glGenBuffers(1, &m_VBO);  // gen vbo
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);  // biding vbo
    glBufferStorage(GL_ARRAY_BUFFER, m_crytekSponzaObj.vertexBuffer.size() * sizeof(m_crytekSponzaObj.vertexBuffer[0]),
                    m_crytekSponzaObj.vertexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // debind vbo

    glGenBuffers(1, &m_IBO);  // gen ibo
    glBindBuffer(GL_ARRAY_BUFFER, m_IBO);  // biding ibo
    glBufferStorage(GL_ARRAY_BUFFER, m_crytekSponzaObj.indexBuffer.size() * sizeof(m_crytekSponzaObj.indexBuffer.data()[0]),
                    m_crytekSponzaObj.indexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // debind ibo

    // Here we load and compile shaders from the library
    m_programGBuffer = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "geometryPass.vs.glsl",
                                               m_ShadersRootPath / m_AppName / "geometryPass.fs.glsl" });
    m_programShading = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl",
                                               m_ShadersRootPath / m_AppName / "shadingPass.fs.glsl" });
    m_directionalSMProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "directionalSM.vs.glsl",
                                                     m_ShadersRootPath / m_AppName / "directionalSM.fs.glsl" });
    m_displayDepthProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl",
                                                    m_ShadersRootPath / m_AppName / "displayDepth.fs.glsl" });

    uMVPMatrixLoc = glGetUniformLocation(m_programGBuffer.glId() , "uModelViewProjMatrix");
    MVMatrixLoc = glGetUniformLocation(m_programGBuffer.glId() , "uModelViewMatrix");
    NormalMatrixLoc = glGetUniformLocation(m_programGBuffer.glId() , "uNormalMatrix");

    uKdLoc = glGetUniformLocation(m_programGBuffer.glId(), "uKd");
    uKsLoc = glGetUniformLocation(m_programGBuffer.glId(), "uKs");
    uKaLoc = glGetUniformLocation(m_programGBuffer.glId(), "uKa");
    uShininessLoc = glGetUniformLocation(m_programGBuffer.glId(), "uShininess");
    uLightDir_vsLoc = glGetUniformLocation(m_programGBuffer.glId(), "uLightDir_vs");
    uLightIntensityLoc = glGetUniformLocation(m_programGBuffer.glId(), "uLightIntensity");

    m_uDiffuseLoc = glGetUniformLocation(m_programGBuffer.glId(), "uKdSampler");
    m_uAmbiantLoc = glGetUniformLocation(m_programGBuffer.glId(), "uKaSampler");
    m_uSpecularLoc = glGetUniformLocation(m_programGBuffer.glId(), "uKsSampler");
    m_uShininessLoc = glGetUniformLocation(m_programGBuffer.glId(), "uShininessSampler");

    m_uLightDir_vsGLoc = glGetUniformLocation(m_programShading.glId(), "uLightDir_vs");
    m_uLightIntensityGLoc = glGetUniformLocation(m_programShading.glId(), "uLightIntensity");

    m_uGPositionGLoc = glGetUniformLocation(m_programShading.glId(), "uGPosition");
    m_uGNormalGLoc = glGetUniformLocation(m_programShading.glId(), "uGNormal");
    m_uGAmbiantGLoc = glGetUniformLocation(m_programShading.glId(), "uGAmbiant");
    m_uGDiffuseGLoc = glGetUniformLocation(m_programShading.glId(), "uGDiffuse");
    m_uGlossyShininessGLoc = glGetUniformLocation(m_programShading.glId(), "uGlossyShininess");

    m_uDirLightViewProjMatrixLoc = glGetUniformLocation(m_programShading.glId() , "uDirLightViewProjMatrix");
    m_uDirLightShadowMapLoc = glGetUniformLocation(m_programShading.glId() , "uDirLightShadowMap");
    m_uDirLightShadowMapBiasLoc = glGetUniformLocation(m_programShading.glId() , "uDirLightShadowMapBias");


    m_uDirLightViewProjMatrixLocation = glGetUniformLocation(m_directionalSMProgram.glId(), "uDirLightViewProjMatrix");
    m_uGDepthSamplerLocation = glGetUniformLocation(m_displayDepthProgram.glId(), "uGDepth");

    const GLint positionAttrLocation = 0;
    const GLint normalAttrLocation = 1;
    const GLint texCoordsAttrLocation = 2;

    glGenVertexArrays(1, &m_VAO);  // gen vao

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    glEnableVertexAttribArray(positionAttrLocation);
    glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, position));

    glEnableVertexAttribArray(normalAttrLocation);
    glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, normal));

    glEnableVertexAttribArray(texCoordsAttrLocation);
    glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*) offsetof(glmlv::Vertex3f3f2f, texCoords));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

        // TEXTURES //
    m_textures.resize(m_crytekSponzaObj.textures.size());
    glGenTextures(m_crytekSponzaObj.textures.size(), m_textures.data());

    for(int i=0; i < m_crytekSponzaObj.textures.size(); i++){
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_crytekSponzaObj.textures[i].width(), m_crytekSponzaObj.textures[i].height(),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, m_crytekSponzaObj.textures[i].data());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(GBufferTextureCount, m_GBufferTextures);
    for(int i=0; i < GBufferTextureCount; i++){
        glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
        //glTexImage2D(GL_TEXTURE_2D, 0, m_GBufferTextureFormat[i], m_nWindowWidth, m_nWindowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexStorage2D(GL_TEXTURE_2D, 1, m_GBufferTextureFormat[i], m_nWindowWidth, m_nWindowHeight);

        /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
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
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, m_nDirectionalSMResolution, m_nDirectionalSMResolution);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &m_directionalSMFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_directionalSMFBO);
    glBindTexture(GL_TEXTURE_2D, m_directionalSMTexture);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_directionalSMTexture, 0);

    const auto fboStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "Error on building directional shadow mapping framebuffer. Error code = " << fboStatus << std::endl;
            throw std::runtime_error("Error on building directional shadow mapping framebuffer.");
        }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glGenSamplers(1, &m_directionalSMSampler);
    glBindSampler(m_directionalSMTexture, m_directionalSMSampler);
    glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(m_directionalSMSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file
 
}
