/**
 * @file JSONSerialization.cpp
 * @brief Implementation of JSON serialization and deserialization for game objects in the ECS framework.
 *
 * This file provides functions for loading and saving game objects to and from JSON files. It includes
 * functionality for handling various components such as `Transform`, `PhysicsBody`, `RenderLayer`, and custom
 * models. The file supports dynamic entity creation and updates the ECS framework accordingly.
 *
 * Key Features:
 * - **JSON Deserialization**:
 *   - `LoadGameObjectsFromJson`: Reads a JSON file and creates game entities with appropriate components.
 *   - Supports various entity types, including text, textures, and animated textures.
 * - **JSON Serialization**:
 *   - `SaveGameObjectsToJson`: Writes the current state of all game entities into a JSON file, including their components.
 * - **Component Management**:
 *   - Handles components such as `Transform`, `HUGraphics::GLModel`, `PhysicsBody`, `RenderLayer`, and `Name`.
 * - **Dynamic Path Normalization**:
 *   - `normalizePath`: Normalizes file paths for compatibility across different systems.
 *
 * Functions:
 * - `LoadGameObjectsFromJson`:
 *   - Reads a JSON file to create entities and assigns components dynamically.
 *   - Handles advanced configurations such as animated textures and color properties.
 * - `SaveGameObjectsToJson`:
 *   - Exports the current ECS state to a JSON file, ensuring all components are serialized correctly.
 * - `normalizePath`:
 *   - Replaces backslashes with forward slashes for consistent path handling.
 *
 * Example JSON Structure:
 * ```json
 * {
 *   "entities": [
 *     {
 *       "name": "Player",
 *       "components": {
 *         "Transform": {
 *           "scale": {"x": 1.0, "y": 1.0, "z": 1.0},
 *           "translate": {"x": 0.0, "y": 0.0, "z": 0.0},
 *           "rotate": 0.0
 *         },
 *         "type": "texture",
 *         "textureFile": "./Assets/Textures/player.png"
 *       }
 *     }
 *   ]
 * }
 * ```
 *
 * Author Contributions:
 * - Che Ee (30%)
 * - Lewis (50%)
 * - RuiJie (20%)
 */
#include <iostream>
#include "JSONSerialization.h"
#include "vector3d.h"
#include <filesystem>
#include <shlobj.h>
#include "ParticleSystem.h"
#include "Coordinator.h"
#include "Render.h"
#include "ButtonComponent.h"


std::string initialGameFilePath;
namespace fs = std::filesystem;
std::unordered_map<unsigned int, Math3D::Vector3D> originalScales;


