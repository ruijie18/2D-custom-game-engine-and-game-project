/**
 * @file GlobalVariables.cpp
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


#include "GlobalVariables.h"
#include "AudioEngine.h"
#include "Render.h"
#include <limits>
#include "Core.h"

HustlersEngine* TateEngine = nullptr;

ECSCoordinator ECoordinator;

bool cameraActive = false;

CoreEngine::InputSystem* InputSystem = nullptr;

int screen_width;

int screen_height;

bool fullscreen_bool;

bool showImgui;

bool isPaused;

bool mouseInTexture = true;

bool allowThiefMoveIfTrue;

int gravity = 837;

int health=2;

int Object_picked;

std::string SystemTimeOutput; // Declare output string to hold system times

CAudioEngine* audioEngine = nullptr;

FontSystem* fontSystem = nullptr;

std::vector<std::string> gDroppedFiles;

AssetLibrary<Prefab> PrefabLibrary;
AssetLibrary<Texture> TextureLibrary;
AssetLibrary<Audio> AudioLibrary;
AssetLibrary<Font> FontLibrary;

Camera2D cameraObj;



EntityID pButtonID;

float numberofsteps;

 int gameStateObject=MainMenu;

 //EntityID deleteSceneID= std::numeric_limits<uint32_t>::max();


 std::vector<std::pair<EntityID,int>> sceneVector;

 std::vector<std::string> categories = {};

 std::unordered_map<std::string, AnimationData> animationPresets;

AnimationStateMachine animStateMachine;

 glm::vec2 startingPos = { 94.49870300292969,849.8424682617188 };

 const int timeLimit = 180;

 TransitionStateManager transitionManager;

 EntityID getBackToVanImage;

 std::pair<float, float>vanLocation = {};

 Timer timerObj;

 EntityID timerID;

 std::unordered_map<std::string, EntityID> healthSplashScreensMap;
 std::unordered_map<std::string, EntityID> entityNameMap;

 int winStatus=0;

 bool wingame = false;

 int totalObjects = 0;

  size_t cutsceneIncrement=0;

  bool isSoundPlayed = false;
  float masterVolume = 1.0f;
  float sfxVolume = 1.0f;
  float musicVolume = 1.0f;

  float SceneTimer = 0.f;

  std::atomic<bool> windowFocused = true;