/**
 * @file Mouse.cpp
 * @brief Implementation of mouse interaction functions for selecting and dragging entities in a 2D scene.
 *
 * This file defines functionality to handle mouse inputs using GLFW and ImGui,
 * allowing the selection and dragging of entities based on their proximity to the mouse cursor.
 *
 * Author: Lewis (100%)
 */

#include <GL/glew.h>
#include "Mouse.h"
#include <optional>  // For std::optional
#include "EntityManager.h"
#include "GlobalVariables.h"
#include "ImguiManager.h"
#include "vector3d.h"

namespace Mouse {
    bool isDragging = false;                       // Flag to indicate dragging state
    Math3D::Vector3D lastMousePos;                 // To store last mouse position
    std::optional<EntityID> selectedEntity;        // Store the currently selected entity
    Math3D::Vector3D offset;                       // Offset from mouse position to entity position

    std::vector<EntityID> gEntities;

    /**
     * @brief Handles mouse button input for selecting and dragging entities.
     *
     * This function captures mouse button presses and releases. On press, it identifies
     * the closest entity to the mouse cursor within a defined threshold and marks it for dragging.
     * On release, it stops dragging and deselects the entity.
     *
     * @param window Pointer to the GLFW window.
     * @param button The mouse button being interacted with.
     * @param action The action being performed (GLFW_PRESS or GLFW_RELEASE).
     * @param mods Modifier keys active during the button press/release.
     */
    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        // Check if ImGui is capturing the mouse
        (void)mods;
        if (ImGui::GetIO().WantCaptureMouse) {
            // ImGui is capturing the mouse input, exit early
            return;
        }

        // Handle mouse button actions only when left button is pressed
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            Math3D::Vector3D mousePosition(static_cast<float>(mouseX), static_cast<float>(mouseY), 0.0f);

            if (action == GLFW_PRESS) {
                float closestDistance = std::numeric_limits<float>::max();

                // Replace gEntities with your actual entity collection
                for (auto& entity : gEntities) {
                    if (!ECoordinator.HasComponent<Transform>(entity)) {
                        continue; // Skip to the next entity
                    }

                    auto& transform = ECoordinator.GetComponent<Transform>(entity);
                    Math3D::Vector3D entityPosition(transform.translate.x, transform.translate.y, 0.0f);

                    float distance = (mousePosition - entityPosition).Length(); 

                    // Select the closest entity within a specified threshold (e.g., 50 units)
                    if (distance < closestDistance && distance < 50.0f) {
                        closestDistance = distance;
                        selectedEntity = entity;  // Select the entity
                        offset = entityPosition - mousePosition;  // Calculate the offset
                    }
                }

                if (selectedEntity) {
                    isDragging = true;  // Start dragging
                }
            }
            else if (action == GLFW_RELEASE) {
                isDragging = false;  // Stop dragging
                selectedEntity = std::nullopt;  // Reset selected entity for next click
            }
        }
    }

    /**
     * @brief Handles mouse position changes for updating entity positions while dragging.
     *
     * This function is invoked whenever the mouse cursor moves. If an entity is selected
     * and being dragged, it updates the entity's position based on the mouse cursor's movement.
     *
     * @param window Pointer to the GLFW window.
     * @param xpos Current x-coordinate of the mouse cursor.
     * @param ypos Current y-coordinate of the mouse cursor.
     */
    void MousePositionCallback(GLFWwindow*, double xpos, double ypos) {
        if (isDragging && selectedEntity) {
            Math3D::Vector3D mousePosition(static_cast<float>(xpos), static_cast<float>(ypos), 0.0f);

            // Calculate the new position by adding the offset
            Math3D::Vector3D newPosition = mousePosition + offset;

            // Update the position of the selected entity directly
            if (ECoordinator.HasComponent<Transform>(*selectedEntity)) {
                auto& transform = ECoordinator.GetComponent<Transform>(*selectedEntity);
                transform.translate = static_cast<glm::vec3>(newPosition); // Directly set the new position
            }
        }
    }

    /**
     * @brief Initializes mouse callbacks and associates them with a GLFW window.
     *
     * This function sets up the required mouse button and cursor position callbacks
     * for handling entity selection and dragging.
     *
     * @param window Pointer to the GLFW window.
     * @param entities A vector of entity IDs that can be interacted with.
     */
    void InitMouseCallbacks(GLFWwindow* window, const std::vector<EntityID>& entities) {
        gEntities = entities;

        // Set mouse button callback
        glfwSetMouseButtonCallback(window, MouseButtonCallback);

        // Set cursor position callback
        glfwSetCursorPosCallback(window, MousePositionCallback);
    }
}
