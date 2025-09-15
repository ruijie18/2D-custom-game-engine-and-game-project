/**
 * @file Physics.cpp
 * @brief Implementation of the core physics system, including movement, collision detection, and force application.
 *
 * This file defines the physics logic of the game engine, responsible for updating the positions and velocities of entities,
 * detecting and responding to collisions, and applying various forces (e.g., gravity, drag, and custom forces).
 * The file also integrates physics with the game's Entity Component System (ECS) for dynamic object manipulation and interaction.
 *
 * Key Features:
 * - **Force Management**:
 *   - `ApplyForces`: Calculates and applies forces to entities, updating acceleration and velocity based on mass.
 * - **Movement & Collision**:
 *   - `Movement`: Updates the position and velocity of a physics body, including movement based on applied forces.
 *   - `HandleCollisions`: Detects and handles collisions between physics bodies and entities.
 * - **Collision Response**:
 *   - `CollisionResponse`: Handles the response to detected collisions between entities. This function adjusts the velocities and positions of entities involved in the collision, ensuring realistic interaction and separation after impact.
 *   - Specific collision response functions:
 *     - `HandleThiefWallCollision`: Resolves collisions between Thief entities and Wall entities.
 *     - `HandleThiefObjectCollision`: Handles collisions between Thief entities and other objects.
 *     - `HandleThiefSwitchCollision`: Manages the response to Thief entities interacting with Switch entities.y
 *     - `HandleThiefDoorCollision`: Handles collisions between Thief entities and Door entities.
 * - **Gravity and Jumping**:
 *   - `ApplyGravity`: Applies gravity to entities, affecting their velocity and position over time.
 *   - `Jumping`: Manages the jump mechanics, adjusting the vertical velocity for jumping entities.
 *   - `CalculateLine`: Provides debug functionality for calculating and visualizing trajectories for physics bodies during movement.
 *   - `MouseDragInfo`: Handles drag actions, storing relevant information for drag-based movements.
 * - **Pause Mode**:
 *   - `isPaused`, `stepFrame`: Controls the pause functionality and frame stepping for debugging purposes.
 * - **Entity Updates**:
 *   - `MoveEntity`: Updates entity positions based on velocity and delta time.
 *   - `UpdateTransform`: Synchronizes physics-based positions with ECS transform components.
 * - **Audio Integration**:
 *   - Plays footstep sounds during movement and pauses the audio when the entity is idle.
 * Utility Functions:
 * - `SyncAABBWithTransform`: Synchronizes the AABB (bounding box) with the entity's transformation.
 * - `CheckPauseToggle`: Checks for pause/resume actions to control physics updates during gameplay.
 * - `EnforceWindowBoundaries`: Ensures entities stay within the window boundaries during the physics update.
 *
 * Key Systems and Interactions:
 * - Integrated with the ECS (Entity Component System), each entity has a `PhysicsBody` component that is updated during the physics simulation.
 * - Works alongside the collision detection system (`Collision.h`) to handle entity interactions and respond to collisions.
 * - Handles various force types like linear, rotational, drag, and custom forces to simulate realistic movement and behavior of game objects.
 * - Supports gravity, jumping, and friction as part of the physics engine, offering flexibility for different types of interactions and mechanics.
 * - Implements message handling through `CoreEngine::Observer` for reacting to collision events and other system messages.
 *
 * Author: Jason (80%)
 * Co-Author: Jarren (20%)
 */
#include "ParticleSystem.h"
#include "Physics.h"
#include "iostream"
#include "imgui.h"
#include "GlobalVariables.h"
#include "GameLogic.h"
#include "Render.h"
#include "Physics.h"
#include "ConfigLoading.h"
#include <random>
#include "Graphics.h"
#include "GlobalVariables.h"
#include "AnimationState.h"


// Audio Stuff
static const std::vector<std::string> footstepSounds = {
    "Footstep1.ogg", "Footstep2.ogg", "Footstep3.ogg",
    "Footstep4.ogg", "Footstep5.ogg", "Footstep6.ogg",
    "Footstep7.ogg"
};

static const std::vector<std::string> crawlSounds = {
    "Player_Crawl_01.ogg", "Player_Crawl_02.ogg", "Player_Crawl_03.ogg",
    "Player_Crawl_04.ogg", "Player_Crawl_05.ogg", "Player_Crawl_06.ogg",
    "Player_Crawl_07.ogg", "Player_Crawl_08.ogg"
};

static const std::vector<std::string> jumpSounds = {
    "JumpSound_1.ogg", "", "JumpSound_2.ogg", "JumpSound_3.ogg", "JumpSound_4.ogg", "", ""
};

static const std::vector<std::string> wooshSounds = {
    "Jump_Whoosh_01.ogg", "Jump_Whoosh_02.ogg", "Jump_Whoosh_03.ogg", "Jump_Whoosh_04.ogg"
};



// Store the channel ID for footsteps
int footstepChannelId = 10;
int jumpChannelId = 15;

static std::string currentCrawlSound = "";
static std::string currentFootstepSound = ""; 
static std::string currentJumpSound = ""; 
static std::string currentWooshSound = "";

void PhysicsSystem::Init()
{
    // Set up the signature for the PhysicsSystem
    Signature physicsSignature;
    physicsSignature.set(ECoordinator.GetComponentType<PhysicsSystem::PhysicsBody>()); // PhysicsSystem needs PhysicsBody component
    physicsSignature.set(ECoordinator.GetComponentType<RenderLayer>());
    ECoordinator.SetSystemSignature<PhysicsSystem>(physicsSignature);
}

void PhysicsSystem::Update(double deltaTime) {
    if (windowFocused) {
        spatialGrid.clear(); // Clear old data
        for (auto& entity : mEntities) {
            if (!ECoordinator.HasComponent<PhysicsBody>(entity)) continue;
            PhysicsBody& body = ECoordinator.GetComponent<PhysicsBody>(entity);
            spatialGrid.addEntity(entity, body.aabb.minX, body.aabb.minY, body.aabb.maxX, body.aabb.maxY);
        }

        if (CoreEngine::InputSystem::Stage == 1 || CoreEngine::InputSystem::Stage == 11 || CoreEngine::InputSystem::Stage == 12 || CoreEngine::InputSystem::Stage == 13) {
            for (auto& entity : mEntities) {
                if (entity == ECoordinator.getThiefID()) {
                    ProcessEntity(entity, deltaTime);
                }
            }
        }
    }
}


