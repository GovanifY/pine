#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include <tuple>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <thread>

#define to64b(arr) (((uint64_t)(((uint8_t *)(arr))[7]) <<  0)+\
        ((uint64_t)(((uint8_t *)(arr))[6]) <<  8)+\
        ((uint64_t)(((uint8_t *)(arr))[5]) << 16)+\
        ((uint64_t)(((uint8_t *)(arr))[4]) << 24)+\
        ((uint64_t)(((uint8_t *)(arr))[3]) << 32)+\
        ((uint64_t)(((uint8_t *)(arr))[2]) << 40)+\
        ((uint64_t)(((uint8_t *)(arr))[1]) << 48)+\
        ((uint64_t)(((uint8_t *)(arr))[0]) << 56))

#define to32b(arr) (((uint32_t)(((uint8_t *)(arr))[3]) <<  0)+\
        ((uint32_t)(((uint8_t *)(arr))[2]) <<  8)+\
        ((uint32_t)(((uint8_t *)(arr))[1]) << 16)+\
        ((uint32_t)(((uint8_t *)(arr))[0]) << 24))

#define to16b(arr) (((uint16_t)(((uint8_t *)(arr))[1]) <<  0)+\
        ((uint16_t)(((uint8_t *)(arr))[0]) <<  8))

#define to8b(arr) (((uint8_t)(((uint8_t *)(arr))[0]) <<  0))

#ifdef __linux__
#include <unistd.h>
#endif
#ifdef _WIN32 
#include <windows.h>
#endif

void msleep(int sleepMs)
{
#ifdef __linux__
    usleep(sleepMs * 1000);   // usleep takes sleep time in us (1 millionth of a second)
#endif
#ifdef _WIN32
    Sleep(sleepMs);
#endif
}


class PCSX2Ipc {
    public:
        const char* SOCKET_NAME = "/tmp/pcsx2";

        enum IPCCommand {
            MsgRead8 = 0,
            MsgRead16 = 1,
            MsgRead32 = 2,
            MsgRead64 = 3,
            MsgWrite8 = 4,
            MsgWrite16 = 5,
            MsgWrite32 = 6,
            MsgWrite64 = 7
        };

        enum Status {
            Fail,
            Success,
            Unknown
        };
        Status result = Unknown;

        int SendCommand(std::tuple<int, char*> command, std::tuple<int, char*> ret) {
            int sock;
            struct sockaddr_un server;


            sock = socket(AF_UNIX, SOCK_STREAM, 0);
            if (sock < 0) {
                return -1;
            }
            server.sun_family = AF_UNIX;
            strcpy(server.sun_path, SOCKET_NAME);


            if (connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
                close(sock);
                return -1;
            }
            if (write(sock, std::get<1>(command), std::get<0>(command)) < 0) {
                return -1;
            }

            if (read(sock, std::get<1>(ret), std::get<0>(ret)) < 0) {
                return -1;
            }
            close(sock);

            if(std::get<1>(ret)[0] == (char)0xFF) {
                return -1;
            }
            return 0;
        }
        char* FormatBeginning(char* cmd, uint32_t address, IPCCommand command) {
            cmd[0] = (unsigned char) command;
            cmd[1] = (unsigned char) (address >> 24) & 0xff;
            cmd[2] = (unsigned char) (address >> 16) & 0xff;
            cmd[3] = (unsigned char) (address >> 8) & 0xff;
            cmd[4] = (unsigned char) address;
            return cmd;
        }

        uint8_t Read8(uint32_t address) {
            char* cmd = (char*)malloc(5*sizeof(char));
            FormatBeginning(cmd, address, MsgRead8);
            char* res_array = (char*)malloc(2*sizeof(char));
            if (SendCommand(std::make_tuple(5,cmd), std::make_tuple(2,res_array)) < 0) {
                result = Fail;
            } else { result = Success; } 
            return to8b(&res_array[1]);
        }

        uint16_t Read16(uint32_t address) {
            char* cmd = (char*)malloc(5*sizeof(char));
            FormatBeginning(cmd, address, MsgRead16);
            char* res_array = (char*)malloc(3*sizeof(char));
            if (SendCommand(std::make_tuple(5,cmd), std::make_tuple(3,res_array)) < 0) {
                result = Fail;
            } else { result = Success; } 
            return to16b(&res_array[1]);
        }

        uint32_t Read32(uint32_t address) {
            char* cmd = (char*)malloc(5*sizeof(char));
            FormatBeginning(cmd, address, MsgRead32);
            char* res_array = (char*)malloc(5*sizeof(char));
            if (SendCommand(std::make_tuple(5,cmd), std::make_tuple(5,res_array)) < 0) {
                result = Fail;
            } else { result = Success; }
            return to32b(&res_array[1]);
        }

