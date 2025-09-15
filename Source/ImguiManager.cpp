/**
 * @file ImGuiManager.cpp
 * @brief Handles the ImGui level editor interface and the implementation
 *
 * Renders the ImGuiManager off/on using class ImGuiManager
 * 
 * Creates a frame buffer based off the original frame buffer (screen) and puts it in the main scene.
 * Has features such as Entity Picking, Gizmos, Property Editor, Asset Library (Drag&Drop from Library > Scene and Window Explorer > Library),
 * Terminal, Layer Selection/View, Level loading, Undo, Entity List, Resource Graph and Pause/Play/Stop button.
 * 
 * M4 Added:
 * Linking Switches to Interactables
 * Animation Editor
 * Laser Frequency Editor
 * Delete Textures from Texture Library
 *
 * Author: Jarren (60%)
 * Co-Author: Rui Jie (20%), Lewis (10%), Jason (10%)
*/

#include "GlobalVariables.h"
#include "ImguiManager.h"
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <filesystem>
#include <imgui_internal.h>
#include "Physics.h"
#include <GL/glew.h>
#include <glm/vec3.hpp>
#include "JSONSerialization.h"
#include "FontSystem.h"
#include <stack>
#include <utility>

static bool checking = false;
static bool isGizmoOperationInProgress = false; // Tracks if a gizmo operation is active

bool isAdjustingScale = false;  // Flag to track if the user is adjusting
glm::vec3 preAdjustPosition, preAdjustScale;
float preAdjustRotation;

static bool hasSavedInitialState = false;

static int selectedCategory = -1;  // Index of the selected category
static char newCategory[32] = ""; // Buffer for adding a new category

static bool selectedInteraction = false; //for game logic components

//used to track the lazer modules 
static std::vector<EntityID> laserModuleEntities;
static std::vector<std::string> laserModuleNames;

enum class ManipulationState {
    None,        // Not manipulating
    Started,     // Manipulation just started
    Ongoing,     // Manipulation is in progress
    Ended        // Manipulation just ended
};


//when laser modules are being updated
void ScanLaserModules() {
    laserModuleEntities.clear();
    laserModuleNames.clear();

    for (auto entity : ECoordinator.GetAllEntities()) {
        if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {
            auto& entityPhysicsBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);
           

            if (entityPhysicsBody.category == "Laser Module" && ECoordinator.HasComponent<Name>(entity)) {
                laserModuleEntities.push_back(entity);
                laserModuleNames.push_back(ECoordinator.GetComponent<Name>(entity).name);
            }
        }
    }
}


//Undo stuff
struct ObjectState {
    glm::vec3 position{};
    float rotation{};
    glm::vec3 scale{};
    unsigned int entityType{};
    unsigned int entityID{};
    bool isDeleted;
    bool wasCreated; // Added to track newly created entities

    ObjectState(const glm::vec3& pos, float rot, const glm::vec3& sc, unsigned int entityId, bool deleted = false, bool created = false)
        : position(pos), rotation(rot), scale(sc), entityID(entityId), isDeleted(deleted), wasCreated(created) {
    }
};

// Handle Levels
static std::vector<std::string> levelList = {
    ""
};
static std::string currentLevel = levelList[0];  // Store the currently loaded level filename
int currentSelectedLevel;

// Undo stack
std::stack<ObjectState> undoStack;

// Save the current state of an entity
static void saveState(unsigned int entityID, const glm::vec3& pos, float rot, const glm::vec3& scale, bool isDeleted = false, bool wasCreated = false) {
    // If the undo stack is not empty, compare the current state with the last saved state
    if (!undoStack.empty()) {
        const ObjectState& lastState = undoStack.top();
        if (lastState.entityID == entityID &&
            lastState.position == pos &&
            lastState.rotation == rot &&
            lastState.scale == scale &&
            lastState.isDeleted == isDeleted &&
            lastState.wasCreated == wasCreated) {
            return; // Do not save identical states
        }
    }
    // Save the new state if it differs
    undoStack.emplace(pos, rot, scale, entityID, isDeleted, wasCreated);
}


//static void saveStateIfNeeded(unsigned int entityID, const glm::vec3& pos, float rot, const glm::vec3& scale, bool isDeleted = false) {
//    if (!undoStack.empty()) {
//        const ObjectState& lastState = undoStack.top();
//
//        // Skip saving if the state is unchanged
//        if (lastState.position == pos && lastState.rotation == rot && lastState.scale == scale && lastState.isDeleted == isDeleted) {
//            return;
//        }
//    }
//
//    // Save the state only if it's not already saved
//    saveState(entityID, pos, rot, scale, isDeleted);
//}

// Fixed width and height as our objects spawn in 1600x900 world coordinates
const int targetWidth = 1600;
const int targetHeight = 900;

// Gets the initial game file path
extern std::string initialGameFilePath;

// Stores all active entityIDs
std::vector<EntityID> activeEntities;

GLuint fbo = 0;
GLuint fboTexture = 0;
static GLuint rboDepth = 0;
static std::vector<FolderContent> folders; // Move this variable to be a global/static variable

// For Terminal logging
static std::vector<std::string> commandLog;
static char inputBuffer[256] = "";

// Cache to store loaded textures by filename
std::unordered_map<std::string, GLuint> textureCache;

// Outside the function, create a map to store each asset's selection state
static std::unordered_map<std::string, bool> assetSelectionStates;
static std::unordered_map<std::string, bool> audioPlayedStates;
std::string selectedAudio;
std::map<int, std::string> audioPlaying;

// To copy deltatime from main gameplay loop
static float deltaTime;

// Gets position and Scale of the frame buffer
ImVec2 texturePos;
static ImVec2 textureScale;
static int resizedfboX;
static int resizedfboY;

// For Mouse Clicks
static std::optional<EntityID> selectedEntity;
static std::optional<EntityID> lastSelectedEntity;
static glm::vec3 offset, mousePos;
static bool isDragging = false;
static bool allowClickingIfTrue = false;
ImVec2 mousePosInTexture;

// For Gizmo
static ImVec2 staticMousePosInTexture;
float scaleX, scaleY;
static int gizmoChoice;
ImGuizmo::OPERATION gizmoe;
static bool lockGizmoChoice;

// For textuer asset
static bool isEditTextureAsset = false;
static char TextureAssetBuffer1[256] = "";
static char TextureAssetBuffer2[256] = "";
static ImTextureID TextureAssetTextureID;
static std::string TextureAssetTextureFileName;
static float TextureAssetImageWidth;
static float TextureAssetImageHeight;
static float TextureAssetPaddingX = 20.0f;
static float TextureAssetPaddingY = 100.0f;
static Texture* TextureAssetRef;
std::map<GLuint, std::string> textureIDtoTextureFileMap;

// For deleting textures
static bool showDeletionPopup = false;
static std::vector<std::string> warningDeletionObjects;
std::unordered_map<std::string, bool> showDeletionPopupMap;

//for text file changing
static char textBuffer[256] = ""; // Global shared text buffer
static std::string text_change = ""; // Global shared text string

// For layer display
bool layerVisibility[3] = { true, true, true }; // Assuming you have 3 layers: Background, GameObject, UI

// For changing file name
namespace fs = std::filesystem;
static fs::path originalFilePath;

// For drag-drop files into editor
static bool showFilePopup = false;
static bool validFileExtension = true;
static std::string droppedFileName;
static std::string savePath;
std::vector<std::string> validAudioExtensions = { ".wav", ".mp3", ".ogg" };
std::vector<std::string> validImageExtensions = { ".png", ".jpg", ".jpeg" };

bool show = false;

// For Switch
static std::vector<EntityID> insideGroupEntities;
static std::vector<EntityID> outsideGroupEntities;
static bool needsUpdate = true; // Set this to true when changes occur
static std::optional<EntityID> lastSelectedSwitchEntity;
bool hasSwitch = false;

static const std::unordered_map<std::string, GameState> StringToGameState = {
    {"Main_Menu.json",      GameState::MainMenu},
    {"GameObjects.json",    GameState::Playing},
    {"LoseMenu.json",       GameState::Lose},
    {"LevelSelect.json",    GameState::LevelSelect},
    {"PauseMenu.json",     GameState::Pause},
    {"HowToPlay.json",     GameState::HowToPlay},
    {"ConfirmQuit.json",   GameState::confirmQuit},
    {"Level1.json",        GameState::Playing1},
    {"Level2.json",        GameState::Playing3}, // Note: Playing3 maps to level2
    {"Level3.json",        GameState::Playing2}, // Note: Playing2 maps to level3
    {"cutScene.json",      GameState::cutScene},
    {"endScene.json",      GameState::endScene},
    {"StarRating.json",    GameState::starRating},
    {"splashscreen.json",  GameState::splashscreen},
    {"Credit.json",        GameState::Credit}  
};


// Define the component order you want to check
std::vector<int> componentOrder = {
    0, // Transform
    1, // HUGraphics::GLModel
    2, // PhysicsSystem::PhysicsBody
    3, // RenderLayer
    4  // Name
};
int currentRenderLayerIndex = 1;

/**
 * @brief Logs the inputted strings into the terminal
 *
 * @param logEntry: The string you want logged into the terminal.
 */
void AddLog(const std::string& logEntry) {
    commandLog.push_back(logEntry);
}

/**
 * @brief Shows the terminal
 */