//Helper Function
void PhysicsSystem::ProcessEntity(EntityID entity, double deltaTime) {
    if (!ECoordinator.HasComponent<PhysicsBody>(entity) || !ECoordinator.HasComponent<Transform>(entity)) {
        return;
    }
    PhysicsBody& body = ECoordinator.GetComponent<PhysicsBody>(entity);
    Transform& transform = ECoordinator.GetComponent<Transform>(entity);

    SyncAABBWithTransform(entity, body, transform);

    if (body.category == "Thief") {
        ApplyGravity(body, deltaTime);
        Movement(body);
        if (allowThiefMoveIfTrue) {
            MouseDragInfo(body);
        }
        HandleCollisions(entity, body, deltaTime);
        EnforceWindowBoundaries(body, 1600.0f, 900.0f);
    }

    ApplyForces(body, deltaTime);
    MoveEntity(body, deltaTime);
    UpdateTransform(entity, body);
}

void PhysicsSystem::MoveEntity(PhysicsBody& body, double deltaTime) {
    body.aabb.minX += body.velocity.x * static_cast<float>(deltaTime);
    body.aabb.minY += body.velocity.y * static_cast<float>(deltaTime);
    body.aabb.maxX += body.velocity.x * static_cast<float>(deltaTime);
    body.aabb.maxY += body.velocity.y * static_cast<float>(deltaTime);

    body.position.x += body.velocity.x * static_cast<float>(deltaTime);
    body.position.y += body.velocity.y * static_cast<float>(deltaTime);
}

void PhysicsSystem::UpdateTransform(EntityID entity, PhysicsBody& body) {
    try {
        Transform& trans = ECoordinator.GetComponent<Transform>(entity);
        trans.translate = {
            body.position.x,
            body.position.y, 
            1
        };
    }
    catch (const std::exception& e) {
        std::cerr << "Entity " << entity << " is missing Transform component: " << e.what() << "\n";
    }
}

void PhysicsSystem::ApplyForces(PhysicsBody& body, double deltaTime) {
    // Get the total force for the body
    const Math2D::Vector2D totalForce = body.forcesManager.GetNetForce(deltaTime);

    // Update acceleration from total force: F = ma, so a = F/m
    body.acceleration.x = totalForce.x / body.mass; // Using mass for realistic simulation
    body.acceleration.y = totalForce.y / body.mass;

    // Update velocity with current acceleration
    body.velocity.x += static_cast<float>(body.acceleration.x * deltaTime);
    body.velocity.y += static_cast<float>(body.acceleration.y * deltaTime);

    // Reset forces after applying them
    body.forcesManager.ClearForces();
}


//Main function
void PhysicsSystem::Movement(PhysicsSystem::PhysicsBody& body) {
    //apply movement based on key presses ('A' for left, 'D' for right), and friction when no key is pressed
    if (InputSystem->IsKeyPress(GLFW_KEY_A) && body.isGrounded) {
        Math2D::Vector2D linearForce = { -1000.0f * body.mass, 0 };
        body.forcesManager.AddForce(linearForce);

        if (animStateMachine.GetCurrentState()->GetState() != AnimationState::CrouchWalk) {
            // Play or resume footsteps sound
            if (!audioEngine->isPlaying(currentFootstepSound.c_str())) {
                PlayRandomSound(footstepSounds, 10, currentFootstepSound, 0.4f);
            }
            else {
                audioEngine->UnpauseSoundByName(currentFootstepSound.c_str());
            }
        }
        else {
            if (!audioEngine->isPlaying(currentCrawlSound.c_str())) {
                PlayRandomSound(crawlSounds, 10, currentCrawlSound, 0.4f);
            }
            else {
                audioEngine->UnpauseSoundByName(currentCrawlSound.c_str());
            }
        }
        
    }
    else if (InputSystem->IsKeyPress(GLFW_KEY_D) && body.isGrounded) {
        Math2D::Vector2D linearForce = { 1000.0f * body.mass, 0 };
        body.forcesManager.AddForce(linearForce);

        if (animStateMachine.GetCurrentState()->GetState() != AnimationState::CrouchWalk) {
            // Play or resume footsteps sound
            if (!audioEngine->isPlaying(currentFootstepSound.c_str())) {
                PlayRandomSound(footstepSounds, 10, currentFootstepSound, 0.4f);
            }
            else {
                audioEngine->UnpauseSoundByName(currentFootstepSound.c_str());
            }
        }
        else {
            if (!audioEngine->isPlaying(currentCrawlSound.c_str())) {
                PlayRandomSound(crawlSounds, 10, currentCrawlSound, 0.4f);
            }
            else {
                audioEngine->UnpauseSoundByName(currentCrawlSound.c_str());
            }
        }

    }
    else {
        // Pause footsteps sound when no movement keys are pressed
        if (!currentFootstepSound.empty() && audioEngine->isPlaying(currentFootstepSound.c_str())) {
            audioEngine->PauseSoundByName(currentFootstepSound.c_str());
        }
    }

    if ((body.velocity.x > MOVE_VELOCITY) && body.isGrounded == true) {
        body.velocity.x = MOVE_VELOCITY;
    }
    if ((body.velocity.x < -MOVE_VELOCITY) && body.isGrounded == true) {
        body.velocity.x = -MOVE_VELOCITY;
    }

    //friction
    if (body.isGrounded == true) {
        body.velocity.x *= float(0.2);
    }
}

