#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>

struct Planet
{
    // these parameters don't change
    Planet * parent;
    std::vector<Planet*> children;
    float distToParent; // distance from planet to its sun
    glm::vec3 orbitAxisOrientation; // euler ZYX
    glm::vec3 rotationAxisOrientation; // euler ZYX
    float speedOrbit;
    float speedRotation;
    float radius;
    bool asteroide;
    float offsetOrbitAngle;

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
        // glm::mat4 transform = glm::translate(glm::mat4(1), glm::vec3(distToParent));
        glm::mat4 transform = glm::mat4(1);

        transform = glm::rotate(transform, orbitAxisOrientation.x, glm::vec3(0, 0, 1));
        up = glm::rotate(up, orbitAxisOrientation.x, glm::vec3(0, 0, 1));
        transform = glm::rotate(transform, orbitAxisOrientation.y, glm::vec3(0, 1, 0));
        up = glm::rotate(up, orbitAxisOrientation.y, glm::vec3(0, 1, 0));
        transform = glm::rotate(transform, orbitAxisOrientation.z, glm::vec3(1, 0, 0));
        up = glm::rotate(up, orbitAxisOrientation.z, glm::vec3(1, 0, 0));
        // final transformation matrix for the (1, 0, 0) vector
        transform = glm::rotate(transform, offsetOrbitAngle + orbitAngle, up);
        // final position of the planet
        worldPosition = parent->worldPosition + glm::vec3(transform * glm::vec4(distToParent, 0, 0, 1));
    }

    glm::mat4 getWorldMatrix() {
        // computeorbitAxisOrientations euler ZYX rotation matrix
        glm::vec3 up = glm::vec3(0, 1, 0);
        up = glm::rotate(up, rotationAxisOrientation.x, glm::vec3(0, 0, 1));
        up = glm::rotate(up, rotationAxisOrientation.y, glm::vec3(0, 1, 0));
        up = glm::rotate(up, rotationAxisOrientation.z, glm::vec3(1, 0, 0));
        glm::mat4 rotMatrix = glm::rotate(glm::mat4(1), rotationAngle, up);
        return glm::translate(glm::mat4(1), worldPosition) * rotMatrix * glm::scale(glm::mat4(1), glm::vec3(radius));
    }

    void updatePlanet(float time) {
        orbitAngle = time * speedOrbit;
        rotationAngle = time * speedRotation;
        updatePosition();
    }

    Planet(Planet * _parent, float _distToParent,
           glm::vec3 _orbitAxisOrientation, glm::vec3 _rotationAxisOrientation,
           float _speedOrbit, float _speedRotation, float _radius, bool _asteroide=false)
        : parent(_parent), distToParent(_distToParent), orbitAxisOrientation(_orbitAxisOrientation),
          rotationAxisOrientation(_rotationAxisOrientation), speedOrbit(_speedOrbit), speedRotation(_speedRotation),
          worldPosition(glm::vec3(0)), orbitAngle(0), rotationAngle(0), radius(_radius), asteroide(_asteroide),
          offsetOrbitAngle(0)
    {
    }
};

struct SolarSystem
{
    std::vector<Planet*> planets; // first is the root

    SolarSystem() {}
    ~SolarSystem() {
        // delete planets
        for(auto & planet: planets) {
            delete(planet);
        }
    }

#define MAX_CHILDREN_NUMBER 4
#define MIN_CHILDREN_NUMBER 2
#define NB_ASTEROIDS_IN_RING 200

    // Fills the system with planets in a procedural way
    void generate()
    {
        // create the root of the system
        planets.push_back(new Planet(NULL, 0.0f, glm::vec3(0), glm::vec3(0.1, 0, 0), 0.0f, 0.7f, 0.1));
        recursiveAddChildren(planets[0], 3, 0);
    }

    void update(float time) {
        updateRecursive(time, planets[0]);
    }

private:
    glm::vec3 randomEulerZYX(glm::vec3 amplitude) {
        return glm::vec3(
                    amplitude.x * (-0.5 + (float)rand() / (float)RAND_MAX),
                    amplitude.y * (-0.5 + (float)rand() / (float)RAND_MAX),
                    amplitude.z * (-0.5 + (float)rand() / (float)RAND_MAX)
                    );
    }

    void addAsteroidRing(Planet * planet) {
        auto orbitAxis = randomEulerZYX(glm::vec3(0.3, 0.3, 0.3));
        float ringThickness = planet->radius * 0.2;

        for(int i = 0; i < NB_ASTEROIDS_IN_RING; ++i) {
            Planet * ast = new Planet(planet,
                                      1.5 * planet->radius + (ringThickness * rand() / float(RAND_MAX)),
                                      orbitAxis, glm::vec3(0, 0, 0),
                                      0.1, 0, 0.02 * planet->radius,
                                      true
                                      );
            // set offset orbit angle
            ast->offsetOrbitAngle = 360 * (rand() / float(RAND_MAX));

            planets.push_back(ast);
            planet->children.push_back(ast);
        }
    }

    void recursiveAddChildren(Planet * planet, uint depth, uint depth_inverse)
    {
        // TODO more sophisticated
        if(depth == 0)
            return;
        // random number of children
        int nb_children = MIN_CHILDREN_NUMBER + rand() % (MAX_CHILDREN_NUMBER - MIN_CHILDREN_NUMBER);
        for(int i = 0; i < nb_children; ++i) {
            float d = 1. / (depth_inverse*depth_inverse + 1);
            Planet * child = new Planet(planet,
                                        d,
                                        randomEulerZYX(glm::vec3(0.3, 0.3, 0.3)),
                                        randomEulerZYX(glm::vec3(1.57, 1.57, 1.57)),
                                        d * (0.1 + rand() / (float)RAND_MAX), 0.1, 0.02 + 0.03 / ((float)depth_inverse + 1)
                                        );
            planets.push_back(child);
            planet->children.push_back(child);
            recursiveAddChildren(child, depth - 1, depth_inverse + 1);
        }
        if(rand() / float(RAND_MAX) > 0.8) // proba faible
        {
            addAsteroidRing(planet);
        }
    }

    void updateRecursive(float time, Planet * planet) {
        planet->updatePlanet(time);
        for(auto & child: planet->children) {
            updateRecursive(time, child);
        }
    }
};