        uint64_t Read64(uint32_t address) {
            char* cmd = (char*)malloc(5*sizeof(char));
            FormatBeginning(cmd, address, MsgRead64);
            char* res_array = (char*)malloc(9*sizeof(char));
            if (SendCommand(std::make_tuple(5,cmd), std::make_tuple(9,res_array)) < 0) {
                result = Fail;
            } else { result = Success; }
            return to64b(&res_array[1]);
        }

        void Write8(uint32_t address, uint8_t value) {
            char* cmd = (char*)malloc(6*sizeof(char));
            FormatBeginning(cmd, address, MsgWrite8);
            char* res_array = (char*)malloc(1*sizeof(char));
            cmd[5] = (char)value;
            if (SendCommand(std::make_tuple(6,cmd), std::make_tuple(1,res_array)) < 0) {
                result = Fail;
            } else { result = Success; } 
        }

        void Write16(uint32_t address, uint16_t value) {
            char* cmd = (char*)malloc(7*sizeof(char));
            FormatBeginning(cmd, address, MsgWrite16);
            char* res_array = (char*)malloc(1*sizeof(char));
            cmd[5] = (unsigned char) (value >> 8) & 0xff;
            cmd[6] = (unsigned char) value;
            if (SendCommand(std::make_tuple(7,cmd), std::make_tuple(1,res_array)) < 0) {
                result = Fail;
            } else { result = Success; } 
        }

        void Write32(uint32_t address, uint32_t value) {
            char* cmd = (char*)malloc(9*sizeof(char));
            FormatBeginning(cmd, address, MsgWrite32);
            char* res_array = (char*)malloc(1*sizeof(char));
            cmd[5] = (unsigned char) (value >> 24) & 0xff;
            cmd[6] = (unsigned char) (value >> 16) & 0xff;
            cmd[7] = (unsigned char) (value >> 8) & 0xff;
            cmd[8] = (unsigned char) value;
            if (SendCommand(std::make_tuple(9,cmd), std::make_tuple(1,res_array)) < 0) {
                result = Fail;
            } else { result = Success; } 
        }

        void Write64(uint32_t address, uint64_t value) {
            char* cmd = (char*)malloc(13*sizeof(char));
            FormatBeginning(cmd, address, MsgWrite64);
            char* res_array = (char*)malloc(1*sizeof(char));
            cmd[5] = (unsigned char) (value >> 56) & 0xff;
            cmd[6] = (unsigned char) (value >> 48) & 0xff;
            cmd[7] = (unsigned char) (value >> 40) & 0xff;
            cmd[8] = (unsigned char) (value >> 32) & 0xff;
            cmd[9] = (unsigned char) (value >> 24) & 0xff;
            cmd[10] = (unsigned char) (value >> 16) & 0xff;
            cmd[11] = (unsigned char) (value >> 8) & 0xff;
            cmd[12] = (unsigned char) value;
            if (SendCommand(std::make_tuple(13,cmd), std::make_tuple(1,res_array)) < 0) {
                result = Fail;
            } else { result = Success; } 
        }

};

// this function is an infinite loop reading a value in memory, this shows you
// how timer can work
void read_background() {
    while(true) {
        // you can go slower but go higher at your own risk 
        msleep(100);

        // we instantiate a new PCSX2Ipc object, it is a good idea to create a new
        // one per thread, so you have a result code for each operation to verify
        // on.
        // You are NOT obligated to verify error codes but it is good practice to do
        // so.
        PCSX2Ipc* test = new PCSX2Ipc();

        // we read a 32 bit value from memory address 0x00347D34
        uint32_t value = test->Read32(0x00347D34);

        // if the operation failed
        if (test->result != test->Success) {
            printf("ERROR!!!!!\n");
        } 
        else {
            // otherwise we print the result
            printf("PCSX2Ipc::Read32(0x00347D34) :  %u\n", value);
        }
    }

}

int main(int argc, char *argv[]) {
    // we create a new thread
    std::thread first (read_background);


    // in this case we wait 5 seconds before writing to our address
    msleep(5000);
    PCSX2Ipc* test = new PCSX2Ipc();
    test->Write8(0x00347D34, 0xFF);
    printf("PCSX2Ipc::Write8(0x00347D34, 255)\n");

    // we wait for the thread to finish. in our case it is an infinite loop
    // (while true) so it will never do so.
    first.join();
    return 0;
}
