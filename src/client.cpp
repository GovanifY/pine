#include "pine.h"
#include <iostream>
#include <ostream>
#include <stdio.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

using enum PINE::PCSX2::IPCCommand;

// a portable sleep function
auto msleep(int sleepMs) -> void {
#ifdef _WIN32
    Sleep(sleepMs);
#else
    usleep(sleepMs * 1000);
#endif
}

// this function is an infinite loop reading the game title, this shows you
// how timers can work
auto read_background(PINE::PCSX2 *ipc) -> void {
    while (true) {
        // you can go slower but go higher at your own risk
        msleep(100);

        try {
            // WARNING: all datastreams that are returned by the library changes
            // ownership, it is your duty to free them after use.
            char *title = ipc->GetGameTitle();
            printf("%s\n", title);
            delete[] title;
        } catch (...) {
            // if the operation failed
            printf("ERROR!!!!!\n");
        }
    }
}

// the main function that is executed at the start of our program
auto main(int argc, char *argv[]) -> int {

    // we instantiate a new PINE::PCSX2 object. It should be shared across all
    // your threads.
    PINE::PCSX2 *ipc = new PINE::PCSX2();

    // we create a new thread
    std::thread first(read_background, ipc);

    // in this case we wait 5 seconds before writing to our address
    msleep(5000);
    try {
        // a normal write can be done this way
        ipc->Write<u8>(0x00347D34, 0x5);

        // if you need to make a lot of IPC requests at once(eg >50/frame @
        // 60fps) it is recommended to build a batch message: you should build
        // this message at the start of your thread once and keep the
        // BatchCommand to avoid wasting time recreating this
        // IPC packet.
        //
        // to create a batch IPC packet you need to initialize it, be sure to
        // enable the batch command in templates(read the documentation, for
        // Read it is Read<T, true>) and finalize it.
        ipc->InitializeBatch();
        ipc->Write<u8, true>(0x00347D34, 0xFF);
        ipc->Write<u8, true>(0x00347D33, 0xEF);
        ipc->Write<u8, true>(0x00347D32, 0xDF);
        auto res = ipc->FinalizeBatch();
        // our batch ipc packet is now saved and ready to be used whenever! When
        // we need it we just fire up a SendCommand:
        ipc->SendCommand(res);

        // let's do it another time, but this time with Read, which returns
        // arguments!
        ipc->InitializeBatch();
        ipc->Read<u8, true>(0x00347D34);
        ipc->Read<u8, true>(0x00347D33);
        ipc->Version<true>();
        ipc->Read<u8, true>(0x00347D32);
        auto resr = ipc->FinalizeBatch();
        // same as before
        ipc->SendCommand(resr);

        // now reading the return value is a little bit more tricky, you'll have
        // to know the type of your function and the number it was executed in.
        // For example, Read(0x00347D32) was our third function, and is a
        // function of type MsgRead8, so we will do:
        //   GetReply<MsgRead8>(resr, 2);
        // 2 since arrays start at 0 in C++, so 3-1 = 2 and resr being our
        // BatchCommand that we saved above!
        // Refer to the documentation of IPCCommand to know all the possible
        // function types
        printf("PINE::PCSX2::Version() :  %s\n",
               ipc->GetReply<MsgVersion>(resr, 2));
        printf("PINE::PCSX2::Read<uint8_t>(0x00347D32) :  %u\n",
               ipc->GetReply<MsgRead8>(resr, 3));
    } catch (...) {
        // if the operation failed
        printf("ERROR!!!!!\n");
    }

    // we wait for the thread to finish. in our case it is an infinite loop
    // (while true) so it will never do so.
    first.join();

    // we do not forget to free our IPC object to avoid any memory leak,
    // although they will technically get automatically freed by the OS at
    // process shutdown
    delete ipc;

    return 0;
}
