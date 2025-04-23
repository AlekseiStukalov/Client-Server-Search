#pragma once

#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <optional>

#include "../ClientConnection.hpp"
#include "../ClientResponse.hpp"
#include "../MacOSFileWatcher.hpp"
#include "SearchResultMessageBuilder.hpp"
#include "SearchDataStructures.hpp"


class SearchEngine{
public:
    SearchEngine(const std::filesystem::path& searchPath, ClientResponse& resp);
    void InitialiseWatcher();

    void Run(const std::unordered_set<std::string>& patterns);
    void WaitForExit();

private:
    void JoinThreads();
    void TraverseAndAddToTheQueue(const SearchConfig& config);
    void AddPathToSearch(const SearchConfig& config);
    void AddFileToTheQueue(const SearchConfig& config);
    void AddFilesToTheQueue(const std::vector<SearchConfig>& configs);
    void ParallelWork(const std::unordered_set<std::string>& patterns);
    void SaveAndSendIfNeed(const SearchResult& res);
    bool IsResultsEqual(const SearchResult& oldRes, const SearchResult& newRes) const;
    std::optional<SearchResult> CheckFile(const SearchConfig& config, const std::unordered_set<std::string> patterns) const;
    std::optional<SearchResult> SearchInBuffer(const std::string& buffer, const std::unordered_set<std::string> patterns) const;
    std::optional<SearchResult> SearchInFile(const std::filesystem::path& path, const std::unordered_set<std::string> patterns) const;
    std::optional<SearchResult> SearchInFileByLine(const std::filesystem::path& path, const std::unordered_set<std::string> patterns) const;
    void ReportFail(const SearchConfig& config);

    std::filesystem::path m_SearchPath;
    MacOSFileWatcher m_FileWatcher;
    ClientResponse& m_ResponseManager;

    std::mutex m_SaveLock;
    std::unordered_map<std::filesystem::path, SearchResult> m_SavedResults;
    SearchResultMessageBuilder m_MessageBuilder;

    std::atomic<uint64_t> m_TotalFilesAdded;
    std::atomic<uint64_t> m_TotalFilesHandled;
    std::atomic_bool m_IsInitialSearchFinished;
    std::condition_variable m_FinishCondition;
    std::mutex m_FinishLock;

    std::queue<SearchConfig> m_SearchQueue;
    std::vector<std::thread> m_Threads;
    std::atomic_bool m_IsForceQuitRequest;
    std::condition_variable m_IdleCondition;
    std::mutex m_InputLock;
};