void PhysicsSystem::ApplyGravity(PhysicsSystem::PhysicsBody& body, double deltaTime) {
    // Apply gravity when the object is above the ground
    if (body.aabb.maxY < gravity) {
        // Apply continuous gravity force downwards
        /*Math2D::Vector2D gravityForce(0.0f, GRAVITY * body.mass);
        body.forcesManager.AddForce(gravityForce);*/
        body.velocity.y = body.velocity.y + static_cast<float>(GRAVITY) * static_cast<float>(deltaTime);
    }
    else {
        // Snap to ground level and reset vertical velocity
        /*body.aabb.minY = gravity - (body.aabb.maxY - body.aabb.minY);
        body.aabb.maxY = static_cast<float>(gravity);*/
        body.velocity.y = 0.0f;
        //body.velocity.x = 0.0f;
        body.isGrounded = true;
    }
}


// Collision Function
bool PhysicsSystem::HandleCollisions(EntityID entity, PhysicsBody& body, double deltaTime) {
    bool colliding = false;
    (void)deltaTime;

    // Get entity position for broad-phase filtering
    /*float centerX = (body.aabb.minX + body.aabb.maxX) / 2.0f;
    float centerY = (body.aabb.minY + body.aabb.maxY) / 2.0f;*/

    // Retrieve only nearby entities
    std::vector<int> potentialCollisions = spatialGrid.getNearbyEntities(
        body.aabb.minX, body.aabb.minY, body.aabb.maxX, body.aabb.maxY
    );

    for (EntityID otherEntity : potentialCollisions) { // this line throw error
        if (entity != otherEntity) {
            if (!ECoordinator.HasComponent<RenderLayer>(otherEntity)) {
                continue; // Skip if no RenderLayer exists
            }

            RenderLayer& otherRenderLayer = ECoordinator.GetComponent<RenderLayer>(otherEntity);

            // Check if the other entity's layer is the same or different
            //and check whether its a game object layer 
            if (entity  != static_cast<decltype(entity)>(otherEntity) && otherRenderLayer.layer == RenderLayerType::GameObject) {
                
                if (!ECoordinator.HasComponent<PhysicsBody>(otherEntity)) {
                    continue; // Skip if no PhysicsBody exists
                }

                PhysicsBody& otherBody = ECoordinator.GetComponent<PhysicsBody>(otherEntity);
                float firstTimeOfCollision;

                if (CollisionIntersection_RectRect(
                    body.aabb, body.velocity.x, body.velocity.y,
                    otherBody.aabb, otherBody.velocity.x, otherBody.velocity.y,
                    firstTimeOfCollision)) {

                    CollisionResponse(body, otherBody, firstTimeOfCollision, entity, otherEntity);
                    colliding = true;

                    //if collide with object sent a message to the rest of observers
                    CoreEngine::IMessage collisionMessage(CoreEngine::CollisionDetected, "PhysicsSystem");
                    CoreEngine::MessageBroker::Instance().Notify(&collisionMessage);
                }
            }
        }
    }

    for (EntityID id : entitiesToDestroy) {
        ECoordinator.DestroyGameObject(id);
        Object_picked += 1;

        //std::cout << Object_picked << "\n";

    }
    entitiesToDestroy.clear();
    return colliding;
}

// For Collision Response
void PhysicsSystem::CollisionResponse(PhysicsBody& body1, PhysicsBody& body2, float firstTimeOfCollision, EntityID entity, EntityID otherEntity) {

    // Helper lambda to reverse velocities
    auto ReverseVelocity = [](PhysicsBody& body, float timeRemaining) {
        body.velocity.x = -body.velocity.x * timeRemaining;
    };


    // Helper lambda to rollback to collision point
    auto RollbackPosition = [](PhysicsBody& body, float timeOfCollision) {
        body.aabb.minX += body.velocity.x * timeOfCollision;
        body.aabb.maxX = body.aabb.minX + 200.0f;  // Adjust AABB to maintain size
    };

    // Handle specific collision cases
    if (IsCollision("Thief", "Wall", body1, body2)) {
        HandleThiefWallCollision(body1, body2, firstTimeOfCollision);
    }
    else if (IsCollision("Thief", "Object", body1, body2)) {
        // Check which body is the "Thief" and which is the "Object"
        EntityID objectEntityID = (body1.category == "Object") ? entity : otherEntity;
        HandleThiefObjectCollision(body1, body2, firstTimeOfCollision, objectEntityID); // Pass correct EntityID
    }
    else if (IsCollision("Thief", "Switch", body1, body2)) {
        // Check which body is the "Thief" and which is the "Switch"
        EntityID switchEntityID = (body1.category == "Switch") ? entity : otherEntity;
        HandleThiefSwitchCollision(body1, firstTimeOfCollision, switchEntityID); // Pass correct EntityID
    }
    else if (IsCollision("Thief", "Door", body1, body2)) {
        EntityID doorEntityID = (body1.category == "Door") ? entity : otherEntity;
        HandleThiefDoorCollision(body1, body2, firstTimeOfCollision, doorEntityID);
    }
    else if (IsCollision("Thief", "Laser", body1, body2)) {
        if (body2.category == "Laser Module") {
            return; // Skip processing if it's a "Laser Module"
        }
        EntityID laserEntityID = (body1.category == "Laser") ? entity : otherEntity;
        HandleThiefLaserCollision(body1, body2, firstTimeOfCollision, laserEntityID);
    }
    else if (IsCollision("Thief", "Vent", body1, body2)) {
        EntityID ventEntityID = (body1.category == "Vent") ? entity : otherEntity;
        HandleThiefVentCollision(body1, body2, firstTimeOfCollision, ventEntityID);
    }

    // Send message to MessageBroker
    CoreEngine::IMessage* collisionMessage = new CoreEngine::IMessage(CoreEngine::MessageID::CollisionDetected, "PhysicsSystem");
    CoreEngine::MessageBroker::Instance().Notify(collisionMessage);
    delete collisionMessage;  // Clean up
}

// Utility function to check if the collision involves the specified entities
bool PhysicsSystem::IsCollision(const std::string& entity1, const std::string& entity2, PhysicsBody& body1, PhysicsBody& body2) {
    return ((body1.category.find(entity1) != std::string::npos) && (body2.category.find(entity2) != std::string::npos)) ||
        ((body1.category.find(entity2) != std::string::npos) && (body2.category.find(entity1) != std::string::npos));
}

