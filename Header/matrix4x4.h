/**
 * @file matrix4x4.h
 * @brief Implementation of a 4x4 matrix class for 3D transformations.
 *
 * This file contains the implementation of the Matrix4x4 class, which is
 * part of the Matrix4 namespace. It includes various utility functions
 * for matrix operations such as addition, multiplication, determinant
 * calculation, and transformations (scaling, rotation, and translation).
 *
 * Author: Lewis (100%)
 */

#pragma once
#ifndef MATRIX4X4_H
#define MATRIX4X4_H

#include "vector3d.h"
#include <cmath>

namespace Matrix4 {

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

    using namespace Math3D;

    static constexpr float EPSILON = 1e-5f; //Used to ensure no floating point precision errors

    class Matrix4x4 {
    private:
        float elements[4][4];

    public:
        // Constructors
        Matrix4x4(); // Default constructor
        Matrix4x4(float diag); // Constructor for diagonal matrix
        Matrix4x4(float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33); // Constructor with specific values

        // Setters
        void SetToZero(); // Set all elements to zero
        void Set(float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33); // Set specific values
        void SetElement(int row, int col, float value); // Set specific element

        // Getters
        float GetElement(int row, int col) const; // Get specific element

        // Static methods to create special matrices
        static Matrix4x4 CreateIdentity(); // Create identity matrix
        static Matrix4x4 CreateZero(); // Create zero matrix
        static Matrix4x4 CreateTranslation(float x, float y, float z); // Create translation matrix
        static Matrix4x4 CreateScaling(float sx, float sy, float sz); // Create scaling matrix
        static Matrix4x4 CreateRotationX(float angle); // Create rotation matrix around X-axis
        static Matrix4x4 CreateRotationY(float angle); // Create rotation matrix around Y-axis
        static Matrix4x4 CreateRotationZ(float angle); // Create rotation matrix around Z-axis

        // Matrix operations
        Matrix4x4 operator+(const Matrix4x4& other) const; // Addition
        Matrix4x4 operator-(const Matrix4x4& other) const; // Subtraction
        Matrix4x4 operator*(const Matrix4x4& other) const; // Matrix multiplication
        Matrix4x4 operator*(float scalar) const; // Scalar multiplication
        Vector3D operator*(const Vector3D& vec) const; // Multiply with Vector3D

        // Transpose
        Matrix4x4 Transpose() const; // Transpose of the matrix

        // Comparison operators
        bool operator==(const Matrix4x4& other) const; // Equality
        bool operator!=(const Matrix4x4& other) const; // Inequality

        float Determinant() const;
        float Minor(int row, int col) const;

        // Inverse
        Matrix4x4 Inverse() const; // Inverse of the matrix
    };

} // namespace Matrix4

#endif // MATRIX4X4_H