void RenderTerminal() {
    // Create a child window to scroll through command output
    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar)) {
        for (const auto& entry : commandLog) {
            ImGui::TextUnformatted(entry.c_str());
        }

        // Scroll to the bottom to always show the latest log entry
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();

    // Input for typing commands
    ImGui::Separator(); // Add a separator to distinguish the terminal from the input field
    if (ImGui::InputText("##CommandInput", inputBuffer, sizeof(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        std::string command(inputBuffer);

        if (!command.empty()) {
            AddLog("> " + command); // Add the typed command to the log
            ProcessCommand(command); // Execute command (custom function)
            inputBuffer[0] = '\0';   // Clear input buffer after command
        }
    }
}

/**
 * @brief Processes the command put into the terminal
 *
 * @param command: The command inputted into the terminal
 */
void ProcessCommand(const std::string& command) {
    if (command == "help") {
        AddLog("Available commands:");
        AddLog("  help - Show this help message");
        AddLog("  clear - Clear the terminal");
        AddLog("  exit - Close the application");
    }
    else if (command == "clear") {
        commandLog.clear(); // Clear the log
    }
    else if (command == "exit") {
        AddLog("Exiting application...");
    }
    else {
        AddLog("Unknown command: " + command);
    }
}

/**
 * @brief Displays the contents of the specified asset library.
 *
 * @tparam T: Asset type.
 * @param libraryName: Name of the Asset Library.
 * @param assetLibrary: The asset library instance containing the assets to be displayed.
 */
template <typename T>
void DisplayLibraryContents(const std::string& libraryName, const AssetLibrary<T>& assetLibrary) {

    // Track selected assets
    static std::unordered_map<std::string, bool> selectedTextureAssets; // Keep track of which assets are selected

    // Create a tree node for each asset library type
    if (ImGui::TreeNode(libraryName.c_str())) {
        // Display assets within the library
        const float iconSize = 64.0f;
        float sidebar_width = ImGui::GetWindowWidth();
        const float sidebarMaxWidth = static_cast<float>(sidebar_width) - 20.0f;

        auto loadedAssets = assetLibrary.GetAllLoadedAssets();
        ImGui::NewLine();
        for (const auto& assetPair : loadedAssets) {
            std::string assetName = assetPair.first;
            const std::shared_ptr<T>& asset = assetPair.second;

            ImGui::BeginGroup();

            // Define variables to store edit state and asset name
            static std::unordered_map<std::string, bool> editStates;
            static char editableName[128]; 

            if constexpr (std::is_same_v<T, Texture>) {
                // Preparing payload for drag from asset library onto main scene
                if (asset->GetTextureID() != 0) {
                    ImGui::Image((ImTextureID)asset->GetTextureID(), ImVec2(iconSize, iconSize));

                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        const Texture* texturePtr = asset.get();
                        ImGui::SetDragDropPayload("TEXTURE_ASSET", &texturePtr, sizeof(texturePtr));
                        ImGui::Image((ImTextureID)asset->GetTextureID(), ImVec2(iconSize, iconSize));
                        std::string fullLabel = assetName + " - " + std::to_string(ECoordinator.GetTotalNumberOfEntities());
                        ImGui::Text("%s", fullLabel.c_str());
                        ImGui::EndDragDropSource();

                    }
                }
                else {
                    ImGui::Text("%s (Failed to load)", assetName.c_str());
                }

                // For TextureAsset Editing
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    isEditTextureAsset = true;
                    TextureAssetTextureFileName = asset->GetFileName();
                    TextureAssetImageWidth = static_cast<float>(asset->GetImageWidth());
                    TextureAssetImageHeight = static_cast<float>(asset->GetImageHeight());
                    TextureAssetTextureID = asset->GetTextureID();
                    TextureAssetRef = asset.get();
                }

                ImGui::SameLine();

                // Check if the name is in edit mode
                bool& isEditing = editStates[assetName]; // Reference to this asset's edit state
                std::string displayName = assetName;

                // Adjust text display based on sidebar width
                ImVec2 textSize = ImGui::CalcTextSize(displayName.c_str());
                if (textSize.x > sidebarMaxWidth - iconSize) {
                    size_t charsToFit = displayName.length() * static_cast<size_t>(((sidebarMaxWidth - iconSize) / textSize.x));
                    displayName = displayName.substr(0, charsToFit - 3) + "...";
                }

                // If in edit mode, display an InputText field; otherwise, display text
                if (isEditing) {
                    // Copy the asset name to editable buffer if entering edit mode
                    strncpy_s(editableName, assetName.c_str(), sizeof(editableName) - 1);
                    editableName[sizeof(editableName) - 1] = '\0';

                    // Store the original file path if entering edit mode
                    if (originalFilePath.empty()) {
                        originalFilePath = fs::path("./Assets/Textures") / assetName;  // Update with actual path to assets
                    }

                    // Display InputText field for renaming
                    if (ImGui::InputText("##edit", editableName, sizeof(editableName), ImGuiInputTextFlags_EnterReturnsTrue)) {
                        // Apply the new name when Enter is pressed
                        assetName = editableName;

                        // Form the new file path with the updated name
                        fs::path newFilePath = fs::path("./Assets/Textures") / assetName;

                        // Rename the file on the filesystem
                        try {
                            fs::rename(originalFilePath, newFilePath);
                            originalFilePath = newFilePath; // Update the original path for further edits
                        }
                        catch (const fs::filesystem_error& e) {
                            // Handle error, e.g., log it or display a message
                            ImGui::Text("Error renaming file: %s", e.what());
                        }
                        TextureLibrary.deleteallassets();
                        TextureLibrary.LoadAssets("./Assets/Textures"); // Reload textures

                        isEditing = false; // Exit edit mode
                    }

                    // End editing if user clicks outside the input field
                    if (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                        isEditing = false;
                    }
                }
                else {
                    // Display text normally
                    ImGui::TextWrapped(displayName.c_str());

                    // Make the text editable on double-click
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        isEditing = true; // Enter edit mode
                        originalFilePath.clear(); // Clear previous path in preparation for a new edit session
                    }
                }

                if (ImGui::BeginPopupContextItem(assetName.c_str())) {
                    if (ImGui::MenuItem("Delete")) {
                        // Perform deletion checks
                        warningDeletionObjects.clear();
                        std::filesystem::path assetFilePath = std::filesystem::path("./Assets/Textures") / assetName;
                        std::string assetFilePathStr = assetFilePath.string();  // Convert to std::string

                        // Normalize the path to use forward slashes
                        std::replace(assetFilePathStr.begin(), assetFilePathStr.end(), '\\', '/');

                        // Check all JSON files for references to the texture
                        fs::path jsonDir = "./Json";  // Directory where JSON files are stored
                        if (fs::exists(jsonDir) && fs::is_directory(jsonDir)) {
                            for (const auto& entry : fs::directory_iterator(jsonDir)) {
                                if (entry.is_regular_file() && entry.path().extension() == ".json") {
                                    std::ifstream file(entry.path());
                                    if (!file.is_open()) {
                                        std::cerr << "Failed to open " << entry.path() << std::endl;
                                        continue;
                                    }

                                    json jsonData;
                                    try {
                                        file >> jsonData;  // Read JSON content
                                    }
                                    catch (const json::parse_error& e) {
                                        std::cerr << "Error parsing " << entry.path() << ": " << e.what() << std::endl;
                                        continue;
                                    }
                                    file.close();

                                    // Iterate through entities in the JSON file
                                    if (jsonData.contains("entities")) {
                                        for (const auto& entity : jsonData["entities"]) {
                                            if (entity.contains("components") && entity["components"].contains("textureFile")) {
                                                std::string textureFilePath = entity["components"]["textureFile"].get<std::string>();

                                                // Normalize the path to use forward slashes
                                                std::replace(textureFilePath.begin(), textureFilePath.end(), '\\', '/');

                                                // Check if the texture file matches the one being deleted
                                                if (textureFilePath == assetFilePathStr) {
                                                    std::string entityName = entity.contains("name") ? entity["name"].get<std::string>() : "Unnamed Entity";
                                                    warningDeletionObjects.push_back(entityName);

                                                    // Add JSON file name to the warning list
                                                    warningDeletionObjects.push_back(entry.path().filename().string());
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // If no entities use the texture, delete it immediately
                        if (warningDeletionObjects.empty()) {
                            // Delete associated entities
                            activeEntities = ECoordinator.GetAllEntities();  // Fetch all active entities
                            for (auto entity : activeEntities) {
                                if (ECoordinator.GetComponent<HUGraphics::GLModel>(entity).textureFile == assetFilePathStr) {
                                    ECoordinator.DestroyGameObject(entity);
                                }
                            }
                            DeleteAssetAndUpdateReferences(assetName);  // This will delete the asset and update all references in JSON
                            // Delete the texture asset
                            TextureLibrary.DeleteAssets(assetName);
                            fs::path filePath = fs::path("./Assets/Textures") / assetName;
                            fs::remove(filePath); // Delete the file
                            TextureLibrary.RefreshTextures();
                        }
                        else {
                            showDeletionPopupMap[assetName] = true; // Set the flag for this specific item
                        }
                    }
                    ImGui::EndPopup();
                }

                // Display the deletion warning popup if the flag is set for this specific texture
                if (showDeletionPopupMap[assetName]) {
                    std::string popupName = "DeleteWarning_" + assetName; // Unique popup name
                    ImGui::OpenPopup(popupName.c_str());

                    if (ImGui::BeginPopupModal(popupName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("Texture File: %s is being used by the following entities and JSON files:", assetName.c_str());
                        for (const auto& item : warningDeletionObjects) {
                            ImGui::Text(item.c_str());
                        }
                        ImGui::NewLine();
                        ImGui::Text("Deleting this texture will also delete the associated entities.");

                        if (ImGui::Button("Cancel")) {
                            ImGui::CloseCurrentPopup(); // Close the popup on cancel
                            showDeletionPopupMap[assetName] = false; // Reset the flag for this specific item
                        }

                        ImGui::SameLine();

                        if (ImGui::Button("Delete Anyway")) {
                            std::filesystem::path assetFilePath = std::filesystem::path("./Assets/Textures") / assetName;
                            std::string assetFilePathStr = assetFilePath.string();  // Convert to std::string

                            // Delete the texture asset
                            TextureLibrary.DeleteAssets(assetName);
                            fs::path filePath = fs::path("./Assets/Textures") / assetName;
                            fs::remove(filePath); // Delete the file
                            TextureLibrary.RefreshTextures();
                            activeEntities = ECoordinator.GetAllEntities();  // Fetch all active entities
                            for (auto entity : activeEntities) {
                                if (ECoordinator.GetComponent<HUGraphics::GLModel>(entity).textureFile == assetFilePathStr) {
                                    ECoordinator.DestroyGameObject(entity);
                                }
                            }

                            // Update JSON files to remove references to the deleted asset
                            UpdateJSONFilesAfterDeletion(assetFilePathStr);

                            ImGui::CloseCurrentPopup(); // Close the popup
                            showDeletionPopupMap[assetName] = false; // Reset the flag for this specific item
                        }

                        ImGui::EndPopup();
                    }
                }

            }



            else if constexpr (std::is_same_v<T, Audio>) {
                bool& isSelected = assetSelectionStates[assetName];
                bool& isCurrentlyPlaying = audioPlayedStates[assetName];
                float volume = 1.0f;

                audioEngine->LoadSound(assetName.c_str());

                // Determine button color based on selection and playback state
                if (isSelected) {
                    if (audioEngine->isPlaying(assetName.c_str())) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // Green for playing
                    }
                    else {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // Red for paused or stopped
                    }
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f)); // Default color
                }

                // Check if the sound has finished playing
                if (isCurrentlyPlaying && !audioEngine->isPlaying(assetName.c_str())) {
                    isSelected = false;            // Reset selection state
                    isCurrentlyPlaying = false;    // Reset playback state
                }

                // Create selectable button
                if (ImGui::Selectable(assetName.c_str(), &isSelected)) {
                    if (!isCurrentlyPlaying) {
                        audioEngine->PlaySound(assetName.c_str(), 0, volume);
                        isCurrentlyPlaying = true; // Mark as played
                    }
                    else {
                        // Toggle play/pause if it has been played before
                        if (audioEngine->isPlaying(assetName.c_str())) {
                            audioEngine->PauseSoundByName(assetName.c_str());
                        }
                        else {
                            audioEngine->PlaySound(assetName.c_str(), 0, volume);
                        }
                    }
                }

                if (ImGui::BeginPopupContextItem(assetName.c_str())) {
                    // Checkbox for toggling looping state
                    bool isLooping = audioEngine->IsSoundLooping(assetName);
                    if (ImGui::Checkbox("Looping", &isLooping)) {
                        // Toggle the looping state when the checkbox is clicked
                        if (audioEngine->ToggleSoundLooping(assetName)) {
                            isLooping = audioEngine->IsSoundLooping(assetName);
                        }
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopStyleColor(); // Restore color after the button is drawn

                


            }
            else if constexpr (std::is_same_v<T, Font>) {
                // Display text for font assets
                ImGui::Text("Font: %s", assetName.c_str());

                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    const char* assetNamePtr = assetName.c_str(); // Pointer to the C-string of assetName
                    ImGui::SetDragDropPayload("TEXT_ASSET", assetNamePtr, strlen(assetNamePtr) + 1); // Include null terminator in size
                    ImGui::Text(assetName.c_str()); // Display the asset name during the drag
                    ImGui::EndDragDropSource();
                }
            }

            ImGui::EndGroup();
            ImGui::NewLine();
        }


        ImGui::TreePop();


    }
}

/**
 * @brief Displays the each library.
 */
void DisplayAllLibraries() {
    if (ImGui::Button("Refresh")) {
        // Set the button color to indicate it has been pressed
        RefreshLibraries();
    }

    // Display each library in its own tree node
    DisplayLibraryContents("Texture Library", TextureLibrary);
    DisplayLibraryContents("Audio Library", AudioLibrary);
    DisplayLibraryContents("Font Library", FontLibrary);
}

/*
* @brief Resets all libraries
*/
void RefreshLibraries() {
    // Reload assets from the specified directories
    TextureLibrary.LoadAssets("./Assets/Textures");
    AudioLibrary.LoadAssets("./Assets/Audio");
    FontLibrary.LoadAssets("./Assets/Fonts");

    // Clean up textures no longer needed
    TextureLibrary.PruneAssets("./Assets/Textures");
    AudioLibrary.PruneAssets("./Assets/Audio");
    FontLibrary.PruneAssets("./Assets/Fonts");
}

/**
 * @brief Returns all active entities.
 */
std::vector<EntityID> getAllEntities() {
    std::vector<EntityID> x = ECoordinator.GetAllEntities();
    return x;
}

/*
 * @brief Displays the contents of the specified asset library.
 *
 * @param entityPos: Position of the entity.
 * @param entityScale: Scale of the entity.
 * @param entityType: Type of entity.
 * @param mmousePos: Position of Mouse translated to the screen
 */
bool insideEntity(glm::vec3 entityPos, glm::vec3 entityScale, unsigned int entityType) {
    float entityLeft = entityPos.x - (entityScale.x / 2.0f);
    float entityRight = entityPos.x + (entityScale.x / 2.0f);
    float entityTop = entityPos.y + (entityScale.y / 2.0f);
    float entityBottom = entityPos.y - (entityScale.y / 2.0f);

    // Checks if the entity type
    if (entityType == rectangle || entityType == texture|| entityType==text_texture || entityType == texture_animation) {
        // Checks if the mouse is within the bounds of the entity
        if (mousePos.x >= entityLeft && mousePos.x <= entityRight &&
            mousePos.y >= entityBottom && mousePos.y <= entityTop) {
            return true;  // Inside the entity
        }
    }
    else if (entityType == circle) {
        // Don't need the check above as distance from radius is enough
        if (glm::distance(mousePos, entityPos) < entityScale.x) {
            return true;
        }
    }

    return false;  // Outside the entity if all checks fail
}

/*
 * @brief Handles the input handler for the main scene, mostly moving objects in the Main Scene
 * Only allows clicking and selection of entities when mouse is not hovered over any ImGuizmo
 * Its main purpose is to get lastSelectedEntity for Properties and Gizmos
 */
void HandleMouseClicks() {
    mousePos = { staticMousePosInTexture.x, staticMousePosInTexture.y, 0.0f };

    static bool isDraggingObject = false; // Tracks if dragging is in progress
    static bool hasSavedInitialDragState = false; // Tracks if the initial state was saved during the drag

    if (!ImGuizmo::IsOver()) { // Ignore clicks when ImGuizmo is in use
        if (ImGui::IsMouseDown(0)) {
            if (!isDraggingObject) { // Start dragging
                activeEntities = getAllEntities();
                float closestDistance = std::numeric_limits<float>::max();

                for (auto& entity : activeEntities) {
                    if (!ECoordinator.HasComponent<Transform>(entity)) continue;

                    auto& transform = ECoordinator.GetComponent<Transform>(entity);
                    auto& mdlType = ECoordinator.GetComponent<HUGraphics::GLModel>(entity);
                    RenderLayerType entityLayer = ECoordinator.GetComponent<RenderLayer>(entity).layer;

                    glm::vec3 entityPosition(transform.translate.x, transform.translate.y, 0.0f);
                    glm::vec3 entityScale(transform.scale.x, transform.scale.y, 0.0f);

                    float entityDistance = glm::distance(mousePos, entityPosition);

                    if ((entityDistance < closestDistance) &&
                        insideEntity(entityPosition, entityScale, mdlType.shapeType) &&
                        (static_cast<int>(entityLayer) == currentRenderLayerIndex)) {
                        closestDistance = entityDistance;
                        selectedEntity = entity; // Select the entity
                        offset = entityPosition - mousePos; // Calculate the offset

                    }
                    lastSelectedEntity = selectedEntity;
                    if (lastSelectedEntity.has_value()) {
                        if (ECoordinator.HasComponent<PhysicsSystem::Switch>(*lastSelectedEntity)) {
                            lastSelectedSwitchEntity = lastSelectedEntity;
                            needsUpdate = true;
                        }
                    }

                    isDragging = true;  // Start dragging
                }

                if (selectedEntity.has_value()) {
                    lastSelectedEntity = selectedEntity;
                    isDraggingObject = true; // Mark dragging as started
                    if (ECoordinator.HasComponent<Transform>(*selectedEntity)) {
                        saveState(*selectedEntity,
                            ECoordinator.GetComponent<Transform>(*selectedEntity).translate,
                            ECoordinator.GetComponent<Transform>(*selectedEntity).rotate,
                            ECoordinator.GetComponent<Transform>(*selectedEntity).scale,
                            false);
                    }                   
                }
            }
        }
    }

    // Save state and end dragging on mouse release
    if (ImGui::IsMouseReleased(0)) {
        if (isDraggingObject && selectedEntity.has_value() && (*selectedEntity > 0) && (*selectedEntity < ECoordinator.GetTotalNumberOfEntities())) {
            auto& transform = ECoordinator.GetComponent<Transform>(*selectedEntity);
            saveState(*selectedEntity, transform.translate, transform.rotate, transform.scale); // Save final state
        }

        // Reset flags
        isDraggingObject = false;
        hasSavedInitialDragState = false;
    }
}

/*
 * @brief Funtion that allows the user to undo the scale, translate and rotation
*/
void undo() {
    if (ImGui::Button("Undo")) {
        // Loop until we find an undoable state or the stack is empty
        while (!undoStack.empty()) {
            ObjectState previousState = undoStack.top();
            undoStack.pop();

            // Check if the entity has a Transform component
            if (ECoordinator.HasComponent<Transform>(previousState.entityID)) {
                auto& transform = ECoordinator.GetComponent<Transform>(previousState.entityID);

                // Check if current transform state is different from the previous state
                bool hasChanged = transform.translate != previousState.position ||
                    transform.rotate != previousState.rotation ||
                    transform.scale != previousState.scale;

                // If the state has not changed and it's not a "new" entity, skip this undo and move to the next one
                if (!hasChanged && !previousState.wasCreated) {
                    continue;  // Skip this undo state and continue with the next one
                }
            }

            // If we reached here, the state has changed or it's a newly created entity that needs removal
            if (previousState.wasCreated) {
                // Remove newly created entity
                if (ECoordinator.HasComponent<Transform>(previousState.entityID)) {
                    ECoordinator.DestroyGameObject(previousState.entityID);
                }
                else {
                }
            }
            else {
                // Restore existing entity's state
                if (ECoordinator.HasComponent<Transform>(previousState.entityID)) {
                    auto& transform = ECoordinator.GetComponent<Transform>(previousState.entityID);
                    transform.translate = previousState.position;
                    transform.rotate = previousState.rotation;
                    transform.scale = previousState.scale;
                }
                else {
                }
            }
            return;  // Exit after processing the first valid undo
        }
    }
}

/*
 * @brief Funtion that checks if Imguizmo is currently being used to manipulate the object
*/
//static ManipulationState GetManipulationState(bool isCurrentlyManipulating, bool& wasManipulatingLastFrame) {
//    if (isCurrentlyManipulating && !wasManipulatingLastFrame) {
//        wasManipulatingLastFrame = true;
//        return ManipulationState::Started;
//    }
//    if (!isCurrentlyManipulating && wasManipulatingLastFrame) {
//        wasManipulatingLastFrame = false;
//        return ManipulationState::Ended;
//    }
//    return isCurrentlyManipulating ? ManipulationState::Ongoing : ManipulationState::None;
//}

/*
 * @brief Handles the gizmos to scale, rotate and translate the object
 * @param entityID needs the entityID to pass in for the gizmo to center on the entity itself
*/
void EntityClickGizmo(EntityID entityID) {
    
    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame(); // Ensure the gizmo is initialized

    // Get the transform component by reference
    if (!ECoordinator.HasComponent<Transform>(entityID)) {
        return;
    }
    Transform& transform = ECoordinator.GetComponent<Transform>(entityID);

    glm::vec3& entityPos = transform.translate;
    glm::vec3& entityScale = transform.scale;
    float& entityRotation = transform.rotate;

    // Set extended gizmo rectangle
    // AddLog("scale X: " + std::to_string(textureScale.x) + " scale Y: " + std::to_string(textureScale.y));
    ImGuizmo::SetRect(texturePos.x, texturePos.y, textureScale.x, textureScale.y);

    glm::mat4 projectionMatrix = glm::ortho(
        0.0f, static_cast<float>(targetWidth),             // Left, Right
        static_cast<float>(targetHeight), 0.0f,           // Top, Bottom (Flipped Y-axis)
        -1.0f, 1.0f                                        // Near, Far
    ); 

    glm::mat4 viewMatrix = glm::mat4(1.0f);
    ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);

    // Flip the position diagonally by applying negative scaling to both X and Y axes
    glm::mat4 flipMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, -1.0f));

    // Apply the flip to the object's matrix along with translation, rotation, and scale
    glm::mat4 objectMatrix = glm::translate(glm::mat4(1.0f), entityPos) *
        glm::rotate(glm::mat4(1.0f), glm::radians(entityRotation), glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::scale(glm::mat4(1.0f), entityScale) *
        flipMatrix;

    // Increase gizmo size
    ImGuizmo::SetGizmoSizeClipSpace(0.2f);

    static int lastOperationType = -1;
    static bool wasManipulating = false; // Track if manipulation was active

    // Determine the current operation
    switch (gizmoChoice) {
    case 0:
        gizmoe = ImGuizmo::SCALE; // Scale
        break;
    case 1:
        gizmoe = ImGuizmo::ROTATE; // Rotate
        break;
    case 2:
        gizmoe = ImGuizmo::TRANSLATE; // Translate
        break;
    }

    // Check if the gizmo is being manipulated
    if (ImGuizmo::Manipulate(
        glm::value_ptr(viewMatrix),
        glm::value_ptr(projectionMatrix),
        gizmoe, ImGuizmo::LOCAL,
        glm::value_ptr(objectMatrix)))
    {
        // Update the transform based on the operation
        if (gizmoChoice == 0) { // SCALE
            // Extract the scale from the matrix
            glm::mat3 scaleMat = glm::mat3(objectMatrix);
            entityScale = glm::vec3(scaleMat[0][0], scaleMat[1][1], scaleMat[2][2]);

            // Ensure that negative scale values are not applied unintentionally
            if (entityScale.x < 0.0f) entityScale.x = -entityScale.x;
            if (entityScale.y < 0.0f) entityScale.y = -entityScale.y;
        }
        else if (gizmoChoice == 1) { // ROTATE
            glm::mat3 rotationMat = glm::mat3(objectMatrix);
            rotationMat[0] = glm::normalize(rotationMat[0]);
            rotationMat[1] = glm::normalize(rotationMat[1]);

            // Extract the rotation angle in degrees (Z-axis rotation in 2D)
            float rawAngle = glm::degrees(atan2(rotationMat[1][0], rotationMat[0][0]));

            // Add 360 if the raw angle is negative to keep it in range [0, 360)
            if (rawAngle < 0) {
                rawAngle += 360.0f;
            }

            // Ensure smooth incremental rotation
            entityRotation = rawAngle;

            entityRotation = glm::degrees(atan2(rotationMat[1][0], rotationMat[0][0]));
            if (entityRotation < 0) entityRotation += 360.0f;
        }
        else if (gizmoChoice == 2) { // TRANSLATE
            entityPos = glm::vec3(objectMatrix[3].x, objectMatrix[3].y, 0);
        }

        wasManipulating = true; // Mark manipulation as active
    }
    else if (wasManipulating && !ImGuizmo::IsUsing()) {
        // Save state when manipulation ends
        saveState(entityID, transform.translate, transform.rotate, transform.scale);
        //AddLog("Fail");
        wasManipulating = false; // Reset manipulation flag
    }
}

/*
 * @brief Handles the entityDragging, the static global isDragging and selectedEntity.
 */
void HandleEntityDragging() {
    RenderLayer entityLayer;

    if (selectedEntity.has_value()) {
        if (ECoordinator.HasComponent<RenderLayer>(*selectedEntity)) {
            entityLayer = ECoordinator.GetComponent<RenderLayer>(*selectedEntity);
        }
    }

    if (mousePos.x < 0 || mousePos.y < 0 || mousePos.x > screen_width || mousePos.y > screen_height) {
        return;
    }
    // Only triggers if we're on the selected entity + isDragging is true
    if (isDragging && selectedEntity && (static_cast<int>(entityLayer.layer) == currentRenderLayerIndex)) {
        glm::vec3 newPos = mousePos + offset;
        if (ECoordinator.HasComponent<Transform>(*selectedEntity)) {
            auto& transform = ECoordinator.GetComponent<Transform>(*selectedEntity);
            transform.translate = newPos; // Directly set the new position
        }
    }
}

/*
 * @brief Renders all the other scenes.
 */
void RenderDefaultScene() {

    // Render the main scene
    RenderMainScene();

    // Render the left sidebar
    RenderLeftSidebar();

    // Render the bottom bar
    RenderBottomBar();

    // Render the right sidebar
    RenderRightSidebar();

    // Render the Layers choosing
    RenderLayerWindow();

    // Render the entity List
    RenderEntityList();

    // Render the resource consumption
    RenderResourceGraph();

    // Shows the level manager
    ShowLevelManagerWindow();
}

/*
 * @brief Handles the input handler for the main scene, mostly moving objects in the Main Scene
 * Also handles the sizing of all elements to be strictly 1600x900 dimensions as entity spawning is in that resolution
 */
void RenderMainScene() {
    // Start ImGui window
    ImGui::Begin("Game");

    // Get actual framebuffer size
    glfwGetFramebufferSize(glfwGetCurrentContext(), &screen_width, &screen_height);

    // Calculate the aspect ratios
    float targetAspectRatio = static_cast<float>(targetWidth) / targetHeight;

    // Use the smaller of the two dimensions to maintain aspect ratio
    float newWidth, newHeight;

     // Get available size in the ImGui window
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    if (viewportSize.x / viewportSize.y > targetAspectRatio) {
        // Viewport is wider than target, limit by height
        newHeight = viewportSize.y;
        newWidth = newHeight * targetAspectRatio;
    }
    else {
        // Viewport is taller than target, limit by width
        newWidth = viewportSize.x;
        newHeight = newWidth / targetAspectRatio;
    }


    // Center the texture
    float offsetY = ImGui::GetCursorPosY();
    ImVec2 offset2((viewportSize.x - newWidth) * 0.5f, (viewportSize.y - newHeight) * 0.5f + offsetY);
    ImGui::SetCursorPos(offset2);

    // Calculate texture position
    ImVec2 windowPos = ImGui::GetWindowPos();
    texturePos = ImVec2(windowPos.x + offset2.x, windowPos.y + offset2.y);

    // Render the main scene image
    ImGui::Image((ImTextureID)fboTexture, ImVec2(newWidth, newHeight), ImVec2(0, 1), ImVec2(1, 0));

    // Calculate mouse position relative to the texture
    ImVec2 mousePosition = ImGui::GetMousePos();

    // Calculate mouse position relative to texture
    mousePosInTexture = ImVec2((mousePosition.x - texturePos.x) / scaleX, (mousePosition.y - texturePos.y) / scaleY);

    // Save the static mouse position in texture
    staticMousePosInTexture = mousePosInTexture;

    if (screen_width == targetWidth) {
        // Calculate scale factors based on the target resolution
        scaleX = newWidth / screen_width;  // Scale factor for width
        scaleY = newHeight / screen_height; // Scale factor for height
        textureScale = ImVec2{ static_cast<float>(newWidth), static_cast<float>(newHeight) };
    }
    else {
        // Calculate scale factors based on the target resolution
        scaleX = newWidth / static_cast<float>(targetWidth);  // Scale factor for width
        scaleY = newHeight / static_cast<float>(targetHeight); // Scale factor for height
        textureScale = ImVec2{ static_cast<float>(newWidth) , static_cast<float>(newHeight) };
    }

    // Check if the mouse is within the bounds of the texture
    if (mousePosInTexture.x >= 0 && mousePosInTexture.x <= screen_width &&
        mousePosInTexture.y >= 0 && mousePosInTexture.y <= screen_height) {
        mouseInTexture = true;
        if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_ASSET")) {

            // Cast the payload data back to a pointer to Texture
            Texture* droppedTexture = *reinterpret_cast<Texture* const*>(payload->Data);
            if (droppedTexture) {
                // Log the texture information
                //AddLog("Dropped Texture: " + droppedTexture->GetFileName());

                // Apply scaling to mouse positions
                ImVec2 test = ImVec2(mousePosInTexture.x, mousePosInTexture.y);

                // Now use the texture information to create a new entity
                ECoordinator.CreateNewTextureEntity(*droppedTexture,    // Height
                    test.x,                        // X position
                    test.y                         // Y position
                );
            }
        }
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXT_ASSET")) {
            // Validate the payload size (strlen + 1 ensures null-terminated string)
            if (payload->DataSize > 0) {
                const char* droppedTextPtr = static_cast<const char*>(payload->Data); // Cast to C-string
                if (droppedTextPtr) {
                    std::string droppedText = droppedTextPtr; // Convert to std::string for easier handling

                    // Handle the dropped text (e.g., create a text entity)
                    ImVec2 scaledPosition = ImVec2(mousePosInTexture.x, mousePosInTexture.y);
                    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); // White color
                    float scale = 1.0f;
                    float width = 100.0f, height = 30.0f;
                    int fontSize = 24;
                    std::string name = "TextObject";
                    // Create the text entity using the dropped text
                    ECoordinator.CreateTextEntity("font", scale, color,
                        scaledPosition.x, scaledPosition.y,
                        width, height, droppedText, fontSize,name);
                    std::snprintf(textBuffer, sizeof(textBuffer), "%s", droppedText.c_str());
                    text_change = droppedText;
                }

            }
        }

            ImGui::EndDragDropTarget();
        }
    }
    else {
        mouseInTexture = false;
    }

    // Checks if the TextureAsset needs to be edited
    if (isEditTextureAsset) {
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always); // Top-left corner of the current screen
        ImGui::SetNextWindowSize(ImVec2(TextureAssetImageWidth + TextureAssetPaddingX, TextureAssetImageHeight + TextureAssetPaddingY), ImGuiCond_Always); // Optional: Fixed size
        renderTextureAssetEdit(*TextureAssetRef);
    }

    // Enables gizmos if user has clicked an entity + S/R/T has been clicked
    if (lastSelectedEntity.has_value()) {
        if (gizmoChoice != 4) {
            if (lastSelectedEntity != std::nullopt) {
                EntityClickGizmo(*lastSelectedEntity);
                ImGuizmo::Enable(true);
            }
        }
    }

    // Checks for the inputs by user to swap from Scale, Rotate or Transform
    if (InputSystem->IsKeyPress(GLFW_KEY_S)) {
        if (!lockGizmoChoice) {
            if (gizmoChoice == 0) {
                gizmoChoice = 4;
            }
            else {
                gizmoChoice = 0;
            }
            lockGizmoChoice = true;
        }
    }
    else if (InputSystem->IsKeyPress(GLFW_KEY_R)) {
        if (!lockGizmoChoice) {
            if (gizmoChoice == 1) {
                gizmoChoice = 4;
            }
            else {
                gizmoChoice = 1;
            }
            lockGizmoChoice = true;
        }
    }
    else if (InputSystem->IsKeyPress(GLFW_KEY_T) == GLFW_PRESS) {
        if (!lockGizmoChoice) {
            if (gizmoChoice == 2) {
                gizmoChoice = 4;
            }
            else {
                gizmoChoice = 2;
            }
            lockGizmoChoice = true;
        }
    }

    if (InputSystem->IsKeyReleased(GLFW_KEY_S) || InputSystem->IsKeyReleased(GLFW_KEY_R) || InputSystem->IsKeyReleased(GLFW_KEY_T)) {
        lockGizmoChoice = false;
    }

    if (lastSelectedEntity) {
        if (InputSystem->IsKeyPress(GLFW_KEY_DELETE)) {
            ECoordinator.DestroyGameObject(*lastSelectedEntity);
            lastSelectedEntity.reset();
            selectedEntity.reset();
            if (lastSelectedEntity.value() == ECoordinator.getThiefID()) {
                ECoordinator.resetThiefID();
            }
        }

    }

    if (ImGui::IsWindowFocused() && allowClickingIfTrue) {
        HandleMouseClicks(); // Call your function to handle clicks
        //HandleEntityDragging();
    }


    ImGui::End();
}

