#include "pcsx2_ipc.h"
#include <iostream>
#include <ostream>
#include <stdio.h>

// a portable sleep function
auto msleep(int sleepMs) -> void {
#ifdef _WIN32
    Sleep(sleepMs);
#else
    usleep(sleepMs * 1000);
#endif
}

// this function is an infinite loop reading a value in memory, this shows you
// how timer can work
auto read_background(PCSX2Ipc *ipc) -> void {
    while (true) {
        // you can go slower but go higher at your own risk
        msleep(100);

        // we read a 32 bit value from memory address 0x00347D34
        try {
            // those comments calculate a rough approximation of the latency
            // time of socket IPC, in µs, if you want to have an idea.

            // auto t1 = std::chrono::high_resolution_clock::now();
            uint32_t value = ipc->Read<uint32_t>(0x00347D34);
            // auto t2 = std::chrono::high_resolution_clock::now();
            // auto duration =
            // std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1
            // ).count(); std::cout << "execution time: " << duration <<
            // std::endl;
            printf("PCSX2Ipc::Read<uint32_t>(0x00347D34) :  %u\n", value);
        } catch (...) {
            // if the operation failed
            printf("ERROR!!!!!\n");
        }
    }
}

// the main function that is executed at the start of our program
auto main(int argc, char *argv[]) -> int {

    // we instantiate a new PCSX2Ipc object. It should be shared across all your
    // threads.
    PCSX2Ipc *ipc = new PCSX2Ipc();

    // we create a new thread
    std::thread first(read_background, ipc);

    // in this case we wait 5 seconds before writing to our address
    msleep(5000);
    try {
        // a normal write can be done this way
        ipc->Write<uint8_t>(0x00347D34, 0x5);

        // if you need to make a lot of IPC requests at once(eg >50/16ms) it is
        // recommended to build a batch message: you should build this message
        // at the start of your thread once and keep the command/ret combo
        // defined below to avoid wasting time recreating this IPC packet.
        ipc->InitializeBatch();
        ipc->Write<uint8_t, true>(0x00347D34, 0xFF);
        ipc->Write<uint8_t, true>(0x00347D33, 0xEF);
        ipc->Write<uint8_t, true>(0x00347D32, 0xDF);
        auto res = ipc->FinalizeBatch();
        // our batch ipc packet is now saved and ready to be used whenever! When
        // we need it we just fire up a SendCommand:
        ipc->SendCommand(res.ipc_message, res.ipc_return);

        // let's do it another time, but this time with Read, which returns
        // arguments!
        ipc->InitializeBatch();
        ipc->Read<uint8_t, true>(0x00347D34);
        ipc->Read<uint8_t, true>(0x00347D33);
        ipc->Read<uint8_t, true>(0x00347D32);
        auto resr = ipc->FinalizeBatch();
        // same as before
        ipc->SendCommand(resr.ipc_message, resr.ipc_return);

        // now this is a little bit more tricky, return_locations returned by
        // FinalizeBatch tells us where the replies are stored in the return
        // buffer, so let's save this value in the variable result, and try to
        // get the reply of the third function, in our case Read(0x00347D32)
        unsigned int result = resr.return_locations[2];

        // and now let's use this location to get the reply from our return
        // buffer! NB: you'll have to read the doc to verify what the return
        // value of the command is and convert it accordingly, in our case
        // Read<uint8_t> returns an uint8_t, so this isn't very hard :p
        printf("PCSX2Ipc::Read<uint8_t>(0x00347D32) :  %u\n",
               ipc->FromArray<uint8_t>(resr.ipc_return.buffer, result));
    } catch (...) {
        // if the operation failed
        printf("ERROR!!!!!\n");
    }

    // we wait for the thread to finish. in our case it is an infinite loop
    // (while true) so it will never do so.
    first.join();

    return 0;
}