void loadgame(json j) {
    if (j.contains("categories")) {
        const auto& categoryList = j["categories"];
        for (const auto& category : categoryList) {
            categories.push_back(category.get<std::string>());
        }
    }


    for (const auto& entity : j["entities"]) {
        if (entity["components"].contains("type")) {
            std::string type = entity["components"]["type"];
            if (type == "text_texture") {
                std::string text = entity["components"].value("text", " ");
                float scale = entity["components"].value("scale", 1.0f);

                glm::vec3 color = glm::vec3(1.0f);  // Default to white
                if (entity["components"].contains("color") && entity["components"]["color"].is_object()) {
                    color = glm::vec3(
                        entity["components"]["color"].value("r", 1.0f),
                        entity["components"]["color"].value("g", 1.0f),
                        entity["components"]["color"].value("b", 1.0f)
                    );
                }

                float posX = entity["components"]["Transform"]["translate"]["x"];
                float posY = entity["components"]["Transform"]["translate"]["y"];
                float width = entity["components"]["Transform"]["scale"]["x"];
                float height = entity["components"]["Transform"]["scale"]["y"];

                std::string fontname = entity["components"].value("fontname", "Orbitron.ttf");
                int size = entity["components"].value("size", 24);


                // Call CreateTextEntity to handle this text entity
                ECoordinator.CreateTextEntity(text, scale, color, posX, posY, width, height, fontname, size, entity["name"]);
                continue;
            }
        }


        EntityID newEntity = ECoordinator.CreateGameObject(); // Create a new entity
        Signature entitySig = ECoordinator.GetEntitySignature(newEntity);
        HUGraphics::GLModel model;
        Transform transform;


        // Transform check
        if (entity["components"].contains("Transform")) {

            transform.scale.x = entity["components"]["Transform"]["scale"]["x"];
            transform.scale.y = entity["components"]["Transform"]["scale"]["y"];
            transform.scale.z = entity["components"]["Transform"]["scale"]["z"];
            transform.translate.x = entity["components"]["Transform"]["translate"]["x"];
            transform.translate.y = entity["components"]["Transform"]["translate"]["y"];
            transform.translate.z = entity["components"]["Transform"]["translate"]["z"];
            transform.rotate = entity["components"]["Transform"]["rotate"];

            ECoordinator.AddComponent(newEntity, transform);

            // Store the original scale of the entity for hove  r effects
            originalScales[newEntity] = Math3D::Vector3D{
                transform.scale.x,
                transform.scale.y,
                transform.scale.z
            };

        }

        if (entity["components"].contains("Lasercomp")) {
            // Check if "Laser" component exists within "components"
            LaserComponent lasercomp;

            // Extract Laser component from JSON
            auto& laserData = entity["components"]["Lasercomp"];

            // Extract values safely with defaults
            lasercomp.activeTime = laserData.value("activeTime", 3.0f);
            lasercomp.inactiveTime = laserData.value("inactiveTime", 2.0f);
            lasercomp.isActive = laserData.value("isActive", true);
            lasercomp.timer = lasercomp.activeTime; // Start with active time
            lasercomp.turnedOn = laserData.value("turnedOn", true);
            lasercomp.linkModuleID=laserData.value("linkModuleID", true);

            // Add laser component
            ECoordinator.AddComponent(newEntity, lasercomp);
        }

        //std::string textureFile = entity["components"]["textureFile"];

        if (entity["components"].contains("Button")) {
            ButtonComponent button;

            // Parse the "action" field from the JSON file
            if (entity["components"]["Button"].contains("action") && entity["components"]["Button"]["action"].is_string()) {
                std::string action = entity["components"]["Button"]["action"];
                button.action = action; // Store the action in the ButtonComponent
                button.isHover = false;
                //std::cout << "Parsed button action: " << action << std::endl; // Debug log
            }
            else {
                std::cerr << "Error: Missing or invalid 'action' field for button in JSON file." << std::endl;
            }

            // Add the ButtonComponent to the entity
            ECoordinator.AddComponent(newEntity, button);
        }

        // Type/Model check
        if (entity["components"].contains("type")) {
            std::string type = entity["components"]["type"];
            if (type == "texture") {
                if (entity["components"].contains("textureFile") && entity["components"]["textureFile"].is_string()) {
                    std::string texFile = entity["components"]["textureFile"];
                    Texture& tex = *TextureLibrary.GetAssets(TextureLibrary.GetName(texFile));
                    std::string textureFile = tex.GetFileName();
                    model = HUGraphics::texture_mesh(tex);
                    model.textureFile = textureFile;
                    model.shapeType = texture;
                }
            }
            else if (type == "animation_texture") {
                if (entity["components"].contains("textureFile") && entity["components"]["textureFile"].is_string()) {
                    std::string texFile = entity["components"]["textureFile"];
                    Texture& tex = *TextureLibrary.GetAssets(TextureLibrary.GetName(texFile));
                    std::string textureFile = tex.GetFileName();

                    if (texFile.empty() || texFile.find("./Assets/Textures") != 0) {
                        // std::cout<< "Invalid texture path: " << texFile << std::endl;
                    }
                    else {
                        // Extract animation parameters
                        int rows = 1, columns = 1, totalFrames = 1;
                        float frameTime = 0.1f;

                        if (entity["components"].contains("animations")) {
                            const auto& animations = entity["components"]["animations"];
                            if (animations.contains("Row") && animations["Row"].is_number_integer()) {
                                rows = animations["Row"];
                            }
                            if (animations.contains("Column") && animations["Column"].is_number_integer()) {
                                columns = animations["Column"];
                            }
                            if (animations.contains("TotalFrame") && animations["TotalFrame"].is_number_integer()) {
                                totalFrames = animations["TotalFrame"];
                            }
                            if (animations.contains("FrameTime") && animations["FrameTime"].is_number()) {
                                frameTime = animations["FrameTime"];
                            }
                        }
                        // Create the animated mesh
                        model = HUGraphics::animation_mesh(tex, rows, columns, frameTime, totalFrames);
                        model.textureFile = textureFile;
                    }
                }
                model.shapeType = texture_animation;  // Assign the shape type.
            }
            else if (type == "triangle") {
                model = HUGraphics::triangle_model();
                model.shapeType = triangle;
            }
            else if (type == "circle") {
                model = HUGraphics::circle_model();
                model.shapeType = circle;
            }
            else if (type == "rectangle") {

                model = HUGraphics::rectangle_model(model.color);
                model.shapeType = rectangle;

            }
            else if (type == "point") {
                //model = HUGraphics::points_model();
                model.shapeType = line;
            }
            else if (type == "text_texture") {
                model.shapeType = text_texture;
            }

            if (entity.contains("colorX") && !entity["colorX"].is_null()) {
                model.color = { entity["colorX"], entity["colorY"], entity["colorZ"] };
            }
            else {
                model.color = { 1.0f, 1.0f, 1.0f };
            }
        }




        // If the entity is a switch
        if (entity["components"].contains("Switch")) {
            PhysicsSystem::Switch switchComponent;
            switchComponent.isOn = entity["components"]["Switch"]["isOn"];

            // Link doors
            for (const auto& interactables : entity["components"]["Switch"]["interactables"]) {
                switchComponent.interactables.push_back(interactables);
            }

            ECoordinator.AddComponent(newEntity, switchComponent);
        }

        // If the entity is a laser
        if (entity["components"].contains("LaserComp")) {
            // Initialize the LaserComponent
            LaserComponent lasercomp;

            // Directly assign values from JSON
            lasercomp.activeTime = entity["components"]["LaserComp"]["activeTime"];
            lasercomp.inactiveTime = entity["components"]["LaserComp"]["inactiveTime"];
            lasercomp.isActive = entity["components"]["LaserComp"]["isActive"];
            lasercomp.timer = lasercomp.activeTime; // Start with active time
            lasercomp.turnedOn = entity["components"]["LaserComp"]["turnedOn"];
            lasercomp.linkModuleID = entity["components"]["LaserComp"]["linkModuleID"];


            // Add the LaserComponent to the entity
            ECoordinator.AddComponent(newEntity, lasercomp);
        }

        // If the entity is a door
        if (entity["components"].contains("AutoDoor")) {
            PhysicsSystem::AutoDoor autoDoorComponent;
            autoDoorComponent.switchName = entity["components"]["AutoDoor"]["switch"];
            autoDoorComponent.isOpen = entity["components"]["AutoDoor"]["isOpen"];

            ECoordinator.AddComponent(newEntity, autoDoorComponent);
        }

        // PhysicsBody/Model check
        if (entity["components"].contains("PhysicsBody")) {
            const auto& physicsBody = entity["components"]["PhysicsBody"];
            std::string category = "";
            // Extract properties
            if (entity["components"]["PhysicsBody"].contains("category")) {
                category = physicsBody["category"];

                //if item is a collectible
                if (category == "Object") {
                    ParticleComponent pc;
                    ECoordinator.AddComponent(newEntity, pc);
                }

            }
            
            const float mass = physicsBody["mass"];
            //const float inertiaMass = physicsBody["inertiaMass"];
            const auto& velocity = physicsBody["velocity"];
            const auto& acceleration = physicsBody["acceleration"];
            const auto& aabb = physicsBody["aabb"];
            const float friction = physicsBody["friction"];

            // Extract force type
            PhysicsSystem::ForceType forceType = PhysicsSystem::ForceType::None;
            if (physicsBody.contains("forceType")) {
                std::string forceTypeStr = physicsBody["forceType"];
                if (forceTypeStr == "Linear") forceType = PhysicsSystem::ForceType::Linear;
                else if (forceTypeStr == "Rotational") forceType = PhysicsSystem::ForceType::Rotational;
                else if (forceTypeStr == "Mixed") forceType = PhysicsSystem::ForceType::Mixed;
                else if (forceTypeStr == "Drag") forceType = PhysicsSystem::ForceType::Drag;
            }

            // Extract position and size
            float minX = aabb["minX"];
            float minY = aabb["minY"];
            float maxX = aabb["maxX"];
            float maxY = aabb["maxY"];

            //// Calculate size and position
            float width = maxX - minX;
            float height = maxY - minY;
            float centerX = minX + width / 2.0f;
            float centerY = minY + height / 2.0f;

            // Create PhysicsBody component
            PhysicsSystem::PhysicsBody body{
                category,
                    mass,
                    1.0f,
                { velocity["vx"], velocity["vy"] },          // velocity
                { acceleration["ax"], acceleration["ay"] },  // acceleration
                    0.0f,
                    0.0f,
                    0.0f,
                { centerX, centerY },
                { 0.0f, 0.0f },
                    PhysicsSystem::ForcesManager{},
                    AABB{ minX, minY, maxX, maxY },              // AABB
                    friction,
                    false,
                    false,
                    newEntity
            };

            ECoordinator.AddComponent(newEntity, body);
        }

        // RenderLayer check
        if (entity["components"].contains("RenderLayer") && !entity["components"]["RenderLayer"].is_null()) {
            RenderLayer layer;

            // Cast the integer from JSON to RenderLayerType
            layer.layer = static_cast<RenderLayerType>(entity["components"]["RenderLayer"].get<int>());

            ECoordinator.AddComponent(newEntity, layer);
        }

        //check for alpha
        if (entity["components"].contains("alpha") && !entity["components"]["alpha"].is_null()) {
            model.alpha = entity["components"]["alpha"];
        }

        // Name check
        if (entity.contains("name") && !entity["name"].is_null()) {
            Name name;
            name.name = entity["name"];


            if (entity["name"] == "Thief") {
                ECoordinator.setThiefID(newEntity);
            }

            else if (entity["name"] == "cutscene" && entity["components"].contains("seconds")) {
                int seconds = entity["components"]["seconds"];
                sceneVector.push_back(std::pair(newEntity,seconds));

            }

            else if (entity["name"] == "GetBackToVan") {
                getBackToVanImage = newEntity;
            }

            else if (entity["name"] == "Timer") {
                timerID = newEntity;
            }

            //this applies only to starRating stage
            else if (entity["name"] == "Win100" || entity["name"]=="Win101" || entity["name"]=="Win110" || entity["name"]=="Win111") {
                //newEntity is entityID
                std::string substr = std::string(entity["name"]).substr(3);

                if (std::stoi(substr) == winStatus) {
                    model.alpha = 1.0f;
                }
                
            }


           



            ECoordinator.AddComponent(newEntity, name);
        }

        

        ECoordinator.AddComponent(newEntity, model);
    }
}

