#pragma once

#include <unordered_set>
#include <unordered_map>
#include <filesystem>
#include "../ClientResponse.hpp"

enum class DataType {
    All,
    Content,
    FileName
};

enum class FileState {
    Inital,
    Created,
    Removed,
    Modified,
    ScanFail
};

struct SearchConfig {
    std::filesystem::path path;
    DataType dataType;
    FileState fileState;
};

struct SearchResult {
    std::unordered_map<std::string, size_t> wordToPos;
    SearchConfig resultState;
};