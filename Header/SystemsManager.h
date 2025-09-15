/**
 * @file SystemsManager.h
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


#pragma once
#ifndef  SYSTEMS_MANAGER_H
#define SYSTEMS_MANAGER_H


#include "CommonIncludes.h"
#include "EntityManager.h"
class System
{
public:
	std::set<EntityID> mEntities;

	virtual void Update(double deltaTime) = 0;
	virtual const char* getName() const = 0; //for debug purpose
	virtual void Init() = 0;
	virtual ~System() = default;
};

class SystemManager
{
private:

	std::unordered_map<const char*, Signature> mSystemSignatures{};

	std::unordered_map<const char*, std::shared_ptr<System>> mRegisteredSystems{};

public:


	void DestroyAllUIEntities();

	void DestroyAllEntities() {

		for (auto& pair : mRegisteredSystems) {
			pair.second->mEntities.clear();
		}

	}


	void Init() {
		for (auto& pair : mRegisteredSystems) {
			pair.second->Init();
		}
	}

	void Update(double deltaTime) {

		//// std::cout<< mRegisteredSystems.size()<<"\n";
		for (auto& pair : mRegisteredSystems) {

			pair.second->Update(deltaTime);
		}
	}

	template<typename T>
	std::shared_ptr<T> RegisterSystem()
	{
		const char* typeName = typeid(T).name();

		assert(mRegisteredSystems.find(typeName) == mRegisteredSystems.end() && "System has already been Registered.");

		auto system = std::make_shared<T>();
		mRegisteredSystems.insert({ typeName, system });
		return system;
	}

	template<typename T>
	void SetSystemSignature(Signature signature)
	{
		const char* typeName = typeid(T).name();

		assert(mRegisteredSystems.find(typeName) != mRegisteredSystems.end() && "Trying to set system signature before registering.");

		mSystemSignatures.insert({ typeName, signature });
	}

	void EntityDestroyed(EntityID entity)
	{

		for (auto const& pair : mRegisteredSystems)
		{
			auto const& system = pair.second;

			system->mEntities.erase(entity);
		}
	}

	void EntitySignatureChanged(EntityID entity, Signature entitySignature);


	//Get a list of all registered systems for system process (Debug)
	std::vector<std::shared_ptr<System>> GetAllSystems() {
		std::vector<std::shared_ptr<System>> systems;
		for (const auto& pair : mRegisteredSystems) {
			systems.push_back(pair.second);
		}
		return systems;
	}


};

#endif // ! SYSTEMS_MANAGER_H