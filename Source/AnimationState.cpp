/**
 * @file AnimationStateMachine.cpp
 * @brief Implements the animation state management system.
 *
 * This file defines the `AnimationStateMachine` class, responsible for managing animation states
 * and transitions for a player character. It includes implementations for state handling,
 * JSON-based animation model loading, and updating animation states based on player input.
 *
 * Author: Rui Jie (85%)
 * co-author: Jason (10%)
 * co-author : jarren(5%)
 */

#include "GlobalVariables.h"
#include "AnimationState.h"
#include <iostream>
#include <fstream>
#include <unordered_map>


static const std::vector<std::string> landsound = {
    "Player_Land_01.ogg", "Player_Land_02.ogg", "Player_Land_03.ogg", "Player_Land_04.ogg"
};

static std::string currentlandSound = "";

static bool isFacingRight = true;  //the player faces right at the start

//convert enum to string in order std::out or read to map
std::string ToString(AnimationState state) {
    switch (state) {
    case AnimationState::Walking: return "Walking";
    case AnimationState::IDLE: return "Idle";
    case AnimationState::Jumping: return "Jumping";
    case AnimationState::Crouching: return "Crouching";
    case AnimationState::CrouchWalk: return "CrouchWalk";
    case AnimationState::Falling: return "Falling";

    default: return "Undefined";
    }
}

std::ostream& operator<<(std::ostream& os, const AnimationState& state) {
    switch (state) {
    case AnimationState::IDLE:
        os << "IDLE";
        break;
    case AnimationState::Walking:
        os << "Walking";
        break;
    case AnimationState::Jumping:
        os << "Jumping";
        break;
    case AnimationState::CrouchWalk:
        os << "CrouchWalk";
        break;
    case AnimationState::Falling:
        os << "Falling";
    default:
        os << "Unknown State";
        break;
    }
    return os;
}


// Forward declaration of the animation models map
static std::unordered_map<AnimationState, AnimationModel> player_models;

// Add a state to the state machine
void AnimationStateMachine::AddState(std::unique_ptr<State> state) {
    AnimationState stateEnum = state->GetState();
    states[stateEnum] = std::move(state);
}

// Transition to a new state
void AnimationStateMachine::TransitionTo(AnimationState newState) {
    if (states.find(newState) == states.end()) {
        std::cerr << "State " << ToString(newState) << " not found!" << std::endl;
        return;
    }

    if (currentState != nullptr) {
        currentState->Exit(); // Call exit on current state
    }

    currentState = states[newState].get();

    if (currentState != nullptr) {
        currentState->Enter(); // Call enter on the new state
    }
    else {
        std::cerr << "Error: currentState is null after transition to " << ToString(newState) << std::endl;
    }
}


// Update the current state
void AnimationStateMachine::UpdateState() {
    if (currentState != nullptr) {
        currentState->Update(); // Update the current state
    }
}

class WalkingState : public State {
public:
    void Enter() override {
        //read from json file
    }

    void Exit() override {
    }

    void Update() override {
        animStateMachine.PrintCurrentState();

        AnimationModel model = player_models[AnimationState::Walking];
        auto& playermodel = ECoordinator.GetComponent<HUGraphics::GLModel>(ECoordinator.getThiefID());
        auto& transform = ECoordinator.GetComponent<Transform>(ECoordinator.getThiefID());
        playermodel.textureID = model.texture->GetTextureID();
        playermodel.totalframe = model.totalFrames;
        playermodel.rows = model.rows;
        playermodel.columns = model.columns;
        playermodel.frametime = model.frametime;

        // Update the player's model size (transform)
        transform.scale.x = model.width;
        transform.scale.y = model.height;
        //std::cout << "WalkingState: FlipTexture: " << playermodel.flipTextureHorizontally << std::endl;

    }

    AnimationState GetState() const override { return AnimationState::Walking; }
};

// Example State: Idle
class IdleState : public State {
public:
    void Enter() override {
        //std::cout << "Entering Idle State\n";
    }

