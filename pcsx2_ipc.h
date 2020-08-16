#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
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

/**
 * The PCSX2Ipc API.  
 * This is the client side implementation of PCSX2 IPC.  
 * It allows for a three
 * way communication between the emulated game, the emulator and an external
 * tool, using the external tool as a relay for all communication.   
 * It is a socket based IPC that is _very_ fast.  
 *
 * If you want to draw comparisons you can think of this as an equivalent of the
 * BizHawk LUA API, although with the logic out of the core and in an external
 * tool.  
 * While BizHawk would run a lua script at each frame in the core of the
 * emulator we opt instead to keep the entire logic out of the emulator to make
 * it more easily extensible, more portable, require less code and be more
 * performant.   
 *
 * Event based commands such as ExecuteOnFrameEnd (not yet implemented) can thus
 * be a blocking socket event, which is then noticed by our API, executes out
 * IPC commands and then tells the game to resume. Thanks to the speed of the
 * IPC even complex events can be outsourced from the emulator, thus keeping
 * the main codebase lean and minimal.
 *
 * Have fun!  
 * -Gauvain "GovanifY" Roussel-Tarbouriech, 2020
 */
class PCSX2Ipc {
  protected:
#if  defined(_WIN32) || defined(DOXYGEN)
    /**
     * TCP socket port.  
     * Used by the IPC on platforms with TCP sockets.  
     * Currently Windows only.
     */
    const uint16_t PORT = 28011;
#endif
#if !defined(_WIN32) || defined(DOXYGEN)
    /**
     * Unix socket name.  
     * The name of the unix socket used on platforms with unix socket support.  
     * Currently everything except Windows.  
     */
    const char *SOCKET_NAME = "/tmp/pcsx2.sock";
#endif

    /**
     * IPC Command messages opcodes.  
     * A list of possible operations possible by the IPC.  
     * Each one of them is what we call an "opcode" and is the first
     * byte sent by the IPC to differentiate between commands.  
     */
    enum IPCCommand {
        MsgRead8 = 0,   /**< Read 8 bit value to memory. */
        MsgRead16 = 1,  /**< Read 16 bit value to memory. */
        MsgRead32 = 2,  /**< Read 32 bit value to memory. */
        MsgRead64 = 3,  /**< Read 64 bit value to memory. */
        MsgWrite8 = 4,  /**< Write 8 bit value to memory. */
        MsgWrite16 = 5, /**< Write 16 bit value to memory. */
        MsgWrite32 = 6, /**< Write 32 bit value to memory. */
        MsgWrite64 = 7  /**< Write 64 bit value to memory. */
    };

    /**
     * IPC result codes.  
     * A list of possible result codes the IPC can send back.  
     * Each one of them is what we call an "opcode" or "tag" and is the
     * first byte sent by the IPC to differentiate between results.  
     */
    enum IPCResult {
        IPC_OK = 0,     /**< IPC command successfully completed. */
        IPC_FAIL = 0xFF /**< IPC command failed to complete. */
    };

    /**
     * Converts an uint to an char* in little endian.  
     * @param res_array The array to modify.
     * @param res The value to convert.
     * @param i When to insert it into the array
     * @return res_array
     */
    template <typename T> static char *ToArray(char *res_array, T res, int i) {
		memcpy((res_array + i), (char*)&res, sizeof(T));
		return res_array;
    }

    /**
     * Converts a char* to an uint in little endian.
     * @param arr The array to convert.
     * @param i When to load it from the array.
     * @return The converted value.
     */
    template <typename T> static T FromArray(char *arr, int i) {
		return *(T*)(arr + i);
    }

