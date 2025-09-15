/**
 * @file GameLogic.h
 * @brief Implementation of core game logic, including initialization, updates, and management of game objects and states.
 *
 * This file defines the core gameplay logic for a game engine, including functions to initialize the game,
 * update game states, manage timers, and handle interactions between game entities. The file also provides
 * utility functions for managing game stages, loading assets, and resetting the game state.
 *
 * Key Features:
 * - **Timer Management**:
 *   - `StartTimer`: Initializes a countdown timer.
 *   - `UpdateTimer`: Updates the timer based on elapsed time and manages the countdown logic.
 * - **Game Initialization**:
 *   - `InitGame`: Sets up the engine, registers components and systems, and loads initial game assets.
 *   - `InitGameObjects`: Loads assets and creates objects for the main menu and game stages.
 * - **Stage Management**:
 *   - `CreateObjectsForStage`: Dynamically loads entities based on the current stage (e.g., Main Menu, Gameplay, Pause Menu).
 * - **Gameplay Updates**:
 *   - `updateGame`: Updates game states, processes input, and handles entity interactions like health and timer updates.
 * - **Entity and Asset Management**:
 *   - Manages entities dynamically, including their creation, destruction, and resetting.
 *   - Loads assets such as textures, audio, and fonts to support gameplay and UI elements.
 * - **Health and Reset Logic**:
 *   - Implements health-based mechanics and resets the game state when necessary.
 *
 * Utility Functions:
 * - `getRandomFloat`: Generates a random float within a specified range.
 * - `FreeGame`: Placeholder function for cleaning up resources.
 * - `ResetGame`: Resets game variables, timer, and entities to their initial state.
 *
 * Key Systems and Interactions:
 * - Timer and health are integrated into the gameplay loop, with visual feedback for health status.
 * - Stage-based entity management allows for dynamic changes between menus, gameplay, and pause states.
 * - Integration with external libraries for rendering (`Graphics.h`), physics (`Physics.h`), and configuration loading.
 *
 * Author: Che Ee (50%)
 * Co-Author: Rui Jie (30%)
 * co-Author: Jason(20%)
 */

#pragma once
#include "GlobalVariables.h"
#include "ListOfComponents.h"
#include "CommonIncludes.h"
#include "EntityManager.h"
#include "Coordinator.h"
#include "JSONSerialization.h"
#include "AnimationState.h"


void ResetGame();

//#include "Render.h"
void InitGameObjects();

void PrintGameObjectsData();

void CreateObjectsForStage(int stage);

void InitGame();

void updateGame(GLFWwindow* window, double deltaTime);

void freeGame();

void updatelasers(float deltaTime); 

void PlayProximitySound(EntityID laserID);

void resetGame();

