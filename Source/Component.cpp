/**
 * @file ComponentManager.cpp
 * @brief Implementation of the `ComponentManager` method to destroy UI entities.
 *
 * This file implements the `DestroyAllUIEntities` method of the `ComponentManager` class, which iterates
 * through all component storages and removes entities associated with the UI render layer.
 *
 * Key Features:
 * - **Selective Component Removal**:
 *   - Checks for components of type `RenderLayer` and identifies those belonging to the `UI` layer.
 *   - Clears all components associated with entities that match the `RenderLayerType::UI`.
 * - **Dynamic Type Casting**:
 *   - Uses `std::dynamic_pointer_cast` to identify specific component types dynamically.
 *
 * Functions:
 * - `DestroyAllUIEntities`:
 *   - Iterates through all registered component storages.
 *   - Identifies and clears components associated with the `RenderLayerType::UI` layer.
 *
 * Integration:
 * - Relies on dynamic type casting to identify components.
 * - Operates within the ECS framework to manage components dynamically.
 *
 * Author: Che Ee (100%)
 */


#include "Component.h"
#include "ListOfComponents.h"
#include "GlobalVariables.h"
void ComponentManager::DestroyAllUIEntities()
{
    // Check each shared_ptr in the map
    for (const auto& [key, component] : mComponentStorages) {
        // Attempt a dynamic_pointer_cast to RenderLayer

        //Delete RenderLayer UI Component...
        auto renderLayer = std::dynamic_pointer_cast<RenderLayer>(component);
        if (renderLayer) {
            //MenuUI

            auto name = std::dynamic_pointer_cast<Name>(component);
            if ( (renderLayer->layer == RenderLayerType::UI) && (name->name=="MenuUI")) {
                component->Clear();
            }

        }

    }
	
}