void savegame(nlohmann::json& jsonComponents, nlohmann::json& jsonData) {
    jsonData.clear();

    std::vector<EntityID> entityIDs = ECoordinator.GetAllEntities();

    for (EntityID entityID : entityIDs) {
        //Entity level
        nlohmann::json jsonEntity;
        jsonEntity.clear();
        jsonComponents.clear();
        Signature sig = ECoordinator.GetEntitySignature(entityID);
        Transform transform = ECoordinator.GetComponent<Transform>(entityID);

        // Transform check
        if (sig.test(0)) {
            jsonComponents["Transform"]["scale"] = {
                {"x", transform.scale.x},
                {"y", transform.scale.y},
                {"z", transform.scale.z} };
            jsonComponents["Transform"]["rotate"] = transform.rotate;
            jsonComponents["Transform"]["translate"] = {
                {"x", transform.translate.x},
                {"y", transform.translate.y},
                {"z", transform.translate.z} };
        }

        // GLModel check
        if (sig.test(1)) {
            HUGraphics::GLModel model = ECoordinator.GetComponent<HUGraphics::GLModel>(entityID);
            std::string texturePath = model.textureFile;
            std::string normalizedTextureFile = normalizePath(texturePath);  // Normalize the path
            glm::vec3 color = model.color;


            // Check if the texture file path is valid
            if (!model.textureFile.empty() && !std::string(model.textureFile).empty() && std::ifstream(model.textureFile).good()) {

                jsonComponents["textureFile"] = normalizedTextureFile;

                if (model.shapeType == texture_animation) { // Check for animation type
                    jsonComponents["type"] = "animation_texture";
                    jsonComponents["animations"] = {
                        {"Row", model.rows},
                        {"Column", model.columns},
                        {"TotalFrame", model.totalframe},
                        {"FrameTime", model.frametime} };
                }
                else {
                    jsonComponents["type"] = "texture";
                }
            }
            else if (model.shapeType == text_texture) {
                jsonComponents["type"] = "text_texture";

                // Save additional text-specific data
                jsonComponents["text"] = model.text;            // Save the actual text
                jsonComponents["fontname"] = model.fontName;    // Save the font name


                jsonComponents["fontSize"] = model.fontSize;    // Save the font size
                jsonComponents["scale"] = model.fontScale;
                jsonComponents["color"] = {
                    {"r", model.color.r},
                    {"g", model.color.g},
                    {"b", model.color.b}
                };  // Save the text color as RGB values

            }
            else {

                // If no texture, fall back to checking shape type
                switch (model.shapeType) {
                case triangle:
                    jsonComponents["type"] = "triangle";
                    break;
                case circle:
                    jsonComponents["type"] = "circle";
                    break;
                case rectangle:
                    jsonComponents["type"] = "rectangle";
                    break;
                case line:
                    jsonComponents["type"] = "line";
                    break;
                case point:
                    jsonComponents["type"] = "point";
                    break;
                default:
                    jsonComponents["type"] = "unknown";
                }
                jsonComponents["colorX"] = color.x;
                jsonComponents["colorY"] = color.y;
                jsonComponents["colorZ"] = color.z;
            }
        }

        // PhysicsBody check
        if (sig.test(2)) {
            const PhysicsSystem::PhysicsBody& body = ECoordinator.GetComponent<PhysicsSystem::PhysicsBody>(entityID);
            jsonComponents["PhysicsBody"] = {
                {"category", body.category},
                {"acceleration", {{"ax", body.acceleration.GetX() }, {"ay", body.acceleration.GetY()}}},
                {"velocity", {{"vx", body.velocity.GetX()}, {"vy", body.velocity.GetY()}}},
                {"aabb", {{"minX", body.aabb.minX}, {"minY", body.aabb.minY}, {"maxX", body.aabb.maxX}, {"maxY", body.aabb.maxY}} },
                {"mass", body.mass},
                {"friction", body.friction},
            };
        }

        // RenderLayer check
        if (sig.test(3)) {
            const RenderLayer& renderlayer = ECoordinator.GetComponent<RenderLayer>(entityID);
            jsonComponents["RenderLayer"] = renderlayer.layer;
        }

        // Name check
        if (sig.test(4)) {
            Name entityName = ECoordinator.GetComponent<Name>(entityID);
            jsonEntity["name"] = entityName.name.c_str();
        }

        // Assuming Switch component is at index 5 in the entity signature
        if (sig.test(5)) {
            const PhysicsSystem::Switch& switchComponent = ECoordinator.GetComponent<PhysicsSystem::Switch>(entityID);
            jsonComponents["Switch"] = {
                {"isOn", switchComponent.isOn},
                {"interactables", switchComponent.interactables}
            };
        }


        if (sig.test(7)) {
            const LaserComponent& laserComp = ECoordinator.GetComponent<LaserComponent>(entityID);
            jsonComponents["LaserComp"] = {
                {"activeTime", laserComp.activeTime},
                {"inactiveTime", laserComp.inactiveTime},
                {"isActive", laserComp.isActive},
                {"timer", laserComp.timer},
                {"turnedOn", laserComp.turnedOn},
                {"linkModuleID", laserComp.linkModuleID}
            };
        }

        jsonEntity["components"] = jsonComponents;
        jsonData["entities"].push_back(jsonEntity);
    }

}

