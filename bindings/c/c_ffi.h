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

PCSX2Ipc* newPCSX2Ipc();

void c_InitializeBatch(PCSX2Ipc* v);

PCSX2Ipc::BatchCommand c_FinalizeBatch(PCSX2Ipc* v);

/* We always cast as uint64_t to make the bindings easier to make/use */
uint64_t c_GetReplyReadBatch(PCSX2Ipc* v, PCSX2Ipc::BatchCommand cmd, int place, PCSX2Ipc::IPCCommand msg);

void c_SendCommandBatch(PCSX2Ipc* v, PCSX2Ipc::BatchCommand cmd);


/* ------------------------------------------------- */

uint8_t c_Read8(PCSX2Ipc* v, uint32_t address);

char* c_Read8Batch(PCSX2Ipc* v, uint32_t address);

uint16_t c_Read16(PCSX2Ipc* v, uint32_t address);

char* c_Read16Batch(PCSX2Ipc* v, uint32_t address);

uint32_t c_Read32(PCSX2Ipc* v, uint32_t address);

char* c_Read32Batch(PCSX2Ipc* v, uint32_t address);

uint64_t c_Read64(PCSX2Ipc* v, uint32_t address);

char* c_Read64Batch(PCSX2Ipc* v, uint32_t address);

/* ------------------------------------------------- */



/* ------------------------------------------------- */

void c_Write8(PCSX2Ipc* v, uint32_t address, uint8_t val);

char* c_Write8Batch(PCSX2Ipc* v, uint32_t address, uint8_t val);

void c_Write16(PCSX2Ipc* v, uint32_t address, uint16_t val);

char* c_Write16Batch(PCSX2Ipc* v, uint32_t address, uint16_t val);

void c_Write32(PCSX2Ipc* v, uint32_t address, uint32_t val);

char* c_Write32Batch(PCSX2Ipc* v, uint32_t address, uint32_t val);

void c_Write64(PCSX2Ipc* v, uint32_t address, uint64_t val);

char* c_Write64Batch(PCSX2Ipc* v, uint32_t address, uint64_t val);

/* ------------------------------------------------- */

void deletePCSX2Ipc(PCSX2Ipc* v);

#ifdef __cplusplus
}
#endif
