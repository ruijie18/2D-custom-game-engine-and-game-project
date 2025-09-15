/**
 * @file SystemsManager.cpp
 * @brief Implementation of the `SystemManager` class for managing systems and entities in the ECS framework.
 *
 * This file provides the implementation for system management, including functionality to destroy specific
 * entities (e.g., UI entities) and to handle changes in entity signatures that determine system associations.
 *
 * Key Features:
 * - **System and Entity Management**:
 *   - `DestroyAllUIEntities`: Iterates through all registered systems and removes UI-specific entities
 *     (those with `RenderLayerType::UI` and name `MenuUI`).
 *   - `EntitySignatureChanged`: Updates the association of entities with systems based on their updated signatures.
 * - **Dynamic Entity Updates**:
 *   - Ensures entities are added to or removed from systems as their signatures change, maintaining correct
 *     system associations dynamically.
 *
 * Functions:
 * - `DestroyAllUIEntities`:
 *   - Iterates through all registered systems and removes entities with `RenderLayerType::UI` and a matching name.
 * - `EntitySignatureChanged`:
 *   - Handles updates to an entity's signature.
 *   - Adds the entity to a system if its signature matches the system's signature or removes it otherwise.
 *
 * Key Concepts:
 * - **Entity Signatures**:
 *   - A bitset that represents the components an entity possesses.
 *   - Used to determine which systems an entity belongs to.
 * - **Systems Management**:
 *   - Each system has a specific signature that defines the set of components required for entities in that system.
 *
 * Author: Che Ee (100%)
 */

#include "CommonIncludes.h"
#include "SystemsManager.h"
#include "Render.h"
#include <random>
#include <bitset>
void SystemManager::DestroyAllUIEntities()
{
    for (auto& pair : mRegisteredSystems) {
        auto& system = pair.second;

        // Iterate through the entities in the system and remove UI entities
        for (auto it = system->mEntities.begin(); it != system->mEntities.end(); ) {
            EntityID entityID = *it;

            // Check if the entity has a RenderLayer component
            if (ECoordinator.HasComponent<RenderLayer>(entityID)) {
                auto renderLayer = ECoordinator.GetComponent<RenderLayer>(entityID);

                // Check if the RenderLayer is of type UI and has a specific name
                if (renderLayer.layer == RenderLayerType::UI &&
                    ECoordinator.HasComponent<Name>(entityID) &&
                    ECoordinator.GetComponent<Name>(entityID).name == "MenuUI") {

                    // Erase the entity and update iterator
                    it = system->mEntities.erase(it);
                    continue; // Skip incrementing the iterator as erase updates it
                }
            }
            ++it; // Increment iterator if no entity was erased
        }
    }
}


void SystemManager::EntitySignatureChanged(EntityID entity, Signature entitySignature)
{

	for (auto const& pair : mRegisteredSystems)
	{
		auto const& type = pair.first;
		//auto const& system = pair.second;
		auto const& systemSignature = mSystemSignatures[type];

		if ((entitySignature & systemSignature) == systemSignature)
		{
			if ((entitySignature & systemSignature) == systemSignature) {
				if (mRegisteredSystems[type]->mEntities.find(entity) == mRegisteredSystems[type]->mEntities.end()) {
					mRegisteredSystems[type]->mEntities.insert(entity);
				}
			}
			else {
				// Otherwise, remove it if it was previously added
				mRegisteredSystems[type]->mEntities.erase(entity);
			}
		}
	}
}