/**
 * @file AssetLibrary.h
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

#pragma once
#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "CommonIncludes.h"
#include <filesystem>
#include <GL/glew.h> 
//#include "GlobalVariables.h"


//#include "AudioEngine.h"

/**
 * @brief Template class for managing a library of assets.
 *
 * This function can use to load,get assets, delete asset, get file name
 * This uses hash map to store assets,
 *
 * @T consist of audio,texture,font
 */


template <typename T>
class AssetLibrary {
public:
	AssetLibrary() = default;
	~AssetLibrary();

	void LoadAssets(const std::string& directory_path);
	std::shared_ptr<T> GetAssets(const std::string&)const;
	void DeleteAssets(const std::string& Assets_name);
	std::string GetFileName(const std::string& Asset_name) const;
	std::string GetName(const std::string& Asset_file_name) const;
	bool IsAssetLoaded(const std::string& asset_name) const;
	size_t GetLoadedAssetCount() const;
	void ListLoadedAssets() const;
	std::vector<std::pair<std::string, std::shared_ptr<T>>> GetAllLoadedAssets() const;
	void deleteallassets();
	void PruneAssets(const std::string& directory_path);

	// Only enable RefreshTextures if T = Texture
	template <typename T = Texture>
	void RefreshTextures() {
		for (auto& [name, texture] : Mem_Assets) {
			texture->RefreshTexture();
		}
	}


private:
	std::unordered_map < std::string, std::shared_ptr<T> > Mem_Assets;
};



/**
 * @brief Class for texture assets
 *
 * This class manages the loading and unloading of texture assets, handling
 * OpenGL texture ID generation and cleanup.
 */
class Texture {
public:
	Texture(const std::string& file_name)
		: Asset(file_name), textureID(0) {
		// Load the texture from the file
		textureID = LoadTextureFromFile(file_name);
		if (textureID != 0) {
		}
		else {
			// std::cout<< "Failed to load texture: " << Asset << std::endl;
		}
	}

	~Texture() {
		// Cleanup
		if (textureID != 0) {
			glDeleteTextures(1, &textureID);
		}
	}

	// Method to refresh the texture by reloading it from the file
	void RefreshTexture() {
		if (this->textureID != 0) {
			glDeleteTextures(1, &this->textureID); // Delete the old texture
		}

		this->textureID = LoadTextureFromFile(this->Asset); // Reload the texture
		if (this->textureID != 0) {
			// std::cout<< "Texture refreshed: " << this->Asset << " with new ID: " << this->textureID << std::endl;
		}
		else {
			std::cerr << "Failed to refresh texture: " << this->Asset << std::endl;
		}
	}

	GLuint GetTextureID() const {
		return textureID;
	}

	std::string GetFileName() const {
		return Asset;
	}

	int GetImageWidth() const {
		return width;
	}

	int GetImageHeight() const {
		return height;
	}


private:
	GLuint textureID; //able to store texture ID to know which assets to reuse
	std::string Asset;
	int width;
	int height;

	GLuint LoadTextureFromFile(const std::string& filename) {
		// Load image data using stb_image
		int imgWidth = 0, imgHeight = 0, channels = 0;

		// Load image data using stb_image
		unsigned char* data = stbi_load(filename.c_str(), &imgWidth, &imgHeight, &channels, 0);
		if (!data) {
			std::cerr << "Failed to load texture file: " << filename << std::endl;
			return 0; // Return 0 if loading failed
		}

		width = imgWidth;
		height = imgHeight;

		// Generate a new OpenGL texture ID
		GLuint texture;
		if (data) {
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			// Set texture parameters
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			GLenum format = (channels == 1) ? GL_RED :
				(channels == 3) ? GL_RGB :
				(channels == 4) ? GL_RGBA : 0;

			if (format != 0) {
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else {
				std::cerr << "Unsupported texture format!" << std::endl;
			}
			stbi_image_free(data);

			return texture; // Return the generated texture ID
		}
		else {
			std::cerr << "Failed to load texture: " << filename << std::endl;
			return 0;
		}
		
	}

};

/**
 * @brief Class for audio assets
 *
 * This class manages the loading and unloading of audio assets, handling
 *	this assets will be sent to load audio to play the sound
 */

class Audio {
public:
	Audio(const std::string& file_name);

	~Audio();

	std::string GetFileName() const {
		return Asset;
	}

private:
	std::string Asset;

};


/**
 * @brief Class for font assets
 *
 * This class manages the loading and unloading of font assets, handling
 *
 */
class Font {
public:

	Font(const std::string& file_name);
	~Font();

	std::string GetFileName() const {
		return Asset;
	}

private:
	std::string Asset;
};



/**
 * @brief Class for prefab assets
 *
 * This class manages loading and unloading of prefab assets, which could be predefined sets of objects or game entities.
 */
class Prefab {
public:
	Prefab(const std::string& file_name) : Asset(file_name) {
		// Load the prefab data from the file
		// LoadPrefabFromFile(file_name);
		// std::cout<< "Loaded prefab from: " << Asset << std::endl;
	}

	~Prefab() {
		// std::cout<< "Prefab destroyed: " << Asset << std::endl;
	}

	std::string GetFileName() const {
		return Asset;
	}

private:
	std::string Asset;

