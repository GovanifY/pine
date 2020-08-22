Here is a quick explanation of the files available here:

* utils/pnach\_to\_ipc.py: Converts pnach scripts to IPC cpp files
  automatically.   
* utils/pcsx2\_ipc.patch: The patchset containing this feature. Will be removed when
  merged into master, you can track the status of this PR
  [here](https://github.com/PCSX2/pcsx2/pull/3591).
* utils/build-release.sh: A script used to generate the release zip files.
* utils/ideas.md: Ideas for future incremental improvements to the protocol.  
* utils/default.nix: A file to setup the environment necessary to compile, run scripts
  and generate documentation on NixOS. Just run `nix-shell`.
