/**
* @file AudioEngine.cpp
* @brief Handles the implementation of the Audio Engine
*
* This uses the fmod studio to create an AudioCore, the core system.
*
* It maps :
* - each channel to their respective ID (mapChannels)
* - each sound to their name (mapSounds)
*
* Passes through most if not all functions through ErrorCheck for proper error checking
*
* Author: Jarren (95%)
* co-author :ruijie(5%)
*/

#include "AudioEngine.h"
#include <GlobalVariables.h>
#include <fmod/fmod_studio.hpp>
#include <fmod/fmod_errors.h>
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

#define MAX_PAN_DISTANCE 1.0f


// Static Global Pointer to AudioCore
AudioCore* sgpAudioCore = nullptr;
// Constructor
/**
* @brief Initializes the FMOD Studio system and core system.
*/
AudioCore::AudioCore() {
    mpStudioSystem = NULL;
    mnNextChannelId = 0;
    fMasterVolume = 0.5f;
    CAudioEngine::ErrorCheck(FMOD::Studio::System::create(&mpStudioSystem));
    CAudioEngine::ErrorCheck(mpStudioSystem->initialize(64, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, NULL));
    mpSystem = NULL;
    CAudioEngine::ErrorCheck(mpStudioSystem->getCoreSystem(&mpSystem));
    isInitialized = true;
}

// Destructor
/**
* @brief Cleans up and releases FMOD Studio resources.
*/AudioCore::~AudioCore() {
    for (auto& [id, channel] : mapChannels) {
        channel->stop();
    }

    // Release all sounds
    for (auto& [name, sound] : mapSounds) {
        sound->release();
    }

    // Cleanup FMOD systems
    if (mpStudioSystem) {
        mpStudioSystem->release();
    }
    if (mpSystem) {
        mpSystem->release();
    }

}

/**
* @brief Updates the FMOD system, applies master volume to all channels, and removes stopped channels.
*/
// In AudioCore.cpp
void AudioCore::Update() {
    // Only update volumes if masterVolume changed
    if (masterVolume != mLastMasterVolume) {
        for (auto& channelPair : mapChannels) {
            FMOD::Channel* pChannel = channelPair.second;
            bool isPlaying = false;
            pChannel->isPlaying(&isPlaying);

            if (isPlaying) {
                float currentVolume;
                pChannel->getVolume(&currentVolume); // Get current volume

                // Remove old masterVolume influence, then apply new masterVolume
                float correctedVolume = (currentVolume / mLastMasterVolume) * masterVolume;
                pChannel->setVolume(correctedVolume);
            }
        }
        mLastMasterVolume = masterVolume; // Update tracking variable
    }

    // Remove stopped channels (existing code)
    for (auto it = mapChannels.begin(); it != mapChannels.end(); ) {
        FMOD::Channel* pChannel = it->second;
        bool isPlaying = false;
        pChannel->isPlaying(&isPlaying);

        if (!isPlaying) {
            int channelId = it->first;
            it = mapChannels.erase(it);
            sgpAudioCore->mChannelToSound.erase(channelId);
        }
        else {
            ++it;
        }
    }

    // Update FMOD Studio System
    CAudioEngine::ErrorCheck(mpStudioSystem->update());
}


/**
* @brief Initializes the audio engine by creating an AudioCore instance.
*/
void CAudioEngine::Init() {
    sgpAudioCore = new AudioCore();
}

bool CAudioEngine::isInitialized() {
    return sgpAudioCore->isInitialized;
}

AudioCore* CAudioEngine::GetAudioCore() {
    return sgpAudioCore; // Assuming AudioCoreInstance is the internal instance
}

/**
* @brief Updates the audio engine by calling AudioCore's update method.
*/
void CAudioEngine::Update() {
    sgpAudioCore->Update();
}

/**
* @brief Shuts down the audio engine and cleans up resources.
*/
void CAudioEngine::Shutdown() {
    delete sgpAudioCore; // Clean up the AudioCore instance
    sgpAudioCore = nullptr;
}