	void LoadPrefabFromFile(const std::string& filename) {
		(void)filename;
		// std::cout<< "Parsing prefab data from: " << filename << std::endl;
	}
};



/*

 * @brief delete all assets once not in use
 *
 * This function deletes all the assetts once out of scope
 *
 * @T refers to the different type of assets
 * @param asset_name The name of the asset to retrieve
 * @return std::shared_ptr<T> A shared pointer to the asset if found; otherwise, `nullptr`.
 */
template <typename T>
AssetLibrary<T>::~AssetLibrary() {
	deleteallassets();
}




/*
 * @brief load an assets to the ssystem
 *
 * This function search from assets in the file for the various system
 *
 * @T refers to the different type of assets
 * @param asset_name The name of the asset to retrieve
 * @return std::shared_ptr<T> A shared pointer to the asset if found; otherwise, `nullptr`.
 */
template<typename T>
void AssetLibrary<T>::LoadAssets(const std::string& directory_path) {
	if (std::filesystem::exists(directory_path) && std::filesystem::is_directory(directory_path)) {
		for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
			if (entry.is_regular_file()) {
				std::string file_path = entry.path().string();
				std::string asset_name = entry.path().filename().string();

				// Check if the asset is already loaded
				if (Mem_Assets.find(asset_name) == Mem_Assets.end()) {
					std::shared_ptr<T> asset = std::make_shared<T>(file_path);
					Mem_Assets[asset_name] = asset;
				}
				else {
					// std::cout<< "Asset already loaded: " << asset_name << std::endl;
				}
			}
		}
	}
	else {
		// std::cout<< "Directory not found: " << directory_path << std::endl;
	}
}


/*
 * @brief get an assets from the library
 *
 * This function searches the memory of loaded assest with the specified name
 * if found, return the shared pointer to the asssets,else return nullptr
 *
 * @T refers to the different type of assets
 * @param asset_name The name of the asset to retrieve
 * @return std::shared_ptr<T> A shared pointer to the asset if found; otherwise, `nullptr`.
 */
template<typename T>
std::shared_ptr<T> AssetLibrary<T>::GetAssets(const std::string& Asset_name)const {
	auto it = Mem_Assets.find(Asset_name);
	if (it != Mem_Assets.end()) {
		return it->second;
	}
	return nullptr;
}


/*
 * @brief delete assets from the library
 *
 * This function searches the memory of loaded assest with the specified name
 * if not in use, delete the assets
 *
 * @T refers to the different type of assets
 * @param Asset_name The name of the asset to retrieve
 */
template <typename T>
void AssetLibrary<T>::DeleteAssets(const std::string& Assets_name) {
	auto it = Mem_Assets.find(Assets_name);
	if (it != Mem_Assets.end()) {
		Mem_Assets.erase(it);
	}
}


/**
 * @brief Get the filename of the specified asset.
 *
 * This function filename based on the associated name
 *
 * @tparam T Refers to the different type of assets.
 *
 * @param Asset_name The name of the asset.
 * @return std::string The filename of the asset, or an empty string if not found.
 */
template <typename T>
std::string AssetLibrary<T>::GetFileName(const std::string& Asset_name) const {
	auto it = Mem_Assets.find(Asset_name);
	if (it != Mem_Assets.end()) {
		return it->second->GetFileName();
	}
	return "";
}

template <typename T>
std::string AssetLibrary<T>::GetName(const std::string& Asset_file_name) const {
	// Extract the file name
	std::string fileName = std::filesystem::path(Asset_file_name).filename().string();

	auto it = Mem_Assets.find(fileName);
	if (it != Mem_Assets.end()) {
		return fileName;
	}
	return "";
}

/**
 * @brief Check is loaded
 *
 * This prevent mutiple loaded assets
 * @tparam T The type of asset (e.g., Texture, Audio, Font).
 * @param asset_name The name of the asset to check.
 * @return bool True if the asset is loaded, false otherwise.
 */
template<typename T>
bool AssetLibrary<T>::IsAssetLoaded(const std::string& asset_name) const {
	return Mem_Assets.find(asset_name) != Mem_Assets.end();
}


/**
 * @brief Get total number of assets
 *
 *	for debugging purpose
 *
 * @tparam T The type of asset (e.g., Texture, Audio, Font).
 * @return size_t The number of assets currently loaded in memory.
 */
template<typename T>
size_t AssetLibrary<T>::GetLoadedAssetCount() const {
	return Mem_Assets.size();
}

// List all loaded assets
template<typename T>
void AssetLibrary<T>::ListLoadedAssets() const {
	for (const auto& asset : Mem_Assets) {
		// std::cout<< "- " << asset.first << std::endl;  // asset.first is the asset name
	}
}

template <typename T>
std::vector<std::pair<std::string, std::shared_ptr<T>>> AssetLibrary<T>::GetAllLoadedAssets() const {
	std::vector<std::pair<std::string, std::shared_ptr<T>>> assets;
	for (const auto& asset : Mem_Assets) {
		assets.emplace_back(asset.first, asset.second);
	}
	return assets;
}

template <typename T>
void AssetLibrary<T>::deleteallassets() {
	for (auto& [name, asset] : Mem_Assets) {
		asset.reset(); // Release shared pointer
	}
	Mem_Assets.clear();
}

template <typename T>
void AssetLibrary<T>::PruneAssets(const std::string& directory_path) {
	std::unordered_set<std::string> current_files;
	if (std::filesystem::exists(directory_path) && std::filesystem::is_directory(directory_path)) {
		for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
			if (entry.is_regular_file()) {
				current_files.insert(entry.path().filename().string());
			}
		}
	}

	// Remove assets not in the current directory
	for (auto it = Mem_Assets.begin(); it != Mem_Assets.end();) {
		if (current_files.find(it->first) == current_files.end()) {
			it = Mem_Assets.erase(it); // Remove unused asset
		}
		else {
			++it;
		}
	}
}


#endif