// Handle Thief vs Object Collision
void PhysicsSystem::HandleThiefObjectCollision(PhysicsBody& body1, PhysicsBody& body2, float firstTimeOfCollision, EntityID objectEntityID) {
    (void)firstTimeOfCollision;
    (void)objectEntityID;

    PhysicsBody& object = (body1.category == "Object") ? body1 : body2;

    // Check for 'E' key press to pick up the object
    if (!audioEngine->isPlaying("TreasurePickUp.ogg")) {
        audioEngine->PlaySound("TreasurePickUp.ogg", 0, 0.1f * sfxVolume);
    }
    /*if (ECoordinator.HasComponent<ParticleComponent>(objectEntityID)) {
        ECoordinator.RemoveComponent<ParticleComponent>(objectEntityID);
    }*/
    
    entitiesToDestroy.push_back(object.entityID);  // Destroy the object
    std::sort(entitiesToDestroy.begin(), entitiesToDestroy.end());

    auto newEnd = std::unique(entitiesToDestroy.begin(), entitiesToDestroy.end());

    // Erase the duplicate elements
    entitiesToDestroy.erase(newEnd, entitiesToDestroy.end());

    
}

// Handle Thief vs Switch collision
void PhysicsSystem::HandleThiefSwitchCollision(PhysicsBody& body1, float firstTimeOfCollision, EntityID switchEntityID) {
    (void)firstTimeOfCollision;
    (void)switchEntityID;

    static bool yKeyPreviouslyPressed = false;

    // Check if the 'E' key is pressed
    bool yKeyPressed = CoreEngine::InputSystem::IsKeyPress(GLFW_KEY_E);
    if (yKeyPressed && !yKeyPreviouslyPressed && body1.isGrounded == true) {
        // Determine which body is the Switch
        //std::cout << switchEntityID;
        EntityID switchEntity = switchEntityID;
        PhysicsBody& switchBody = ECoordinator.GetComponent<PhysicsBody>(switchEntity);
        if (!ECoordinator.HasComponent<Switch>(switchEntity)) {
            return; // Exit early if the entity is not actually a switch
        }
        Switch& switchComponent = ECoordinator.GetComponent<Switch>(switchEntity);


        // Toggle the switch state
        switchBody.Switch = !switchBody.Switch;

        // Play switch sound
        audioEngine->PlaySound("SwitchInteract.ogg", 0, 0.3f * sfxVolume);
        

        // Retrieve the Switch's model component
        HUGraphics::GLModel& switchModel = ECoordinator.GetComponent<HUGraphics::GLModel>(switchEntity);
        std::string newTextureFile;
        GLuint textureID = 0;
        std::shared_ptr<Texture> activeTexture;
       
        // Update the color based on the switch state
        if (ECoordinator.HasComponent<PhysicsSystem::Switch>(switchEntity)) {
            if (ECoordinator.GetComponent<HUGraphics::GLModel>(switchEntity).textureFile == "./Assets/Textures\\SwitchesOn.png" || 
                ECoordinator.GetComponent<HUGraphics::GLModel>(switchEntity).textureFile == "./Assets/Textures\\SwitchesOff.png" || 
                ECoordinator.GetComponent<HUGraphics::GLModel>(switchEntity).textureFile == "SwitchesOn.png" || 
                ECoordinator.GetComponent<HUGraphics::GLModel>(switchEntity).textureFile == "SwitchesOff.png") {
                if (switchBody.Switch) {
                    activeTexture = TextureLibrary.GetAssets("SwitchesOn.png");
                    newTextureFile = "SwitchesOn.png";
                    audioEngine->PlaySound("Laser_Off.ogg", 0, 0.1f * sfxVolume);
                }
                else {
                    activeTexture = TextureLibrary.GetAssets("SwitchesOff.png");
                    newTextureFile = "SwitchesOff.png";
                    audioEngine->PlaySound("Laser_On.ogg", 0, 0.1f * sfxVolume);
                }
            }
            else {
                if (switchBody.Switch) {
                    activeTexture = TextureLibrary.GetAssets("DoorSwitchesOn.png");
                    newTextureFile = "DoorSwitchesOn.png";
                    
                }
                else {
                    activeTexture = TextureLibrary.GetAssets("DoorSwitchesOff.png");
                    newTextureFile = "DoorSwitchesOff.png";
                }
                audioEngine->PlaySound("LockedDoorCut.ogg", 0, 0.3f * sfxVolume);
            }

            if (activeTexture) {
                textureID = activeTexture->GetTextureID();
                if (textureID == 0) {
                    //std::cerr << "Failed to load active texture: toplazergreen.png" << std::endl;
                }
            }
            else {
                //std::cerr << "Active texture not found in library!" << std::endl;
            }

            if (textureID != 0) {
                switchModel.textureID = textureID;
                switchModel.textureFile = newTextureFile; // Update the texture reference
            }

        }

        for (const auto& interactable : switchComponent.interactables) {
            for (auto& entity : mEntities) {
                // Check for name
                if (ECoordinator.GetComponent<Name>(entity).name != interactable) {
                    continue;
                }
                // If name matches but no physicsBody, add in.
                if (!ECoordinator.HasComponent<PhysicsBody>(entity)) {
                    ECoordinator.AddComponent(entity, PhysicsBody{});
                }
                // Use a separate variable for the interactable entity's physics body
                PhysicsBody& interactablePhysBody = ECoordinator.GetComponent<PhysicsBody>(entity);

                if (interactablePhysBody.category == "LockDoor") {
                    interactablePhysBody.Switch = !interactablePhysBody.Switch;
                    HUGraphics::GLModel& doorModel = ECoordinator.GetComponent<HUGraphics::GLModel>(entity);

                    std::string newTextureFiles = interactablePhysBody.Switch ? "./Assets/Textures/OpenDoor.png" : "./Assets/Textures/Door.png";

                    // Ensure the texture is valid and update the model
                    if (!newTextureFiles.empty()) {
                        Texture& newTexture = *TextureLibrary.GetAssets(TextureLibrary.GetName(newTextureFiles));
                        doorModel = HUGraphics::texture_mesh(newTexture); // Update the model with the new texture
                        doorModel.textureFile = newTextureFiles;           // Store the texture file for reference
                    }
                }
                if (interactablePhysBody.category == "Laser") {
                    // If no laser component, add in.
                    if (!ECoordinator.HasComponent<LaserComponent>(entity)) {
                        ECoordinator.AddComponent(entity, LaserComponent{});
                    }

                    LaserComponent& laserComp = ECoordinator.GetComponent<LaserComponent>(entity);
                    laserComp.turnedOn = !laserComp.turnedOn;
                }
            }
        }
    }

    // Update the previous key state
    yKeyPreviouslyPressed = yKeyPressed;                                                                                 
}

