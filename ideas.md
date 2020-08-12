to implement:
WaitFrames
GetFPS
GetGameID
LoadSaveState
SaveSaveState
SetEmulationSpeed


maybe later:
frameskip
client.SetSoundOn
client.GetSoundOn
client.pause
client.ispaused
client.openrom
client.screenshot
client.getconfig
client.getversion

other ideas:
input
renderer hooks



NEXT: OnFrame:
implementation:
SysCoreThread: on vsync if IPC then call IpcEvent, sends a blocking socket
connection each frame to do your thing