/*
 * @brief Renders the left sidebar (mostly the Asset Libraries)
 */
void RenderLeftSidebar() {
    //ImGui::SetNextWindowPos(ImVec2(0, engineOutlineHeight)); // Position within the main encompassing window
    //ImGui::SetNextWindowSize(ImVec2(static_cast<float>(sidebar_width), static_cast<float>(sidebar_height)));

    ImGui::Begin("Asset Library", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    // Sidebar content, like tabs and assets
    if (ImGui::BeginTabBar("SidebarTabs")) {
        if (ImGui::BeginTabItem("Folder Contents")) {
            DisplayAllLibraries();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Undo")) {
            undo();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Audio Analysis")) {
            audioEngine->Update();
            audioPlaying.clear();
            audioPlaying = audioEngine->ListSounds();
            auto audioChannels = audioEngine->getChannels();

            static std::map<int, float> volumeMap; // Store volume per audioID

            for (auto& [audioID, soundName] : audioPlaying) {
                auto channelIt = audioChannels.find(audioID); // Find the corresponding channel

                if (channelIt == audioChannels.end() || !channelIt->second) continue; // Skip if no valid channel

                FMOD::Channel* channel = channelIt->second; // Get the channel

                // Retrieve volume (initialize if not found)
                if (volumeMap.find(audioID) == volumeMap.end()) {
                    float initialVol;
                    channel->getVolume(&initialVol);
                    volumeMap[audioID] = initialVol;
                }

                float& vol = volumeMap[audioID]; // Reference the volume from the map

                //std::cout << soundName.c_str() << std::endl;
                ImGui::Text(soundName.c_str());

                if (ImGui::SliderFloat(("##Volume" + std::to_string(audioID)).c_str(), &vol, 0.0f, 1.0f)) {
                    channel->setVolume(vol);
                }
                ImGui::Separator();
            }

            ImGui::EndTabItem();
        }


        ImGui::EndTabBar();
    }

    // Check if any files were dropped
    if (!gDroppedFiles.empty()) {
        for (const auto& filePath : gDroppedFiles) {
            droppedFileName = filePath; // Save the filename for the popup

            // Check if the dropped file has a valid extension for audio or image
            if (hasValidExtension(filePath, validAudioExtensions)) {
                validFileExtension = true;
                savePath = "./Assets/Audio/" + std::filesystem::path(filePath).filename().string();
            }
            else if (hasValidExtension(filePath, validImageExtensions)) {
                validFileExtension = true;
                savePath = "./Assets/Textures/" + std::filesystem::path(filePath).filename().string();
            }
            else {
                validFileExtension = false;
                savePath = ""; // Clear the path if the file type is invalid
            }

            showFilePopup = true;  // Set the flag to show the popup
        }
        gDroppedFiles.clear(); // Clear the dropped files after processing
    }

    // Create a popup to show the dropped filename
    if (showFilePopup) {
        ImGui::OpenPopup("Dropped File");

        // Display the filename in a popup window
        if (ImGui::BeginPopup("Dropped File")) {
            ImGui::Text("Dropped File: %s", droppedFileName.c_str());

            if (validFileExtension) {
                // Show the save path for audio or image files
                ImGui::Text("File will be saved to: %s", savePath.c_str());

                // Show "Save" and "Cancel" buttons
                if (ImGui::Button("Save")) {
                    
                    auto absoluteSource = fs::absolute(droppedFileName);
                    auto absoluteDestination = fs::absolute(savePath);


                    fs::create_directories(fs::path(absoluteDestination).parent_path());


                    if (fs::exists(absoluteDestination)) {
                        std::cerr << "File already exists at: " << absoluteDestination << std::endl;
                    }
                    else {
                        try {
                            if (fs::copy_file(absoluteSource, absoluteDestination, fs::copy_options::none)) {
                                RefreshLibraries();

                            }
                            else {
                                std::cerr << "Copy operation failed without exception." << std::endl;
                            }
                        }
                        catch (const fs::filesystem_error& e) {
                            std::cerr << "Filesystem error: " << e.what() << std::endl;
                        }
                    }




                    ImGui::CloseCurrentPopup(); // Close the popup after saving
                    showFilePopup = false; // Reset the flag
                }
            }
            else {
                // If the file extension is invalid, show a message instead of buttons
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Wrong file extension");
            }
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup(); // Close the popup on cancel
                showFilePopup = false; // Reset the flag
            }

            ImGui::EndPopup();
        }
        
    }
    ImGui::End();
}