/**
* @brief Checks for FMOD errors.
* @param result The FMOD result code.
* @return FMOD_OK if no error, logs the error otherwise.
*/
int CAudioEngine::ErrorCheck(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        // std::cout<< "FMOD ERROR " << result << " " << FMOD_ErrorString(result) << std::endl;
        return 1;
    }
    return 0;
}


float CAudioEngine::GetMasterVolume() {
    return sgpAudioCore->fMasterVolume;
}

/**
* @brief Sets the master volume for all active channels.
* @param fVolume The master volume (0.0 to 1.0).
*/
void CAudioEngine::SetMasterVolume(float fVolume) {
    // Clamp volume to ensure it is within a reasonable range
    fVolume = std::clamp(fVolume, 0.0f, 1.0f);
    //std::cout << "\nmasterVolume: " << fVolume;
    sgpAudioCore->fMasterVolume = fVolume;
}



/**
* @brief Loads a sound into memory.
* @param

Name The name of the sound file.
* @param b3d Whether the sound is 3D.
* @param bLooping Whether the sound should loop.
* @param bStream Whether the sound should be streamed from disk.
*/
void CAudioEngine::LoadSound(const std::string& strSoundName, bool b3d, bool bLooping, bool bStream) {
    std::string filePath = "./Assets/Audio\\" + strSoundName;
    if (sgpAudioCore->mapSounds.find(filePath) != sgpAudioCore->mapSounds.end())
        return;


    FMOD_MODE eMode = FMOD_DEFAULT;
    eMode |= b3d ? FMOD_3D : FMOD_2D;
    eMode |= bLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
    eMode |= bStream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

    FMOD::Sound* pSound = nullptr;
    FMOD_RESULT result = sgpAudioCore->mpSystem->createSound(strSoundName.c_str(), eMode, nullptr, &pSound);
    if (result != FMOD_OK) {
        std::cerr << "Error loading sound: " << strSoundName << " | Error: " << FMOD_ErrorString(result) << std::endl;
        return;
    }

    if (pSound) {
        sgpAudioCore->mapSounds[strSoundName] = pSound;
    }
    else {
        std::cerr << "FMOD created a null sound for: " << strSoundName << std::endl;
    }
}



/**
* @brief Unloads a sound from memory.
* @param strSoundName The name of the sound to unload.
*/
void CAudioEngine::UnLoadSound(const std::string& strSoundName) {
    std::string filePath = "./Assets/Audio\\" + strSoundName;
    auto tFoundIt = sgpAudioCore->mapSounds.find(filePath);
    if (tFoundIt == sgpAudioCore->mapSounds.end()) {
        return;
    }

    CAudioEngine::ErrorCheck(tFoundIt->second->release());
    sgpAudioCore->mapSounds.erase(tFoundIt);
}

/**
* @brief Plays a sound with specified panning and volume.
* @param strSoundName The name of the sound to play.
* @param fPan The panning position (-1.0 for left, 1.0 for right).
* @param volume The volume level in decibels.
* @return int The channel ID if the sound plays successfully, or -1 if there is an error.
*/

