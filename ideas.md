Future ideas to implement in the IPC API

to implement:  
* WaitFrames
* GetFPS
* GetGameID
* LoadSaveState
* SaveSaveState
* SetEmulationSpeed


maybe later:  
* frameskip
* client.SetSoundOn
* client.GetSoundOn
* client.pause
* client.ispaused
* client.openrom
* client.screenshot
* client.getconfig
* client.getversion

other ideas:  
* input
* renderer hooks
* stored procedures for even faster batch processing?
* MsgReadXX < for bigger blocks



NEXT:  
OnFrame:  
implementation:  
SysCoreThread: on vsync if IPC then call IpcEvent, sends a blocking socket
connection each frame to do your thing
