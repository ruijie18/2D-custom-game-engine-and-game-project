/**
 * @file EntityManager.cpp
 * @brief Implementation of the `GameObjectManager` class for managing UI-related game objects in the ECS framework.
 *
 * This file provides an implementation for removing UI-specific game objects in the ECS framework using the
 * `DestroyAllUIGameObjects` method. The function is designed to iterate through all active entities, identify
 * entities with a `RenderLayer` component categorized as `MainMenuUI`, and remove them from the system.
 *
 * Key Features:
 * - **UI Entity Management**:
 *   - Provides functionality to destroy all UI-related entities based on their `RenderLayer` type.
 * - **Component Integration**:
 *   - Leverages the `RenderLayer` component to determine the type of entities being processed.
 * - **Iterative Entity Destruction**:
 *   - Iterates through active entities to selectively destroy UI-specific game objects.
 *
 * Key Method:
 * - `DestroyAllUIGameObjects`:
 *   - Iterates through all active entities in the ECS framework and removes those categorized under `RenderLayerType::MainMenuUI`.
 *   - Ensures clean removal of UI entities while leaving non-UI entities unaffected.
 *
 * Author: Che Ee (100%)
 */



#include "EntityManager.h"
#include <Component.h>
#include <Coordinator.h>
#include "GlobalVariables.h"
#include <HelperFunctions.h>
void GameObjectManager::DestroyAllUIGameObjects()
{
    //std::cout << mActiveEntities.size();

    std::vector<EntityID> entitiesToRemove;

    // Collect entities to remove
    for (const auto& entityID : mActiveEntities) {
        if (ECoordinator.HasComponent<RenderLayer>(entityID) && ECoordinator.HasComponent<Name>(entityID)) {
            auto renderLayer = ECoordinator.GetComponent<RenderLayer>(entityID);
            auto name = ECoordinator.GetComponent<Name>(entityID);
            if (renderLayer.layer == RenderLayerType::UI && name.name == "MenuUI") {
                entitiesToRemove.push_back(entityID);
            }
        }
    }

    // Remove entities in a second pass
    for (const auto& entityID : entitiesToRemove) {
        DestroyGameObject(entityID);
        mActiveEntities.erase(std::remove(mActiveEntities.begin(), mActiveEntities.end(), entityID), mActiveEntities.end());
    }
}


void GameObjectManager::FadeOutAllObjects() {
    for (const auto& entityID : mActiveEntities) {
        FadeOutObject(entityID);

    }
}


void GameObjectManager::FadeInAllObjects() {
    for (const auto& entityID : mActiveEntities) {
        FadeInObject(entityID);

    }
}