    void Exit() override {
        //std::cout << "Exiting Idle State\n";
    }

    void Update() override {
        animStateMachine.PrintCurrentState();

        AnimationModel model = player_models[AnimationState::IDLE];
        auto& playermodel = ECoordinator.GetComponent<HUGraphics::GLModel>(ECoordinator.getThiefID());
        auto& transform = ECoordinator.GetComponent<Transform>(ECoordinator.getThiefID());
        playermodel.textureID = model.texture->GetTextureID();
        playermodel.totalframe = model.totalFrames;
        playermodel.rows = model.rows;
        playermodel.columns = model.columns;
        playermodel.frametime = model.frametime;

        // Update the player's model size (transform)
        transform.scale.x = model.width;
        transform.scale.y = model.height;
    }

    AnimationState GetState() const override { return AnimationState::IDLE; }
};


class CrouchState : public State {
public:
    void Enter() override {
        //std::cout << "Entering Crouch Walk State\n";
        auto& physbody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(ECoordinator.getThiefID());
        physbody.aabb.minX -= 19.0f;
        physbody.aabb.maxX += 19.0f;
        physbody.aabb.minY += 32.3f;
    }

    void Exit() override {
        //std::cout << "Exiting Crouch Walk State\n";
        auto& physbody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(ECoordinator.getThiefID());
        physbody.aabb.minX += 19.0f;
        physbody.aabb.maxX -= 19.0f;
        physbody.aabb.minY -= 32.3f;
    }

    void Update() override {
        animStateMachine.PrintCurrentState();

        AnimationModel model = player_models[AnimationState::Crouching];
        auto& playermodel = ECoordinator.GetComponent<HUGraphics::GLModel>(ECoordinator.getThiefID());
        auto& transform = ECoordinator.GetComponent<Transform>(ECoordinator.getThiefID());

        // Save the original translate.y value before modifying it
        float originalTranslateY = transform.translate.y;

        playermodel.textureID = model.texture->GetTextureID();
        playermodel.totalframe = model.totalFrames;
        playermodel.rows = model.rows;
        playermodel.columns = model.columns;
        playermodel.frametime = model.frametime;

        // Update the player's model size (transform)
        transform.scale.x = model.width;
        transform.scale.y = model.height;

        // Modify the translate.y and apply the change
        transform.translate.y = originalTranslateY + 10;  // You can adjust this if you want some offset
    }


    AnimationState GetState() const override { return AnimationState::Crouching; }
};

// Example State: CrouchWalk
class CrouchWalkState : public State {
public:
    void Enter() override {
        //std::cout << "Entering Crouch Walk State\n";
        auto& physbody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(ECoordinator.getThiefID());
        physbody.aabb.minX -= 19.0f;
        physbody.aabb.maxX += 19.0f;
        physbody.aabb.minY += 32.3f;
    }

    void Exit() override {
        //std::cout << "Exiting Crouch Walk State\n";
        auto& physbody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(ECoordinator.getThiefID());
        physbody.aabb.minX += 19.0f;
        physbody.aabb.maxX -= 19.0f;
        physbody.aabb.minY -= 32.3f;
    }

    void Update() override {
        animStateMachine.PrintCurrentState();

        AnimationModel model = player_models[AnimationState::CrouchWalk];
        auto& playermodel = ECoordinator.GetComponent<HUGraphics::GLModel>(ECoordinator.getThiefID());
        auto& transform = ECoordinator.GetComponent<Transform>(ECoordinator.getThiefID());
        playermodel.textureID = model.texture->GetTextureID();
        playermodel.totalframe = model.totalFrames;
        playermodel.rows = model.rows;
        playermodel.columns = model.columns;
        playermodel.frametime = model.frametime;

        // Update the player's model size (transform)
        transform.scale.x = model.width;
        transform.scale.y = model.height;
    }

    AnimationState GetState() const override { return AnimationState::CrouchWalk; }
};

