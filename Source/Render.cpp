/**
 * @file Render.cpp
 * @brief Implementation of the Render System for managing rendering processes and updates.
 *
 * This source file defines the `RenderSystem` class, which handles initialization, updates,
 * and rendering of entities in a layered structure. The Render System integrates with
 * various components such as `Transform`, `GLModel`, and `RenderLayer` to manage rendering
 * tasks dynamically based on the entity composition.
 *
 * Key Features:
 * - **Layered Rendering**: Entities are rendered based on their assigned layer, with sorting
 *   and visibility control to ensure proper rendering order and culling.
 * - **Interactive Input Handling**: Includes functionality to handle user inputs like zooming,
 *   rotating, scaling, and toggling debug drawing.
 * - **Debug Drawing**: Allows visualization of bounding boxes and other debug overlays.
 * - **Dynamic Resolution Scaling**: Adjusts mouse input and rendering to match screen resolution,
 *   supporting both fullscreen and windowed modes.
 * - **Message Integration**: Interacts with the Message Broker for events like rendering objects,
 *   detecting collisions, and handling application quit signals.
 * - **Camera Control**: Work in progress
 *
 * Utility Functions:
 * - `GenerateOutlines` and `DrawOutlines`: Provides debug visualization of entity boundaries.
 * - `BeginLayerRendering`: Configures OpenGL states for different layers.
 * - `toggleDebugDrawing`: Toggles debug drawing mode, clearing and regenerating outlines.
 *
 * Author: Che Ee (70%)
 * Co-Author: Rui Jie (10%)
 * Co-Author: Lewis (10%)
 * Co-Author: Jarren (10%)
 */

#include "Render.h"
#include "Graphics.h"
#include "Physics.h"
#include "matrix3x3.h"
#include "vector3d.h"
#include <random>
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include "HelperFunctions.h"
#include <filesystem>
#include <chrono>
#include <chrono>
#include "ButtonComponent.h"

#include "Core.h"
#include "../Volume.h"
 //GLFWwindow* window;

bool ispaused = false;
bool debugDrawingEnabled;
bool previousOKeyState = false;
const std::chrono::milliseconds clickCooldown = std::chrono::milliseconds(300);
std::chrono::steady_clock::time_point lastClickTime;
bool playHover;

AABB selectedAABB;
int selectedHandle = -1;
bool hasSelectedAABB = false;
std::vector<EntityID> outlineEntities;
static PhysicsSystem physicssystem;

int currentMasterVolume = 100;
int currentSFXVolume = 100;
int currentMusicVolume = 100;
float initialBGMVolume;
bool hasSetInitialBGMVolume = false;

// Audio Stuff
static const std::vector<std::string> foregroundSounds = {
    /*"Scene1_Foreground.ogg",
    "Scene2_Foreground.ogg",
    "Scene3_Foreground.ogg",
    "Scene4_Foreground.ogg",
    "Scene5_Foreground.ogg",
    "Scene6_Foreground.ogg",
    "Scene7_Foreground.ogg",
    "Scene8_Foreground.ogg",
    "Scene9_Foreground.ogg",*/

    "cutscene_audio.ogg",
    "Ending_Cutscene.ogg"

};

// Audio Stuff
static const std::vector<std::string> backgroundSounds = {
    /*"Scene1_Background.ogg",
    "Scene2_Background.ogg",
    "Scene3_Background.ogg",
    "Scene4_Background.ogg",
    "Scene5_Background.ogg",
    "Scene6_Background.ogg",
    "Scene7_Background.ogg",
    "Scene8_Background.ogg",
    "Scene9_Background.ogg",*/
};

namespace {
    bool isDragging = false; // Flag to indicate dragging state
    Math3D::Vector3D lastMousePosition; // To store last mouse position
}

void ResetHoverScaling() {
    static bool hasPrinted = false;  // Ensures message prints only once

    if (!hasPrinted) {
        for (int id = 3; id <= 7; ++id) {
            if (ECoordinator.HasComponent<Transform>(id)) {
                Transform& transform = ECoordinator.GetComponent<Transform>(id);
                transform.scale.x = 200.0f;
                transform.scale.y = 100.0f;
            }
        }
        hasPrinted = true;  // Prevent further prints
    }
}
//void HandleHover(EntityID entity, const std::pair<float, float>& mousePos) {
//    auto& transform = ECoordinator.GetComponent<Transform>(entity);
//    float buttonWidth = transform.scale.x;
//    float buttonHeight = transform.scale.y;
//
//    bool isHovered = IsAreaClicked(
//        mousePos.first, mousePos.second,
//        transform.translate.x, transform.translate.y,
//        buttonWidth, buttonHeight
//    );
//
//    if (isHovered) {
//        transform.scale.x *= 1.2f;
//        transform.scale.y *= 1.2f;
//    }
//    else {
//        transform.scale.x = buttonWidth;
//        transform.scale.y = buttonHeight;
//    }
//}

/***********************************************
 * @brief Initializes the Render System.
 *
 * This function sets up the Render System by:
 * - Defining the required component signature (`Transform`, `GLModel`, `RenderLayer`).
 * - Registering message handlers with the Message Broker for events related to rendering,
 *   collision detection, and application quit signals.
 * - Attaching the render object handler for rendering requests.
 *
 * This initialization ensures that the system is properly configured to manage entity rendering.
 ***********************************************/
void RenderSystem::Init()
{
    Signature signature;
    signature.set(ECoordinator.GetComponentType<Transform>());
    signature.set(ECoordinator.GetComponentType<HUGraphics::GLModel>());
    signature.set(ECoordinator.GetComponentType<RenderLayer>());

    //ECoordinator.SetSystemSignature<RenderSystem>(signature);

    //register with message broker 
    CoreEngine::MessageBroker::Instance().Register(CoreEngine::RenderObject, this);
    CoreEngine::MessageBroker::Instance().Register(CoreEngine::CollisionDetected, this);
    CoreEngine::MessageBroker::Instance().Register(CoreEngine::MessageID::Quit, this);

    // Attach a specific handler for RenderObject messages
    AttachHandler(CoreEngine::RenderObject, RenderSystem::RenderObjectHandler);
}

/***********************************************
 * @brief Updates the Render System.
 *
 * This function is responsible for:
 * - Handling object fade effects.
 * - Checking and processing the game state (`MainMenu`, `Pause`, `Playing`, `HowToPlay`).
 * - Verifying if the player reaches the van, triggering the win condition.
 * - Sorting entities by rendering layer and rendering them accordingly.
 * - Processing UI interactions like button clicks.
 * - Managing debug drawing overlays.
 *
 * It runs every frame to ensure dynamic rendering and input handling.
 *
 * @param deltaTime Time elapsed since the last frame update.
 ***********************************************/

