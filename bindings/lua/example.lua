local ffi = require("ffi")

local C = ffi.load('pine_c')

-- If you need more functions than that you need to copy paste the C bindings,
-- replace struct pointers by void pointers and change enums to their underlying
-- types. By default it is an int in C but I believe I've set all of them
-- explicitely
ffi.cdef[[
void *pine_pcsx2_new();
uint64_t pine_read(void *v, uint32_t address, unsigned char msg, bool batch);
void pine_pcsx2_delete(void *v);
unsigned int pine_get_error(void *v);
]]

-- we create a new PCSX2Ipc object
ipc = C.pine_pcsx2_new()

-- we read an uint8_t from memory location 0x00347D34
print(string.sub(tostring(C.pine_read(ipc, 0x00347D34, 0, false)), 1, -4))

-- we check for errors
print("Error (if any): ", tostring(C.pine_get_error(ipc)))

-- we delete the object and free the resources
C.pine_pcsx2_delete(ipc)

-- for more infos check out the C bindings documentation :D !
