#include "CommandLineArgs.hpp"
#include <iostream>

CommandLineArgs::CommandLineArgs(int argc, char* argv[]){
    m_CurrentDir = std::filesystem::path(argv[0]).parent_path();
    if (argc < 2) return;
    
    m_ConfigPathStr = std::string(argv[1]);
    if (!std::filesystem::exists(m_ConfigPathStr)){
        std::cout << "Cannot use provided config path as it is not exist: " << m_ConfigPathStr << std::endl;
        m_ConfigPathStr.clear();
    }
}