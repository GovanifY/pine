#pragma once

/** \file c_ffi.h
 * This is the C bindings definition of the PCSX2 IPC API. @n
 * The C bindings of the API are inherently unsafe. You _can_ shoot yourself in
 * the foot, and probably will. @n
 * I encourage you to carefully read the documentation for the C++
 * library on top of the C bindings to have a proper understanding of what each
 * function does, and how the binding differs from the source material. @n
 * The function mostly behaves in the same way, with a notable difference that
 * batch commands will always return 0/nullptr.
 */

#include "pine.h"
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define EXPORT_LIB __declspec(dllexport)
#else
#define EXPORT_LIB
#endif

/**
 * @see PINE::PCSX2
 */
EXPORT_LIB PINE::PCSX2 *pine_pcsx2_new();

/**
 * @see PINE::RPCS3
 */
EXPORT_LIB PINE::RPCS3 *pine_rpcs3_new();

/**
 * @see PINE::DuckStation
 */
EXPORT_LIB PINE::DuckStation *pine_duckstation_new();

/**
 * @see PINE::Shared::InitializeBatch
 */
EXPORT_LIB void pine_initialize_batch(PINE::Shared *v);

/**
 * This function frees datastream whose ownership was passed down to you. @n
 * This is just a fancy wrapper around delete[] that is easier to use through
 * FFI.
 */
EXPORT_LIB void pine_free_datastream(char *data);

/**
 * In contrast to the C++ library this returns a handle to a struct. @n
 * This requires you to free handles by yourself, see
 * pine_free_batch_command.
 * @see pine_free_batch_command
 * @return PINE::Shared::BatchCommand handle.
 * @see PINE::Shared::FinalizeBatch
 */
EXPORT_LIB int pine_finalize_batch(PINE::Shared *v);

/**
 * Variant of PINE::Shared::GetReply that exclusively deals with integers
 * replies.
 * @see PINE::Shared::GetReply
 */
EXPORT_LIB uint64_t pine_get_reply_int(PINE::Shared *v, int cmd, int place,
                                       PINE::Shared::IPCCommand msg);

/**
 * @see PINE::Shared::SendCommand
 */
EXPORT_LIB void pine_send_command(PINE::Shared *v, int cmd);

/**
 * @see PINE::Shared::Read
 */
EXPORT_LIB uint64_t pine_read(PINE::Shared *v, uint32_t address,
                              PINE::Shared::IPCCommand msg, bool batch);

/**
 * @see PINE::Shared::Version
 */
EXPORT_LIB char *pine_version(PINE::Shared *v, bool batch);

/**
 * @see PINE::Shared::Status
 */
EXPORT_LIB PINE::Shared::EmuStatus pine_status(PINE::Shared *v, bool batch);

/**
 * @see PINE::Shared::GetGameTitle
 */
EXPORT_LIB char *pine_getgametitle(PINE::Shared *v, bool batch);

/**
 * @see PINE::Shared::GetGameID
 */
EXPORT_LIB char *pine_getgameid(PINE::Shared *v, bool batch);

/**
 * @see PINE::Shared::GetGameUUID
 */
EXPORT_LIB char *pine_getgameuuid(PINE::Shared *v, bool batch);

/**
 * @see PINE::Shared::GetGameVersion
 */
EXPORT_LIB char *pine_getgameversion(PINE::Shared *v, bool batch);

/**
 * @see PINE::Shared::SaveState
 */
EXPORT_LIB void pine_savestate(PINE::Shared *v, uint8_t slot, bool batch);

/**
 * @see PINE::Shared::LoadState
 */
EXPORT_LIB void pine_loadstate(PINE::Shared *v, uint8_t slot, bool batch);

/**
 * @see PINE::Shared::Write
 */
EXPORT_LIB void pine_write(PINE::Shared *v, uint32_t address, uint64_t val,
                           PINE::Shared::IPCCommand msg, bool batch);

/**
 * @see PINE::~PCSX2
 */
EXPORT_LIB void pine_pcsx2_delete(PINE::PCSX2 *v);

/**
 * @see PINE::~RPCS3
 */
EXPORT_LIB void pine_rpcs3_delete(PINE::RPCS3 *v);

/**
 * @see PINE::~DuckStation
 */
EXPORT_LIB void pine_duckstation_delete(PINE::DuckStation *v);

/**
 * Frees given PINE::Shared::BatchCommand through its int handle. @n
 * As the C bindings handle structures for you, you have to tell them when to
 * free the batch commands if you want to free memory.
 * @param cmd PINE::Shared::BatchCommand handle.
 */
EXPORT_LIB void pine_free_batch_command(int cmd);
/**
 * @see PINE::Shared::GetError
 */
EXPORT_LIB PINE::Shared::IPCStatus pine_get_error(PINE::Shared *v);

#ifdef __cplusplus
}
#endif