void LoadGameObjectsFromJson(const std::string& filename) {

    //ECoordinator.ClearAllEntities();

    std::ifstream file(filename);
    if (!file) {
        // std::cout<< "Unable to open file";
    }


    json j;
    file >> j;

    initialGameFilePath = filename;
    loadgame(j);
}


std::string normalizePath(const std::string& path) {
    std::string normalizedPath = path;  // Create a copy of the path

    // Replace all backslashes with forward slashes
    std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');

    return normalizedPath;
}

void SaveCategoriesToJson(const std::string& filename) {
    nlohmann::json jsonCategories;

    // Add each category to the JSON object
    for (const auto& category : categories) {
        jsonCategories["categories"].push_back(category);
    }

    std::string filePath = "Json/" + filename;
    std::ofstream outFile(filePath, std::ios::trunc);
    if (outFile.is_open()) {
        outFile << jsonCategories.dump(4);
        outFile.close();
    }
    else {
        std::cerr << "Error opening sssfile for writing: " << filename << std::endl;
    }
}


void SaveGameObjectsToJson(const std::string& filename) {
    // Components level
    nlohmann::json jsonComponents;
    // OG data (to be deleted)
    nlohmann::json jsonData;

    savegame(jsonComponents, jsonData);
   
    std::string filePath = "Json/" + filename;
    // Write JSON to file
    std::ofstream outFile(filePath, std::ios::trunc);
    if (outFile.is_open()) {
        outFile << jsonData.dump(4);
        outFile.close();
    }
    else {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
    }
}