// Handle Thief vs MDoor collision
void PhysicsSystem::HandleThiefDoorCollision(PhysicsBody& body1, PhysicsBody& body2, float firstTimeOfCollision, EntityID doorEntityID) {
    (void)firstTimeOfCollision;
    (void)doorEntityID;

    PhysicsBody& doorBody = (body1.category.find("Door") != std::string::npos) ? body1 : body2;
    PhysicsBody& thiefBody = (body1.category.find("Thief") != std::string::npos) ? body1 : body2;

    static bool yKeyPreviouslyPressed = false;

    // Check if the 'E' key is pressed
    bool yKeyPressed = CoreEngine::InputSystem::IsKeyPress(GLFW_KEY_E);

    if (doorBody.category.find("Lock") != std::string::npos) {
        yKeyPressed = false;  // Ignore 'E' press on locked doors
    }

    if (yKeyPressed && !yKeyPreviouslyPressed) {

        // To ignore Door control by switch
        if (doorBody.category.find("Lock") != std::string::npos) {
            return;
        }

        // Determine which body is the Switch
        EntityID doorEntity = (body1.category.find("Door") != std::string::npos) ? body1.entityID : body2.entityID;
        
        // Toggle the door state
        doorBody.Switch = !doorBody.Switch;

        // Play the sound for door open
        audioEngine->PlaySound("NormalDoor.ogg", 0, 0.2f * sfxVolume);

        // Update the texture based on the door state
        std::string newTextureFile = doorBody.Switch ? "./Assets/Textures/OpenDoor.png" : "./Assets/Textures/Door.png";
        // Get the transform component of the door entity
        auto& doorTransform = ECoordinator.GetComponent<Transform>(doorEntity);



        if (doorBody.Switch) {
            // Door is open: Increase width and move right
            doorTransform.scale.x += 40;
            doorTransform.translate.x -=20;
        }
        else {
            // Door is closed: Reset to original
            doorTransform.scale.x -= 40;
            doorTransform.translate.x += 20;
        }
    }

    // Prevent Thief from walking through if door is closed
    // if they at left stop them at left, right at right
    // if overlay when door close toggle, calculate which side closer and push thief to that sid
    if (!body2.Switch) {

        // Calculate the closest side to resolve overlap
        float leftOverlap = std::abs(thiefBody.aabb.maxX - doorBody.aabb.minX - 1.0f);
        float rightOverlap = std::abs(thiefBody.aabb.minX - doorBody.aabb.maxX);

        float thiefwidth = thiefBody.aabb.maxX - thiefBody.aabb.minX;

        // Resolve position based on the smallest overlap
        if (leftOverlap <= rightOverlap) {
            thiefBody.aabb.maxX = doorBody.aabb.minX + 0.01f;
            thiefBody.aabb.minX = thiefBody.aabb.maxX - thiefwidth;
        }
        else if (rightOverlap <= leftOverlap) {
            thiefBody.aabb.minX = doorBody.aabb.maxX - 0.01f;
            thiefBody.aabb.maxX = thiefBody.aabb.minX + thiefwidth;
        }
    }

    // Update the previous key state
    yKeyPreviouslyPressed = yKeyPressed;
}

// Handle Thief vs Vent collision
void PhysicsSystem::HandleThiefVentCollision(PhysicsBody& body1, PhysicsBody& body2, float firstTimeOfCollision, EntityID doorEntityID) {
    (void)firstTimeOfCollision;
    (void)doorEntityID;

    PhysicsBody& doorBody = (body1.category.find("Door") != std::string::npos) ? body1 : body2;
    PhysicsBody& thiefBody = (body1.category.find("Thief") != std::string::npos) ? body1 : body2;

    static bool yKeyPreviouslyPressed = false;

    // Check if the 'Y' key is pressed
    bool yKeyPressed = CoreEngine::InputSystem::IsKeyPress(GLFW_KEY_E);
    if (yKeyPressed && !yKeyPreviouslyPressed && body1.isGrounded == true) {

        // To ignore Door control by switch
        if (doorBody.category.find("Lock") != std::string::npos) {
            return;
        }

        // Determine which body is the Switch
        EntityID doorEntity = (body1.category.find("Door") != std::string::npos) ? body1.entityID : body2.entityID;

        // Toggle the door state
        doorBody.Switch = !doorBody.Switch;

        // Play the sound for door open
        audioEngine->PlaySound("NormalDoor.ogg", 0, 0.2f * sfxVolume);

        // Retrieve the door's model component
        HUGraphics::GLModel& doorModel = ECoordinator.GetComponent<HUGraphics::GLModel>(doorEntity);

        // Update the texture based on the door state
        std::string newTextureFile = doorBody.Switch ? "./Assets/Textures/OpenDoor.png" : "./Assets/Textures/Door.png";

        // Ensure the texture is valid and update the model
        if (!newTextureFile.empty()) {
            Texture& newTexture = *TextureLibrary.GetAssets(TextureLibrary.GetName(newTextureFile));
            doorModel = HUGraphics::texture_mesh(newTexture); // Update the model with the new texture
            doorModel.textureFile = newTextureFile;           // Store the texture file for reference
        }
    }

    // Prevent Theif from walking through if door is closed
    // if they at left stop them at left, right at right
    // if overlay when door close toggle, calculate which side closer and push thieft to that sid
    if (!body2.Switch) {

        // Calculate the closest side to resolve overlap
        float leftOverlap = std::abs(thiefBody.aabb.maxX - doorBody.aabb.minX);
        float rightOverlap = std::abs(thiefBody.aabb.minX - doorBody.aabb.maxX);
        float topOverlap = std::abs(thiefBody.aabb.maxY - doorBody.aabb.minY);
        float bottomOverlap = std::abs(thiefBody.aabb.minY - doorBody.aabb.maxY);

        // Resolve position based on the smallest overlap
        if (leftOverlap <= rightOverlap && leftOverlap <= topOverlap && leftOverlap <= bottomOverlap) {
            thiefBody.aabb.maxX = doorBody.aabb.minX - 0.01f;
            thiefBody.aabb.minX = thiefBody.aabb.maxX - (thiefBody.aabb.maxX - thiefBody.aabb.minX);
        }
        else if (rightOverlap <= leftOverlap && rightOverlap <= topOverlap && rightOverlap <= bottomOverlap) {
            thiefBody.aabb.minX = doorBody.aabb.maxX + 0.01f;
            thiefBody.aabb.maxX = thiefBody.aabb.minX + (thiefBody.aabb.maxX - thiefBody.aabb.minX);
        }
        else if (topOverlap <= leftOverlap && topOverlap <= rightOverlap && topOverlap <= bottomOverlap) {
            thiefBody.aabb.maxY = doorBody.aabb.minY - 0.01f;
            thiefBody.aabb.minY = thiefBody.aabb.maxY - (thiefBody.aabb.maxY - thiefBody.aabb.minY);
        }
        else {
            thiefBody.aabb.minY = doorBody.aabb.maxY + 0.01f;
            thiefBody.aabb.maxY = thiefBody.aabb.minY + (thiefBody.aabb.maxY - thiefBody.aabb.minY);
        }
    }

    // Update the previous key state
    yKeyPreviouslyPressed = yKeyPressed;
}

