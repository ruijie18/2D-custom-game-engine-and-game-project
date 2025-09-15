/**
 * @file Graphics.cpp
 * @brief Implementation of the HUGraphics module for managing OpenGL rendering of various geometric shapes and textures.
 *
 * This source file defines the functions and methods of the `HUGraphics` class, which is responsible for
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
 Co-Author : Che Ee (5%)
 */

#include "Graphics.h"
#include "ParticleSystem.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <iomanip>
#include "vector3d.h"
#include "vector2d.h"
#include "InputSystem.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <random>
#include <GlobalVariables.h>

std::vector<HUGraphics::GLModel> HUGraphics::AllModels;
std::vector<HUGraphics::GLModel> HUGraphics::outlineModels;

//for animations
static double accumulatedTime = 0.0;


const std::string HUShader_vs = {
  #include "../Shaders/HU_Graphic_Shader.vert"
};

const std::string HUShader_fs = {
  #include "../Shaders/HU_Graphic_Shader.frag"
};

const std::string HU_TexShader_vs = {
  #include "../Shaders/HU_Tex_Shader.vert"
};

const std::string HU_TexShader_fs = {
  #include "../Shaders/HU_Tex_Shader.frag"
};


HUGraphics::HUGraphics() {
}

void HUGraphics::Init()
{

}






void HUGraphics::Update(double deltaTime)
{
    if (windowFocused) {


        // Loop through all entities to update animations
        for (auto& entity : mEntities) {
            if (ECoordinator.HasComponent<HUGraphics::GLModel>(entity)) {
                GLModel& model = ECoordinator.GetComponent<GLModel>(entity);

                // If this is a laser animation, check if the laser is active
                //bool isLaserActive = true; // Default to true if no laser component

                if (ECoordinator.HasComponent<LaserComponent>(entity)) {
                    const LaserComponent& laserComp = ECoordinator.GetComponent<LaserComponent>(entity);
                    if (!laserComp.isActive) {

                        continue;
                    }
                }

                // Update all animations (including laser if active)
                if (model.isanimation) {
                    for (int step = 0; step < numberofsteps; ++step) {
                        update_animation_model(model,
                            deltaTime,   // Use fixed frame update interval
                            model.rows,
                            model.columns,
                            model.frametime,
                            model.totalframe);
                    }
                }

            }
        }
    }
}

void HUGraphics::draw()
{
}

void HUGraphics::cleanup()
{
    for (auto& model : AllModels) {
        model.cleanup();
    }
    AllModels.clear();
    clearOutlineModels();
}

void HUGraphics::GLModel::setup_shdrpgm(std::string const& vtx_shdr, std::string const& frag_shdr)
{
    if (!shdr_pgm.CompileShaderFromString(GL_VERTEX_SHADER, vtx_shdr)) {
        // std::
        // << "Vertex shader failed to compile: ";
        // std::cout<< shdr_pgm.GetLog() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (!shdr_pgm.CompileShaderFromString(GL_FRAGMENT_SHADER, frag_shdr)) {
        // std::cout<< "Fragment shader failed to compile: ";
        // std::cout<< shdr_pgm.GetLog() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (!shdr_pgm.Link()) {
        // std::cout<< "Shader program failed to link!" << std::endl;
        // std::cout<< shdr_pgm.GetLog() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    if (!shdr_pgm.Validate()) {
        // std::cout<< "Shader program failed to validate!" << std::endl;
        // std::cout<< shdr_pgm.GetLog() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void HUGraphics::GLModel::draw(const glm::mat4& transform, const glm::mat4& projection, const glm::mat4& view)
{
    //projection: Handles the projection transformation( orthographic).
    //view : Transforms the scene from world space to camera space.
    //transform : Transforms the model from object space to world space.

    GLint colorLoc;
    GLint projLoc, viewLoc, transformLoc, alphaLoc, flipLoc;

    // Use the shader program
    shdr_pgm.Use();
    glBindVertexArray(vaoid);

    // Set the transform (model) matrix
    transformLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

    // Set the projection matrix
    projLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Set the view matrix (camera)
    viewLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    //set the alpha value....
    
    alphaLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "u_Alpha");
    glUniform1f(alphaLoc, alpha);

    // Pass flip flag
    flipLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "flipTexture");
    glUniform1i(flipLoc, flipTextureHorizontally ? 1 : 0); // Convert bool to int


    if (textureID != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        GLint textureLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "texture1");
        glUniform1i(textureLoc, 0);

        GLint useTextureLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "useTexture");
        glUniform1i(useTextureLoc, GL_TRUE);

        // Pass UV offset and scale for animated models
        GLint uvOffsetLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "uvOffset");
        glUniform2fv(uvOffsetLoc, 1, glm::value_ptr(uvOffset));


        GLint uvScaleLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "uvScale");
        glUniform2fv(uvScaleLoc, 1, glm::value_ptr(uvScale));

        GLint tintColorLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "tintColor");
        glUniform3f(tintColorLoc, color.r, color.g, color.b);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    else
    {
        GLint useTextureLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "useTexture");
        glUniform1i(useTextureLoc, GL_FALSE);  // No texture, use color
    }

    // Draw based on primitive type
    switch (primitive_type) {
    case GL_POINTS:
        glPointSize(10.f);
        colorLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "shapeColor");
        glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
        glDrawArrays(primitive_type, 0, draw_cnt);
        glPointSize(1.f);
        break;
    case GL_TRIANGLE_FAN:
        colorLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "shapeColor");
        glUniform3f(colorLoc, color.r, color.g, color.b);
        glDrawArrays(primitive_type, 0, draw_cnt);
        break;
    case GL_TRIANGLES:
        colorLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "shapeColor");
        glUniform3f(colorLoc, color.r, color.g, color.b);
        glDrawElements(GL_TRIANGLES, draw_cnt, GL_UNSIGNED_INT, 0);
        break;
    case GL_LINES:
        glLineWidth(10.f);
        colorLoc = glGetUniformLocation(shdr_pgm.GetHandle(), "shapeColor");
        glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
        glDrawArrays(primitive_type, 0, draw_cnt);
        break;
    }

    glBindVertexArray(0);
    shdr_pgm.UnUse();
}

