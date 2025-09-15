/**
 * @file ConfigLoading.h
 * @brief Implementation of configuration loading from an XML file.
 *
 * This file provides functionality for loading application configuration data such as screen width,
 * height, and fullscreen mode from an XML configuration file.
 *
 * Key Features:
 * - **XML Parsing**:
 *   - Uses the TinyXML2 library to parse configuration data.
 *   - Extracts values for screen width, height, and fullscreen mode.
 * - **Error Handling**:
 *   - Handles errors in loading or parsing the configuration file gracefully.
 * - **Flexible Configuration**:
 *   - Designed to load standard XML configuration files with a specific structure.
 *
 * Function:
 * - `loadConfigXML`:
 *   - Loads configuration values (`width`, `height`, `fullscreen`) from the specified XML file.
 *   - Outputs error messages if the file fails to load or parse correctly.
 *
 * Example XML Structure:
 * ```xml
 * <config>
 *     <width>800</width>
 *     <height>600</height>
 *     <fullscreen>false</fullscreen>
 * </config>
 * ```
 *
 * Author: Che Ee (100%)
 */

#pragma once
#ifndef CONFIG_LOADING_H

#define CONFIG_LOADING_H

#include "CommonIncludes.h"

void loadConfigXML(const std::string& filename, int& width, int& height, bool& fullscreen);


#endif