void RenderSystem::Update(double deltaTime) {
    if (windowFocused) {
        UpdateFadeEffects((float)deltaTime);

        masterVolume = currentMasterVolume * 0.01f;
        sfxVolume = currentSFXVolume * 0.01f;
        musicVolume = currentMusicVolume * 0.01f;
        audioEngine->SetMasterVolume(masterVolume);

        if (hasSetInitialBGMVolume == false) {
            if (audioEngine->isPlaying("BGM.ogg")) {
                initialBGMVolume = audioEngine->GetSoundVolume("BGM.ogg");
                hasSetInitialBGMVolume = true;
            }
        }

        //if thief exists,timer exists
        if (ECoordinator.hasThiefID()) {
            if (Object_picked >= totalObjects) {

                if (ECoordinator.HasComponent<HUGraphics::GLModel>(getBackToVanImage)) {
                    ECoordinator.GetComponent<HUGraphics::GLModel>(getBackToVanImage).alpha = 1.0f;
                }

                // std::cout << "object picked > than 0"<<std::endl;
                Transform t = ECoordinator.GetComponent<Transform>(ECoordinator.getThiefID());

                const int vanX = 285;

                const int vanY = 775;

                const float toleranceX = 105;
                const float toleranceY = 70;

                if (std::abs(t.translate.x - vanX) <= toleranceX && std::abs(t.translate.y - vanY) <= toleranceY) {

                    // CoreEngine::InputSystem::SavedStage = Playing;
                    CoreEngine::InputSystem::LevelPlayed = CoreEngine::InputSystem::Stage;
                    CoreEngine::InputSystem::Stage = starRating;

                    //complete level ,complete level , never lose health

                    if (health == 2) {
                        winStatus = (timerObj.GetTimeRemaining() > 60) ? 111 : 101;
                    }
                    else {
                        winStatus = (timerObj.GetTimeRemaining() > 60) ? 110 : 100;
                    }

                    wingame = true;
                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                }
            }
            else {
                if (ECoordinator.HasComponent<HUGraphics::GLModel>(getBackToVanImage)) {
                    HUGraphics::GLModel& gBTVImodel = ECoordinator.GetComponent<HUGraphics::GLModel>(getBackToVanImage);
                    gBTVImodel.alpha = 0.0f;
                }

            }

        }


        (void)deltaTime;
        auto now = std::chrono::steady_clock::now();

        if (CoreEngine::InputSystem::IsKeyReleased(GLFW_KEY_ESCAPE)) {
            if (CoreEngine::InputSystem::Stage == HowToPlay2) {
                ECoordinator.DestroyAllUIObjects();
                CoreEngine::InputSystem::SavedStage = Pause;
                CreateObjectsForStage(CoreEngine::InputSystem::Stage);
            }


            else if (CoreEngine::InputSystem::Stage == MainMenu) {
                return;
            }

            else if (CoreEngine::InputSystem::Stage == Playing) {
                CoreEngine::InputSystem::isPaused = !CoreEngine::InputSystem::isPaused;

                if (CoreEngine::InputSystem::isPaused) {
                    timerObj.Pause();
                    SaveGameObjectsToJson_doc("tempasas.json");
                    CoreEngine::InputSystem::SavedStage = Playing;
                    CoreEngine::InputSystem::Stage = Pause;
                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);

                }
                else {
                    timerObj.Resume();
                    // Restore previous stage
                    ECoordinator.DestroyAllUIObjects();
                    CoreEngine::InputSystem::Stage = Playing;
                    CoreEngine::InputSystem::SavedStage = Pause;
                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);

                    if (ECoordinator.hasThiefID()) {
                        /*auto mdl = ECoordinator.GetComponent<HUGraphics::GLModel>(ECoordinator.getThiefID());
                        mdl.alpha = 1.0f;*/
                    }


                }
            }
            else if (CoreEngine::InputSystem::Stage == LevelSelect) {
                ECoordinator.DestroyAllUIObjects();
                CoreEngine::InputSystem::SavedStage = MainMenu;
                CreateObjectsForStage(CoreEngine::InputSystem::Stage);
            }

            else if (CoreEngine::InputSystem::Stage == Playing1) {
                CoreEngine::InputSystem::isPaused = !CoreEngine::InputSystem::isPaused;

                if (CoreEngine::InputSystem::isPaused) {
                    timerObj.Pause();
                    SaveGameObjectsToJson_doc("tempasas.json");
                    CoreEngine::InputSystem::SavedStage = Playing1;
                    CoreEngine::InputSystem::Stage = CoreEngine::InputSystem::Stage = Pause;
                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);

                }
                else {
                    //timerObj.Resume();
                    // Restore previous stage
                    ECoordinator.DestroyAllUIObjects();
                    CoreEngine::InputSystem::Stage = Playing1;
                    CoreEngine::InputSystem::SavedStage = Pause;
                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);




                }
            }
            else if (CoreEngine::InputSystem::Stage == Playing2) {
                CoreEngine::InputSystem::isPaused = !CoreEngine::InputSystem::isPaused;

                if (CoreEngine::InputSystem::isPaused) {
                    timerObj.Pause();
                    SaveGameObjectsToJson_doc("tempasas.json");
                    CoreEngine::InputSystem::SavedStage = Playing2;
                    CoreEngine::InputSystem::Stage = Pause;
                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);

                }
                else {
                    // timerObj.Resume();
                     // Restore previous stage
                    ECoordinator.DestroyAllUIObjects();
                    CoreEngine::InputSystem::Stage = Playing2;
                    CoreEngine::InputSystem::SavedStage = Pause;
                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);



                }
            }
            else if (CoreEngine::InputSystem::Stage == Playing3) {
                CoreEngine::InputSystem::isPaused = !CoreEngine::InputSystem::isPaused;

                if (CoreEngine::InputSystem::isPaused) {
                    timerObj.Pause();
                    SaveGameObjectsToJson_doc("tempasas.json");
                    CoreEngine::InputSystem::SavedStage = Playing3;
                    CoreEngine::InputSystem::Stage = Pause;
                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);

                }
                else {
                    // timerObj.Resume();

                     // Restore previous stage
                    ECoordinator.DestroyAllUIObjects();
                    CoreEngine::InputSystem::Stage = Playing3;
                    CoreEngine::InputSystem::SavedStage = Pause;
                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);

                    if (ECoordinator.hasThiefID()) {

                        /*auto mdl = ECoordinator.GetComponent<HUGraphics::GLModel>(ECoordinator.getThiefID());
                        mdl.alpha = 1.0f;*/
                    }
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        std::vector<std::pair<int, EntityID>> entitiesWithLayers;


        glm::mat4 viewMatrixForUI = glm::mat4(1.0f);

        // Get the view matrix from the camera
        // Get the view matrix from the camera
        glm::mat4 viewMatrix = cameraObj.GetViewMatrix();

        for (const auto& entity : mEntities) {
            // Check if the entity has the RenderLayer component before accessing it

            if (ECoordinator.HasComponent<RenderLayer>(entity)) {
                auto& layer = ECoordinator.GetComponent<RenderLayer>(entity);
                entitiesWithLayers.emplace_back(int(layer.layer), entity);
            }
        }

        //Sort entities by layer (ascending)
        std::stable_sort(entitiesWithLayers.begin(), entitiesWithLayers.end(),
            [](const std::pair<int, EntityID>& a, const std::pair<int, EntityID>& b) {
                return a.first < b.first;  // Sort by layer
            });

        // Push the player entity to the end
        if (ECoordinator.hasThiefID()) {
            entitiesWithLayers.emplace_back(
                int(ECoordinator.GetComponent<RenderLayer>(ECoordinator.getThiefID()).layer),
                ECoordinator.getThiefID()
            );
        }

        // Render each layer group separately
        int currentLayer = -1;
        bool* visibleLayers = ImGuiManager::getVisibleLayers();

        for (auto& pair : entitiesWithLayers) {
            int layer = pair.first;

            EntityID entity = pair.second;

            // Check if the current layer is visible
            if (!visibleLayers[layer]) {
                continue; // Skip rendering this layer if it's not visible
            }

            if (!ECoordinator.HasComponent<Transform>(entity)) {
                continue;
            }

            if (ECoordinator.HasComponent<LaserComponent>(entity)) {
                const LaserComponent& laserComp = ECoordinator.GetComponent<LaserComponent>(entity);
                if (!laserComp.isActive || !laserComp.turnedOn) {
                    continue; // Skip rendering this entity if the laser is inactive
                }
            }

            // Change the render pass when a new layer starts
            if (layer != currentLayer) {
                currentLayer = layer;
                BeginLayerRendering(layer);
            }

            auto& transform2 = ECoordinator.GetComponent<Transform>(entity);
            auto& mdl = ECoordinator.GetComponent<HUGraphics::GLModel>(entity);

            if (CoreEngine::InputSystem::Stage == MainMenu || CoreEngine::InputSystem::Stage == Pause || CoreEngine::InputSystem::Stage == HowToPlay
                || CoreEngine::InputSystem::Stage == confirmQuit || CoreEngine::InputSystem::Stage == LevelSelect || CoreEngine::InputSystem::Stage == confirmQuit2
                || CoreEngine::InputSystem::Stage == HowToPlay2 || CoreEngine::InputSystem::Stage == Credit || CoreEngine::InputSystem::Stage == Lose
                || CoreEngine::InputSystem::Stage == Settings) {
                auto pos = getScaledMousePos();

                if (showImgui) {
                    pos = std::make_pair(mousePosInTexture.x, mousePosInTexture.y);
                }

                // Iterate through all entities
                std::vector<EntityID> allEntities = ECoordinator.GetAllEntities();
                for (auto& entity2 : allEntities) {
                    // Check if the entity has Transform, RenderLayer, and ButtonComponent
                    if (!ECoordinator.HasComponent<Transform>(entity2) ||
                        !ECoordinator.HasComponent<RenderLayer>(entity2) ||
                        !ECoordinator.HasComponent<ButtonComponent>(entity2)) {
                        continue; // Skip entities without required components
                    }

                    auto& transform = ECoordinator.GetComponent<Transform>(entity2);
                    auto& renderLayer = ECoordinator.GetComponent<RenderLayer>(entity2);

                    // Check if the entity is in the UI layer
                    if (renderLayer.layer == RenderLayerType::UI) {
                        // Calculate bounding box based on Transform
                        Math3D::Vector3D originalScale = originalScales[entity2];

                        float left = transform.translate.x - (originalScale.x / 20.0f);
                        float bottom = transform.translate.y - (originalScale.y / 7.0f);

                        auto& button = ECoordinator.GetComponent<ButtonComponent>(entity2);

                        // Check if the mouse is hovering over the button
                        bool isHovered = IsAreaClicked(pos.first, pos.second, left, bottom, originalScale.x, originalScale.y);
                        // Apply hover scaling
                        if (isHovered) {
                            //std::cout <<"button action is "<< button.action << "\n";
                            if (!button.isHover) {
                                audioEngine->PlaySound("UI_Hover_New.ogg", 0.f, 0.1f, 16);
                                button.isHover = true;
                            }

                            //if (CoreEngine::InputSystem::Stage != LevelSelect)
                            //{
                            //    if (button.action == "Vol" || 
                            //        button.action == "incrementMasterVolume" || button.action == "incrementSFXVolume" || button.action == "incrementMusicVolume" ||
                            //        button.action == "decrementMasterVolume" || button.action == "decrementSFXVolume" || button.action == "decrementMusicVolume") {
                            //        transform.scale.x = 50.0f * 1.2f; // Scale up by 20%
                            //        transform.scale.y = 50.0f * 1.2f; // Scale up by 20%
                            //    }
                            //    else {
                            //        transform.scale.x = 400.0f * 1.2f; // Scale up by 20%
                            //        transform.scale.y = 100.0f * 1.2f; // Scale up by 20%
                            //    }

                            //}
                            //else {
                            //    if (button.action == "back")
                            //    {
                            //        transform.scale.x = 400.0f * 1.2f; // Reset to normal width
                            //        transform.scale.y = 300.0f * 1.2f; // Reset to normal height
                            //    }

                            //    else {
                            //        transform.scale.x = 400.0f * 1.2f; // Scale up by 20%
                            //        transform.scale.y = 250.0f * 1.2f; // Scale up by 20%
                            //    }


                            //}
                            transform.scale.x = originalScale.x * 1.2f; // Reset to original width
                            transform.scale.y = originalScale.y * 1.2f; // Reset to original height
                        }
                        else {
                            button.isHover = false;
                            transform.scale.x = originalScale.x; // Reset to original width
                            transform.scale.y = originalScale.y; // Reset to original height
                        }


                        // Handle button click
                        if (isHovered && CoreEngine::InputSystem::IsMouseClicked(GLFW_MOUSE_BUTTON_LEFT)) {

                            static std::chrono::steady_clock::time_point lastSceneChange = std::chrono::steady_clock::now();
                            lastSceneChange = std::chrono::steady_clock::now(); // Reset scene transition timer

                            // std::cout << button.action << "\n";

                            if (button.action == "startGame") {



                                audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);
                                button.onClick = []() {



                                    CoreEngine::InputSystem::Stage = hasSeenCutscene ? Playing1 : cutScene; //Playing; //
                                    ECoordinator.FadeOutAllObjects();

                                    //fade in new object


                                    ResetHoverScaling();
                                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                                    if (!hasSeenCutscene) {
                                        FadeInObject(sceneVector.back().first);
                                    }

                                    else {
                                        ECoordinator.FadeInAllObjects();
                                    }

                                    };
                            }
                            else if (button.action == "howToPlay") {
                                audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);
                                button.onClick = []() {

                                    ////later on in howtoplay if the savedstage is not main menu ,we go back to that stage else we go back to main menu
                                    //CoreEngine::InputSystem::SavedStage = CoreEngine::InputSystem::Stage;
                                    if (CoreEngine::InputSystem::Stage == Pause) {

                                        CoreEngine::InputSystem::Stage = HowToPlay2;
                                    }

                                    else if (CoreEngine::InputSystem::Stage == MainMenu) {
                                        CoreEngine::InputSystem::Stage = HowToPlay;
                                    }
                                    ECoordinator.DestroyAllUIObjects();
                                    ResetHoverScaling();
                                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                                    };
                            }
                            else if (button.action == "Vol") {
                                audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);
                                button.onClick = []() {
                                    CoreEngine::InputSystem::Stage = Settings;
                                    CoreEngine::InputSystem::SavedStage = Pause;
                                    ResetHoverScaling();
                                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                                    };
                            }
                            else if (button.action == "levelSelect") {
                                audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);
                                button.onClick = []() {
                                    CoreEngine::InputSystem::Stage = LevelSelect;
                                    CoreEngine::InputSystem::SavedStage = Pause;
                                    ECoordinator.DestroyAllUIObjects();
                                    ResetHoverScaling();
                                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                                    };
                            }
                            else if (button.action == "quitGame") {
                                if (CoreEngine::InputSystem::Stage == MainMenu) {
                                    audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);
                                    button.onClick = []() {
                                        CoreEngine::InputSystem::Stage = confirmQuit;
                                        ECoordinator.DestroyAllUIObjects();
                                        CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                                        };
                                }
                                else
                                {
                                    audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);
                                    button.onClick = []() {
                                        CoreEngine::InputSystem::Stage = confirmQuit2;
                                        ECoordinator.DestroyAllUIObjects();
                                        ResetHoverScaling();
                                        CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                                        };
                                }

                            }
                            else if (button.action == "resume") {
                                audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);
                                button.onClick = []() {
                                    CoreEngine::InputSystem::isPaused = false;
                                    CoreEngine::InputSystem::Stage = CoreEngine::InputSystem::SavedStage;
                                    CoreEngine::InputSystem::SavedStage = Pause;
                                    ECoordinator.DestroyAllUIObjects();

                                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);

                                    if (ECoordinator.HasComponent<HUGraphics::GLModel>(getBackToVanImage)) {
                                        ECoordinator.GetComponent<HUGraphics::GLModel>(getBackToVanImage).alpha = 0.0f;
                                    }


                                    timerObj.Resume();
                                    };
                            }
                            else if (button.action == "mainMenu") {
                                audioEngine->PlaySound("UI_Back.ogg", 0.0f, 0.05f * sfxVolume);
                                ResetGame();
                            }
                            else if (button.action == "back") {

                                lastClickTime = now; // Reset the cooldown timer
                                if (CoreEngine::InputSystem::Stage == HowToPlay || CoreEngine::InputSystem::Stage == LevelSelect || CoreEngine::InputSystem::Stage == Settings) {
                                    audioEngine->PlaySound("UI_Back.ogg", 0.0f, 0.05f * sfxVolume);
                                    ECoordinator.DestroyAllUIObjects();
                                    CoreEngine::InputSystem::Stage = MainMenu;
                                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                                }
                                if (CoreEngine::InputSystem::Stage == HowToPlay2) {
                                    //std::cout << "testing";
                                    std::chrono::milliseconds cooldown(500); // 500ms delay before accepting new clicks

                                    audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);
                                    CoreEngine::InputSystem::isPaused = false;
                                    ECoordinator.DestroyAllUIObjects();
                                    CoreEngine::InputSystem::Stage = Pause;
                                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                                }
                            }
                            else if (button.action == "quit") {
                                audioEngine->PlaySound("UI_Back.ogg", 0.0f, 0.05f * sfxVolume);
                                ECoordinator.DestroyAllGameObjects();
                                CoreEngine::IMessage quitMessage(CoreEngine::MessageID::Quit, "HustlersEngine");
                                CoreEngine::MessageBroker::Instance().Notify(&quitMessage);
                                glfwSetWindowShouldClose(InputSystem->window, GL_TRUE);
                            }
                            else if (button.action == "nope") {
                                if (CoreEngine::InputSystem::Stage == confirmQuit2)
                                {
                                    CoreEngine::InputSystem::SavedStage = Pause;
                                    CoreEngine::InputSystem::Stage = Pause;
                                    audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);
                                    ECoordinator.DestroyAllUIObjects();
                                    //  std::cout << InputSystem->Stage << std::endl;
                                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);

                                }

                                if (CoreEngine::InputSystem::Stage == confirmQuit)
                                {
                                    CoreEngine::InputSystem::Stage = MainMenu;
                                    ECoordinator.DestroyAllUIObjects();
                                    audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);

                                    //  std::cout << InputSystem->Stage << std::endl;
                                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                                }

                            }

                            else if (button.action == "incrementMasterVolume") {
                                audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);
                                masterVolume = audioEngine->GetMasterVolume();
                                if (currentMasterVolume != 100) {
                                    currentMasterVolume += 10;
                                }
                                //std::cout << "currentMasterVolume: " << currentMasterVolume;
                                UpdateVolumeDisplays();
                            }

                            else if (button.action == "decrementMasterVolume") {
                                audioEngine->PlaySound("UI_Back.ogg", 0.0f, 0.05f * sfxVolume);
                                masterVolume = audioEngine->GetMasterVolume();
                                if (currentMasterVolume != 0) {
                                    currentMasterVolume -= 10;
                                }

                                //std::cout << "currentMasterVolume: " << currentMasterVolume;
                                UpdateVolumeDisplays();
                            }

                            else if (button.action == "incrementSFXVolume") {
                                audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);

                                if (currentSFXVolume != 100) {
                                    currentSFXVolume += 10;
                                }

                                //std::cout << "currentSFXVolume: " << currentSFXVolume;
                                UpdateVolumeDisplays();
                            }

                            else if (button.action == "decrementSFXVolume") {
                                audioEngine->PlaySound("UI_Back.ogg", 0.0f, 0.05f * sfxVolume);
                                if (currentSFXVolume != 0) {
                                    currentSFXVolume -= 10;
                                }
                                //std::cout << "currentSFXVolume: " << currentSFXVolume;
                                UpdateVolumeDisplays();
                            }

                            else if (button.action == "incrementMusicVolume") {
                                audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);
                                if (currentMusicVolume != 100) {
                                    currentMusicVolume += 10;
                                }

                                if (audioEngine->isPlaying("BGM.ogg")) {
                                    audioEngine->SetSoundVolume("BGM.ogg", initialBGMVolume * musicVolume);
                                }
                                //std::cout << "currentMusicVolume: " << currentMusicVolume;
                                UpdateVolumeDisplays();
                            }

                            else if (button.action == "decrementMusicVolume") {
                                audioEngine->PlaySound("UI_Back.ogg", 0.0f, 0.05f * sfxVolume);
                                if (currentMusicVolume != 0) {
                                    currentMusicVolume -= 10;
                                }

                                if (audioEngine->isPlaying("BGM.ogg")) {
                                    audioEngine->SetSoundVolume("BGM.ogg", initialBGMVolume * musicVolume);
                                }

                                //std::cout << "currentMusicVolume: " << currentMusicVolume;
                                UpdateVolumeDisplays();
                            }


                            else if (button.action == "lvl1") {
                                ECoordinator.DestroyAllUIObjects();
                                CoreEngine::InputSystem::Stage = Playing1;
                                CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                            }
                            else if (button.action == "lvl4") {
                                ECoordinator.DestroyAllUIObjects();
                                CoreEngine::InputSystem::Stage = Playing;
                                CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                            }
                            else if (button.action == "lvl3") {
                                ECoordinator.DestroyAllUIObjects();
                                CoreEngine::InputSystem::Stage = Playing2;
                                CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                            }
                            else if (button.action == "lvl2") {
                                ECoordinator.DestroyAllUIObjects();
                                CoreEngine::InputSystem::Stage = Playing3;
                                CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                            }
                            // Execute the onClick function
                            if (button.onClick) {
                                button.onClick(); // This triggers the assigned action
                            }
                        }
                    }
                }
            }



            //else if (CoreEngine::InputSystem::Stage == HowToPlay) {
            //    auto pos = getScaledMousePos();
            //    if (showImgui) {
            //        pos = std::make_pair(mousePosInTexture.x, mousePosInTexture.y);
            //    }

            //    // Debug: Print mouse position
            //    //std::cout << "Mouse Position: (" << pos.first << ", " << pos.second << ")\n";

            //    // Find the return button dynamically
            //    int returnButtonID = -1;  // Default to an invalid ID

            //    for (auto entities : ECoordinator.GetAllEntities()) {
            //        if (ECoordinator.HasComponent<Transform>(entities)) {
            //            Transform& transformed = ECoordinator.GetComponent<Transform>(entities);

            //            // Assuming the return button is located at (800, 600)
            //            if (IsAreaClicked(transformed.translate.x, transformed.translate.y, 800, 625, 200, 75)) {
            //                returnButtonID = entities;
            //                break;
            //            }
            //        }
            //    }

            //    // Debug: Print found button ID
            //    if (returnButtonID == -1) {
            //        //std::cout << "Warning: Could not find the return button entity!\n";
            //    }
            //    else {
            //    }

            //    // Handle button click
            //    if (CoreEngine::InputSystem::IsMouseClicked(GLFW_MOUSE_BUTTON_LEFT) &&
            //        IsAreaClicked(pos.first, pos.second, 800, 600, 200, 100)) {

            //        audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);

            //        // Clear UI elements before switching back
            //        ECoordinator.DestroyAllUIObjects();

            //        CoreEngine::InputSystem::Stage = MainMenu;
            //        CreateObjectsForStage(CoreEngine::InputSystem::Stage);
            //    }
            //}




            //else if (CoreEngine::InputSystem::Stage == confirmQuit) {
            //    auto pos = getScaledMousePos();
            //    if (showImgui) {
            //        pos = std::make_pair(mousePosInTexture.x, mousePosInTexture.y);
            //    }

            //    for (int i = 0; i < 2; i++) {
            //        if (CoreEngine::InputSystem::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)
            //            && IsAreaClicked(pos.first, pos.second, 800.f, 425.f + static_cast<float>(i) * 140.f, 400.f, 120.f)) {


            //            if (i == 0) {
            //                audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);
            //                ECoordinator.DestroyAllGameObjects();
            //                CoreEngine::IMessage quitMessage(CoreEngine::MessageID::Quit, "HustlersEngine");
            //                CoreEngine::MessageBroker::Instance().Notify(&quitMessage);
            //                glfwSetWindowShouldClose(InputSystem->window, GL_TRUE);
            //            }

            //            else if (i == 1) {
            //                CoreEngine::InputSystem::Stage = MainMenu;
            //                audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);
            //                //  std::cout << InputSystem->Stage << std::endl;
            //                CreateObjectsForStage(CoreEngine::InputSystem::Stage);
            //            }
            //        }

            //    }


            //}

           /* else if (CoreEngine::InputSystem::Stage == confirmQuit2) {
                auto pos = getScaledMousePos();
                if (showImgui) {
                    pos = std::make_pair(mousePosInTexture.x, mousePosInTexture.y);
                }

                for (int i = 0; i < 2; i++) {
                    if (CoreEngine::InputSystem::IsMousePressed(GLFW_MOUSE_BUTTON_LEFT)
                        && IsAreaClicked(pos.first, pos.second, 800.f, 425.f + static_cast<float>(i) * 140.f, 400.f, 120.f)) {

                        if (i == 0) {
                            audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.2f);
                            ECoordinator.DestroyAllGameObjects();
                            CoreEngine::IMessage quitMessage(CoreEngine::MessageID::Quit, "HustlersEngine");
                            CoreEngine::MessageBroker::Instance().Notify(&quitMessage);
                            glfwSetWindowShouldClose(InputSystem->window, GL_TRUE);
                        }

                        else if (i == 1) {
                            lastClickTime = now;
                            audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.2f);

                            ECoordinator.DestroyAllUIObjects();
                            CoreEngine::InputSystem::Stage = Pause;
                            CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                        }
                    }

                }
            }*/

            //constantly check if the next button is pressed
            else if (CoreEngine::InputSystem::Stage == cutScene) {

                for (auto& entitys : getAllEntities()) {
                    if (!ECoordinator.HasComponent<ButtonComponent>(entitys)) {
                        continue;
                    }

                    auto& transforms = ECoordinator.GetComponent<Transform>(entitys);
                    float left = transforms.translate.x - (transforms.scale.x / 12.0f);
                    float bottom = transforms.translate.y - (transforms.scale.y / 10.0f);

                    auto& button = ECoordinator.GetComponent<ButtonComponent>(entitys);
                    auto pos_mouse = getScaledMousePos();
                    bool isHovered = IsAreaClicked(pos_mouse.first, pos_mouse.second, left, bottom, transforms.scale.x, transforms.scale.y);
                    if (isHovered) {
                        if (!button.isHover) {
                            audioEngine->PlaySound("UI_Hover_New.ogg", 0.0f, 0.1f * sfxVolume, 16);

                            button.isHover = true;
                        }
                        transforms.scale.x = 400.0f * 1.2f; // Scale up by 20%
                        transforms.scale.y = 100.0f * 1.2f; // Scale up by 20%
                    }

                    else {
                        button.isHover = false;
                        transforms.scale.x = 400.f;
                        transforms.scale.y = 100.f;
                    }

                }

                if (audioEngine->isPlaying("BGM.ogg")) {
                    audioEngine->StopSound("BGM.ogg");
                }
                hasSeenCutscene = true;
                playCutsceneSound(0);
                auto pos = getScaledMousePos();

                if (showImgui) {
                    pos = std::make_pair(mousePosInTexture.x, mousePosInTexture.y);
                }
                int currentSceneDuration = sceneVector.back().second;



                if (SceneTimer > currentSceneDuration)
                {
                    playCutsceneSound(0);
                    if (sceneVector.size() > 1) {

                        EntityID lastEntity = sceneVector.back().first;

                        FadeOutObject(lastEntity);

                    }
                    else {
                        audioEngine->SetSoundVolume("BGM.ogg", 0.15f * musicVolume);


                        FadeOutObject(sceneVector.back().first);

                        sceneVector.clear();
                        CoreEngine::InputSystem::Stage = Playing1;

                        CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                        ECoordinator.FadeInAllObjects();

                    }

                    SceneTimer = 0.f;
                }

                //jump straight to the game
                if (CoreEngine::InputSystem::IsMouseClicked(GLFW_MOUSE_BUTTON_LEFT)
                    && IsAreaClicked(pos.first, pos.second, 1320.f, 700.f, 400.f, 100.f)) {
                    FadeOutObject(sceneVector.back().first);
                    sceneVector.clear();
                    CoreEngine::InputSystem::Stage = Playing1;
                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                    ECoordinator.FadeInAllObjects();
                    // isSoundPlayed = true;
                    if (audioEngine->isPlaying(foregroundSounds[0])) {
                        audioEngine->StopSound(foregroundSounds[0]);
                    }




                    SceneTimer = 0.f;
                }

            }

            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);

            // Determine active resolution
            int currentWidth = isFullscreen ? mode->width : 1600; // Use fullscreen or windowed resolution
            int currentHeight = isFullscreen ? mode->height : 900;
            if (ECoordinator.hasThiefID()) {
                if (entity == ECoordinator.getThiefID()) {
                    /*cameraObj.CenterOnCharacter(glm::vec2(transform.translate.x, transform.translate.y));*/

                    //if not full screen

                    cameraObj.CenterOnCharacter(glm::vec2(float(currentWidth / 2), float(currentHeight / 2)));

                    //else if full screen center at the 

                }

            }

            //else if (CoreEngine::InputSystem::Stage == HowToPlay2) {


            //    auto pos = getScaledMousePos();
            //    if (showImgui) {
            //        pos = std::make_pair(mousePosInTexture.x, mousePosInTexture.y);
            //    }


            //    if (CoreEngine::InputSystem::IsMouseClicked(GLFW_MOUSE_BUTTON_LEFT) && IsAreaClicked(pos.first, pos.second, 800, 625, 200, 75)) {


            //        //CoreEngine::InputSystem::isPaused = false;
            //        //timerActive = true;

            //        //// Adjust the timer to account for the paused duration
            //        //auto now = std::chrono::steady_clock::now();
            //        //startTime = now - std::chrono::seconds(pausedElapsed);

            //        //// Reset pausedElapsed to prepare for future pauses
            //        //pausedElapsed = 0;

            //        ECoordinator.DestroyAllUIObjects();
            //        CoreEngine::InputSystem::Stage = Pause;
            //        CreateObjectsForStage(CoreEngine::InputSystem::Stage);


            //    }




            //}

            else if (CoreEngine::InputSystem::Stage == gameWon) {

                auto pos = getScaledMousePos();

                int currentSceneDuration = sceneVector.back().second;


                for (auto& entitys : getAllEntities()) {
                    if (!ECoordinator.HasComponent<ButtonComponent>(entitys)) {
                        continue;
                    }

                    auto& transforms = ECoordinator.GetComponent<Transform>(entitys);
                    float left = transforms.translate.x - (transforms.scale.x / 12.0f);
                    float bottom = transforms.translate.y - (transforms.scale.y / 10.0f);

                    auto& button = ECoordinator.GetComponent<ButtonComponent>(entitys);
                    auto pos_mouse = getScaledMousePos();
                    bool isHovered = IsAreaClicked(pos_mouse.first, pos_mouse.second, left, bottom, transforms.scale.x, transforms.scale.y);
                    // Apply hover scaling
                    if (isHovered) {
                        if (!button.isHover) {
                            audioEngine->PlaySound("UI_Hover_New.ogg", 0.0f, 0.1f * sfxVolume, 16);

                            button.isHover = true;
                        }


                        transforms.scale.x = 400.0f * 1.2f; // Scale up by 20%
                        transforms.scale.y = 100.0f * 1.2f; // Scale up by 20%



                    }

                    else {
                        button.isHover = false;
                        transforms.scale.x = 400.f;
                        transforms.scale.y = 100.f;
                    }

                }



                // std::cout << sceneVector.size()<<"\n";
                playCutsceneSound(1);
                if (SceneTimer > currentSceneDuration)
                {
                    //FadeInObject(sceneVector.back().first);

                    if (sceneVector.size() > 1) {

                        playCutsceneSound(1);
                        EntityID lastEntity = sceneVector.back().first;

                        FadeOutObject(lastEntity);

                    }

                    else {
                        audioEngine->SetSoundVolume("BGM.ogg", 0.15f * musicVolume);

                        FadeOutObject(sceneVector.back().first);

                        sceneVector.clear();
                        CoreEngine::InputSystem::Stage = Credit;
                        CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                        ECoordinator.FadeInAllObjects();
                        //isSoundPlayed = true;
                    }

                    SceneTimer = 0.f;
                }
                //jump straight to the game
                if (CoreEngine::InputSystem::IsMouseClicked(GLFW_MOUSE_BUTTON_LEFT)
                    && IsAreaClicked(pos.first, pos.second, 1320.f, 700.f, 400.f, 100.f)) {
                    FadeOutObject(sceneVector.back().first);
                    sceneVector.clear();
                    CoreEngine::InputSystem::Stage = Credit;
                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                    ECoordinator.FadeInAllObjects();
                    // isSoundPlayed = true;
                    if (audioEngine->isPlaying(foregroundSounds[1])) {
                        audioEngine->StopSound(foregroundSounds[1]);
                    }




                    SceneTimer = 0.f;
                }

                //if (CoreEngine::InputSystem::IsMouseClicked(GLFW_MOUSE_BUTTON_LEFT)
                //    && IsAreaClicked(pos.first, pos.second, 1400.f, 700.f, 200.f, 100.f))
                //{
                //    audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.2f);
                //    if (audioEngine->isPlaying(foregroundSounds[1])) {
                //        audioEngine->StopSound(foregroundSounds[1]);
                //    }
                //    sceneVector.clear();
                //    ResetGame();

                //    ECoordinator.FadeInAllObjects();

                //    //wtf is this
                //    //temp fix
                //    CoreEngine::InputSystem::SavedStage = 0;

                //}


            }

            else if (CoreEngine::InputSystem::Stage == starRating) {
                auto pos = getScaledMousePos();
                for (auto& entitys : getAllEntities()) {
                    if (!ECoordinator.HasComponent<ButtonComponent>(entitys)) {
                        continue;
                    }

                    auto& transforms = ECoordinator.GetComponent<Transform>(entitys);
                    float left = transforms.translate.x - (transforms.scale.x / 12.0f);
                    float bottom = transforms.translate.y - (transforms.scale.y / 10.0f);

                    auto& button = ECoordinator.GetComponent<ButtonComponent>(entitys);
                    auto pos_mouse = getScaledMousePos();
                    bool isHovered = IsAreaClicked(pos_mouse.first, pos_mouse.second, left, bottom, transforms.scale.x, transforms.scale.y);
                    // Apply hover scaling
                    if (isHovered) {
                        if (!button.isHover) {
                            audioEngine->PlaySound("UI_Hover_New.ogg", 0.0f, 0.1f * sfxVolume, 16);

                            button.isHover = true;
                        }


                        transforms.scale.x = 400.0f * 1.2f; // Scale up by 20%
                        transforms.scale.y = 100.0f * 1.2f; // Scale up by 20%



                    }

                    else {
                        button.isHover = false;
                        transforms.scale.x = 400.f;
                        transforms.scale.y = 100.f;
                    }

                }
                if (showImgui) {
                    pos = std::make_pair(mousePosInTexture.x, mousePosInTexture.y);
                }

                if (CoreEngine::InputSystem::IsMouseClicked(GLFW_MOUSE_BUTTON_LEFT) && IsAreaClicked(pos.first, pos.second, 800.f, 625.f, 400.f, 100.f)) {

                    audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.05f * sfxVolume);

                    cycleLevels(CoreEngine::InputSystem::LevelPlayed);


                }
            }

            else if (CoreEngine::InputSystem::Stage == Lose) {

                auto pos = getScaledMousePos();

                if (showImgui) {
                    pos = std::make_pair(mousePosInTexture.x, mousePosInTexture.y);
                }


                if (CoreEngine::InputSystem::IsMouseClicked(GLFW_MOUSE_BUTTON_LEFT)) {
                    for (int i = 0; i < 2; i++) {
                        float centerX = 690.f + static_cast<float>(i * 175.f);
                        float centerY = 625.f;

                        if (IsAreaClicked(pos.first, pos.second, centerX, centerY, 175.f, 75.f)) {
                            if (i == 0) {
                                sceneVector.clear();
                                CoreEngine::InputSystem::Stage = CoreEngine::InputSystem::LevelPlayed;
                                CreateObjectsForStage(CoreEngine::InputSystem::Stage);
                            }
                            else if (i == 1) {
                                audioEngine->PlaySound("MenuSelect.ogg", 0.0f, 0.2f * sfxVolume);
                                CoreEngine::InputSystem::Stage = MainMenu;
                                ResetGame();
                            }
                            break; // No need to check both
                        }
                    }
                }



            }


            glm::mat4 modelMatrix = glm::mat4(1.0f);

            modelMatrix = glm::translate(modelMatrix, glm::vec3(transform2.translate));
            modelMatrix = glm::rotate(modelMatrix, glm::radians(transform2.rotate), glm::vec3(0.0f, 0.0f, 1.0f));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(transform2.scale));

            glm::mat4 projectionMatrix;
            projectionMatrix = glm::ortho(0.0f, 1600.0f, 900.0f, 0.0f, -1.0f, 1.0f);


            //will only draw UI Stuff with identity matrix but not other things.
            if (layer == int(RenderLayerType::UI)) {
                mdl.draw(modelMatrix, projectionMatrix, glm::mat4(1.0f));
            }
            else {
                mdl.draw(modelMatrix, projectionMatrix, cameraObj.GetViewMatrix());
            }
        }

        //bool currentOKeyState = InputSystem->IsKeyPress(GLFW_KEY_O) == GLFW_PRESS;

        //if (currentOKeyState && !previousOKeyState) {
        //    // Toggle debug drawing
        //    toggleDebugDrawing();
        //}

        //// Update the previous state
        //previousOKeyState = currentOKeyState;

        // If debug drawing is enabled, continuously update and draw outlines
        if (debugDrawingEnabled) {
            GenerateOutlines();
            DrawOutlines();
            physicssystem.spatialGrid.clear();
        }
    }
}
/***********************************************
* @brief Configures OpenGL states for rendering layers.
*
* This function applies specific OpenGL configurations based on the provided layer:
* - Layer 0: Background (blending disabled).
* - Layer 1: Midground (alpha blending enabled).
* - Layer 2: Foreground (unique blending mode).
* - Layer 3: Additional effects.
*
* Proper layer management ensures correct rendering order and transparency handling.
*
* @param layer The rendering layer to configure.
***********************************************/
void RenderSystem::BeginLayerRendering(int layer) {
    switch (layer) {
    case 0: // Background layer
        // Example: Set background shader or blending mode
        glDisable(GL_BLEND);
        break;
    case 1: // Midground layer
        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case 2: // Foreground layer
        // Set up unique shader or lighting for foreground
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        break;
        // Add more cases for additional layers if needed


    case 3:
        // Set up unique shader or lighting for foreground
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        break;
        // Add more cases for additional layers if needed
    }
}
/***********************************************
 * @brief Renders debug outlines for entities.
 *
 * This function iterates over all stored outline models, applying the appropriate
 * transformations and rendering them on screen. It is used for debugging purposes
 * to visualize bounding boxes and entity boundaries.
 ***********************************************/
