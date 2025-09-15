/**
 * @file matrix3x3.cpp
 * @brief Implementation of a 3x3 matrix class for 2D transformations.
 *
 * This file contains the implementation of the Matrix3x3 class, which is
 * part of the Matrix3 namespace. It includes various utility functions
 * for matrix operations such as addition, multiplication, determinant
 * calculation, and transformations (scaling, rotation, and translation).
 *
 * Author: Lewis (100%)
 */

#include <cmath>
#include "matrix3x3.h"
#include "vector2d.h"

namespace Matrix3 {

    using namespace Math2D;

    /**
     * @class Matrix3x3
     * @brief Represents a 3x3 matrix for 2D geometric transformations.
     */

     /**
      * @brief Default constructor. Initializes the matrix to a zero matrix.
      */

    // Constructors
    Matrix3x3::Matrix3x3() {
        SetToZero();
    }

    /**
     * @brief Constructor to initialize the matrix as a diagonal matrix.
     * @param diag The value to set on the diagonal elements.
     */
    Matrix3x3::Matrix3x3(float diag) {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                elements[i][j] = (i == j) ? diag : 0.0f;
            }
        }
    }

    /**
     * @brief Constructor to initialize the matrix with specific elements.
     * @param m00 Element at (0, 0), m01 Element at (0, 1), ..., m22 Element at (2, 2).
     */
    Matrix3x3::Matrix3x3(float m00, float m01, float m02,
        float m10, float m11, float m12,
        float m20, float m21, float m22) {
        Set(m00, m01, m02, m10, m11, m12, m20, m21, m22);
    }

    /**
    * @brief Sets all elements of the matrix to zero.
    */
    void Matrix3x3::SetToZero() {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                elements[i][j] = 0.0f;
            }
        }
    }

    /**
     * @brief Sets the elements of the matrix to the specified values.
     * @param m00 Element at (0, 0), m01 Element at (0, 1), ..., m22 Element at (2, 2).
     */
    void Matrix3x3::Set(float m00, float m01, float m02,
    float m10, float m11, float m12,
    float m20, float m21, float m22) {
    elements[0][0] = m00; elements[0][1] = m01; elements[0][2] = m02;
    elements[1][0] = m10; elements[1][1] = m11; elements[1][2] = m12;
    elements[2][0] = m20; elements[2][1] = m21; elements[2][2] = m22;
    }

    /**
     * @brief Sets a specific element in the matrix.
     * @param row Row index (0-2).
     * @param col Column index (0-2).
     * @param value The value to set.
     */
    void Matrix3x3::SetElement(int row, int col, float value) {
        if (row >= 0 && row < 3 && col >= 0 && col < 3) {
            elements[row][col] = value;
        }
    }

    /**
     * @brief Retrieves a specific element from the matrix.
     * @param row Row index (0-2).
     * @param col Column index (0-2).
     * @return The value of the specified element.
     */    
    float Matrix3x3::GetElement(int row, int col) const {
        if (row >= 0 && row < 3 && col >= 0 && col < 3) {
            return elements[row][col];
        }
        return 0.0f; // Return 0 for out-of-bounds access
    }

    /**
     * @brief Creates and returns an identity matrix.
     * @return A 3x3 identity matrix.
     */
    Matrix3x3 Matrix3x3::CreateIdentity() {
        return Matrix3x3(1.0f);
    }

    /**
     * @brief Creates 3x3 zero matrix.
     * @return A 3x3 zero matrix.
     */    
    Matrix3x3 Matrix3x3::CreateZero() {
        return Matrix3x3();
    }

    /**
     * @brief Adds two matrices element-wise.
     *
     * @param other The matrix to add.
     * @return Matrix3x3 The result of the addition.
     */
    Matrix3x3 Matrix3x3::operator+(const Matrix3x3& other) const {
        Matrix3x3 result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result.elements[i][j] = this->elements[i][j] + other.elements[i][j];
            }
        }
        return result;
    }

    /**
     * @brief Subtracts another matrix from this matrix element-wise.
     *
     * @param other The matrix to subtract.
     * @return Matrix3x3 The result of the subtraction.
     */
    Matrix3x3 Matrix3x3::operator-(const Matrix3x3& other) const {
        Matrix3x3 result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result.elements[i][j] = this->elements[i][j] - other.elements[i][j];
            }
        }
        return result;
    }

    /**
     * @brief Multiplies two matrices.
     *
     * @param other The matrix to multiply by.
     * @return Matrix3x3 The result of the multiplication.
     */    
    Matrix3x3 Matrix3x3::operator*(const Matrix3x3& other) const {
        Matrix3x3 result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result.elements[i][j] = 0;
                for (int k = 0; k < 3; ++k) {
                    result.elements[i][j] += this->elements[i][k] * other.elements[k][j];
                }
            }
        }
        return result;
    }

    /**
     * @brief Multiplies the matrix by a scalar value.
     *
     * @param scalar The scalar value to multiply by.
     * @return Matrix3x3 The result of the scalar multiplication.
     */
    Matrix3x3 Matrix3x3::operator*(float scalar) const {
        Matrix3x3 result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result.elements[i][j] = this->elements[i][j] * scalar;
            }
        }
        return result;
    }
    /**
     * @brief Multiplies the matrix by a 2D vector.
     *
     * @param vec The vector to multiply by.
     * @return Vector2D The resulting vector after multiplication.
     */
    Vector2D Matrix3x3::operator*(const Vector2D& vec) const {
        // Treat vec as a 3D vector (x, y, 1)
        float x = elements[0][0] * vec.x + elements[0][1] * vec.y + elements[0][2];
        float y = elements[1][0] * vec.x + elements[1][1] * vec.y + elements[1][2];

        return Vector2D(x, y);
    }

    /**
     * @brief Creates a translation matrix.
     *
     * @param x Translation along the x-axis.
     * @param y Translation along the y-axis.
     * @return Matrix3x3 A translation matrix.
     */
    Matrix3x3 Matrix3x3::CreateTranslation(float x, float y) {
        return Matrix3x3(1.0f, 0.0f, x,
            0.0f, 1.0f, y,
            0.0f, 0.0f, 1.0f);
    }

    /**
     * @brief Creates a scaling matrix.
     *
     * @param sx Scaling factor along the x-axis.
     * @param sy Scaling factor along the y-axis.
     * @return Matrix3x3 A scaling matrix.
     */
    Matrix3x3 Matrix3x3::CreateScaling(float sx, float sy) {
        return Matrix3x3(sx, 0.0f, 0.0f,
            0.0f, sy, 0.0f,
            0.0f, 0.0f, 1.0f);
    }

    /**
     * @brief Creates a rotation matrix using an angle in radians.
     *
     * @param angle Angle in radians for rotation.
     * @return Matrix3x3 A rotation matrix.
     */
    Matrix3x3 Matrix3x3::CreateRotationRad(float angle) {
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);
        return Matrix3x3(cosA, -sinA, 0.0f,
            sinA, cosA, 0.0f,
            0.0f, 0.0f, 1.0f);
    }

    /**
     * @brief Creates a rotation matrix using an angle in degrees.
     *
     * @param angle Angle in degrees for rotation.
     * @return Matrix3x3 A rotation matrix.
     */
    Matrix3x3 Matrix3x3::CreateRotationDeg(float angle) {
        return CreateRotationRad(angle * (M_PI / 180.0f));
    }

    /**
     * @brief Transposes the matrix (swaps rows and columns).
     *
     * @return Matrix3x3 The transposed matrix.
     */
    Matrix3x3 Matrix3x3::Transpose() const {
        Matrix3x3 result;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result.elements[j][i] = this->elements[i][j];
            }
        }
        return result;
    }
    /**
     * @brief Calculates the determinant of the matrix.
     *
     * @return float The determinant value.
     */
    float Matrix3x3::Determinant() const {
        return elements[0][0] * (elements[1][1] * elements[2][2] - elements[1][2] * elements[2][1]) -
            elements[0][1] * (elements[1][0] * elements[2][2] - elements[1][2] * elements[2][0]) +
            elements[0][2] * (elements[1][0] * elements[2][1] - elements[1][1] * elements[2][0]);
    }

    /**
     * @brief Checks if two matrices are approximately equal.
     *
     * @param other The matrix to compare with.
     * @return true If the matrices are approximately equal.
     * @return false Otherwise.
     */
    bool Matrix3x3::operator==(const Matrix3x3& other) const {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (std::fabs(elements[i][j] != other.elements[i][j]) > EPSILON) {
                    return false;
                }
            }
        }
        return true;
    }

    /**
     * @brief Checks if two matrices are not approximately equal.
     *
     * @param other The matrix to compare with.
     * @return true If the matrices are not approximately equal.
     * @return false Otherwise.
     */
    bool Matrix3x3::operator!=(const Matrix3x3& other) const {
        return !(*this == other);
    }

    /**
     * @brief Computes the inverse of the matrix if it exists.
     *
     * @param out The resulting inverse matrix.
     * @param determinant Optionally store the determinant here (null if unused).
     */
    void Matrix3x3::Inverse(Matrix3x3& out, float*) const {
        float det = this->Determinant();

        if (det == 0.0f) {
            out.SetToZero();  // Return a zero matrix if inversion fails
            return;
        }

        out.elements[0][0] = (elements[1][1] * elements[2][2] - elements[1][2] * elements[2][1]) / det;
        out.elements[0][1] = (elements[0][2] * elements[2][1] - elements[0][1] * elements[2][2]) / det;
        out.elements[0][2] = (elements[0][1] * elements[1][2] - elements[0][2] * elements[1][1]) / det;
        out.elements[1][0] = (elements[1][2] * elements[2][0] - elements[1][0] * elements[2][2]) / det;
        out.elements[1][1] = (elements[0][0] * elements[2][2] - elements[0][2] * elements[2][0]) / det;
        out.elements[1][2] = (elements[0][2] * elements[1][0] - elements[0][0] * elements[1][2]) / det;
        out.elements[2][0] = (elements[1][0] * elements[2][1] - elements[1][1] * elements[2][0]) / det;
        out.elements[2][1] = (elements[0][1] * elements[2][0] - elements[0][0] * elements[2][1]) / det;
        out.elements[2][2] = (elements[0][0] * elements[1][1] - elements[0][1] * elements[1][0]) / det;
    }

} //namespace Matrix3
