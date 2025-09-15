/**
 * @file Graphics.h
 * @brief Implementation of the HUGraphics module for managing OpenGL rendering of various geometric shapes and textures.
 *
 * This header file defines the functions and methods of the `HUGraphics` class, which is responsible for
 * managing OpenGL buffers, shaders, and models in a 2D/3D rendering pipeline. It includes utilities for rendering
 * points, lines, triangles, circles, rectangles, and textured meshes. The file also handles animation for sprite-based
 * models, as well as the setup and cleanup of OpenGL resources.
 *
 * Key Features:
 * - **Model Creation**: Provides functions to create various shapes such as points, lines, rectangles, triangles, and circles.
 * - **Texturing and Animation**: Supports texture mapping and sprite sheet-based animations for 2D models.
 * - **Shader Management**: Loads and compiles vertex and fragment shaders for rendering different types of objects.
 * - **OpenGL Resource Management**: Handles OpenGL buffer objects (VBO, VAO, EBO) for efficient rendering.
 * - **Outline Drawing**: Includes functionality to render outlines for models, supporting debugging and visualization.
 * - **Model Cleanup**: Ensures proper cleanup of OpenGL resources by clearing models and outlines when no longer needed.
 *
 * Utility Functions:
 * - `update_animation_model`: Updates animation frame based on elapsed time for sprite-based models.
 * - `setup_shdrpgm`: Compiles and links shaders for rendering models.
 * - `points_model`, `lines_model`, `rectangle_model`, `triangle_model`, `circle_model`, `texture_mesh`: Functions to create and render various shapes and objects.
 * - `clearOutlineModels`: Clears and deallocates memory for outline models, ensuring resources are freed.
 *
 * Author: Jasper (85%)
 * Co-Author: Rui Jie (10%)
 * Co-Author: Che Ee (5%)
 */


#pragma once
#ifndef HUGraphics_H
#define HUGraphics_H
#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Shader.h"
#include "vector2d.h"
#include "SystemsManager.h"
//#include "GlobalVariables.h"
#include "AssetsManager.h"



class HUGraphics : public System
{
public:
    HUGraphics();
    ~HUGraphics() {}

    void Init()override;
    void Update(double deltaTime)override;
    static void draw();
    static void cleanup();
    static void print_specs();

    std::unordered_map<std::string, GLuint> textTextureCache;
    void ClearTextTextureCache() {
        // Clear the cache
        textTextureCache.clear();
    }


    class GLModel {
    public:
        // 
        //  transparency for rendering (0.0 = fully transparent, 1.0 = fully opaque)
        float alpha = 1.0f;

        // Variables for fade-out effect
        bool isFading = false;            // Is the model currently fading out?
        float fadeTimer = 0.0f;           // Remaining time for the fade-out effect
        float original
            = 0.0f; // Total duration of the fade-out effect

        // Variables for fade-in effect
        bool isFadingIn = false;          // Is the model currently fading in?
        float fadeInTimer = 0.0f;         // Remaining time for the fade-in effect
        float fadeDuration = 0.0f;      // Total duration of the fade-in effect
        bool flipTextureHorizontally = false; //to flip assets
        bool textChanged = false;

        GLModel() = default;
        ~GLModel() { }


        GLuint vaoid = 0;
        GLuint vbo_hdl = 0;
        GLuint ebo_hdl = 0;

        GLuint textureID{};

        void setup_shdrpgm(std::string const& vtx_shdr, std::string const& frag_shdr);
        //void draw();

        //for text 
        std::string text=""; // The text to be rendered
        std::string fontName=""; // Font file used
        int fontSize=0;
        float fontScale=0;


        bool isanimation = false;
        std::string textureFile;
        unsigned int shapeType{};
        //spirtesheet properties
        int currentFrame = 0;    //used to check which frame 
        float frametime = 0.1f;     //Time before switching to next frame
        float elapsedtime = 0.0f;   //Total time for frame switch
        int rows = 1, columns = 1;
        glm::vec2 uvOffset = { 0.0f, 0.0f };  // UV offset for the current frame
        glm::vec2 uvScale = { 1.0f, 1.0f };   // UV scale per frame
        int totalframe=1;
        glm::vec2 size= { 0.0f, 0.0f };
        GLenum primitive_type = 0;  // Which OpenGL primitive to render (e.g., GL_TRIANGLES)
        GLuint primitive_cnt = 0;   // Number of primitives to render
        GLuint draw_cnt = 0;        // Draw count (optional, can be based on vertex count)
        HUShader shdr_pgm;          // Shader program
        glm::vec3 color = { 1.0f, 1.0f, 1.0f }; // Default color for the model
        void draw(const glm::mat4& transform, const glm::mat4& projection, const glm::mat4& view);


        void cleanup() {
            if (vaoid) {
                glBindVertexArray(0);
                glDeleteVertexArrays(1, &vaoid);
                vaoid = 0;
            }
            if (vbo_hdl) {
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glDeleteBuffers(1, &vbo_hdl);
                vbo_hdl = 0;
            }
            if (ebo_hdl) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                glDeleteBuffers(1, &ebo_hdl);
                ebo_hdl = 0;
            }
            if (textureID) {
                glBindTexture(GL_TEXTURE_2D, 0);
                glDeleteTextures(1, &textureID);
                textureID = 0;
            }
            shdr_pgm.cleanup();
        }

    };


    const char* getName() const override {
        return "GraphicsSystem"; // Return the name of the system
    }

    //static GLModel mdl;
    static HUGraphics::GLModel points_model(const std::vector<Math2D::Vector2D>& points);
    static HUGraphics::GLModel lines_model(Math2D::Vector2D start, Math2D::Vector2D end, glm::vec3 color = { 1.0f,1.0f,1.0f });
    static HUGraphics::GLModel rectangle_model(glm::vec3 color = { 1.0f,1.0f,1.0f });
    static HUGraphics::GLModel triangle_model( glm::vec3 color = { 1.0f,1.0f,1.0f });
    static HUGraphics::GLModel circle_model(float radius = 0.5f, int segments = 32, glm::vec3 color = {1.0f,1.0f,1.0f});

    static HUGraphics::GLModel star_model(float radius, float inner_radius, int points, glm::vec3 color);

    static HUGraphics::GLModel texture_mesh(Texture& texture);
    static HUGraphics::GLModel text_mesh(GLuint textID);
    static HUGraphics::GLModel animation_mesh(Texture& texture, int rows, int columns, float frametime, int totalframe);
    static HUGraphics::GLModel update_animation_model(GLModel& model, double deltatime, int rows, int columns, float frametime,int totalframe);

    //private:
    static std::vector<GLModel> AllModels, outlineModels;
    static void clearOutlineModels();
    //static std::vector<GLModel> animation_models;


};

#endif // !HUGraphics_H