void RenderBottomBar() {
    ImGui::Begin("Terminal", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    if (lastSelectedEntity.has_value() && ECoordinator.HasComponent<HUGraphics::GLModel>(*lastSelectedEntity)) {
        auto& mdl = ECoordinator.GetComponent<HUGraphics::GLModel>(*lastSelectedEntity);
        static std::vector<std::pair<std::string, std::shared_ptr<Texture>>> loadedTextures;
        static std::vector<std::pair<std::string, std::shared_ptr<Font>>> loadedFonts;


        ImGui::SeparatorText("Texture / Font Picker (Auto)");

        if (ImGui::Button("Refresh")) {
            RefreshLibraries();          // Rescan asset folders
            loadedTextures.clear();      // Force UI to reload next frame
            loadedFonts.clear();

        }
         
        ImGui::SameLine();
        ImGui::Text("Click a resource to apply to selected entity.");

        

        // ---------- TEXTURE PICKER ----------
        if (mdl.shapeType == texture_animation || mdl.shapeType == texture){
            if (loadedTextures.empty()) {
                for (const auto& [name, texture] : TextureLibrary.GetAllLoadedAssets()) {
                    if (texture && texture->GetTextureID() != 0) {
                        loadedTextures.emplace_back(name, texture);
                    }
                }
            }

            const float iconSize = 48.0f;
            const float padding = 8.0f;
            float availWidth = ImGui::GetContentRegionAvail().x;
            int itemsPerRow = std::max(1, static_cast<int>(availWidth / (iconSize + padding)));
            int count = 0;

            for (const auto& [texName, texture] : loadedTextures) {
                GLuint texID = texture->GetTextureID();

                ImGui::BeginGroup();

                std::string buttonID = "##tex_" + texName;
                if (ImGui::ImageButton(buttonID.c_str(), (ImTextureID)(uintptr_t)texID, ImVec2(iconSize, iconSize))) {
                    mdl.textureFile = "./Assets/Textures/" + texName;
                    mdl.textureID = texID;
                    mdl.uvOffset = { 0.0f, 0.0f };
                    mdl.uvScale = { 1.0f, 1.0f };
                    mdl.isanimation = false;

                    AddLog("Texture updated to: " + texName);
                }

                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + iconSize);
                ImGui::TextWrapped("%s", texName.c_str());
                ImGui::PopTextWrapPos();

                ImGui::EndGroup();

                if (++count % itemsPerRow != 0)
                    ImGui::SameLine();
            }
        }

        // ---------- FONT PICKER ----------
        else if (mdl.shapeType == text_texture) {
            if (loadedFonts.empty()) {
                for (const auto& [fontName, fontPtr] : FontLibrary.GetAllLoadedAssets()) {
                    if (fontPtr) {
                        loadedFonts.emplace_back(fontName, fontPtr);
                    }
                }
            }

            ImGui::SeparatorText("Font Picker");

            for (const auto& [fontName, fontPtr] : loadedFonts) {
                if (ImGui::Selectable(fontName.c_str())) {
                    mdl.fontName = fontName; 
                    
                 
                   GLuint textTexture = fontSystem->RenderTextToTexture(mdl.text, mdl.fontScale, mdl.color, fontName, mdl.fontSize);
                    mdl.textureID = textTexture;  // Use the generated text texture

                    AddLog("Font updated to: " + fontName);
                }
            }
        }
    }

    ImGui::End();
}


/*
 * @brief Renders the properties input elements for each object when lastSelectedEntity is not nullopt
 */
