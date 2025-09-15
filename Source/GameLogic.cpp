/**
 * @file GameLogic.cpp
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
 * Author: Che Ee (20%)
 * Co-Author: Rui Jie (20%)
 * co-Author: Jason(20%)
 * co-author:jarren (20%)
 * co-author:jason(20%)
 *
 */

#include "ParticleSystem.h"
#include "GameLogic.h"
#include "Render.h"
#include "Physics.h"
#include "Graphics.h"
#include "ConfigLoading.h"
#include <random>
#include <chrono>
#include "GlobalVariables.h"
#include <filesystem>
#include "ButtonComponent.h"

 //for timer
static bool reset = false;
bool circleCreate = false;
std::vector<EntityID> circleEntities;
std::vector<EntityID> laserEntities;

// Create a random device and a Mersenne Twister engine
std::random_device rd;  // Seed for random number engine
std::mt19937 gen(rd()); // Mersenne Twister engine


//static count object that is required to pick up
//static int totalObjects=0;
PhysicsTemp::DragInfo DragInfo;
PhysicsSystem physicssystem;

static bool hasPlayedVanHonk = false;
static bool hasWind = false;
std::unordered_map<EntityID, bool> laserSoundPlaying; // Track per-laser
static float fPan = 0.0;


// Function to generate a random float between min and max
float getRandomFloat(float min, float max) {
    std::uniform_real_distribution<> dis(min, max);
    return static_cast<float>(dis(gen));
}

// Function to initialize game objects, e.g., loading assets
void InitGameObjects() {
    //LoadGameObjectsFromJson("GameObjects.json");

    TextureLibrary.LoadAssets("./Assets/Textures");
    AudioLibrary.LoadAssets("./Assets/Audio");
    FontLibrary.LoadAssets("./Assets/Fonts");

    audioEngine->ToggleSoundLooping("BGM.ogg");
    audioEngine->ToggleSoundLooping("LEVEL_BGM.ogg");
    audioEngine->ToggleSoundLooping("WIND-SOFTER.ogg");

    LoadAnimationPresets("Json/spritesheet_ref.json");
    CreateObjectsForStage(splashscreen);
}

