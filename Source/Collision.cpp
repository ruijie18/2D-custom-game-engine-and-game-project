/**
 * @file Collision.cpp
 * @brief Contains functions for detecting and handling collisions between various objects.
 *
 * This file provides the logic for detecting collisions between axis-aligned bounding boxes (AABBs),
 * circles, and binary map-based collision detection. It includes functions for calculating collision times,
 * handling binary map-based checks, and detecting object boundaries.
 *
 * Key collision functions:
 * - CollisionIntersection_RectRect: Detects collisions between two AABBs and calculates the time of collision.
 * - CollisionIntersection_CircleCircle: Detects collisions between two circles and calculates the time of collision.
 * - CheckInstanceBinaryMapCollision: Checks for collisions between an object and a binary collision map.
 *
 * The functions in this file are designed to support real-time physics and object interactions,
 * and work with the custom grid system and binary map to manage collision detection in the game environment.
 *
 * Collision flags are used to indicate which sides of the objects have collided (left, right, top, bottom).
 * The binary map system helps in detecting static collision objects based on their position within a grid.
 *
 * Collision constants:
 * - COLLISION_LEFT, COLLISION_RIGHT, COLLISION_TOP, COLLISION_BOTTOM: Indicate the sides of collisions.
 * - TYPE_OBJECT_COLLISION: Marks objects that are involved in collisions in the binary map.
 *
 * @note The functions make use of the global time step `g_dt` to handle time-based calculations, ensuring that
 * collisions are detected and processed within the given frame.
 * 
 * Author: Jason (100%)
 */

#include "Collision.h"

#define COLLISION_LEFT   1
#define COLLISION_RIGHT  2
#define COLLISION_TOP    4
#define COLLISION_BOTTOM 8
#define TYPE_OBJECT_COLLISION 1

static int** BinaryCollisionArray;
static int				BINARY_MAP_WIDTH;
static int				BINARY_MAP_HEIGHT;


float g_dt;

bool CollisionIntersection_RectRect(const AABB& aabb1,
    float vel1X, float vel1Y,
    const AABB& aabb2,
    float vel2X, float vel2Y,
    float& firstTimeOfCollision) {

    // Check if there is no overlap between the AABBs in any dimension
    if (aabb1.maxX <= aabb2.minX || aabb2.maxX <= aabb1.minX || aabb1.maxY <= aabb2.minY || aabb2.maxY <= aabb1.minY) {
        return false;
    }

    // Initialize time of first and last collision
    float tFirst = 0.0f;
    float tLast = g_dt; // g_dt represents the time step

    //// Calculate relative velocity between the two objects
    float VrelX = vel2X - vel1X;
    float VrelY = vel2Y = vel1Y;

    // Calculate collision times for X-axis
    if (VrelX != 0) {
        float tEnterX = (VrelX > 0) ? ((aabb2.minX - aabb1.maxX) / VrelX) : ((aabb2.maxX - aabb1.minX) / VrelX);
        float tExitX = (VrelX > 0) ? ((aabb2.maxX - aabb1.minX) / VrelX) : ((aabb2.minX - aabb1.maxX) / VrelX);
        // Swap tEnterX and tExitX if necessary to ensure tEnterX <= tExitX
        if (tEnterX > tExitX) {
            float temp = tEnterX;
            tEnterX = tExitX;
            tExitX = temp;
        }

        // Update tFirst and tLast based on collision times
        tFirst = (tEnterX > tFirst) ? tEnterX : tFirst;
        tLast = (tExitX < tLast) ? tExitX : tLast;
    }

    // Calculate collision times for Y-axis
    if (VrelY != 0) {
        float tEnterY = (VrelY > 0) ? ((aabb2.minY - aabb1.maxY) / VrelY) : ((aabb2.maxY - aabb1.minY) / VrelY);
        float tExitY = (VrelY > 0) ? ((aabb2.maxY - aabb1.minY) / VrelY) : ((aabb2.minY - aabb1.maxY) / VrelY);
        // Swap tEnterY and tExitY if necessary to ensure tEnterY <= tExitY
        if (tEnterY > tExitY) {
            float temp = tEnterY;
            tEnterY = tExitY;
            tExitY = temp;
        }
        // Update tFirst and tLast based on collision times
        tFirst = (tEnterY > tFirst) ? tEnterY : tFirst;
        tLast = (tExitY < tLast) ? tExitY : tLast;
    }

    //Check if there is a collision within the movement interval
    if (tFirst > tLast) {
        return false; // No collision within movement interval
    }

    // Update the time of first collision
    firstTimeOfCollision = tFirst;
    return true; // Collision detected
}