void RenderSystem::DrawOutlines() {
    // Only draw existing outline models, do not add new ones.
    for (auto& outline : HUGraphics::outlineModels) {
        //// std::cout<< "Drawing outline model with VAO ID: " << outline.vaoid << "\n";
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        outline.draw(modelMatrix, glm::ortho(0.0f, 1600.f, 900.f, 0.0f), cameraObj.GetViewMatrix());
    }
}
/***********************************************
 * @brief Toggles debug drawing mode.
 *
 * This function enables or disables debug drawing:
 * - If enabled, it generates outlines for all objects.
 * - If disabled, it clears all existing outlines.
 *
 * Debug mode is useful for visualizing entity boundaries and debugging collisions.
 ***********************************************/
void RenderSystem::toggleDebugDrawing() {
    if (debugDrawingEnabled) {
        // Debug drawing was enabled, so we are now disabling it
        HUGraphics::clearOutlineModels();  // Clear previous outlines
        debugDrawingEnabled = false;
    }
    else {
        // Debug drawing was disabled, so we are now enabling it
        //GenerateOutlines();  // Generate outlines for all models
        debugDrawingEnabled = true;
    }
}
/***********************************************
 * @brief Generates debug outlines for entities.
 *
 * This function:
 * - Clears previous outlines.
 * - Iterates through all entities to check if they have a `PhysicsBody` component.
 * - Computes their bounding box corners and creates debug lines for visualization.
 *
 * The outlines are drawn in red and provide a visual representation of entity boundaries.
 ***********************************************/