int CAudioEngine::PlaySound(const std::string& strSoundName, float fPan, float volume, int customChannelId) {
    // Load sound if it does not already exist
    std::string filePath = "./Assets/Audio\\" + strSoundName;
    auto it = sgpAudioCore->mapSounds.find(filePath);

    if (it == sgpAudioCore->mapSounds.end()) {
        LoadSound(strSoundName);
        it = sgpAudioCore->mapSounds.find(filePath);
        if (it == sgpAudioCore->mapSounds.end()) {
            return -1; // Unable to find or load sound
        }
    }

    FMOD::Channel* pChannel = nullptr;

    // ===== NEW: Clean up stopped channels first =====
    for (auto it2 = sgpAudioCore->mapChannels.begin(); it2 != sgpAudioCore->mapChannels.end(); ) {
        bool isPlaying = false;
        it2->second->isPlaying(&isPlaying);
        if (!isPlaying) {
            // Remove from both maps
            sgpAudioCore->mChannelToSound.erase(it2->first);
            it2 = sgpAudioCore->mapChannels.erase(it2);
        }
        else {
            ++it2;
        }
    }

    // ===== NEW: Reset channel counter if no channels exist =====
    if (sgpAudioCore->mapChannels.empty()) {
        sgpAudioCore->mnNextChannelId = 0;
    }

    // Check if a valid custom channel ID is provided
    if (customChannelId != -1) {
        // Stop any existing sound on this channel
        auto channelIt = sgpAudioCore->mapChannels.find(customChannelId);
        if (channelIt != sgpAudioCore->mapChannels.end()) {
            channelIt->second->stop();
        }

        // Play new sound on this channel
        CAudioEngine::ErrorCheck(sgpAudioCore->mpSystem->playSound(it->second, nullptr, true, &pChannel));
        if (pChannel) {
            sgpAudioCore->mapChannels[customChannelId] = pChannel;
            sgpAudioCore->mChannelToSound[customChannelId] = strSoundName;
        }
    }

    // If no custom channel or creation failed, find an available channel
    if (!pChannel) {
        // First try to reuse a stopped channel
        for (auto& channelPair : sgpAudioCore->mapChannels) {
            bool isPlaying = false;
            bool isPausedd = true;
            channelPair.second->isPlaying(&isPlaying);
            channelPair.second->getPaused(&isPausedd);

            if (!isPlaying && isPausedd) {
                pChannel = channelPair.second;
                customChannelId = channelPair.first;
                break;
            }
        }

        // If no available channels, create new one
        if (!pChannel) {
            CAudioEngine::ErrorCheck(sgpAudioCore->mpSystem->playSound(it->second, nullptr, true, &pChannel));
            if (!pChannel) {
                return -1; // Failed to create channel
            }
            customChannelId = sgpAudioCore->mnNextChannelId++;
            sgpAudioCore->mapChannels[customChannelId] = pChannel;
        }
    }

    // Configure channel
    pChannel->setVolume(volume * sgpAudioCore->fMasterVolume);
    pChannel->setPan(fPan);
    pChannel->setPaused(false);
    sgpAudioCore->mChannelToSound[customChannelId] = strSoundName;

    return customChannelId;
}

/**
* @brief Sets a channel's volume using decibel values.
* @param nChannelId The channel ID.
* @param volume The desired volume in decibels.
*/
void CAudioEngine::SetChannelVolume(int nChannelId, float volume) {
    auto it = sgpAudioCore->mapChannels.find(nChannelId);
    if (it != sgpAudioCore->mapChannels.end()) {
        it->second->setVolume(volume * sgpAudioCore->fMasterVolume);
    }
}



/**
* @brief Checks if a specified sound is playing through any channel.
* @param strSoundName The name of the sound to check.
* @return True if the sound is playing, false otherwise.
*/
bool CAudioEngine::isPlaying(const std::string& strSoundName) {
    // Iterate through the active channels
    for (const auto& channelPair : sgpAudioCore->mapChannels) {
        FMOD::Channel* channel = channelPair.second;

        // Get the sound associated with this channel
        auto soundIt = sgpAudioCore->mChannelToSound.find(channelPair.first);
        if (soundIt != sgpAudioCore->mChannelToSound.end()) {
            // Check if this channel is playing the specified sound
            std::string soundName = strSoundName;
            if (soundIt->second == strSoundName) {
                bool isPlaying = false;
                channel->isPlaying(&isPlaying); // Check if this channel is currently playing
                return isPlaying;
            }
        }
    }
    return false; // The sound is not playing through any channel
}

/**
* @brief Calculates the panning based on object and player positions.
* @param objectPosition The position of the sound source.
* @param playerPosition The position of the listener/player.
* @param fPanX Output panning value for X-axis.
* @param fPanY Output panning value for Y-axis.
*/
#include <glm/glm.hpp>
#include <algorithm>  // For std::clamp
#include <iostream>