// Helper function to create objects for each stage
void CreateObjectsForStage(int stage) {

    ECoordinator.resetThiefID();
    if ((stage != Pause) && (stage != HowToPlay2) && (stage != confirmQuit2)) {
        ECoordinator.DestroyAllGameObjects(); // Clear previous entities
    }
    if (stage == MainMenu) {

        LoadGameObjectsFromJson("Json/Main_Menu.json");
        if (audioEngine->isPlaying("LEVEL_BGM.ogg")) {
            audioEngine->StopSound("LEVEL_BGM.ogg");
        }
        if (audioEngine->isPlaying("NearLaser.ogg")) {
            audioEngine->SetSoundVolume("NearLaser.ogg", 0.0f);
        }
        if (!audioEngine->isPlaying("BGM.ogg")) {
            audioEngine->PlaySound("BGM.ogg", 0, 0.15f * musicVolume);
        }
    }
    else if (stage == Playing) {
        if (CoreEngine::InputSystem::SavedStage != Pause)
            timerObj.changeDuration(240);
        if (audioEngine->isPlaying("BGM.ogg")) {
            audioEngine->StopSound("BGM.ogg");
        }
        if (!audioEngine->isPlaying("LEVEL_BGM.ogg")) {
            audioEngine->PlaySound("LEVEL_BGM.ogg", 0, 0.15f * musicVolume, 100);
        }
        if (CoreEngine::InputSystem::SavedStage == Pause) {
            LoadGameObjectsFromJson_doc("tempasas.json");
            CoreEngine::InputSystem::SavedStage = MainMenu;
            //timerObj.Resume();
        }
        else {
            timerObj.Resume();
            totalObjects = 0;
            LoadGameObjectsFromJson("Json/Category.json");
            LoadGameObjectsFromJson("Json/GameObjects.json");
            auto Entities = ECoordinator.GetAllEntities();
            for (const auto& entity : Entities) {
                //std::
                // << "Entity ID: " << entity << ", Name: " << name.name << '\n';
                if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {
                    PhysicsSystem::PhysicsBody& physBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);
                    if (physBody.category == "Object") {
                        totalObjects += 1;
                    }
                }
            }
        }
    }
    else if (stage == Playing1) {
        if (CoreEngine::InputSystem::SavedStage != Pause)
            timerObj.changeDuration(120);
        if (audioEngine->isPlaying("BGM.ogg")) {
            audioEngine->StopSound("BGM.ogg");
        }
        if (!audioEngine->isPlaying("LEVEL_BGM.ogg")) {
            audioEngine->PlaySound("LEVEL_BGM.ogg", 0, 0.15f * musicVolume, 40);
        }

        if (CoreEngine::InputSystem::SavedStage == Pause) {
            LoadGameObjectsFromJson_doc("tempasas.json");
            CoreEngine::InputSystem::SavedStage = MainMenu;
        }
        else {
            timerObj.Resume();
            totalObjects = 0;
            LoadGameObjectsFromJson("Json/Category.json");
            LoadGameObjectsFromJson("Json/Level1.json");
            auto Entities = ECoordinator.GetAllEntities();
            for (const auto& entity : Entities) {
                //std::
                // << "Entity ID: " << entity << ", Name: " << name.name << '\n';
                if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {
                    PhysicsSystem::PhysicsBody& physBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);
                    if (physBody.category == "Object") {
                        totalObjects += 1;
                    }
                }
            }
        }
    }
    else if (stage == Playing2) {
        if (CoreEngine::InputSystem::SavedStage != Pause)
            timerObj.changeDuration(240);
        if (audioEngine->isPlaying("BGM.ogg")) {
            audioEngine->StopSound("BGM.ogg");
        }

        if (!audioEngine->isPlaying("LEVEL_BGM.ogg")) {
            audioEngine->PlaySound("LEVEL_BGM.ogg", 0, 0.15f * musicVolume, 40);
        }

        if (CoreEngine::InputSystem::SavedStage == Pause) {
            LoadGameObjectsFromJson_doc("tempasas.json");
            CoreEngine::InputSystem::SavedStage = MainMenu;

        }
        else {
            timerObj.Resume();
            totalObjects = 0;
            LoadGameObjectsFromJson("Json/Category.json");
            LoadGameObjectsFromJson("Json/Level3.json");
            auto Entities = ECoordinator.GetAllEntities();
            for (const auto& entity : Entities) {
                //std::
                // << "Entity ID: " << entity << ", Name: " << name.name << '\n';
                if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {
                    PhysicsSystem::PhysicsBody& physBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);

                    if (physBody.category == "Object") {
                        totalObjects += 1;
                    }
                }
            }
        }
    }
    else if (stage == Playing3) {
        if (CoreEngine::InputSystem::SavedStage != Pause)
            timerObj.changeDuration(180);
        if (audioEngine->isPlaying("BGM.ogg")) {
            audioEngine->StopSound("BGM.ogg");
        }
        if (!audioEngine->isPlaying("LEVEL_BGM.ogg")) {
            audioEngine->PlaySound("LEVEL_BGM.ogg", 0, 0.15f * musicVolume, 40);
        }

        if (CoreEngine::InputSystem::SavedStage == Pause) {
            LoadGameObjectsFromJson_doc("tempasas.json");
            CoreEngine::InputSystem::SavedStage = MainMenu;
        }
        else {
            timerObj.Resume();
            totalObjects = 0;
            LoadGameObjectsFromJson("Json/Category.json");
            LoadGameObjectsFromJson("Json/Level2.json");
            auto Entities = ECoordinator.GetAllEntities();
            for (const auto& entity : Entities) {
                //std::
                // << "Entity ID: " << entity << ", Name: " << name.name << '\n';
                if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {
                    PhysicsSystem::PhysicsBody& physBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);
                    if (physBody.category == "Object") {
                        totalObjects += 1;
                    }
                }
            }
        }
    }
    else if (stage == Pause) {
        LoadGameObjectsFromJson("Json/PauseMenu.json");
    }
    else if ((stage == HowToPlay) || (stage == HowToPlay2)) {
        LoadGameObjectsFromJson("Json/HowToPlay.json");
    }

    else if (stage == confirmQuit || (stage == confirmQuit2)) {
        LoadGameObjectsFromJson("Json/ConfirmQuit.json");
    }

    else if (stage == Lose) {
        LoadGameObjectsFromJson("Json/LoseMenu.json");
    }

    else if (stage == starRating) {
        LoadGameObjectsFromJson("Json/StarRating.json");
    }

    else if (stage == LevelSelect) {
        LoadGameObjectsFromJson("Json/LevelSelect.json");
    }
    else if (stage == splashscreen) {
        LoadGameObjectsFromJson("Json/splashscreen.json");
    }
    else if (stage == Credit) {
        LoadGameObjectsFromJson("Json/Credit.json");
    }

    else if (stage == Settings) {
        LoadGameObjectsFromJson("Json/Volume.json");
    }



    //currently the json loading is load all images .

    //multiple ways to do this: 1 is to put 6 json files with 6 images and slowly one by one transition but that will take alot of if statements and logic handling its too troublesome

    //other way is to in loadgamneobjects function u manually set custom logic like load one by one image .
    else if (stage == cutScene) {
        sceneVector.clear();
        LoadGameObjectsFromJson("Json/cutScene.json");
    }

    else if (stage == gameWon) {
        LoadGameObjectsFromJson("Json/endScene.json");
    }

    auto Entities = ECoordinator.GetAllEntities();
    for (const auto& entity : Entities) {
        if (ECoordinator.HasComponent<Name>(entity)) {
            auto& name = ECoordinator.GetComponent<Name>(entity);

            if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {
                auto& phy = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);
                if (phy.category == "Laser Module") {
                    entityNameMap[name.name] = entity;
                }
            }
        }
    }
}