HUGraphics::GLModel HUGraphics::update_animation_model(GLModel& model, double deltaTime, int rows, int columns, float frameTime, int totalframe) {
    // Calculate the scale for each frame based on sprite sheet rows and columns
    model.uvScale = { 1.0f / columns, 1.0f / rows };

    //the animation only move when the main character is walkning
    //Update the elapsed time for the current frame
    model.elapsedtime += static_cast<float>(deltaTime);

    //Check if we need to switch to the next frame
    if (model.elapsedtime >= frameTime) {
        model.currentFrame = (model.currentFrame + 1) % (totalframe);  // Advance frame
        model.elapsedtime = 0.0f;  // Reset elapsed time
    }


    // Calculate UV offset for the new frame
    int row = model.currentFrame / columns;
    int col = model.currentFrame % columns;
    
    if (model.flipTextureHorizontally) {
        col = columns - col - 1; // Reverse the column when flipped
    }

    model.uvOffset = { col * model.uvScale.x, row * model.uvScale.y };


    return model;
}

// @brief Generates a model that renders points at specified 2D coordinates.
// @param points A vector of 2D points to render.
// @return A GLModel representing the points.

HUGraphics::GLModel HUGraphics::points_model(const std::vector<Math2D::Vector2D>& points) {
    // Create local handles for the VAO and VBO
    GLuint VBO, VAO;

    // Create and set up the VAO
    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);

    // Bind and populate the VBO with point data
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Math2D::Vector2D) * points.size(), points.data(), GL_STATIC_DRAW);

    // Set up vertex attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Math2D::Vector2D), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO and VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create and set up the GLModel
    HUGraphics::GLModel model;
    model.vaoid = VAO;                      // Store the VAO
    model.vbo_hdl = VBO;                    // Store the VBO
    model.primitive_type = GL_POINTS;       // Set to GL_POINTS
    model.draw_cnt = static_cast<GLuint>(points.size());  // Set the number of vertices
    model.primitive_cnt = model.draw_cnt;   // Number of primitives is the same as number of points
    model.color = { 1.0f, 1.0f, 1.0f };     // Default white color
    model.setup_shdrpgm(HUShader_vs, HUShader_fs);  // Attach the shader program

    // Optionally add to global tracking if needed
    AllModels.emplace_back(model);

    return model;
}




