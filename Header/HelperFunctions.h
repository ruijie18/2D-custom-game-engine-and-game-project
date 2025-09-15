/**
 * @file HelperFunctions.h
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

#pragma once
#ifndef  HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H
#include "Graphics.h"


 // Function to fade in an object
void FadeInObject(EntityID entity, float fadeDuration=2.0f);
void FadeOutAllObjects(float fadeDuration=3.0f);

void UpdateFadeInEffects(float deltaTime);

void UpdateFadeEffects(float deltaTime);

bool IsAreaClicked(double mouseX, double mouseY, float centerX, float centerY, float width, float height);

bool IsScaledAreaClicked(float mouseX, float mouseY, float areaX, float areaY, float areaWidth, float areaHeight,
    const ImVec2& texturePos, float Xscale, float Yscale);

glm::vec2 worldToCameraCoordinates(const glm::vec2& worldCoords);

glm::vec2 cameraToWorldCoordinates(const glm::vec2& cameraCoords);

std::pair<int, int> getScreenDimensions();

            std::pair<float, float> getScaledMousePos();

            void freezeTimer();
            void ContinueTimer();

            void       resumeDaGame();

            void FadeOutObject(EntityID entity, float fadeDuration=2.0f);


            //code to skip stages
            void cycleLevels(int& stage);


            /*void RenderImageWith
            (Image& image, float alpha);*/


#endif // ! HELPER_FUNCTIONS_H