    /**
     * Sends an IPC command to PCSX2.  
     * Fails if the IPC cannot be sent or if PCSX2 returns IPC_FAIL.
     * @param command A pair containing the IPC command size and buffer.
     * @param ret A pair containing the IPC return size and buffer.
     * @see IPCResult
     * @return 0 on success, -1 on failure.
     */
    int SendCommand(std::pair<int, char *> command,
                    std::pair<int, char *> ret) {
#ifdef _WIN32
        SOCKET sock;
        struct sockaddr_in server;

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            return -1;
        }

        // Prepare the sockaddr_in structure
        server.sin_family = AF_INET;
        // localhost only
        server.sin_addr.s_addr = inet_addr("127.0.0.1");
        server.sin_port = htons(PORT);

        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
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

        if (connect(sock, (struct sockaddr *)&server,
                    sizeof(struct sockaddr_un)) < 0) {
            close(sock);
            return -1;
        }

#endif

#ifdef _WIN32
        if (send(sock, command.second, command.first, 0) < 0) {
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

        if (ret.second[0] == (char)IPC_FAIL) {
            return -1;
        }
        return 0;
    }

    /**
     * Formats an IPC buffer.  
     * Creates a new buffer with IPC opcode set and first address argument
     * currently used for memory IPC commands.
     * @param size The size of the array to allocate.
     * @param address The address to put as an argument of the IPC command.
     * @param command The IPC message tag(opcode).
     * @see IPCCommand
     * @return The IPC buffer.
     */
    char *FormatBeginning(int size, uint32_t address, IPCCommand command) {
        char *cmd = (char *)malloc(size * sizeof(char));
        cmd[0] = (unsigned char)command;
        return ToArray(cmd, address, 1);
    }

    /**
     * Allocates an array of given size.
     * @param size Size of the array to allocate.
     * @return The array.
     */
    char *ResArray(int size) {
        char *cmd = (char *)malloc(size * sizeof(char));
        return cmd;
    }

  public:
    /**
     * Result code of the IPC operation.  
     * A list of result codes that should be returned, or thrown, depending
     * on the state of the result of an IPC command.
     */
    enum IPCStatus {
        Fail,    /**< IPC command failed to execute. */
        Success, /**< IPC command successfully completed. */
        Unknown  /**< Unknown if the command completed successfully or not. */
    };

    /**
     * Reads an 8 bit value from PCSX2 game memory.  
     * On error throws an IPCStatus.  
     * Format: XX YY YY YY YY  
     * Legend: XX = IPC Tag, YY = Address.  
     * @see IPCCommand
     * @see IPCStatus
     * @param address The address to read.
     * @return The value read in memory.
     */
    uint8_t Read8(uint32_t address) {
        char *res = ResArray(2);
        if (SendCommand(
                std::make_pair(5, FormatBeginning(5, address, MsgRead8)),
                std::make_pair(2, res)) < 0)
            throw Fail;
        return FromArray<uint8_t>(res, 1);
    }

    /**
     * Reads a 16 bit value from PCSX2 game memory.  
     * On error throws an IPCStatus.  
     * Format: XX YY YY YY YY  
     * Legend: XX = IPC Tag, YY = Address.  
     * @see IPCCommand
     * @see IPCStatus
     * @param address The address to read.
     * @return The value read in memory.
     */
    uint16_t Read16(uint32_t address) {
        char *res = ResArray(3);
        if (SendCommand(
                std::make_pair(5, FormatBeginning(5, address, MsgRead16)),
                std::make_pair(3, res)) < 0)
            throw Fail;
        return FromArray<uint16_t>(res, 1);
    }

    /**
     * Reads a 32 bit value from PCSX2 game memory.  
     * On error throws an IPCStatus.  
     * Format: XX YY YY YY YY  
     * Legend: XX = IPC Tag, YY = Address. 
     * @see IPCCommand
     * @see IPCStatus
     * @param address The address to read.
     * @return The value read in memory.
     */
    uint32_t Read32(uint32_t address) {
        char *res = ResArray(5);
        if (SendCommand(
                std::make_pair(5, FormatBeginning(5, address, MsgRead32)),
                std::make_pair(5, res)) < 0)
            throw Fail;
        return FromArray<uint32_t>(res, 1);
    }

