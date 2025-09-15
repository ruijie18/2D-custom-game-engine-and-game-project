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
#include "Volume.h"

#include FT_FREETYPE_H

void UpdateVolumeDisplays() {
    for (EntityID entity : ECoordinator.GetAllEntities()) {
        if (ECoordinator.HasComponent<Name>(entity)) {
            Name& name = ECoordinator.GetComponent<Name>(entity);
            if (name.name == "MasterVolumeDisplay" || name.name == "SFXVolumeDisplay" || name.name == "MusicVolumeDisplay") {
                HUGraphics::GLModel& mdl = ECoordinator.GetComponent<HUGraphics::GLModel>(entity);

                // Clean up the old texture if it exists
                if (mdl.textureID != 0) {
                    glBindTexture(GL_TEXTURE_2D, 0);  // Unbind any bound texture
                    glDeleteTextures(1, &mdl.textureID);
                    mdl.textureID = 0;
                }

                // Update the text content with the current volume
                std::string volumeType = "";
                if (name.name == "MasterVolumeDisplay") {
                    mdl.text = std::to_string(static_cast<int>(CAudioEngine::GetInstance().GetMasterVolume()));
                }

                // Render the updated text to a new texture
                GLuint updated_text = fontSystem->RenderTextToTexture(
                    mdl.text,
                    mdl.fontScale,
                    mdl.color,
                    mdl.fontName,
                    mdl.fontSize
                );

                // Assign the new texture ID
                mdl.textureID = updated_text;
            }
        }
    }
}