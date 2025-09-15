/**
 * @file vector3d.cpp
 * @brief Implementation of the Vector3D class for 3D mathematical operations.
 *
 * This file contains the definitions for the Vector3D class methods,
 * including vector arithmetic, transformations, and utility functions.
 *
 * Author: Lewis (100%)
 */

#pragma once
#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <cmath>
#include <glm/vec3.hpp> 

namespace Math3D {

    class Vector3D {
    public:
        float x, y, z;

        // Constructor
        Vector3D(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
        static constexpr float EPSILON = 1e-5f; //Used to ensure no floating point precision errors

        // Functions
        operator glm::vec3() const {
            return glm::vec3(x, y, z);
        }
        float Length() const;
        float LengthSquare() const;
        float Distance(const Vector3D& other) const;
        float DistanceSquare(const Vector3D& other) const;
        Vector3D Normalize() const;
        float Dot(const Vector3D& other) const;
        Vector3D Cross(const Vector3D& other) const;
        Vector3D operator+(const Vector3D& other) const;
        Vector3D operator-(const Vector3D& other) const;
        Vector3D operator*(float scalar) const;
        Vector3D operator/(float scalar) const;
        Vector3D& operator=(const Vector3D& other);
        Vector3D RotateX(float angle) const;
        Vector3D RotateY(float angle) const;
        Vector3D RotateZ(float angle) const;
        Vector3D Translate(const Vector3D& translation) const;
        float GetX() const;
        float GetY() const;
        float GetZ() const;
    };
} // namespace Math3D

#endif // VECTOR3D_H
