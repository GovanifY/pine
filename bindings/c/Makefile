all:
	$(CXX) -I../../ -std=c++17 -fPIC -c c_ffi.cpp -o ffi.o 
	$(CXX) -shared -o libpcsx2_ipc_c.so ffi.o 
