/**
 * @file Collision.h
 * @brief Collision detection utilities for various types of objects in the game engine.
 *
 * This file provides the necessary data structures and functions to detect collisions between different
 * types of objects such as rectangles, circles, and binary map instances. These functions calculate
 * the first point of intersection between two colliding objects and handle the physical response.
 *
 * Key Structures:
 * - **AABB (Axis-Aligned Bounding Box)**:
 *   - Defines a rectangular bounding box used for efficient collision checks between rectangle-based objects.
 * - **Ray**:
 *   - Represents a ray for raycasting (not yet fully implemented).
 * - **LineSegment**:
 *   - Defines a line segment, including endpoints and a normal vector, for line-based collisions.
 * - **Circle**:
 *   - Defines a circle with a center and radius, used for collision detection with other circular objects.
 * - **Grid**:
 *   - A spatial partitioning grid that stores entities in cells for efficient collision checking with nearby entities.
 *   - Includes functions to clear the grid, add entities to the grid, and retrieve nearby entities.

 * Key Functions:
 * - **CollisionIntersection_RectRect**:
 *   - Detects if two rectangles (represented by AABBs) are colliding, considering their velocities.
 *   - Computes the first time of collision between two moving rectangles.
 * - **CollisionIntersection_CircleCircle**:
 *   - Detects if two circles are colliding, considering their velocities.
 *   - Computes the first time of collision between two moving circles.
 * - **CheckInstanceBinaryMapCollision**:
 *   - Checks for collisions between an entity's position and a binary map (e.g., collision grid or obstacle map).
 *   - Returns an integer indicating the collision status based on the binary map.

 * Utility Functions:
 * - **Grid Management**:
 *   - The `Grid` structure provides spatial partitioning for efficiently checking for collisions between entities.
 *   - The `clear()` method resets the grid, while `addEntity()` places entities in the appropriate grid cell, and `getNearbyEntities()` retrieves entities in adjacent cells for collision checks.
 *
 * Collision Types Supported:
 * - Rectangle-Rectangle (AABB-AABB) collisions.
 * - Circle-Circle collisions.
 * - Collision with binary map instances (e.g., checking against walls or obstacles).
 *
 * This system is optimized for 2D games using a grid-based spatial partitioning method for better performance during collision checks.
 * 
 * Author: Jason (100%)
 */

#pragma once
#include "vector2d.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>

const int GRID_CELL_SIZE = 50; // Size of each grid cell (adjust as necessary)
const int GRID_WIDTH = 1600 / GRID_CELL_SIZE;
const int GRID_HEIGHT = 900 / GRID_CELL_SIZE;

struct AABB {
	float minX, minY;
	float maxX, maxY;
};

struct Ray {
	//AEVector origin;
	//AEvector direction;
};

struct LineSegment {
	Math2D::Vector2D	m_pt0;
	Math2D::Vector2D	m_pt1;
	Math2D::Vector2D	m_normal;
};

struct Circle
{
	Math2D::Vector2D  m_center;
	float	m_radius;
};

struct Grid {
    std::vector<std::vector<std::vector<int>>> cells; // Each cell stores indices of entities (circles)

    Grid() {
        cells.resize(GRID_WIDTH, std::vector<std::vector<int>>(GRID_HEIGHT));
    }

    void clear() {
        for (auto& row : cells) {
            for (auto& cell : row) {
                cell.clear();
            }
        }
    }

    void addEntity(int entityID, float minX, float minY, float maxX, float maxY) {
        int startCellX = static_cast<int>(minX) / GRID_CELL_SIZE;
        int startCellY = static_cast<int>(minY) / GRID_CELL_SIZE;
        int endCellX = static_cast<int>(maxX) / GRID_CELL_SIZE;
        int endCellY = static_cast<int>(maxY) / GRID_CELL_SIZE;

        for (int x = startCellX; x <= endCellX; ++x) {
            for (int y = startCellY; y <= endCellY; ++y) {
                if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
                    cells[x][y].push_back(entityID);
                }
            }
        }
    }

    std::vector<int> getNearbyEntities(float minX, float minY, float maxX, float maxY) {
        std::vector<int> nearbyEntities;

        // Compute grid coverage based on AABB
        int startCellX = static_cast<int>(std::floor(minX / GRID_CELL_SIZE));
        int startCellY = static_cast<int>(std::floor(minY / GRID_CELL_SIZE));
        int endCellX = static_cast<int>(std::ceil(maxX / GRID_CELL_SIZE));
        int endCellY = static_cast<int>(std::ceil(maxY / GRID_CELL_SIZE));

        // Iterate through all grid cells that the AABB covers
        for (int x = startCellX - 1; x <= endCellX + 1; ++x) {
            for (int y = startCellY - 1; y <= endCellY + 1; ++y) {
                if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
                    // Add all entities from the grid cell
                    nearbyEntities.insert(nearbyEntities.end(), cells[x][y].begin(), cells[x][y].end());
                }
            }
        }
        return nearbyEntities;
    }


};

//Collision between rectangle object
bool CollisionIntersection_RectRect(const AABB& aabb1,
	float vel1X, float vel1Y,
	const AABB& aabb2,
	float vel2X, float vel2Y,
	float& firstTimeOfCollision);

//Collision between circle object
bool CollisionIntersection_CircleCircle(const Circle& circle1,
	float vel1X, float vel1Y,
	const Circle& circle2,
	float vel2X, float vel2Y,
	float& firstTimeOfCollision);

//collision between binary map
int CheckInstanceBinaryMapCollision(float PosX, float PosY, float scaleX, float scaleY);
