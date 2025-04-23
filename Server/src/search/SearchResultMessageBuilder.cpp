#include "SearchResultMessageBuilder.hpp"

std::string SearchResultMessageBuilder::Build(const SearchResult &searchRes) const {
    std::string message;
    AppendFileState(message, searchRes.resultState.fileState);
    message += searchRes.resultState.path.string();
    AppendClarification(message, searchRes);
    return message;
}

std::string SearchResultMessageBuilder::BuildDiff(const SearchResult &prevRes, const SearchResult &newRes) const {
    std::string message;
    AppendFileState(message, FileState::Modified);
    message += newRes.resultState.path.string();
    AppendClarification(message, newRes, prevRes);
    return message;
}

void SearchResultMessageBuilder::AppendFileState(std::string &msg, const FileState& fileState) const {
    switch (fileState) {
        case FileState::Inital:
            msg += "File:  ";
            break;
        case FileState::Created:
            msg += "Created:  ";
            break;
        case FileState::Modified:
            msg += "Modified: ";
            break;
        case FileState::Removed:
            msg += "Removed:  ";
            break;
        case FileState::ScanFail:
            msg += "SCAN FAILED: ";
            break;
        default: break;
    }
}

void SearchResultMessageBuilder::AppendClarification(std::string &msg, const SearchResult &searchRes, const std::optional<SearchResult> prevRes) const {
    switch (searchRes.resultState.fileState) {
        case FileState::Inital:
        case FileState::Created:{
            msg += " - Found in file ";
            msg += (searchRes.resultState.dataType == DataType::FileName ? "name: " : "content: ");
            AppendWords(msg, searchRes.wordToPos);
            break;
        }

        case FileState::Modified:{
            if (prevRes.has_value() && !prevRes->wordToPos.empty()){
                msg += (prevRes->resultState.dataType == DataType::FileName ? " - File name contained: " : " - Contained: ");
                AppendWords(msg, prevRes->wordToPos);
            }
            else{
                msg += " - Was clear";
            }

            msg += " -> Now ";

            if (searchRes.wordToPos.empty()){
                msg += "file is clear";
                return;
            }

            msg += (searchRes.resultState.dataType == DataType::FileName ? "file name contains: " : "file contains: ");
            AppendWords(msg, searchRes.wordToPos);
            break;
        }
        case FileState::ScanFail:
        case FileState::Removed:
        default: break;
    }
}

void SearchResultMessageBuilder::AppendWords(std::string &msg, const std::unordered_map<std::string, size_t> &words) const {
    if (words.empty()){
        return;
    }
    
    for (const auto& word : words){
        msg += word.first;
        msg += ", ";
    }

    msg.pop_back();
    msg.pop_back();
}