// Function to initialize game
void InitGame() {
    ECoordinator.Init();

    // Register components
    ECoordinator.RegisterComponent<Transform>();
    ECoordinator.RegisterComponent<HUGraphics::GLModel>();
    ECoordinator.RegisterComponent<PhysicsSystem::PhysicsBody>();

    ECoordinator.RegisterComponent<RenderLayer>();
    ECoordinator.RegisterComponent<Name>();
    ECoordinator.RegisterComponent<PhysicsSystem::Switch>();
    ECoordinator.RegisterComponent<PhysicsSystem::AutoDoor>();
    ECoordinator.RegisterComponent<LaserComponent>();
    ECoordinator.RegisterComponent<ButtonComponent>();

    ECoordinator.RegisterComponent<ParticleComponent>();

    //Register systems
    ECoordinator.RegisterSystem<RenderSystem>();
    ECoordinator.RegisterSystem<PhysicsSystem>();
    ECoordinator.RegisterSystem<HUGraphics>();

    ECoordinator.RegisterSystem<ParticleSystem>();


    ECoordinator.InitSystems();

    InitGameObjects(); // Load assets
    audioEngine->SetMasterVolume(1.0);

    InitializeAnimationModels();
    Object_picked = 0;



}


// Function to update game state
void updateGame(GLFWwindow*, double deltaTime) {
    if (windowFocused) {
        (void)deltaTime;

        static float splashScreenTimer = 0.0f;
        static bool splashScreenVisible = true;

        if (CoreEngine::InputSystem::Stage == splashscreen) {
            splashScreenTimer += static_cast<float>(deltaTime);

            const float totalDuration = 6.0f;
            const float fadeOutStart0 = 0.7f;
            const float fadeOutStart1 = 4.0f;
            const float fadeOutDuration0 = 1.5f;
            const float fadeOutDuration1 = 1.5f;

            // Transition to Main Menu
            if (splashScreenTimer >= totalDuration) {
                CoreEngine::InputSystem::Stage = MainMenu;
                ECoordinator.DestroyAllUIObjects();
                CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                splashScreenVisible = false;
                splashScreenTimer = 0.0f;
                return;
            }

            if (splashScreenVisible) {
                // Initialize alphas to 0
                if (ECoordinator.HasComponent<HUGraphics::GLModel>(0)) {
                    auto& mdl0 = ECoordinator.GetComponent<HUGraphics::GLModel>(0);
                    mdl0.alpha = 0.0f;
                }
                if (ECoordinator.HasComponent<HUGraphics::GLModel>(1)) {
                    auto& mdl1 = ECoordinator.GetComponent<HUGraphics::GLModel>(1);
                    mdl1.alpha = 0.0f;
                }

                // --- Entity 0 ---
                if (ECoordinator.HasComponent<HUGraphics::GLModel>(0)) {
                    auto& mdl0 = ECoordinator.GetComponent<HUGraphics::GLModel>(0);

                    if (splashScreenTimer < fadeOutStart0) {
                        mdl0.alpha = 1.0f;
                    }
                    else if (splashScreenTimer < fadeOutStart0 + fadeOutDuration0) {
                        float fadeProgress = (splashScreenTimer - fadeOutStart0) / fadeOutDuration0;
                        mdl0.alpha = 1.0f - std::clamp(fadeProgress, 0.0f, 1.0f);
                    }
                    else {
                        mdl0.alpha = 0.0f;
                    }
                }

                // --- Entity 1 ---
                if (ECoordinator.HasComponent<HUGraphics::GLModel>(1)) {
                    auto& mdl1 = ECoordinator.GetComponent<HUGraphics::GLModel>(1);

                    if (splashScreenTimer >= fadeOutStart0 + fadeOutDuration0 && splashScreenTimer < fadeOutStart1) {
                        // Entity 1 shows fully after Entity 0 fades out
                        mdl1.alpha = 1.0f;
                    }
                    else if (splashScreenTimer >= fadeOutStart1) {
                        float fadeProgress = (splashScreenTimer - fadeOutStart1) / fadeOutDuration1;
                        mdl1.alpha = 1.0f - std::clamp(fadeProgress, 0.0f, 1.0f);
                    }
                }
            }

            return; // Still in splash screen
        }





        if (CoreEngine::InputSystem::Stage == cutScene || CoreEngine::InputSystem::Stage == gameWon)
            if (windowFocused)
                SceneTimer += float(deltaTime);

        if (!isPaused) {

            for (int step = 0; step < numberofsteps; ++step) {

                UpdateAnimationStateMachine();


                updatelasers(float(deltaTime));
            }


        }
        if (CoreEngine::InputSystem::Stage == LevelSelect) {
            CoreEngine::InputSystem::SavedStage = LevelSelect;
        }
        int remainderTime = timerObj.GetTimeRemaining();

        int minutes = remainderTime / 60;

        int seconds = remainderTime % 60;


        // Format timer text
        std::ostringstream timerText;
        timerText << std::setw(2) << std::setfill('0') << minutes << ":"
            << std::setw(2) << std::setfill('0') << seconds;
        //std::cout << "\nfPan: " << fPan;
        //Update Timer for gameplay

        PhysicsSystem::PhysicsBody body;
        laserEntities.clear();
        auto Entities = ECoordinator.GetAllEntities();

        for (const auto& entity : Entities) {
            //check for name Timer to get text entity
            if (!ECoordinator.HasComponent<Name>(entity)) {
                continue;
            }
            auto& name = ECoordinator.GetComponent<Name>(entity);
            auto& mdl = ECoordinator.GetComponent<HUGraphics::GLModel>(entity);

            if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {
                body = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);
                if (body.category == "Laser") {
                    laserEntities.push_back(entity);
                }
            }


            if (name.name == "Timer") {
                //auto& mdl = ECoordinator.GetComponent<HUGraphics::GLModel>(entity);

                if (mdl.textureID != 0) {
                    glBindTexture(GL_TEXTURE_2D, 0);  // Unbind any bound texture
                    glDeleteTextures(1, &mdl.textureID);
                    GLenum err = glGetError();
                    if (err != GL_NO_ERROR) {
                    }
                    mdl.textureID = 0;
                }
                //band-aid
                mdl.alpha = 1.0f;
                mdl.text = timerText.str();
                GLuint updated_text = fontSystem->RenderTextToTexture(mdl.text, mdl.fontScale, mdl.color, mdl.fontName, mdl.fontSize);

                mdl.textureID = updated_text;
                continue;
            }
            else if (name.name == "ObjectCollected") {

                if (mdl.textureID != 0) {
                    glBindTexture(GL_TEXTURE_2D, 0);  // Unbind any bound texture
                    glDeleteTextures(1, &mdl.textureID);
                    GLenum err = glGetError();
                    if (err != GL_NO_ERROR) {
                        //std::cout << "OpenGL Error after glDeleteTextures: " << err << std::endl;
                    }
                    mdl.textureID = 0;
                }
                mdl.text = std::to_string(Object_picked) + " / " + std::to_string(totalObjects);
                GLuint updated_text = fontSystem->RenderTextToTexture(mdl.text, mdl.fontScale, mdl.color, mdl.fontName, mdl.fontSize);

                mdl.textureID = updated_text;
                continue;
            }

            else if (name.name == "heartLeft") {
                if (mdl.textureID != 0) {
                    glBindTexture(GL_TEXTURE_2D, 0);  // Unbind any bound texture
                    glDeleteTextures(1, &mdl.textureID);
                    GLenum err = glGetError();
                    if (err != GL_NO_ERROR) {
                        //std::cout << "OpenGL Error after glDeleteTextures: " << err << std::endl;
                    }
                    mdl.textureID = 0;
                }
                mdl.text = std::to_string(health) + " / 2";
                GLuint updated_text = fontSystem->RenderTextToTexture(mdl.text, mdl.fontScale, mdl.color, mdl.fontName, mdl.fontSize);

                mdl.textureID = updated_text;
                continue;
            }
            else if (name.name == "azer10") {
                auto& laser = ECoordinator.GetComponent<LaserComponent>(entity);
                if (!laser.turnedOn) {
                    laser.isActive = false;
                }

            }


            if (name.name == "Heart1" && health == 0) {
                mdl.color.r = 0;
                mdl.color.g = 0;
                mdl.color.b = 0;
            }

            if (name.name == "Heart2" && health == 1) {
                mdl.color.r = 0;
                mdl.color.g = 0;
                mdl.color.b = 0;
            }

            if (wingame) {
                timerObj.Pause();
                audioEngine->PlaySound("Win Sting v1.ogg", 0, 0.15f * musicVolume, 34);
                audioEngine->StopSound("LEVEL_BGM.ogg");
                UpdateAnimationStateMachine();
                wingame = false;
            }

            if (health <= 0) {
                UpdateAnimationStateMachine();
                //reset = true;
                sceneVector.clear();
                audioEngine->SetSoundVolume("NearLaser.ogg", 0.0f);
                audioEngine->StopSound("LEVEL_BGM.ogg");
                audioEngine->PlaySound("Lose Sting v1 1.ogg", 0, 0.15f * musicVolume, 33);

                //logic for saving the level that the player is lost here
                CoreEngine::InputSystem::LevelPlayed = CoreEngine::InputSystem::Stage;
                CoreEngine::InputSystem::Stage = Lose;
                Object_picked = 0;
                CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                health = 2;
                timerObj.Reset();
            }
            if (timerObj.GetTimeRemaining() <= 0) {
                UpdateAnimationStateMachine();
                sceneVector.clear();
                //logic for saving the level that the player is lost here
                CoreEngine::InputSystem::LevelPlayed = CoreEngine::InputSystem::Stage;
                CoreEngine::InputSystem::Stage = Lose;
                audioEngine->StopSound("LEVEL_BGM.ogg");
                audioEngine->PlaySound("Lose Sting v1 1.ogg", 0, 0.15f * musicVolume);
                Object_picked = 0;
                CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                health = 2;
                timerObj.Reset();
            }
        }


        // Cheat code for player teleport to all "Object" entities
        if (InputSystem->Stage == Playing ||
            InputSystem->Stage == Playing1 ||
            InputSystem->Stage == Playing2 ||
            InputSystem->Stage == Playing3) {

            if (InputSystem->IsKeyPress(GLFW_KEY_1)) {
                for (auto& entity : ECoordinator.GetAllEntities()) {
                    if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {
                        PhysicsSystem::PhysicsBody& physBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);
                        auto ID = ECoordinator.getThiefID();
                        PhysicsSystem::PhysicsBody& phys = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(ID);
                        auto& transform = ECoordinator.GetComponent<Transform>(ID);

                        float distancex = transform.translate.x - physBody.position.x;
                        float distancey = transform.translate.y - physBody.position.y;

                        if (Object_picked >= totalObjects) {
                            float newX = startingPos.x + 300.0f;
                            float newY = startingPos.y;

                            transform.translate.x = newX;
                            transform.translate.y = newY;

                            float width = phys.aabb.maxX - phys.aabb.minX;
                            float height = phys.aabb.maxY - phys.aabb.minY;

                            phys.aabb.minX = newX;
                            phys.aabb.maxX = newX + width;
                            phys.aabb.minY = newY;
                            phys.aabb.maxY = newY + height;
                            break;
                        }


                        if (physBody.category == "Object") {  // Check if entity is an "Object"
                            transform.translate.x = physBody.position.x;
                            transform.translate.y = physBody.position.y;
                            phys.aabb.minX -= distancex;
                            phys.aabb.maxX -= distancex;
                            phys.aabb.minY -= distancey;
                            phys.aabb.maxY -= distancey;
                            break; // Teleport to the first "Object" found (Remove break if cycling through all)
                        }
                    }
                }
            }

        }


        for (EntityID lasers : laserEntities) {
            if ((CoreEngine::InputSystem::Stage == Playing) ||
                (CoreEngine::InputSystem::Stage == Playing1) ||
                (CoreEngine::InputSystem::Stage == Playing2) ||
                (CoreEngine::InputSystem::Stage == Playing3)) {
                PlayProximitySound(lasers);
            }
        }



        if (Object_picked >= totalObjects && CoreEngine::InputSystem::Stage == Playing ||
            Object_picked >= totalObjects && CoreEngine::InputSystem::Stage == Playing1 ||
            Object_picked >= totalObjects && CoreEngine::InputSystem::Stage == Playing2 ||
            Object_picked >= totalObjects && CoreEngine::InputSystem::Stage == Playing3) {
            if (!hasPlayedVanHonk) {
                audioEngine->PlaySound("VAN_HONK.ogg", 0, 0.3f * sfxVolume);
                hasPlayedVanHonk = true;
            }
        }
        else {
            hasPlayedVanHonk = false;
        }

        if (!hasWind) {
            if ((CoreEngine::InputSystem::Stage == Playing) ||
                (CoreEngine::InputSystem::Stage == Playing1) ||
                (CoreEngine::InputSystem::Stage == Playing2) ||
                (CoreEngine::InputSystem::Stage == Playing3) ||
                (CoreEngine::InputSystem::Stage == Pause)) {
                audioEngine->PlaySound("WIND-SOFTER.ogg", 0, 0.1f * musicVolume, 17);
                hasWind = true;
            }
        }
        else {
            if (!((CoreEngine::InputSystem::Stage == Playing) ||
                (CoreEngine::InputSystem::Stage == Playing1) ||
                (CoreEngine::InputSystem::Stage == Playing2) ||
                (CoreEngine::InputSystem::Stage == Playing3) ||
                (CoreEngine::InputSystem::Stage == Pause))) {
                audioEngine->StopSound("WIND-SOFTER.ogg");
                hasWind = false;
            }
        }

        // Handle game exit
        if (InputSystem->IsKeyPress(GLFW_KEY_ESCAPE)) {
            /*ECoordinator.DestroyAllGameObjects();
            CoreEngine::IMessage quitMessage(CoreEngine::MessageID::Quit, "HustlersEngine");
            CoreEngine::MessageBroker::Instance().Notify(&quitMessage);
            glfwSetWindowShouldClose(window, GL_TRUE);*/
        }

        if (reset) {
            ECoordinator.DestroyAllGameObjects();
            LoadGameObjectsFromJson("Json/GameObjects.json");
            health = 2;
            reset = false;

            Object_picked = 0;
        }
    }
}

