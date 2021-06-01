#include "c_ffi.h"

extern "C" {

static std::vector<PINE::Shared::BatchCommand *> batch_commands;

PINE::PCSX2 *pine_pcsx2_new() { return new PINE::PCSX2(); }

PINE::RPCS3 *pine_rpcs3_new() { return new PINE::RPCS3(); }

void pine_initialize_batch(PINE::Shared *v) { return v->InitializeBatch(); }

void pine_free_datastream(char *data) { delete[] data; }

int pine_finalize_batch(PINE::Shared *v) {
    auto p_batch = new PINE::Shared::BatchCommand;
    auto batch = v->FinalizeBatch();
    p_batch->ipc_message = batch.ipc_message;
    p_batch->ipc_return = batch.ipc_return;
    p_batch->return_locations = batch.return_locations;
    batch_commands.push_back(p_batch);
    return batch_commands.size() - 1;
}

uint64_t pine_get_reply_int(PINE::Shared *v, int cmd, int place,
                            PINE::Shared::IPCCommand msg) {
    auto lcmd = PINE::Shared::BatchCommand{
        PINE::Shared::IPCBuffer{ batch_commands[cmd]->ipc_message.size,
                                 batch_commands[cmd]->ipc_message.buffer },
        PINE::Shared::IPCBuffer{ batch_commands[cmd]->ipc_return.size,
                                 batch_commands[cmd]->ipc_return.buffer },
        batch_commands[cmd]->return_locations
    };
    switch (msg) {
        case PINE::Shared::MsgRead8:
            return (uint64_t)v->GetReply<PINE::Shared::MsgRead8>(lcmd, place);
        case PINE::Shared::MsgRead16:
            return (uint64_t)v->GetReply<PINE::Shared::MsgRead16>(lcmd, place);
        case PINE::Shared::MsgRead32:
            return (uint64_t)v->GetReply<PINE::Shared::MsgRead32>(lcmd, place);
        case PINE::Shared::MsgRead64:
            return v->GetReply<PINE::Shared::MsgRead64>(lcmd, place);
        default:
            return 0;
    }
}

void pine_send_command(PINE::Shared *v, int cmd) {
    auto lcmd = PINE::Shared::BatchCommand{
        PINE::Shared::IPCBuffer{ batch_commands[cmd]->ipc_message.size,
                                 batch_commands[cmd]->ipc_message.buffer },
        PINE::Shared::IPCBuffer{ batch_commands[cmd]->ipc_return.size,
                                 batch_commands[cmd]->ipc_return.buffer },
        batch_commands[cmd]->return_locations
    };
    return v->SendCommand(lcmd);
}

uint64_t pine_read(PINE::Shared *v, uint32_t address,
                   PINE::Shared::IPCCommand msg, bool batch) {
    if (!batch) {
        switch (msg) {
            case PINE::Shared::MsgRead8:
                return (uint64_t)v->Read<uint8_t>(address);
            case PINE::Shared::MsgRead16:
                return (uint64_t)v->Read<uint16_t>(address);
            case PINE::Shared::MsgRead32:
                return (uint64_t)v->Read<uint32_t>(address);
            case PINE::Shared::MsgRead64:
                return v->Read<uint64_t>(address);
            default:
                return 0;
        }
    } else {
        switch (msg) {
            case PINE::Shared::MsgRead8:
                v->Read<uint8_t, true>(address);
                return 0;
            case PINE::Shared::MsgRead16:
                v->Read<uint16_t, true>(address);
                return 0;
            case PINE::Shared::MsgRead32:
                v->Read<uint32_t, true>(address);
                return 0;
            case PINE::Shared::MsgRead64:
                v->Read<uint64_t, true>(address);
                return 0;
            default:
                return 0;
        }
    }
}

void pine_write(PINE::Shared *v, uint32_t address, uint64_t val,
                PINE::Shared::IPCCommand msg, bool batch) {
    if (!batch) {
        switch (msg) {
            case PINE::Shared::MsgWrite8:
                v->Write<uint8_t>(address, (uint8_t)val);
                break;
            case PINE::Shared::MsgWrite16:
                v->Write<uint16_t>(address, (uint16_t)val);
                break;
            case PINE::Shared::MsgWrite32:
                v->Write<uint32_t>(address, (uint32_t)val);
                break;
            case PINE::Shared::MsgWrite64:
                v->Write<uint64_t>(address, val);
                break;
            default:
                break;
        }
    } else {
        switch (msg) {
            case PINE::Shared::MsgWrite8:
                v->Write<uint8_t, true>(address, (uint8_t)val);
                break;
            case PINE::Shared::MsgWrite16:
                v->Write<uint16_t, true>(address, (uint16_t)val);
                break;
            case PINE::Shared::MsgWrite32:
                v->Write<uint32_t, true>(address, (uint32_t)val);
                break;
            case PINE::Shared::MsgWrite64:
                v->Write<uint64_t, true>(address, val);
                break;
            default:
                break;
        }
    }
}

char *pine_version(PINE::Shared *v, bool batch) {
    if (batch) {
        v->Version<true>();
        return nullptr;
    } else {
        return v->Version<false>();
    }
}

PINE::Shared::EmuStatus pine_status(PINE::Shared *v, bool batch) {
    if (batch) {
        v->Status<true>();
        return (PINE::Shared::EmuStatus)0;
    } else {
        return v->Status<false>();
    }
}

char *pine_getgametitle(PINE::Shared *v, bool batch) {
    if (batch) {
        v->GetGameTitle<true>();
        return nullptr;
    } else {
        return v->GetGameTitle<false>();
    }
}

char *pine_getgameid(PINE::Shared *v, bool batch) {
    if (batch) {
        v->GetGameID<true>();
        return nullptr;
    } else {
        return v->GetGameID<false>();
    }
}

char *pine_getgameuuid(PINE::Shared *v, bool batch) {
    if (batch) {
        v->GetGameUUID<true>();
        return nullptr;
    } else {
        return v->GetGameUUID<false>();
    }
}

char *pine_getgameversion(PINE::Shared *v, bool batch) {
    if (batch) {
        v->GetGameVersion<true>();
        return nullptr;
    } else {
        return v->GetGameVersion<false>();
    }
}

void pine_savestate(PINE::Shared *v, uint8_t slot, bool batch) {
    if (batch) {
        v->SaveState<true>(slot);
    } else {
        v->SaveState<false>(slot);
    }
}

void pine_loadstate(PINE::Shared *v, uint8_t slot, bool batch) {
    if (batch) {
        v->LoadState<true>(slot);
    } else {
        v->LoadState<false>(slot);
    }
}

PINE::Shared::IPCStatus pine_get_error(PINE::Shared *v) {
    return v->GetError();
}

void pine_free_batch_command(int cmd) {
    if (batch_commands[cmd] != NULL) {
        delete[] batch_commands[cmd]->ipc_message.buffer;
        delete[] batch_commands[cmd]->ipc_return.buffer;
        delete[] batch_commands[cmd]->return_locations;
        delete batch_commands[cmd];
        batch_commands[cmd] = NULL;
    }
}
void pine_pcsx2_delete(PINE::PCSX2 *v) {
    for (long unsigned int i = 0; i < batch_commands.size(); i++)
        pine_free_batch_command(i);
    delete v;
}

void pine_rpcs3_delete(PINE::RPCS3 *v) {
    for (long unsigned int i = 0; i < batch_commands.size(); i++)
        pine_free_batch_command(i);
    delete v;
}
}
