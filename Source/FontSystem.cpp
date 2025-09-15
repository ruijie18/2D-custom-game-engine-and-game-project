/**
 * @file FontSystem.cpp
 * @brief file for the FontSystem class, responsible definations for managing and rendering fonts in OpenGL.
 *
 * The FontSystem class provides functionality to load fonts, render text, manage font metadata,
 * and handle OpenGL resources for text rendering. It uses the FreeType library for font processing
 * and integrates OpenGL for GPU-based text rendering.
 *
 * Author:  * Author: Ruijie (%50)
 * Co-Author: Jarren (%20)
 * Co-Author: Jasper (%30)
 */

#include "FontSystem.h"
#include "ImguiManager.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>


GLuint textTexture;
GLuint textFBO;
GLuint textDepth;

// Shader source strings for text rendering
const std::string HU_FontShader_vs = {
  #include "../Shaders/HU_Font_Shader.vert"
};

const std::string HU_FontShader_fs = {
  #include "../Shaders/HU_Font_Shader.frag"
};



/**
 * @brief Initializes the FontSystem, including OpenGL and FreeType setup.
 */
void FontSystem::Initialize() {
    // Initialize FreeType library
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    if (!fontShader.CompileShaderFromString(GL_VERTEX_SHADER, HU_FontShader_vs))
    {
        std::cerr << "Failed to compile vertex shader!" << std::endl;
    }

    if (!fontShader.CompileShaderFromString(GL_FRAGMENT_SHADER, HU_FontShader_fs))
    {
        std::cerr << "Failed to compile fragment shader!" << std::endl;
    }

    if (!fontShader.Link()) {
        std::cerr << "ERROR::SHADER: Failed to link font shader program!" << std::endl;
        return;
    }

    // Set up VAO and VBO for text rendering
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create a framebuffer for rendering text to a textTexture (instead of relying on a global one)
    textFBO = SetupFramebuffer(screen_width, screen_height);
}

/**
 * @brief Sets the default font for rendering text especially when font given is not found
 */
void FontSystem::SetDefaultFont(const std::string& fontPath, int fontSize) {
    // Use AssetLibrary to get the font
    auto font = FontLibrary.GetAssets(fontPath);  // Use the AssetLibrary to get the font
    if (font) {
        fontSystem->LoadFont(font->GetFileName(), fontSize);  // Load the font into the system
    }
}


/**
 * @brief Retrieves metadata for a specific font.
 * @param fontPath Path to the font file.
 * @param fontSize Size of the font.
 * @return Pointer to the font data, or nullptr if not found.
 */
const FontSystem::FontData* FontSystem::GetCurrentFontData(const std::string& fontPath, int fontSize) const {
    if (!fontPath.empty()) {
        FontId requestedId{ FontLibrary.GetFileName(fontPath), fontSize };
        auto it = fonts.find(requestedId);

        if (it != fonts.end()) {
            // Font found, return it
            return &it->second;
        }
        else {
            // Font is not loaded yet, try loading it
            fontSystem->LoadFont(FontLibrary.GetFileName(fontPath), fontSize);

            // After attempting to load the font, check again
            it = fonts.find(requestedId);
            if (it != fonts.end()) {
                return &it->second;  // Return the loaded font data
            }
            else {
                std::cerr << "Error: Failed to load font: " << fontPath << " with size: " << fontSize << std::endl;
                return nullptr;
            }
        }
    }

    // Fallback to default font if the fontPath is empty
    auto defaultFontIt = fonts.find(defaultFontId);
    if (defaultFontIt != fonts.end()) {
        return &defaultFontIt->second;  // Return the default font if available
    }

    // If no default font exists, return the first font found in the map
    if (!fonts.empty()) {
        return &fonts.begin()->second;
    }

    return nullptr;  // No fonts found
}

/**
 * @brief get font to see if its loaded
 **/
std::vector<FontSystem::FontId> FontSystem::GetLoadedFonts() const {
    std::vector<FontId> loadedFonts;
    for (const auto& pair : fonts) {
        loadedFonts.push_back(pair.first);
    }
    return loadedFonts;
}

/**
 * @brief Loads a font into the system.
 * @param fontPath Path to the font file.
 * @param fontSize Size of the font.
 * @param fontName Name for the font (optional).
 * @param setAsDefault Whether to set the font as default.
 * @return True if the font was successfully loaded, false otherwise.
 */
