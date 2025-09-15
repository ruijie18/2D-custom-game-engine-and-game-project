/**
 * @file Core.cpp
 * @brief Header file defining an asset management system for handling multimedia assets in an application.
 *
This file implements the HustlersEngine class, which serves as the core engine for the game application. 
It manages the game loop, including input handling, audio management, frame rate control, and rendering. 
The engine integrates with various subsystems like graphics, game logic, and ImGui for UI rendering. 
It also handles window focus changes, full-screen toggling, and system performance monitoring.
 *
 * Author: Ruijie (%60)
 * Co-Author: Jarren (%40)
*/

#include "GlobalVariables.h"
#include <GL/glew.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImguiManager.h"
#include <iostream>
#include "Core.h"
#include "Graphics.h"
#include <random>
#include "InputSystem.h"
#include "GameLogic.h"
#include "Render.h"
#include "Mouse.h"
#include <chrono>
#include "FontSystem.h"
#include <atomic>
#include "ImguiManager.h"
#include "Physics.h"


bool isFullscreen = false; // Global or member variable
bool wasPressed = false;
int windowedWidth = 1600, windowedHeight = 900;  // Windowed mode dimensions
int windowedPosX, windowedPosY;
static bool isFKeyPressed = false;  // Track whether the F key was pressed
static bool showFPS = false;        // Flag to track whether FPS is currently shown

void FocusCallback(GLFWwindow* window, int focused) {
    (void)window;
    windowFocused = (focused == GLFW_TRUE);
    if (windowFocused) {
        // Resume game sounds
        //// std::cout<< "Window focused. Resuming sounds..." << std::endl;
        timerObj.Resume();
        audioEngine->ResumeAllSounds();
    }
    else {
        // Pause game sounds
        // // std::cout<< "Window unfocused. Pausing sounds..." << std::endl;
        timerObj.Pause();
        audioEngine->PauseAllSounds();
    }
}

void ToggleFullscreen(GLFWwindow* window) {

    if (isFullscreen) {
        // Switch to windowed mode
        glfwSetWindowMonitor(window, nullptr, windowedPosX, windowedPosY, windowedWidth, windowedHeight, 0);
        screen_width = windowedWidth;
        screen_height = windowedHeight;
        isFullscreen = false;
    }
    else {
        // Save current window position and size
        glfwGetWindowPos(window, &windowedPosX, &windowedPosY);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

        // Get the primary monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // Switch to fullscreen mode
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        screen_width = mode->width;
        screen_height = mode->height;
        isFullscreen = true;
    }
}

 
HustlersEngine::HustlersEngine(GLFWwindow* window) {

    glfwSetWindowFocusCallback(window, FocusCallback);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    //Init Game variables
    audioEngine->Init();
    InitGame();
    InputSystem = new CoreEngine::InputSystem(window);
    //Mouse::InitMouseCallbacks(window, clonedEntities); // Now, clonedEntities is properly populated
    // 
    //ToggleFullscreen(window);

}

HustlersEngine::~HustlersEngine() {
    //Remove resources for ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

}



