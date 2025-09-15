/**
 * @file ConfigLoading.cpp
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
#include "ConfigLoading.h"
#include "tinyXML/tinyxml2.h"
void loadConfigXML(const std::string& filename, int& width, int& height, bool& fullscreen) {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Failed to load config file!" << std::endl;
        return;
    }
   /* <config>
        < width>800 < / width >
        < height>600 < / height >
        <fullscreen>false< / fullscreen>
        < / config>*/

   /* tinyxml2::XMLElement* p_root_element = doc.RootElement();
    tinyxml2::XMLElement* p_title = p_root_element->FirstChildElement("width");*/
    tinyxml2::XMLElement* p_root_element = doc.RootElement();
    tinyxml2::XMLElement* root = doc.FirstChildElement("config");
    if (root) {
        root->FirstChildElement("width")->QueryIntText(&width);
        root->FirstChildElement("height")->QueryIntText(&height);

        tinyxml2::XMLElement* p_fullscreen = p_root_element->FirstChildElement("fullscreen");

        if (std::strcmp(p_fullscreen->GetText(), "false") == 0) {
            fullscreen = false;
        }
        else if (std::strcmp(p_fullscreen->GetText(), "true") == 1) {
            fullscreen = true;
        }

    }
}