HUGraphics::GLModel HUGraphics::lines_model(Math2D::Vector2D start, Math2D::Vector2D end, glm::vec3 color)
{
    // Step 1: Define the vertices for the line directly from start to end
    std::vector<Math2D::Vector2D> pos_vtx = {
        start,  // Start point
        end     // End point
    };

    glm::vec3 white = { 1.0f,1.0f,1.0f };

    // Step 2: Create buffers (VBO and VAO)
    GLuint vbo_hdl, vao_hdl;
    glCreateBuffers(1, &vbo_hdl);
    glCreateVertexArrays(1, &vao_hdl);

    // Step 3: Upload the vertex data to the GPU
    glNamedBufferStorage(vbo_hdl, sizeof(Math2D::Vector2D) * pos_vtx.size(), pos_vtx.data(), GL_DYNAMIC_STORAGE_BIT);

    // Step 4: Set up the VAO
    glBindVertexArray(vao_hdl);
    glEnableVertexArrayAttrib(vao_hdl, 0);  // Enable the vertex attribute at location 0 (for position)
    glVertexArrayVertexBuffer(vao_hdl, 0, vbo_hdl, 0, sizeof(Math2D::Vector2D));  // Bind the VBO to the VAO
    glVertexArrayAttribFormat(vao_hdl, 0, 2, GL_FLOAT, GL_FALSE, 0);  // Define the format of the position data
    glVertexArrayAttribBinding(vao_hdl, 0, 0);  // Bind the attribute to buffer binding 0
    glBindVertexArray(0);  // Unbind the VAO

    // Step 5: Create the GLModel and setup the shader program
    HUGraphics::GLModel mdl;
    mdl.vaoid = vao_hdl;
    mdl.vbo_hdl = vbo_hdl;
    mdl.primitive_type = GL_LINES;  // Use GL_LINES to draw the line
    mdl.setup_shdrpgm(HUShader_vs, HUShader_fs);
    mdl.draw_cnt = 2;  // We have 2 vertices to draw 1 line
    mdl.primitive_cnt = 1;  // One line
    mdl.color = color;  // Set the color for the line
    outlineModels.emplace_back(mdl);
    return mdl;
}


HUGraphics::GLModel HUGraphics::rectangle_model(glm::vec3 color) {
    // Define the vertices for a 1x1 rectangle, centered at the origin
    std::vector<Math2D::Vector2D> pos_vtx = {
        {-0.5f, -0.5f},  // Bottom-left corner
        {0.5f, -0.5f},   // Bottom-right corner
        {-0.5f, 0.5f},   // Top-left corner
        {0.5f, 0.5f}     // Top-right corner
    };

    // Define the indices for two triangles (6 indices total)
    unsigned int indices[] = {
        0, 1, 2,  // First triangle (Bottom-left, Bottom-right, Top-left)
        1, 3, 2   // Second triangle (Bottom-right, Top-right, Top-left)
    };

    // Create buffers (VBO, EBO, and VAO)
    GLuint vbo_hdl, ebo_hdl, vao_hdl;
    glCreateBuffers(1, &vbo_hdl);
    glCreateBuffers(1, &ebo_hdl);
    glCreateVertexArrays(1, &vao_hdl);

    // Upload the vertex data to the GPU
    glNamedBufferStorage(vbo_hdl, sizeof(Math2D::Vector2D) * pos_vtx.size(), pos_vtx.data(), GL_DYNAMIC_STORAGE_BIT);

    // Upload the index data to the GPU
    glNamedBufferStorage(ebo_hdl, sizeof(indices), indices, GL_DYNAMIC_STORAGE_BIT);

    // Set up the VAO
    glBindVertexArray(vao_hdl);
    glEnableVertexArrayAttrib(vao_hdl, 0);  // Enable the vertex attribute at location 0
    glVertexArrayVertexBuffer(vao_hdl, 0, vbo_hdl, 0, sizeof(Math2D::Vector2D));  // Bind the VBO to the VAO
    glVertexArrayAttribFormat(vao_hdl, 0, 2, GL_FLOAT, GL_FALSE, 0);  // Define the format of the vertex data
    glVertexArrayAttribBinding(vao_hdl, 0, 0);  // Bind the attribute to buffer binding 0
    glVertexArrayElementBuffer(vao_hdl, ebo_hdl);  // Bind the EBO to the VAO
    glBindVertexArray(0);  // Unbind the VAO

    // Create the GLModel and setup the shader program
    HUGraphics::GLModel mdl;
    mdl.vaoid = vao_hdl;
    mdl.ebo_hdl = ebo_hdl;
    mdl.vbo_hdl = vbo_hdl;
    mdl.primitive_type = GL_TRIANGLES;  // Use GL_TRIANGLES for drawing
    mdl.setup_shdrpgm(HUShader_vs, HUShader_fs);
    mdl.draw_cnt = 6;  // We have 6 indices for two triangles
    mdl.primitive_cnt = 2;  // Two triangles
    mdl.color = color;  // Set the color for the rectangle
    AllModels.emplace_back(mdl);
    return mdl;
}