void RenderSystem::GenerateOutlines() {
    HUGraphics::clearOutlineModels();  // Clear previous outlines before generating new ones
    physicssystem.spatialGrid.clear(); // Clear old data
    for (auto& entity : mEntities) {
        if (!ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) continue;
        auto& body = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);
        physicssystem.spatialGrid.addEntity(entity, body.aabb.minX, body.aabb.minY, body.aabb.maxX, body.aabb.maxY);
    }

    std::vector<EntityID> allEntities = ECoordinator.GetAllEntities();

    EntityID thiefEntity = static_cast<EntityID>(-1);
    for (auto& entity : allEntities) {
        if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {
            auto& physBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);
            if (physBody.category == "Thief") {
                thiefEntity = entity;
            }
        }
    }

    auto& body = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(thiefEntity);

    std::vector<int> potentialCollisions = physicssystem.spatialGrid.getNearbyEntities(
        body.aabb.minX, body.aabb.minY, body.aabb.maxX, body.aabb.maxY
    );

    for (auto& entity : potentialCollisions) {
        if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {
            auto& physBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);
            auto& aabb = physBody.aabb;

            // Define corners of the bounding box using the retrieved AABB data
            Math2D::Vector2D bottomLeft(aabb.minX, aabb.minY);
            Math2D::Vector2D bottomRight(aabb.maxX, aabb.minY);
            Math2D::Vector2D topLeft(aabb.minX, aabb.maxY);
            Math2D::Vector2D topRight(aabb.maxX, aabb.maxY);

            // Generate outline models for the AABB
            HUGraphics::outlineModels.emplace_back(HUGraphics::lines_model(bottomLeft, bottomRight, glm::vec3(1.0f, 0.0f, 0.0f)));
            HUGraphics::outlineModels.emplace_back(HUGraphics::lines_model(bottomRight, topRight, glm::vec3(1.0f, 0.0f, 0.0f)));
            HUGraphics::outlineModels.emplace_back(HUGraphics::lines_model(topRight, topLeft, glm::vec3(1.0f, 0.0f, 0.0f)));
            HUGraphics::outlineModels.emplace_back(HUGraphics::lines_model(topLeft, bottomLeft, glm::vec3(1.0f, 0.0f, 0.0f)));
        }
    }
}


