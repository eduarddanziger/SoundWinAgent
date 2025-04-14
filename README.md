# Sound Windows Agent

Sound Agent detects and visualizes plug-and-play audio endpoint devices under Windows. It handles audio notifications and device changes.
The Sound Agent Service collects audio device information and sends it to a remote backend ASP.Net Core REST API Server (Audio Device Repository Service)

## Executables Generated
- **SoundWinAgent**: Windows Service collects audio device information and sends it to a remote server.
- **SoundAgentCli**: Command-line test CLI.

## Technologies Used
- **C++**: Core logic implementation.
- **Packages**: Poco and Cppservice vcpkg packages in order to utilize HTTP and Windows Server Manager

## Usage

**How to build:**

1. Check out the current repository and the repository commonLibsCpp;
2. Add to NuGet sources the local path to "commonLibsCpp\OutputArtifacts"
3. Set NuGet environment variable to the path of the NuGet executable.
4. Build the solution
	- "%nuget%" restore SoundAgent.sln
	- "c:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\MSBuild.exe" SoundWinAgent.sln /p:Configuration=Release /target:Rebuild -restore

**How to run:**

1. Install / Uninstall the SoundWinAgent service
	- (elevated) SoundWinAgent.exe /registerService [/startup=auto|manual|disabled]. 
	- (elevated) SoundWinAgent.exe /unregisterService
	- net start SoundWinAgent
	- net stop SoundWinAgent
2. Start / Stop the SoundWinAgent service

**Developer mode:**

- SoundWinAgent.exe can be started as a Windows CLI, too. Stop it via Ctrl-C
- SoundWinAgent.exe accepts an optional command line parameter, the URL of the backend ASP.Net Core REST API Server, e.g.:

```powershell or bash
SoundWinAgent.exe "http://localhost:5027"
```

