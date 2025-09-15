///////////////////////////////////////////////////////////////////////////////////////
/// 
/// @file InputSystem.cpp
/// @brief Header file for the Input System handling keyboard and mouse input
///
/// This file contains the definetation of the InputSystem class, which manages
/// input from the keyboard and mouse for the application. It includes functionality
/// to track key states, mouse button states, and mouse position, as well as providing
/// callback functions for input events.
///
/// @author Ruijie
/// 
///////////////////////////////////////////////////////////////////////////////////////

#include <GlobalVariables.h>
#include "InputSystem.h"
#include "ImguiManager.h"

namespace CoreEngine {

    int InputSystem::SavedStage=0;
    int InputSystem::LevelPlayed = 0;
    bool InputSystem::isPaused=false;
    bool InputSystem::keystateF = false;
    bool InputSystem::isEnabled = true;

    //constructor 
    std::unordered_map<int, InputSystem::ButtonState> InputSystem::mouseButtons;
    std::unordered_map<int, InputSystem::ButtonState> InputSystem::keyStates;
    std::unordered_map<int, CoreEngine::MessageID> InputSystem::keyToMessageMap;
    std::unordered_map<int, bool> InputSystem::keyMessageSent;
    int InputSystem::Stage = splashscreen;

    double InputSystem::xPos = 0;
    double InputSystem::yPos = 0;

    double InputSystem::mouseX = 0.0;
    double InputSystem::mouseY = 0.0;

    //in the update loop
    void pauseHandler() {
        
        //if (isPaused == true) {
        //    InputSystem::SavedStage = InputSystem::Stage;
        //    InputSystem::Stage = 6; //pause number
        //}
        //else {
        //    
        //} 
    }

    //constructor to setup keyboard and mouse
    InputSystem::InputSystem(GLFWwindow* window) : window(window) {
        // Setup callbacks upon construction
        // Register the input-related callbacks with the window
        glfwSetKeyCallback(window, key_cb);
        glfwSetMouseButtonCallback(window, mouse_button_cb);
        glfwSetCursorPosCallback(window, mouse_pos_cb);
        glfwSetScrollCallback(window, mouse_scroll_cb);
        InitializeKeyToMessageMap();

        for (const auto& pair : keyToMessageMap) {
            keyMessageSent[pair.first] = false; // Initially, no messages have been sent
        }
    }

    // Callback for keyboard input
    //Check for keyboard input for player 
    void InputSystem::key_cb(GLFWwindow* , int key, int scancode, int action, int mod) {
        if (!isEnabled) return;
        mod = 0;
        scancode = 0;
        if (action == GLFW_PRESS) {
            // Transition to Pressed
            keyStates[key] = ButtonState::Pressed;
            keyMessageSent[key] = false;             // Reset sent state
        }
        else if (action == GLFW_REPEAT) {
            // Transition to Held
            keyStates[key] = ButtonState::Held;
        }
        else if (action == GLFW_RELEASE) {
            // Transition to Released
            keyStates[key] = ButtonState::Released;
        }
    }


    // Callback for mouse button input
    void InputSystem::mouse_button_cb(GLFWwindow* window, int button, int action, int mods) {
        if (!isEnabled) return;
        mods = 0;
        if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {

            if (action == GLFW_PRESS) {
                mouseButtons[button] = ButtonState::Pressed;
                // std::
                // << "Mouse Button Pressed: " << button << std::endl;
            }
            else if (action == GLFW_RELEASE) {
                mouseButtons[button] = ButtonState::Released;
                // std::cout<< "Mouse Button Released: " << button << std::endl;
            }
        } 
    }