HUGraphics::GLModel HUGraphics::triangle_model(glm::vec3 color) {
    // Define the vertices for a unit triangle centered at the origin
    std::vector<Math2D::Vector2D> pos_vtx = {
        {-0.5f, 0.5f},  // Top-left corner
        {0.5f, 0.5f},   // Top-right corner
        {0.0f, -0.5f}   // Bottom corner
    };

    unsigned int indices[] = { 0, 1, 2 };  // Indices for the triangle

    // Create the VBO, EBO, and VAO
    GLuint vbo_hdl, ebo_hdl, vao_hdl;
    glCreateBuffers(1, &vbo_hdl);
    glCreateBuffers(1, &ebo_hdl);
    glCreateVertexArrays(1, &vao_hdl);

    // Upload the vertex data to the GPU
    glNamedBufferStorage(vbo_hdl, sizeof(Math2D::Vector2D) * pos_vtx.size(), pos_vtx.data(), GL_DYNAMIC_STORAGE_BIT);

    // Upload the index data to the GPU
    glNamedBufferStorage(ebo_hdl, sizeof(indices), indices, GL_DYNAMIC_STORAGE_BIT);

    // Set up the VAO
    glBindVertexArray(vao_hdl);
    glEnableVertexArrayAttrib(vao_hdl, 0);  // Enable the vertex attribute at location 0
    glVertexArrayVertexBuffer(vao_hdl, 0, vbo_hdl, 0, sizeof(Math2D::Vector2D));  // Bind the VBO to the VAO
    glVertexArrayAttribFormat(vao_hdl, 0, 2, GL_FLOAT, GL_FALSE, 0);  // Format of the vertex data
    glVertexArrayAttribBinding(vao_hdl, 0, 0);  // Bind the attribute to buffer binding 0
    glVertexArrayElementBuffer(vao_hdl, ebo_hdl);  // Bind the EBO to the VAO
    glBindVertexArray(0);  // Unbind the VAO

    // Create the GLModel and set its properties
    HUGraphics::GLModel mdl;
    mdl.vaoid = vao_hdl;
    mdl.ebo_hdl = ebo_hdl;
    mdl.vbo_hdl = vbo_hdl;
    mdl.primitive_type = GL_TRIANGLES;  // Use GL_TRIANGLES for rendering
    mdl.setup_shdrpgm(HUShader_vs, HUShader_fs);
    mdl.draw_cnt = 3;  // 3 vertices
    mdl.primitive_cnt = 1;  // 1 triangle
    mdl.color = color;  // Set the triangle's color
    AllModels.emplace_back(mdl);
    return mdl;
}

HUGraphics::GLModel HUGraphics::star_model(float radius, float inner_radius, int points, glm::vec3 color) {
    std::vector<Math2D::Vector2D> pos_vtx;
    float angle_step = glm::pi<float>() / points; // Half the step since a star has inner and outer points

    // Center of the star at the origin (0, 0)
    pos_vtx.emplace_back(0.0f, 0.0f);

    // Generate the vertices alternating between outer and inner radius
    for (int i = 0; i <= 2 * points; ++i) {
        float angle = i * angle_step;
        float r = (i % 2 == 0) ? radius : inner_radius;
        pos_vtx.emplace_back(glm::cos(angle) * r, glm::sin(angle) * r);
    }

    // Create the VBO and VAO
    GLuint vbo_hdl, vao_hdl;
    glCreateBuffers(1, &vbo_hdl);
    glCreateVertexArrays(1, &vao_hdl);

    // Upload vertex data to the GPU
    glNamedBufferStorage(vbo_hdl, sizeof(Math2D::Vector2D) * pos_vtx.size(), pos_vtx.data(), GL_DYNAMIC_STORAGE_BIT);

    // Set up the VAO
    glBindVertexArray(vao_hdl);
    glEnableVertexArrayAttrib(vao_hdl, 0);
    glVertexArrayVertexBuffer(vao_hdl, 0, vbo_hdl, 0, sizeof(Math2D::Vector2D));
    glVertexArrayAttribFormat(vao_hdl, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao_hdl, 0, 0);
    glBindVertexArray(0);

    // Create the GLModel and set its properties
    HUGraphics::GLModel mdl;
    mdl.vaoid = vao_hdl;
    mdl.vbo_hdl = vbo_hdl;
    mdl.primitive_type = GL_TRIANGLE_FAN;
    mdl.setup_shdrpgm(HUShader_vs, HUShader_fs);
    mdl.draw_cnt = static_cast<GLuint>(pos_vtx.size());
    mdl.primitive_cnt = mdl.draw_cnt - 2;
    mdl.color = color;
    AllModels.emplace_back(mdl);
    return mdl;
}



