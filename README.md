PCSX2 IPC Client Example
======
You'll find [here](https://code.govanify.com/govanify/pcsx2_ipc/)
a naive and portable implementation of PCSX2 IPC Socket
implementation.    
The PCSX2 side of things is as of yet not merged into master.  
In the meantime the patchset containing this feature is available [here](https://code.govanify.com/govanify/pcsx2_ipc/raw/branch/master/pcsx2_ipc.patch).  

A small client example is provided along with the API. It can be compiled on
Linux and MacOs by using `make` and on Windows by loading "windows-qt.pro" with
Qt Creator.

On Doxygen you can find the documentation of the API [here](@ref PCSX2Ipc).  

Have fun!  
-Gauvain "GovanifY" Roussel-Tarbouriech, 2020
