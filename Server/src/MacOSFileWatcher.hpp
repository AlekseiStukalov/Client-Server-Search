#include <CoreServices/CoreServices.h>
#include <dispatch/dispatch.h>
#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>

class MacOSFileWatcher {
    enum class EventType { Created, Removed, Modified };
    using Callback = std::function<void(const std::string& path)>;

public:

    MacOSFileWatcher(const std::string& path) : m_Path(path), m_Stream(nullptr) {}
    ~MacOSFileWatcher() { Stop(); }

    void OnCreated(Callback cb) { m_Callbacks[EventType::Created] = cb; }
    void OnDeleted(Callback cb) { m_Callbacks[EventType::Removed] = cb; }
    void OnModified(Callback cb) { m_Callbacks[EventType::Modified] = cb; }

    void Start();
    void Stop();

private:
    static void HandleCallback(ConstFSEventStreamRef streamRef, void* clientCallBackInfo, size_t numEvents,
        void* eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]);

    void Notify(EventType type, const std::string& path) { m_Callbacks[type](path); }
    void ScanExistingPaths();

    std::string m_Path;
    FSEventStreamRef m_Stream;
    dispatch_queue_t m_DispatchQueue;
    std::unordered_map<EventType, Callback> m_Callbacks;
    std::unordered_set<std::string> m_KnownPaths;
};
