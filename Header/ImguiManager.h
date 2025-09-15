#pragma once
#pragma once

/**
 * @file ImGuiManager.h
 * @brief Handles the declarations of the ImGuiManager
 *
 * Author: Jarren (100%)
*/


#include "GlobalVariables.h"
#include <GL/glew.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GameLogic.h"
#include "ImGuizmo.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include "Core.h"
#include <GL/glew.h>
#include <iostream>
#include <unordered_map>


namespace ImGuiManager {
    // Declare the necessary variables
    extern int imguiWidth;           // Width for ImGui
    extern int imguiHeight;          // Height for ImGui
    extern ImGuiIO* io;              // Reference to ImGui IO (should be initialized later)

    void Initialize(GLFWwindow* window);
    void Shutdown();
    void UpdateAudioStates();
    bool* getVisibleLayers();
    void RenderImGui(bool showImGuiWindow);
    GLuint SetupFBO(int width, int height);
    void RenderSceneToFBO(double deltaTime);
    void DisplayEntityList(int& selectedEntityID);
    void ResizeFBO(int newWidth, int newHeight);
}

// Structure to hold folder and its contents
struct FolderContent {
    std::string name;                              // Name of the folder or file
    std::vector<FolderContent> contents;           // Contents (subfolders and files)
    bool isSelected = false;                        // Indicates if this folder is selected
    bool isFolder;
};

extern GLuint fbo;
extern GLuint fboTexture;
extern ImVec2 texturePos;
extern float scaleX;
extern float scaleY;
extern ImVec2 mousePosInTexture;

// Terminal stuff
void AddLog(const std::string& logEntry);
void RenderTerminal();
void ProcessCommand(const std::string& command);

// Asset Library stuff
template <typename T>
void DisplayLibraryContents(const std::string& libraryName, const AssetLibrary<T>& assetLibrary);
void DisplayAllLibraries();
void RefreshLibraries();

// Entity stuff
std::vector<EntityID> getAllEntities();
bool insideEntity(glm::vec3 entityPos, glm::vec3 entityScale, unsigned int entityType);

// Input Handler within Imgui
void HandleMouseClicks();
void HandleEntityDragging();
void EntityClickGizmo(EntityID entityID);

// Scene
void RenderMainScene();
void RenderDefaultScene();
void RenderLeftSidebar();
void RenderBottomBar();
void RenderRightSidebar();

void renderTextureAssetEdit(Texture& texture);
void resizeImageWithSTB(const std::filesystem::path& inputPath, const std::filesystem::path& outputPath, int targetWidth, int targetHeight);
void PopulateTextureMap();

std::string GameStateToJsonFile(GameState state);

void RenderLayers();
void RenderLayerWindow();
void RenderLayerSelection();
void RenderEntityList();
void RenderResourceGraph();
void ShowLevelManagerWindow();

void glfwDropCallback(GLFWwindow* window, int count, const char** paths);
bool hasValidExtension(const std::string& filename, const std::vector<std::string>& validExtensions);

void undo();
