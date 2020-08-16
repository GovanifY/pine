#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <mutex>

#ifdef _WIN32
#define read_portable(a, b, c) (recv(a, b, c, 0))
#define write_portable(a, b, c) (send(a, b, c, 0))
#include <windows.h>
#else
#define read_portable(a, b, c) (read(a, b, c))
#define write_portable(a, b, c) (write(a, b, c))
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
     * IPC return buffer.  
     * A preallocated buffer used to store all IPC replies. Currently allocated
     * to the size of 50.000 MsgWrite64 IPC calls.  
     * WARNING: No checks are executed client or server-side about the size of
     * this buffer, to ensure a fast implementation.  
     * It is assumed you're not an absolutely insane person and you
     * won't try to send in batch more than 50k IPC calls in a single batch.  
     * @see ipc_buffer
     */
    char* ret_buffer;

    /**
     * IPC messages buffer.  
     * A preallocated buffer used to store all IPC messages. Currently allocated
     * to the size of 50.000 MsgWrite64 IPC calls.  
     * WARNING: No checks are executed client or server-side about the size of
     * this buffer, to ensure a fast implementation.  
     * It is assumed you're not an absolutely insane person and you
     * won't try to send in batch more than 50k IPC calls in a single batch.  
     * @see ret_buffer
     */
    char* ipc_buffer;

    /**
     * Length of the batch IPC request.  
     * This is used when chaining multiple IPC commands in one go by using
     * MsgMultiCommand to store the length of the entire IPC message.  
     * @see IPCCommand 
     */
    uint16_t batch_len = 0;

    /**
     * Length of the reply of the batch IPC request.  
     * This is used when chaining multiple IPC commands in one go by using
     * MsgMultiCommand to store the length of the reply of the IPC message.  
     * @see IPCCommand 
     */
    unsigned int reply_len = 0;

    /**
     * Number of IPC messages of the batch IPC request.  
     * This is used when chaining multiple IPC commands in one go by using
     * MsgMultiCommand to store the number of IPC messages chained together.  
     * @see IPCCommand 
     */
    unsigned int arg_cnt = 0;

    /**
     * Position of the batch arguments.  
     * This is used when chaining multiple IPC commands in one go by using
     * MsgMultiCommand. Stores the location of each message reply in the buffer
     * sent by FinalizeBatch.
     * @see FinalizeBatch
     * @see IPCCommand 
     */
    unsigned int* batch_arg_place;

    /**
     * Sets the state of the batch command building.  
     * This is used when chaining multiple IPC commands in one go by using
     * MsgMultiCommand.  
     * As we cannot build multiple batch IPC commands at the same time because
     * of state keeping issues we block the initialization of another batch
     * request until the other ends.  
     */
    std::mutex batch_blocking;

    /**
     * Sets the state of the IPC message building.  
     * As we cannot build multiple batch IPC commands at the same time because
     * of state keeping issues we block the initialization of another message 
     * request until the other ends.  
     */
    std::mutex ipc_blocking;

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
        MsgWrite64 = 7,  /**< Write 64 bit value to memory. */
        MsgMultiCommand = 0xFF  /**< Treats multiple IPC commands in batch. */
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
     * Formats an IPC buffer.  
     * Creates a new buffer with IPC opcode set and first address argument
     * currently used for memory IPC commands.
     * @param size The size of the array to allocate.
     * @param address The address to put as an argument of the IPC command.
     * @param command The IPC message tag(opcode).
     * @see IPCCommand
     * @return The IPC buffer.
     */
    char *FormatBeginning(char* cmd, uint32_t address, IPCCommand command) {
        cmd[0] = (unsigned char)command;
        return ToArray(cmd, address, 1);
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

        if (write_portable(sock, command.second, command.first) < 0) {
            return -1;
        }

        if (read_portable(sock, ret.second, ret.first) < 0) {
            return -1;
        }
        close(sock);

        if (ret.second[0] == (char)IPC_FAIL) {
            return -1;
        }
        return 0;
    }

    /**
     * Initializes a MsgMultiCommand IPC message.  
     * @see batch_blocking
     * @see batch_len
     * @see reply_len
     * @see arg_cnt
     */
    void InitializeBatch() {
        batch_blocking.lock();
        ipc_blocking.lock();
        ipc_buffer[0] = (unsigned char)MsgMultiCommand;
        batch_len = 3;
        reply_len = 1;
        arg_cnt = 0;
    }

    /**
     * Finalizes a MsgMultiCommand IPC message.  
     * @return A tuple with, in order:  
     *         * The IPC command length.  
     *         * The IPC message.  
     *         * The IPC reply length.  
     *         * A buffer to store the IPC reply.  
     *         * A buffer storing the offset of the argument for each function in
     *         the IPC reply buffer.  
     *         NB: ownership of the buffers is delegated to the calling thread.
     *         It is your duty to free them when done! (or not, I'm a
     *         documentation, not a cop).  
     * @see batch_blocking
     * @see batch_len
     * @see reply_len
     * @see arg_cnt
     */
    std::tuple<uint16_t, char*, unsigned int, char*, unsigned int*> FinalizeBatch() {
        // save size in IPC message header.
        ToArray(ipc_buffer, batch_len, 1);

        // we copy our arrays to unblock the IPC class.
        uint16_t bl = batch_len;
        int rl = reply_len;
        char *c_cmd = (char*)malloc(batch_len*sizeof(char));
        memcpy(c_cmd, ipc_buffer, batch_len*sizeof(char));
        char *c_ret = (char*)malloc(reply_len*sizeof(char));
        memcpy(c_ret, ret_buffer, reply_len*sizeof(char));
        unsigned int *arg_place = (unsigned int*)malloc(arg_cnt*sizeof(unsigned int));
        memcpy(arg_place, batch_arg_place, arg_cnt*sizeof(unsigned int));

        // we unblock the mutex
        batch_blocking.unlock();
        ipc_blocking.unlock();

        // MultiCommand is done!
        return std::make_tuple(bl, c_cmd, rl, c_ret, arg_place);
    }

    /**
     * Reads a value from PCSX2 game memory.  
     * On error throws an IPCStatus.  
     * Format: XX YY YY YY YY  
     * Legend: XX = IPC Tag, YY = Address.  
     * @see IPCCommand
     * @see IPCStatus
     * @param address The address to read.
     * @param T Flag to enable batch processing or not.
     * @return The value read in memory. If in batch mode the IPC message.
     */
    template <typename Y, bool T = false> auto Read(uint32_t address) {
        // deduce ipc tag
        IPCCommand tag;
        switch(sizeof(Y)) {
            case 1:
                tag = MsgRead8;
                break;
            case 2:
                tag = MsgRead16;
                break;
            case 4:
                tag = MsgRead32;
                break;
            case 8:
                tag = MsgRead64;
                break;
            default:
                throw Fail;
        }

        // batch mode
        if constexpr (T) {
            char *cmd = FormatBeginning(&ipc_buffer[batch_len], address, tag);
            batch_len += 5;
            reply_len += sizeof(Y);
            arg_cnt += 1;
            return cmd;
        } else {
            // we are already locked in batch mode
            std::lock_guard<std::mutex> lock(ipc_blocking);
            if (SendCommand(
                        std::make_pair(5, FormatBeginning(ipc_buffer, address, tag)),
                        std::make_pair(1 + sizeof(Y), ret_buffer)) < 0)
                throw Fail;
            return FromArray<Y>(ret_buffer, 1);
        }
    }

    /**
     * Writes a value to PCSX2 game memory.  
     * On error throws an IPCStatus.  
     * Format: XX YY YY YY YY (ZZ*??)   
     * Legend: XX = IPC Tag, YY = Address, ZZ = Value.  
     * @see IPCCommand
     * @see IPCStatus
     * @param address The address to write to.
     * @param value The value to write.
     * @return If in batch mode the IPC message otherwise void.
     */
    template <typename Y, bool T = false> auto Write(uint32_t address, Y value) {
        // deduce ipc tag
        IPCCommand tag;
        switch(sizeof(Y)) {
            case 1:
                tag = MsgWrite8;
                break;
            case 2:
                tag = MsgWrite16;
                break;
            case 4:
                tag = MsgWrite32;
                break;
            case 8:
                tag = MsgWrite64;
                break;
            default:
                throw Fail;
        }

        // batch mode
        if constexpr (T) {
            char *cmd = ToArray<Y>(FormatBeginning(&ipc_buffer[batch_len], address, tag), value, 5);
            batch_len += 5 + sizeof(Y);
            arg_cnt += 1;
            return cmd;
        }
        else {
            // we are already locked in batch mode
            std::lock_guard<std::mutex> lock(ipc_blocking);
            char *cmd = ToArray(FormatBeginning(ipc_buffer, address, tag), value, 5);
            if (SendCommand(std::make_pair(5 + sizeof(Y), cmd),
                            std::make_pair(1, ret_buffer)) < 0)
                throw Fail;
            return;
        }
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
        // for the sake of speed we malloc once a return buffer and reuse it by just
        // cropping its size when needed, it is 450k long which is the size of 50k
        // MsgWrite64 replies, should be good enough even if we implement batch IPC
        // processing. Coincidentally 650k is the size of 50k MsgWrite64 REQUESTS so
        // we just allocate a 1mb buffer in the end, lul
        ret_buffer = (char*)malloc(450000 * sizeof(char));
        ipc_buffer = (char*)malloc(650000 * sizeof(char));
        batch_arg_place = (unsigned int*)malloc(50000 * sizeof(unsigned int));
    }

    /**
     * PCSX2Ipc Destructor.
     */
    ~PCSX2Ipc() {
        // We clean up winsock.
#ifdef _WIN32
        WSACleanup();
#endif
        free(ret_buffer);
        free(ipc_buffer);
        free(batch_arg_place);
    }
};
