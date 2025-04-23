#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdexcept>

class JsonParser {
public:
    void Parse(const std::ifstream& file);

    const std::vector<std::string>& GetSearchWords() const { return searchWords; }
    const std::vector<std::string>& GetServers() const { return servers; }

private:

    void SkipWhitespace(const std::string& s, size_t& i) const;
    void skipValue(const std::string& s, size_t& i) const;
    
    std::string ParseString(const std::string& s, size_t& i) const;
    std::vector<std::string> ParseArray(const std::string& s, size_t& i) const;

    std::vector<std::string> searchWords;
    std::vector<std::string> servers;
};
