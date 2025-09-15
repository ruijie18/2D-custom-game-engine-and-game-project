///////////////////////////////////////////////////////////////////////////////////////
///
/// @file Coordinator.h
/// @brief The `ECSCoordinator` class manages the core functionality of an Entity-Component-System (ECS) framework.
///
/// This file provides the definition of the `ECSCoordinator` class, which serves as a central hub for managing 
/// entities, components, and systems in an ECS architecture. It handles entity creation, destruction, 
/// and interactions between components and systems, while also enabling custom logic for specific use cases.
///
/// ### Key Features:
/// - **Entity Management**:
///   - Create, destroy, and manage all game entities, including UI objects and gameplay objects.
/// - **Component Management**:
///   - Add, remove, and retrieve components associated with entities.
///   - Register new component types dynamically.
/// - **System Management**:
///   - Register systems and manage their interactions with entities based on signatures.
///   - Update systems during each game loop cycle.
/// - **Specialized Features**:
///   - Support for cloning entities with new positions.
///   - Ability to create specific types of entities (e.g., text or texture entities).
///   - Handles the main character (thief) entity with dedicated methods.
///
/// ### Integration:
/// - Relies on `ComponentManager`, `GameObjectManager`, and `SystemManager` for modularity.
/// - Interacts with external systems, such as rendering and physics, through registered systems.
///
/// ### Author:
/// - Che Ee (100%)
///
///////////////////////////////////////////////////////////////////////////////////////

#ifndef COORDINATOR_H
#define COORDINATOR_H

#include "CommonIncludes.h"
#include "Component.h"
#include "SystemsManager.h"
#include "ListOfComponents.h"
#include "Graphics.h"
#include "ButtonComponent.h"

#include "AssetsManager.h"

constexpr EntityID INVALID_ENTITY = static_cast<EntityID>(-1);

class ECSCoordinator
{

private:
//meant to get the info for the main character.
	EntityID thiefID = INVALID_ENTITY;

	std::unique_ptr<ComponentManager> mComponentManager;
	std::unique_ptr<GameObjectManager> mGameObjectManager;
	std::unique_ptr<SystemManager> mSystemManager;
	static std::unordered_set<std::string> existingEntityNames;  // Static set to track names

	
public:

	struct ButtonComponent {
		std::function<void()> onClick;
		std::string action; // Store the action from the JSON file
		bool isHovered = false;        // Tracks whether the button is currently hovered
		float hoverScaleFactor = 1.2f; // Scale factor for hover effect
	};

	template<typename T>
	void RegisterComponent() {
		mComponentManager->RegisterComponent<T>();
	}


	void DestroyAllUIObjects() {
		mComponentManager->DestroyAllUIEntities();

		mSystemManager->DestroyAllUIEntities();

		mGameObjectManager->DestroyAllUIGameObjects();

	}

	void FadeInAllObjects() {
		mGameObjectManager->FadeInAllObjects();
	}

	void FadeOutAllObjects() {
		mGameObjectManager->FadeOutAllObjects();
	}



	void DestroyAllGameObjects() {
		mComponentManager->DestroyAllEntities();

		mSystemManager->DestroyAllEntities();

		mGameObjectManager->DestroyAllGameObjects();

	}

	//assuming object has glmodel and transform
	//EntityID CloneEntityWithNewPosition(EntityID originalEntity, float newX, float newY);
	void CloneEntityWithNewPosition(EntityID originalEntity, float newX, float newY);
	void CreateNewEntity(std::string name, float sizeX, float sizeY, float posX, float posY);
	void CreateNewTextureEntity(Texture& texture, float posX, float posY);
	void CreateTextEntity(const std::string& text, float scale, glm::vec3 color, float posX, float posY, float width, float height, std::string fontname,int size,std::string entityName);
	void ClearAllEntities();
	std::string FormatEntityName(const std::string& filename); 
	void StopGame();

	
// Setter for thiefID
	//ideas of asserting if thief ID is valid
	void setThiefID(EntityID id) {
		if (id != INVALID_ENTITY) { // Ensure it's a valid ID
			thiefID = id;
		}
	}

	// Getter for thiefID
	EntityID getThiefID() const {
		return thiefID;
	}

	// Check if thiefID is valid
	bool hasThiefID() const {
		return thiefID != INVALID_ENTITY;
	}

