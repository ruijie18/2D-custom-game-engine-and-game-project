/**
 * @file Coordinator.cpp
 * @brief Implementation of entity management and utility functions for the ECS (Entity-Component-System) framework.
 *
 * This file provides implementations for managing entities and components in the ECS framework, including
 * functionality for creating, cloning, formatting, and clearing entities. It also includes utility functions
 * for rendering-related tasks such as creating text and texture entities.
 *
 * Key Features:
 * - **Entity Management**:
 *   - `CloneEntityWithNewPosition`: Clones an existing entity and places it at a new position.
 *   - `CreateNewTextureEntity`: Creates a new entity with a texture component and sets its position and size.
 *   - `CreateTextEntity`: Creates a text entity with customizable attributes such as font, size, and color.
 *   - `ClearAllEntities`: Removes all entities from the ECS system.
 * - **Name Formatting**:
 *   - `FormatEntityName`: Formats an entity's name by removing extensions, capitalizing the first letter,
 *     replacing underscores with spaces, and ensuring uniqueness by appending/incrementing numeric suffixes.
 * - **Component Integration**:
 *   - Manages components like `Transform`, `HUGraphics::GLModel`, `Name`, and `RenderLayer`.
 * - **Utility and Debugging**:
 *   - Supports flexible entity creation for various use cases, including UI and game objects.
 *   - Debugging outputs are included for better traceability during development.
 *
 * Key Classes and Methods:
 * - **ECSCoordinator**:
 *   - Centralized class for handling entity creation and management in the ECS framework.
 *   - Integrates closely with `HUGraphics` and `FontSystem` for rendering-related tasks.
 *   - Uses `existingEntityNames` to ensure entity name uniqueness.
 *
 * Author: Che Ee (100%)
 */


#include "Coordinator.h"
#include "GlobalVariables.h"
#include "Graphics.h"
#include "ButtonComponent.h"

std::unordered_set<std::string> ECSCoordinator::existingEntityNames;


void ECSCoordinator::CloneEntityWithNewPosition(EntityID originalEntity, float newX, float newY)
{
    //ComponentType transform_bit = mComponentManager->GetComponentType<Transform>();

    EntityID clonedEntity;

    Signature originalSignature = mGameObjectManager->GetComponentSignature(originalEntity);

    auto& model_ref = ECoordinator.GetComponent<HUGraphics::GLModel>(originalEntity);

    auto& transform_ref = ECoordinator.GetComponent<Transform>(originalEntity);

    clonedEntity = mGameObjectManager->CreateGameObject();
    HUGraphics::GLModel mdl;

    Transform transform;
    transform.scale = transform_ref.scale;

    transform.rotate = transform_ref.rotate;
    transform.translate = glm::vec3(newX, newY, 1.0f);
    AddComponent(clonedEntity, transform);

    //get type of object

    if (model_ref.shapeType == circle) {
        mdl = HUGraphics::circle_model();
    }

    if (model_ref.shapeType == rectangle) {
        mdl = HUGraphics::rectangle_model();
    }

    if (model_ref.shapeType == texture) {
        //mdl = HUGraphics::texture_mesh(model_ref.textureFile.c_str());
    }

    if (model_ref.shapeType == triangle) {
        mdl = HUGraphics::triangle_model();
    }


    mdl.color = model_ref.color;

    mdl.shapeType = model_ref.shapeType;

    AddComponent(clonedEntity, mdl);
}

void ECSCoordinator::CreateNewTextureEntity(Texture& tex, float posX, float posY) {
    // Obtain references to the components of the original entity
    Transform transform;
    EntityID newEntity = ECoordinator.CreateGameObject(); // Create a new entity
    HUGraphics::GLModel model;

    float sizeX = static_cast<float>(tex.GetImageWidth());
    float sizeY = static_cast<float>(tex.GetImageHeight());

    transform.scale = { sizeX, sizeY, 1 };
    transform.rotate = { 0 };
    transform.translate = { posX, posY, 1 };
    ECoordinator.AddComponent(newEntity, transform);

    model.shapeType = texture_animation;
 
    model = HUGraphics::animation_mesh(tex,1,1,0,1);
    model.textureFile = tex.GetFileName();
    model.color = { 1.0f, 1.0f, 1.0f };
    AddComponent(newEntity, model);

    Name entityName;
    entityName.name = tex.GetFileName();
    AddComponent(newEntity, entityName);

    RenderLayer layer;
    layer.layer = RenderLayerType::GameObject;
    AddComponent(newEntity, layer);

    
}