// Function to get the Documents folder path
std::string GetDocumentsFolder() {
    char path[MAX_PATH];
    // SHGetSpecialFolderPathA retrieves the Documents folder path
    if (SHGetSpecialFolderPathA(NULL, path, CSIDL_PERSONAL, FALSE)) {
        return std::string(path);
    }
    else {
        std::cerr << "Failed to find Documents folder!" << std::endl;
        return "";
    }
}

// Function to save game objects to JSON file
void SaveGameObjectsToJson_doc(const std::string& filename) {
    std::string documentsFolder = GetDocumentsFolder();
    if (documentsFolder.empty()) {
        std::cerr << "Error: Failed to retrieve Documents folder!" << std::endl;
        return;
    }

    fs::path gameDirectory = fs::path(documentsFolder) / "MyGame";
    if (!fs::exists(gameDirectory)) {
        if (!fs::create_directory(gameDirectory)) {
            std::cerr << "Error: Failed to create directory: " << gameDirectory << std::endl;
            return;
        }
    }

    // Debug directory check
    if (!fs::exists(gameDirectory)) {
        std::cerr << "Error: Game directory still does not exist: " << gameDirectory << std::endl;
        return;
    }

    // Save JSON
    nlohmann::json jsonComponents;
    nlohmann::json jsonData;
    savegame(jsonComponents, jsonData);

    // Debug JSON content
    if (jsonData.empty()) {
        std::cerr << "Error: JSON data is empty, nothing to save!" << std::endl;
        return;
    }

    // Full file path
    fs::path filePath = gameDirectory / filename;

    // Attempt to write file
    std::ofstream outFile(filePath, std::ios::trunc);
    if (!outFile) {
        std::cerr << "Error: Unable to open file for writing: " << filePath.string() << std::endl;
        return;
    }

    outFile << jsonData.dump(4);
    outFile.close();
}


