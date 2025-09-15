/**
 * @file Physics.h
 * @brief Implementation of the core physics system, including movement, collision detection, and force application.
 *
 * This file defines the physics logic of the game engine, responsible for updating the positions and velocities of entities,
 * detecting and responding to collisions, and applying various forces (e.g., gravity, drag, and custom forces).
 * The file also integrates physics with the game's Entity Component System (ECS) for dynamic object manipulation and interaction.
 *
 * Key Features:
 * - **Force Management**:
 *   - `Force`: Defines forces (linear, rotational, drag) applied to physics bodies, including calculations for force vectors and torque.
 *   - `DragForce`: A specialized force for drag effects that applies a force proportional to the velocity of an object.
 *   - `ForcesManager`: Manages the application and removal of forces on physics bodies and calculates the net force and torque.
 *   - `ApplyForces`: Calculates and applies forces to entities, updating acceleration and velocity based on mass.
 * - **Physics Body Properties**:
 *   - `PhysicsBody`: Contains core physical properties like mass, velocity, acceleration, rotational dynamics (angle, angular velocity), and forces.
 *   - Integrated with `AABB` for collision detection and `ForcesManager` for managing forces.
 * - **Movement & Collision**:
 *   - `Movement`: Updates the position and velocity of a physics body, including movement based on applied forces.
 *   - `HandleCollisions`: Detects and handles collisions between physics bodies and entities.
 * - **Collision Response**:
 *   - `CollisionResponse`: Handles the response to detected collisions between entities. This function adjusts the velocities and positions of entities involved in the collision, ensuring realistic interaction and separation after impact.
 *   - Specific collision response functions:
 *     - `HandleThiefWallCollision`: Resolves collisions between Thief entities and Wall entities.
 *     - `HandleThiefObjectCollision`: Handles collisions between Thief entities and other objects.
 *     - `HandleThiefSwitchCollision`: Manages the response to Thief entities interacting with Switch entities.
 *     - `HandleThiefDoorCollision`: Handles collisions between Thief entities and Door entities.
 * - **Gravity and Jumping**:
 *   - `ApplyGravity`: Applies gravity to entities, affecting their velocity and position over time.
 *   - `Jumping`: Manages the jump mechanics, adjusting the vertical velocity for jumping entities.
 *   - `CalculateLine`: Provides debug functionality for calculating and visualizing trajectories for physics bodies during movement.
 *   - `MouseDragInfo`: Handles drag actions, storing relevant information for drag-based movements.
 * - **Pause Mode**:
 *   - `isPaused`, `stepFrame`: Controls the pause functionality and frame stepping for debugging purposes.
 * - **Entity Updates**:
 *   - `MoveEntity`: Updates entity positions based on velocity and delta time.
 *   - `UpdateTransform`: Synchronizes physics-based positions with ECS transform components.
 * - **Audio Integration**:
 *   - Plays footstep sounds during movement and pauses the audio when the entity is idle.
 * Utility Functions:
 * - `SyncAABBWithTransform`: Synchronizes the AABB (bounding box) with the entity's transformation.
 * - `CheckPauseToggle`: Checks for pause/resume actions to control physics updates during gameplay.
 * - `EnforceWindowBoundaries`: Ensures entities stay within the window boundaries during the physics update.
 *
 * Key Systems and Interactions:
 * - Integrated with the ECS (Entity Component System), each entity has a `PhysicsBody` component that is updated during the physics simulation.
 * - Works alongside the collision detection system (`Collision.h`) to handle entity interactions and respond to collisions.
 * - Handles various force types like linear, rotational, drag, and custom forces to simulate realistic movement and behavior of game objects.
 * - Supports gravity, jumping, and friction as part of the physics engine, offering flexibility for different types of interactions and mechanics.
 * - Implements message handling through `CoreEngine::Observer` for reacting to collision events and other system messages.
 *
 * Author: Jason (100%)
 */

#pragma once
#include "GlobalVariables.h"
#include "SystemsManager.h"
#include "Collision.h"
#include "vector"
#include "MessageSystem.h"
#include "vector2d.h"

namespace PhysicsTemp
{
	struct DragInfo {
		float DragBeginX{}, DragBeginY{};
		float DragEndX{}, DragEndY{};
		bool isDragging = false;
		Math2D::Vector2D dragVector;

		std::vector<EntityID> trajectoryEntities;
	};
}

class PhysicsSystem : public System, public CoreEngine::Observer
{
public:
	const float	MOVE_VELOCITY = 20.0f;
	const float GRAVITY = 30.81f;
	Grid spatialGrid;

	enum class ForceType {
		None,
		Linear,
		Rotational,
		Mixed,
		Drag
	};

	struct Force {
		Math2D::Vector2D direction;
		float magnitude;
		ForceType type;
		float lifetime;
		float age = 0.0f;
		bool IsActive = true;

		float radius = 0.0f;    // Distance from center of mass for torque calculations
		float torque = 0.0f;    // Precomputed torque, for rotational forces

		Force(Math2D::Vector2D dir = { 0, 0 }, float mag = 1.0f, ForceType ft = ForceType::None, float life = 0.0f)
			: direction(dir), magnitude(mag), type(ft), lifetime(life) {}

		Math2D::Vector2D getForceVector() const {
			return direction * magnitude;
		}

		// Calculate torque based on radius for rotational forces
		float getTorque() const {
			if (type == ForceType::Rotational) {
				return radius * magnitude;  // Torque = radius * force magnitude
			}
			return torque;  // If torque is set directly
		}
	};

	class DragForce : public Force {
	public:
		DragForce(float dragCoeff = 0.1f)
			: Force({ 0.0f, 0.0f }, 1.0f), dragCoefficient(dragCoeff) {}