void RenderRightSidebar() {
    ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ScanLaserModules();



    Signature missingSig;

    // Set padding for button
    float padding = 8.0f; // Adjust as needed for padding around the "x" button
    float buttonWidth = ImGui::CalcTextSize("x").x + padding;

    //AddLog("Selected Entity: " + lastSelectedEntity.has_value());

    if (lastSelectedEntity.has_value()) {
        Signature sig = ECoordinator.GetEntitySignature(*lastSelectedEntity);

        // Name component
        if (sig.test(4)) {
            auto& name = ECoordinator.GetComponent<Name>(*lastSelectedEntity);
            ImGui::Text("Name");

            // Delete button for Name component
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - buttonWidth);  // Move delete button to the far right
            if (ImGui::Button("x")) {
                ECoordinator.RemoveComponent<Name>(*lastSelectedEntity);
                //sig.reset(4);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Delete");
            }

            // Input text field for name
            static char nameBuffer[128];
            std::snprintf(nameBuffer, sizeof(nameBuffer), "%s", name.name.c_str());
            if (ImGui::InputText("##Name", nameBuffer, sizeof(nameBuffer))) {
                name.name = nameBuffer;
            }
            if (ImGui::IsItemActive()) {
                InputSystem->Disable();
            }
            else {
                InputSystem->Enable();
            }

        }
        else {
            missingSig.set(4);
        }


        // Transform component
        if (sig.test(0)) {
            auto& transform = ECoordinator.GetComponent<Transform>(*lastSelectedEntity);
            ImGui::Text("Size");

            // Delete button for Transform component

            ImGui::InputFloat("Width", &transform.scale.x);
            ImGui::InputFloat("Height", &transform.scale.y);
            ImGui::Separator();

            ImGui::Text("Rotation");
            ImGui::InputFloat("Rotate", &transform.rotate);
            ImGui::Separator();

            ImGui::Text("Transform");
            ImGui::InputFloat("X", &transform.translate.x);
            ImGui::InputFloat("Y", &transform.translate.y);
            ImGui::InputFloat("Z", &transform.translate.z);
            ImGui::Separator();
        }
        else {
            missingSig.set(0);
        }

        // GLModel component
        if (sig.test(1)) {
            auto& mdl = ECoordinator.GetComponent<HUGraphics::GLModel>(*lastSelectedEntity);
            ImGui::Checkbox("SpriteSheet", &mdl.isanimation);
            if (mdl.isanimation) {
                mdl.shapeType = texture_animation;
                ImGui::InputFloat("FrameTime", &mdl.frametime);
                ImGui::InputInt("Rows", &mdl.rows);
                ImGui::InputInt("Columns", &mdl.columns);
                ImGui::InputInt("Total No. of frames", &mdl.totalframe);

                static int selectedAnimationIndex = -1; // Store selected index
                std::string selectedAnimationName; // Store the name of the selected animation

                if (selectedAnimationIndex >= 0 && selectedAnimationIndex < animationPresets.size()) {
                    // Set current animation name based on selected index
                    selectedAnimationName = std::next(animationPresets.begin(), selectedAnimationIndex)->first;
                }

                if (ImGui::BeginCombo("Animation Preset", selectedAnimationName.c_str())) {
                    int index = 0;
                    for (const auto& [name, animData] : animationPresets) {
                        bool isSelected = (selectedAnimationIndex == index);
                        if (ImGui::Selectable(name.c_str(), isSelected)) {
                            selectedAnimationIndex = index;
                            selectedAnimationName = name; // Update selected name

                            // Apply the preset to the model
                            mdl.frametime = animData.frametime;
                            mdl.rows = animData.rows;
                            mdl.columns = animData.columns;
                            mdl.totalframe = animData.totalFrames;
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                        index++;
                    }
                    ImGui::EndCombo();
                }
          
                static char selectedAnimationNameBuffer[256];
                // Create the "Name" input field
                ImGui::PushItemWidth(75);

                // Create the "Name" input field with the specified width
                ImGui::InputText("Name", selectedAnimationNameBuffer, IM_ARRAYSIZE(selectedAnimationNameBuffer));

                // Restore the previous item width to prevent other elements from being affected
                ImGui::PopItemWidth();
                // Place the "Save Animation Preset" button beside the "Name" input field
                ImGui::SameLine();  // This places the next widget (button) on the same line
                if (ImGui::Button("Save Animation Preset")) {
                    std::cout << "Save button clicked!" << std::endl;

                    selectedAnimationName = selectedAnimationNameBuffer;

                    // Ensure the name is not empty
                    if (selectedAnimationName.empty()) {
                        std::cerr << "Error: Animation preset name is empty! Please enter a name." << std::endl;
                    }
                    else {
                        if (selectedAnimationIndex >= 0 && selectedAnimationIndex < animationPresets.size()) {
                            // Modify existing animation preset
                            auto& selectedAnim = std::next(animationPresets.begin(), selectedAnimationIndex)->second;
                            selectedAnim.frametime = mdl.frametime;
                            selectedAnim.rows = mdl.rows;
                            selectedAnim.columns = mdl.columns;
                            selectedAnim.totalFrames = mdl.totalframe;
                        }
                        else {
                            // Add new animation preset
                            animationPresets[selectedAnimationName] = {
                                mdl.frametime, mdl.rows, mdl.columns, mdl.totalframe
                            };
                        }

                        // Save all presets to JSON
                        SaveAnimationPresetsToJSON("Json/spritesheet_ref.json");
                    }
                }
            
            }
            else if (!mdl.isanimation && mdl.shapeType == text_texture) {
                mdl.shapeType = text_texture;
            }
            else {
                mdl.shapeType = texture;
            }

            ImGui::Separator();

            if (mdl.shapeType == text_texture) {
                // Ensure textBuffer reflects the current text from the model
                static bool isInitialized = false;
                if (!isInitialized) {
                    std::snprintf(textBuffer, sizeof(textBuffer), "%s", mdl.text.c_str());
                    isInitialized = true;
                }

                // Allow the user to edit the text
                if (ImGui::InputText("Text Content", textBuffer, sizeof(textBuffer))) {
                    mdl.text = textBuffer; // Update the model's text property
                    if (mdl.textureID != 0) {
                        glDeleteTextures(1, &mdl.textureID);  // Delete the previous texture
                        GLenum err = glGetError();
                        if (err != GL_NO_ERROR) {
                        }
                        mdl.textureID = 0;
                    }

                    GLuint updated_text = fontSystem->RenderTextToTexture(mdl.text,mdl.fontScale, mdl.color, mdl.fontName, mdl.fontSize);
                    mdl.textureID = updated_text;
                    InputSystem->Enable();
                }
                if (ImGui::IsItemActive()) {
                    InputSystem->Disable();
                }
                else {
                    InputSystem->Enable();
                }
            }

        }
        else {
            missingSig.set(1);
        }


        // Header Title
        {
            const char* title = "Add Game Logics to entity";
            float contentWidth = ImGui::GetContentRegionAvail().x;
            float textWidth = ImGui::CalcTextSize(title).x;
            float offset_mouse = (contentWidth - textWidth) * 0.5f;
            if (offset_mouse > 0) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_mouse);
            ImGui::TextUnformatted(title);
}

        // Ensure an entity is selected
        if (!lastSelectedEntity.has_value()) return;

        EntityID& entity = *lastSelectedEntity;

        // Get component states
        bool isThief = ECoordinator.getThiefID() == entity;
        bool hasLaser = ECoordinator.HasComponent<LaserComponent>(entity);
        bool hasSwitchlogic = ECoordinator.HasComponent<PhysicsSystem::Switch>(entity);
        bool hasPhysics = ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity);

        // Retrieve current category if PhysicsBody exists
        std::string currentCategory = "";
        if (hasPhysics) {
            currentCategory = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity).category;
        }

        ImVec2 buttonSize(120, 40);

        // ===== TABLE ===== //
        if (ImGui::BeginTable("GameLogicTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthFixed, 110.0f); // Wider column

            ImGui::TableNextRow();
            const char* row1[] = { "Thief", "Laser", "Switch" };

            for (int i = 0; i < 3; ++i) {
                ImGui::TableSetColumnIndex(i);

                std::string label = ((i == 0 && isThief) || (i == 1 && hasLaser) || (i == 2 && hasSwitchlogic))
                    ? (std::string(row1[i]) + " [X]") : row1[i];

                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - buttonSize.x) * 0.5f);

                if (i == 0) { // Thief
                    if (ImGui::Button(label.c_str(), buttonSize)) {
                        if (isThief) {
                            ECoordinator.resetThiefID();
                            isThief = false;
                        }
                        else if (ECoordinator.hasThiefID() && ECoordinator.getThiefID() != entity) {
                            ImGui::OpenPopup("Thief Already Assigned");
                        }
                        else {
                            ECoordinator.setThiefID(entity);
                            isThief = true;
                            if (!hasPhysics) {
                                ECoordinator.AddComponent(entity, PhysicsSystem::PhysicsBody{});
                                hasPhysics = true;
                                selectedInteraction = false;
                            }
                            ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity).category = "Thief";

                        }
                    }

                    if (ImGui::BeginPopupModal("Thief Already Assigned", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("A Thief is already assigned! %d", ECoordinator.getThiefID() + 1);
                        ImGui::Separator();
                        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
                        ImGui::EndPopup();
                    }
                }
                else if (i == 1) { // Laser
                    if (isThief) ImGui::BeginDisabled();
                    if (ImGui::Button(label.c_str(), buttonSize)) {
                        if (!hasLaser) {
                            ECoordinator.AddComponent(entity, LaserComponent{});
                            ECoordinator.GetComponent<LaserComponent>(entity).turnedOn = true;
                            if (!hasPhysics) {
                                ECoordinator.AddComponent(entity, PhysicsSystem::PhysicsBody{});
                            }
                            ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity).category = "Laser";

                            hasLaser = true;
                            selectedInteraction = false;

                        }
                        else {
                            ECoordinator.RemoveComponent<LaserComponent>(entity);
                            hasLaser = false;
                        }
                    }
                    if (isThief) ImGui::EndDisabled();
                }
                else if (i == 2) { // Switch
                    if (isThief) ImGui::BeginDisabled();
                    if (ImGui::Button(label.c_str(), buttonSize)) {
                        if (!hasSwitchlogic) {
                            ECoordinator.AddComponent(entity, PhysicsSystem::Switch{});
                            if (!hasPhysics) {
                                ECoordinator.AddComponent(entity, PhysicsSystem::PhysicsBody{});
                            }

                            hasSwitchlogic = true;
                            selectedInteraction = false;

                        }
                        else {
                            ECoordinator.RemoveComponent<PhysicsSystem::Switch>(entity);
                            hasSwitchlogic = false;
                        }
                    }
                    if (isThief) ImGui::EndDisabled();
                }
            }

            // === Interaction / Wall Row ===
            ImGui::TableNextRow();
            const char* row2[] = { "Interactions", "Wall" };

            // Disable both buttons if Laser is active
            if (hasLaser) ImGui::BeginDisabled();

            for (int i = 0; i < 2; ++i) {
                ImGui::TableSetColumnIndex(i);

                bool isInteraction =
                    currentCategory == "LockDoor" ||
                    currentCategory == "Laser Module" ||
                    currentCategory == "Object" ||
                    currentCategory == "Door";
                if (isInteraction) {
                    selectedInteraction = true;
                }
                bool isWall = currentCategory == "Wall";
                bool isSelected = (i == 0 && isInteraction) || (i == 1 && isWall);

                std::string label = isSelected ? (std::string(row2[i]) + " [X]") : row2[i];
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - buttonSize.x) * 0.5f);

                if (isThief) ImGui::BeginDisabled();

                if (ImGui::Button(label.c_str(), buttonSize)) {
                    if (!hasPhysics) {
                        ECoordinator.AddComponent(entity, PhysicsSystem::PhysicsBody{});
                        hasPhysics = true;
                    }

                    auto& physicsBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);

                    if (i == 0) {
                        if (isInteraction) {
                            physicsBody.category.clear();
                            selectedInteraction = false;
                        }
                        else {
                            physicsBody.category = "Object";
                            selectedInteraction = true;
                            
                        }
                    }
                    else if (i == 1) {
                        if (isWall) {
                            physicsBody.category.clear();
                        }
                        else {
                            physicsBody.category = "Wall";
                            selectedInteraction = false;
                        }
                    }
                }

                if (isThief) ImGui::EndDisabled();
            }

            if (hasLaser) ImGui::EndDisabled();

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("");

            ImGui::EndTable();
        }

        // === Dropdown appears AFTER table, ONLY IF Interactions was selected ===
        if (selectedInteraction) {
            std::vector<std::string> interactionTypes = { "Object", "Door", "LockDoor", "Laser Module" };
            static int selectedInteractionIndex = 0;

            // Try to set index to match current
            for (int i = 0; i < interactionTypes.size(); ++i) {
                if (currentCategory == interactionTypes[i]) {
                    selectedInteractionIndex = i;
                    break;
                }
            }
            ImGui::Text("Selected Interaction Type: %s", interactionTypes[selectedInteractionIndex].c_str());

            if (ImGui::BeginCombo("##InteractionType", interactionTypes[selectedInteractionIndex].c_str())) {
                for (int i = 0; i < interactionTypes.size(); ++i) {
                    bool selected = (selectedInteractionIndex == i);
                    if (ImGui::Selectable(interactionTypes[i].c_str(), selected)) {
                        selectedInteractionIndex = i;
                        if (!hasPhysics) {
                            ECoordinator.AddComponent(entity, PhysicsSystem::PhysicsBody{});
                            hasPhysics = true;
                        }
                        ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity).category = interactionTypes[i];
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }


        /* How it works:
        * All the objects that are not linked to the switch will be on the right side.
        * On the left are linked objects
        * When interacting with the switch:
            - Lasers will turn on/off through their turnedOn bool.
            - Locked Doors will unlock.
            - More will be added in the future.*/
        if (ECoordinator.HasComponent<PhysicsSystem::Switch>(*lastSelectedEntity)) {

            //Switches components
            ImGui::Text("Game Logic Switch Component");

            ImGui::BeginChild("SwitchComponentBox", ImVec2(0, 0), true, 0);
            if (ImGui::Button("Toggle Switch")) {
                PhysicsSystem::PhysicsBody& switchPhysBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(*lastSelectedEntity);
                PhysicsSystem::Switch& switchBody = ECoordinator.GetComponent<PhysicsSystem::Switch>(*lastSelectedEntity);
                // Retrieve the Switch's model component
                HUGraphics::GLModel& switchModel = ECoordinator.GetComponent<HUGraphics::GLModel>(*lastSelectedEntity);
                std::string newTextureFile;
                GLuint textureID = 0;
                std::shared_ptr<Texture> activeTexture;

                // Toggle the switch state
                switchPhysBody.Switch = !switchPhysBody.Switch;

                // Play switch sound
                audioEngine->PlaySound("SwitchInteract.ogg", 0, 0.3f * sfxVolume);

                // Update the color based on the switch state
                if (ECoordinator.HasComponent<PhysicsSystem::Switch>(*lastSelectedEntity)) {
                    //std::cout << ECoordinator.GetComponent<HUGraphics::GLModel>(*lastSelectedEntity).textureFile;
                    if (ECoordinator.GetComponent<HUGraphics::GLModel>(*lastSelectedEntity).textureFile == "./Assets/Textures\\SwitchesOn.png" || 
                        ECoordinator.GetComponent<HUGraphics::GLModel>(*lastSelectedEntity).textureFile == "./Assets/Textures\\SwitchesOff.png" || 
                        ECoordinator.GetComponent<HUGraphics::GLModel>(*lastSelectedEntity).textureFile == "SwitchesOn.png" ||
                        ECoordinator.GetComponent<HUGraphics::GLModel>(*lastSelectedEntity).textureFile == "SwitchesOff.png") {
                        if (switchPhysBody.Switch) {
                            activeTexture = TextureLibrary.GetAssets("SwitchesOn.png");
                            newTextureFile = "SwitchesOn.png";
                            audioEngine->PlaySound("Laser_Off.ogg", 0, 0.1f * sfxVolume);
                        }
                        else {
                            activeTexture = TextureLibrary.GetAssets("SwitchesOff.png");
                            newTextureFile = "SwitchesOff.png";
                            audioEngine->PlaySound("Laser_On.ogg", 0, 0.1f * sfxVolume);
                            
                        }
                    }
                    else {
                        if (switchPhysBody.Switch) {
                            activeTexture = TextureLibrary.GetAssets("DoorSwitchesOn.png");
                            newTextureFile = "DoorSwitchesOn.png";

                        }
                        else {
                            activeTexture = TextureLibrary.GetAssets("DoorSwitchesOff.png");
                            newTextureFile = "DoorSwitchesOff.png";
                        }
                        audioEngine->PlaySound("LockedDoorCut.ogg", 0, 0.3f * sfxVolume);
                    }

                    if (activeTexture) {
                        textureID = activeTexture->GetTextureID();
                        if (textureID == 0) {
                            //std::cerr << "Failed to load active texture: toplazergreen.png" << std::endl;
                        }
                    }
                    else {
                        //std::cerr << "Active texture not found in library!" << std::endl;
                    }

                    if (textureID != 0) {
                        switchModel.textureID = textureID;
                        switchModel.textureFile = newTextureFile; // Update the texture reference
                    }

                }
                //std::cout << "\ninteractables: ";
                for (const auto& interactable : switchBody.interactables) {


                    for (auto& entitycheck : ECoordinator.GetAllEntities()) {
                        // Check for name
                        if (ECoordinator.GetComponent<Name>(entitycheck).name != interactable) {
                            continue;
                        }
                        else {
                            //std::cout << " " << ECoordinator.GetComponent<Name>(entity).name.c_str();
                        }
                        // If name matches but no physicsBody, add in.
                        if (!ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entitycheck)) {
                            ECoordinator.AddComponent(entitycheck, PhysicsSystem::PhysicsBody{});
                        }
                        // Use a separate variable for the interactable entity's physics body
                        PhysicsSystem::PhysicsBody& interactablePhysBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entitycheck);

                        if (interactablePhysBody.category == "LockDoor") {
                            interactablePhysBody.Switch = !interactablePhysBody.Switch;
                            HUGraphics::GLModel& doorModel = ECoordinator.GetComponent<HUGraphics::GLModel>(entitycheck);

                            std::string newTextureFiles = interactablePhysBody.Switch ? "./Assets/Textures/OpenLockedDoorsV2.png" : "./Assets/Textures/LockedDoorV2.png";

                            // Ensure the texture is valid and update the model
                            if (!newTextureFiles.empty()) {
                                Texture& newTexture = *TextureLibrary.GetAssets(TextureLibrary.GetName(newTextureFiles));
                                doorModel = HUGraphics::texture_mesh(newTexture); // Update the model with the new texture
                                doorModel.textureFile = newTextureFiles;           // Store the texture file for reference
                            }
                        }
                        if (interactablePhysBody.category == "Laser") {
                            // If no laser component, add in.
                            if (!ECoordinator.HasComponent<LaserComponent>(entitycheck)) {
                                ECoordinator.AddComponent(entitycheck, LaserComponent{});
                            }

                            LaserComponent& laserComp = ECoordinator.GetComponent<LaserComponent>(entitycheck);
                            laserComp.turnedOn = !laserComp.turnedOn;
                        }
                    }
                }
            }

            // So it doesn't update every frame
            if (needsUpdate) {
                insideGroupEntities.clear();
                outsideGroupEntities.clear();

                for (auto& entitychecks : activeEntities) {
                    for (auto& interactable : ECoordinator.GetComponent<PhysicsSystem::Switch>(*lastSelectedEntity).interactables) {
                        if (!ECoordinator.HasComponent<Name>(entitychecks)) {
                            continue;
                        }
                        // If it has the name of the interactable within the switch's interactables array, move it into insideGroup
                        if (ECoordinator.GetComponent<Name>(entitychecks).name == interactable) {
                            insideGroupEntities.emplace_back(entitychecks);
                            break; // No need to check further interactables for this entity
                        }
                    }

                    // Else, move all thats not inside the insideGroup, into the outsideGroup.
                    auto it = std::find(insideGroupEntities.begin(), insideGroupEntities.end(), entitychecks);
                    if (it == insideGroupEntities.end()) {
                        outsideGroupEntities.emplace_back(entitychecks);
                    }
                }

                needsUpdate = false; // Reset the flag after updating
            }

            ImGui::Columns(3, "3Columns", true);
            for (auto& insideGroup : insideGroupEntities) {
                if (ImGui::Selectable(ECoordinator.GetComponent<Name>(insideGroup).name.c_str(), lastSelectedSwitchEntity == insideGroup)) {
                    lastSelectedSwitchEntity = insideGroup;
                };
            }
            ImGui::NextColumn();

            // Insert to the linked objects
            if (ImGui::ArrowButton("Move Left", ImGuiDir_Left)) {
                if (lastSelectedSwitchEntity.has_value()) {
                    auto it = std::find(outsideGroupEntities.begin(), outsideGroupEntities.end(), lastSelectedSwitchEntity);
                    if (it != outsideGroupEntities.end()) {
                        if (!ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(*lastSelectedSwitchEntity)) {
                            ECoordinator.AddComponent<PhysicsSystem::PhysicsBody>(*lastSelectedSwitchEntity, PhysicsSystem::PhysicsBody{});
                        }
                        ECoordinator.GetComponent<PhysicsSystem::Switch>(*lastSelectedEntity).interactables.emplace_back(ECoordinator.GetComponent<Name>(*lastSelectedSwitchEntity).name);
                        outsideGroupEntities.erase(it);
                    }
                    needsUpdate = true;
                }
            }
            ImGui::SameLine();
            ImGui::Text("Insert");
            ImGui::NewLine();

            // Takes it out of linked objects
            if (ImGui::ArrowButton("Move Right", ImGuiDir_Right)) {
                if (lastSelectedSwitchEntity.has_value()) {
                    auto it = std::find(insideGroupEntities.begin(), insideGroupEntities.end(), lastSelectedSwitchEntity);
                    if (it != insideGroupEntities.end()) {
                        // Remove the entity's name from the doorName list
                        auto& switchComponent = ECoordinator.GetComponent<PhysicsSystem::Switch>(*lastSelectedEntity);
                        auto nameIt = std::find(switchComponent.interactables.begin(), switchComponent.interactables.end(),
                            ECoordinator.GetComponent<Name>(*lastSelectedSwitchEntity).name);
                        if (nameIt != switchComponent.interactables.end()) {
                            switchComponent.interactables.erase(nameIt);
                        }


                        // Remove the entity from the insideGroupEntities list
                        insideGroupEntities.erase(it);
                    }
                    needsUpdate = true;
                }
            }
            ImGui::SameLine();
            ImGui::Text("Remove");
            ImGui::NextColumn();

            for (auto& outsideGroup : outsideGroupEntities) {
                if (ECoordinator.HasComponent<Name>(outsideGroup)) {
                    if (ImGui::Selectable(ECoordinator.GetComponent<Name>(outsideGroup).name.c_str(), lastSelectedSwitchEntity == outsideGroup)) {
                        lastSelectedSwitchEntity = outsideGroup;
                    };
                }
            }

            ImGui::Columns(1);

            // End the child window
            ImGui::EndChild();
        }



       

        
        // LASER
        if (ECoordinator.HasComponent<LaserComponent>(*lastSelectedEntity)) {


            ImGui::Separator();
            ImGui::Text("Laser Game Logic Component");

            LaserComponent& laserComp = ECoordinator.GetComponent<LaserComponent>(*lastSelectedEntity);


            // Toggle Laser turnedOn State
            ImGui::Checkbox("Turned On", &laserComp.turnedOn);

            // Toggle Laser Active State
            ImGui::Checkbox("Is Active", &laserComp.isActive);

            // Adjust Active Time
            ImGui::InputFloat("Active Time", &laserComp.activeTime, 0.2f, 1.0f, "%.2f sec");

            // Adjust Inactive Time
            ImGui::InputFloat("Inactive Time", &laserComp.inactiveTime, 0.2f, 1.0f, "%.2f sec");

            // Adjust Timer (if needed)
            ImGui::SliderFloat("Timer", &laserComp.timer, 0.0f, laserComp.activeTime, "%.2f sec");

           
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Delete");
            }

            // 🔗 Laser Module Linking (only if LaserComponent exists)
            if (ECoordinator.HasComponent<LaserComponent>(entity)) {
                if (laserModuleNames.empty()) {
                    ImGui::Text("No Laser Modules available.");
                }
                else {
                    laserComp = ECoordinator.GetComponent<LaserComponent>(entity);

                    // Generate C-style strings
                    std::vector<const char*> moduleCStrs;
                    for (const std::string& name : laserModuleNames) {
                        moduleCStrs.push_back(name.c_str());
                    }

                    // Get current index
                    int selectedModuleIndex = -1;
                    for (size_t i = 0; i < laserModuleNames.size(); ++i) {
                        if (laserComp.linkModuleID == laserModuleNames[i]) {
                            selectedModuleIndex = static_cast<int>(i);
                            break;
                        }
                    }

                    if (laserModuleNames.size() == 1) {
                        laserComp.linkModuleID = laserModuleNames[0];
                        ImGui::Text("Automatically linked to: %s", laserModuleNames[0].c_str());
                    }
                    else {
                        if (ImGui::Combo("Link to Laser Module", &selectedModuleIndex, moduleCStrs.data(), int(moduleCStrs.size()))) {
                            laserComp.linkModuleID = laserModuleNames[selectedModuleIndex];
                        }
                    }

                    ImGui::Text("Linked Laser Module Name: %s", laserComp.linkModuleID.c_str());
                }
            }

            if (ImGui::Button("Remove Laser Component##LaserDelete")) {
                ECoordinator.RemoveComponent<LaserComponent>(*lastSelectedEntity);
                //sig.reset(4);
            }
        }
        else {
            missingSig.set(7);
        }

        // PhysicsBody component
        if (sig.test(2)) {
            auto& physicsBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(*lastSelectedEntity);
            Transform& transform = ECoordinator.GetComponent<Transform>(*lastSelectedEntity);
            ImGui::Text("PhysicsBody");

            // Delete button for PhysicsBody component
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - buttonWidth);  // Move delete button to the far right
            if (ImGui::Button("x##PhysicsBody")) {
                ECoordinator.RemoveComponent<PhysicsSystem::PhysicsBody>(*lastSelectedEntity);
                //sig.reset(2);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Delete");
            }

            ImGui::Text("Mass");
            ImGui::InputFloat("Mass", &physicsBody.mass);

            ImGui::Text("Gravity");
            ImGui::InputInt("Gravity", &gravity);


            /*
            ImGui::Text("Category: %s", physicsBody.category.c_str());
            const char* hold = nullptr; // Start as null to leave it blank

            if (selectedCategory >= 0) {
                hold = categories[selectedCategory].c_str(); // Assign valid string
            }

            if (ImGui::BeginCombo("##Category", hold ? hold : "")) {
                for (int i = 0; i < categories.size(); ++i) {
                    bool isSelected = (selectedCategory == i);
                    if (ImGui::Selectable(categories[i].c_str(), isSelected)) {
                        selectedCategory = i;
                        physicsBody.category = categories[selectedCategory]; // Set new category

                        // If the selected category is "Laser Module", scan for laser modules
                        if (physicsBody.category == "Laser Module") {
                            ScanLaserModules();  // Refresh the laser modules
                        }

                        if (physicsBody.category == "Thief") {
                            if (ECoordinator.hasThiefID()) {
                                ImGui::OpenPopup("Thief Already Assigned");


                                physicsBody.category = "";
                            }
                            else {
                                ECoordinator.setThiefID(lastSelectedEntity.value());
                            }
                        }
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // REMOVE CURRENTLY SELECTED CATEGORY
            ImGui::SameLine();

            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - buttonWidth);
            if (!categories.empty() && ImGui::Button("x##Category")) {
                if (categories.size() > 1) { // Prevent removing all categories
                    // Remove the category
                    categories.erase(categories.begin() + selectedCategory);

                    // Ensure the selectedCategory is valid after removal
                    selectedCategory = std::max(0, selectedCategory - 1);

                    // Now update the categories in the JSON file
                    SaveCategoriesToJson("Category.json");
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("If accidentally deleted a category, \nplease go to your history in Github Desktop to discard changes.");
            }

            // ADD NEW CATEGORY INPUT
            ImGui::InputText("New", newCategory, IM_ARRAYSIZE(newCategory));
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - buttonWidth);

            if (ImGui::IsItemActive()) {
                InputSystem->Disable();
            }
            else {
                InputSystem->Enable();
            }

            if (ImGui::Button("+")) {
                if (strlen(newCategory) > 0) {
                    categories.push_back(newCategory);
                    memset(newCategory, 0, sizeof(newCategory)); // Clear input

                    SaveCategoriesToJson("Category.json");
                }
            }

            */
            ImGui::Text("AABB");

            ImGui::SameLine();

            if (ImGui::Button("inherit")) {
                physicsBody.aabb.maxX = transform.translate.x + transform.scale.x / 2;
                physicsBody.aabb.minX = transform.translate.x - transform.scale.x / 2;
                physicsBody.aabb.maxY = transform.translate.y + transform.scale.y / 2;
                physicsBody.aabb.minY = transform.translate.y - transform.scale.y / 2;
            }

            // Define step size for AABB adjustments
            float aabbStep = 1.0f;

            // MinX
            if (ImGui::Button("-##MinX")) { physicsBody.aabb.minX -= aabbStep; }
            ImGui::SameLine();
            if (ImGui::Button("+##MinX")) { physicsBody.aabb.minX += aabbStep; }
            ImGui::SameLine();
            ImGui::InputFloat("MinX", &physicsBody.aabb.minX, 0.0f, 0.0f, "%.3f");

            if (ImGui::Button("-##MaxX")) { physicsBody.aabb.maxX -= aabbStep; }
            ImGui::SameLine();
            if (ImGui::Button("+##MaxX")) { physicsBody.aabb.maxX += aabbStep; }
            ImGui::SameLine();
            ImGui::InputFloat("MaxX", &physicsBody.aabb.maxX);

            if (ImGui::Button("-##MinY")) { physicsBody.aabb.minY -= aabbStep; }
            ImGui::SameLine();
            if (ImGui::Button("+##MinY")) { physicsBody.aabb.minY += aabbStep; }
            ImGui::SameLine();
            ImGui::InputFloat("MinY", &physicsBody.aabb.minY);

            if (ImGui::Button("-##MaxY")) { physicsBody.aabb.maxY -= aabbStep; }
            ImGui::SameLine();
            if (ImGui::Button("+##MaxY")) { physicsBody.aabb.maxY += aabbStep; }
            ImGui::SameLine();
            ImGui::InputFloat("MaxY", &physicsBody.aabb.maxY);


        }
        else {
            missingSig.set(2);
        }



        // RenderLayer component
        if (sig.test(3)) {
            ImGui::Separator();
            const char* renderLayerItems[] = { "Background", "Game Object", "UI" };
            int layerIndex = static_cast<int>(ECoordinator.GetComponent<RenderLayer>(*lastSelectedEntity).layer);
            ImGui::Text("RenderLayer");
            if (ImGui::ListBox("##", &layerIndex, renderLayerItems, IM_ARRAYSIZE(renderLayerItems))) {
                ECoordinator.GetComponent<RenderLayer>(*lastSelectedEntity).layer = static_cast<RenderLayerType>(layerIndex);
            }
        }
        else {
            missingSig.set(3);
        }


       

        // "Add Components" Section
        ImGui::Separator();
        ImGui::Text("Add Components:");

        if (missingSig.test(4) && ImGui::Button("Add Name Component")) {
            ECoordinator.AddComponent<Name>(*lastSelectedEntity, Name{});
            sig.set(4);
            missingSig.reset(4);
        }
        if (missingSig.test(0) && ImGui::Button("Add Transform Component")) {
            ECoordinator.AddComponent<Transform>(*lastSelectedEntity, Transform{});
            sig.set(0);
            missingSig.reset(0);
        }
        if (missingSig.test(1) && ImGui::Button("Add GLModel Component")) {
            ECoordinator.AddComponent<HUGraphics::GLModel>(*lastSelectedEntity, HUGraphics::GLModel{});
            sig.set(1);
            missingSig.reset(1);
        }
        if (missingSig.test(2) && ImGui::Button("Add PhysicsBody Component")) {
            ECoordinator.AddComponent<PhysicsSystem::PhysicsBody>(*lastSelectedEntity, PhysicsSystem::PhysicsBody{});
            Transform& transform = ECoordinator.GetComponent<Transform>(*lastSelectedEntity);
            PhysicsSystem::PhysicsBody& body = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(*lastSelectedEntity);
            float halfWidth = transform.scale.x / 2.0f;
            float halfHeight = transform.scale.y / 2.0f;

            body.aabb.minX = transform.translate.x - halfWidth;
            body.aabb.maxX = transform.translate.x + halfWidth;
            body.aabb.minY = transform.translate.y - halfHeight;
            body.aabb.maxY = transform.translate.y + halfHeight;
            sig.set(2);
            missingSig.reset(2);
        }
        if (missingSig.test(3) && ImGui::Button("Add RenderLayer Component")) {
            ECoordinator.AddComponent<RenderLayer>(*lastSelectedEntity, RenderLayer{});
            sig.set(3);
            missingSig.reset(3);
        }
    }

    // Check if an entity is selected
    if (lastSelectedEntity.has_value()) {
        // Delete the selected entity when the button is clicked
        if (ImGui::Button("Delete Selected Entity")) {
            if (lastSelectedEntity.has_value()) {

                if (lastSelectedEntity.value() == ECoordinator.getThiefID()) {
                    ECoordinator.resetThiefID();
                }
                // Call the destroy function in ECoordinator
                ECoordinator.DestroyGameObject(*lastSelectedEntity);

                // Reset the selection after deletion
                lastSelectedEntity.reset();  // Clears the selected entity
                selectedEntity.reset();
            }
        }
    }

    if (ImGui::BeginPopupModal("Thief Already Assigned", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("A Thief entity is already assigned!\nPlease reset it before selecting a new one.");
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();  // Close the popup when clicked
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

/*
 * @brief Renders the TextureAsset editing scene
 */
void renderTextureAssetEdit(Texture& texture) {
    // Begin the window, passing the "open" variable to track if it's closed
    if (!ImGui::Begin("TextureAsset Editing", &isEditTextureAsset, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
        isEditTextureAsset = false;
        ImGui::End(); // Ensure the window is ended even if it's closed
        return; // Early exit if the window is closed
    }

    float maxHeight = 100.0f;
    float maxWidth = 200.0f;
    float TextureAssetAspectRatio = TextureAssetImageWidth / TextureAssetImageHeight;
    float displayHeight = TextureAssetImageHeight;
    float displayWidth = TextureAssetImageWidth;

    if (displayWidth > maxWidth) {
        displayWidth = maxWidth;
        displayHeight = displayWidth / TextureAssetAspectRatio; // Adjust height to maintain aspect ratio
    }
    if (displayHeight > maxHeight) {
        displayHeight = maxHeight;
        displayWidth = displayHeight * TextureAssetAspectRatio; // Adjust width to maintain aspect ratio
    }

    ImGui::Image(TextureAssetTextureID, ImVec2(displayWidth, displayHeight));

    ImGui::Separator();

    ImGui::Text(TextureAssetTextureFileName.c_str());

    ImGui::NewLine();

    ImGui::Text("Width");
    ImGui::InputFloat("##TextureAssetImageWidth", &TextureAssetImageWidth);

    ImGui::NewLine();

    ImGui::Text("Height");
    ImGui::InputFloat("##TextureAssetImageHeight", &TextureAssetImageHeight);

    if (ImGui::Button("Save")) {
        // File paths
        std::filesystem::path inputPath = TextureAssetTextureFileName;
        std::filesystem::path outputPath = TextureAssetTextureFileName;

        // Resize the image and save
        try {
            resizeImageWithSTB(inputPath, outputPath, static_cast<int>(TextureAssetImageWidth), static_cast<int>(TextureAssetImageHeight));
            std::vector<EntityID> mEntities = ECoordinator.GetAllEntities();
            for (auto& entity : mEntities) {
                if (ECoordinator.HasComponent<HUGraphics::GLModel>(entity)) {
                    auto& mdl = ECoordinator.GetComponent<HUGraphics::GLModel>(entity);
                    if (mdl.textureID == TextureAssetTextureID) {
                        if (textureIDtoTextureFileMap.find(mdl.textureID) != textureIDtoTextureFileMap.end()) {
                            texture = *TextureLibrary.GetAssets(TextureLibrary.GetName(textureIDtoTextureFileMap[mdl.textureID]));
                            texture.RefreshTexture();
                            mdl.textureFile = texture.GetFileName();
                            mdl = HUGraphics::texture_mesh(texture);
                            if (ECoordinator.HasComponent<Transform>(entity)) {
                                auto& scale = ECoordinator.GetComponent<Transform>(entity).scale;
                                scale.x = TextureAssetImageWidth;
                                scale.y = TextureAssetImageHeight;
                            }
                        }
                        else {
                            std::cerr << "Texture ID " << mdl.textureID << " not found in textureMap!" << std::endl;
                        }
                    }
                }
            }
            ImGui::Text("Image saved successfully!");
        }
        catch (const std::exception& e) {
            ImGui::Text("Failed to save image: %s", e.what());
        }
    }

    ImGui::End(); // End of the window
}

/*
 * @brief Allows user to see what audio is being played at the current moment
 */
void audioAnalysis() {
    ImGui::Begin("Audio Analysis");

    // Cache to store loaded textures by filename
    std::map<int, std::string> d = audioEngine->ListSounds();
    for (auto x : d) {
        ImGui::Text(x.second.c_str());
    }
    


    ImGui::End(); // End of the window
}

/*
 * @brief Resizes the image to be a square icon
 *
 * @param inputPath filepath of the file
 * @param outputPath mostly also the filepath of the file
 * @param imgTargetWidth forces width
 * @param imgTargetWidth forces height
 */
void resizeImageWithSTB(const std::filesystem::path& inputPath,
    const std::filesystem::path& outputPath,
    int imgTargetWidth,
    int imgTargetHeight) {
    // Load the image using stb_image
    int width, height, channels;
    unsigned char* data = stbi_load(inputPath.string().c_str(), &width, &height, &channels, 0);
    if (!data) {
        throw std::runtime_error("Failed to load image");
    }

    // Allocate memory for the resized image
    unsigned char* resizedData = new unsigned char[imgTargetWidth * imgTargetHeight * channels];
    if (!resizedData) {
        stbi_image_free(data);
        throw std::runtime_error("Failed to allocate memory for resized image");
    }

    // Perform nearest-neighbor resizing
    for (int y = 0; y < imgTargetHeight; ++y) {
        for (int x = 0; x < imgTargetWidth; ++x) {
            int srcX = x * width / imgTargetWidth;
            int srcY = y * height / imgTargetHeight;
            for (int c = 0; c < channels; ++c) {
                resizedData[(y * imgTargetWidth + x) * channels + c] =
                    data[(srcY * width + srcX) * channels + c];
            }
        }
    }

    // Save the resized image
    if (!stbi_write_png(outputPath.string().c_str(), imgTargetWidth, imgTargetHeight, channels, resizedData, imgTargetWidth * channels)) {
        delete[] resizedData;
        stbi_image_free(data);
        throw std::runtime_error("Failed to save resized image");
    }

    // Clean up
    delete[] resizedData;
    stbi_image_free(data);
}

/*
 * @brief Renders the possible rendered layers tab, checkboxes to check which layers should be rendered
 */
void RenderLayers() {
    ImGui::Checkbox("Show Background", &layerVisibility[0]);
    ImGui::Checkbox("Show GameObjects", &layerVisibility[1]);
    ImGui::Checkbox("Show UI", &layerVisibility[2]);
}


/*
 * @brief Renders the Layer selection
 */
void RenderLayerSelection() {
    const char* renderLayerItems[] = { "Background", "Game Object", "UI" };

    ImGui::Text("Render Layer:");
    for (int i = 0; i < IM_ARRAYSIZE(renderLayerItems); ++i) {
        bool isSelected = (currentRenderLayerIndex == i);  // Check if the item is currently selected
        if (ImGui::Selectable(renderLayerItems[i], isSelected)) {
            currentRenderLayerIndex = i;  // Update selected index
            gizmoChoice = 4;
        }
    }
}

/*
 * @brief Allows you to swap between Layer Selection and Display
 */
void RenderLayerWindow() {
    ImGui::Begin("Layering", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginTabBar("Layering")) {
        if (ImGui::BeginTabItem("Layer Display")) {
            RenderLayers();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Layer Selection")) {
            RenderLayerSelection();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

/*
 * @brief Renders the Entity List
 */
void RenderEntityList() {
    ImGui::Begin("EntityList");

    std::vector<EntityID> entities = ECoordinator.GetAllEntities();
    int i = 1; // Start from 1 for Entity 1, Entity 2, etc.
    for (auto& entity : entities) {
        // Create a label with a unique identifier for each entity
        std::string label = "Entity " + std::to_string(i);

        // Check if the entity was clicked to update lastSelectedEntity
        if (ImGui::Selectable(label.c_str(), lastSelectedEntity == entity)) {
            lastSelectedEntity = entity; // Update lastSelectedEntity with the selected entity
        }
        i++;
    }

    ImGui::End();
}

/*
 * @brief Shows all the possible level jsons
 */

void ShowLevelManagerWindow() {
    ImGui::Begin("Level Manager");

    static char levelName[128] = "";

    // Load level names from the "Json" folder dynamically
    static bool levelsLoaded = false; // Prevent unnecessary reloads

    if (!levelsLoaded) {
        levelList.clear();
        for (const auto& entry : fs::directory_iterator("./Json")) {
            if (entry.path().extension() == ".json") { // Only add JSON files
                std::string filename = entry.path().filename().string();
                // Exclude specific categories
                if (filename.find("Category") == std::string::npos &&
                    filename.find("PlayerAnimation") == std::string::npos &&
                    filename.find("spritesheet_ref") == std::string::npos) {
                    levelList.push_back(filename);
                }
            }
        }
        levelsLoaded = true; // Mark levels as loaded to prevent redundant operations
    }



    if (ImGui::BeginCombo("##currentLevel", currentLevel.c_str())) {
        for (int i = 0; i < levelList.size(); ++i) {
            bool isSelected = (currentSelectedLevel == i);
            if (ImGui::Selectable(levelList[i].c_str(), isSelected)) {
                // Load the selected level
                ECoordinator.ClearAllEntities();
                currentLevel = levelList[i];
                currentSelectedLevel = i;
                timerObj.Reset();

                undoStack = std::stack<ObjectState>();
                // Set the stage based on filename
                auto it = StringToGameState.find(currentLevel);
                if (it != StringToGameState.end()) {
                    CoreEngine::InputSystem::Stage = static_cast<int>(it->second);
                    CreateObjectsForStage(CoreEngine::InputSystem::Stage);

                    totalObjects = 0;
                    auto Entities = ECoordinator.GetAllEntities();
                    for (const auto& entity : Entities) {
                        //std::
                        // << "Entity ID: " << entity << ", Name: " << name.name << '\n';
                        if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {
                            PhysicsSystem::PhysicsBody& physBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);

                            if (physBody.category == "Object") {
                                totalObjects += 1;
                            }
                        }
                    }
                }
                else {
                    // Fallback to a default state if filename not found
                    CoreEngine::InputSystem::Stage = static_cast<int>(GameState::NotApplicable);
                    std::cerr << "Warning: No GameState mapping for file: " << currentLevel << std::endl;
                }

                selectedEntity.reset();
                lastSelectedEntity.reset();
                
            }
        }
        ImGui::EndCombo();
    }

    // Save Level Button
    if (ImGui::Button("Save Level")) {
        if (currentLevel.empty()) {
            std::cerr << "[ERROR] No level loaded! Load a level before saving.\n";
        }
        else {
            savePath = currentLevel;

            SaveGameObjectsToJson(savePath);

        }
    }
    ImGui::End();
}


/*
 * @brief Shows the resource graphs for each system
 */
void RenderResourceGraph() {
    ImGui::Begin("Resource Graph");

    std::string output;

    if (isPaused) {
        TateEngine->CheckSystemProcess(0, output);  // Get system time data
    }
    else {
        TateEngine->CheckSystemProcess(deltaTime, output);  // Get system time data
    }
        
    TateEngine->CheckSystemProcess(deltaTime, output);  // Get system time data

    std::vector<std::string> systemNames;
    std::vector<float> systemTimes;
    std::vector<float> systemPercentages;
    std::istringstream iss(output);
    std::string line;

    // Extract data from output string
    while (std::getline(iss, line)) {
        size_t nameEnd = line.find(" Time:");
        if (nameEnd != std::string::npos) {
            systemNames.push_back(line.substr(0, nameEnd));
            size_t timeStart = line.find(": ", nameEnd) + 2;
            size_t timeEnd = line.find(" ms", timeStart);
            systemTimes.push_back(std::stof(line.substr(timeStart, timeEnd - timeStart)));
            size_t percentStart = line.find("(", timeEnd) + 1;
            size_t percentEnd = line.find("%", percentStart);
            if (percentStart != std::string::npos && percentEnd != std::string::npos) {
                systemPercentages.push_back(std::stof(line.substr(percentStart, percentEnd - percentStart)));
            }
        }
    }


    ImGui::Text("System Resource Usage");
    ImGui::Separator();

    if (!systemTimes.empty()) {
        // Find the maximum time to scale the progress bars
       // float maxTime = *std::max_element(systemTimes.begin(), systemTimes.end());

        // Define a list of colors to assign to each system (cycle through these if more than available)
        std::vector<ImVec4> systemColors = {
            ImVec4(0.7f, 0.2f, 0.2f, 1.0f), // Red
            ImVec4(0.2f, 0.7f, 0.2f, 1.0f), // Green
            ImVec4(0.2f, 0.2f, 0.7f, 1.0f), // Blue
            ImVec4(1.0f, 0.8f, 0.2f, 1.0f), // Yellow
            ImVec4(0.8f, 0.2f, 1.0f, 1.0f), // Purple
            ImVec4(0.0f, 1.0f, 1.0f, 1.0f)  // Cyan
        };

        // Render each system's bar with its unique color
        for (size_t i = 0; i < systemTimes.size(); ++i) {
            ImGui::Text("%s", systemNames[i].c_str());
            ImGui::SameLine(150);

            // Cycle through the colors based on system index
            ImVec4 systemColor = systemColors[i % systemColors.size()];  // Cycle color if more systems than colors

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, systemColor);
            ImGui::ProgressBar(systemPercentages[i] / 100.0f, ImVec2(0, 20));  // Scale based on the percentage
            ImGui::PopStyleColor();

            // Display exact time and percentage beside each bar
            ImGui::SameLine();
            ImGui::Text(" %.2f ms (%.2f%%)", systemTimes[i], systemPercentages[i]);
        }
    }
    else {
        ImGui::Text("No system resource data available.");
    }

    ImGui::End();
}


/*
 * @brief Stops the game
 */
void ECSCoordinator::StopGame() {
    //GameState currentStage;
    // Clear current game state (if needed)
    
    ClearAllEntities();
    lastSelectedEntity.reset();
    selectedEntity.reset();
    timerObj.Reset();
    undoStack = std::stack<ObjectState>();
    CreateObjectsForStage(CoreEngine::InputSystem::Stage);
    totalObjects = 0;
    Object_picked = 0;
    auto Entities = ECoordinator.GetAllEntities();
    for (const auto& entity : Entities) {
        //std::
        // << "Entity ID: " << entity << ", Name: " << name.name << '\n';
        if (ECoordinator.HasComponent<PhysicsSystem::PhysicsBody>(entity)) {
            PhysicsSystem::PhysicsBody& physBody = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entity);

            if (physBody.category == "Object") {
                totalObjects += 1;
            }
        }
    }
}

/*
 * @brief Populates a local texture map
 */
void PopulateTextureMap() {
    auto loadedAssets = TextureLibrary.GetAllLoadedAssets(); // Assuming GetAllLoadedAssets() returns a map of assets
    for (const auto& assetPair : loadedAssets) {
        std::string assetName = assetPair.first;
        const std::shared_ptr<Texture>& asset = assetPair.second;

        textureIDtoTextureFileMap[asset->GetTextureID()] = asset->GetFileName();
    }
}

/*
 * @brief Renders clicking state for the clicking button
 */
void RenderClickState(bool& allowClickingWhenTrue, std::string label) {
    // Save the original style colors
    ImVec4 originalButtonColor = ImGui::GetStyle().Colors[ImGuiCol_Button];
    ImVec4 originalHoveredColor = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
    ImVec4 originalActiveColor = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
    ImVec4 originalTextColor = ImGui::GetStyle().Colors[ImGuiCol_Text];
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // Black text

    // Set colors based on allowClickingWhenTrue
    if (allowClickingWhenTrue) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
    }
    else {


        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.4f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.3f, 0.3f, 1.0f));
    }

    // Render the button
    if (ImGui::Button(label.c_str())) {
        allowClickingWhenTrue = !allowClickingWhenTrue;
        gizmoChoice = 4;
    }

    ImGui::PopStyleColor(4); // Restore Text, Button, Hovered, and Active colors
}

