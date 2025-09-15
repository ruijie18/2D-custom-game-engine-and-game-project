/**
 * @file EntityManager.h
 * @brief Entity management definitions including `GameObjectManager` for handling ECS entity lifecycle.
 *
 * This file declares the `GameObjectManager` class, which is responsible for the creation, destruction,
 * and tracking of entities within an ECS (Entity Component System) framework. It defines types such as
 * `EntityID`, `ComponentType`, and `Signature`, and provides mechanisms for managing up to
 * `MAX_GAME_OBJECTS` entities.
 *
 * Key Responsibilities:
 * - **Entity Creation and Destruction**:
 *   - Reuse of entity IDs via a queue to avoid ID exhaustion.
 *   - Resetting signatures and maintaining accurate active entity count.
 *
 * - **Signature Management**:
 *   - Tracks each entity's component signature using bitsets.
 *   - Allows external systems to set or query entity signatures.
 *
 * - **Active Entity Tracking**:
 *   - Maintains a list of currently active entities.
 *   - Provides utility methods to retrieve or clear all active entities.
 *
 * - **Entity Naming Support**:
 *   - Maps entity IDs to names for identification and debugging (placeholder support shown).
 *
 * Types Defined:
 * - `EntityID`: Unsigned 32-bit integer used as a unique ID for game objects.
 * - `ComponentType`: Unsigned 8-bit integer representing a component type.
 * - `Signature`: Bitset indicating which components an entity has.
 *
 * Author: Che Ee (80%)
 * Co-Author: Rui Jie (20%)
 */



// Add this at the top of your EntityManager.h
#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H


#include "CommonIncludes.h"
#include "ListOfComponents.h"

using EntityID = std::uint32_t;

const EntityID MAX_GAME_OBJECTS = 5000;

using ComponentType = std::uint8_t;

const ComponentType MAX_COMPONENT_TYPES = 32;

using Signature = std::bitset<MAX_COMPONENT_TYPES>;

class GameObjectManager {



private:
	
	// Queue of unused game object IDs
	std::queue<EntityID> mAvailableGameObjectIDs{};

	// Array of component signatures where the index corresponds to the game object ID
	std::array<Signature, MAX_GAME_OBJECTS> mSignatures{};

	// Total active game objects - used to keep limits on how many exist
	uint32_t mActiveGameObjectCount{};

	// New vector to store active entities
	std::vector<EntityID> mActiveEntities;

	// Store entity names
	std::unordered_map<EntityID, std::string> mEntityNames;

public:

	void DestroyAllUIGameObjects();

	void DestroyAllGameObjects() {
		// Refill the queue with available game object IDs
		while (!mAvailableGameObjectIDs.empty()) {
			mAvailableGameObjectIDs.pop();  // Empty the queue first
		}

		// Refill the queue with all possible EntityIDs
		for (EntityID id = 0; id < MAX_GAME_OBJECTS; ++id) {
			mAvailableGameObjectIDs.push(id);
		}

		// Reset all component signatures
		for (auto& signature : mSignatures) {
			signature.reset();
		}

		// Reset the active game object count
		mActiveGameObjectCount = 0;
		mActiveEntities.clear(); // Clear the active entities
	}


	void FadeOutAllObjects();

	void FadeInAllObjects();


	GameObjectManager() {

		//initialize the Entity IDS
		for (EntityID entity = 0; entity < MAX_GAME_OBJECTS; ++entity) {
			mAvailableGameObjectIDs.push(entity);
		}
	}

	EntityID CreateGameObject() {
		assert(mActiveGameObjectCount < MAX_GAME_OBJECTS && "Too many game objects in existence.");

		// Take an ID from the front of the queue
		EntityID id = mAvailableGameObjectIDs.front();
		mAvailableGameObjectIDs.pop();
		++mActiveGameObjectCount;

		mActiveEntities.push_back(id); // Add to active entities
		return id;
	}

	void DestroyGameObject(EntityID gameObjectID)
	{
		assert(gameObjectID < MAX_GAME_OBJECTS && "Game object ID out of range.");

		// Invalidate the destroyed game object's component signature
		mSignatures[gameObjectID].reset();

		// Put the destroyed ID at the back of the queue
		mAvailableGameObjectIDs.push(gameObjectID);
		--mActiveGameObjectCount;

		// Remove the entity from active entities
		auto it = std::remove(mActiveEntities.begin(), mActiveEntities.end(), gameObjectID);
		mActiveEntities.erase(it, mActiveEntities.end()); // Erase removes the entity
	}

	void SetComponentSignature(EntityID gameObjectID, Signature signature)
	{
		assert(gameObjectID < MAX_GAME_OBJECTS && "Game object ID out of range.");


		mSignatures[gameObjectID] = signature;
	}

	Signature GetComponentSignature(EntityID gameObjectID)
	{
		assert(gameObjectID < MAX_GAME_OBJECTS && "Game object ID out of range.");

		// Get this game object's component signature from the array
		return mSignatures[gameObjectID];
	}

	Signature GetSignature(EntityID entityID) const {
		assert(entityID < MAX_GAME_OBJECTS && "Game object ID out of range.");
		return mSignatures[entityID];
	}

	uint32_t GetActiveEntityCount() const {
		return mActiveGameObjectCount;
	}

	std::vector<EntityID> GetAllEntities() const {
		return mActiveEntities; // Return the list of active entities
	}

	void PrintAllEntitiesWithComponents() {

	}

	//	// Check and print Velocity component
	//	try {
	//		Velocity& vel = CM.GetComponent<Velocity>(entity);
	//		// std::cout<< "Velocity: (" << vel.vx << ", " << vel.vy << ")\n";
	//	}
	//	catch (const std::exception& e) {
	//		// std::cout<< "No Velocity component\n";
	//	}

	//	// Check and print Direction component
	//	try {
	//		Direction& dir = CM.GetComponent<Direction>(entity);
	//		// std::cout<< "Direction: " << dir.angle << " degrees\n";
	//	}
	//	catch (const std::exception& e) {
	//		// std::cout<< "No Direction component\n";
	//	}

	//	// std::cout<< "-------------------\n"; // Separator for readability
	//}


};

#endif // ENTITY_MANAGER_H