HUGraphics::GLModel HUGraphics::circle_model(float radius, int segments, glm::vec3 color) {
    std::vector<Math2D::Vector2D> pos_vtx;
    float angle_step = glm::two_pi<float>() / segments;

    // Center of the circle at the origin (0, 0)
    pos_vtx.emplace_back(0.0f, 0.0f);

    // Generate the vertices around the circle's perimeter
    for (int i = 0; i <= segments; ++i) {
        float angle = i * angle_step;
        pos_vtx.emplace_back(glm::cos(angle) * radius, glm::sin(angle) * radius);
    }

    // Create the VBO and VAO
    GLuint vbo_hdl, vao_hdl;
    glCreateBuffers(1, &vbo_hdl);
    glCreateVertexArrays(1, &vao_hdl);

    // Upload vertex data to the GPU
    glNamedBufferStorage(vbo_hdl, sizeof(Math2D::Vector2D) * pos_vtx.size(), pos_vtx.data(), GL_DYNAMIC_STORAGE_BIT);

    // Set up the VAO
    glBindVertexArray(vao_hdl);
    glEnableVertexArrayAttrib(vao_hdl, 0);
    glVertexArrayVertexBuffer(vao_hdl, 0, vbo_hdl, 0, sizeof(Math2D::Vector2D));
    glVertexArrayAttribFormat(vao_hdl, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vao_hdl, 0, 0);
    glBindVertexArray(0);

    // Create the GLModel and set its properties
    HUGraphics::GLModel mdl;
    mdl.vaoid = vao_hdl;
    mdl.vbo_hdl = vbo_hdl;
    mdl.primitive_type = GL_TRIANGLE_FAN;
    mdl.setup_shdrpgm(HUShader_vs, HUShader_fs);
    mdl.draw_cnt = static_cast<GLuint>(pos_vtx.size());
    mdl.primitive_cnt = mdl.draw_cnt - 2;
    mdl.color = color;
    AllModels.emplace_back(mdl);
    return mdl;
}

HUGraphics::GLModel HUGraphics::texture_mesh(Texture& texture) {
    // Define the vertices for a 1x1 rectangle (centered at the origin)
    struct Vertex {
        Math2D::Vector2D position;
        Math2D::Vector2D texCoord;
    };

    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {0.0f, 0.0f}},  // Bottom-left corner
        {{0.5f, -0.5f}, {1.0f, 0.0f}},   // Bottom-right corner
        {{-0.5f, 0.5f}, {0.0f, 1.0f}},   // Top-left corner
        {{0.5f, 0.5f}, {1.0f, 1.0f}}     // Top-right corner
    };

    unsigned int indices[] = {
        0, 1, 2,  // First triangle
        1, 3, 2   // Second triangle
    };

    GLuint VBO, VAO, EBO;
    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);
    glCreateBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Math2D::Vector2D)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, texture.GetTextureID());

    // Create the GLModel and set its properties
    HUGraphics::GLModel model;
    model.vaoid = VAO;
    model.ebo_hdl = EBO;
    model.vbo_hdl = VBO;
    model.primitive_type = GL_TRIANGLES;
    model.primitive_cnt = 2;  // Two triangles
    model.draw_cnt = 6;        // Six indices for two triangles
    model.textureID = texture.GetTextureID();
    model.color = { 1.0f, 1.0f, 1.0f };  // Default white color
    model.setup_shdrpgm(HU_TexShader_vs, HU_TexShader_fs);
    //model.projection = glm::ortho(0.0f, 1600.f, 900.f, 0.0f);  // Top-left (0, 0) origin
    //model.transform = glm::mat4(1.0f);
    //model.position = pos;
    AllModels.emplace_back(model);
    model.setup_shdrpgm(HUShader_vs, HUShader_fs);
    //model.projection = glm::ortho(0.0f, 1600.f, 900.f, 0.0f);  // Top-left (0, 0) origin
    //model.transform = glm::mat4(1.0f);
    //model.position = pos;

    return model;
}

