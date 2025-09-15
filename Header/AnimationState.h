/**
 * @file AnimationStateMachine.h
 * @brief Defines an animation state management system for player animations.
 *
 * This file provides the `AnimationStateMachine` class, which manages different animation states
 * and transitions for a player character. It defines the `AnimationState` enum, the `AnimationModel` struct,
 * and an abstract `State` class, which serves as the base for various animation states.
 * The system allows dynamic state transitions and updates to ensure smooth animations.
 *
 * Author: Rui Jie (100%)
 */


#ifndef ANIMATIONSTATEMACHINE_H
#define ANIMATIONSTATEMACHINE_H

#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include "Graphics.h"
#include "AssetsManager.h"
#include "Physics.h"
#include "JSONSerialization.h"
#include "GlobalVariables.h"




/**
 * @brief Enum for different player states
 */
enum class AnimationState {
    IDLE,
    Walking,
    Jumping,
    Crouching,
    CrouchWalk,
    Falling,
    Undefined
};

/**
 * @brief A struct that holds the player's animation model
 */
struct AnimationModel {
    std::shared_ptr<Texture> texture = nullptr;
    int totalFrames = 0;
    int rows = 0;
    int columns = 0;
    float frametime = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};


/**
 * @brief Abstract base class for animation states.
 *
 * This class defines the interface for all animation states.
 * Each specific animation state (e.g., `WalkingState`, `JumpingState`) must inherit from this class.
 */
class State {
public:
    virtual ~State() = default;
    virtual void Enter() = 0;
    virtual void Exit() = 0;
    virtual void Update() = 0;
    virtual AnimationState GetState() const = 0;
};


/**
 * @brief StateMachine class to manage the transitions between states.
 */
class AnimationStateMachine {
private:
    State* currentState;
    std::unordered_map<AnimationState, std::unique_ptr<State>> states;

public:
    AnimationStateMachine();

    void AddState(std::unique_ptr<State> state);
    void PrintCurrentState() const;
    void TransitionTo(AnimationState newState);
    void UpdateState();
    State* GetCurrentState() const { return currentState; }

};

// Function to load player animations from a JSON file
void LoadPlayerAnimationsFromJSON(const std::string& filePath);
void InitializeAnimationModels();
void UpdateAnimationStateMachine();

#endif // ANIMATIONSTATEMACHINE_H
