/**
 * @file Component.h
 * @brief Header file defining the Component Storage and Management system for the ECS framework.
 *
 * This file provides the implementation of core classes and templates to manage components in an
 * Entity-Component-System (ECS) framework. It includes the `IComponentStorage` interface for generic
 * component storage, the `ComponentStorage` template for type-specific storage, and the `ComponentManager`
 * class to register and manage components dynamically.
 *
 * Key Features:
 * - **Generic Component Storage**:
 *   - `IComponentStorage`: Interface for component storage, enabling polymorphic behavior for clearing and
 *     handling destroyed entities.
 * - **Type-Specific Component Storage**:
 *   - `ComponentStorage<T>`: Template for managing components of type `T`, providing methods to add, remove,
 *     and retrieve components by entity ID.
 * - **Dynamic Component Management**:
 *   - `ComponentManager`: Registers component types, dynamically assigns component types, and provides
 *     methods to manage components for entities.
 *
 * Key Classes:
 * - `IComponentStorage`:
 *   - Interface defining methods for clearing component data and handling entity destruction.
 * - `ComponentStorage<T>`:
 *   - Manages component data for a specific type, ensuring efficient storage and retrieval using entity IDs.
 * - `ComponentManager`:
 *   - Manages multiple component types, provides dynamic component registration, and integrates with
 *     entities to handle their components.
 *
 * Functions:
 * - **IComponentStorage**:
 *   - `Clear`: Clears all stored component data.
 *   - `EntityDestroyed`: Handles cleanup when an entity is destroyed.
 * - **ComponentStorage<T>**:
 *   - `InsertEntityData`: Adds a component for a specific entity.
 *   - `RemoveEntityData`: Removes a component associated with an entity.
 *   - `GetEntityData`: Retrieves a component for a given entity.
 * - **ComponentManager**:
 *   - `RegisterComponent`: Registers a new component type.
 *   - `AddComponent`: Adds a component of a specific type to an entity.
 *   - `RemoveComponent`: Removes a component of a specific type from an entity.
 *   - `DestroyAllEntities`: Clears all component data across all registered types.
 *
 * Integration:
 * - The `ComponentManager` works seamlessly with the ECS framework, dynamically associating components
 *   with entities based on their IDs.
 * - Ensures components are efficiently managed and cleaned up when entities are destroyed.
 *
 * Author: Che Ee (100%)
 */

#pragma once
#ifndef COMPONENT_H
#define COMPONENT_H
#include "CommonIncludes.h"
#include "EntityManager.h"

class IComponentStorage
{
public:
	// New virtual method for clearing component storage
	virtual void Clear() = 0;
	virtual ~IComponentStorage() = default;

	virtual void EntityDestroyed(EntityID entity) = 0;
};

//ComponentStorage is basically an array. For eg ComponentStorage PositionArray<Struct Position>;
template<typename T>
class ComponentStorage : public IComponentStorage
{
private:

	//contains the raw component -Position struct for example
	std::array<T, MAX_GAME_OBJECTS> mComponentStorage;

	//Entity is the key and index of this particular Array (eg PositionArray) is the value
	std::unordered_map<EntityID, size_t> mEntityToComponentIndexMap;

	std::unordered_map<size_t, EntityID> mComponentIndexToEntityMap;
	size_t mSize{}; //number of Components

public:

	void Clear() override {
		mComponentStorage.fill(T{});  // This sets all elements in the array to the default-constructed value of T
		mEntityToComponentIndexMap.clear();
		mComponentIndexToEntityMap.clear();
		mSize = 0;
	}
	void InsertEntityData(EntityID entity, T component)
	{
		assert(mEntityToComponentIndexMap.find(entity) == mEntityToComponentIndexMap.end() && "Component added to same entity more than once.");

		size_t newIndex = mSize;
		mEntityToComponentIndexMap[entity] = newIndex;
		mComponentIndexToEntityMap[newIndex] = entity;
		mComponentStorage[newIndex] = component;
		++mSize;
	}

