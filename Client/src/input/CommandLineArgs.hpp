#pragma once
#include <string>
#include <filesystem>

class CommandLineArgs{
public:
    CommandLineArgs(int argc, char* argv[]);

    std::filesystem::path m_CurrentDir;
    std::string m_ConfigPathStr;
};