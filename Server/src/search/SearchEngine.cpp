#include "SearchEngine.hpp"
#include <iostream>
#include <fstream>
#include <chrono>

SearchEngine::SearchEngine(const std::filesystem::path& searchPath, ClientResponse& resp): 
    m_SearchPath(searchPath),
    m_FileWatcher(searchPath.string()),
    m_ResponseManager(resp) {
}

void SearchEngine::InitialiseWatcher(){
    m_FileWatcher.OnCreated([this](const std::string& path) {
        SearchConfig config {
            .path = path,
            .dataType = DataType::All,
            .fileState = FileState::Created,
        };
        
        AddPathToSearch(config);
    });

    m_FileWatcher.OnDeleted([this](const std::string& path) {
        SearchConfig config {
            .path = path,
            .dataType = DataType::FileName,
            .fileState = FileState::Removed,
        };

        AddFileToTheQueue(config);
    });

    m_FileWatcher.OnModified([this](const std::string& path) {
        SearchConfig config {
            .path = path,
            .dataType = DataType::Content,
            .fileState = FileState::Modified,
        };
        
        AddFileToTheQueue(config);
    });

    m_FileWatcher.Start();
}

void SearchEngine::Run(const std::unordered_set<std::string>& patterns) {
    try{
        if (!std::filesystem::exists(m_SearchPath)){
            m_ResponseManager.SendMessage("No such file or directory: " + m_SearchPath.string());
            return;
        }
    
        m_ResponseManager.SendMessage("Initiated search in directory: " + m_SearchPath.string());
        std::cout << "Client " << m_ResponseManager.GetClientIp() << " initiated search in directory: " << m_SearchPath.string() << std::endl;

        m_IsForceQuitRequest.store(false);
        m_IsInitialSearchFinished.store(false);

        InitialiseWatcher();
    
        int threadsCount = std::max(static_cast<int>(std::thread::hardware_concurrency()) - 1, 2);
        for (int i = 0; i < threadsCount; ++i){
            m_Threads.emplace_back(&SearchEngine::ParallelWork, this, patterns);
        }
    
        SearchConfig config {
            .path = m_SearchPath,
            .dataType = DataType::All,
            .fileState = FileState::Inital,
        };
    
        TraverseAndAddToTheQueue(config);

        std::unique_lock<std::mutex> lock(m_FinishLock);
        m_FinishCondition.wait(lock, [this](){ return m_TotalFilesAdded == m_TotalFilesHandled; });
        m_ResponseManager.SendMessage("Search in direcory " + m_SearchPath.string() + " has finished.");
        m_IsInitialSearchFinished.store(true);
    }
    catch (const std::exception &e) {
        std::cerr << "Unexpected exception: " << e.what() << std::endl;
    }
}

void SearchEngine::WaitForExit(){
    m_FileWatcher.Stop();
    m_IsForceQuitRequest.store(true);
    m_IdleCondition.notify_all();
    JoinThreads();
    if (!m_IsInitialSearchFinished.load()){
        std::cout << "Search was interrupted by client" << std::endl;
    }
}

void SearchEngine::JoinThreads() {
    for (std::thread& t : m_Threads){
        if (t.joinable()){
            t.join();
        }
    }
}

