/* WARNING: C FFI bindings are not really documented, you should read this
 * binding file and the documentation of the main header file in order to
 * understand the C bindings! As this is very much thought out like a C++
 * library the bindings will get pretty repetitive... */
#pragma once

#include "pcsx2_ipc.h"
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @see PCSX2Ipc::PCSX2Ipc
 */
PCSX2Ipc *pcsx2ipc_new();

/**
 * @see PCSX2Ipc::InitializeBatch
 */
void pcsx2ipc_initialize_batch(PCSX2Ipc *v);

/**
 * @see PCSX2Ipc::FinalizeBatch
 */
int pcsx2ipc_finalize_batch(PCSX2Ipc *v);

/**
 * We always cast as uint64_t to make the bindings easier to make/use.
 * @see PCSX2Ipc::GetReply
 */
uint64_t pcsx2ipc_get_reply_read(PCSX2Ipc *v, int cmd, int place,
                                 PCSX2Ipc::IPCCommand msg);

/**
 * @see PCSX2Ipc::SendCommand
 */
void pcsx2ipc_send_command(PCSX2Ipc *v, int cmd);

/**
 * @see PCSX2Ipc::Read
 */
uint64_t pcsx2ipc_read(PCSX2Ipc *v, uint32_t address, PCSX2Ipc::IPCCommand msg,
                       bool batch);

/**
 * @see PCSX2Ipc::Write
 */
void pcsx2ipc_write(PCSX2Ipc *v, uint32_t address, uint8_t val,
                    PCSX2Ipc::IPCCommand msg, bool batch);

/**
 * @see PCSX2Ipc::~PCSX2Ipc
 */
void pcsx2ipc_delete(PCSX2Ipc *v);

/**
 * @see PCSX2Ipc::GetError
 */
PCSX2Ipc::IPCStatus pcsx2ipc_get_error(PCSX2Ipc *v);

#ifdef __cplusplus
}
#endif
