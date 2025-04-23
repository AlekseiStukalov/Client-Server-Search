#include "MacOSFileWatcher.hpp"
#include <filesystem>

void MacOSFileWatcher::Start() {
    ScanExistingPaths();

    CFStringRef cfPath = CFStringCreateWithCString(NULL, m_Path.c_str(), kCFStringEncodingUTF8);
    CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void**)&cfPath, 1, NULL);

    FSEventStreamContext context = {0, this, NULL, NULL, NULL};

    m_Stream = FSEventStreamCreate(NULL, &MacOSFileWatcher::HandleCallback, &context,
        pathsToWatch, kFSEventStreamEventIdSinceNow, 0.1,
        kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagNoDefer);

    CFRelease(cfPath);
    CFRelease(pathsToWatch);

    m_DispatchQueue = dispatch_queue_create("com.filewatcher.queue", DISPATCH_QUEUE_SERIAL);
    FSEventStreamSetDispatchQueue(m_Stream, m_DispatchQueue);
    FSEventStreamStart(m_Stream);
}

void MacOSFileWatcher::Stop() {
    if (m_Stream) {
        FSEventStreamStop(m_Stream);
        FSEventStreamInvalidate(m_Stream);
        FSEventStreamRelease(m_Stream);
        m_Stream = nullptr;
    }
}

void MacOSFileWatcher::HandleCallback(ConstFSEventStreamRef streamRef, void *clientCallBackInfo, size_t numEvents,
            void *eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]) {
                
    MacOSFileWatcher* watcher = static_cast<MacOSFileWatcher*>(clientCallBackInfo);
    char** paths = static_cast<char**>(eventPaths);

    for (size_t i = 0; i < numEvents; ++i) {
        std::string path(paths[i]);
        FSEventStreamEventFlags flags = eventFlags[i];

        bool existedBefore = watcher->m_KnownPaths.contains(path);
        bool existsNow = std::filesystem::exists(path);

        if (flags & kFSEventStreamEventFlagItemCreated) {
            watcher->Notify(EventType::Created, path);
            watcher->m_KnownPaths.insert(path);
        }
        else if (flags & kFSEventStreamEventFlagItemRemoved) {
            watcher->Notify(EventType::Removed, path);
            watcher->m_KnownPaths.erase(path);
        }
        else if (flags & kFSEventStreamEventFlagItemModified) {
            watcher->Notify(EventType::Modified, path);
        }
        else if (flags & kFSEventStreamEventFlagItemRenamed) {
            if (!existedBefore && existsNow) {
                watcher->Notify(EventType::Created, path);
                watcher->m_KnownPaths.insert(path);
            }
            else {
                watcher->Notify(EventType::Removed, path);
                watcher->m_KnownPaths.erase(path);
            }
        }
    }
}

void MacOSFileWatcher::ScanExistingPaths() {
    try{
        for (const auto& entry : std::filesystem::recursive_directory_iterator(m_Path, std::filesystem::directory_options::skip_permission_denied)) {
            m_KnownPaths.insert(entry.path().string());
        }
    }
    catch (const std::exception& e){}
}