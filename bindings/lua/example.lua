local ffi = require("ffi")

local C = ffi.load('pcsx2_ipc_c')

-- If you need more functions than that you need to copy paste the C bindings,
-- replace struct pointers by void pointers and change enums to their underlying
-- types. By default it is an int in C but I believe I've set all of them
-- explicitely
ffi.cdef[[
void *newPCSX2Ipc();
uint64_t Read(void *v, uint32_t address, unsigned char msg, bool batch);
void deletePCSX2Ipc(void *v);
unsigned int GetError(void *v);
]]

-- we create a new PCSX2Ipc object
ipc = C.newPCSX2Ipc()

-- we read an uint8_t from memory location 0x00347D34
print(string.sub(tostring(C.Read(ipc, 0x00347D34, 0, false)), 1, -4))

-- we check for errors
print("Error (if any): ", tostring(C.GetError(ipc)))

-- we delete the object and free the resources
C.deletePCSX2Ipc(ipc)

-- for more infos check out the C bindings documentation :D !