void ECSCoordinator::CreateTextEntity(const std::string& text, float scale, glm::vec3 color, float posX, float posY,float width,float height, std::string fontname, int size, std::string entityName="") {

    GLuint textTexture = fontSystem->RenderTextToTexture(text, scale, color, fontname, size);
    if (textTexture == 0) {
        std::cerr << "Failed to create text texture for: \"" << text << "\"" << std::endl;
    }


    // Step 4: Create the entity and add components
    EntityID newEntity = CreateGameObject();  // Generate a new entity

    // Transform component
    Transform transform;
    transform.scale = { width, height, 1.0f };
    transform.rotate = { 0.0f };
    transform.translate = {
        posX, 
        posY, 
        0.0f
    };
    AddComponent(newEntity, transform);

    // Model component (to store rendering details)
    HUGraphics::GLModel model;
    model = HUGraphics::text_mesh(textTexture); 
    model.text = text;
    model.fontName = fontname;
    model.fontSize = size;
    model.shapeType = text_texture;  // Set shape type to texture
    model.textureID = textTexture;  // Use the generated text texture
    model.color = color; 
    model.fontScale=scale;
    AddComponent(newEntity, model);

    //Check if the entity already has a Name component
    if (!HasComponent<Name>(newEntity)) { 
        Name nameComponent;
        if (!entityName.empty()) {
            //Use the provided name if it's not empty
            nameComponent.name = entityName;
        }
        else {
            // Otherwise, generate a default name
            nameComponent.name = "TextObject_" + std::to_string(newEntity);
        }
        AddComponent(newEntity, nameComponent);
    }


    // Render layer component for rendering order control
    RenderLayer layer;
    layer.layer = RenderLayerType::UI;  // Place text in the appropriate rendering layer
    AddComponent(newEntity, layer);

    // Debugging output
    // std::cout<< "Text entity created: \"" << text << "\" with ID " << newEntity << " at position (" << posX << ", " << posY << ")." << std::endl;
}


std::string ECSCoordinator::FormatEntityName(const std::string& filename) {
    // Remove the extension
    size_t lastDot = filename.find_last_of('.');
    std::string baseName = (lastDot == std::string::npos) ? filename : filename.substr(0, lastDot);

    // Capitalize the first letter
    if (!baseName.empty()) {
        baseName[0] = static_cast<char>(std::toupper(baseName[0]));
    }

    // Replace underscores with spaces
    for (auto& ch : baseName) {
        if (ch == '_') {
            ch = ' ';
        }
    }

    // Check if the base name ends with a number
    size_t len = baseName.length();
    size_t spacePos = baseName.find_last_of(' ');

    if (spacePos != std::string::npos && spacePos < len - 1) {
        // Check if the part after the last space is a number
        std::string numberPart = baseName.substr(spacePos + 1);
        bool isNumber = true;
        for (char c : numberPart) {
            if (!std::isdigit(c)) {
                isNumber = false;
                break;
            }
        }

        if (isNumber) {
            // If it's a number, increment it
            int number = std::stoi(numberPart);
            baseName = baseName.substr(0, spacePos); // Remove the old number part

            // Generate a new name with incremented number
            std::string uniqueName;
            int suffix = number + 1;
            do {
                uniqueName = baseName + " " + std::to_string(suffix++);
            } while (existingEntityNames.find(uniqueName) != existingEntityNames.end());

            existingEntityNames.insert(uniqueName);
            return uniqueName;
        }
    }

    // If no number suffix is found, add " 1"
    std::string uniqueName = baseName + " 1";
    while (existingEntityNames.find(uniqueName) != existingEntityNames.end()) {
        // Keep incrementing until we find a unique name
        uniqueName = baseName + " " + std::to_string(std::stoi(uniqueName.substr(baseName.size() + 1)) + 1);
    }

    // Add the unique name to the set
    existingEntityNames.insert(uniqueName);
    return uniqueName;
}

void ECSCoordinator::ClearAllEntities() {
    auto entityIDs = ECoordinator.GetAllEntities();
    ECoordinator.DestroyAllGameObjects();
    
    entityIDs = ECoordinator.GetAllEntities();
}