// Handle Thief vs Wall collision
void PhysicsSystem::HandleThiefWallCollision(PhysicsBody& body1, PhysicsBody& body2, float firstTimeOfCollision) {
    PhysicsBody& thief = (body1.category == "Thief") ? body1 : body2;
    PhysicsBody& wall = (body1.category == "Wall") ? body1 : body2;
    
    // Calculate the thief's center position and half size for both dimensions
    float thiefCenterX = (thief.aabb.minX + thief.aabb.maxX) / 2.0f;
    float thiefCenterY = (thief.aabb.minY + thief.aabb.maxY) / 2.0f;
    float thiefHalfWidth = (thief.aabb.maxX - thief.aabb.minX) / 2.0f;
    float thiefHalfHeight = (thief.aabb.maxY - thief.aabb.minY) / 2.0f;

    // Calculate the wall's center position and half size for both dimensions
    float wallCenterX = (wall.aabb.minX + wall.aabb.maxX) / 2.0f;
    float wallCenterY = (wall.aabb.minY + wall.aabb.maxY) / 2.0f;
    float wallHalfWidth = (wall.aabb.maxX - wall.aabb.minX) / 2.0f;
    float wallHalfHeight = (wall.aabb.maxY - wall.aabb.minY) / 2.0f;

    // Check the direction of the collision
    float deltaX = thiefCenterX - wallCenterX;
    float deltaY = thiefCenterY - wallCenterY;

    // Calculate the overlap in both directions
    float overlapX = wallHalfWidth + thiefHalfWidth - std::abs(deltaX);
    float overlapY = wallHalfHeight + thiefHalfHeight - std::abs(deltaY);

    if (overlapX < overlapY) {
        // Horizontal collision
        float thiefwidth = thief.aabb.maxX - thief.aabb.minX;
        if (deltaX > 0) {
            // Collision from the left of the wall
            thief.aabb.minX = wall.aabb.maxX + 0.01f;  // Add offset
            thief.aabb.maxX = thief.aabb.minX + thiefwidth;
            thief.position.x += 0.01f;
        }
        else {
            // Collision from the right of the wall
            thief.aabb.maxX = wall.aabb.minX - 0.01f;  // Add offset
            thief.aabb.minX = thief.aabb.maxX - thiefwidth;
            thief.position.x -= 0.01f;
        }
    }
    else {
        bool checktop = false; 
        bool checkbtm = false;
        // Vertical collision
        thief.aabb.minY += thief.velocity.y * firstTimeOfCollision;
        float thiefheight = thief.aabb.maxY - thief.aabb.minY;
        if (deltaY > 0) {
            // Collision above (below the wall)
            thief.velocity.y = 0;  // Stop downward movement
            thief.aabb.minY = wall.aabb.maxY;  // Place thief on top of the wall
            thief.aabb.maxY = thief.aabb.minY + thiefheight;
            checktop = true;
        }
        if (deltaY < 0) {
            // Collision below (on top the wall)
            thief.velocity.y = 0;  // Stop upward movement
            thief.aabb.maxY = wall.aabb.minY + 0.1f;  // Align thief's top with the wall's bottom
            thief.aabb.minY = thief.aabb.maxY - thiefheight;
            checkbtm = true;
            if (!(thief.isGrounded)) {
                thief.velocity.x = 0;
                thief.velocity.y = 0;
            }
            thief.isGrounded = true;
        }
    }
}