bool CollisionIntersection_CircleCircle(const Circle& circle1,
    float vel1X, float vel1Y,
    const Circle& circle2,
    float vel2X, float vel2Y,
    float& firstTimeOfCollision) {
    // Calculate relative velocity
    float VrelX = vel2X - vel1X;
    float VrelY = vel2Y - vel1Y;

    // Calculate initial distance between centers
    float dx = circle2.m_center.x - circle1.m_center.x;
    float dy = circle2.m_center.y - circle1.m_center.y;
    float distSquared = dx * dx + dy * dy;

    // Calculate sum of radii
    float combinedRadius = circle1.m_radius + circle2.m_radius;
    float combinedRadiusSquared = combinedRadius * combinedRadius;

    // Check if circles are currently overlapping
    if (distSquared <= combinedRadiusSquared) {
        firstTimeOfCollision = 0.0f;
        return true; // Circles are already colliding
    }

    // Calculate relative velocity squared
    float VrelSquared = VrelX * VrelX + VrelY * VrelY;

    // Early exit if relative velocity is zero (parallel motion)
    if (VrelSquared == 0) {
        return false;
    }

    // Calculate the dot product of the relative velocity and the vector between centers
    float dotProduct = dx * VrelX + dy * VrelY;

    // Check if circles are moving towards each other
    if (dotProduct > 0) {
        return false; // Circles are moving apart
    }

    // Calculate the discriminant of the quadratic equation
    float discriminant = dotProduct * dotProduct - VrelSquared * (distSquared - combinedRadiusSquared);

    // If discriminant is negative, no collision occurs
    if (discriminant < 0) {
        return false;
    }

    // Calculate the time of collision
    float sqrtDiscriminant = sqrt(discriminant);
    float tEnter = (-dotProduct - sqrtDiscriminant) / VrelSquared;

    // Ensure collision happens within the time step g_dt
    if (tEnter >= 0.0f && tEnter <= g_dt) {
        firstTimeOfCollision = tEnter;
        return true; // Collision detected within the time step
    }

    return false; // No collision within the time step
}


int GetCellValue(int X, int Y)
{
    // Check if X and Y are within bounds
    if (X >= 0 && X < BINARY_MAP_WIDTH && Y >= 0 && Y < BINARY_MAP_HEIGHT) {
        // Return the value at the specified indices
        return BinaryCollisionArray[Y][X];
    }
    else {
        // Return 0 if out of bound
        return 0;
    }
}

int CheckHotSpot(float x, float y) {
    return GetCellValue(static_cast<int>(x), static_cast<int>(y)) == TYPE_OBJECT_COLLISION;
}

int CheckInstanceBinaryMapCollision(float PosX, float PosY, float scaleX, float scaleY) {
    int Flag;

    float leftX = PosX - scaleX / 2;
    float rightX = PosX + scaleX / 2;
    float topY = PosY + scaleY / 2;
    float bottomY = PosY - scaleY / 2;
    float middleY1 = PosY + scaleY / 4;
    float middleY2 = PosY - scaleY / 4;
    float middleX1 = PosX - scaleX / 4;
    float middleX2 = PosX + scaleX / 4;

    // Check left side
    if (CheckHotSpot(leftX, middleY1) || CheckHotSpot(leftX, middleY2)) {
        Flag |= COLLISION_LEFT;
    }

    // Check right side
    if (CheckHotSpot(rightX, middleY1) || CheckHotSpot(rightX, middleY2)) {
        Flag |= COLLISION_RIGHT;
    }

    // Check top side
    if (CheckHotSpot(middleX1, topY) || CheckHotSpot(middleX2, topY)) {
        Flag |= COLLISION_TOP;
    }

    // Check bottom side
    if (CheckHotSpot(middleX1, bottomY) || CheckHotSpot(middleX2, bottomY)) {
        Flag |= COLLISION_BOTTOM;
    }

    return Flag;
}