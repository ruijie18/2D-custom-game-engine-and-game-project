#pragma once
#include <functional>

/**
 * @file ButtonComponent.h
 * @brief Declaration of the ButtonComponent struct.
 *
 * This file contains the definition of the ButtonComponent struct, which represents
 * a button in a system. It encapsulates properties such as the action to perform,
 * a callback function for click events, and hover state tracking.
 *
* Author: Lewis (100%)
* */

struct ButtonComponent {
    std::string action; // Store the action from the JSON file
    std::function<void()> onClick; // Function to execute when the button is clicked
    bool isHover;
};