// Handle Thief vs Laser collision
void PhysicsSystem::HandleThiefLaserCollision(PhysicsBody& body1, PhysicsBody& body2, float firstTimeOfCollision, EntityID laserEntityID) {
    (void)firstTimeOfCollision;
    (void)laserEntityID;

    static float lastHitTime = 0.0f;
    const float HIT_COOLDOWN = 0.5f;

    PhysicsBody& thief = (body1.category == "Thief") ? body1 : body2;
    PhysicsBody& laser = (body1.category == "Laser") ? body1 : body2;

    if (ECoordinator.GetComponent<LaserComponent>(laser.entityID).isActive &&
        ECoordinator.GetComponent<LaserComponent>(laser.entityID).turnedOn) {

        float currentTime = float(glfwGetTime());

        if (currentTime - lastHitTime > HIT_COOLDOWN) {
            LaserComponent& laserComp = ECoordinator.GetComponent<LaserComponent>(laser.entityID);

            if (laserComp.isActive) {
                health -= 1;
                lastHitTime = currentTime;
                audioEngine->PlaySound("ElectricZap.ogg", 0, 0.2f * sfxVolume);
                /*float laserCenterX = (laser.aabb.minX + laser.aabb.maxX) / 2.0f;
                float laserCenterY = (laser.aabb.minY + laser.aabb.maxY) / 2.0f;

                float thiefCenterX = (thief.aabb.minX + thief.aabb.maxX) / 2.0f;
                float thiefCenterY = (thief.aabb.minY + thief.aabb.maxY) / 2.0f;*/

                // Calculate distances from thief to laser edges
                float distLeft = abs(thief.aabb.maxX - laser.aabb.minX);
                float distRight = abs(thief.aabb.minX - laser.aabb.maxX);
                float distTop = abs(thief.aabb.minY - laser.aabb.maxY);
                float distBottom = abs(thief.aabb.maxY - laser.aabb.minY);

                // Find closest escape direction
                float minDist = std::min({ distLeft, distRight, distTop, distBottom });

                float bounceX = 0.0f;
                float bounceY = 0.0f;

                float size = body1.aabb.maxX - body1.aabb.minX;

                if (minDist == distLeft) {
                    // Move thief left
                    bounceX = -50.0f;
                    thief.aabb.maxX = laser.aabb.minX;
                    thief.aabb.minX = thief.aabb.maxX - size;
                }
                else if (minDist == distRight) {
                    // Move thief right
                    bounceX = 50.0f;
                    thief.aabb.minX = laser.aabb.maxX;
                    thief.aabb.maxX = thief.aabb.minX + size;
                }
                else if (minDist == distTop) {
                    // Move thief upwards
                    bounceY = 50.0f;
                    thief.aabb.minY = laser.aabb.maxY;
                    thief.aabb.maxY = thief.aabb.minY + size;
                }
                else if (minDist == distBottom) {
                    // Move thief downwards
                    bounceY = -50.0f;
                    thief.aabb.maxY = laser.aabb.minY;
                    thief.aabb.minY = thief.aabb.maxY - size;
                }

                // Apply knockback
                thief.velocity.x = bounceX;
                thief.velocity.y = bounceY;
            }
        }
    }
}


// Handle Thief vs Screen Boundary
void PhysicsSystem::EnforceWindowBoundaries(PhysicsBody& body, float windowWidth, float windowHeight) {
    if (body.category == "Thief") {
        // Ensure the Thief stays within the window boundaries
        float width = body.aabb.maxX - body.aabb.minX;
        float height = body.aabb.maxY - body.aabb.minY;

        // Left boundary
        if (body.aabb.minX < 0.0f) {
            body.aabb.minX = 0.0f;
            body.aabb.maxX = body.aabb.minX + width;
            body.velocity.x = 0.0f;

        }
        // Right boundary
        if (body.aabb.maxX > windowWidth) {
            body.aabb.maxX = windowWidth;
            body.aabb.minX = windowWidth - width;
            body.velocity.x = 0.0f; // Stop horizontal movement
        }
        // Bottom boundary
        if (body.aabb.maxY > windowHeight - 63.0f) {
            body.aabb.maxY = windowHeight - 63.0f;
            body.aabb.minY = body.aabb.maxY - height;
            body.velocity.y = 0.0f; // Stop vertical movement
        }
        
        // Top boundary
        if (body.aabb.minY < 0.0f) {
            body.aabb.minY = 0.1f;
            body.aabb.maxY = body.aabb.minY + height;
            body.velocity.y = 0.0f; // Stop vertical movement
        }

        body.position.x = (body.aabb.minX + body.aabb.maxX) / 2.0f;
        body.position.y = (body.aabb.minY + body.aabb.maxY) / 2.0f;

        //if (body.position.x - body.size.x / 2.0f < 0.0f) {
        //    body.position.x = body.size.x / 2.0f;  // Ensure it's inside the boundary
        //    body.velocity.x = 0.0f;
        //}

        //// Right boundary
        //if (body.position.x + body.size.x / 2.0f > windowWidth) {
        //    body.position.x = windowWidth - body.size.x / 2.0f;
        //    body.velocity.x = 0.0f;
        //}

        //// Bottom boundary
        //if (body.position.y + body.size.y / 2.0f > windowHeight) {
        //    body.position.y = windowHeight - body.size.y / 2.0f;
        //    body.velocity.y = 0.0f;
        //}

        //// Top boundary
        //if (body.position.y - body.size.y / 2.0f < 0.0f) {
        //    body.position.y = body.size.y / 2.0f;
        //    body.velocity.y = 0.0f;
        //}
    }
}

void PhysicsSystem::SyncAABBWithTransform(EntityID entity, PhysicsBody& body, const Transform& transform) {
    (void)entity;
    body.position.x = transform.translate.x;
    body.position.y = transform.translate.y;
    body.size.x = transform.scale.x;
    body.size.y = transform.scale.y;
}


void PhysicsSystem::MouseDragInfo(PhysicsSystem::PhysicsBody& body) {

    // Prevent mid air jumping again
    if (body.isGrounded == false) {
        return;
    }
    if (InputSystem->IsMousePositionValid()) {
        if (InputSystem->IsMousePressed(0) && (DragInfo.isDragging == false)) {
            DragInfo.DragBeginX = static_cast<float>(InputSystem->GetMousePosition().first);
            DragInfo.DragBeginY = static_cast<float>(InputSystem->GetMousePosition().second);
            DragInfo.isDragging = true;
        }
        else if (DragInfo.isDragging) {
            DragInfo.DragEndX = static_cast<float>(InputSystem->GetMousePosition().first);
            DragInfo.DragEndY = static_cast<float>(InputSystem->GetMousePosition().second);

            // Calculate drag vector
            DragInfo.dragVector.x = DragInfo.DragEndX - DragInfo.DragBeginX;
            DragInfo.dragVector.y = DragInfo.DragEndY - DragInfo.DragBeginY;

            // Draw the calculated trajectory preview line
            CalculateLine(&DragInfo, body);

            // When mouse is released, execute the jump
            if (!InputSystem->IsMousePressed(0)) {
                DragInfo.isDragging = false;
                body.isGrounded = false;
                // Destroy all previously created trajectory entities
                for (EntityID entity : DragInfo.trajectoryEntities) {
                    ECoordinator.DestroyGameObject(entity); // Clean up the entity completely
                }
                DragInfo.trajectoryEntities.clear();

                PlayRandomSound(jumpSounds, 15, currentJumpSound, 0.3f);

                PlayRandomSound(wooshSounds, 12, currentWooshSound, 0.2f);
               

                Jumping(body, &DragInfo);
            }
        }
    }
}

