/**
 * @file ParticleSystem.h
 * @brief Defines the ParticleSystem class for simulating and rendering particle effects in the ECS framework.
 *
 * This header file declares the `ParticleSystem` class, responsible for managing particle effects such as
 * floating or glowing visual elements around entities. It supports particle spawning, updating, and
 * rendering logic, integrated with the ECS framework.
 *
 * Key Features:
 * - **Particle Spawning**: Dynamically spawns new particles at randomized positions around an entity,
 *   with customizable velocity, lifetime, and size.
 * - **Lifecycle Management**: Automatically updates particle positions based on velocity and lifetime,
 *   and removes expired particles from memory.
 * - **Integrated Rendering**: Uses `HUGraphics::GLModel` to render star-shaped particle visuals with proper
 *   transformation matrices.
 * - **Entity-based Behavior**: Operates on entities that possess the `ParticleComponent`.
 * - **Stage-aware Updates**: Only updates particles when the game is in an active "playing" stage.
 * - **Minimal Overhead**: Limits the number of active particles per entity to avoid performance issues.
 *
 * Utility Functions:
 * - `Update`: Core function to update existing particles and spawn new ones.
 * - `SpawnParticle`: Handles randomized generation of particle positions and velocities.
 * - `GetActiveParticles`: Returns active particle-model pairs for possible external use (e.g., debugging).
 *
 * Author: Che Ee (100%)

 */




#pragma once

#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <Coordinator.h>
#include <EntityManager.h>
#include "SystemsManager.h"
#include "GlobalVariables.h"
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
    //this size is useless lol
    float size=20.f;
    bool active;

    Particle(glm::vec3 pos, glm::vec3 vel, float life, float sz)
        : position(pos), velocity(vel), lifetime(life), size(sz), active(true) {}
};

struct ParticleComponent {
    std::vector<Particle> particles;   // Particle data (position, velocity, lifetime)
    std::vector<HUGraphics::GLModel> particleModels;  // GLModels for rendering
    int maxParticles = 20;
    float spawnRate = 0.1f;
    float timeSinceLastSpawn = 0.0f;
};
//HUGraphics::GLModel HUGraphics::star_model(float radius, float inner_radius, int points, glm::vec3 color)

class ParticleSystem : public System {
public:

    const char* getName() const override {
        return "ParticleSystem"; // Return the name of the system
    }

    void Init() override{
        Signature signature;
        signature.set(ECoordinator.GetComponentType<ParticleComponent>());
        ECoordinator.SetSystemSignature<ParticleSystem>(signature);
    }

    void Update(double deltaTime) override {
        if (windowFocused) {
            //std::cout << CoreEngine::InputSystem::Stage<<"\n";
            if (CoreEngine::InputSystem::Stage == Playing ||
                CoreEngine::InputSystem::Stage == Playing1 ||
                CoreEngine::InputSystem::Stage == Playing2 ||
                CoreEngine::InputSystem::Stage == Playing3) {
                for (auto entity : mEntities) {
                    auto& particleComp = ECoordinator.GetComponent<ParticleComponent>(entity);
                    // Remove expired particles
                    for (size_t i = 0; i < particleComp.particles.size(); ) {
                        auto& particle = particleComp.particles[i];

                        if (particle.lifetime <= 0) {
                            // std::cout << "Cleaning up particle" << std::endl;
                            particleComp.particleModels[i].cleanup();  // Correct: cleanup before erase
                            particleComp.particleModels.erase(particleComp.particleModels.begin() + i);
                            particleComp.particles.erase(particleComp.particles.begin() + i);
                            continue; // Don't increment i since erase shifts elements left
                        }
                        i++; // Only increment if no deletion occurs
                    }

                    // Update remaining particles
                    for (size_t i = 0; i < particleComp.particles.size(); i++) {
                        auto& particle = particleComp.particles[i];
                        auto& model = particleComp.particleModels[i];

                        if (particle.active) {
                            particle.position += particle.velocity * static_cast<float>(deltaTime);
                            particle.lifetime -= float(deltaTime);

                            // Transform for rendering
                            glm::mat4 transform = glm::mat4(1.0f);
                            transform = glm::translate(transform, particle.position);
                            transform = glm::scale(transform, glm::vec3(particle.size));
                            //if(CoreEngine::InputSystem::Stage==Playing)
                            model.draw(transform, glm::ortho(0.0f, 1600.0f, 900.0f, 0.0f, -1.0f, 1.0f), cameraObj.GetViewMatrix());
                        }
                    }

                    // Spawn new particles
                    particleComp.timeSinceLastSpawn += float(deltaTime);
                    if (particleComp.timeSinceLastSpawn >= particleComp.spawnRate) {
                        particleComp.timeSinceLastSpawn = 0.0f;
                        SpawnParticle(entity, particleComp);
                    }
                }

            }
        }
    }


