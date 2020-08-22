PCSX2 IPC Client Example
======
You'll find [here](https://code.govanify.com/govanify/pcsx2_ipc/)
the reference implementation of PCSX2 IPC Socket client-side C++ API.    

Here is a list of files included in this repository:


* .clang-format: A syntax formatting definition for this project files.  
* .gitignore: A file to avoid commiting unnecessary files.  
* Doxyfile: Doxygen configuration to automatically generate our documentation.  
* meson.build: The file that is used to define how to build the
  program.  
* README.md: Take a wild guess.  
* client.cpp: An example client file using PCSX2 IPC C++ Reference API.
* pcsx2\_ipc.h: The reference implementation of PCSX2 IPC C++ API.  
* windows\_qt.pro: A Qt build definition file. Useful if you want to compile
  this example on Windows with Qt Creator.  
* bindings/: A folder containing bindings for multiple popular languages.


A small client example is provided along with the API. It can be compiled on
by using the command `meson build && cd build && ninja`. Please
refer to [meson documentation](https://mesonbuild.com/Using-with-Visual-Studio.html) if you want
to use another generator, say, Visual Studio, instead of ninja.  
Alternatively, loading the "windows-qt.pro" on Windows with Qt Creator will work just fine if you're lazy.  
Once it builds just hack on it and make whatever you want!   
If you dislike C++
[bindings in popular languages are
available](https://code.govanify.com/govanify/pcsx2_ipc/src/branch/master/bindings/).

On Doxygen you can find the documentation of the API [here](@ref PCSX2Ipc).  

Have fun!  
-Gauvain "GovanifY" Roussel-Tarbouriech, 2020
