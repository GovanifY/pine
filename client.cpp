#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tuple>
#include <sys/types.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#endif

/* Quick HOWTO:
 * The Entire PCSX2IPC API is below, you can find it clearly defined between the
 * comment just below me with a bunch of equals and a relatively similar one at
 * the end of the class, just search for equals.
 *
 * This API exposes a result value which is assumed to be checked after
 * executing a command. If you want to see a list of currently implemented API
 * functions currently usable by the end user of this API look up the "public:"
 * keyword of this class and read the comments of the functions.
 *
 * A small usage example is available at the end of this file.
 *
 * /!\ ON WINDOWS YOU HAVE TO INITIALIZE SOCKETS ONCE OUT OF THIS API AND CLEAN
 * THEM UP AFTERWARDS, THIS CANNOT BE DONE FROM THE API, SEE THE USAGE EXAMPLE /!\
 *
 * Have fun!
 * -GovanifY, 2020 */

/* ========================= PCSX2IPC API =====================*/



class PCSX2Ipc {
    protected:
#ifdef _WIN32
        #define PORT 28011
#else
        const char* SOCKET_NAME = "/tmp/pcsx2.sock";
#endif

        // possible command messages
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

        
        // possible result codes
        enum IPCResult {
            IPC_OK = 0,
            IPC_FAIL = 0xFF
        };

        /* Converts an uint to an char* in little endian 
         * res_array: the array to modify 
         * res: the value to convert
         * i: when to insert it into the array 
         * return value: res_array */
        template <typename T>
        static char* ToArray(char* res_array, T res, int i) {
           for(int y=sizeof(T); y > 0; y--) {
               res_array[i-(y-sizeof(T))] = (unsigned char)(res >> ((y-1)*8)) & 0xff;
           }
           return res_array;
        }

        /* Converts a char* to an uint in little endian 
         * arr: the array to convert
         * i: when to load it from the array 
         * return value: the converted value */
        template <typename T>
        static T FromArray(char* arr, int i) {
            T res = 0;
            for(int y=sizeof(T); y > 0; y--) {
                res += (((T)(((uint8_t*)(arr))[i-(y-sizeof(T))]) << ((y-1)*8)));
            }
            return res;
        }


        /* Internal function, sends an IPC command.
         * command: pair containing the IPC command size and buffer.
         * ret: pair containing the IPC return size and buffer.
         * return value: 0 on success, -1 on failure */
        int SendCommand(std::pair<int, char*> command, std::pair<int, char*> ret) {
#ifdef _WIN32
            SOCKET sock;
            struct sockaddr_in server;

            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                return -1;
            }

            //Prepare the sockaddr_in structure
            server.sin_family = AF_INET;
            // localhost only
            server.sin_addr.s_addr = inet_addr("127.0.0.1");
            server.sin_port = htons(PORT);

            if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
                close(sock);
                return -1;
            }

#else
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

#endif

