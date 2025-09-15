/**
 * @file matrix4x4.cpp
 * @brief Implementation of a 4x4 matrix class for 3D transformations.
 *
 * This file contains the implementation of the Matrix4x4 class, which is
 * part of the Matrix4 namespace. It includes various utility functions
 * for matrix operations such as addition, multiplication, determinant
 * calculation, and transformations (scaling, rotation, and translation).
 *
 * Author: Lewis (100%)
 */

#include <cmath>
#include "matrix4x4.h"
#include "matrix3x3.h"
#include "vector3d.h"

namespace Matrix4 {

    using namespace Math3D;


    /**
    * @brief Default constructor that initializes the matrix to a zero matrix.
    */
    Matrix4x4::Matrix4x4() {
        SetToZero();
    }

    /**
     * @brief Constructs a diagonal matrix with the given value.
     * @param diag The value to set along the diagonal elements.
     */    Matrix4x4::Matrix4x4(float diag) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                elements[i][j] = (i == j) ? diag : 0.0f;
            }
        }
    }

     /**
     * @brief Constructs a matrix with specified element values.
     * @param m00..m33 The individual elements of the 4x4 matrix.
     */
    Matrix4x4::Matrix4x4(float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33) {
        Set(m00, m01, m02, m03,
            m10, m11, m12, m13,
            m20, m21, m22, m23,
            m30, m31, m32, m33);
    }

    /**
    * @brief Sets all elements of the matrix to zero.
    */
    void Matrix4x4::SetToZero() {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                elements[i][j] = 0.0f;
            }
        }
    }

    /**
    * @brief Sets the values of the matrix elements.
    * @param m00..m33 The individual elements of the 4x4 matrix.
    */

    void Matrix4x4::Set(float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33) {
        elements[0][0] = m00; elements[0][1] = m01; elements[0][2] = m02; elements[0][3] = m03;
        elements[1][0] = m10; elements[1][1] = m11; elements[1][2] = m12; elements[1][3] = m13;
        elements[2][0] = m20; elements[2][1] = m21; elements[2][2] = m22; elements[2][3] = m23;
        elements[3][0] = m30; elements[3][1] = m31; elements[3][2] = m32; elements[3][3] = m33;
    }

    /**
     * @brief Sets a specific element in the matrix.
     * @param row The row index (0-3).
     * @param col The column index (0-3).
     * @param value The value to assign to the element.
     */
    void Matrix4x4::SetElement(int row, int col, float value) {
        if (row >= 0 && row < 4 && col >= 0 && col < 4) {
            elements[row][col] = value;
        }
    }

    /**
     * @brief Gets the value of a specific element in the matrix.
     * @param row The row index (0-3).
     * @param col The column index (0-3).
     * @return The value of the specified element, or 0 if out of bounds.
     */
    float Matrix4x4::GetElement(int row, int col) const {
        if (row >= 0 && row < 4 && col >= 0 && col < 4) {
            return elements[row][col];
        }
        return 0.0f; // Return 0 for out-of-bounds access
    }

    /**
     * @brief Creates an identity matrix.
     * @return A 4x4 identity matrix.
     */
    Matrix4x4 Matrix4x4::CreateIdentity() {
        return Matrix4x4(1.0f);
    }

    /**
     * @brief Creates a zero matrix.
     * @return A 4x4 zero matrix.
     */
    Matrix4x4 Matrix4x4::CreateZero() {
        return Matrix4x4();
    }

    /**
     * @brief Adds two matrices.
     * @param other The matrix to add.
     * @return A new matrix that is the sum of the two matrices.
     */
    Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.elements[i][j] = this->elements[i][j] + other.elements[i][j];
            }
        }
        return result;
    }

    /**
     * @brief Subtracts one matrix from another.
     * @param other The matrix to subtract.
     * @return A new matrix that is the difference between the two matrices.
     */
    Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.elements[i][j] = this->elements[i][j] - other.elements[i][j];
            }
        }
        return result;
    }

    /**
     * @brief Multiplies the matrix by a scalar.
     * @param scalar The scalar value to multiply by.
     * @return A new matrix that is the result of the scalar multiplication.
     */
    Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.elements[i][j] = 0;
                for (int k = 0; k < 4; ++k) {
                    result.elements[i][j] += this->elements[i][k] * other.elements[k][j];
                }
            }
        }
        return result;
    }

    // Scalar multiplication
    Matrix4x4 Matrix4x4::operator*(float scalar) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.elements[i][j] = this->elements[i][j] * scalar;
            }
        }
        return result;
    }

    /**
     * @brief Multiplies the matrix by another matrix.
     * @param other The matrix to multiply with.
     * @return A new matrix that is the result of the matrix multiplication.
     */
    Vector3D Matrix4x4::operator*(const Vector3D& vec) const {
        // Treat vec as a 4D vector (x, y, z, 1)
        float x = elements[0][0] * vec.x + elements[0][1] * vec.y + elements[0][2] * vec.z + elements[0][3];
        float y = elements[1][0] * vec.x + elements[1][1] * vec.y + elements[1][2] * vec.z + elements[1][3];
        float z = elements[2][0] * vec.x + elements[2][1] * vec.y + elements[2][2] * vec.z + elements[2][3];

        return Vector3D(x, y, z);
    }

    /**
     * @brief Creates a translation transformation matrix.
     * @param translation The vector containing translation values for x, y, and z axes.
     * @return A translation transformation matrix.
     */
    Matrix4x4 Matrix4x4::CreateTranslation(float x, float y, float z) {
        return Matrix4x4(1.0f, 0.0f, 0.0f, x,
            0.0f, 1.0f, 0.0f, y,
            0.0f, 0.0f, 1.0f, z,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    /**
     * @brief Creates a scaling transformation matrix.
     * @param scale The vector containing scale factors for x, y, and z axes.
     * @return A scaling transformation matrix.
     */
    Matrix4x4 Matrix4x4::CreateScaling(float sx, float sy, float sz) {
        return Matrix4x4(sx, 0.0f, 0.0f, 0.0f,
            0.0f, sy, 0.0f, 0.0f,
            0.0f, 0.0f, sz, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    /**
     * @brief Creates a rotation transformation matrix around the X-axis.
     * @param angle The angle in radians to rotate.
     * @return A rotation transformation matrix.
     */
    Matrix4x4 Matrix4x4::CreateRotationX(float angle) {
        float cosA = cos(angle);
        float sinA = sin(angle);
        return Matrix4x4(1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, cosA, -sinA, 0.0f,
            0.0f, sinA, cosA, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    /**
     * @brief Creates a rotation transformation matrix around the Y-axis.
     * @param angle The angle in radians to rotate.
     * @return A rotation transformation matrix.
     */
    Matrix4x4 Matrix4x4::CreateRotationY(float angle) {
        float cosA = cos(angle);
        float sinA = sin(angle);
        return Matrix4x4(cosA, 0.0f, sinA, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -sinA, 0.0f, cosA, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    /**
     * @brief Creates a rotation transformation matrix around the Z-axis.
     * @param angle The angle in radians to rotate.
     * @return A rotation transformation matrix.
     */
    Matrix4x4 Matrix4x4::CreateRotationZ(float angle) {
        float cosA = cos(angle);
        float sinA = sin(angle);
        return Matrix4x4(cosA, -sinA, 0.0f, 0.0f,
            sinA, cosA, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);
    }

    /**
     * @brief Transposes the matrix.
     * @return A new matrix that is the transpose of the current matrix.
     */
    Matrix4x4 Matrix4x4::Transpose() const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.elements[j][i] = this->elements[i][j];
            }
        }
        return result;
    }

    // Equality operator
    bool Matrix4x4::operator==(const Matrix4x4& other) const {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (elements[i][j] != other.elements[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }

    // Inequality operator
    bool Matrix4x4::operator!=(const Matrix4x4& other) const {
        return !(*this == other);
    }

    /**
     * @brief Calculates the determinant of the matrix.
     * @return The determinant of the matrix.
     */
    float Matrix4x4::Determinant() const {
        float det = 0.0f;
        for (int j = 0; j < 4; ++j) {
            det += (j % 2 == 0 ? 1 : -1) * elements[0][j] * Minor(0, j);
        }
        return det;
    }

    // Minor
    float Matrix4x4::Minor(int row, int col) const {
        Matrix3::Matrix3x3 minorMatrix;
        int minorRow = 0, minorCol;

        for (int i = 0; i < 4; ++i) {
            if (i == row) continue; // Skip the row
            minorCol = 0;
            for (int j = 0; j < 4; ++j) {
                if (j == col) continue; // Skip the column
                minorMatrix.SetElement(minorRow, minorCol, elements[i][j]);
                minorCol++;
            }
            minorRow++;
        }
        return minorMatrix.Determinant(); // Return the determinant of the 3x3 minor matrix
    }

    /**
     * @brief Inverts the matrix if possible.
     * @return A new matrix that is the inverse of the current matrix.
     * @throws std::runtime_error if the matrix is not invertible (determinant is zero).
     */
    Matrix4x4 Matrix4x4::Inverse() const {
        float det = this->Determinant();
        if (det == 0) return Matrix4x4(); // Return zero matrix if not invertible

        Matrix4x4 adjugate;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                adjugate.SetElement(i, j, ((i + j) % 2 == 0 ? 1 : -1) * Minor(j, i)); // Note: Transpose
            }
        }

        return adjugate * (1.0f / det); // Multiply adjugate by 1/det
    }

} // namespace Matrix4
