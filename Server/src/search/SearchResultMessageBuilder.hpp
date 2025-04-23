#pragma once
#include <string>
#include "SearchDataStructures.hpp"

class SearchResultMessageBuilder {
public:
    std::string Build(const SearchResult& searchRes) const;
    std::string BuildDiff(const SearchResult& prevRes, const SearchResult& newRes) const;

private:
    void AppendFileState(std::string& msg, const FileState& fileState) const;
    void AppendClarification(std::string& msg, const SearchResult &searchRes, const std::optional<SearchResult> prevRes = std::nullopt) const;
    void AppendWords(std::string& msg, const std::unordered_map<std::string, size_t>& words) const;
};