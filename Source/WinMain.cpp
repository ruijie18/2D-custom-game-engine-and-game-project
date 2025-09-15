/**
* @file Winmain.cpp
* @brief Main application entry point for the Hustler's University game engine. 
* Which contains initialization of game logic and setting up of GLFW, open functions 
* setting up the different engines needed and the cleanup process
*   
*  
* Author: Everyone
* 
* */

#include "GlobalVariables.h"
#include<iostream>
#include "imgui_impl_glfw.h"
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include "Graphics.h"
#include "SignalHandler.h"
#include "ConfigLoading.h"
#include <crtdbg.h>
#ifdef APIENTRY
#undef APIENTRY
#endif
#include <ParticleSystem.h>

//Global window dimensions
int Window_Width = 1600;
int Window_Height = 900;


/// @brief Callback function to adjust the OpenGL viewport on framebuffer resize.
/// @param window Pointer to the GLFW window.
/// @param width New window width.
/// @param height New window height.
/// 
/// This function adjusts the OpenGL viewport to maintain a 16:9 aspect ratio by
/// calculating new dimensions based on the framebuffer's new size and then centers
/// the viewport on the screen.
static void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    float targetAspect = 16.0f / 9.0f;
    int newWidth, newHeight;

    // Calculate the new dimensions that maintain the aspect ratio
    if (width / (float)height > targetAspect)
    {
        newWidth = static_cast<int>(height * targetAspect);
        newHeight = height;
    }
    else
    {
        newWidth = width;
        newHeight = static_cast<int>(width / targetAspect);
    }

    // Center the viewport
    int viewportX = (width - newWidth) / 2;
    int viewportY = (height - newHeight) / 2;

    screen_width = newWidth;
    screen_height = newHeight;

    // Set the OpenGL viewport
    glViewport(viewportX, viewportY, newWidth, newHeight);
    ImGuiManager::ResizeFBO(newWidth, newHeight);

}



/// @brief GLFW drop callback function to handle file drops.
/// @param window Pointer to the GLFW window.
/// @param count Number of dropped files.
/// @param paths Array of file paths for dropped files.
///
/// This function is triggered when files are dragged and dropped onto the window.
/// It clears the previously dropped files list and updates it with the new file paths.
void GLFW_DropCallback(GLFWwindow* window, int count, const char** paths) {
    (void)window;
    gDroppedFiles.clear(); // Clear the previous dropped files list

    for (int i = 0; i < count; ++i) {
        gDroppedFiles.push_back(paths[i]);
    }
}


/**
*  @brief Main function of the entire programm where it starts
* to initialise the game engine
**/
int main()
{
    //pybind11::scoped_interpreter guard{};  // Start and stop the interpreter

    //try {
    //pybind11::module my_module = pybind11::module::import("PythonScripts");
    //}
    //catch (const pybind11::error_already_set& e) {
    //    std::cerr << "Python error: " << e.what() << std::endl;
    //}



    //enable run-time memory check
    #if defined(DEBUG) | defined(_DEBUG)
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
      // _CrtSetBreakAlloc(152);
       //_CrtSetBreakAlloc(152);
        
    #endif
    loadConfigXML("Config.xml",screen_width,screen_height,fullscreen_bool);
    //Debugging, will print out all the errors in file
    HU_SetupSignalHandlers();

    GLFWwindow* window;
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create a window
    window = glfwCreateWindow(screen_width, screen_height, "Hustler's University", NULL, NULL);\
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW to load OpenGL functions
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // std::cout<< "GLEW Version: " << glewGetString(GLEW_VERSION) << std::endl;
    // Specify the viewport of OpenGL in the Window
    glViewport(0, 0, screen_width, screen_height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetDropCallback(window, GLFW_DropCallback);  // Set drop callback
    
    audioEngine = new CAudioEngine();
    fontSystem = new FontSystem();
    TateEngine = new HustlersEngine(window);
    //loop will run inside core.cpp
    TateEngine->run(window);

    // Delete window before ending the program
    glfwDestroyWindow(window);


    //delete input system for engine shutdown
    delete InputSystem;
    delete TateEngine;

    AudioLibrary.deleteallassets();
    audioEngine->Shutdown();
    fontSystem->Shutdown();
    delete audioEngine;
    delete fontSystem;

    //std::cout << "Remaining particles: " << getactiveparticles<< std::endl;

    // Terminate GLFW before ending the program
    glfwTerminate();
    return 0;
}




