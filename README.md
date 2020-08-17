PCSX2 IPC Client Example
======
You'll find [here](https://code.govanify.com/govanify/pcsx2_ipc/)
the reference implementation of PCSX2 IPC Socket client-side C++ API.    

Here is a list of files included in this repository:

* utils/pnach\_to\_ipc.py: Converts pnach scripts to IPC cpp files
  automatically.   
* .clang-format: A syntax formatting definition for this project files.  
* .gitignore: A file to avoid commiting unnecessary files.  
* Doxyfile: Doxygen configuration to automatically generate our documentation.  
* Makefile: The file that is used on unix-like system to define how to build the
  program.  
* README.md: Take a wild guess.  
* client.cpp: An example client file using PCSX2 IPC C++ Reference API.
* ideas.md: Ideas for future incremental improvements to the protocol.  
* pcsx2\_ipc.h: The reference implementation of PCSX2 IPC C++ API.  
* pcsx2\_ipc.patch: The patchset containing this feature. Will be removed when
  merged into master, you can track the status of this PR
  [here](https://github.com/PCSX2/pcsx2/pull/3591).
* windows\_qt.pro: A Qt build definition file. Useful if you want to compile
  this example on Windows.  
* default.nix: A file to setup the environment necessary to compile, run scripts
  and generate documentation on NixOS. Just run `nix-shell`.


A small client example is provided along with the API. It can be compiled on
Linux and MacOs by using `make` and on Windows by loading "windows-qt.pro" with
Qt Creator.  

On Doxygen you can find the documentation of the API [here](@ref PCSX2Ipc).  

Have fun!  
-Gauvain "GovanifY" Roussel-Tarbouriech, 2020
