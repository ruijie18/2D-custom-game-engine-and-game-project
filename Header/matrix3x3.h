/**
 * @file matrix3x3.h
 * @brief Implementation of a 3x3 matrix class for 2D transformations.
 *
 * This file contains the implementation of the Matrix3x3 class, which is
 * part of the Matrix3 namespace. It includes various utility functions
 * for matrix operations such as addition, multiplication, determinant
 * calculation, and transformations (scaling, rotation, and translation).
 *
 * Author: Lewis (100%)
 */
#pragma once
#ifndef MATRIX3X3_H
#define MATRIX3X3_H

#include "vector2d.h"

namespace Matrix3 {

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
    using namespace Math2D;

    static constexpr float EPSILON = 1e-5f; // Used to ensure no floating point precision errors

    class Matrix3x3 {
    private:
        float elements[3][3];

    public:
        // Constructors
        Matrix3x3(); // Default constructor
        Matrix3x3(float diag); // Constructor for diagonal matrix
        Matrix3x3(float m00, float m01, float m02,
            float m10, float m11, float m12,
            float m20, float m21, float m22); // Constructor with specific values

        // Setters
        void SetToZero(); // Set all elements to zero
        void Set(float m00, float m01, float m02,
            float m10, float m11, float m12,
            float m20, float m21, float m22); // Set specific values
        void SetElement(int row, int col, float value); // Set specific element

        // Getters
        float GetElement(int row, int col) const; // Get specific element

        // Static methods to create special matrices
        static Matrix3x3 CreateIdentity(); // Create identity matrix
        static Matrix3x3 CreateZero(); // Create zero matrix
        static Matrix3x3 CreateTranslation(float x, float y); // Create translation matrix
        static Matrix3x3 CreateScaling(float sx, float sy); // Create scaling matrix
        static Matrix3x3 CreateRotationRad(float angle); // Create rotation matrix (radians)
        static Matrix3x3 CreateRotationDeg(float angle); // Create rotation matrix (degrees)

        // Matrix operations
        Matrix3x3 operator+(const Matrix3x3& other) const; // Addition
        Matrix3x3 operator-(const Matrix3x3& other) const; // Subtraction
        Matrix3x3 operator*(const Matrix3x3& other) const; // Matrix multiplication
        Matrix3x3 operator*(float scalar) const; // Scalar multiplication
        Vector2D operator*(const Vector2D& vec) const; // Multiply with Vector2D

        // Transpose 
        Matrix3x3 Transpose() const; // Transpose of the matrix

        float Determinant() const;//Determninant

        // Comparison operators
        bool operator==(const Matrix3x3& other) const; // Equality
        bool operator!=(const Matrix3x3& other) const; // Inequality

        // Inverse
        void Inverse(Matrix3x3& out, float* determinant) const; // Inverse of the matrix
    };

} // namespace Matrix3

#endif // MATRIX3X3_H