void LoadGameObjectsFromJson_doc(const std::string& filename) {
    std::string documentsFolder = GetDocumentsFolder();


    fs::path gameDirectory = fs::path(documentsFolder) / "MyGame";
    fs::path filePath = gameDirectory / filename;

    
    std::ifstream file(filePath.string());



    json j;
    file >> j;

    initialGameFilePath = filename;
    loadgame(j);
}





void LoadAnimationPresets(const std::string& filePath) {
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open JSON file!" << std::endl;
        return;
    }

    nlohmann::json jsonData;
    inputFile >> jsonData;

    // Iterate over each item and populate the animationPresets map
    for (auto& item : jsonData.items()) {
        const std::string& name = item.key();
        const nlohmann::json& data = item.value();

        AnimationData animData;
        animData.frametime = data["frameTime"];
        animData.rows = data["rows"];
        animData.columns = data["columns"];
        animData.totalFrames = data["totalFrames"];

        animationPresets[name] = animData;

    }

    inputFile.close();
}

void SaveAnimationPresetsToJSON(const std::string& filePath) {
    nlohmann::json jsonData;

    // Try to read existing JSON data if the file exists
    std::ifstream inFile(filePath);
    if (inFile.is_open()) {
        try {
            inFile >> jsonData;
        }
        catch (const nlohmann::json::parse_error& e) {
            std::cerr << "Error parsing JSON file: " << e.what() << std::endl;
            // Continue with an empty jsonData if parsing fails
        }
        inFile.close();
    }

    // Update JSON data with current animation presets
    for (const auto& [name, animData] : animationPresets) {
        jsonData[name] = {
            {"frameTime", animData.frametime},
            {"rows", animData.rows},
            {"columns", animData.columns},
            {"totalFrames", animData.totalFrames}
        };
    }

    // Write updated JSON data back to the file
    std::ofstream outFile(filePath, std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open the file for writing: " << filePath << std::endl;
        return;
    }

    try {
        outFile << std::setw(4) << jsonData;  // Save with indentation for readability
    }
    catch (const std::ios_base::failure& e) {
        std::cerr << "Error writing to file: " << e.what() << std::endl;
    }

    outFile.close();
}

