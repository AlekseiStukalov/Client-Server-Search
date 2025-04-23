#include "JsonParser.hpp"
#include <fstream>
#include <sstream>

void JsonParser::Parse(const std::ifstream& file) {
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json = buffer.str();

    size_t i = 0;
    SkipWhitespace(json, i);

    if (json[i] != '{') {
        throw std::runtime_error("Wrong json formst. Expected { at beginning");
    }
    ++i;

    while (i < json.size()) {
        SkipWhitespace(json, i);
        if (json[i] == '}') {
            break;
        }

        std::string key = ParseString(json, i);
        SkipWhitespace(json, i);
        if (json[i] != ':'){
            throw std::runtime_error("Wrong json formst. Expected ':' after name at idx = " + std::to_string(i));
        }

        ++i;
        SkipWhitespace(json, i);

        if (key == "searchWords") {
            searchWords = ParseArray(json, i);
        } else if (key == "servers") {
            servers = ParseArray(json, i);
        } else {
            skipValue(json, i);
        }

        SkipWhitespace(json, i);
        if (json[i] == ','){
            ++i;
        }
    }
}

void JsonParser::SkipWhitespace(const std::string &s, size_t &i) const {
    while (i < s.size() && std::isspace(s[i])) ++i;
}

void JsonParser::skipValue(const std::string &s, size_t &i) const {
    if (s[i] == '"') {
        ParseString(s, i);
    } else if (s[i] == '[') {
        int depth = 1;
        ++i;
        while (i < s.size() && depth > 0) {
            if (s[i] == '[') ++depth;
            else if (s[i] == ']') --depth;
            ++i;
        }
    } else {
        throw std::runtime_error("Wrong json formst. Unsupported value type at idx = " + std::to_string(i));
    }
}

std::string JsonParser::ParseString(const std::string &s, size_t &i) const {
    if (s[i] != '"'){
        throw std::runtime_error("Wrong json formst. Expected string at idx = " + std::to_string(i));
    }
    ++i;

    std::string result;
    while (i < s.size() && s[i] != '"') {
        if (s[i] == '\\') ++i;
        result += s[i++];
    }

    if (i >= s.size() || s[i] != '"'){
        throw std::runtime_error("Wrong json formst. Unterminated string at idx = " + std::to_string(i));
    }
    ++i;
    return result;
}

std::vector<std::string> JsonParser::ParseArray(const std::string &s, size_t &i) const {
    std::vector<std::string> result;
    if (s[i] != '['){
        throw std::runtime_error("Wrong json formst. Expected '[' at idx = " + std::to_string(i));
    }
    ++i;

    while (true) {
        SkipWhitespace(s, i);
        if (s[i] == ']') {
            ++i;
            break;
        }
        result.push_back(ParseString(s, i));
        SkipWhitespace(s, i);
        if (s[i] == ',') ++i;
    }

    return result;
}
