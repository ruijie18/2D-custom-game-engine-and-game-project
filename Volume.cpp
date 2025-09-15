/**
 * @file Volume.cpp
 * @brief Implementation of the function to update the volume displays in the volume settings page
 *
 * UpdateVolumeDisplays is needed to update the volume displays in the volume settings page.
 * 
 * Author: Lewis (100%)
 */


#include "CommonIncludes.h"
#include "AudioEngine.h"
#include <EntityManager.h> 
#include <GlobalVariables.h>
#include "algorithm"
#include "ButtonComponent.h"
#include <GL/glew.h>
#include <ft2build.h>
#include <Render.h>
#include "Core.h"
#include FT_FREETYPE_H

void UpdateVolumeDisplays();
RenderSystem renderSystem;

void UpdateVolumeDisplays() {
    for (EntityID entity : ECoordinator.GetAllEntities()) {
        if (ECoordinator.HasComponent<Name>(entity)) {
            Name& name = ECoordinator.GetComponent<Name>(entity);
            if (name.name == "MasterVolumeDisplay" || name.name == "SFXVolumeDisplay" || name.name == "MusicVolumeDisplay") {
                HUGraphics::GLModel& mdl = ECoordinator.GetComponent<HUGraphics::GLModel>(entity);
                auto& trans = ECoordinator.GetComponent<Transform>(entity);
                // Clean up the old texture if it exists
                if (mdl.textureID != 0) {
                    glBindTexture(GL_TEXTURE_2D, 0);  // Unbind any bound texture
                    glDeleteTextures(1, &mdl.textureID);
                    mdl.textureID = 0;
                }

                // Update the text content with the current volume
                if (name.name == "MasterVolumeDisplay") {
                    mdl.text = std::to_string(currentMasterVolume);
                }
                else if (name.name == "SFXVolumeDisplay") {
                    mdl.text = std::to_string(currentSFXVolume);
                }
                else if (name.name == "MusicVolumeDisplay") {
                    mdl.text = std::to_string(currentMusicVolume);
                }

                size_t digitCount = mdl.text.length();

                digitCount = std::clamp(digitCount, size_t(1), size_t(3));

                // 2. OR if you're using `fontSize` instead of scale, something like this:
                // mdl.fontSize = static_cast<int>(std::max(12, 48 - (digitCount * 4)));

                // Render the updated text to a new texture
                GLuint updated_text = fontSystem->RenderTextToTexture(
                    mdl.text,
                    mdl.fontScale,
                    mdl.color,
                    mdl.fontName,
                    mdl.fontSize
                );
                // Smallest at 1 digit, increases with more digits
                int baseSize = 50;
                int step = 25;

                // Adjust font size to grow with digit count
                trans.scale.x = static_cast<float>(baseSize) + (digitCount - 1) * step;

                // Assign the new texture ID
                mdl.textureID = updated_text;
            }
        }
    }
}