void CAudioEngine::PositionToPan(const glm::vec3& objectPosition, const glm::vec3& playerPosition, float& fPanX) {
    float deltaX = objectPosition.x - playerPosition.x;
    float absDeltaX = std::abs(deltaX);  // Absolute X distance
    float maxDistX = 220.0f;  // Maximum X range

    // Ignore if beyond 150 units in X-axis
    if (absDeltaX > maxDistX) {
        fPanX = 0.0f;  // No panning if out of range
        return;
    }

    // Normalize deltaX to the range [-1, 1]
    fPanX = deltaX / maxDistX;

    // Clamp fPanX to the range [-1, 1] (just in case)
    fPanX = std::clamp(fPanX, -1.0f, 1.0f);

    //std::cout << "\nfPan: " << fPanX;
}


/**
 * @brief Gets all channels along with their associated sound names, regardless of playing status.
 * @return A map of channel IDs to their associated sound names.
 */
AudioCore::ChannelMap CAudioEngine::getChannelsAndSounds() {
    AudioCore::ChannelMap allChannels; // To store the result

    // Iterate through all tracked channels
    for (const auto& channelPair : sgpAudioCore->mapChannels) {
        int channelId = channelPair.first;
        FMOD::Channel* pChannel = channelPair.second;


        // Look up the associated sound name
        auto soundNameIt = sgpAudioCore->mChannelToSound.find(channelId);
        std::string soundName = (soundNameIt != sgpAudioCore->mChannelToSound.end())
            ? soundNameIt->second
            : "Unknown"; // Default to "Unknown" if not found

        // Add the channel ID and sound name to the result
        allChannels[channelId] = pChannel;
    }

    return allChannels; // Return all tracked channels
}

AudioCore::ChannelMap CAudioEngine::getChannels() {
    // Iterate through all tracked channels
    return sgpAudioCore->mapChannels;

}



std::map<int, std::string> CAudioEngine::ListSounds() const {
    std::map<int, std::string> activeSounds;

    // Iterate over the channels currently mapped to sounds
    for (const auto& channelPair : sgpAudioCore->mChannelToSound) {
        int channelId = channelPair.first;          // Channel ID
        const std::string& soundName = channelPair.second; // Sound name
        activeSounds[channelId] = soundName;       // Store in the result map
    }

    return activeSounds; // Return the map of active sounds
}


/**
* @brief Converts a custom Vector3 to FMOD's FMOD_VECTOR type.
* @param vPosition The custom 3D position vector.
* @return The corresponding FMOD_VECTOR.
*/
FMOD_VECTOR CAudioEngine::VectorToFmod(const Math3D::Vector3D& vPosition) {
    FMOD_VECTOR fmodVector = { vPosition.x, vPosition.y, vPosition.z };
    return fmodVector;
}

/**
* @brief Pauses or unpauses a specific channel by its ID.
* @param nChannelId The ID of the channel to pause or unpause.
* @param bPause True to pause, false to unpause.
*/
void CAudioEngine::PauseChannel(int nChannelId, bool bPause) {
    auto it = sgpAudioCore->mapChannels.find(nChannelId);
    if (it != sgpAudioCore->mapChannels.end()) {
        it->second->setPaused(bPause);
    }
}

/**
* @brief Pauses or unpauses all active channels.
* @param bPause True to pause all channels, false to unpause.
*/
void CAudioEngine::PauseAllChannels(bool bPause) {
    for (auto& channel : sgpAudioCore->mapChannels) {
        if (channel.second) {
            channel.second->setPaused(bPause);
        }
    }
}

/**
* @brief Pauses state of channels associated with a specific sound name.
* @param strSoundName The name of the sound to toggle.
*/
void CAudioEngine::PauseSoundByName(const std::string& strSoundName) {
    for (const auto& pair : sgpAudioCore->mChannelToSound) {
        // Check if the channel is associated with the specified sound name
        if (pair.second == strSoundName) {
            auto it = sgpAudioCore->mapChannels.find(pair.first);
            if (it != sgpAudioCore->mapChannels.end()) {
                it->second->setPaused(true);  // Set the pause state to true (paused)
            }
        }
    }
}

