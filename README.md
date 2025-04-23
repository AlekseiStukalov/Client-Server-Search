# C++ Client-Server Application

## Overview

This repository contains a C++ implementation of a client-server application. Both the client and servers are console-based programs. The architecture follows a one-client, multiple-servers model.

Each server waits for a client connection. Upon connection, the server searches for a specified words within a given directory, including all subdirectories and file names (excluding the root directory itself). If a match is found, the server sends a report to the client indicating the path and the matched word.

## Limitations

- Works only on macOS (relies on UNIX sockets and macOS-specific APIs for file system event watching).
- On a single machine, server ports start at 4000 and increment by 1 for each instance.
- The search is limited to the directory specified at server startup. Modifying or deleting this directory may cause undefined behavior.
- If the specified directory on the server contains cyclic references (e.g., due to symbolic links, hard links, or bind mounts), the behavior is undefined.
- The client reads the configuration file path from command-line arguments. If no path is provided, it looks for `config.json` in the same directory as the executable.
- Maximum JSON size that the client can send is 64 KB.

## Requirements

- C++20 or newer  
- make

## Build and Usage

To build the project, follow these steps:

1. Open a terminal and navigate to the root directory of the project:

    ```bash
    cd /path/to/project
    ```

2. Run the `make` command to compile both the client and server applications. This will produce two executable files near the makefile:

    ```bash
    jbha_server
    jbha_client
    ```

3. Create a JSON configuration file in the root directory and save it as `config.json`, or use a custom filename. Example:

    ```bash
    {
        "servers": [
            "127.0.0.1:4000",
            "127.0.0.1:4001"
        ],
        "searchWords": [
            "word1",
            "word2"
        ]
    }
    ```

4. Each server instance must be launched in a separate terminal window. Run the following command in each terminal. The port argument is optional:
    ```bash
    ./jbha_server /path/to/search/directory [port]
    ```

5. In a separate terminal, run the client. You can optionally provide a path to the config file:

    ```bash
    ./jbha_client [/config/path]
    ```

6. To stop the applications, manually close the terminal windows or press 'Ctrl+C'.

7. (Optional) To clean up all build artifacts, use:
    ```bash
    make clean
    ```

## Possible Improvements

- Add support for multiple concurrent clients (make `SearchEngine` shared across clients to handle concurrent requests without spawning separate instances).
- Add detection and handling of cyclic directory structures.
- Use a different file watching system to properly handle moved and renamed files (this would also remove the macOS-only limitation).
- Introduce caching on the server for previously found words.
- Add a background thread on the server to properly detect disconnected clients.
- Implement periodic status messages from the server to indicate ongoing searches (or simply return a `PONG` response).