void FreeGame() {
}

void ResetGame() {

    //ECoordinator.FadeOutAllObjects();
    CoreEngine::InputSystem::Stage = MainMenu;
    CoreEngine::InputSystem::isPaused = false;

    health = 2;

    Object_picked = 0;
    timerObj.Reset();
    ECoordinator.ClearAllEntities();
    CreateObjectsForStage(CoreEngine::InputSystem::Stage);

    // ECoordinator.FadeInAllObjects();

}


void updatelasers(float deltaTime) {

    for (auto entity : ECoordinator.GetAllEntities()) {
        if (ECoordinator.HasComponent<LaserComponent>(entity)) {
            LaserComponent& laser = ECoordinator.GetComponent<LaserComponent>(entity);

            // Update the laser's timer only if turnedOn is false
            laser.timer -= deltaTime;

            // Toggle state when timer expires
            if (laser.timer <= 0.0f) {
                laser.isActive = !laser.isActive;
                laser.timer = laser.isActive ? laser.activeTime : laser.inactiveTime;
            }

            // Check if linkModuleID exists in the entityNameMap
            const std::string& linkedName = laser.linkModuleID;

            if (linkedName.empty()) {
                continue;  // Skip if no link ID assigned
            }

            // Debug: Print what it's trying to link to
            // std::cout << "[Laser] Trying to link to: " << linkedName << std::endl;

            auto it = entityNameMap.find(linkedName);
            if (it != entityNameMap.end()) {
                EntityID linkedEntity = it->second;

                // Get components for linked entity
                if (ECoordinator.HasComponent<HUGraphics::GLModel>(linkedEntity) &&
                    ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(linkedEntity)) {

                    auto& graphics = ECoordinator.GetComponent<HUGraphics::GLModel>(linkedEntity);
                    auto& linkedPhysics = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(linkedEntity);

                    if (linkedPhysics.category == "Laser Module") {
                        GLuint textureID = 0;

                        if (laser.turnedOn && laser.isActive) {
                            auto activeTex = TextureLibrary.GetAssets("SmallTopLaserRED.png");
                            if (activeTex) {
                                textureID = activeTex->GetTextureID();
                            }
                            else {
                                std::cerr << " Missing texture: SmallTopLaserRED.png\n";
                            }
                        }
                        else {
                            auto inactiveTex = TextureLibrary.GetAssets("SmallTopLazer.png");
                            if (inactiveTex) {
                                textureID = inactiveTex->GetTextureID();
                            }
                            else {
                                std::cerr << "Missing texture: SmallTopLazer.png\n";
                            }
                        }

                        if (textureID != 0) {
                            graphics.textureID = textureID;
                        }
                    }
                }
            }
            else {
                std::cerr << "[Laser WARNING] Entity name not found in map: " << linkedName << std::endl;
            }
        }
    }

    // Optional: Print current map contents
    /*
    std::cout << "----- entityNameMap -----\n";
    for (const auto& [name, id] : entityNameMap) {
        std::cout << name << "\n";
    }
    std::cout << "--------------------------\n";
    */
}