/**
* @brief Unpauses state of channels associated with a specific sound name.
* @param strSoundName The name of the sound to toggle.
*/
void CAudioEngine::UnpauseSoundByName(const std::string& strSoundName) {
    for (const auto& pair : sgpAudioCore->mChannelToSound) {
        if (pair.second == strSoundName) {
            auto it = sgpAudioCore->mapChannels.find(pair.first);
            if (it != sgpAudioCore->mapChannels.end()) {
                it->second->setPaused(false);  // Set the pause state to true (paused)
            }
        }
    }
}



/**
* @brief Stops a sound by its name.
* @param strSoundName The name of the sound to stop.
*/
void CAudioEngine::StopSound(const std::string& strSoundName) {
    /* std::cout << "\n[AudioEngine] ====================" << std::endl;
     std::cout << "[AudioEngine] Requested to stop: " << strSoundName << std::endl;
     std::cout << "[AudioEngine] Channels currently mapped to sounds:" << std::endl;*/



    for (auto it = sgpAudioCore->mChannelToSound.begin(); it != sgpAudioCore->mChannelToSound.end(); ++it) {
        // std::cout << "[AudioEngine] Checking sound: " << it->second << " (Channel ID: " << it->first << ")" << std::endl;

        if (it->second == strSoundName) {
            auto chanIt = sgpAudioCore->mapChannels.find(it->first);
            if (chanIt != sgpAudioCore->mapChannels.end()) {
                // std::cout << "[AudioEngine] Found matching sound on Channel ID: " << it->first << std::endl;
                chanIt->second->stop();
                sgpAudioCore->mapChannels.erase(chanIt);
                // std::cout << "[AudioEngine] Channel " << it->first << " stopped and removed from mapChannels." << std::endl;
            }
            else {
                // std::cout << "[AudioEngine] Channel ID " << it->first << " not found in mapChannels!" << std::endl;
            }

            sgpAudioCore->mChannelToSound.erase(it);

            return;
        }
    }

    //std::cout << "[AudioEngine] Sound not currently playing or not found: " << strSoundName << std::endl;
    //std::cout << "[AudioEngine] ====================\n" << std::endl;
}


/**
* @brief ToggleSoundLooping
* @param strSoundName The name of the sound to Toggle
*/
bool CAudioEngine::ToggleSoundLooping(const std::string& strSoundName) {
    for (auto sound : sgpAudioCore->mapSounds) {
    }
    // Find sound in mapSounds
    std::string filePath = "./Assets/Audio\\" + strSoundName;
    auto it = sgpAudioCore->mapSounds.find(filePath);
    if (it == sgpAudioCore->mapSounds.end()) {
        return false;
    }

    // Get the FMOD::Sound object 
    FMOD::Sound* sound = it->second;

    // Retrieve the current mode
    FMOD_MODE currentMode;
    if (sound->getMode(&currentMode) != FMOD_OK) {
        return false;
    }

    // Toggle the looping mode
    FMOD_MODE newMode = (currentMode & FMOD_LOOP_NORMAL) ? FMOD_LOOP_OFF : FMOD_LOOP_NORMAL;

    // Set the new looping mode
    if (sound->setMode(newMode) != FMOD_OK) {
        return false;
    }

    // Successfully toggled looping mode
    return true;
}


/**
* @brief Checks if a sound is set to loop.
* @param strSoundName The name of the sound.
* @return bool True if the sound is looping, false otherwise.
*/
bool CAudioEngine::IsSoundLooping(const std::string& strSoundName) {
    // Search for the sound in the mapSounds container
    std::string filePath = "./Assets/Audio\\" + strSoundName;
    auto it = sgpAudioCore->mapSounds.find(filePath);
    if (it != sgpAudioCore->mapSounds.end()) {
        FMOD::Sound* sound = it->second;
        if (sound) {
            // Retrieve current looping mode
            FMOD_MODE currentMode;
            if (sound->getMode(&currentMode) == FMOD_OK) {
                return (currentMode & FMOD_LOOP_NORMAL) != 0; // Check if looping
            }
        }
    }

    return false; // Sound not found or unable to retrieve mode
}