// if (InputSystem->IsMouseClicked(GLFW_MOUSE_BUTTON_LEFT)) {
     //auto [mouseX, mouseY] = CoreEngine::InputSystem::GetMousePosition();
     //// std::cout<< mouseX;
     //glm::vec3 mousePosition(static_cast<float>(mouseX), static_cast<float>(mouseY), 0.0f);

     //// std::cout<< "Mouse Pressed at: (" << mouseX << ", " << mouseY << ")\n";

     //float closestDistance = std::numeric_limits<float>::max(); // Initialize with max float value
     //// selectedEntity = std::nullopt; // Reset selected entity

     //// Check each entity for selection
     //if (!ECoordinator.HasComponent<Transform>(entity)) {
     //    // std::cout<< "Entity ID: " << entity << " does not have a Transform component.\n";
     //    continue; // Skip to the next entity
     //}

     //auto& transform = ECoordinator.GetComponent<Transform>(entity);
     //glm::vec3 entityPosition(transform.translate.x, transform.translate.y, 0.0f);
     //// std::cout<< "Checking entity ID: " << entity << " at position: (" << entityPosition.x << ", " << entityPosition.y << ")\n";

     //float distance = glm::distance(mousePosition, entityPosition);

     //// Select the closest entity within a specified threshold (e.g., 50 units)
     //if (distance < closestDistance && distance < 50.0f) {
     //    closestDistance = distance;
     //    selectedEntity = entity;  // Select the entity
     //    offset = entityPosition - mousePosition;  // Calculate the offset
     //}
 //    // std::cout<< "true";
 //}

 //if (selectedEntity) {
 //    isDragging = true;  // Start dragging
 //    // std::cout<< "Selected entity ID: " << selectedEntity << "\n";
 //}
 //else {
 //    // std::cout<< "No entity selected.\n"; // Feedback if no entity is selected
 //}


 //else if (CoreEngine::InputSystem::IsMouseReleased(GLFW_MOUSE_BUTTON_LEFT)) {
 //    if (selectedEntity) {
 //        // Optionally implement visual feedback reset for the selected entity here
 //        // std::cout<< "Released entity ID: " << *selectedEntity << "\n";
 //    }
 //    isDragging = false;  // Stop dragging
 //    selectedEntity = std::nullopt;  // Reset selected entity for next click
 //}



