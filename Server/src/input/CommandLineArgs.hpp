#pragma once
#include <filesystem>

class CommandLineArgs{
public:
    CommandLineArgs(int argc, char* argv[]);

    std::filesystem::path m_SearchPath;
    int m_Port;
};