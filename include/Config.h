#include "include/FileManager.h"

class Config
{
public:
    // loads a coonfiguration from file
    void load(const std::string& filePath);

    // Save the configuration to a file
    void save(const std::string& filePath) const;

    // Check if a key exists
    bool contains(const std::string& key) const;

public:
    // Accessor methods for configuration data
    // Get a value from the configuration, with type safety
    template <typename T>
    T get(const std::string& key) const
    {
        if (contains(key))
        {
            return data.at(key).get<T>();
        }
        throw std::runtime_error("Key not found: " + key);
    }

    // Set a value in the configuration
    template <typename T>
    void set(const std::string& key, const T& value)
    {
        if (!contains(key))
            return;

        //  set the key
        data[key] = value;
    }

    // creates a new entry in the configuration
    template<typename T>
    bool create(const std::string& path, const std::string& key, const T& value)
    {
        if (contains(key))
            return false;   //  entry already exists

        data.push_back({ key, value });

        save(path);
    }

public:
    //  default constructor
    Config() = default;

    // Constructor that loads the configuration from a file (or uses default if not found)
    explicit Config(const std::string& filePath);

protected:
    nlohmann::json data;  // Stores the JSON data

    virtual void defaults(const std::string& filePath);
    virtual void defaults(const std::string& path, const nlohmann::json& data);
};