bool FontSystem::LoadFont(const std::string& fontPath, int fontSize, const std::string& fontName, bool setAsDefault) {
    FontId id{ fontPath,fontSize };

    // Check if font is already loaded
    if (fonts.find(id) != fonts.end()) {
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "Failed to load font: " << fontPath << std::endl;
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, fontSize);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    FontData fontData;
    fontData.name = fontName.empty() ? fontPath : fontName;
    fontData.isDefault = setAsDefault;

    // Load characters and create textures
    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "Failed to load Glyph: " << c << std::endl;
            continue;
        }
        glGenTextures(1, &textTexture);
        glBindTexture(GL_TEXTURE_2D, textTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            textTexture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };

        fontData.characters.insert(std::pair<char, Character>(c, character));
    }

    fonts[id] = fontData;
    FT_Done_Face(face);

    if (setAsDefault || fonts.size() == 1) {
        SetDefaultFont(fontPath, fontSize);
    }

    return true;
}


/**
 * @brief Renders a given text string at a specified position.
 * @param text The text to render.
 * @param x X-coordinate for the text's position.
 * @param y Y-coordinate for the text's position.
 * @param scale Scaling factor for the text size.
 * @param color Color of the text (RGB vector).
 * @param fontPath Path to the font file (optional).
 * @param fontSize Size of the font (optional).
 * @param targetFBO Target framebuffer to render into.
 */
void FontSystem::RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color, const std::string& fontPath, int fontSize, GLuint targetFBO) {
    const FontData* fontData = GetCurrentFontData(fontPath, fontSize);
    if (!fontData) {
        std::cerr << "Font data not found!" << std::endl;
        return;
    }

    // Bind the target framebuffer (if provided)
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

    // Get current viewport dimensions
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    float framebufferWidth = static_cast<float>(viewport[2]);
    float framebufferHeight = static_cast<float>(viewport[3]);

    // Set up orthographic projection based on framebuffer size
    glm::mat4 projection = glm::ortho(0.0f, framebufferWidth, framebufferHeight, 0.0f);

    // Use font shader
    fontShader.Use();
    glUniform3f(glGetUniformLocation(fontShader.GetHandle(), "textColor"), color.x, color.y, color.z);
    glUniformMatrix4fv(glGetUniformLocation(fontShader.GetHandle(), "projection"), 1, GL_FALSE, &projection[0][0]);

    // Enable blending for transparent text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Bind the VAO
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Render each character
    for (char c : text) {
        const Character& ch = fontData->characters.at(c);

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
        { xpos,     ypos + h,  0.0f, 1.0f },  // Bottom-left vertex
        { xpos,     ypos,      0.0f, 0.0f },  // Top-left vertex
        { xpos + w, ypos,      1.0f, 0.0f },  // Top-right vertex
        { xpos,     ypos + h,  0.0f, 1.0f },  // Bottom-left vertex
        { xpos + w, ypos,      1.0f, 0.0f },  // Top-right vertex
        { xpos + w, ypos + h,  1.0f, 1.0f }   // Bottom-right vertex
        };

        // Bind character texture
        glBindTexture(GL_TEXTURE_2D, ch.charID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render the character
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Move cursor to the next character position
        x += (ch.Advance >> 6) * scale;
    }

    // Cleanup
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


/**
*  @brief Helper function to calculate the width and height of text texture
* 
* 
**/
std::pair<float, float> FontSystem::CalculateTextureSize(const std::string& text, float scale, const FontData* fontData) const {
    float width = 0.0f;
    float height = 0.0f;

    // Loop through each character in the text to calculate its total width and height
    for (char c : text) {
        auto it = fontData->characters.find(c);
        if (it != fontData->characters.end()) {
            const Character& ch = it->second;
            width += (ch.Advance >> 6) * scale;
            height = std::max(height, static_cast<float>(ch.Size.y) * scale);
        }
        else {
            std::cerr << "Warning: Character '" << c << "' not found in font data.\n";
        }
    }


    return { width, height };  // Return the calculated width and height
}

GLuint FontSystem::SetupFramebuffer(int width, int height) {
    if (framebufferID == 0) {
        glGenFramebuffers(1, &framebufferID);
    }

    // Create or update the texture that stores the text
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        //std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind framebuffer
    return tex;
}

// Helper function to get next power of 2
unsigned int nextPowerOfTwo(unsigned int n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}


/**
 * @brief Renders a given text string at a specified position.
 * @param text The text to render.
 * @param x X-coordinate for the text's position.
 * @param y Y-coordinate for the text's position.
 * @param scale Scaling factor for the text size.
 * @param color Color of the text (RGB vector).
 * @param fontPath Path to the font file (optional).
 * @param fontSize Size of the font (optional).
 * @param targetFBO Target framebuffer to render into.
 */
GLuint FontSystem::RenderTextToTexture(const std::string& text,float scale, glm::vec3 color, const std::string& fontPath, int fontSize) {
    const FontData* fontData = GetCurrentFontData(fontPath, fontSize);
    if (!fontData) {
        std::cerr << "Font data not found for: " << fontPath << " with size: " << fontSize << std::endl;
        return 0;
    }

    auto [width_1, height_1] = fontSystem->CalculateTextureSize(text, scale, fontData);

    GLuint textTextureID;
    glGenTextures(1, &textTextureID);
    glBindTexture(GL_TEXTURE_2D, textTextureID);

    float resolutionMultiplier = 200.0f; // Adjust this for better sharpness
    int textureWidth = static_cast<int>(width_1 * resolutionMultiplier);
    int textureHeight = static_cast<int>(height_1 * resolutionMultiplier);

    (void)textureHeight, textureWidth;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (textFBO == 0) {
        glGenFramebuffers(1, &textFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, textFBO);
        //std::cout << "Framebuffer created with ID: " << textFBO << std::endl;
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, textFBO);
        //std::cout << "Framebuffer already exists. ID: " << textFBO << std::endl;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textTextureID, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        //std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        return 0;
    }

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width_1), static_cast<float>(height_1), 0.0f);
    fontShader.Use();
    glUniform3f(glGetUniformLocation(fontShader.GetHandle(), "textColor"), color.x, color.y, color.z);
    glUniformMatrix4fv(glGetUniformLocation(fontShader.GetHandle(), "projection"), 1, GL_FALSE, &projection[0][0]);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    float x = 0.0f, y = 0;
    for (char c : text) {
        const Character& ch = fontData->characters.at(c);

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos, ypos + h, 0.0f, 0.0f },
            { xpos, ypos, 0.0f, 1.0f },
            { xpos + w, ypos, 1.0f, 1.0f },
            { xpos, ypos + h, 0.0f, 0.0f },
            { xpos + w, ypos, 1.0f, 1.0f },
            { xpos + w, ypos + h, 1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.charID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return textTextureID;
}