//after deleting texture entity
// Function to update JSON after asset deletion
void UpdateJSONFilesAfterDeletion(const std::string& deletedAssetPath) {
    // Path to the directory containing JSON files
    fs::path jsonDir = "./Json";  // Directory where the JSON files are stored

    // Check if the directory exists
    if (!fs::exists(jsonDir) || !fs::is_directory(jsonDir)) {
        std::cerr << "Error: Directory does not exist or is not a directory." << std::endl;
        return;
    }

    // Normalize path separators to forward slashes
    std::string normalizedDeletedAssetPath = deletedAssetPath;
    std::replace(normalizedDeletedAssetPath.begin(), normalizedDeletedAssetPath.end(), '\\', '/');

    // Iterate through all JSON files in the directory
    for (const auto& entry : fs::directory_iterator(jsonDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            fs::path jsonFilePath = entry.path();

            // Open the JSON file
            std::ifstream file(jsonFilePath);
            if (!file.is_open()) {
                std::cerr << "Failed to open " << jsonFilePath << std::endl;
                continue;
            }

            json jsonData;
            try {
                file >> jsonData;  // Read JSON content
            }
            catch (const json::parse_error& e) {
                std::cerr << "Error parsing " << jsonFilePath << ": " << e.what() << std::endl;
                continue;
            }
            file.close();  // Close file after reading

            bool modified = false;

            // Check if the JSON file contains the deleted asset path
            for (auto it = jsonData["entities"].begin(); it != jsonData["entities"].end(); /* increment inside loop */) {
                if (it->contains("components") && it->at("components").contains("textureFile")) {
                    std::string textureFilePath = it->at("components").at("textureFile").get<std::string>();

                    // Normalize path separators to forward slashes
                    std::replace(textureFilePath.begin(), textureFilePath.end(), '\\', '/');

                    // Compare with the deleted asset
                    if (textureFilePath == normalizedDeletedAssetPath) {
                        it = jsonData["entities"].erase(it);  // Remove the entire entity
                        modified = true;
                    }
                    else {
                        ++it;  // Only increment if not erased
                    }
                }
                else {
                    ++it;  // Increment if no textureFile
                }
            }

            // Save changes if modifications were made
            if (modified) {
                std::ofstream outFile(jsonFilePath, std::ios::trunc);
                if (!outFile.is_open()) {
                    std::cerr << "Failed to open " << jsonFilePath << " for writing" << std::endl;
                    continue;
                }
                outFile << std::setw(4) << jsonData;  // Save formatted JSON
                outFile.close();
            }
        }
    }
}



// Function to delete the asset and update references in Main_Menu.json
void DeleteAssetAndUpdateReferences(const std::string& assetName) {
    // Full path to the asset file
    std::filesystem::path assetFilePath = std::filesystem::path("./Assets/Textures") / assetName;

    if (fs::exists(assetFilePath)) {
        try {
            fs::remove(assetFilePath);  // Delete the asset
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error deleting asset: " << e.what() << std::endl;
            return;
        }
    }
    else {
    }

    // Now update only Main_Menu.json after deleting the asset
    UpdateJSONFilesAfterDeletion(assetFilePath.string());
}