// Example State: Jumping
class JumpingState : public State {
public:
    void Enter() override {
        if (ECoordinator.hasThiefID()) {
            auto& physbody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(ECoordinator.getThiefID());
            physbody.aabb.minX -= 14.25f;
            physbody.aabb.maxX += 14.25f;
            physbody.aabb.minY += 7.6f;
            physbody.aabb.maxY -= 7.6f;
        }
    }

    void Exit() override {
        //std::cout << "Exiting Jumping State\n";
        if (ECoordinator.hasThiefID()) {
            auto& physbody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(ECoordinator.getThiefID());
            physbody.aabb.minX += 14.25f;
            physbody.aabb.maxX -= 14.25f;
            physbody.aabb.minY -= 7.6f;
            physbody.aabb.maxY += 7.6f;
        }
      
    }
    void Update() override {
        animStateMachine.PrintCurrentState();

        AnimationModel model = player_models[AnimationState::Jumping];
        auto& playermodel = ECoordinator.GetComponent<HUGraphics::GLModel>(ECoordinator.getThiefID());
        auto& transform = ECoordinator.GetComponent<Transform>(ECoordinator.getThiefID());
        playermodel.textureID = model.texture->GetTextureID();
        playermodel.totalframe = model.totalFrames;
        playermodel.rows = model.rows;
        playermodel.columns = model.columns;
        playermodel.frametime = model.frametime;

        // Update the player's model size (transform)
        transform.scale.x = model.width;
        transform.scale.y = model.height;
    }
    AnimationState GetState() const override { return AnimationState::Jumping; }

};

class FallingState : public State {
public:
    int playCount = 0;
    const int maxPlays = 5;

    void Enter() override {
        playCount = 0;
        if (ECoordinator.hasThiefID()) {
            auto& physbody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(ECoordinator.getThiefID());
            if (!physbody.isGrounded) {
                physbody.aabb.minX -= 14.25f;
                physbody.aabb.maxX += 14.25f;
                physbody.aabb.minY += 7.6f;
                physbody.aabb.maxY -= 7.6f;
                physbody.inertiaMass = 10.0f;
            }
        }
    }

    void Exit() override {
        playCount = 0;
        //std::cout << "Exiting falling State\n";
        if (ECoordinator.hasThiefID()) {
            auto& physbody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(ECoordinator.getThiefID());
            if (physbody.isGrounded && physbody.inertiaMass == 10.0f) {
                physbody.aabb.minX += 14.25f;
                physbody.aabb.maxX -= 14.25f;
                physbody.aabb.minY -= 7.6f;
                physbody.aabb.maxY += 7.6f;
                physbody.inertiaMass = 0.0f;
            }
        }

    }

    void Update() override {
        //std::cout << "Entering falling State\n";

        animStateMachine.PrintCurrentState();

        AnimationModel model = player_models[AnimationState::Falling];
        auto& playermodel = ECoordinator.GetComponent<HUGraphics::GLModel>(ECoordinator.getThiefID());
        auto& transform = ECoordinator.GetComponent<Transform>(ECoordinator.getThiefID());
        
        playermodel.textureID = model.texture->GetTextureID();
        playermodel.totalframe = model.totalFrames;
        playermodel.rows = model.rows;
        playermodel.columns = model.columns;
        playermodel.frametime = model.frametime;

        // Update the player's model size (transform)
        transform.scale.x = model.width;
        transform.scale.y = model.height;

    }

    AnimationState GetState() const override { return AnimationState::Falling; }
};



// AnimationStateMachine constructor
AnimationStateMachine::AnimationStateMachine() : currentState(nullptr) {

    AddState(std::make_unique<IdleState>());
    AddState(std::make_unique<WalkingState>());
    AddState(std::make_unique<JumpingState>());
    AddState(std::make_unique<CrouchState>());
    AddState(std::make_unique<CrouchWalkState>());
    AddState(std::make_unique<FallingState>());
    // Initialize the current state to idle
    TransitionTo(AnimationState::IDLE);
}

