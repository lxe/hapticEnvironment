Just some notes for now.
Documentation here: https://mfliu.github.io/hapticEnvironment/index.html

If you are cloning this repository and need the `rpclib` library, use 
```
git submodule init
git submodule update`
```
to populate the `external/rpclib` folder
Build rpclib from the `external/rpclib` folder with:
```
mkdir build && cd build && cmake .. && make 
```

RPC Server is built into MessageHandler, both the C++ module and the Python module runs a RPC
client. Same `rpclib` for the server and client in C++, [`msgpack-rpc`](https://github.com/msgpack-rpc/msgpack-rpc-python) for Python client 

Heavily based on and inspired by: [this project](https://github.com/djoshea/haptic-control)

### Pre-built Binaries

Pre-built binaries for Windows and Linux are available on the [Releases page](../../releases). These are automatically built for:
- Every push to the main branch (nightly builds)
- Every version tag (versioned releases)

The binaries include:
- HapticEnvironment (main application)
- messageHandler (message handling service)
- chai3d-demo (CHAI3D demo application)

### Installing Dependencies

Windows installation instructions.

#### Using winget (Recommended)
1. Install Visual Studio 2022 Community Edition:
```powershell
winget install Microsoft.VisualStudio.2022.Community --override "--add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended"
```

2. Install CMake:
```powershell
winget install Kitware.CMake
```

#### Manual Installation
1. Download and install [Visual Studio 2022 Community Edition](https://visualstudio.microsoft.com/vs/community/)
   - During installation, select "Desktop development with C++"
   - Make sure to include the "Windows 10/11 SDK" and "C++ CMake tools"

2. Download and install [CMake](https://cmake.org/download/)
   - During installation, select "Add CMake to system PATH"

### Building from Source

To build on Windows, you'll need:
1. Visual Studio (2019 or later recommended)
2. CMake (3.15 or later)
3. Git
4. OpenGL

Follow these steps:

1. Clone the repository and initialize submodules:
```powershell
git clone https://github.com/lxe/hapticEnvironment.git
cd hapticEnvironment
git submodule init
git submodule update
```

2. Build the project:
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

Note: Replace "Visual Studio 17 2022" with your installed Visual Studio version if different:
- For VS 2019: use "Visual Studio 16 2019"
- For VS 2017: use "Visual Studio 15 2017"

The build will create several executables in the `build/bin/Release` directory:
- `HapticEnvironment.exe` - Main haptic environment application
- `messageHandler.exe` - Message handling service
- `chai3d-demo.exe` - CHAI3D demo application

### Usage

The main HapticEnvironment application can be run with the following arguments:
```powershell
HapticEnvironment.exe [IP_ADDRESS] [PORT] [MH_IP] [MH_PORT]
```

Parameters:
- `IP_ADDRESS`: IP address for the module (default: 127.0.0.1)
- `PORT`: Port number for the module (default: 7000)
- `MH_IP`: Message Handler IP address (default: 127.0.0.1)
- `MH_PORT`: Message Handler port number (default: 8080)

Keyboard Controls:
- `F`: Enable/Disable full screen mode
- `Q`: Exit application

The application initializes in the following sequence:
1. Display and scene initialization
2. Haptics initialization
3. Messaging system initialization
4. Connection to Message Handler
5. Trial control subscription
6. Streamer and listener startup

### Message Handler Service

The Message Handler service needs to be running before starting the main HapticEnvironment application. To run the service:

```powershell
messageHandler.exe [IP_ADDRESS] [PORT]
```

Parameters:
- `IP_ADDRESS`: IP address for the Message Handler service (default: 127.0.0.1)
- `PORT`: Port number for the Message Handler service (default: 8080)

The Message Handler service acts as an RPC server that facilitates communication between different modules. Make sure it's running and accessible before starting the main application.

Note: If you're running both the Message Handler and HapticEnvironment on the same machine, use the default settings. If running on different machines, make sure to:
1. Start the Message Handler service first with the desired IP address and port
2. Configure the main application's `MH_IP` and `MH_PORT` parameters to match the Message Handler service settings

For example, to run the Message Handler service on a specific network interface:
```powershell
messageHandler.exe 192.168.1.100 8080
```
