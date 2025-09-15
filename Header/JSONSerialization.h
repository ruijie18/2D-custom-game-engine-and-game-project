/**
 * @file JSONSerialization.h
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
#pragma once
#ifndef  JSON_SERIALIZER_H
#define JSON_SERIALIZER_H


#include "GlobalVariables.h"
#include "CommonIncludes.h"
#include "EntityManager.h"
#include "ListOfComponents.h"

#include "Coordinator.h"
using json = nlohmann::json;
extern std::unordered_map<unsigned int, Math3D::Vector3D> originalScales;
void LoadGameObjectsFromJson(const std::string& filename);
std::string normalizePath(const std::string& path);
void SaveGameObjectsToJson(const std::string& filename);
void SaveCategoriesToJson(const std::string& filename);
void SaveGameObjectsToJson_doc(const std::string& filename);
std::string GetDocumentsFolder();
void LoadGameObjectsFromJson_doc(const std::string& filename);
void LoadAnimationPresets(const std::string& filename);
void SaveAnimationPresetsToJSON(const std::string& filePath);
void UpdateJSONFilesAfterDeletion(const std::string& deletedAssetName);
void DeleteAssetAndUpdateReferences(const std::string& assetName);
#endif // ! JSON_SERIALIZER_H
