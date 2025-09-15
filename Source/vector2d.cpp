/**
 * @file vector2d.cpp
 * @brief Implementation of the Vector2D class for 2D mathematical operations.
 *
 * This file contains the definitions for the Vector2D class methods,
 * including vector arithmetic, transformations, and utility functions.
 *
 * Author: Lewis (100%)
 */

#include <cmath>
#include "vector2d.h"

namespace Math2D {

    /**
     * @brief Computes the length (magnitude) of the vector.
     * @return The length of the vector.
     */
    float Vector2D::Length() const {
        return sqrt(x * x + y * y);
    }

    /**
     * @brief Computes the squared length of the vector.
     * @return The squared length of the vector.
     */
    float Vector2D::LengthSquare() const {
        return x * x + y * y;
    }

    /**
     * @brief Computes the distance to another vector.
     * @param other The vector to calculate the distance to.
     * @return The distance to the other vector.
     */
    float Vector2D::Distance(const Vector2D& other) const {
        return (*this - other).Length();
    }

    /**
     * @brief Computes the squared distance to another vector.
     * @param other The vector to calculate the squared distance to.
     * @return The squared distance to the other vector.
     */
    float Vector2D::DistanceSquare(const Vector2D& other) const {
        return (*this - other).LengthSquare();
    }

    /**
     * @brief Normalizes the vector (scales it to have a length of 1).
     * @return A normalized vector. If the length is zero, returns a zero vector.
     */
    Vector2D Vector2D::Normalize() const {
        float length = Length();
        return (length > 0) ? (*this / length) : Vector2D(0, 0);
    }

    /**
     * @brief Computes the dot product with another vector.
     * @param other The vector to compute the dot product with.
     * @return The dot product of the two vectors.
     */
    float Vector2D::Dot(const Vector2D& other) const {
        return x * other.x + y * other.y;
    }

    /**
     * @brief Computes the cross product (scalar in 2D) with another vector.
     * @param other The vector to compute the cross product with.
     * @return The scalar result of the cross product.
     */
    float Vector2D::Cross(const Vector2D& other) const {
        return x * other.y - y * other.x;
    }

    /**
     * @brief Adds another vector to the current vector.
     * @param other The vector to add.
     * @return A new vector that is the sum of the two vectors.
     */
    Vector2D Vector2D::operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);
    }

    /**
     * @brief Subtracts another vector from the current vector.
     * @param other The vector to subtract.
     * @return A new vector that is the difference of the two vectors.
     */
    Vector2D Vector2D::operator-(const Vector2D& other) const {
        return Vector2D(x - other.x, y - other.y);
    }

    /**
     * @brief Scales the vector by a scalar.
     * @param scalar The scalar to multiply by.
     * @return A new vector that is the result of the scalar multiplication.
     */
    Vector2D Vector2D::operator*(float scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }

    /**
     * @brief Divides the vector by a scalar.
     * @param scalar The scalar to divide by.
     * @return A new vector that is the result of the scalar division.
     *         Returns a zero vector if the scalar is zero.
     */
    Vector2D Vector2D::operator/(float scalar) const {
        if (scalar == 0) {
            return Vector2D(0, 0);
        }
        return Vector2D(x / scalar, y / scalar);
    }

    /**
     * @brief Rotates the vector by a given angle in radians.
     * @param angle The angle to rotate by, in radians.
     * @return A new vector that is the result of the rotation.
     */
    Vector2D Vector2D::RotateRad(float angle) const {
        float cosAngle = std::cos(angle);
        float sinAngle = std::sin(angle);
        return Vector2D(x * cosAngle - y * sinAngle, x * sinAngle + y * cosAngle);
    }

    /**
     * @brief Rotates the vector by a given angle in degrees.
     * @param angle The angle to rotate by, in degrees.
     * @return A new vector that is the result of the rotation.
     */
    Vector2D Vector2D::RotateDeg(float angle) const {
        return RotateRad(angle * (M_PI / 180.0f));
    }

    /**
     * @brief Translates the vector by another vector.
     * @param translation The vector to translate by.
     * @return A new vector that is the result of the translation.
     */
    Vector2D Vector2D::Translate(const Vector2D& translation) const {
        return Vector2D(x + translation.x, y + translation.y);
    }

    /**
     * @brief Compares the vector with another vector for equality.
     * @param other The vector to compare with.
     * @return True if the vectors are equal, false otherwise.
     */
    bool Vector2D::operator==(const Vector2D& other) const {
        return (x == other.x && y == other.y);
    }

    /**
     * @brief Compares the vector with another vector for inequality.
     * @param other The vector to compare with.
     * @return True if the vectors are not equal, false otherwise.
     */
    bool Vector2D::operator!=(const Vector2D& other) const {
        return !(*this == other);
    }

    /**
     * @brief Retrieves the x-coordinate of the vector.
     * @return The x-coordinate.
     */
    float Vector2D::GetX() const {
        return x;
    }

    /**
     * @brief Retrieves the y-coordinate of the vector.
     * @return The y-coordinate.
     */
    float Vector2D::GetY() const {
        return y;
    }

    /**
     * @brief Provides a modifiable reference to the x-coordinate.
     * @return A reference to the x-coordinate.
     */
    float& Vector2D::RefX() { 
        return x; 
    }

    /**
     * @brief Provides a modifiable reference to the y-coordinate.
     * @return A reference to the y-coordinate.
     */
    float& Vector2D::RefY() {
        return y; 
    }

} // namespace Math2D