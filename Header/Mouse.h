/**
 * @file Mouse.h
 * @brief Implementation of mouse interaction functions for selecting and dragging entities in a 2D scene.
 *
 * This file defines functionality to handle mouse inputs using GLFW and ImGui,
 * allowing the selection and dragging of entities based on their proximity to the mouse cursor.
 *
 * Author: Lewis (100%)
 */

#pragma once
#ifndef MOUSE_H
#define MOUSE_H

#include <GLFW/glfw3.h>
#include "vector3d.h" 
#include "EntityManager.h"

namespace Mouse {
    extern bool isDragging;                  // Flag to indicate dragging state

    //static std::vector<EntityID> GenerateEntities(int numberOfEntities);
    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    void MousePositionCallback(GLFWwindow* window, double xpos, double ypos);
    void InitMouseCallbacks(GLFWwindow* window, const std::vector<EntityID>& entities);
}



#endif // MOUSE_H