/* WARNING: C FFI bindings are not really documented, you should read this
 * binding file and the documentation of the main header file in order to
 * understand the C bindings! As this is very much thought out like a C++
 * library the bindings will get pretty repetitive... */

#include "c_ffi.h"

extern "C" {
PCSX2Ipc *newPCSX2Ipc() { return new PCSX2Ipc(); }

void InitializeBatch(PCSX2Ipc *v) { return v->InitializeBatch(); }

PCSX2Ipc::BatchCommand FinalizeBatch(PCSX2Ipc *v) { return v->FinalizeBatch(); }

/* We always cast as uint64_t to make the bindings easier to make/use */
uint64_t GetReplyRead(PCSX2Ipc *v, PCSX2Ipc::BatchCommand cmd, int place,
                      PCSX2Ipc::IPCCommand msg) {
    switch (msg) {
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

void SendCommand(PCSX2Ipc *v, PCSX2Ipc::BatchCommand cmd) {
    return v->SendCommand(cmd);
}

uint64_t Read(PCSX2Ipc *v, uint32_t address, PCSX2Ipc::IPCCommand msg,
              bool batch) {
    if (batch == false) {
        switch (msg) {
            case PCSX2Ipc::MsgRead8:
                return (uint64_t)v->Read<uint8_t>(address);
            case PCSX2Ipc::MsgRead16:
                return (uint64_t)v->Read<uint16_t>(address);
            case PCSX2Ipc::MsgRead32:
                return (uint64_t)v->Read<uint32_t>(address);
            case PCSX2Ipc::MsgRead64:
                return v->Read<uint64_t>(address);
            default:
                return 0;
        }
    } else {
        switch (msg) {
            case PCSX2Ipc::MsgRead8:
                v->Read<uint8_t, true>(address);
                return 0;
            case PCSX2Ipc::MsgRead16:
                v->Read<uint16_t, true>(address);
                return 0;
            case PCSX2Ipc::MsgRead32:
                v->Read<uint32_t, true>(address);
                return 0;
            case PCSX2Ipc::MsgRead64:
                v->Read<uint64_t, true>(address);
                return 0;
            default:
                return 0;
        }
    }
}

void Write(PCSX2Ipc *v, uint32_t address, uint8_t val, PCSX2Ipc::IPCCommand msg,
           bool batch) {
    if (batch == false) {
        switch (msg) {
            case PCSX2Ipc::MsgWrite8:
                v->Write<uint8_t>(address, val);
            case PCSX2Ipc::MsgWrite16:
                v->Write<uint16_t>(address, val);
            case PCSX2Ipc::MsgWrite32:
                v->Write<uint32_t>(address, val);
            case PCSX2Ipc::MsgWrite64:
                v->Write<uint64_t>(address, val);
        }
    } else {
        switch (msg) {
            case PCSX2Ipc::MsgWrite8:
                v->Write<uint8_t, true>(address, val);
            case PCSX2Ipc::MsgWrite16:
                v->Write<uint16_t, true>(address, val);
            case PCSX2Ipc::MsgWrite32:
                v->Write<uint32_t, true>(address, val);
            case PCSX2Ipc::MsgWrite64:
                v->Write<uint64_t, true>(address, val);
        }
    }
}

void deletePCSX2Ipc(PCSX2Ipc *v) { delete v; }
}
