/**
 * @file FontSystem.h
 * @brief Header file for the FontSystem class, responsible for managing and rendering fonts in OpenGL.
 *
 * The FontSystem class provides functionality to load fonts, render text, manage font metadata,
 * and handle OpenGL resources for text rendering. It uses the FreeType library for font processing
 * and integrates OpenGL for GPU-based text rendering.
 *
 * Author:  * Author: Ruijie (%50)
 * Co-Author: Jarren (%20)
 * Co-Author: Jasper (%30)
 */

#pragma once

#include <map>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Shader.h"
#include "imgui.h"
#include "CommonIncludes.h"
#include <unordered_set>




 /**
  * @class FontSystem
  * @brief Handles the loading, management, and rendering of fonts in OpenGL.
  */
class FontSystem {
public:
    struct Character {
        GLuint charID;   // ID handle of the glyph texture
        glm::ivec2 Size;    // Size of the glyph
        glm::ivec2 Bearing; // Offset from baseline to left/top of glyph
        GLuint Advance;     // Offset to advance to next glyph
    };

    struct FontId {
        std::string path="";
        int size=0;

        bool operator==(const FontId& other) const {
            return path == other.path && size == other.size;
        }
        bool operator!=(const FontId& other) const {
            return !(*this == other);
        }
    };

    //Custom hash function for FontId
    struct FontIdHash {
        std::size_t operator()(const FontId& id) const {
            return std::hash<std::string>()(id.path) ^ std::hash<int>()(id.size);
        }
    };

    // Font data structure to hold character map and metadata
    struct FontData {
        std::map<GLchar, Character> characters;
        std::string name;  // Display name for the font
        bool isDefault;    // Is this the default font?
    };

    /**
     * @brief Default constructor. Initializes the FreeType library and OpenGL resources.
     */
    FontSystem() {
        Initialize();
    }

    /**
    * @brief Initializes the FreeType library and sets up OpenGL buffers.
    */   
    void Initialize();

    // Loads a font from a given path and size
    bool LoadFont(const std::string& fontPath, int fontSize, const std::string& fontName = "", bool setAsDefault = false);

    //render text to scrren
    void RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color, const std::string& fontPath, int fontSize, GLuint targetFBO);

    //set default font to be used if cant find font given
    void SetDefaultFont(const std::string& fontPath, int fontSize);

    
    std::vector<FontId> GetLoadedFonts() const;

    GLuint RenderTextToTexture(const std::string& text,float scale, glm::vec3 color, const std::string& fontPath, int fontSize);

    void Shutdown();

    void UpdateTextOnTexture(
        GLuint textureID,                // The texture to update
        const std::string& text,         // The new text to render
        float scale,                     // Scale factor for the text
        glm::vec3 color,                 // Color for the text
        const std::string& fontPath,     // Path to the font
        int fontSize                     // Font size
    );

    std::pair<float, float> CalculateTextureSize(const std::string& text, float scale, const FontData* fontData) const;
    const FontData* GetCurrentFontData(const std::string& fontPath, int fontSize) const;

private:
    FT_Library ft;  // FreeType library
    std::unordered_map<FontId, FontData, FontIdHash> fonts;
    FontId defaultFontId;

    GLuint VAO{}, VBO{};  // OpenGL objects for text rendering

    HUShader fontShader;  // Dedicated shader for rendering text
    GLuint framebufferID{};
    GLuint SetupFramebuffer(int width, int height);
};


