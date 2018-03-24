#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

struct Planet
{
    // these parameters don't change
    Planet * parent;
    std::vector<Planet> children;
    float distToParent;
    glm::vec3 orbitAxisOrientation; // euler ZYX
    glm::vec3 rotationAxisOrientation; // euler ZYX
    float speedOrbit;
    float speedRotation;

    // these values are changing
    glm::vec3 worldPosition;
    float orbitAngle;
    float rotationAngle;

    void updatePosition()
    {
        if(parent == NULL) // if the planet is the root of the system
            return;
        // computes euler ZYX rotation matrix
        glm::vec3 up = glm::vec3(0, 1, 0);
        glm::mat4 transform = glm::translate(glm::mat4(1), glm::vec3(distToParent));
        transform = glm::rotate(transform, orbitAxisOrientation.x, glm::vec3(0, 0, 1));
        up = glm::rotate(up, orbitAxisOrientation.x, glm::vec3(0, 0, 1));
        transform = glm::rotate(transform, orbitAxisOrientation.y, glm::vec3(0, 1, 0));
        up = glm::rotate(up, orbitAxisOrientation.y, glm::vec3(0, 1, 0));
        transform = glm::rotate(transform, orbitAxisOrientation.z, glm::vec3(1, 0, 0));
        up = glm::rotate(up, orbitAxisOrientation.z, glm::vec3(1, 0, 0));
        // final transformation matrix for the (1, 0, 0) vector
        transform = glm::rotate(transform, orbitAngle, up);
        // final position of the planet
        worldPosition = parent->worldPosition + glm::vec3(transform * glm::vec4(1, 0, 0, 0));
    }

    glm::mat4 getWorldMatrix() {
        // computeorbitAxisOrientations euler ZYX rotation matrix
        glm::vec3 up = glm::vec3(0, 1, 0);
        up = glm::rotate(up, rotationAxisOrientation.x, glm::vec3(0, 0, 1));
        up = glm::rotate(up, rotationAxisOrientation.y, glm::vec3(0, 1, 0));
        up = glm::rotate(up, rotationAxisOrientation.z, glm::vec3(1, 0, 0));
        glm::mat4 rotMatrix = glm::rotate(glm::mat4(1), rotationAngle, up);
        return glm::translate(glm::mat4(1), worldPosition) * rotMatrix;
    }

    void updatePlanet(const float time) {
        orbitAngle = time * speedOrbit;
        rotationAngle = time * speedRotation;
        updatePosition();
    }

    Planet(Planet * _parent, float _distToParent,
           glm::vec3 _orbitAxisOrientation, glm::vec3 _rotationAxisOrientation,
           float _speedOrbit, float _speedRotation)
        : parent(_parent), distToParent(_distToParent), orbitAxisOrientation(_orbitAxisOrientation),
          rotationAxisOrientation(_rotationAxisOrientation), speedOrbit(_speedOrbit), speedRotation(_speedRotation),
          worldPosition(glm::vec3(0)), orbitAngle(0), rotationAngle(0)
    {
    }
};

struct SolarSystem
{
    Planet root;

    SolarSystem()
       : root(NULL, 0.0f, glm::vec3(0), glm::vec3(0.1, 0, 0), 0.0f, 2.0f)
    {
    }
};
