/**
 * @file ListOfComponents.h
 * @brief Header file defining various components, enums, and utilities for the game engine.
 *
 * This file provides declarations for essential components and data structures used within the game engine,
 * such as `Transform`, `Visibility`, `UIButton`, and `RenderLayer`. It also defines helper classes like
 * `Camera2D` for managing 2D camera functionality and enumerations for game states and rendering layers.
 *
 * Key Features:
 * - **Game State Management**: Includes the `GameState` enum to define different states like MainMenu, Tutorial, and Playing.
 * - **Component Definitions**: Provides structures for various entity components:
 *   - `Transform`: Handles position, rotation, and scale.
 *   - `Visibility`: Tracks whether an entity is visible.
 *   - `UIButton`: Manages interactive UI button states and click callbacks.
 *   - `RenderLayer`: Organizes entities into rendering layers (e.g., background, UI).
 *   - `ObjectSize`, `PointsComponent`, and `Name`: Represent additional properties for entities.
 * - **Camera Control**: Implements a 2D camera (`Camera2D`) for centering on characters and managing zoom.
 * - **Utility Functions**: Declares helper functions for UI handling, such as detecting mouse-over events
 *   and managing the state of an exit button.
 *
 * Key Classes and Structures:
 * - **Transform**: A reusable component for handling an entity's position, rotation, and scaling.
 * - **Camera2D**: Supports camera panning, zooming, and centering on entities.
 * - **UIButton**: Provides functionality for interactive UI buttons, including hover and click detection.
 *
 * Key Enums:
 * - `GameState`: Defines states like MainMenu, Playing, and Pause.
 * - `shapeType`: Enumerates shapes like circle, triangle, rectangle, and text texture.
 * - `RenderLayerType`: Categorizes layers like Background, GameObject, and UI.
 *
 * Utility Functions:
 * - `initializeExitButton`, `resetExitButton`, and `handleExitButtonClick`: Manage the functionality of the exit button.
 * - `isMouseOverRectangle`: Checks if the mouse is over a rectangular area.
 *
 * Author: Che Ee (50%)
 * Co-Author: Everyone(50%)
 */



#pragma once

#ifndef LIST_OF_COMPONENTS_H
#define LIST_OF_COMPONENTS_H
#include <chrono>

#include "vector3d.h"
#include <glm/gtc/type_ptr.hpp>
#include <string>
#ifdef _DEBUG
#undef _DEBUG
#include <python.h>
#define _DEBUG
#else
#include <python.h>
#endif
//#include <EntityManager.h>
//#include <GameLogic.h>

enum TransitionState {
	NoTransition,
	FadingOut,
	FadingIn
};

struct TransitionStateManager {
	TransitionState transState=NoTransition;
	float alpha = 0;
	int nextStage = -1;
	float transSpeed = 1.0f;
};

enum GameState {
	MainMenu = 0,
	Playing = 1,
	Lose = 2,
	LevelSelect = 3,
	Pause = 6,
	HowToPlay = 7,
	confirmQuit=8,
	HowToPlay2=9,
	confirmQuit2=10,
	Playing1 = 11,
	Playing2 = 12,
	Playing3 = 13,
	Credit = 15,
	Settings = 16,
	NotApplicable=69,
	cutScene,
	endScene,
	gameWon,
	starRating,
	splashscreen,
};

struct Transform {
	glm::vec3 scale;
	float rotate=0;
	glm::vec3 translate;

	// Constructor to initialize scale, rotate, and translate
	Transform(const glm::vec3& initScale = glm::vec3(1.0f), // Default scale (1,1,1)
		float initRotate = 0.0f,                      // Default rotation (0 degrees)
		const glm::vec3& initTranslate = glm::vec3(0.0f)) // Default translation (0,0,0)
		: scale(initScale), rotate(initRotate), translate(initTranslate) {}

	// Copy assignment operator
	Transform& operator=(const Transform& other) {
		if (this == &other) {
			return *this; // Check for self-assignment
		}

		// Copy values from the other instance
		scale = other.scale;
		rotate = other.rotate;
		translate = other.translate;

		return *this; // Return the current object to allow chaining
	}
};



struct PointsComponent {
	std::vector<glm::vec2> points;
};

struct ObjectSize {
	float size;
	ObjectSize(float sz) :size(sz) {

	}
	//hardcode to 100
	ObjectSize() :size(100) {

	}

};


enum shapeType {
	circle,
	triangle,
	rectangle,
	texture,
	line,
	point,
	texture_animation,
	text_texture
};

enum class RenderLayerType {
	//0 for background, 1 for game object, 2 for UI(pause,menu)
	Background = 0,
	GameObject = 1,
	UI = 2,
	MainMenuUI=3
};

struct RenderLayer {
	RenderLayerType layer;

	RenderLayer(RenderLayerType layer = RenderLayerType::GameObject) : layer(layer) {}
};

struct Name {
	std::string name;
};

class Camera2D {
public:

	glm::mat4 viewMatrix{};

	//need to change this accoeding to fullscreen or not
	glm::vec2 screenCenter{800.0f,450.0f};
	float zoomLevel = 1.0f;  // New zoom level
	Camera2D();

	// Initialize with screen center for proper centering
	Camera2D(float screenWidth, float screenHeight);

	void CenterOnCharacter(const glm::vec2& characterPosition);
	void zoomIn(float amount);

	void zoomOut(float amount);
	glm::mat4 GetViewMatrix() const;

};

struct AnimationData {
	float frametime;     // Duration of each frame
	int rows;            // Number of rows in the sprite sheet
	int columns;         // Number of columns in the sprite sheet
	int totalFrames;     // Total number of frames in the animation
};



struct LaserComponent {
	bool isActive;       // Whether the laser is active
	float activeTime;    // Duration the laser stays active
	float inactiveTime;  // Duration the laser stays inactive
	float timer;         // Tracks the elapsed time
	bool turnedOn;	 // Permanently sets inactive timer to 99s
	std::string linkModuleID; // Default to no link/ each laser only has one module linked to it

};

class Timer {
public:
	Timer(int seconds);
	Timer();
	bool isPause();
	void Pause();
	void Resume();
	void Reset();

	void changeDuration(int seconds);

	int GetTimeRemaining() const;
	bool IsTimeUp() const;

private:
	int duration; // Duration of the timer in seconds
	int timeRemaining; // Remaining time in seconds
	bool paused; // Whether the timer is paused
	std::chrono::steady_clock::time_point startTime; // Time when the timer started
};




#endif 