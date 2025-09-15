/**
 * @file Render.h
 * @brief Implementation of the Render System for managing rendering processes and updates.
 *
 * This header file defines the `RenderSystem` class, which handles initialization, updates,
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
 * Author: Che Ee (90%)
 * Co-Author: Rui Jie (10%)
 */

#pragma once
#ifndef RENDER_H
#define RENDER_H

#include "GlobalVariables.h"
#include "CommonIncludes.h"
#include "Coordinator.h"
#include "SystemsManager.h"
#include "Graphics.h"
#include "ImguiManager.h"
#include "ListOfComponents.h"
#include "Volume.h"
#include "matrix4x4.h"

static bool hasSeenCutscene = false;
extern int currentMasterVolume;
extern int currentSFXVolume;
extern int currentMusicVolume;

class RenderSystem :public System, public CoreEngine::Observer {
private:
	HUGraphics graphics;
public:
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
	void Init()override;
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
	void Update(double deltaTime)override;
	/***********************************************
 * @brief Retrieves the name of the Render System.
 *
 * This function returns a constant character pointer representing
 * the name of the system. It is primarily used for identification
 * and debugging purposes.
 *
 * @return The name of the system as a C-string ("RenderSystem").
 ***********************************************/
	const char* getName() const override {
		return "RenderSystem"; // Return the name of the system
	}
	/***********************************************
 * @brief Renders debug outlines for entities.
 *
 * This function iterates over all stored outline models, applying the appropriate
 * transformations and rendering them on screen. It is used for debugging purposes
 * to visualize bounding boxes and entity boundaries.
 ***********************************************/
	void DrawOutlines();

	/***********************************************
 * @brief Toggles debug drawing mode.
 *
 * This function enables or disables debug drawing:
 * - If enabled, it generates outlines for all objects.
 * - If disabled, it clears all existing outlines.
 *
 * Debug mode is useful for visualizing entity boundaries and debugging collisions.
 ***********************************************/
	void toggleDebugDrawing();
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
	void GenerateOutlines();
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
	void BeginLayerRendering(int layer);
	/***********************************************
 * @brief Handles incoming messages for the Render System.
 *
 * This function processes messages received by the Render System
 * and acts accordingly. Currently, it:
 * - Calls `RenderObjectHandler` when a `RenderObject` message is received.
 * - Ignores other message types but can be expanded in the future.
 *
 * @param message Pointer to the received `IMessage` object.
 ***********************************************/
	void HandleMessage(CoreEngine::IMessage* message) override {
		//can add more in the future
		switch (message->GetMessageID()) {
		case CoreEngine::RenderObject:
			RenderObjectHandler(message);
			break;
		default:
			break;
		}
	}

	void DrawOutline(const AABB& aabb);

	//void DrawResizingPoints(const AABB& aabb);

	int GetHoveredHandle(const AABB& aabb, float mouseX, float mouseY);

	void UpdateAABB(AABB& aabb, float mouseX, float mouseY);

	void HandleMouseEvents(float mouseX, float mouseY, bool isMouseDown);

	void ClearLinesAndPoints();

	void ToggleFPSDisplay();
	void UpdateFPS();

private:
	bool showFPS = false;
	double previousTime = glfwGetTime();
	int frameCount = 0;
	int fps = 0;

	//EntityID fpsEntity = std::numeric_limits<EntityID>::max();;  // Entity ID for FPS text

	/***********************************************
 * @brief Handles rendering-related messages.
 *
 * This static function processes messages that request
 * object rendering. It currently outputs a debug message
 * and can be extended with additional rendering logic.
 *
 * @param message Pointer to the received `IMessage` object.
 ***********************************************/

	 static void RenderObjectHandler([[maybe_unused]] CoreEngine::IMessage* message) {
	}

};

void ResetHoverScaling();
void UpdateFadeOut(float deltaTime);
void playCutsceneSound(size_t i);



#endif