void SearchEngine::TraverseAndAddToTheQueue(const SearchConfig& config) {
    std::vector<SearchConfig> batchFiles;
    constexpr size_t batchSize = 10;
    batchFiles.reserve(batchSize);

    std::stack<std::filesystem::path> dirs;
    std::unordered_set<std::string> visitedDirs;
    dirs.push(config.path);

    while (!dirs.empty()) {
        std::filesystem::path currentDir = dirs.top();
        dirs.pop();

        std::error_code ec;
        std::filesystem::path canonicalDir = std::filesystem::weakly_canonical(currentDir, ec);
        if (ec){
            continue;
        }

        if (!visitedDirs.insert(canonicalDir.string()).second) {
            continue;
        }

        std::filesystem::directory_iterator dirIt(currentDir, ec);
        if (ec) {
            continue;
        }

        for (const std::filesystem::directory_entry& entry : dirIt) {
            DataType type = DataType::FileName;
            if (entry.is_directory(ec)) {
                if (!ec) {
                    dirs.push(entry.path());
                }

                std::filesystem::directory_iterator test(entry.path(), ec);
                type = !ec ? config.dataType : DataType::FileName;
            }
            else{
                std::filesystem::perms p = std::filesystem::status(entry.path(), ec).permissions();
                if (!ec && ((p & std::filesystem::perms::owner_read) != std::filesystem::perms::none)) {
                    type = config.dataType;
                }
            }

            batchFiles.emplace_back(entry.path(), type, config.fileState);

            if (m_IsForceQuitRequest.load()){
                return;
            }

            if (batchFiles.size() == batchSize) {
                AddFilesToTheQueue(batchFiles);
                batchFiles.clear();
            }
        }
    }

    if (!batchFiles.empty()) {
        AddFilesToTheQueue(batchFiles);
    }
}

void SearchEngine::AddPathToSearch(const SearchConfig& config) {
    try {
        if (std::filesystem::is_directory(config.path)){
            TraverseAndAddToTheQueue(config);
        }
        else{
            AddFileToTheQueue(config);
        }
    }
    catch(const std::exception& e){}
}

void SearchEngine::AddFileToTheQueue(const SearchConfig& config) {
    m_InputLock.lock();
    m_SearchQueue.push(config);
    ++m_TotalFilesAdded;
    m_InputLock.unlock();

    m_IdleCondition.notify_one();
}

void SearchEngine::AddFilesToTheQueue(const std::vector<SearchConfig> &configs){
    m_InputLock.lock();
    for (const SearchConfig& config : configs){
        m_SearchQueue.push(config);
        ++m_TotalFilesAdded;
    }
    m_InputLock.unlock();

    m_IdleCondition.notify_all();
}

void SearchEngine::ParallelWork(const std::unordered_set<std::string>& patterns){
    while (true){
        SearchConfig searchConfig;
        std::unique_lock<std::mutex> lock {m_InputLock};
        m_IdleCondition.wait(lock, [&]() { return !m_SearchQueue.empty() || m_IsForceQuitRequest.load(); });

        if (m_IsForceQuitRequest.load()){
            return;
        }
        
        searchConfig = std::move(m_SearchQueue.front());
        m_SearchQueue.pop();
        lock.unlock();
    
        try {
            std::optional<SearchResult> fileResults = CheckFile(searchConfig, patterns);
    
            if (fileResults.has_value()){
                SaveAndSendIfNeed(fileResults.value());
            }
            else if (m_SavedResults.contains(searchConfig.path)) {
                SaveAndSendIfNeed(SearchResult {{}, searchConfig});
            }
        }
        catch (const std::exception& e ){
            ReportFail(searchConfig);
        }

        ++m_TotalFilesHandled;
        m_FinishCondition.notify_one();
    }
}

void SearchEngine::SaveAndSendIfNeed(const SearchResult &res){
    std::lock_guard<std::mutex> lock { m_SaveLock };

    bool wasSaved = m_SavedResults.contains(res.resultState.path);

    if (res.resultState.fileState == FileState::Removed){
        if (wasSaved){
            m_SavedResults.erase(res.resultState.path);
            m_ResponseManager.SendMessage(m_MessageBuilder.Build(res));
        }
        return;
    }

    if (res.resultState.fileState != FileState::Modified){
        if (wasSaved){
            m_SavedResults.erase(res.resultState.path);
        }
        m_SavedResults.emplace(res.resultState.path, res);
        m_ResponseManager.SendMessage(m_MessageBuilder.Build(res));
        return;
    }

    //modified
    if (wasSaved){
        bool isEqualToPrev = IsResultsEqual(m_SavedResults[res.resultState.path], res);
        if (!isEqualToPrev){
            m_ResponseManager.SendMessage(m_MessageBuilder.BuildDiff(m_SavedResults[res.resultState.path], res));
            m_SavedResults[res.resultState.path] = res;
        }
    }
    else{
        m_ResponseManager.SendMessage(m_MessageBuilder.Build(res));
        m_SavedResults.emplace(res.resultState.path, res);
    }
}