void FontSystem::UpdateTextOnTexture(
    GLuint textureID,
    const std::string& text,
    float scale,
    glm::vec3 color,
    const std::string& fontPath,
    int fontSize
) {
    // Retrieve font data based on fontPath and fontSize
    const FontData* fontData = GetCurrentFontData(fontPath, fontSize);
    if (!fontData) {
        std::cerr << "Font data not found for: " << fontPath << " with size: " << fontSize << std::endl;
        return;
    }

    // Calculate the size of the texture required for the text
    auto [width, height] = CalculateTextureSize(text, scale, fontData);

    // Bind the framebuffer and texture
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

    // Clear the framebuffer to remove any previous content
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up orthographic projection matrix for rendering text
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
    fontShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(fontShader.GetHandle(), "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3f(glGetUniformLocation(fontShader.GetHandle(), "textColor"), color.r, color.g, color.b);

    // Render each character of the text onto the texture
    float x = 0.0f, y = 0.0f; // Position to start drawing text

    for (char c : text) {
        // Get the character's data (texture, size, bearing, and advance)
        const Character& ch = fontData->characters.at(c);

        // Calculate the position and size of the character in screen space
        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        // Define vertices for the current character
        float vertices[6][4] = {
            { xpos, ypos + h, 0.0f, 0.0f },
            { xpos, ypos, 0.0f, 1.0f },
            { xpos + w, ypos, 1.0f, 1.0f },
            { xpos, ypos + h, 0.0f, 0.0f },
            { xpos + w, ypos, 1.0f, 1.0f },
            { xpos + w, ypos + h, 1.0f, 0.0f }
        };

        // Bind the character's texture
        glBindTexture(GL_TEXTURE_2D, ch.charID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Draw the character (text is rendered using triangles)
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Move the cursor to the next position based on the character's advance
        x += (ch.Advance >> 6) * scale;  // Advance is in 1/64th of pixels, so divide by 64
    }

    // Unbind the VAO and texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);

    // Unbind the framebuffer and restore OpenGL state
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
* 
* @brief shutdown of all systems and font
**/

void FontSystem::Shutdown() {
    // Iterate through all loaded fonts
    for (auto& fontPair : fonts) {
        FontData& fontData = fontPair.second;

        // Delete textures associated with each character
        for (auto& charPair : fontData.characters) {
            glDeleteTextures(1, &charPair.second.charID);
        }
    }

    fonts.clear(); // Clear the font map

    if (ft) { // Check if FreeType was initialized
        FT_Done_FreeType(ft);
        ft = nullptr; // Ensure it’s set to null after cleanup
    }

    // Cleanup OpenGL resources if necessary
    if (textTexture) {
        glDeleteTextures(1, &textTexture);
        textTexture = 0;
    }

    //std::cout << "FontSystem shutdown completed successfully." << std::endl;
}


