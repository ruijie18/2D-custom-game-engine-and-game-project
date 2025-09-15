/**
 * @file vector3d.cpp
 * @brief Implementation of the Vector3D class for 3D mathematical operations.
 *
 * This file contains the definitions for the Vector3D class methods,
 * including vector arithmetic, transformations, and utility functions.
 *
 * Author: Lewis (100%)
 */

#include <cmath>
#include "vector3d.h"
#include <iostream>

namespace Math3D {

    /**
     * @brief Calculates the length (magnitude) of the vector.
     * @return The length of the vector as a float.
     */
    float Vector3D::Length() const {
        return sqrt(x * x + y * y + z * z);
    }

    /**
     * @brief Calculates the squared length of the vector.
     * @return The squared length of the vector as a float.
     */
    float Vector3D::LengthSquare() const {
        return x * x + y * y + z * z;
    }

    /**
     * @brief Calculates the distance to another vector.
     * @param other The other Vector3D to measure distance to.
     * @return The distance between the vectors as a float.
     */
    float Vector3D::Distance(const Vector3D& other) const {
        return (*this - other).Length();
    }

    /**
     * @brief Calculates the squared distance to another vector.
     * @param other The other Vector3D to measure distance to.
     * @return The squared distance between the vectors as a float.
     */
    float Vector3D::DistanceSquare(const Vector3D& other) const {
        return (*this - other).LengthSquare();
    }

    /**
     * @brief Normalizes the vector to a unit vector.
     * @return A normalized vector with the same direction.
     */
    Vector3D Vector3D::Normalize() const {
        float length = Length();
        return (length > 0) ? (*this / length) : Vector3D(0, 0, 0);
    }

    /**
     * @brief Calculates the dot product with another vector.
     * @param other The other Vector3D.
     * @return The dot product as a float.
     */
    float Vector3D::Dot(const Vector3D& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    /**
     * @brief Calculates the cross product with another vector.
     * @param other The other Vector3D.
     * @return The cross product as a new Vector3D.
     */
    Vector3D Vector3D::Cross(const Vector3D& other) const {
        return Vector3D(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    /**
     * @brief Adds two vectors.
     * @param other The other Vector3D to add.
     * @return The resulting Vector3D after addition.
     */
    Vector3D Vector3D::operator+(const Vector3D& other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }

    /**
     * @brief Subtracts one vector from another.
     * @param other The Vector3D to subtract.
     * @return The resulting Vector3D after subtraction.
     */
    Vector3D Vector3D::operator-(const Vector3D& other) const {
        return Vector3D(x - other.x, y - other.y, z - other.z);
    }

    /**
     * @brief Multiplies the vector by a scalar.
     * @param scalar The scalar value.
     * @return The scaled Vector3D.
     */
    Vector3D Vector3D::operator*(float scalar) const {
        return Vector3D(x * scalar, y * scalar, z * scalar);
    }

    /**
     * @brief Divides the vector by a scalar.
     * @param scalar The scalar value.
     * @return The resulting Vector3D after division.
     * @note If scalar is 0, the resulting vector is (0, 0, 0).
     */
    Vector3D Vector3D::operator/(float scalar) const {
        if (scalar == 0) {
            return Vector3D(0, 0, 0);
        }
        return Vector3D(x / scalar, y / scalar, z / scalar);
    }

    Vector3D& Vector3D::operator=(const Vector3D& other)
    {
        if (this != &other) { // Self-assignment check
            x = other.x;
            y = other.y;
            z = other.z;
        }
        return *this;
    }

    /**
     * @brief Rotates the vector around the X-axis.
     * @param angle The rotation angle in radians.
     * @return The rotated Vector3D.
     */
    Vector3D Vector3D::RotateX(float angle) const {
        float cosAngle = std::cos(angle);
        float sinAngle = std::sin(angle);
        return Vector3D(x, y * cosAngle - z * sinAngle, y * sinAngle + z * cosAngle);
    }

    /**
     * @brief Rotates the vector around the Y-axis.
     * @param angle The rotation angle in radians.
     * @return The rotated Vector3D.
     */
    Vector3D Vector3D::RotateY(float angle) const {
        float cosAngle = std::cos(angle);
        float sinAngle = std::sin(angle);
        return Vector3D(x * cosAngle + z * sinAngle, y, -x * sinAngle + z * cosAngle);
    }

    /**
     * @brief Rotates the vector around the Z-axis.
     * @param angle The rotation angle in radians.
     * @return The rotated Vector3D.
     */
    Vector3D Vector3D::RotateZ(float angle) const {
        float cosAngle = std::cos(angle);
        float sinAngle = std::sin(angle);
        return Vector3D(x * cosAngle - y * sinAngle, x * sinAngle + y * cosAngle, z);
    }

    /**
     * @brief Translates the vector by another vector.
     * @param translation The Vector3D used for translation.
     * @return The translated Vector3D.
     */
    Vector3D Vector3D::Translate(const Vector3D& translation) const {
        return Vector3D(x + translation.x, y + translation.y, z + translation.z);
    }

    /**
     * @brief Retrieves the X component of the vector.
     * @return The X component as a float.
     */
    float Vector3D::GetX() const {
        return x;
    }

    /**
     * @brief Retrieves the Y component of the vector.
     * @return The Y component as a float.
     */
    float Vector3D::GetY() const {
        return y;
    }

    /**
     * @brief Retrieves the Z component of the vector.
     * @return The Z component as a float.
     */
    float Vector3D::GetZ() const {
        return z;
    }

} // namespace Math3D