/*
 * @brief Callback for glfw to take in files
 */
void glfwDropCallback(GLFWwindow* window, int count, const char** paths) {
    (void)window;

    for (int i = 0; i < count; i++) {
        gDroppedFiles.push_back(paths[i]);
    }
    for (auto& file : gDroppedFiles) {
        (void)file;
    }
}

/*
 * @brief Checks the extensions of the files (for drag&drop window > level editor)
 *
 * @param filename The file being dragged in
 * @param validExtensions The file extensions that are allowed
 */
bool hasValidExtension(const std::string& filename, const std::vector<std::string>& validExtensions) {
    std::filesystem::path filePath(filename);
    std::string extension = filePath.extension().string();
    for (const auto& ext : validExtensions) {
        if (extension == ext) {
            return true;
        }
    }
    return false;
}

/**
* @brief Binds the several game states to the level manager
* @param GameState state of the game to be binded
*/
std::string GameStateToJsonFile(GameState state) {
    // Map enum values to strings
    static const std::unordered_map<GameState, std::string> mapping = {
        {GameState::MainMenu,      "Main_Menu.json"},
        {GameState::Playing,       "GameObjects.json"},
        {GameState::Lose,          "LoseMenu.json"},
        {GameState::LevelSelect, "LevelSelect.json"},
        {GameState::Pause, "PauseMenu.json" },
        {GameState::HowToPlay, "HowToPlay.json" },
        {GameState::HowToPlay2, "HowToPlay.json" },
        {GameState::confirmQuit, "ConfirmQuit.json" },
        {GameState::confirmQuit2, "ConfirmQuit.json" },
        {GameState::Playing1, "Level1.json" },
        {GameState::Playing3, "Level2.json" },
        {GameState::Playing2, "Level3.json" },
        {GameState::cutScene, "cutScene.json" },
        {GameState::endScene, "endScene.json" },
        {GameState::gameWon, "" },
        {GameState::starRating, "StarRating.json" },
        {GameState::splashscreen, "splashscreen.json" }
    };

    if (auto it = mapping.find(state); it != mapping.end()) {
        return it->second;
    }
    throw std::runtime_error("Invalid GameState");
}