void HustlersEngine::run(GLFWwindow* window) {

    static double lastTime = glfwGetTime();
    static double frameCount = 0;
    static double fpsTimer = glfwGetTime(); // Timer to track FPS every second
    double accumulatedTime = 0.0; // Time accumulator
    double currentFPS = 0.0;
    const double fixedDeltaTime = 1.0 / targetFPS;

    //bool showImGuiWindow = false; // Track the visibility of the ImGui window
    bool isLKeyPressed = false;   // Track whether the "L" key is currently pressed
    //IMGUI
    std::string output; // Declare output string to hold system times

    ImGuiManager::Initialize(window);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // Always poll events to detect focus changes
        // Calculate deltaTime
        numberofsteps = 0;
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        accumulatedTime += deltaTime;

        // Update game systems, pass deltaTime
        while (accumulatedTime >= fixedDeltaTime) {

            accumulatedTime -= fixedDeltaTime;
            numberofsteps++;
        }

        if (isPaused) {
            ECoordinator.UpdateSystems(0);
            CheckSystemProcess(0, SystemTimeOutput);
        }
        else {
            ECoordinator.UpdateSystems(fixedDeltaTime);
            CheckSystemProcess(fixedDeltaTime, SystemTimeOutput);
        }

        // Fullscreen toggle shortcut (F11)
        if(InputSystem->IsKeyPress(GLFW_KEY_M)){
            if (!wasPressed) {
                ToggleFullscreen(window);
                wasPressed = true;
            }
        }
        else {
            wasPressed = false;
        }

        updateGame(window, deltaTime);
        audioEngine->Update();

        //Toggle ImGui window visibility with "L"
        if (InputSystem->IsKeyPress(GLFW_KEY_L)) {
            if (!isLKeyPressed) {
                showImgui = !showImgui; // Toggle ImGui window visibility

                isLKeyPressed = true;  // Mark key as pressed

                    if (showImgui) {
                        // Resize the scene to the new dimensions and position it to the top right
                        //glViewport(screen_width - targetSceneWidth, 0, targetSceneWidth, targetSceneHeight);

                }
                else {
                    // Reset the viewport back to the original dimensions
                    //glViewport(0, 0, screen_width, screen_height);
                    int actualWidth, actualHeight;
                    glfwGetFramebufferSize(window, &actualWidth, &actualHeight);
                    screen_width = actualWidth;
                    screen_height = actualHeight;
                    glViewport(0, 0, screen_width, screen_height);
                }
            }
        }
        else {
            isLKeyPressed = false; // Reset key press state when the key is released
        }

        if (!showImgui) {
            allowThiefMoveIfTrue = true;
        }
        
        //ImGuiManager::SetupFBO(1280, 720);
        ImGuiManager::RenderSceneToFBO(deltaTime);

        // Call RenderImGui to handle all rendering
        ImGuiManager::RenderImGui(showImgui);

        // Render ImGui UI
        ImGui::Render(); // Render ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Render the ImGui data

        // Frame rate control
        frameCount++;
        if (currentTime - fpsTimer >= 1.0) {
            // Calculate FPS
            currentFPS = frameCount / (currentTime - fpsTimer);

            // Reset counters
            frameCount = 0;
            fpsTimer = currentTime;
        }

        if (CoreEngine::InputSystem::IsKeyPress(GLFW_KEY_F)) {
            if (!isFKeyPressed) {
                // Toggle FPS display
                showFPS = !showFPS;  // Toggle the showFPS flag
                isFKeyPressed = true; // Mark the key as pressed
            }
        }
        else {
            isFKeyPressed = false;  // Reset key press state when the key is released
        }

        // Render FPS text if showFPS is true
        if (showFPS) {
            std::stringstream stream;
            stream << std::fixed << std::setprecision(2) << currentFPS; // Format to 2 decimal places
            std::string fpsText = "FPS: " + stream.str();

            // Render the FPS text
            fontSystem->RenderText(fpsText, 50.0f, 30.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), "Orbitron.ttf", 24, 0);
        }

        // Update window title based on FPS visibility
        std::ostringstream titleOSS;
        titleOSS << "Hustler's University - Stage: Demo Stage";

        if (showFPS) {
            // Add FPS to the window title when visible
            titleOSS << " - FPS: " << std::fixed << std::setprecision(2) << currentFPS;
        }

        std::string title = titleOSS.str();  // Create final title string
        glfwSetWindowTitle(window, title.c_str()); // Set the window title

        //Update lastTime AFTER the frame logic
        lastTime = currentTime;

        // Swap buffers
        glfwSwapBuffers(window);
    }
}

void HustlersEngine::CheckSystemProcess(double deltaTime, std::string& output) {
    auto systems = ECoordinator.GetRegisteredSystems();
    if (!systems.empty()) {
        std::vector<double> systemTimes(systems.size());
        double totalSystemTime = 0.0;

        // Record the start time for the system updates
        for (size_t i = 0; i < systems.size(); ++i) {
            double systemStartTime = glfwGetTime(); // Start time for the system update
            if (isPaused) {
                systems[i]->Update(0);  // Update the system
            }
            else {
                systems[i]->Update(deltaTime);  // Update the system
            }
            
            double systemEndTime = glfwGetTime(); // End time for the system update
            if (isPaused) {
                systemTimes[i] = (systemEndTime - systemStartTime) * 0.0; // Convert to ms
            }
            else {
                systemTimes[i] = (systemEndTime - systemStartTime) * 1000.0; // Convert to ms
            }
            
            totalSystemTime += systemTimes[i];
        }

        // Now prepare the output data
        std::ostringstream oss;  // Output string stream
        for (size_t i = 0; i < systemTimes.size(); ++i) {
            double percentage = (totalSystemTime > 0) ? (100.0 * systemTimes[i] / totalSystemTime) : 0.0; // Safe division
            oss << systems[i]->getName() << " Time: " << systemTimes[i] << " ms (" << percentage << "%)\n";
        }

        output = oss.str(); // Assign the final output to the string

    }
    else {
        output = "No systems to update. Skipping system updates.\n";
    }
}