/**
* @brief Pauses all the sounds currently played
*/
void CAudioEngine::PauseAllSounds() {
    for (auto& [channelId, pChannel] : sgpAudioCore->mapChannels) {
        if (pChannel) {
            bool isPlaying = false;
            pChannel->isPlaying(&isPlaying);
            if (isPlaying) {
                pChannel->setPaused(true); // Pause the channel
            }

        }
    }
}

/**
* @brief Resumes all the sounds currently played
*/
void CAudioEngine::ResumeAllSounds() {
    for (auto& [channelId, pChannel] : sgpAudioCore->mapChannels) {
        if (pChannel) {
            bool isPausedd = false;
            pChannel->getPaused(&isPausedd);
            if (isPausedd) {
                pChannel->setPaused(false); // Resume the channel
            }
        }
    }
}


/**
 * @brief Sets the volume for a sound, either by channel ID or by name.
 * @param strSoundName The sound name to target (if no channel ID is provided).
 * @param volume The desired volume level (0.0 to 1.0).
 * @param customChannelId The specific channel ID to target (-1 for all matching channels).
 */
void CAudioEngine::SetSoundVolume(const std::string& strSoundName, float volume, int customChannelId) {
    volume = std::clamp(volume, 0.0f, 1.0f);

    // Case 1: Target a specific channel by ID
    if (customChannelId != -1) {
        auto channelIt = sgpAudioCore->mapChannels.find(customChannelId);
        if (channelIt != sgpAudioCore->mapChannels.end()) {
            channelIt->second->setVolume(volume * sgpAudioCore->fMasterVolume);
        }
        return; // Exit early for single-channel updates
    }

    // Case 2: Update all channels playing this sound name
    for (const auto& pair : sgpAudioCore->mChannelToSound) {
        if (pair.second == strSoundName) {
            auto channelIt = sgpAudioCore->mapChannels.find(pair.first);
            if (channelIt != sgpAudioCore->mapChannels.end()) {
                channelIt->second->setVolume(volume * sgpAudioCore->fMasterVolume);
                //std::cout << "\nstrSoundName: " << strSoundName;
            }
        }
    }
}


/**
* @brief Gets the volume of a specific sound by its name.
* @param strSoundName The name of the sound
*/
float CAudioEngine::GetSoundVolume(const std::string& strSoundName) {
    // Iterate through the map of channels to find the ones associated with the sound name
    for (const auto& pair : sgpAudioCore->mChannelToSound) {
        if (pair.second == strSoundName) {
            // Find the channel in the mapChannels
            auto channelIt = sgpAudioCore->mapChannels.find(pair.first);
            if (channelIt != sgpAudioCore->mapChannels.end()) {
                // Get the volume for this channel
                float channelVolume = 0.0f;
                channelIt->second->getVolume(&channelVolume);
                return channelVolume;
            }
        }
    }

    // If no channels are found, return -1.0f (indicating the sound is not playing)
    return -1.0f;
}

/**
 * @brief Sets the panning of a specific sound by its name.
 * @param strSoundName The name of the sound to adjust.
 * @param fPan The desired panning.
 * @param customChannelId (Optional) The specific channel ID to target (-1 means all channels).
 */
void CAudioEngine::SetSoundPan(const std::string& strSoundName, float fPan, int customChannelId) {
    if (customChannelId != -1) {
        // If a specific channel ID is provided, update only that channel
        auto channelIt = sgpAudioCore->mapChannels.find(customChannelId);
        if (channelIt != sgpAudioCore->mapChannels.end()) {
            channelIt->second->setPan(fPan);
        }
        return; // Stop here since we only wanted to modify this one channel
    }

    // If no custom channel ID is provided (-1), update all channels playing this sound
    for (const auto& pair : sgpAudioCore->mChannelToSound) {
        if (pair.second == strSoundName) {
            auto channelIt = sgpAudioCore->mapChannels.find(pair.first);
            if (channelIt != sgpAudioCore->mapChannels.end()) {
                channelIt->second->setPan(fPan);
            }
        }
    }
}