bool SearchEngine::IsResultsEqual(const SearchResult &oldRes, const SearchResult &newRes) const {
    if (oldRes.wordToPos.size() != newRes.wordToPos.size()) {
        return false;
   }

   for (const auto& [word, _] : oldRes.wordToPos) {
       if (!newRes.wordToPos.contains(word)) {
           return false;
       }
   }

   return true;
}

std::optional<SearchResult> SearchEngine::CheckFile(const SearchConfig& config, const std::unordered_set<std::string> patterns) const {
    if (config.fileState == FileState::Removed){
        return SearchResult {{}, config};
    }

    if (config.dataType == DataType::All || config.dataType == DataType::FileName){
        if (std::optional<SearchResult> res = SearchInBuffer(config.path.filename(), patterns)){
            res->resultState = config;
            res->resultState.dataType = DataType::FileName;
            return res;
        }

        if (config.dataType == DataType::FileName){
            return std::nullopt;
        }
    }

    if (std::filesystem::is_directory(config.path)){
        return std::nullopt;
    }

    //if (std::optional<SearchResult> res = SearchInFile(config.path, patterns)){
    if (std::optional<SearchResult> res = SearchInFileByLine(config.path, patterns)){
        res->resultState = config;
        return res;
    }

    return std::nullopt;
}

std::optional<SearchResult> SearchEngine::SearchInBuffer(const std::string& buffer, const std::unordered_set<std::string> patterns) const {
    SearchResult result;

    for (const std::string& word : patterns) {
        size_t pos = buffer.find(word);
        if (pos != std::string::npos) {
            result.wordToPos.emplace(word, pos);
        }

        if (m_IsForceQuitRequest.load()){
            break;
        }
    }

    return result.wordToPos.empty() ? std::nullopt : std::make_optional(std::move(result));
}

std::optional<SearchResult> SearchEngine::SearchInFile(const std::filesystem::path &path, const std::unordered_set<std::string> patterns) const {
    std::ifstream file {path, std::ios::binary | std::ios::ate};
    if (!file) {
        return std::nullopt;
    }

    std::streamsize size = file.tellg();
    std::string buffer(size, '\0');

    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), size);

    std::streamsize bytesRead = file.gcount();
    if (bytesRead > 0) {
        buffer.resize(bytesRead);

        std::transform(buffer.begin(), buffer.end(), buffer.begin(), [](unsigned char c) { return std::tolower(c); });

        return SearchInBuffer(buffer, patterns);
    }

    return std::nullopt;
}

std::optional<SearchResult> SearchEngine::SearchInFileByLine(const std::filesystem::path &path, const std::unordered_set<std::string> patterns) const {
    std::ifstream file{path};
    if (!file) {
        return std::nullopt;
    }

    SearchResult result;
    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;

        std::string lowerLine = line;
        std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), [](unsigned char c) { return std::tolower(c); });

        for (const std::string& pattern : patterns) {
            if (!result.wordToPos.contains(pattern)){
                if (lowerLine.find(pattern) != std::string::npos) {
                    result.wordToPos.emplace(pattern, lineNumber);
                }
            }
        }

        if (result.wordToPos.size() == patterns.size()){
            break;
        }

        if (m_IsForceQuitRequest.load()){
            break;
        }
    }

    return result.wordToPos.empty() ? std::nullopt : std::make_optional(std::move(result));
}

void SearchEngine::ReportFail(const SearchConfig &config) {
    if (!config.path.empty()){
        SearchResult res { .resultState = config };
        res.resultState.fileState = FileState::ScanFail;
        m_ResponseManager.SendMessage(m_MessageBuilder.Build(res));
    }
}
