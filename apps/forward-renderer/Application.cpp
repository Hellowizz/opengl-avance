#include "Application.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/simple_geometry.hpp>

#include <glm/gtc/type_ptr.hpp>

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 };
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), m_nWindowWidth/float(m_nWindowHeight), 0.1f, 100.f);
        glm::mat4 ModelMatrix(1);
        glm::vec3 posCamera = glm::vec3(5,5,5);
        glm::vec3 cible = glm::vec3(0, 0, 0);
        glm::mat4 ViewMatrix(glm::lookAt(posCamera, cible, glm::vec3(0,1,0)));
        glm::mat4 MVMatrix(ViewMatrix * ModelMatrix);
        glm::mat4 uMVPMatrix(ProjMatrix * MVMatrix);
        glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix)); 

        //glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniformMatrix4fv(uMVPMatrixLoc, 1, GL_FALSE, glm::value_ptr(uMVPMatrix));
        glUniformMatrix4fv(MVMatrixLoc, 1, GL_FALSE, glm::value_ptr(MVMatrix));
        glUniformMatrix4fv(NormalMatrixLoc, 1, GL_FALSE, glm::value_ptr(NormalMatrix));

        // Put here rendering code
        glBindVertexArray(m_VAO);

        // Drawing code
        glDrawElements(GL_TRIANGLES, m_cubeIndexBuffer, GL_UNSIGNED_INT, nullptr);

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
            ImGui::End();
        }

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
            //viewController.update(float(ellapsedTime))
        }
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" }

{
    glEnable(GL_DEPTH_TEST);

    glmlv::SimpleGeometry cube = glmlv::makeCube();
    m_cubeIndexBuffer = cube.indexBuffer.size();

    glGenBuffers(1, &m_VBO);  // gen vbo
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);  // biding vbo
    glBufferStorage(GL_ARRAY_BUFFER, cube.vertexBuffer.size() * sizeof(cube.vertexBuffer[0]), cube.vertexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // debind vbo

    glGenBuffers(1, &m_IBO);  // gen ibo
    glBindBuffer(GL_ARRAY_BUFFER, m_IBO);  // biding ibo
    glBufferStorage(GL_ARRAY_BUFFER, cube.indexBuffer.size() * sizeof(cube.indexBuffer.data()[0]), cube.indexBuffer.data(), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // debind ibo

    // Here we load and compile shaders from the library
    m_program = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "forward.vs.glsl", m_ShadersRootPath / m_AppName / "forward.fs.glsl" });

    uMVPMatrixLoc = glGetUniformLocation(m_program.glId() , "uModelViewProjMatrix");
    MVMatrixLoc = glGetUniformLocation(m_program.glId() , "uModelViewMatrix");
    NormalMatrixLoc = glGetUniformLocation(m_program.glId() , "uNormalMatrix");

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

    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file
    m_program.use();
}