HUGraphics::GLModel HUGraphics::text_mesh(GLuint textID) {
    // Define the vertices for a 1x1 rectangle (centered at the origin)
    struct Vertex {
        Math2D::Vector2D position;
        Math2D::Vector2D texCoord;
    };

    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {0.0f, 0.0f}},  // Bottom-left corner
        {{0.5f, -0.5f}, {1.0f, 0.0f}},   // Bottom-right corner
        {{-0.5f, 0.5f}, {0.0f, 1.0f}},   // Top-left corner
        {{0.5f, 0.5f}, {1.0f, 1.0f}}     // Top-right corner
    };

    unsigned int indices[] = {
        0, 1, 2,  // First triangle
        1, 3, 2   // Second triangle
    };

    GLuint VBO, VAO, EBO;
    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);
    glCreateBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Math2D::Vector2D)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Use the provided textID (GLuint) as the texture
    GLuint textureID = textID;  // Use the passed in textureID (e.g., from RenderTextToTexture)

    // Create the GLModel and set its properties
    HUGraphics::GLModel model;
    model.vaoid = VAO;
    model.ebo_hdl = EBO;
    model.vbo_hdl = VBO;
    model.primitive_type = GL_TRIANGLES;
    model.primitive_cnt = 2;  // Two triangles
    model.draw_cnt = 6;       // Six indices for two triangles
    model.textureID = textureID;  // Set the texture to the passed in textureID
    model.color = { 1.0f, 1.0f, 1.0f };  // Default white color
    model.setup_shdrpgm(HUShader_vs, HUShader_fs);

    // Return the model
    return model;
}

HUGraphics::GLModel HUGraphics::animation_mesh(Texture& texture, int rows = 1, int columns = 1, float frametime = 0.1f, int totalframe = 1)
{
    //pos +texture coord 
    float maxSize = 100.0f; 
    struct Vertex {
        Math2D::Vector2D position;
        Math2D::Vector2D texCoord;
    };

    float scaleFactor = 0.01f;  
    glm::vec2 size(texture.GetImageWidth(), texture.GetImageHeight());
    if (size.x > maxSize || size.y > maxSize) {
        float aspectRatio = size.x / size.y;
        if (size.x > size.y) {
            size.x = maxSize;
            size.y = maxSize / aspectRatio;
        }
        else {
            size.y = maxSize;
            size.x = maxSize
                ;
        }
    }
    std::vector<Vertex> vertices = {
        {{-size.x / 2 * scaleFactor, -size.y / 2 * scaleFactor}, {0.0f, 0.0f}},  // Bottom-left corner
        {{size.x / 2 * scaleFactor, -size.y / 2 * scaleFactor}, {1.0f, 0.0f}},   // Bottom-right corner
        {{-size.x / 2 * scaleFactor, size.y / 2 * scaleFactor}, {0.0f, 1.0f}},   // Top-left corner
        {{size.x / 2 * scaleFactor, size.y / 2 * scaleFactor}, {1.0f, 1.0f}}     // Top-right corner
     

    };

    

    unsigned int indices[] = {
        0, 1, 2,  // First triangle
        1, 3, 2   // Second triangle
    };

    GLuint VBO, VAO, EBO;
    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);
    glCreateBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Math2D::Vector2D)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, texture.GetTextureID());

    GLModel model;

    model.isanimation = true;
    model.vaoid = VAO;
    model.ebo_hdl = EBO;
    model.primitive_type = GL_TRIANGLES;
    model.primitive_cnt = 2;
    model.draw_cnt = 6;
    model.color = { 1.0f, 1.0f, 1.0f };
    model.setup_shdrpgm(HU_TexShader_vs, HU_TexShader_fs);
    model.rows = rows;
    model.columns = columns;
    model.uvScale = { 1.0f / columns, 1.0f / rows };
    model.frametime = frametime; // Time per frame
    model.uvOffset = { 0.0f, 0.0f }; // Start at the first frame
    model.totalframe = totalframe;
    model.textureID = texture.GetTextureID();

    AllModels.emplace_back(model);

    return model;
}

void HUGraphics::clearOutlineModels() {
    for (auto& outline : outlineModels) {
        outline.cleanup();
    }
    glFlush();  // Force the driver to process all deletion requests
    outlineModels.clear();  
}
