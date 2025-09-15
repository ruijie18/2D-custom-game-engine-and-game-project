/**
 * @file vector2d.cpp
 * @brief Implementation of the Vector2D class for 2D mathematical operations.
 *
 * This file contains the definitions for the Vector2D class methods,
 * including vector arithmetic, transformations, and utility functions.
 *
 * Author: Lewis (100%)
 */
#pragma once
#ifndef VECTOR2D_H
#define VECTOR2D_H

#include <cmath>

namespace Math2D {

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif



    class Vector2D {
    public:

        float x, y;

        // Constructor
        Vector2D(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

        // Functions
        float Length() const;
        float LengthSquare() const;
        float Distance(const Vector2D& other) const;
        float DistanceSquare(const Vector2D& other) const;
        Vector2D Normalize() const;
        float Dot(const Vector2D& other) const;
        float Cross(const Vector2D& other) const;
        Vector2D operator+(const Vector2D& other) const;
        Vector2D operator-(const Vector2D& other) const;
        Vector2D operator*(float scalar) const;
        Vector2D operator/(float scalar) const;
        Vector2D RotateRad(float angle) const;
        Vector2D RotateDeg(float angle) const;
        Vector2D Translate(const Vector2D& translation) const;
        bool operator==(const Vector2D& other) const;
        bool operator!=(const Vector2D& other) const;
        float GetX() const;
        float GetY() const;
        float& RefX();
        float& RefY();
    };
} // namespace Math2D

#endif // VECTOR2D_H
