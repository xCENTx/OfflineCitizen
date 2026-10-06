#include <pch.h>
#include "include/config.h"

// Constructor loads the configuration from a JSON file
Config::Config(const std::string& filePath) { load(filePath); }

// load a configuration from json file
void Config::load(const std::string& filePath)
{
    std::ifstream configFile(filePath);
    if (configFile.is_open())
    {
        try
        {
            configFile >> data;
        }
        catch (const nlohmann::json::exception& e)
        {
            std::cerr << "Error reading JSON from file: " << e.what() << std::endl;
        }
    }
    else    // create default
    {
        try
        {
            defaults(filePath);
        }
        catch (const nlohmann::json::exception& e)
        {
            std::cerr << "Error saving JSON file: " << e.what() << std::endl;
        }
    }
}

// Save the configuration to a JSON file
void Config::save(const std::string& filePath) const
{
    std::ofstream configFile(filePath);
    if (configFile.is_open())
    {
        configFile << data.dump(4); // Pretty print with an indentation of 4 spaces
    }
    else
    {
        std::cerr << "Could not open file to save config." << std::endl;
    }
}

// Check if a key exists in the configuration
bool Config::contains(const std::string& key) const { return data.contains(key); }

//  creates a new g_config file with default settings
void Config::defaults(const std::string& filePath)
{
    data =
    {
        { "menu_key" , 162},         //  VK_LCONTROL
    };

    save(filePath);
}

//  creates a new configuration file using the input settings
void Config::defaults(const std::string& path, const nlohmann::json& jData)
{
    try { data = jData; }
    catch (const nlohmann::json::exception& e) { std::cerr << "Error setting JSON data: " << e.what() << std::endl; }
    save(path);
}