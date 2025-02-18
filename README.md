# Network Traffic Monitor & DLL Injector

A Windows application for monitoring network traffic through DLL injection and Winsock function hooking.

## Project Structure 

## Features

### Network Monitoring (MyDll)
- Hooks common Winsock functions:
  - send/sendto
  - recv/recvfrom
  - WSASend/WSARecv
- Monitors network traffic in real-time
- Supports specific target applications:
  - Target1.exe
  - Target2.exe
  - Target3.exe

### Injector Application (Unlocker)
- GUI-based injector using MFC
- Supports x64 processes
- Process creation and DLL injection capabilities
- Process monitoring functionality

## Requirements

- Windows operating system
- Visual Studio 2022 or later
- Platform: x64
- MFC support
- Windows SDK

## Building

1. Open the solution in Visual Studio
2. Select Release/x64 configuration
3. Build the entire solution
   - MyDll will build the network monitoring DLL
   - Unlocker will build the injector application

## Usage

1. Build the project
2. Place the built DLL in a known location
3. Launch the Unlocker application
4. Click "Inject" to inject the DLL into the target process

## Technical Details

### DLL Injection Process
1. Creates target process in suspended state
2. Allocates memory in target process
3. Writes DLL path to allocated memory
4. Creates remote thread to load DLL
5. Resumes target process

### Network Hooking
- Uses Mhook library for function hooking
- Intercepts Winsock functions
- Logs network traffic data

## Security Considerations

- Requires administrative privileges
- Target process must be x64
- DLL path must be accessible to target process
- Use with caution as DLL injection can affect system stability

## Dependencies

- Mhook library for function hooking
- Windows SDK
- MFC libraries
- WinSock2 libraries

## Known Limitations

- x64 processes only
- Specific target applications only
- Windows platform dependent
- Administrative privileges required

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

MIT License

## Disclaimer

This tool is for educational and research purposes only. Use at your own risk.