#ifdef _WIN32
            if (send(sock, command.second, command.first,0) < 0) {
#else
            if (write(sock, command.second, command.first) < 0) {
#endif
                return -1;
            }

#ifdef _WIN32
            if (recv(sock, ret.second, ret.first, 0) < 0) {
#else
            if (read(sock, ret.second, ret.first) < 0) {
#endif
                return -1;
            }
            close(sock);

            if(ret.second[0] == (char)IPC_FAIL) {
                return -1;
            }
            return 0;
        }



        /* Formats an IPC buffer
         * size: size of the array to allocate
         * address: address to put as arg0 of the IPC command
         * command: IPC message tag
         * return value: buffer containing the status code and arg0 allocated of size
         *               size */
        char* FormatBeginning(int size, uint32_t address, IPCCommand command) {
            char* cmd = (char*)malloc(size*sizeof(char));
            cmd[0] = (unsigned char) command;
            return ToArray(cmd, address, 1);
        }

        /* Allocated an array
         * size: size of the array to allocate
         * return value: array */
        char* ResArray(int size) {
            char* cmd = (char*)malloc(size*sizeof(char));
            return  cmd;
        }


    public:

        // status of the last operation of this object
        enum Status {
            Fail,
            Success,
            Unknown
        };
        Status result = Unknown;

        /* Reads an 8bit value from PCSX2 game memory 
         * address: the address to read
         * return value: the value */
        uint8_t Read8(uint32_t address) {
            char* res = ResArray(2);
            if (SendCommand(std::make_pair(5,FormatBeginning(5, address, MsgRead8)), std::make_pair(2,res)) < 0) {
                result = Fail; return 0;
            } else { result = Success; }
            return FromArray<uint8_t>(res, 1);
        }

        /* Reads a 16bit value from PCSX2 game memory 
         * address: the address to read
         * return value: the value */
        uint16_t Read16(uint32_t address) {
            char* res = ResArray(3);
            if (SendCommand(std::make_pair(5,FormatBeginning(5, address, MsgRead16)), std::make_pair(3,res)) < 0) {
                result = Fail; return 0;
            } else { result = Success; }
            return FromArray<uint16_t>(res, 1);
        }

        /* Reads a 32bit value from PCSX2 game memory 
         * address: the address to read
         * return value: the value */
        uint32_t Read32(uint32_t address) {
            char* res = ResArray(5);
            if (SendCommand(std::make_pair(5,FormatBeginning(5, address, MsgRead32)), std::make_pair(5,res)) < 0) {
                result = Fail; return 0;
            } else { result = Success; }
            return FromArray<uint32_t>(res, 1);
        }

        /* Reads a 64bit value from PCSX2 game memory 
         * address: the address to read
         * return value: the value */
        uint64_t Read64(uint32_t address) {
            char* res = ResArray(9);
            if (SendCommand(std::make_pair(5,FormatBeginning(5, address, MsgRead64)), std::make_pair(9,res)) < 0) {
                result = Fail; return 0;
            } else { result = Success; }
            return FromArray<uint64_t>(res, 1);
        }

        /* Writes an 8bit value to PCSX2 game memory 
         * address: the address to write to
         * value: the value */
        void Write8(uint32_t address, uint8_t value) {
            FormatBeginning(6, address, MsgWrite8);
            char* cmd = ToArray(FormatBeginning(6, address, MsgWrite8), value, 5);
            if (SendCommand(std::make_pair(6,cmd), std::make_pair(1,ResArray(1))) < 0) {
                result = Fail;
            } else { result = Success; }
        }

        /* Writes a 16bit value to PCSX2 game memory 
         * address: the address to write to
         * value: the value */
        void Write16(uint32_t address, uint16_t value) {
            char* cmd = ToArray(FormatBeginning(7, address, MsgWrite16), value, 5);
            if (SendCommand(std::make_pair(7,cmd), std::make_pair(1,ResArray(1))) < 0) {
                result = Fail;
            } else { result = Success; }
        }

        /* Writes a 32bit value to PCSX2 game memory 
         * address: the address to write to
         * value: the value */
        void Write32(uint32_t address, uint32_t value) {
            char* cmd = ToArray(FormatBeginning(9, address, MsgWrite32), value, 5);
            if (SendCommand(std::make_pair(9,cmd), std::make_pair(1,ResArray(1))) < 0) {
                result = Fail;
            } else { result = Success; }
        }

        /* Writes a 64bit value to PCSX2 game memory 
         * address: the address to write to
         * value: the value */
        void Write64(uint32_t address, uint64_t value) {
            char* cmd = ToArray(FormatBeginning(13, address, MsgWrite64), value, 5);
            if (SendCommand(std::make_pair(13,cmd), std::make_pair(1,ResArray(1))) < 0) {
                result = Fail;
            } else { result = Success; }
        }

};

/* ========================= PCSX2IPC API END =====================*/



// a portable sleep function
void msleep(int sleepMs) {
#ifdef _WIN32
    Sleep(sleepMs);
#else
    usleep(sleepMs * 1000);   // usleep takes sleep time in us (1 millionth of a second)
#endif
}


// this function is an infinite loop reading a value in memory, this shows you
// how timer can work
void read_background() {

   // we instantiate a new PCSX2Ipc object, it is a good idea to create a new
   // one per thread, so you have a result code for each operation to verify
   // on.
   // You are NOT obligated to verify error codes but it is good practice to do
   // so.
   PCSX2Ipc* test = new PCSX2Ipc();

    while(true) {
        // you can go slower but go higher at your own risk
        msleep(100);


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

// the main function that is executed at the start of our program
int main(int argc, char *argv[]) {

    // we initialize winsock
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    // we create a new thread
    std::thread first (read_background);


    // in this case we wait 5 seconds before writing to our address
    msleep(5000);
    PCSX2Ipc* test = new PCSX2Ipc();
    test->Write8(0x00347D34, 0xFF);
    // if the operation failed
    if (test->result != test->Success) {
        printf("ERROR!!!!!\n");
    }
    else {
        // otherwise we print the result
        printf("PCSX2Ipc::Write8(0x00347D34, 255)\n");
    }


    // we wait for the thread to finish. in our case it is an infinite loop
    // (while true) so it will never do so.
    first.join();


    // we clean up winsock
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
