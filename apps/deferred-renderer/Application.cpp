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
    
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        // vitesse de la camera
        const auto m_sceneSize = glm::length(m_crytekSponzaObj.bboxMax - m_crytekSponzaObj.bboxMin);
        m_viewController.setSpeed(m_sceneSize * 0.1f);
        glm::mat4 ProjMatrix = glm::perspective(70.f, m_nWindowWidth/float(m_nWindowHeight), 0.01f * m_sceneSize, m_sceneSize); // near = 1% de la taille de la scene, far = 100%
        glm::mat4 ModelMatrix(1);
        glm::vec3 posCamera = glm::vec3(5,5,5);
        glm::vec3 cible = glm::vec3(0, 0, 0);
        glm::mat4 ViewMatrix = m_viewController.getViewMatrix();
        glm::mat4 MVMatrix(ViewMatrix * ModelMatrix);
        glm::mat4 uMVPMatrix(ProjMatrix * MVMatrix);
        glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO); // Set FBO as draw framebuffer

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_program.use();

        glUniformMatrix4fv(uMVPMatrixLoc, 1, GL_FALSE, glm::value_ptr(uMVPMatrix));
        glUniformMatrix4fv(MVMatrixLoc, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

        glUniform3fv(uKdLoc, 1, glm::value_ptr(glm::vec3(0.7,0.4,0.4)));
        glUniform3fv(uKsLoc, 1, glm::value_ptr(glm::vec3(0.7,0.4,0.4)));
        glUniform1f(uShininessLoc, 30.f);
        glUniform3fv(uLightDir_vsLoc, 1, glm::value_ptr(ViewMatrix * glm::vec4(1, 1, 1, 0)));
        glUniform3fv(uLightIntensityLoc, 1, glm::value_ptr(glm::vec3(1,1,1)));

        // Put here rendering code
        glBindVertexArray(m_VAO);

        // Drawing code
        //glDrawElements(GL_TRIANGLES, m_cubeIndexBuffer, GL_UNSIGNED_INT, nullptr);
        //glDrawElements(GL_TRIANGLES, m_sphereIndexBuffer, GL_UNSIGNED_INT, nullptr);
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

            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (const GLvoid*) (indexOffset * sizeof(GLuint)));
            indexOffset += indexCount;
        }

        glBindVertexArray(0);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Restore screen as draw framebuffer

        GBufferTextureType GTextuteType;
       
        // GUI code:
        ImGui_ImplGlfwGL3_NewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
            if (ImGui::ColorEdit3("clearColor", clearColor)) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }

            
            ImGui::RadioButton("Position", &radioChoice, 0);
            ImGui::RadioButton("Normal", &radioChoice, 1);
            ImGui::RadioButton("Ambient", &radioChoice, 2);
            ImGui::RadioButton("Diffuse", &radioChoice, 3);

            switch(radioChoice){
                case 0 :
                    GTextuteType = GPosition;
                case 1 :
                    GTextuteType = GNormal;
                case 2 :
                    GTextuteType = GAmbient;
                case 3 :
                    GTextuteType = GDiffuse;
            }
            ImGui::End();
        }

         // Blit on screen
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + GTextuteType);

        glBlitFramebuffer(0, 0, m_nWindowWidth, m_nWindowHeight, 0, 0,  m_nWindowWidth, m_nWindowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);


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

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_AssetsRootPath { m_AppPath.parent_path() / "assets" }

{
    glEnable(GL_DEPTH_TEST);

    /*glmlv::SimpleGeometry cube = glmlv::makeCube();
    glmlv::SimpleGeometry sphere = glmlv::makeSphere(100);*/

    // MODELE OBJ
    glmlv::loadObj(m_AssetsRootPath / "glmlv" / "models" / "crytek-sponza" / "sponza.obj" , m_crytekSponzaObj);

    /*m_cubeIndexBuffer = cube.indexBuffer.size();
    m_sphereIndexBuffer = sphere.indexBuffer.size();*/

    glGenBuffers(1, &m_VBO);  // gen vbo
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);  // biding vbo
    glBufferStorage(GL_ARRAY_BUFFER, m_crytekSponzaObj.vertexBuffer.size() * sizeof(m_crytekSponzaObj.vertexBuffer[0]), m_crytekSponzaObj.vertexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // debind vbo

    glGenBuffers(1, &m_IBO);  // gen ibo
    glBindBuffer(GL_ARRAY_BUFFER, m_IBO);  // biding ibo
    //glBufferStorage(GL_ARRAY_BUFFER, cube.indexBuffer.size() * sizeof(cube.indexBuffer.data()[0]), cube.indexBuffer.data(), 0);
    glBufferStorage(GL_ARRAY_BUFFER, m_crytekSponzaObj.indexBuffer.size() * sizeof(m_crytekSponzaObj.indexBuffer.data()[0]), m_crytekSponzaObj.indexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // debind ibo

    // Here we load and compile shaders from the library
    m_program = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "geometryPass.vs.glsl", m_ShadersRootPath / m_AppName / "geometryPass.fs.glsl" });
    
    uMVPMatrixLoc = glGetUniformLocation(m_program.glId() , "uModelViewProjMatrix");
    MVMatrixLoc = glGetUniformLocation(m_program.glId() , "uModelViewMatrix");
    NormalMatrixLoc = glGetUniformLocation(m_program.glId() , "uNormalMatrix");
#define FOO(u) if(u == -1) std::cerr << #u << " is -1! (is it used?)" << std::endl;
    FOO(uMVPMatrixLoc);
    FOO(MVMatrixLoc);
    FOO(NormalMatrixLoc);
#undef FOO

    uKdLoc = glGetUniformLocation(m_program.glId(), "uKd");
    uKsLoc = glGetUniformLocation(m_program.glId(), "uKs");
    uShininessLoc = glGetUniformLocation(m_program.glId(), "uShininess");
    uLightDir_vsLoc = glGetUniformLocation(m_program.glId(), "uLightDir_vs");
    uLightIntensityLoc = glGetUniformLocation(m_program.glId(), "uLightIntensity");

    m_uDiffuseLoc = glGetUniformLocation(m_program.glId(), "uKdSampler");
    m_uAmbiantLoc = glGetUniformLocation(m_program.glId(), "uKaSampler");
    m_uSpecularLoc = glGetUniformLocation(m_program.glId(), "uKsSampler");
    m_uShininessLoc = glGetUniformLocation(m_program.glId(), "uShininessSampler");

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

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

        // TEXTURES //
    m_textures.resize(m_crytekSponzaObj.textures.size());
    glGenTextures(m_crytekSponzaObj.textures.size(), m_textures.data());

    for(int i=0; i < m_crytekSponzaObj.textures.size(); i++){
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_crytekSponzaObj.textures[i].width(), m_crytekSponzaObj.textures[i].height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_crytekSponzaObj.textures[i].data());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

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

    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
    glDrawBuffers(5, drawBuffers);

    auto val = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if(val != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERREUR ?? : " << val << std::endl;
        throw std::runtime_error("");
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);


        // Texture poil
    /*glmlv::Image2DRGBA poils = glmlv::readImage(m_AppPath.parent_path() / "assets" / m_AppName /"img" / "poilRose.jpg");

    glBindTexture(GL_TEXTURE_2D, m_textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, poils.width(), poils.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, poils.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

        // Texture bleue
    glmlv::Image2DRGBA bleue = glmlv::readImage(m_AppPath.parent_path() / "assets" / m_AppName / "img" / "texturebleue.jpg");

    glBindTexture(GL_TEXTURE_2D, m_textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, bleue.width(), bleue.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bleue.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);*/

    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file
 
}