	// Reset thiefID to an invalid state
	void resetThiefID() {
		thiefID = INVALID_ENTITY;
	}


	void InitSystems() {
		mSystemManager->Init();
	}

	void UpdateSystems(double deltaTime) {
		mSystemManager->Update(deltaTime);
	}

	void Init()
	{
		// Create pointers to each manager
		mComponentManager = std::make_unique<ComponentManager>();
		mGameObjectManager = std::make_unique<GameObjectManager>();
		mSystemManager = std::make_unique<SystemManager>();
	}


	// Entity methods
	EntityID CreateGameObject()
	{
		return mGameObjectManager->CreateGameObject();
	}

	//destroy all entiti
	void DestroyGameObject(EntityID entity)
	{

		
		mGameObjectManager->DestroyGameObject(entity);

		mComponentManager->EntityDestroyed(entity);

		mSystemManager->EntityDestroyed(entity);
	}

	std::vector<EntityID> GetAllEntities() {
		return mGameObjectManager->GetAllEntities(); // Assuming your GameObjectManager has a method to return all entities
	}

	void PrintAllEntitiesComponents() {
		mGameObjectManager->PrintAllEntitiesWithComponents();
	}

	template<typename T>
	void AddComponent(EntityID entity, T component)
	{
		mComponentManager->AddComponent<T>(entity, component);

		auto signature = mGameObjectManager->GetComponentSignature(entity);
		signature.set(mComponentManager->GetComponentType<T>(), true);
		mGameObjectManager->SetComponentSignature(entity, signature);

		mSystemManager->EntitySignatureChanged(entity, signature);
	}

	template<typename T>
	void RemoveComponent(EntityID entity)
	{
		mComponentManager->RemoveComponent<T>(entity);

		auto signature = mGameObjectManager->GetComponentSignature(entity);
		signature.set(mComponentManager->GetComponentType<T>(), false);
		mGameObjectManager->SetComponentSignature(entity, signature);
		mSystemManager->EntitySignatureChanged(entity, signature);
	}

	Signature GetEntitySignature(EntityID entity) {
		return mGameObjectManager->GetComponentSignature(entity);
	}

	bool hasRequiredComponents(const Signature& sig, const std::vector<int>& componentOrder) {
		for (const auto& componentIndex : componentOrder) {
			if (!sig[componentIndex]) { // Check if the component is present
				return false; // Component is missing
			}
		}
		return true; // All required components are present
	}

	template<typename T>
	T& GetComponent(EntityID entity)
	{
		return mComponentManager->GetComponent<T>(entity);
	}

	template<typename T>
	ComponentType GetComponentType()
	{

		return mComponentManager->GetComponentType<T>();
	}

	// Method to check if an entity has a specific component
	template<typename T>
	bool HasComponent(EntityID entity) {
		// Get the component type for T
		ComponentType componentType = mComponentManager->GetComponentType<T>();

		// Retrieve the component signature for the entity
		const Signature& signature = mGameObjectManager->GetComponentSignature(entity);

		// Check if the signature has the bit set for this component type
		return signature.test(static_cast<size_t>(componentType));
	}

	// System methods
	template<typename T>
	std::shared_ptr<T> RegisterSystem()
	{
		return mSystemManager->RegisterSystem<T>();
	}

	template<typename T>
	void SetSystemSignature(Signature signature)
	{
		mSystemManager->SetSystemSignature<T>(signature);
	}


	std::vector<std::shared_ptr<System>> GetRegisteredSystems() {
		return mSystemManager->GetAllSystems();
	}

	uint32_t GetTotalNumberOfEntities() const {
		return mGameObjectManager->GetActiveEntityCount();
	}

	template <typename T>
	std::shared_ptr<T> GetSystem()
	{
		const char* typeName = typeid(T).name();
		auto it = mSystemManager->mRegisteredSystems.find(typeName);

		if (it != mSystemManager->mRegisteredSystems.end())
		{
			// Use dynamic_pointer_cast to ensure type safety
			return std::dynamic_pointer_cast<T>(it->second);
		}

		return nullptr; // If the system is not found
	}

	ComponentManager& GetComponentManager() {
		return *mComponentManager; // Dereference the unique_ptr to return a reference
	}


};

#endif