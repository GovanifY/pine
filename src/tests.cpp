#include "pcsx2_ipc.h"
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define u128 __int128

/* Test case suite for the PCSX2 IPC API
 * You will probably need to set environment variables to be able
 * to boot emulator(s) with some ISO for all of this to run. Refer to
 * utils/default.nix for an example on how to do that.
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
        (void)system(pcsx2_path);
    }
}

#ifdef _WIN32
[[maybe_unused]] auto kill_pcsx2() -> void { (void)system("tskill PCSX2"); }
#else
[[maybe_unused]] auto kill_pcsx2() -> void { (void)system("pkill PCSX2"); }
#endif

SCENARIO("PCSX2 can be interacted with remotely through IPC", "[pcsx2_ipc]") {

    WHEN("PCSX2 is not started") {
        PCSX2Ipc *ipc = new PCSX2Ipc();
        THEN("Errors should happen") {
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
    }

    GIVEN("PCSX2 with IPC started") {

        open_pcsx2();
        msleep(5000);

        WHEN("We want to read/write to the memory") {
            THEN("The read/writes are consistent") {
                PCSX2Ipc *ipc = new PCSX2Ipc();

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
        }

        WHEN("We want to execute multiple operations in a row") {
            THEN("The operations get executed successfully and consistently") {
                PCSX2Ipc *ipc = new PCSX2Ipc();

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
        }
    }
}