void PhysicsSystem::Jumping(PhysicsSystem::PhysicsBody& body, PhysicsTemp::DragInfo* dragInfo) {
    //const float SCALE_FACTOR = 0.1f;  // Scale drag vector to control jump power
    const float MAX_JUMP_VELOCITY = 170.0f;

    // Calculate the magnitude of the drag vector
    float dragMagnitude = std::sqrt(
        dragInfo->dragVector.x * dragInfo->dragVector.x +
        dragInfo->dragVector.y * dragInfo->dragVector.y
    );

    // If the drag vector exceeds the maximum allowed jump velocity, scale it down
    if (dragMagnitude > MAX_JUMP_VELOCITY) {
        float scaleFactor = MAX_JUMP_VELOCITY / dragMagnitude;
        dragInfo->dragVector.x *= scaleFactor;
        dragInfo->dragVector.y *= scaleFactor;
    }
        
    body.aabb.minY -= 1.0f;
    body.aabb.maxY -= 1.0f;
    // Calculate and scale velocity based on drag vector
    body.velocity.x = -dragInfo->dragVector.x;
    body.velocity.y = -dragInfo->dragVector.y;
}

void PhysicsSystem::CalculateLine(PhysicsTemp::DragInfo* dragInfo, PhysicsSystem::PhysicsBody& body) {
    // Destroy all previously created trajectory entities
    for (EntityID entity : dragInfo->trajectoryEntities) {

        if (ECoordinator.HasComponent<HUGraphics::GLModel>(entity)) {
            ECoordinator.GetComponent<HUGraphics::GLModel>(entity).cleanup();
            glDeleteTextures(1, &ECoordinator.GetComponent<HUGraphics::GLModel>(entity).textureID);
            ECoordinator.DestroyGameObject(entity); // Clean up the entity completely
        }

       
    }
    dragInfo->trajectoryEntities.clear();

    // Define the maximum drag distance
    const float MAX_DRAG_DISTANCE = 170.0f;

    // Calculate raw drag vector and its magnitude
    float dragVectorX = dragInfo->dragVector.x;
    float dragVectorY = dragInfo->dragVector.y;
    float dragMagnitude = std::sqrt(dragVectorX * dragVectorX + dragVectorY * dragVectorY);

    // Clamp the drag magnitude to the maximum allowed distance
    if (dragMagnitude > MAX_DRAG_DISTANCE) {
        float scaleFactor = MAX_DRAG_DISTANCE / dragMagnitude;
        dragVectorX *= scaleFactor;
        dragVectorY *= scaleFactor;
        dragMagnitude = MAX_DRAG_DISTANCE;
    }

    // Updated drag vector for trajectory and force calculation
    Math2D::Vector2D initialVelocity = { -dragVectorX, -dragVectorY };

    // Use the body's position as the starting point
    Math2D::Vector2D position = body.position;

    // Spacing between trajectory points
    const float pointSpacing = 40.0f;
    std::vector<Math2D::Vector2D> trajectoryPoints;
    trajectoryPoints.push_back(position);

    // Determine the number of trajectory points based on clamped drag distance
    const float POINTS_PER_UNIT = 0.2f;  // Adjust this value to control density
    const int MAX_POINTS = 100;          // Upper limit to prevent excessive computation
    int trajectoryPointLimit = static_cast<int>(dragMagnitude * POINTS_PER_UNIT);
    trajectoryPointLimit = std::min(trajectoryPointLimit, MAX_POINTS);

    float totalDistance = 0.0f;

    // Loop to calculate trajectory points
    while (trajectoryPoints.size() < trajectoryPointLimit) {  // Limit the number of points
        float t = totalDistance / (std::sqrt(initialVelocity.x * initialVelocity.x + initialVelocity.y * initialVelocity.y));

        // Calculate the next trajectory point
        Math2D::Vector2D nextPoint;
        nextPoint.x = position.x + initialVelocity.x * t;
        nextPoint.y = position.y + (initialVelocity.y * t) + (0.5f * GRAVITY * t * t);

        // Stop rendering when hitting the ground
        if (nextPoint.y >= 900.0f) break;

        totalDistance += pointSpacing;  // Update the total distance

        trajectoryPoints.push_back(nextPoint);  // Add point to the list
    }

    // Render all trajectory points as a single line entity
    EntityID trajectoryEntity = ECoordinator.CreateGameObject();
    HUGraphics::GLModel model = HUGraphics::points_model(trajectoryPoints);

    // Attach components for rendering
    Transform transform;
    transform.translate = { 0.0f, 0.0f, 1.0f };  // Keep z-index consistent
    RenderLayer layer = RenderLayerType::GameObject;

    ECoordinator.AddComponent(trajectoryEntity, transform);
    ECoordinator.AddComponent(trajectoryEntity, model);
    ECoordinator.AddComponent(trajectoryEntity, layer);

    // Store the trajectory entity
    dragInfo->trajectoryEntities.push_back(trajectoryEntity);
}


void PlayRandomSound(const std::vector<std::string>& soundList, int customChannel, std::string& currentSound, float volume) {
    // Select a random sound
    int randomIndex = rand() % soundList.size();
    const std::string& nextSound = soundList[randomIndex];

    // Update the original variable
    currentSound = nextSound;

    // Play the selected sound
    audioEngine->PlaySound(nextSound, 0, volume * sfxVolume, customChannel);
}