//void RenderSystem::DrawOutline(const AABB& aabb) {
//
//    std::vector<Math2D::Vector2D> outline;
//
//    // Define the four corners of the rectangle
//    Math2D::Vector2D topLeft = { aabb.minX, aabb.maxY };
//    Math2D::Vector2D topRight = { aabb.maxX, aabb.maxY };
//    Math2D::Vector2D bottomLeft = { aabb.minX, aabb.minY };
//    Math2D::Vector2D bottomRight = { aabb.maxX, aabb.minY };
//
//    outline.push_back(topLeft);
//    outline.push_back(topRight);
//    outline.push_back(bottomLeft);
//    outline.push_back(bottomRight);
//
//    glm::vec3 redColor = { 1.0f, 0.0f, 0.0f }; // Red outline
//
//    // Create an entity for the outline
//    EntityID outlineEntity = ECoordinator.CreateGameObject();
//
//    // Create line models for each edge of the rectangle
//    HUGraphics::GLModel topLine = HUGraphics::lines_model(topLeft, topRight, redColor);
//    HUGraphics::GLModel bottomLine = HUGraphics::lines_model(bottomLeft, bottomRight, redColor);
//    HUGraphics::GLModel leftLine = HUGraphics::lines_model(bottomLeft, topLeft, redColor);
//    HUGraphics::GLModel rightLine = HUGraphics::lines_model(bottomRight, topRight, redColor);
//
//    // Attach components for rendering
//    Transform transform;
//    transform.translate = { 0.0f, 0.0f, 1.0f };  // Keep z-index consistent
//    RenderLayer layer = RenderLayerType::GameObject;
//
//    // Add each line as a component
//    ECoordinator.AddComponent(outlineEntity, transform);
//    ECoordinator.AddComponent(outlineEntity, topLine);
//    /*ECoordinator.AddComponent(outlineEntity, bottomLine);
//    ECoordinator.AddComponent(outlineEntity, leftLine);
//    ECoordinator.AddComponent(outlineEntity, rightLine);*/
//    ECoordinator.AddComponent(outlineEntity, layer);
//
//    outline.clear();
//}