namespace ImGuiManager {

    int imguiWidth;
    int imguiHeight;
    //ImGuiIO& io = ImGui::GetIO();
    ImGuiIO* io = nullptr;

    /*
     * @brief Initializes the imgui on the main window
     *
     * @param window: Gets the GLFWwindow context
     */
    void Initialize(GLFWwindow* window) {

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        if (ImGui::GetCurrentContext() == nullptr) {
            std::cerr << "ImGui context not initialized!" << std::endl;
            return;
        }
        io = &ImGui::GetIO();

        io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        io->ConfigDockingAlwaysTabBar = true;
        io->ConfigDockingTransparentPayload = true;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 450");

        glfwSetDropCallback(window, glfwDropCallback);

        SetupFBO(screen_width, screen_height); // Set the initial size for the FBO

        //Initialize the map as well since this comes after libraries get loaded
        PopulateTextureMap();
    }

    /*
     * @brief Shuts down the whole ImGui
     */
    void Shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        for (auto& texture : textureCache) {
            glDeleteTextures(1, &texture.second);
        }
        textureCache.clear();
    }

    /*
     * @brief Updates the audio depending which are playing
     */
    void UpdateAudioStates() {
        // Checks for the string:audio pair
        for (auto& assetPair : audioPlayedStates) {
            const std::string& assetName = assetPair.first;
            bool& hasPlayed = assetPair.second;
            bool& isSelected = assetSelectionStates[assetName]; // Reference the selection state for this asset

            // Update the hasPlayed state based on whether the audio is still playing
            if (hasPlayed && !audioEngine->isPlaying(assetName.c_str())) {
                hasPlayed = false;  // Audio has stopped playing, so reset the state
                isSelected = false; // Optionally, deselect the button if desired
            }
        }
    }

    /*
     * @brief Getter for GameLogic.cpp to render certain layers
     */
    bool* getVisibleLayers() {
        return layerVisibility;
    }

    /*
     * @brief Renders the ImGui
     *
     * @param showImGuiWindow: Shows depending L is pressed from core.cpp, can toggle between ImGui or no ImGui
     */
    void RenderImGui(bool showImGuiWindow) {
        // Check if we should render the ImGui window

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (showImGuiWindow) {
            // Create a full-screen dockspace
            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            ImGuiViewport* viewport = ImGui::GetMainViewport();

            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always); // Allow resizing
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

            // Add flags to prevent moving and resizing
            windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            // Begin the main dockspace window
            ImGui::Begin("DockSpace Demo", nullptr, windowFlags);
            ImGui::PopStyleVar(2);

            // Create a dockspace ID
            ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None); // Change flags here if needed

            // Top Bar
            if (ImGui::BeginMenuBar())
            {
                // Left-aligned text
                ImGui::Text("Tate Engine");

                // Add an invisible item to create space and push the button to the center
                ImGui::Dummy(ImVec2(0, 0));  // This is just an empty space, can be omitted.

                // Place the arrow button in the center
                ImGui::SameLine((ImGui::GetWindowWidth() - ImGui::CalcTextSize("HU Engine").x) / 2 - 16); // Adjust 16px for the button size


                if (isPaused) {
                    // Show the arrow button when not paused
                    if (ImGui::ArrowButton("arrow_right", ImGuiDir_Right)) {
                        // Change the state to paused when clicked
                        isPaused = false;
                        timerObj.Resume();

                    }
                }
                else {
                    // Show the Pause button when paused
                    if (ImGui::Button("Pause")) {
                        isPaused = true;
                        deltaTime = 0;
                        timerObj.Pause();
                    }
                }
                if (ImGui::Button("Stop")) {
                    ECoordinator.StopGame();
                }

                RenderClickState(allowClickingIfTrue, "Entity Picking");
                RenderClickState(allowThiefMoveIfTrue, "Thief Jumping");

                ImGui::EndMenuBar();
            }

            RenderDefaultScene();

            // End the dockspace window
            ImGui::End();

            // Render ImGui
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Handle multiple viewports (for multi-window applications)
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    /*
     * @brief Sets up the frame buffer for ImGui to render the Main Scene as a texture
     *
     * @param width: width of the texture
     * @param height: height of the texture
     */
    GLuint SetupFBO(int width, int height) {
        imguiWidth = width;
        imguiHeight = height;
        // Clean up existing buffers if they exist
        if (fboTexture) {
            glDeleteTextures(1, &fboTexture);
            fboTexture = 0;
        }
        if (fbo) {
            glDeleteFramebuffers(1, &fbo);
            fbo = 0;
        }
        if (rboDepth) {
            glDeleteRenderbuffers(1, &rboDepth);
            rboDepth = 0;
        }

        // Generate and bind the framebuffer
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // Generate and bind the texture
        glGenTextures(1, &fboTexture);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Attach the texture to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

        // Generate and bind the renderbuffer for depth attachment
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

        // Check for completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            //std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        }

        // Unbind the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // Return the texture ID linked to the FBO
        return fboTexture;
    }

    /*
     * @brief Renders the scene from the original main window to a frame buffer
     *
     * @param deltaTime: Allows ImGui to update along with whatever is happening in the game
     */
    void RenderSceneToFBO(double deltatime) {
        // Bind the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // Clear the framebuffer before rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (isPaused) {
            ECoordinator.UpdateSystems(0);
            TateEngine->CheckSystemProcess(0, SystemTimeOutput);
            deltaTime = static_cast<float>(0);
        }
        else {
            ECoordinator.UpdateSystems(deltatime);
            TateEngine->CheckSystemProcess(deltatime, SystemTimeOutput);
            deltaTime = static_cast<float>(deltatime);
        }
        

        // Unbind the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    /*
     * @brief DisplayEntityList
     *
     * @param selectedEntityID: Selects the Entity
     */
    void DisplayEntityList(int& selectedEntityID) {
        for (const auto& entity : activeEntities) {
            // Allow selecting and dragging entities
            if (ImGui::Selectable(("Entity " + std::to_string(entity)).c_str(), static_cast<unsigned int>(selectedEntityID) == entity)) {
                selectedEntityID = entity; // Set the selected entity
            }

            // Drag and drop payload setup
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("ENTITY_PAYLOAD", &entity, sizeof(EntityID)); // Set the payload with the entity ID
                ImGui::Text("Dragging %s", ("Entity " + std::to_string(entity)).c_str());
                ImGui::EndDragDropSource();
            }
        }
    }

    /*
      * @brief DisplayEntityList
      *
      * @param selectedEntityID: Selects the Entity
      */
    void ResizeFBO(int newWidth, int newHeight) {
        if (newWidth == imguiWidth && newHeight == imguiHeight) {
            // No need to resize if dimensions are unchanged
            return;
        }

        imguiWidth = newWidth;
        imguiHeight = newHeight;

        

        // Clean up the existing FBO resources
        if (fboTexture) {
            glDeleteTextures(1, &fboTexture);
            fboTexture = 0;
        }
        if (rboDepth) {
            glDeleteRenderbuffers(1, &rboDepth);
            rboDepth = 0;
        }

        // Update the framebuffer texture
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // Create the new texture for the FBO
        glGenTextures(1, &fboTexture);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newWidth, newHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Attach the new texture to the FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

        // Update the renderbuffer for depth attachment
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, newWidth, newHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

        // Check framebuffer completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            //std::cerr << "Error: Framebuffer is not complete after resizing!" << std::endl;
        }

        // Unbind the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
