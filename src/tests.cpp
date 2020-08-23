#include "pcsx2_ipc.h"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define u128 __int128

// TODO: port the testcase for windows


/* Test case suite for the PCSX2 IPC API
 * This is known to work on MacOS and Linux.
 * You will probably need to set PCSX2_TEST to be able
 * to boot some ISO for all of this to run. Refer to utils/default.nix for an
 * example on how to do that.
 */



// a portable sleep function
auto msleep(int sleepMs) -> void {
#ifdef _WIN32
    Sleep(sleepMs);
#else
    usleep(sleepMs * 1000);
#endif
}



auto open_pcsx2() -> void {
    if (const char *env_p = std::getenv("PCSX2_TEST")) {
        char pcsx2_path[4096];
        strcpy(pcsx2_path, env_p);
        strcat(pcsx2_path, " &");
        system(pcsx2_path);
    }
}

auto kill_pcsx2() -> void { system("pkill PCSX2"); }

TEST_CASE("Errors", "[errors]") {
    PCSX2Ipc *ipc = new PCSX2Ipc();

    try {
        ipc->Write<u64>(0x00347D34, 5);
        REQUIRE(0 == 1);
    } catch (...) {}

    try {
        ipc->Write<u128>(0x00347D34, 5);
        REQUIRE(0 == 1);
    } catch (...) {}

    try {
        ipc->Read<u128>(0x00347D34);
        REQUIRE(0 == 1);
    } catch (...) {}
}

TEST_CASE("Read and Write operations", "[mem_rw]") {
    PCSX2Ipc *ipc = new PCSX2Ipc();

    open_pcsx2();
    msleep(5000);

    try {
        ipc->Write<u64>(0x00347D34, 5);
        ipc->Write<u32>(0x00347D44, 6);
        ipc->Write<u16>(0x00347D54, 7);
        ipc->Write<u8>(0x00347D64, 8);
        REQUIRE(ipc->Read<u64>(0x00347D34) == 5);
        REQUIRE(ipc->Read<u32>(0x00347D44) == 6);
        REQUIRE(ipc->Read<u16>(0x00347D54) == 7);
        REQUIRE(ipc->Read<u8>(0x00347D64) == 8);
    } catch (...) {
        // we shouldn't throw an exception, ever
        REQUIRE(0 == 1);
    }

    kill_pcsx2();
}

TEST_CASE("Batch operations", "[batch_op]") {
    PCSX2Ipc *ipc = new PCSX2Ipc();

    open_pcsx2();
    msleep(5000);

    try {
        ipc->InitializeBatch();
        ipc->Write<u64>(0x00347E34, 5);
        ipc->Write<u32>(0x00347E44, 6);
        ipc->Write<u16>(0x00347E54, 7);
        ipc->Write<u8>(0x00347E64, 8);
        ipc->SendCommand(ipc->FinalizeBatch());

        msleep(1);
        ipc->InitializeBatch();
        ipc->Read<u64, true>(0x00347E34);
        ipc->Read<u32, true>(0x00347E44);
        ipc->Read<u16, true>(0x00347E54);
        ipc->Read<u8, true>(0x00347E64);
        auto resr = ipc->FinalizeBatch();
        ipc->SendCommand(resr);

        REQUIRE(ipc->GetReply<PCSX2Ipc::MsgRead8>(resr, 3) == 8);
        REQUIRE(ipc->GetReply<PCSX2Ipc::MsgRead16>(resr, 2) == 7);
        REQUIRE(ipc->GetReply<PCSX2Ipc::MsgRead32>(resr, 1) == 6);
        REQUIRE(ipc->GetReply<PCSX2Ipc::MsgRead64>(resr, 0) == 5);
    } catch (...) {
        // we shouldn't throw an exception, ever
        REQUIRE(0 == 1);
    }

    kill_pcsx2();
}