    /**
     * Reads a 64 bit value from PCSX2 game memory.  
     * On error throws an IPCStatus.  
     * Format: XX YY YY YY YY  
     * Legend: XX = IPC Tag, YY = Address.
     * @see IPCCommand
     * @see IPCStatus
     * @param address The address to read.
     * @return The value read in memory.
     */
    uint64_t Read64(uint32_t address) {
        char *res = ResArray(9);
        if (SendCommand(
                std::make_pair(5, FormatBeginning(5, address, MsgRead64)),
                std::make_pair(9, res)) < 0)
            throw Fail;
        return FromArray<uint64_t>(res, 1);
    }

    /**
     * Writes an 8 bit value from PCSX2 game memory.  
     * On error throws an IPCStatus.  
     * Format: XX YY YY YY YY ZZ  
     * Legend: XX = IPC Tag, YY = Address, ZZ = Value.  
     * @see IPCCommand
     * @see IPCStatus
     * @param address The address to write to.
     * @param value The value to write.
     */
    void Write8(uint32_t address, uint8_t value) {
        char *cmd = ToArray(FormatBeginning(6, address, MsgWrite8), value, 5);
        if (SendCommand(std::make_pair(6, cmd),
                        std::make_pair(1, ResArray(1))) < 0)
            throw Fail;
    }

    /**
     * Writes a 16 bit value from PCSX2 game memory.  
     * On error throws an IPCStatus.  
     * Format: XX YY YY YY YY ZZ ZZ  
     * Legend: XX = IPC Tag, YY = Address, ZZ = Value.  
     * @see IPCCommand
     * @see IPCStatus
     * @param address The address to write to.
     * @param value The value to write.
     */
    void Write16(uint32_t address, uint16_t value) {
        char *cmd = ToArray(FormatBeginning(7, address, MsgWrite16), value, 5);
        if (SendCommand(std::make_pair(7, cmd),
                        std::make_pair(1, ResArray(1))) < 0)
            throw Fail;
    }

    /**
     * Writes a 32 bit value from PCSX2 game memory.  
     * On error throws an IPCStatus.  
     * Format: XX YY YY YY YY ZZ ZZ ZZ ZZ  
     * Legend: XX = IPC Tag, YY = Address, ZZ = Value.  
     * @see IPCCommand
     * @see IPCStatus
     * @param address The address to write to.
     * @param value The value to write.
     */
    void Write32(uint32_t address, uint32_t value) {
        char *cmd = ToArray(FormatBeginning(9, address, MsgWrite32), value, 5);
        if (SendCommand(std::make_pair(9, cmd),
                        std::make_pair(1, ResArray(1))) < 0)
            throw Fail;
    }

    /**
     * Writes a 64 bit value from PCSX2 game memory.  
     * On error throws an IPCStatus.  
     * Format: XX YY YY YY YY ZZ ZZ ZZ ZZ ZZ ZZ ZZ ZZ  
     * Legend: XX = IPC Tag, YY = Address, ZZ = Value.
     * @see IPCCommand
     * @see IPCStatus
     * @param address The address to write to.
     * @param value The value to write.
     */
    void Write64(uint32_t address, uint64_t value) {
        char *cmd = ToArray(FormatBeginning(13, address, MsgWrite64), value, 5);
        if (SendCommand(std::make_pair(13, cmd),
                        std::make_pair(1, ResArray(1))) < 0)
            throw Fail;
    }

    /**
     * PCSX2Ipc Initializer.
     */
    PCSX2Ipc() {
        // We initialize winsock.
#ifdef _WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
    }

    /**
     * PCSX2Ipc Destructor.
     */
    ~PCSX2Ipc() {
        // We clean up winsock.
#ifdef _WIN32
        WSACleanup();
#endif
    }
};
