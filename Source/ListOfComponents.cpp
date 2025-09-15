/**
 * @file Camera2D.cpp
 * @brief Implementation of the `Camera2D` class and helper functions for managing camera and UI behavior.
 *
 * This file provides the implementation of the `Camera2D` class for handling 2D camera operations such as centering
 * on a character, zooming, and retrieving the view matrix. It also includes utility functions for managing the
 * "Exit" button's behavior and detecting mouse interactions with UI elements.
 *
 * Key Features:
 * - **Camera Operations**:
 *   - `CenterOnCharacter`: Centers the camera on a character's position, with optional static behavior when inactive.
 *   - `GetViewMatrix`: Retrieves the current view matrix of the camera.
 *   - `zoomIn` and `zoomOut`: Adjust the camera's zoom level dynamically.
 * - **UI Button Management**:
 *   - `initializeExitButton`: Initializes the "Exit" button with default properties and adds it to the ECS.
 *   - `resetExitButton`: Resets the "Exit" button to its default state and position.
 *   - `handleExitButtonClick`: Handles mouse clicks on the "Exit" button and toggles its state.
 * - **Mouse Interaction**:
 *   - `isMouseOverRectangle`: Checks if the mouse is hovering over a rectangular area.
 *
 * Functions:
 * - **Camera2D**:
 *   - `Camera2D()`: Default constructor.
 *   - `Camera2D(float screenWidth, float screenHeight)`: Constructor with optional screen dimensions.
 *   - `CenterOnCharacter`: Centers the camera on a given position, with zoom scaling.
 *   - `GetViewMatrix`: Returns the current view matrix.
 *   - `zoomIn` and `zoomOut`: Adjust the zoom level with safeguards against invalid values.
 * - **UI Functions**:
 *   - `initializeExitButton`: Sets up the "Exit" button and registers it in the ECS.
 *   - `resetExitButton`: Resets the button's state and transform to defaults.
 *   - `handleExitButtonClick`: Handles click events and checks for collisions with the button.
 *   - `isMouseOverRectangle`: Determines if a mouse position is within a specified rectangle.
 *
 * Author: Che Ee (100%)
 */


#include "ListOfComponents.h"
#include "CommonIncludes.h"
#include <GlobalVariables.h>


 // Default constructor, initializes the timer with a default duration (e.g., 180 seconds)
Timer::Timer()
	: duration(180), timeRemaining(180), paused(true) {


}

Camera2D::Camera2D()
{
	/*screenCenter = glm::vec2(screen_width / 2.0f, screen_height / 2.0f);
	// std::cout<< screen_width / 2;*/
}

Camera2D::Camera2D([[maybe_unused]] float screenWidth, [[maybe_unused]] float screenHeight)
{
	/*screenCenter = glm::vec2(screen_width / 2.0f, screen_height / 2.0f);
	// std::cout<< screen_width / 2;*/
}

void Camera2D::CenterOnCharacter(const glm::vec2& characterPosition) {

	if (cameraActive==false) {
		// Calculate the new offset based on the current character position
		glm::vec2 offset = (screenCenter- glm::vec2(800,450) ) *zoomLevel ;
		viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(offset, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(zoomLevel, zoomLevel, 1.0f));
	}

	else {
		 // Calculate the new offset based on the current character position
	glm::vec2 offset = (screenCenter - characterPosition)*zoomLevel;
	viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(offset, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(zoomLevel, zoomLevel, 1.0f));
	}

   
}

glm::mat4 Camera2D::GetViewMatrix() const
{
	return viewMatrix;

}

void Camera2D::zoomIn(float amount)
{
	zoomLevel = std::max(0.1f, zoomLevel - amount);  // prevent zoom level going to zero or negative
}

void Camera2D::zoomOut(float amount)
{
	zoomLevel += amount;
}


bool isMouseOverRectangle(double mouseX, double mouseY, float rectX, float rectY, float rectWidth, float rectHeight) {
	return (mouseX >= rectX && mouseX <= rectX + rectWidth &&
		mouseY >= rectY && mouseY <= rectY + rectHeight);
}

Timer::Timer(int seconds)
	: duration(seconds), timeRemaining(seconds), paused(true) {}


void Timer::Pause() {
	if (!paused) {
		auto now = std::chrono::steady_clock::now();
		auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
		timeRemaining -= static_cast<int>(elapsedSeconds); // Explicit cast to int
		if (timeRemaining < 0) timeRemaining = 0;
		paused = true;
	}
}

void Timer::Resume() {
	if (paused) {
		paused = false;
		startTime = std::chrono::steady_clock::now();
	}
}
bool Timer::isPause() {
	return paused;
 }


void Timer::Reset() {
	paused = true;
	timeRemaining = duration;
}

//used after a change in level
void Timer::changeDuration(int seconds) {
	duration = seconds;
	timerObj.Reset();
	timerObj.Resume();
}

int Timer::GetTimeRemaining() const {
	if (paused) {
		return timeRemaining;
	}
	else {
		auto now = std::chrono::steady_clock::now();
		int elapsedTime = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count()); // Explicit cast to int
		return timeRemaining - elapsedTime > 0 ? timeRemaining - elapsedTime : 0;
	}
}
bool Timer::IsTimeUp() const {
	return GetTimeRemaining() <= 0;
}