/* WARNING: C FFI bindings are not really documented, you should read this
 * binding file and the documentation of the main header file in order to
 * understand the C bindings! As this is very much thought out like a C++
 * library the bindings will get pretty repetitive... */

#include "c_ffi.h"

extern "C" {
PCSX2Ipc* newPCSX2Ipc() {
    return new PCSX2Ipc();
}

void c_InitializeBatch(PCSX2Ipc* v) {
    return v->InitializeBatch();
}

PCSX2Ipc::BatchCommand c_FinalizeBatch(PCSX2Ipc* v) {
    return v->FinalizeBatch();
}

/* We always cast as uint64_t to make the bindings easier to make/use */
uint64_t c_GetReplyReadBatch(PCSX2Ipc* v, PCSX2Ipc::BatchCommand cmd, int place, PCSX2Ipc::IPCCommand msg) {
    switch(msg) {
        case PCSX2Ipc::MsgRead8:
            return (uint64_t)v->GetReply<PCSX2Ipc::MsgRead8>(cmd, place);
        case PCSX2Ipc::MsgRead16:
            return (uint64_t)v->GetReply<PCSX2Ipc::MsgRead16>(cmd, place);
        case PCSX2Ipc::MsgRead32:
            return (uint64_t)v->GetReply<PCSX2Ipc::MsgRead32>(cmd, place);
        case PCSX2Ipc::MsgRead64:
            return v->GetReply<PCSX2Ipc::MsgRead64>(cmd, place);
        default:
            return 0;
    }
}

void c_SendCommandBatch(PCSX2Ipc* v, PCSX2Ipc::BatchCommand cmd) {
    return v->SendCommand(cmd);
}


/* ------------------------------------------------- */

uint8_t c_Read8(PCSX2Ipc* v, uint32_t address) {
    return v->Read<uint8_t>(address);
}

char* c_Read8Batch(PCSX2Ipc* v, uint32_t address) {
    return v->Read<uint8_t, true>(address);
}

uint16_t c_Read16(PCSX2Ipc* v, uint32_t address) {
    return v->Read<uint16_t>(address);
}

char* c_Read16Batch(PCSX2Ipc* v, uint32_t address) {
    return v->Read<uint16_t, true>(address);
}

uint32_t c_Read32(PCSX2Ipc* v, uint32_t address) {
    return v->Read<uint32_t>(address);
}

char* c_Read32Batch(PCSX2Ipc* v, uint32_t address) {
    return v->Read<uint32_t, true>(address);
}

uint64_t c_Read64(PCSX2Ipc* v, uint32_t address) {
    return v->Read<uint64_t>(address);
}

char* c_Read64Batch(PCSX2Ipc* v, uint32_t address) {
    return v->Read<uint64_t, true>(address);
}

/* ------------------------------------------------- */



/* ------------------------------------------------- */

void c_Write8(PCSX2Ipc* v, uint32_t address, uint8_t val) {
    return v->Write<uint8_t>(address, val);
}

char* c_Write8Batch(PCSX2Ipc* v, uint32_t address, uint8_t val) {
    return v->Write<uint8_t, true>(address, val);
}

void c_Write16(PCSX2Ipc* v, uint32_t address, uint16_t val) {
    return v->Write<uint16_t>(address, val);
}

char* c_Write16Batch(PCSX2Ipc* v, uint32_t address, uint16_t val) {
    return v->Write<uint16_t, true>(address, val);
}

void c_Write32(PCSX2Ipc* v, uint32_t address, uint32_t val) {
    return v->Write<uint32_t>(address, val);
}

char* c_Write32Batch(PCSX2Ipc* v, uint32_t address, uint32_t val) {
    return v->Write<uint32_t, true>(address, val);
}

void c_Write64(PCSX2Ipc* v, uint32_t address, uint64_t val) {
    return v->Write<uint64_t>(address, val);
}

char* c_Write64Batch(PCSX2Ipc* v, uint32_t address, uint64_t val) {
    return v->Write<uint64_t, true>(address, val);
}

/* ------------------------------------------------- */

void deletePCSX2Ipc(PCSX2Ipc* v){
    delete v;
}

}