// Initialize animation models from the JSON
void InitializeAnimationModels() {
    LoadPlayerAnimationsFromJSON("Json/PlayerAnimation.json");
}


void AnimationStateMachine::PrintCurrentState() const {
    if (currentState != nullptr) {
        // Assuming that each state class has a string representation of the state name
        //std::cout << "Current State: " << ToString(currentState->GetState()) << std::endl;
       // std::cout << "Current State: " << ToString(currentState->GetState()) << std::endl;
    }
    else {
        //std::cout << "Current State is undefined." << std::endl;
    }
}


AnimationState AnimationStateFromString(const std::string& stateName) {
    if (stateName == "Walking") return AnimationState::Walking;
    if (stateName == "IDLE") return AnimationState::IDLE;
    if (stateName == "Jumping") return AnimationState::Jumping;
    if (stateName == "Crouching") return AnimationState::Crouching;
    if (stateName == "CrouchWalk") return AnimationState::CrouchWalk;
    if (stateName == "Falling") return AnimationState::Falling;


    return AnimationState::Undefined; // Default case if the string does not match
}

// Load player animations from JSON configuration
void LoadPlayerAnimationsFromJSON(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open animation config file: " << filePath << std::endl;
        return;
    }

    nlohmann::json jsonData;
    try {
        file >> jsonData;
    }
    catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Error parsing JSON file: " << e.what() << std::endl;
        return;
    }

    if (!jsonData.contains("animations")) {
        std::cerr << "Invalid JSON format: Missing 'animations' key" << std::endl;
        return;
    }

    for (const auto& [stateName, config] : jsonData["animations"].items()) {
        AnimationState state;
        try {
            state = AnimationStateFromString(stateName);
        }
        catch (const std::exception& e) {
            std::cerr << "Invalid animation state name: " << stateName << std::endl;
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }

        AnimationModel model;

        try {
            if (!config.contains("texture") || !config.contains("totalFrames") ||
                !config.contains("rows") || !config.contains("columns") || !config.contains("frametime")) {
                continue;
            }

            model.texture = TextureLibrary.GetAssets(config["texture"].get<std::string>());
            model.totalFrames = config["totalFrames"].get<int>();
            model.rows = config["rows"].get<int>();
            model.columns = config["columns"].get<int>();
            model.frametime = config["frametime"].get<float>();
            model.width = config["width"].get<float>();
            model.height = config["height"].get<float>();

            player_models[state] = model;

        }
        catch (const std::exception& e) {
            std::cerr << "Error loading animation data for state: " << stateName
                << ". Exception: " << e.what() << std::endl;
        }
    }
}

