/* WARNING: C FFI bindings are not really documented, you should read this
 * binding file and the documentation of the main header file in order to
 * understand the C bindings! As this is very much thought out like a C++
 * library the bindings will get pretty repetitive... */
#pragma once

#include "pcsx2_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PCSX2Ipc PCSX2Ipc;

/**
 * @see PCSX2Ipc::PCSX2Ipc
 */
PCSX2Ipc *newPCSX2Ipc();

/**
 * @see PCSX2Ipc::InitializeBatch
 */
void InitializeBatch(PCSX2Ipc *v);

/**
 * @see PCSX2Ipc::FinalizeBatch
 */
PCSX2Ipc::BatchCommand FinalizeBatch(PCSX2Ipc *v);

/**
 * We always cast as uint64_t to make the bindings easier to make/use. 
 * @see PCSX2Ipc::GetReply
 */
uint64_t GetReplyRead(PCSX2Ipc *v, PCSX2Ipc::BatchCommand cmd, int place,
                      PCSX2Ipc::IPCCommand msg);

/**
 * @see PCSX2Ipc::SendCommand
 */
void SendCommand(PCSX2Ipc *v, PCSX2Ipc::BatchCommand cmd);

/**
 * @see PCSX2Ipc::Read
 */
uint64_t Read(PCSX2Ipc *v, uint32_t address, PCSX2Ipc::IPCCommand msg,
              bool batch);

/**
 * @see PCSX2Ipc::Write
 */
void Write(PCSX2Ipc *v, uint32_t address, uint8_t val, PCSX2Ipc::IPCCommand msg,
           bool batch);

/**
 * @see PCSX2Ipc::~PCSX2Ipc
 */
void deletePCSX2Ipc(PCSX2Ipc *v);

/**
 * @see PCSX2Ipc::GetError
 */
PCSX2Ipc::IPCStatus GetError(PCSX2Ipc *v);

#ifdef __cplusplus
}
#endif