		Math2D::Vector2D getForce(const Math2D::Vector2D& velocity) {
			// Drag force is proportional to the negative of the velocity
			return velocity * -dragCoefficient;
		}

		float getDragCoefficient() const { return dragCoefficient; }

	private:
		float dragCoefficient;
	};

	class ForcesManager {
	public:
		void AddForce(const Force& force) {
			forces.push_back(force);
		}

		void RemoveForce(ForceType type) {
			forces.erase(std::remove_if(forces.begin(), forces.end(),
				[type](const Force& f) { return f.type == type; }), forces.end());
		}

		void ClearForces() {
			forces.clear();
		}

		Force* getDragForce() {
			for (auto& force : forces) {
				if (force.type == ForceType::Drag) {
					return &force;
				}
			}
			return nullptr;
		}

		Math2D::Vector2D GetNetForce(double deltaTime) {
			(void)deltaTime;
			Math2D::Vector2D totalForce = { 0.0f, 0.0f };
			for (auto& force : forces) {
				totalForce = totalForce + force.getForceVector();
				/*if (force.IsActive) {
					ValidateAge(force, deltaTime);
					totalForce = totalForce + force.getForceVector();
				}*/
			}
			return totalForce;
		}

		float GetNetTorque(double deltaTime) {
			float totalTorque = 0.0f;
			for (auto& force : forces) {
				if (force.IsActive && force.type == ForceType::Rotational) {
					ValidateAge(force, deltaTime);
					totalTorque += force.getTorque();
				}
			}
			return totalTorque;
		}

	private:
		std::vector<Force> forces;

		void ValidateAge(Force& force, double deltaTime) {
			force.age += static_cast<float>(deltaTime);
			if (force.age >= force.lifetime) {
				force.IsActive = false;
			}
		}
	};

	struct AutoDoor {
		std::string switchName;
		bool isOpen;
	};

	struct Switch {
		bool isOn;
		std::vector<std::string> interactables;
	};

	struct PhysicsBody {
		std::string category;

		// Core physical properties
		float mass = 1.0f;
		float inertiaMass = 1.0f;  // Required for rotational dynamics
		Math2D::Vector2D velocity{ 0.0f, 0.0f };
		Math2D::Vector2D acceleration{ 0.0f, 0.0f };

		// Rotational dynamics
		float angle = 0.0f;
		float angularVelocity = 0.0f;
		float angularAcceleration = 0.0f;
		Math2D::Vector2D position;
		Math2D::Vector2D size;

		// Forces and interactions
		ForcesManager forcesManager;
		AABB aabb;
		float friction = 0.0f;
		bool Switch = false;
		bool isGrounded = true;
 
		EntityID entityID;
	};

	// Core Functions
	void Init()override;
	void Update(double deltaTime)override ;
	const char* getName() const override {
		return "PhysicsSystem";
	}


	// Helper Functions 
	void ProcessEntity(EntityID entity, double deltaTime);
	void MoveEntity(PhysicsBody& body, double deltaTime);
	void UpdateTransform(EntityID entity, PhysicsBody& body);

	// Force application
	void ApplyForces(PhysicsBody& body, double deltaTime);

	// Physics Main Function
	void Movement(PhysicsBody& body);
	void MouseDragInfo(PhysicsSystem::PhysicsBody& body);
	void Jumping(PhysicsBody& body, PhysicsTemp::DragInfo* DragInfo);
	void ApplyGravity(PhysicsBody& body, double deltaTime);
	
	// Demo & Debugging
	void CalculateLine(PhysicsTemp::DragInfo* dragInfo, PhysicsSystem::PhysicsBody& body);

	//	Collision Function
	bool HandleCollisions(EntityID entity, PhysicsBody& body, double deltaTime);
	void CollisionResponse(PhysicsSystem::PhysicsBody& body1, PhysicsSystem::PhysicsBody& body2, float tFirst, EntityID enitty, EntityID otherEntity);
	bool IsCollision(const std::string& entity1, const std::string& entity2, PhysicsBody& body1, PhysicsBody& body2);
	void HandleThiefWallCollision(PhysicsBody& body1, PhysicsBody& body2, float firstTimeOfCollision);
	void HandleThiefLaserCollision(PhysicsBody& body1, PhysicsBody& body2, float firstTimeOfCollision, EntityID other);
	void HandleThiefObjectCollision(PhysicsBody& body1, PhysicsBody& body2, float firstTimeOfCollision, EntityID other);
	void HandleThiefSwitchCollision(PhysicsBody& body1, float firstTimeOfCollision, EntityID other);
	void HandleThiefVentCollision(PhysicsBody& body1, PhysicsBody& body2, float firstTimeOfCollision, EntityID other);
	void HandleThiefDoorCollision(PhysicsBody& body1, PhysicsBody& body2, float firstTimeOfCollision, EntityID other);
	void EnforceWindowBoundaries(PhysicsBody& body, float windowWidth, float windowHeight);

	void HandleMessage(CoreEngine::IMessage* message) override {
		switch (message->GetMessageID()) {
		case CoreEngine::CollisionDetected:
			//CollisionDetectedHandler(message);
			break;
		default:
			break;
		}
	}

private:
	// New variables for pause mode
	bool isPaused = false;
	bool stepFrame = false;

	PhysicsTemp::DragInfo DragInfo;
	std::vector<EntityID> entitiesToDestroy;
	

	// Helper functions for pause and resume
	void CheckPauseToggle();
	void SyncAABBWithTransform(EntityID entity, PhysicsBody& body, const Transform& transform);
};

void PlayRandomSound(const std::vector<std::string>& soundList, int customChannel, std::string &currentSound, float volume);