/**
 * @file HelperFunctions.cpp
 * @brief Implementation of utility functions for input handling, coordinate conversions, and screen management.
 *
 * This file provides helper functions to assist with common tasks in the game engine, including:
 * - Input handling (checking for clicks and scaling mouse coordinates).
 * - Coordinate conversions between world and camera spaces.
 * - Screen management, including retrieving screen dimensions.
 * - Timer control for pausing and resuming gameplay.
 *
 * Key Features:
 * - **Input Handling**:
 *   - `IsAreaClicked`: Determines whether a specific rectangular area is clicked based on mouse coordinates.
 *   - `getScaledMousePos`: Retrieves the scaled mouse position relative to the design resolution.
 * - **Coordinate Conversion**:
 *   - `worldToCameraCoordinates`: Converts world coordinates to camera-relative coordinates.
 *   - `cameraToWorldCoordinates`: Converts camera-relative coordinates back to world coordinates.
 * - **Screen Management**:
 *   - `getScreenDimensions`: Retrieves the current screen resolution, considering fullscreen or windowed mode.
 * - **Timer Control**:
 *   - `freezeTimer`: Pauses the game and stores elapsed time.
 *   - `ContinueTimer`: Resumes the game and timer.
 *
 * Functions:
 * - `freezeTimer`:
 *   - Pauses the game and stops the active timer.
 * - `ContinueTimer`:
 *   - Resumes the game and restarts the timer.
 * - `IsAreaClicked`:
 *   - Parameters:
 *     - `mouseX`, `mouseY`: Current mouse coordinates.
 *     - `centerX`, `centerY`: Center of the rectangle.
 *     - `width`, `height`: Dimensions of the rectangle.
 *   - Returns `true` if the mouse coordinates are within the bounds of the rectangle.
 * - `worldToCameraCoordinates`:
 *   - Converts given world coordinates to coordinates relative to the camera's view.
 * - `cameraToWorldCoordinates`:
 *   - Converts given camera-relative coordinates back to world coordinates.
 * - `getScreenDimensions`:
 *   - Retrieves the current screen resolution based on fullscreen or windowed mode.
 * - `getScaledMousePos`:
 *   - Scales the mouse position from screen resolution to the design resolution (e.g., 1600x900).
 *
 * Author: Che Ee (100%)
 */
#include "GlobalVariables.h"

#include "HelperFunctions.h"
#include <InputSystem.h>

 // Function to fade in an object
void FadeInObject(EntityID entity, float fadeDuration) {
    // Add or update fading variables
    auto& model = ECoordinator.GetComponent<HUGraphics::GLModel>(entity);
    model.fadeTimer = 0.0f;          // Start fade timer at 0
    model.fadeDuration = fadeDuration; // Store total fade duration
    model.isFadingIn = true;        // Flag to indicate fading in is active
    model.alpha = 0.0f;             // Start fully transparent
}

void UpdateFadeEffects(float deltaTime) {
    auto allEntities = ECoordinator.GetAllEntities(); // Get all entities
    for (auto& entity : allEntities) {
        auto& model = ECoordinator.GetComponent<HUGraphics::GLModel>(entity);

        // Check if the model is fading
        if (model.isFading) {
            model.fadeTimer -= deltaTime; // Decrease the fade timer

            if (model.fadeTimer > 0.0f) {
                // Smooth fade-out effect with easing (ease-out formula)
                float t = 1.0f - (model.fadeTimer / model.fadeDuration); // Normalized time
                model.alpha = 1.0f - (t * t * (3.0f - 2.0f * t)); // Ease-out interpolation
            }

            //fade out and destroy the object ,and subsequently start fading in the new object
            else {
                // Fade complete: make the object fully transparent and stop fading
                model.alpha = 0.0f;
                model.isFading = false;

                //Destroy object after fading out
                ECoordinator.DestroyGameObject(entity);

                sceneVector.pop_back();
                //cutsceneIncrement += 1;

                /*isSoundPlayed = false;*/
                //std::cout<<"CutScene Increment"<<cutsceneIncrement<<"\n";

                //check if scene vector has element

                if (sceneVector.size() > 0) {
                    EntityID sceneToFadeIn = sceneVector.back().first;
                    
                    FadeInObject(sceneToFadeIn);

                }


            }
        }

        // Check if this entity is in the process of fading in
        if (model.isFadingIn) {
            // Increment the fade timer by deltaTime
            model.fadeTimer += deltaTime;

            // Ensure the fade timer does not exceed the fade duration
            if (model.fadeTimer < model.fadeDuration) {
                // Normalize time (0 to 1)
                float t = model.fadeTimer / model.fadeDuration;

                // Apply easing formula for fade-in effect
                model.alpha = t * t * (3.0f - 2.0f * t); // Ease-in formula
            }
            else {
                // Fade-in complete: set alpha to 1 and stop fading
                model.alpha = 1.0f;
                model.isFadingIn = false;
            }
        }

    }
}