// A function to update the animation state machine based on inputs
// A function to update the animation state machine based on inputs
void UpdateAnimationStateMachine() {
    if (InputSystem->Stage == Playing ||
        InputSystem->Stage == Playing1 ||
        InputSystem->Stage == Playing2 ||
        InputSystem->Stage == Playing3) {

    
    static bool wasFalling = false;  // Track if the player was in the air
    static bool forceCrouch = false;

    if (health <= 0 || wingame) {
        animStateMachine.TransitionTo(AnimationState::IDLE); // or a Dead state if it exists
    }

    std::vector<EntityID> mEntities = ECoordinator.GetAllEntities();
    if (!mEntities.empty() && ECoordinator.hasThiefID()) {
        auto ID = ECoordinator.getThiefID();
        bool isWalking = (InputSystem->IsKeyPress(GLFW_KEY_A) || InputSystem->IsKeyPress(GLFW_KEY_D));
        bool isCrouching = false;
        auto& body = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(ID);
        if (body.isGrounded) {
            isCrouching = InputSystem->IsKeyPress(GLFW_KEY_S);
        }
        bool isGrounded = body.isGrounded;

        // Determine player direction
        if (body.velocity.x < 0) {
            isFacingRight = false;  // Moving left
        }
        else if (body.velocity.x > 0) {
            isFacingRight = true;   // Moving right
        }

        if (isCrouching || forceCrouch) {
            forceCrouch = true;
            bool canStand = true;

            float firstTimeOfCollision;

            PhysicsSystem::PhysicsBody newTempbody = body;
            newTempbody.aabb.minX += 19.0f;
            newTempbody.aabb.maxX -= 19.0f;
            newTempbody.aabb.minY -= 30.3f;
            newTempbody.aabb.maxY -= 2.0f;
            
            for (auto entity : ECoordinator.GetAllEntities()) {
                if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {

                    auto& otherBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);
                    if (entity != ID && otherBody.category == "Wall") {
                        if (CollisionIntersection_RectRect(newTempbody.aabb, newTempbody.velocity.x, newTempbody.velocity.y,
                            otherBody.aabb, otherBody.velocity.x, otherBody.velocity.y,
                            firstTimeOfCollision)) {
                            canStand = false;
                            break;
                        }
                    }
                }
            }

            if (canStand) {
                forceCrouch = false;
                if (animStateMachine.GetCurrentState()->GetState() != AnimationState::Walking) {
                    animStateMachine.TransitionTo(AnimationState::Walking);
                }
            }
        }

        // Play landing sound when transitioning from falling to grounded
        if (wasFalling && isGrounded && body.velocity.y == 0.0f) {
            PlayRandomSound(landsound, 5, currentlandSound, 0.5f);
        }

        // Handle animation state transitions
        if (health <= 0 || wingame) {
            animStateMachine.TransitionTo(AnimationState::IDLE);
        }
        else if (!isGrounded && body.velocity.y > 0) { // Falling state
            if (animStateMachine.GetCurrentState()->GetState() != AnimationState::Falling) {
                animStateMachine.TransitionTo(AnimationState::Falling);
            }
            wasFalling = true;  // Mark that player is falling
        }
        else if (isGrounded && body.velocity.y > 0) { // Falling state from not jumping
            if (animStateMachine.GetCurrentState()->GetState() != AnimationState::Falling) {
                animStateMachine.TransitionTo(AnimationState::Falling);
            }
            wasFalling = true;  // Mark that player is falling
        }
        else if (!isGrounded && body.velocity.y < 0) { // Jumping state
            if (animStateMachine.GetCurrentState()->GetState() != AnimationState::Jumping) {
                animStateMachine.TransitionTo(AnimationState::Jumping);
            }
            wasFalling = false;
        }
        else if ((isCrouching || body.friction == 100 || forceCrouch) && isWalking) {
            if (animStateMachine.GetCurrentState()->GetState() != AnimationState::CrouchWalk) {
                animStateMachine.TransitionTo(AnimationState::CrouchWalk);
            }
            wasFalling = false;
        }
        else if (isWalking) {
            if (animStateMachine.GetCurrentState()->GetState() != AnimationState::Walking) {
                animStateMachine.TransitionTo(AnimationState::Walking);
            }
            wasFalling = false;
        }
        else if (isCrouching || body.friction == 100 || forceCrouch) {
            if (animStateMachine.GetCurrentState()->GetState() != AnimationState::Crouching) {
                animStateMachine.TransitionTo(AnimationState::Crouching);
            }
            wasFalling = false;
        }
        else {
            if (animStateMachine.GetCurrentState()->GetState() != AnimationState::IDLE) {
                animStateMachine.TransitionTo(AnimationState::IDLE);
            }
            wasFalling = false;
        }

        // Handle texture flipping
        auto& model = ECoordinator.GetComponent<HUGraphics::GLModel>(ID);
        bool shouldFlip = !isFacingRight;
        if (model.flipTextureHorizontally != shouldFlip) {
            model.currentFrame = 0;
            model.flipTextureHorizontally = shouldFlip;
        }
       

        // Update the current animation state
        animStateMachine.UpdateState();
    }
    }
}