    // Callback for mouse movement
    void InputSystem::mouse_pos_cb(GLFWwindow* window, double xpos, double ypos) {
        if (!isEnabled) return;
        // Check if the window is focused before updating mouse position
        if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
            mouseX = xpos;
            mouseY = ypos;
          /* std::cout << "mouse x is "<<mouseX<<" while mouse y is "<<mouseY<<"\n";*/

        }
    }

    // Callback for mouse scroll (e.g., scrolling wheel)
    void InputSystem::mouse_scroll_cb(GLFWwindow* window, double xoffset, double yoffset) {
        (void)xoffset, yoffset;

        if (!isEnabled) return;
        if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
            // Log the scroll event
            // std::cout<< "Mouse Scrolled: (" << xoffset << ", " << yoffset << ")" << std::endl;
        }
    }

    //check whether key is held down
    //return true when key is press down

    bool InputSystem::IsKeyPress(int key) {
        if (!isEnabled) return false;
            // Trigger only when key is "Pressed" or "Held"
            if (keyStates[key] == ButtonState::Pressed || keyStates[key] == ButtonState::Held) {
                keyStates[key] = ButtonState::Held; // Transition to Held if still pressed
                return true;
            }
        
        return false;
    }

    //check whether key is released
    //return true when key is released
    bool InputSystem::IsKeyReleased(int key) {
        if (!isEnabled) return false;

            //Check if the key was previously pressed or held
            if (keyStates[key] == ButtonState::Released) {
                //if release ignore
                return false;
            }


            //check previous state
            if (keyStates[key] == ButtonState::Pressed || keyStates[key] == ButtonState::Held) {
                // If we are releasing it now
                keyStates[key] = ButtonState::Released; // Reset to released after the check
                return true;
            }
        

        return false; // Indicate that the key wasn't released
    }


    //for messaging system
    void InputSystem::ProcessInput() {
        if (!isEnabled) return;
        for (const auto& pair : keyToMessageMap) {
            int key = pair.first;
            CoreEngine::MessageID messageId = pair.second;

            // If key is pressed or held down, send message
            if (keyStates[key] == ButtonState::Pressed || keyStates[key] == ButtonState::Held) {
                if (!keyMessageSent[key]) {
                    std::string sender = "InputSystem";
                    CoreEngine::IMessage* message = new CoreEngine::IMessage(messageId, sender);
                    CoreEngine::MessageBroker::Instance().Notify(message);
                    delete message;

                    keyMessageSent[key] = true; // Mark as sent
                }
            }
            else {
                keyMessageSent[key] = false; // Reset if no longer held
            }
        }
    }

    // Check if the specified mouse button is pressed
    bool InputSystem::IsMousePressed(int button) {
        if (!isEnabled) return false;

        // Check the current state of the mouse button
        return mouseButtons[button] == ButtonState::Pressed || mouseButtons[button] == ButtonState::Held;
    }

    bool InputSystem::IsMouseReleased(int button) {
        if (!isEnabled) return false;

        // Check if the mouse button is in the Released state
        return mouseButtons[button] == ButtonState::Released;
    }

    bool InputSystem::IsMouseClicked(int button) {
        static std::unordered_map<int, bool> wasPressed; // Tracks if the button was previously pressed

        if (!isEnabled) return false;

        // Check if the button was pressed and has now been released
        if (IsMousePressed(button)) {
            wasPressed[button] = true; // Mark the button as pressed
        }
        else if (IsMouseReleased(button) && wasPressed[button]) {
            wasPressed[button] = false; // Reset the pressed state
            return true;               // Consider it a "click"
        }

        return false;
    }

    // Return the current mouse position
    std::pair<double, double> InputSystem::GetMousePosition() {
        return { mouseX, mouseY }; // Return current mouse position
    }

    bool InputSystem::IsMousePositionValid() {
        if (mouseInTexture) {
            return true;
        }
        else {
            return false;
        }
    }

    void InputSystem::InitializeKeyToMessageMap() {    
        /*
        keyToMessageMap[GLFW_KEY_W] = CoreEngine::MessageID::MoveForward;
        keyToMessageMap[GLFW_KEY_S] = CoreEngine::MessageID::MoveBackward;
        keyToMessageMap[GLFW_KEY_A] = CoreEngine::MessageID::MoveLeft;
        keyToMessageMap[GLFW_KEY_D] = CoreEngine::MessageID::MoveRight;
        keyToMessageMap[GLFW_KEY_E] = CoreEngine::MessageID::Button_E;
        keyToMessageMap[GLFW_KEY_F] = CoreEngine::MessageID::Button_F;
        keyToMessageMap[GLFW_KEY_SPACE] = CoreEngine::MessageID::Jump;
        */
    }

}