/**
* @brief Plays the proximity laser sound, pans the sound (-1 to 1, left to right) and sets its volume too.
* @param EntityID laserID to play the laser of that specific entityID
*/
void PlayProximitySound(EntityID laserID) {
    if (!ECoordinator.hasThiefID()) return;

    glm::vec3 thiefPos = ECoordinator.GetComponent<Transform>(ECoordinator.getThiefID()).translate;
    glm::vec3 laserPos = ECoordinator.GetComponent<Transform>(laserID).translate;
    float distance = glm::length(laserPos - thiefPos);
    float maxDistance = 220.0f;

    // Check if sound is currently playing for this laser
    bool isCurrentlyPlaying = false;
    auto it = laserSoundPlaying.find(laserID);
    if (it != laserSoundPlaying.end()) {
        isCurrentlyPlaying = it->second;
    }

    if (distance > maxDistance || std::abs(laserPos.x - thiefPos.x) > 220) {
        // Laser is out of range - stop sound if playing
        if (isCurrentlyPlaying) {
            audioEngine->SetSoundVolume("NearLaser.ogg", 0, static_cast<int>(laserID));
            laserSoundPlaying[laserID] = false;
        }
        return;
    }

    // Calculate pan/volume
    audioEngine->PositionToPan(laserPos, thiefPos, fPan);
    float volume = 0.1f * (1.0f - (distance / maxDistance));

    // Check if laser is active
    bool isLaserActive = ECoordinator.GetComponent<LaserComponent>(laserID).isActive &&
        ECoordinator.GetComponent<LaserComponent>(laserID).turnedOn;

    if (!isLaserActive) {
        if (isCurrentlyPlaying) {
            audioEngine->SetSoundVolume("NearLaser.ogg", 0, static_cast<int>(laserID));
            laserSoundPlaying[laserID] = false;
        }
        return;
    }

    // Play/update logic
    if (!isCurrentlyPlaying) {
        // First-time play
        audioEngine->PlaySound("NearLaser.ogg", fPan, volume * sfxVolume, static_cast<int>(laserID));
        laserSoundPlaying[laserID] = true;
    }
    else {
        // Update existing sound
        audioEngine->SetSoundPan("NearLaser.ogg", fPan, static_cast<int>(laserID));
        audioEngine->SetSoundVolume("NearLaser.ogg", volume, static_cast<int>(laserID));
    }
}