	void RemoveEntityData(EntityID entity)
	{
		assert(mEntityToComponentIndexMap.find(entity) != mEntityToComponentIndexMap.end() && "Removing non-existent component.");

		// Copy element at end into deleted element's place to maintain density
		size_t indexOfRemovedEntity = mEntityToComponentIndexMap[entity];
		size_t indexOfLastElement = mSize - 1;
		mComponentStorage[indexOfRemovedEntity] = mComponentStorage[indexOfLastElement];

		// Update map to point to moved spot
		EntityID entityOfLastElement = mComponentIndexToEntityMap[indexOfLastElement];
		mEntityToComponentIndexMap[entityOfLastElement] = indexOfRemovedEntity;
		mComponentIndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

		mEntityToComponentIndexMap.erase(entity);
		mComponentIndexToEntityMap.erase(indexOfLastElement);

		--mSize;
	}

	T& GetEntityData(EntityID entity)
	{
		assert(mEntityToComponentIndexMap.find(entity) != mEntityToComponentIndexMap.end() && "Retrieving non-existent component.");

		// Return a reference to the entity's component
		return mComponentStorage[mEntityToComponentIndexMap[entity]];
	}

	void EntityDestroyed(EntityID entity) override
	{
		if (mEntityToComponentIndexMap.find(entity) != mEntityToComponentIndexMap.end())
		{
			// Remove the entity's component if it existed
			RemoveEntityData(entity);
		}
	}
};


class ComponentManager
{

private:
	// Map from type string pointer to a component type
	//key is typeid while value is ComponentType
	std::unordered_map<const char*, ComponentType> mComponentTypes{};

	// Map from typeid to a pointer to the IComponentArray (eg: Position typeid maps to Position Component Array);
	std::unordered_map<const char*, std::shared_ptr<IComponentStorage>> mComponentStorages{};

	// The component type to be assigned to the next registered component - starting at 0
	ComponentType mNextComponentType{};

	// Convenience function to get the statically casted pointer to the ComponentStorage of type T.
	template<typename T>
	std::shared_ptr<ComponentStorage<T>> GetComponentStorage()
	{
		const char* typeName = typeid(T).name();

		assert(mComponentTypes.find(typeName) != mComponentTypes.end() && "Component not registered before use.");

		return std::static_pointer_cast<ComponentStorage<T>>(mComponentStorages[typeName]);
	}

public:

	void DestroyAllUIEntities();

	void DestroyAllEntities() {
		// Loop through all component storages and clear each one
		for (auto& pair : mComponentStorages) {
			pair.second->Clear();  // Call Clear() on each component storage
		}
	}

	template<typename T>
	void RegisterComponent()
	{
		const char* typeName = typeid(T).name();

		assert(mComponentTypes.find(typeName) == mComponentTypes.end() && "Registering component type more than once.");

		// Add this component type to the component type map
		mComponentTypes.insert({ typeName, mNextComponentType });

		// Create a ComponentStorage pointer and add it to the component arrays map
		mComponentStorages.insert({ typeName, std::make_shared<ComponentStorage<T>>() });

		// Increment the value so that the next component registered will be different
		++mNextComponentType;
	}

	template<typename T>
	ComponentType GetComponentType()
	{
		const char* typeName = typeid(T).name();

		assert(mComponentTypes.find(typeName) != mComponentTypes.end() && "Component not registered before use.");

		// Return this component's type - used for creating signatures
		return mComponentTypes[typeName];
	}

	template<typename T>
	void AddComponent(EntityID entity, T component)
	{
		// Add a component to the array for an entity
		GetComponentStorage<T>()->InsertEntityData(entity, component);
	}

	template<typename T>
	void RemoveComponent(EntityID entity)
	{
		// Remove a component from the array for an entity
		GetComponentStorage<T>()->RemoveEntityData(entity);
	}

	template<typename T>
	T& GetComponent(EntityID entity)
	{
		// Get a reference to a component from the array for an entity
		return GetComponentStorage<T>()->GetEntityData(entity);
	}

	void EntityDestroyed(EntityID entity)
	{
		// Notify each component array that an entity has been destroyed
		// If it has a component for that entity, it will remove it
		for (auto const& pair : mComponentStorages)
		{
			auto const& component = pair.second;

			component->EntityDestroyed(entity);
		}
	}
};
#endif