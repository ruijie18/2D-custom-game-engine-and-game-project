/**
 * @file AssetLibrary.cpp
 * @brief Header file defining an asset management system for handling multimedia assets in an application.
 *
 * This file provides the template class `AssetLibrary<T>` for managing various types of assets like textures,
 * audio, and fonts. It defines a library to load, retrieve, delete, and list assets dynamically at runtime.
 * The assets are stored in an unordered map using their names as keys, which allows quick lookups and
 * prevents duplicate loading.
 *
 * Author: Ruijie (50%)
 * Co-Author: Jarren (50%)
*/

#include "AssetsManager.h"
#include "GlobalVariables.h"  // Include here to access `audioEngine`

Audio::Audio(const std::string& file_name) {
    // Load the audio from the file
    Asset = file_name;
    if (audioEngine->isInitialized()) {
        
        audioEngine->LoadSound(Asset, false, false, false);  // Load sound with default parameters
    }
    else {
        std::cerr << "Error: audioEngine is not initialized." << std::endl;
        
    }
}

Audio::~Audio() {
    // Cleanup
    //
    // "Audio destroyed: " << Asset << std::endl;
    if (audioEngine) {
        audioEngine->UnLoadSound(Asset);  // Unload sound on destruction
    }
    else {
        std::cerr << "Error: audioEngine is not initialized." << std::endl;
    }
}



Font::Font(const std::string& file_name) :Asset(file_name)
{
    //
    //  << "Loading font from: " << Asset << std::endl;
    fontSystem->LoadFont(file_name, 50);

}

Font::~Font()
{
}
