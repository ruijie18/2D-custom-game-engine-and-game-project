/**
 * @file Shader.h
 * @brief Implementation of the `HUShader` class for managing OpenGL shader programs.
 *
 * This header file implements the `HUShader` class, which provides functions for compiling, linking, using,
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


#ifndef HUSHADER_H
#define HUSHADER_H


#include <GL/glew.h>  
#include <glm/glm.hpp>
#include <string>
#include <vector>


class HUShader {
public:
    HUShader() : pgm_handle(0), is_linked(GL_FALSE) {}
    GLboolean CompileShaderFromString(GLenum shader_type, std::string const& shader_src);
    GLboolean Link();
    void Use();
    void UnUse();
    GLboolean Validate();
    GLuint GetHandle() const;
    GLboolean IsLinked() const;
    std::string GetLog() const;

    void cleanup() {
        if (pgm_handle != 0) {
            glDeleteProgram(pgm_handle);
            pgm_handle = 0;
        }
    }

private:
    enum ShaderType {
        VERTEX_SHADER = GL_VERTEX_SHADER,
        FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
    };

    GLuint pgm_handle = 0;  
    GLboolean is_linked = GL_FALSE; 
    std::string log_string; 
};
#endif /* HUSHADER_H */
