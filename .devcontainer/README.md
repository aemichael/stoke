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

## Notes

- The container uses the `stanfordpl/stoke-base:latest` image which includes pre-compiled Z3 and CVC4
- Development tools are added on top of the base environment
- The workspace is bind-mounted for fast file system access
- Container will stay running with `tail -f /dev/null` when not actively used