void RenderSystem::DrawOutline(const AABB& aabb) {
    // Step 1: Destroy all previously created outline entities
    for (EntityID entity : outlineEntities) {
        if (ECoordinator.HasComponent<HUGraphics::GLModel>(entity)) {
            ECoordinator.GetComponent<HUGraphics::GLModel>(entity).cleanup();
            glDeleteTextures(1, &ECoordinator.GetComponent<HUGraphics::GLModel>(entity).textureID);
            ECoordinator.DestroyGameObject(entity);
        }
    }
    outlineEntities.clear(); // Clear the global list

    // Step 2: Define the four corners of the AABB
    Math2D::Vector2D bottomLeft = { aabb.minX, aabb.minY };
    Math2D::Vector2D bottomRight = { aabb.maxX, aabb.minY };
    Math2D::Vector2D topRight = { aabb.maxX, aabb.maxY };
    Math2D::Vector2D topLeft = { aabb.minX, aabb.maxY };

    // Step 3: Create line models for each edge of the AABB
    std::vector<HUGraphics::GLModel> outlineModels = {
        HUGraphics::lines_model(bottomLeft, bottomRight, {1.0f, 0.0f, 0.0f}),  // Bottom edge
        HUGraphics::lines_model(bottomRight, topRight, {1.0f, 0.0f, 0.0f}),    // Right edge
        HUGraphics::lines_model(topRight, topLeft, {1.0f, 0.0f, 0.0f}),        // Top edge
        HUGraphics::lines_model(topLeft, bottomLeft, {1.0f, 0.0f, 0.0f})       // Left edge
    };

    // Step 4: Create and register entities for each line segment
    for (const auto& model : outlineModels) {
        EntityID outlineEntity = ECoordinator.CreateGameObject();

        Transform transform;
        transform.translate = { 0.0f, 0.0f, 1.0f }; // Keep z-index consistent
        RenderLayer layer = RenderLayerType::GameObject;

        ECoordinator.AddComponent(outlineEntity, transform);
        ECoordinator.AddComponent(outlineEntity, model);
        ECoordinator.AddComponent(outlineEntity, layer);

        outlineEntities.push_back(outlineEntity); // Store the entity ID for cleanup
    }
}

