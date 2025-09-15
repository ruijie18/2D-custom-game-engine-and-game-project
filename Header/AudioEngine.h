#pragma once

#ifndef AUDIO_ENGINE_H

#define AUDIO_ENGINE_H

/**
 * @file AudioEngine.h
 * @brief Declares the variables used in AudioEngine.cpp
 *
 * Holds the class and for the main system (AudioCore) and the audio engine used by the game (CAudioEngine)
 *
 * Author: Jarren (100%)
*/

#include <fmod/fmod.hpp>
#include <fmod/fmod_studio.hpp>
#include <string>
#include <map>
#include <vector>
#include <math.h>
#include <iostream>
#include "vector3d.h"

struct AudioCore {
    AudioCore();
    ~AudioCore();

    void Update();

    FMOD::Studio::System* mpStudioSystem;
    FMOD::System* mpSystem;

    int mnNextChannelId;
    float fMasterVolume;

    using SoundMap = std::map<std::string, FMOD::Sound*>;
    using ChannelMap = std::map<int, FMOD::Channel*>;
    using EventMap = std::map<std::string, FMOD::Studio::EventInstance*>;
    using BankMap = std::map<std::string, FMOD::Studio::Bank*>;
    std::map<int, std::string> mChannelToSound; // Link channels to sound names

    EventMap mEvents;
    SoundMap mapSounds;
    ChannelMap mapChannels;

    bool isInitialized;
    float mLastMasterVolume = 1.0f; // Track the last applied master volume
};

class CAudioEngine {
public:

    static CAudioEngine& GetInstance() {
        static CAudioEngine instance;
        return instance;
    }

    static AudioCore* GetAudioCore();

    static void Init();

    bool isInitialized();

    static void Update();
    static void Shutdown();
    static int ErrorCheck(FMOD_RESULT result);

    static float GetMasterVolume();
    void SetMasterVolume(float fVolume);
    void SetSoundVolume(const std::string& strSoundName, float volume, int customChannelId = -1);
    float GetSoundVolume(const std::string& strSoundName);

    void LoadSound(const std::string& strSoundName, bool b3d = true, bool bLooping = false, bool bStream = false);
    void UnLoadSound(const std::string& strSoundName);

    int PlaySound(const std::string& strSoundName, float fPan, float volume, int customChannelId = -1);

    void SetChannelVolume(int nChannelId, float fVolumedB);

    bool isPlaying(const std::string& strSoundName);

    bool ToggleSoundLooping(const std::string& strSoundName);
    bool IsSoundLooping(const std::string& strSoundName);
    void PositionToPan(const glm::vec3& objectPosition, const glm::vec3& playerPosition, float& fPanX);

    float GetSoundPan(const std::string& strSoundName, int customChannelId = -1);
    void SetSoundPan(const std::string& strSoundName, float fPan, int entityID = -1);

    AudioCore::ChannelMap getChannelsAndSounds();
    AudioCore::ChannelMap getChannels();
    std::map<int, std::string> ListSounds() const;

    FMOD_VECTOR VectorToFmod(const Math3D::Vector3D& vPosition);

    void PauseChannel(int nChannelId, bool bPause);
    void PauseAllChannels(bool bPause);
    void PauseSoundByName(const std::string& strSoundName);
    void UnpauseSoundByName(const std::string& strSoundName);

    void StopSound(const std::string& strSoundName);

    void PauseAllSounds();     // Wrapper for pausing all sounds
    void ResumeAllSounds();    // Wrapper for resuming all sounds
};


#endif