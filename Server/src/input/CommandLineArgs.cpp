#include "CommandLineArgs.hpp"
#include <iostream>

CommandLineArgs::CommandLineArgs(int argc, char* argv[]) : m_Port{0} {
    if (argc < 2) {
        throw std::runtime_error("Cannot start server. No provided path ");
    }

    m_SearchPath = std::filesystem::path(argv[1]);
    if (!std::filesystem::exists(m_SearchPath)){
        throw std::runtime_error("Cannot start server. Provided path not exists: " + m_SearchPath.string());
    }

    if (!std::filesystem::is_directory(m_SearchPath)){
        throw std::runtime_error("Cannot start server. Provided path is not a directory: " + m_SearchPath.string());
    }

    std::error_code ec;
    std::filesystem::directory_iterator test(m_SearchPath, ec);
    if (ec) {
        throw std::runtime_error("Cannot start server. No permission to search in provided deirectory: " + m_SearchPath.string() + ec.message());
    }

    if (argc < 3) return;

    m_Port = std::atoi(argv[2]);
}