#include "pcsx2_ipc.h"

// a portable sleep function
void msleep(int sleepMs) {
#ifdef _WIN32
    Sleep(sleepMs);
#else
    usleep(sleepMs *
           1000); // usleep takes sleep time in us (1 millionth of a second)
#endif
}

// this function is an infinite loop reading a value in memory, this shows you
// how timer can work
void read_background(PCSX2Ipc *ipc) {
    while (true) {
        // you can go slower but go higher at your own risk
        msleep(100);

        // we read a 32 bit value from memory address 0x00347D34
        try {
            uint32_t value = ipc->Read32(0x00347D34);
            printf("PCSX2Ipc::Read32(0x00347D34) :  %u\n", value);
        } catch (...) {
            // if the operation failed
            printf("ERROR!!!!!\n");
        }
    }
}

// the main function that is executed at the start of our program
int main(int argc, char *argv[]) {

    // we instantiate a new PCSX2Ipc object. It should be shared across all your
    // threads.
    PCSX2Ipc *ipc = new PCSX2Ipc();

    // we create a new thread
    std::thread first(read_background, ipc);

    // in this case we wait 5 seconds before writing to our address
    msleep(5000);
    try {
        ipc->Write8(0x00347D34, 0xFF);
        printf("PCSX2Ipc::Write8(0x00347D34, 255)\n");
    } catch (...) {
        // if the operation failed
        printf("ERROR!!!!!\n");
    }

    // we wait for the thread to finish. in our case it is an infinite loop
    // (while true) so it will never do so.
    first.join();

    return 0;
}
