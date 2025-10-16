# Development Container Configuration

This directory contains the configuration for developing STOKE in a VS Code devcontainer.

## Usage

1. Open this project in VS Code
2. When prompted, click "Reopen in Container" or use the command palette (Ctrl+Shift+P) and select "Dev Containers: Reopen in Container"
3. Wait for the container to build and start
4. You're ready to develop!

## What's Included

- **Base Environment**: Based on the existing STOKE Docker setup with Ubuntu 14.04
- **Development Tools**: 
  - GDB debugger with remote debugging support
  - Valgrind for memory debugging  
  - Build tools (make, cmake, gcc 4.9)
  - Utilities (tree, htop, curl, wget)
- **VS Code Extensions**:
  - C/C++ extension pack for IntelliSense and debugging
  - CMake Tools for build configuration
  - Python support for scripts
  - Assembly language support for .s files
  - Git integration

## Container Features

- **User**: Runs as `stoke` user (non-root) with sudo privileges
- **Workspace**: Mounted at `/home/stoke/stoke` 
- **PATH**: Automatically includes `/home/stoke/stoke/bin`
- **SSH**: Port 22 forwarded for remote debugging if needed
- **Debugging**: Configured with ptrace capabilities for GDB

## Building STOKE

The container comes with all dependencies pre-installed. To build STOKE:

```bash
./configure.sh
make
```

## JSON Library Fix

This container includes a fix for JSON linking issues that occur when using GCC 4.9:

### Problem
- STOKE requires GCC 4.9 which uses the old C++ ABI by default
- Modern Ubuntu's `libjsoncpp` is compiled with the new C++11 ABI
- This causes undefined reference errors like `Json::Value::asString() const`

### Solution
- Custom jsoncpp 1.7.4 compiled with GCC 4.9 and old ABI (`-D_GLIBCXX_USE_CXX11_ABI=0`)
- Located at `/usr/local/stoke/lib/libjsoncpp.a`
- Makefile automatically detects and uses the correct library

## SSH Integration

The container supports SSH agent forwarding for Git operations:

1. Ensure SSH keys are loaded on your host: `ssh-add ~/.ssh/id_ed25519`
2. Rebuild the devcontainer to apply SSH configuration
3. Git operations will work seamlessly with GitHub

Alternatively, use GitHub CLI: `gh auth login`

## Notes

- The container uses GCC 4.9 with Ubuntu 20.04 base for better compatibility
- All STOKE dependencies are pre-installed and configured
- The workspace is bind-mounted for fast file system access
- JSON linking issues are resolved with custom-compiled jsoncpp