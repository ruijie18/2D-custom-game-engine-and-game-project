///////////////////////////////////////////////////////////////////////////////////////
/// 
/// @file InputSystem.h
/// @brief Header file for the Input System handling keyboard and mouse input
///
/// This file contains the declaration of the InputSystem class, which manages
/// input from the keyboard and mouse for the application. It includes functionality
/// to track key states, mouse button states, and mouse position, as well as providing
/// callback functions for input events.
///
/// @author Ruijie 
/// 
///////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <GLFW/glfw3.h>
#include "MessageSystem.h"
#include <iostream>
#include "CommonIncludes.h"

namespace CoreEngine {

    class InputSystem {
    public:

        enum class StageType {
            MainMenu,
            Playing,
            Pause,
            HowToPlay,
            HowToPlay2,
            confirmQuit,
            confirmQuit2,
            cutScene,
            gameWon,
            Lose,
            Settings
        };

        //Static members for shared input state management.
        static bool isPaused;
        static int SavedStage;
        //set to 0 if not applicable
        static int LevelPlayed;
        static double xPos;
        static double yPos;

        InputSystem(GLFWwindow* window);
        ~InputSystem() = default;

        //used to check for imgui text typing state
        void Enable() { isEnabled = true; }
        void Disable() { isEnabled = false; }
        bool IsEnabled() const { return isEnabled; }

        //check whether key is held down
        static bool IsKeyPress(int key);
        static bool IsKeyReleased(int key);
        //to showcases graphics
        static int Stage;

        //Callback functions for input
        static void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouse_button_cb(GLFWwindow* window, int button, int action, int mods);

        //check for mouse input
        static void mouse_pos_cb(GLFWwindow* window, double xpos, double ypos);
        static bool IsMousePressed(int button);
        static bool IsMouseReleased(int button);
        static bool IsMouseClicked(int button);
        bool IsMousePositionValid();
        static std::pair<double, double> GetMousePosition();
        static void mouse_scroll_cb(GLFWwindow* window, double xoffset, double yoffset);

        void InitializeKeyToMessageMap();
        void ProcessInput();
        static std::unordered_map<int, bool> keyMessageSent;
        enum class ButtonState {
            Released,
            Pressed,
            Held
        };
        static std::unordered_map<int, InputSystem::ButtonState> keyStates;
        static bool keystateF; 
        GLFWwindow* window;

    private:
        static bool isEnabled;
        static std::unordered_map<int, CoreEngine::MessageID> keyToMessageMap;
        static std::unordered_map<int, InputSystem::ButtonState> mouseButtons;
        static double mouseX, mouseY; 

    };

}