/**
 * @file Shader.cpp
 * @brief Implementation of the `HUShader` class for managing OpenGL shader programs.
 *
 * This source file implements the `HUShader` class, which provides functions for compiling, linking, using,
 * and validating OpenGL shader programs. It supports both vertex and fragment shaders and handles the lifecycle
 * of shader programs, including error reporting during compilation and linking.
 *
 * Key Features:
 * - **Shader Compilation**: Compiles vertex and fragment shaders from source code strings.
 * - **Shader Linking**: Links compiled shaders into a complete shader program for rendering.
 * - **Shader Program Validation**: Validates the shader program for compatibility with the current OpenGL context.
 * - **Error Handling**: Provides detailed logs for shader compilation, linking, and validation errors.
 * - **Shader Usage**: Activates and deactivates the shader program for rendering.
 *
 * Key Functions:
 * - `CompileShaderFromString`: Compiles a shader from source code.
 * - `Link`: Links the compiled shaders into a program.
 * - `Use`: Activates the shader program for use.
 * - `UnUse`: Deactivates the current shader program.
 * - `Validate`: Validates the shader program.
 * - `GetLog`: Retrieves error logs for shader compilation and linking.
 * - `cleanup`: Frees resources associated with the shader program.
 *
 * Author: Jasper (100%)
 */


#include "Shader.h"
#include <iostream>
#include <vector>
#include <sstream>

// @brief Compiles a shader from the provided source string.
// @param shader_type The type of shader to compile (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER).
// @param shader_src The shader source code as a string.
// @return GL_TRUE if compilation is successful, otherwise GL_FALSE.
GLboolean HUShader::CompileShaderFromString(GLenum shader_type, const std::string& shader_src) {
    // Create program if necessary
    if (pgm_handle <= 0) {
        pgm_handle = glCreateProgram();
        if (pgm_handle == 0) {
            log_string = "Cannot create program handle";
            return GL_FALSE;
        }
    }

    // Create shader handle
    GLuint shader_handle = 0;
    if (shader_type == GL_VERTEX_SHADER || shader_type == GL_FRAGMENT_SHADER) {
        shader_handle = glCreateShader(shader_type);
    }
    else {
        log_string = "Incorrect shader type";
        return GL_FALSE;
    }

    // Load and compile the shader
    const char* shader_code[] = { shader_src.c_str() };
    glShaderSource(shader_handle, 1, shader_code, nullptr);
    glCompileShader(shader_handle);

    // Check compilation status
    GLint compile_status = GL_FALSE;
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compile_status);

    if (compile_status == GL_FALSE) {
        log_string = (shader_type == GL_VERTEX_SHADER) ? "Vertex shader compilation failed\n" : "Fragment shader compilation failed\n";

        // Get error log
        GLint log_length = 0;
        glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &log_length);

        if (log_length > 0) {
            std::vector<GLchar> log(log_length);
            glGetShaderInfoLog(shader_handle, log_length, nullptr, log.data());
            log_string.append(log.begin(), log.end());
        }

        return GL_FALSE;
    }

    // Attach shader to program
    glAttachShader(pgm_handle, shader_handle);
    return GL_TRUE;
}

// @brief Links the compiled shaders into a program.
// @return GL_TRUE if linking is successful, otherwise GL_FALSE.
GLboolean HUShader::Link() {
    if (is_linked) {
        return GL_TRUE;
    }
    if (pgm_handle <= 0) {
        return GL_FALSE;
    }

    glLinkProgram(pgm_handle);

    // Check link status
    GLint link_status = GL_FALSE;
    glGetProgramiv(pgm_handle, GL_LINK_STATUS, &link_status);

    if (link_status == GL_FALSE) {
        log_string = "Failed to link shader program\n";

        // Get error log
        GLint log_length = 0;
        glGetProgramiv(pgm_handle, GL_INFO_LOG_LENGTH, &log_length);

        if (log_length > 0) {
            std::vector<GLchar> log(log_length);
            glGetProgramInfoLog(pgm_handle, log_length, nullptr, log.data());
            log_string.append(log.begin(), log.end());
        }

        return GL_FALSE;
    }

    is_linked = GL_TRUE;
    return GL_TRUE;
}

// @brief Activates the shader program for use in rendering.
void HUShader::Use() {
    if (pgm_handle > 0 && is_linked) {
        glUseProgram(pgm_handle);
    }
}

// @brief Deactivates the current shader program.
void HUShader::UnUse() {
    glUseProgram(0);
}

// @brief Validates the shader program for the current OpenGL context.
// @return GL_TRUE if the shader program is valid, otherwise GL_FALSE.
GLboolean HUShader::Validate() {
    if (pgm_handle <= 0 || !is_linked) {
        return GL_FALSE;
    }

    glValidateProgram(pgm_handle);

    // Check validation status
    GLint validate_status = GL_FALSE;
    glGetProgramiv(pgm_handle, GL_VALIDATE_STATUS, &validate_status);

    if (validate_status == GL_FALSE) {
        log_string = "Failed to validate shader program for current OpenGL context\n";

        // Get error log
        GLint log_length = 0;
        glGetProgramiv(pgm_handle, GL_INFO_LOG_LENGTH, &log_length);

        if (log_length > 0) {
            std::vector<GLchar> log(log_length);
            glGetProgramInfoLog(pgm_handle, log_length, nullptr, log.data());
            log_string.append(log.begin(), log.end());
        }

        return GL_FALSE;
    }

    return GL_TRUE;
}

// @brief Retrieves the handle of the shader program.
// @return The shader program handle.
GLuint HUShader::GetHandle() const {
    return pgm_handle;
}

// @brief Checks if the shader program has been successfully linked.
// @return GL_TRUE if the program is linked, otherwise GL_FALSE.
GLboolean HUShader::IsLinked() const {
    return is_linked;
}

// @brief Retrieves the log information for the shader program, including compilation and linking errors.
// @return A string containing the log information.
std::string HUShader::GetLog() const {
    return log_string;
}
