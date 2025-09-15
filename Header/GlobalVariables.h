/**
 * @file GlobalVariables.h
 * @brief Definition of global variables and objects used across the game engine.
 *
 * This file contains global variables, objects, and constants that are shared throughout the game engine.
 * These include core systems, state variables, libraries, and other shared resources. The variables are
 * primarily used for managing engine-wide configurations, assets, and runtime states.
 *
 * Key Features:
 * - **Core Systems**:
 *   - `ECSCoordinator`: Centralized coordinator for managing entities and components.
 *   - `CoreEngine::InputSystem*`: Pointer to the input system for managing player inputs.
 *   - `CAudioEngine*`: Pointer to the audio engine for sound playback.
 *   - `FontSystem*`: Pointer to the font system for text rendering.
 * - **Game State Management**:
 *   - Variables such as `health`, `cameraActive`, `gameStateObject`, and `isPaused` track the runtime state
 *     of the game and gameplay mechanics.
 * - **Asset Management**:
 *   - Asset libraries (`PrefabLibrary`, `TextureLibrary`, `AudioLibrary`, and `FontLibrary`) store assets
 *     like prefabs, textures, audio files, and fonts.
 * - **Screen Configuration**:
 *   - Variables such as `screen_width`, `screen_height`, and `fullscreen_bool` store screen properties.
 * - **Other Global Utilities**:
 *   - `Camera2D`: Handles 2D camera operations for rendering.
 *   - `ExitButton`: Represents the exit button state.
 *   - `SystemTimeOutput`: Stores system time as a string for logging or display.
 *
 * Global Variables:
 * - Variables like `gDroppedFiles` (stores dropped file paths) and `allowThiefMoveIfTrue` are used for gameplay
 *   and interaction handling.
 *
 * Author Contributions:
 * - Che Ee (50%)
 * - Rest of Team (50%)
 */

#pragma once
#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H
#include "SystemsManager.h"
#include "AudioEngine.h"





#include "FontSystem.h"

extern FontSystem* fontSystem;


#include "AssetsManager.h"
#include "Coordinator.h"
#include "Core.h"
#include "InputSystem.h"
#include "ImguiManager.h"
#include "CommonIncludes.h"

#include "AnimationState.h"



class ECSCoordinator;
class HustlersEngine;

extern ECSCoordinator ECoordinator;
extern HustlersEngine* TateEngine;

extern CoreEngine::InputSystem* InputSystem;
//extern SystemManager systemManager;

extern AssetLibrary<Prefab> PrefabLibrary;
extern AssetLibrary<Texture> TextureLibrary;
extern AssetLibrary<Audio> AudioLibrary;
extern AssetLibrary<Font> FontLibrary;


extern int screen_width;

extern int screen_height;

extern bool fullscreen_bool;

extern bool iswalking;

extern bool isPaused;

extern bool mouseInTexture;

extern bool allowThiefMoveIfTrue;

extern bool showImgui;

extern std::vector<std::string> gDroppedFiles;

extern std::string SystemTimeOutput;

extern Camera2D cameraObj;

class CAudioEngine;

extern CAudioEngine* audioEngine;

extern EntityID pButtonID;

extern float numberofsteps;
extern int gameStateObject;


extern bool cameraActive;
extern int gravity;
extern int health;
extern int Object_picked;

//extern EntityID deleteSceneID;

extern std::vector<std::pair<EntityID,int>> sceneVector;

extern std::vector<std::string> categories;

extern std::unordered_map<std::string, AnimationData> animationPresets;

extern AnimationStateMachine animStateMachine;

extern glm::vec2 startingPos;

extern  TransitionStateManager transitionManager;

extern const int timeLimit;

extern EntityID getBackToVanImage;

extern std::pair<float, float> vanLocation;

extern Timer timerObj;


extern std::unordered_map<std::string, EntityID> entityNameMap;

extern EntityID timerID;
// Store the mapping of health state strings to their EntityIDs
extern std::unordered_map<std::string, EntityID> healthSplashScreensMap;

extern int winStatus;


extern bool wingame;

extern int totalObjects;

extern size_t cutsceneIncrement;
extern bool isSoundPlayed;
extern float masterVolume;
extern float sfxVolume;
extern float musicVolume;

extern float SceneTimer;


extern std::atomic<bool> windowFocused;

#endif // ! GLOBAL_VARIABLES_H