void FadeOutObject(EntityID entity, float fadeDuration) {
    // Add or update fading variables
    auto& model = ECoordinator.GetComponent<HUGraphics::GLModel>(entity);
    model.fadeTimer = fadeDuration;  // Track remaining fade duration
    model.fadeDuration = fadeDuration; // Store total fade duration
    model.isFading = true;           // Flag to indicate fading is active
}

bool IsAreaClicked(double mouseX, double mouseY, float centerX, float centerY, float width, float height) {
    // Correctly calculate the rectangle bounds
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    return (mouseX >= (centerX - halfWidth) && mouseX <= (centerX + halfWidth) &&
        mouseY >= (centerY - halfHeight) && mouseY <= (centerY + halfHeight));
}

bool IsScaledAreaClicked(float mouseX, float mouseY, float areaX, float areaY, float areaWidth, float areaHeight,
    const ImVec2& texturePosition, float Xscale, float Yscale) {
    // Adjust the area to match the scaled and offset FBO
    float scaledX = texturePosition.x + areaX * Xscale;
    float scaledY = texturePosition.y + areaY * Yscale;
    float scaledWidth = areaWidth * Xscale;
    float scaledHeight = areaHeight * Yscale;

    // Check if mouse position is within the adjusted area
    return mouseX >= scaledX && mouseX <= (scaledX + scaledWidth) &&
        mouseY >= scaledY && mouseY <= (scaledY + scaledHeight);
}

glm::vec2 worldToCameraCoordinates(const glm::vec2& worldCoords) {
    // The world-to-camera conversion
    float cameraX = worldCoords.x - 800.0f;
    float cameraY = worldCoords.y - 450.0f;

    return glm::vec2(cameraX, cameraY);
}


glm::vec2 cameraToWorldCoordinates(const glm::vec2& cameraCoords) {
    // The camera-to-world conversion
    float worldX = cameraCoords.x + 800.0f;
    float worldY = cameraCoords.y + 450.0f;

    return glm::vec2(worldX, worldY);
}

std::pair<int, int> getScreenDimensions()
{
    // Get the primary monitor
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    // Determine active resolution
    int currentWidth = isFullscreen ? mode->width : 1600; // Use fullscreen or windowed resolution
    int currentHeight = isFullscreen ? mode->height : 900;
    return std::pair<int, int>(currentWidth, currentHeight);
}

std::pair<float, float> getScaledMousePos()
{
    auto screenDim = getScreenDimensions();
    auto mousePosition = CoreEngine::InputSystem::GetMousePosition();

    // Normalize mouse position
    float normalizedMouseX = static_cast<float>(mousePosition.first) / screenDim.first;
    float normalizedMouseY = static_cast<float>(mousePosition.second) / screenDim.second;

    // Scale to design resolution (e.g., 1600x900 as your design resolution)
    float scaledMouseX = normalizedMouseX * 1600; // Design resolution width
    float scaledMouseY = normalizedMouseY * 900;  // Design resolution height
    return std::pair<float, float>(scaledMouseX, scaledMouseY);
}


void cycleLevels(int& stage) {
    
    if (stage == Playing1) {
        //logic for changing level as well as resetting the levels and stuff
        stage = Playing3;
    }

    else if (stage == Playing3) {
        stage = Playing2;
    }

    else if (stage == Playing2) {
        stage = Playing;
    }

    //transition to gameWon
    else if (stage == Playing) {
        ECoordinator.FadeOutAllObjects();
        stage = gameWon;
        
    }

    sceneVector.clear();
    CoreEngine::InputSystem::Stage = stage;

    Object_picked = 0;
    timerObj.Reset();
    
    CreateObjectsForStage(stage);
    
    if (stage == gameWon)
        FadeInObject(sceneVector.back().first);
    
    //reset played level to 0




    CoreEngine::InputSystem::LevelPlayed = 0;


}