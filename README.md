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

### Building on Windows

To build on Windows, you'll need:
1. Visual Studio (2019 or later recommended)
2. CMake (3.15 or later)
3. Git
4. OpenGL

Follow these steps:

1. Clone the repository and initialize submodules:
```powershell
git clone https://github.com/mfliu/hapticEnvironment.git
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

The build will create several executables in the `build/Release` directory:
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