    std::vector<std::pair<Particle, HUGraphics::GLModel*>> GetActiveParticles() {
        std::vector<std::pair<Particle, HUGraphics::GLModel*>> activeParticles;

        for (auto entity : mEntities) {
            auto& particleComp = ECoordinator.GetComponent<ParticleComponent>(entity);

            for (size_t i = 0; i < particleComp.particles.size(); i++) {
                if (particleComp.particles[i].active) {
                    activeParticles.emplace_back(particleComp.particles[i], &particleComp.particleModels[i]);
                }
            }
        }

        return activeParticles;
    }

    void SpawnParticle(EntityID entity, ParticleComponent& particleComp) {
        if (particleComp.particles.size() >= particleComp.maxParticles) {
           // std::cout << "Particles: " << particleComp.particles.size() << "/" << particleComp.maxParticles << std::endl;

            return;
        }

        glm::vec3 objectPosition = ECoordinator.GetComponent<Transform>(entity).translate;

        auto scale = ECoordinator.GetComponent<Transform>(entity).scale;

        // Object dimensions
        float objectWidth = scale.x;
        float objectHeight = scale.y;

        float minDistanceX = objectWidth * 0.4f; // Closer to the object
        float minDistanceY = objectHeight * 0.4f;
        float maxDistanceX = objectWidth * 0.6f; // Keep particles nearby
        float maxDistanceY = objectHeight * 0.6f;

        int particlesToSpawn = 3 + (rand() % 3); // Spawns between 3 to 5 particles

        for (int i = 0; i < particlesToSpawn; ++i) {

            // Generate a random position **just outside** the object
            float xOffset = (minDistanceX + ((rand() / (float)RAND_MAX) * (maxDistanceX - minDistanceX))) * (rand() % 2 == 0 ? -1 : 1);
            float yOffset = (minDistanceY + ((rand() / (float)RAND_MAX) * (maxDistanceY - minDistanceY))) * (rand() % 2 == 0 ? -1 : 1);

            
            glm::vec3 spawnPosition = objectPosition + glm::vec3(xOffset, yOffset, 0.0f);

            // Velocity: Small floating effect instead of big movement
            float maxVelocity = 0.000001f;  
            glm::vec3 velocity = glm::vec3(
                ((rand() / (float)RAND_MAX) * maxVelocity * 2 - maxVelocity), // X movement
                ((rand() / (float)RAND_MAX) * maxVelocity * 2 - maxVelocity), // Y movement
                0.0f
            );

            float lifetime = 1.5f;
            float size = 1.0f;

            // Create a new particle
            particleComp.particles.emplace_back(spawnPosition, velocity, lifetime, size);
            // Log total particles after spawning
           // std::cout << "Spawning particle: Total = " << particleComp.particles.size() << std::endl;
            HUGraphics::GLModel particleModel = HUGraphics::star_model(2.f, 1.f, 5, { 1.0f, 0.84f, 0.0f });

            particleComp.particleModels.push_back(particleModel);
        }
    }




};


#endif