//void RenderSystem::DrawResizingPoints(const AABB& aabb) {
//    float size = 5.0f; // Size of the handles
//
//    std::vector<Math2D::Vector2D> points = {
//        {aabb.minX, aabb.minY}, // Top-left
//        {aabb.maxX, aabb.minY}, // Top-right
//        {aabb.minX, aabb.maxY}, // Bottom-left
//        {aabb.maxX, aabb.maxY}, // Bottom-right
//        {(aabb.minX + aabb.maxX) / 2, aabb.minY}, // Top-center
//        {(aabb.minX + aabb.maxX) / 2, aabb.maxY}, // Bottom-center
//        {aabb.minX, (aabb.minY + aabb.maxY) / 2}, // Left-center
//        {aabb.maxX, (aabb.minY + aabb.maxY) / 2}  // Right-center
//    };
//
//    for (const auto& point : points) {
//        //DrawRectangle
//    }
//}

int RenderSystem::GetHoveredHandle(const AABB& aabb, float mouseX, float mouseY) {
    float size = 5.0f;
    std::vector<Math2D::Vector2D> points = {
        {aabb.minX, aabb.minY}, {aabb.maxX, aabb.minY}, {aabb.minX, aabb.maxY}, {aabb.maxX, aabb.maxY},
        {(aabb.minX + aabb.maxX) / 2, aabb.minY}, {(aabb.minX + aabb.maxX) / 2, aabb.maxY},
        {aabb.minX, (aabb.minY + aabb.maxY) / 2}, {aabb.maxX, (aabb.minY + aabb.maxY) / 2}
    };

    for (int i = 0; i < points.size(); ++i) {
        if (mouseX >= points[i].x - size && mouseX <= points[i].x + size &&
            mouseY >= points[i].y - size && mouseY <= points[i].y + size) {
            return i; // Return the index of the handle being hovered
        }
    }
    return -1; // No handle is hovered
}

//void RenderSystem::UpdateAABB(AABB& aabb, float mouseX, float mouseY, bool isDragging) {
//    if (!isDragging) return;
//
//    switch (selectedHandle) {
//    case 0: aabb.minX = mouseX; aabb.minY = mouseY; break; // Top-left
//    case 1: aabb.maxX = mouseX; aabb.minY = mouseY; break; // Top-right
//    case 2: aabb.minX = mouseX; aabb.maxY = mouseY; break; // Bottom-left
//    case 3: aabb.maxX = mouseX; aabb.maxY = mouseY; break; // Bottom-right
//    case 4: aabb.minY = mouseY; break; // Top-center
//    case 5: aabb.maxY = mouseY; break; // Bottom-center
//    case 6: aabb.minX = mouseX; break; // Left-center
//    case 7: aabb.maxX = mouseX; break; // Right-center
//    }
//}

void RenderSystem::HandleMouseEvents(float mouseX, float mouseY, bool isMouseDown) {
    if (isMouseDown) {
        if (selectedHandle == -1) {
            // Check if clicking on any AABB first
            for (auto& entity : ECoordinator.GetAllEntities()) {
                if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {
                    auto& body = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);
                    auto& aabb = body.aabb;

                    int handle = GetHoveredHandle(aabb, mouseX, mouseY);
                    if (handle != -1) {
                        selectedAABB = aabb;  // Store the selected AABB
                        hasSelectedAABB = true;
                        selectedHandle = handle;
                        break; // Select only the first found
                    }
                }
            }
        }
        else {
            if (hasSelectedAABB) {
                //UpdateAABB(selectedAABB, mouseX, mouseY, true);
            }
        }
    }
    else {
        selectedHandle = -1; // Reset when mouse is released
    }
}


void RenderSystem::ClearLinesAndPoints() {
    // Iterate through all entities in ECS and find those with line or point models
    std::vector<EntityID> entitiesToRemove;

    for (EntityID entity : ECoordinator.GetAllEntities()) {
        if (ECoordinator.HasComponent<HUGraphics::GLModel>(entity)) {
            const HUGraphics::GLModel& model = ECoordinator.GetComponent<HUGraphics::GLModel>(entity);

            // Check if the model is a line or a point
            if (model.primitive_type == GL_LINES || model.primitive_type == GL_POINTS) {
                entitiesToRemove.push_back(entity);
            }
        }
    }

    // Remove all identified entities
    for (EntityID entity : entitiesToRemove) {
        ECoordinator.DestroyGameObject(entity);
    }
}

// Plays the relevant background/foreground sounds for cutscenes

void playCutsceneSound(size_t i) {
    /*if (!isSoundPlayed) {*/
    if (!audioEngine->isPlaying(foregroundSounds[i])) {
        audioEngine->PlaySound(foregroundSounds[i], 0, 0.2f * musicVolume);
    }
    /*if (!audioEngine->isPlaying(backgroundSounds[i])) {
        audioEngine->PlaySound(backgroundSounds[i], 0, 0.1f);
    }*/
    // Set the flag to true after playing the sound
    //isSoundPlayed = true;
/*}*/

    if (i > 0) {
        if (audioEngine->isPlaying(foregroundSounds[i - 1])) {
            audioEngine->StopSound(foregroundSounds[i - 1]);
        }

        /*if (audioEngine->isPlaying(backgroundSounds[i - 1])) {
            audioEngine->StopSound(backgroundSounds[i - 1]);
        }*/
    }
}