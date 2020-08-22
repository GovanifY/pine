#pragma once

#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

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
 * The PCSX2Ipc API. @n
 * This is the client side implementation of PCSX2 IPC. @n
 * It allows for a three
 * way communication between the emulated game, the emulator and an external
 * tool, using the external tool as a relay for all communication. @n
 * It is a socket based IPC that is _very_ fast. @n
 *
 * If you want to draw comparisons you can think of this as an equivalent of the
 * BizHawk LUA API, although with the logic out of the core and in an external
 * tool. @n
 * While BizHawk would run a lua script at each frame in the core of the
 * emulator we opt instead to keep the entire logic out of the emulator to make
 * it more easily extensible, more portable, require less code and be more
 * performant. @n
 *
 * Event based commands such as ExecuteOnFrameEnd (not yet implemented) can thus
 * be a blocking socket event, which is then noticed by our API, executes out
 * IPC commands and then tells the game to resume. Thanks to the speed of the
 * IPC even complex events can be outsourced from the emulator, thus keeping
 * the main codebase lean and minimal. @n
 *
 * Have fun! @n
 * -Gauvain "GovanifY" Roussel-Tarbouriech, 2020
 */
class PCSX2Ipc {
  protected:
#if defined(_WIN32) || defined(DOXYGEN)
    /**
     * TCP socket port. @n
     * Used by the IPC on platforms with TCP sockets. @n
     * Currently Windows only.
     */
    const uint16_t PORT = 28011;
#endif
#if !defined(_WIN32) || defined(DOXYGEN)
    /**
     * Unix socket name. @n
     * The name of the unix socket used on platforms with unix socket support.
     * @n Currently everything except Windows.
     */
    const char *SOCKET_NAME = "/tmp/pcsx2.sock";
#endif

    /**
     * Maximum memory used by an IPC message request.
     * Equivalent to 50,000 Write64 requests.
     * @see MAX_IPC_RETURN_SIZE
     * @see MAX_BATCH_REPLY_COUNT
     */
    const int MAX_IPC_SIZE = 650000;

    /**
     * Maximum memory used by an IPC message reply.
     * Equivalent to 50,000 Read64 replies.
     * @see MAX_IPC_SIZE
     * @see MAX_BATCH_REPLY_COUNT
     */
    const unsigned int MAX_IPC_RETURN_SIZE = 450000;

    /**
     * Maximum number of commands sent in a batch message.
     * @see MAX_IPC_RETURN_SIZE
     * @see MAX_IPC_SIZE
     */
    const unsigned int MAX_BATCH_REPLY_COUNT = 50000;

    /**
     * IPC return buffer. @n
     * A preallocated buffer used to store all IPC replies.
     * @see ipc_buffer
     * @see MAX_IPC_RETURN_SIZE
     */
    char *ret_buffer;

    /**
     * IPC messages buffer. @n
     * A preallocated buffer used to store all IPC messages.
     * @see ret_buffer
     * @see MAX_IPC_SIZE
     */
    char *ipc_buffer;

    /**
     * Length of the batch IPC request. @n
     * This is used when chaining multiple IPC commands in one go by using
     * MsgMultiCommand to store the length of the entire IPC message.
     * @see IPCCommand
     * @see MAX_IPC_SIZE
     */
    uint16_t batch_len = 0;

    /**
     * Length of the reply of the batch IPC request. @n
     * This is used when chaining multiple IPC commands in one go by using
     * MsgMultiCommand to store the length of the reply of the IPC message.
     * @see IPCCommand
     * @see MAX_IPC_RETURN_SIZE
     */
    unsigned int reply_len = 0;

    /**
     * Number of IPC messages of the batch IPC request. @n
     * This is used when chaining multiple IPC commands in one go by using
     * MsgMultiCommand to store the number of IPC messages chained together.
     * @see IPCCommand
     * @see MAX_BATCH_REPLY_COUNT
     */
    unsigned int arg_cnt = 0;

    /**
     * Position of the batch arguments. @n
     * This is used when chaining multiple IPC commands in one go by using
     * MsgMultiCommand. Stores the location of each message reply in the buffer
     * sent by FinalizeBatch.
     * @see FinalizeBatch
     * @see IPCCommand
     * @see MAX_BATCH_REPLY_COUNT
     */
    unsigned int *batch_arg_place;

    /**
     * Sets the state of the batch command building. @n
     * This is used when chaining multiple IPC commands in one go by using
     * MsgMultiCommand. @n
     * As we cannot build multiple batch IPC commands at the same time because
     * of state keeping issues we block the initialization of another batch
     * request until the other ends.
     */
    std::mutex batch_blocking;

    /**
     * Sets the state of the IPC message building. @n
     * As we cannot build multiple batch IPC commands at the same time because
     * of state keeping issues we block the initialization of another message
     * request until the other ends.
     */
    std::mutex ipc_blocking;

    /**
     * IPC result codes. @n
     * A list of possible result codes the IPC can send back. @n
     * Each one of them is what we call an "opcode" or "tag" and is the
     * first byte sent by the IPC to differentiate between results.
     */
    enum IPCResult : unsigned char {
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
    template <typename T>
    static auto ToArray(char *res_array, T res, int i) -> char * {
        memcpy((res_array + i), (char *)&res, sizeof(T));
        return res_array;
    }

    /**
     * Converts a char* to an uint in little endian.
     * @param arr The array to convert.
     * @param i When to load it from the array.
     * @return The converted value.
     */
    template <typename T>
    static auto FromArray(char *arr, int i) -> T {
        return *(T *)(arr + i);
    }

    /**
     * Ensures a batch IPC message isn't too big.
     * @param command_size Additional size required for the message.
     * @param reply_size Additional size required for the reply.
     */
    auto BatchSafetyChecks(int command_size, int reply_size = 0) -> bool {
        // we do not really care about wasting cycles when building batch
        // packets, so let's just do sanity checks for the sake of it.
        // TODO: go back when clang has implemented C++20 [[unlikely]]
        return ((batch_len + command_size) >= MAX_IPC_SIZE ||
                (reply_len + reply_size) >= MAX_IPC_RETURN_SIZE ||
                arg_cnt + 1 >= MAX_BATCH_REPLY_COUNT);
    }

  public:
    /**
     * IPC Command messages opcodes. @n
     * A list of possible operations possible by the IPC. @n
     * Each one of them is what we call an "opcode" and is the first
     * byte sent by the IPC to differentiate between commands.
     */
    enum IPCCommand : unsigned char {
        MsgRead8 = 0,           /**< Read 8 bit value to memory. */
        MsgRead16 = 1,          /**< Read 16 bit value to memory. */
        MsgRead32 = 2,          /**< Read 32 bit value to memory. */
        MsgRead64 = 3,          /**< Read 64 bit value to memory. */
        MsgWrite8 = 4,          /**< Write 8 bit value to memory. */
        MsgWrite16 = 5,         /**< Write 16 bit value to memory. */
        MsgWrite32 = 6,         /**< Write 32 bit value to memory. */
        MsgWrite64 = 7,         /**< Write 64 bit value to memory. */
        MsgUnimplemented,       /**< Unimplemented IPC message. */
        MsgMultiCommand = 0xFF, /**< Treats multiple IPC commands in batch. */
    };

    /**
     * IPC message buffer. @n
     * A list of all needed fields to store an IPC message.
     */
    struct IPCBuffer {
        int size;     /**< Size of the buffer. */
        char *buffer; /**< Buffer. */
        // do NOT specify a destructor to free buffer as we reuse the same
        // buffer to avoid the cost of mallocs; We specify destructors upon need
        // on structures that makes a copy of this, eg BatchCommand.
    };

    /**
     * IPC batch message fields. @n
     * A list of all needed fields to send a batch IPC message command and
     * retrieve their result.
     */
    struct BatchCommand {
        IPCBuffer ipc_message;          /**< IPC message fields. */
        IPCBuffer ipc_return;           /**< IPC return fields. */
        unsigned int *return_locations; /**< Location of arguments in IPC return
                                           fields. */
        /**
         * BatchCommand Destructor.
         */
        ~BatchCommand() {
            delete[] ipc_message.buffer;
            delete[] ipc_return.buffer;
            delete[] return_locations;
        }
    };

    /**
     * Result code of the IPC operation. @n
     * A list of result codes that should be returned, or thrown, depending
     * on the state of the result of an IPC command.
     */
    enum IPCStatus : unsigned int {
        Success = 0,       /**< IPC command successfully completed. */
        Fail = 1,          /**< IPC command failed to execute. */
        OutOfMemory = 2,   /**< IPC command too big to send. */
        Unimplemented = 3, /**< Unimplemented IPC command. */
        Unknown = 4        /**< Unknown status. */
    };

  protected:
    /**
     * Formats an IPC buffer. @n
     * Creates a new buffer with IPC opcode set and first address argument
     * currently used for memory IPC commands.
     * @param size The size of the array to allocate.
     * @param address The address to put as an argument of the IPC command.
     * @param command The IPC message tag(opcode).
     * @see IPCCommand
     * @return The IPC buffer.
     */
    auto FormatBeginning(char *cmd, uint32_t address, IPCCommand command)
        -> char * {
        cmd[0] = (unsigned char)command;
        return ToArray(cmd, address, 1);
    }

#if defined(C_FFI) || defined(DOXYGEN)
    /**
     * Error number. @n
     * Result code of the last executed function. @n
     * Used instead of exceptions on the C library bindings.
     */
    IPCStatus ipc_errno = Success;
#endif

    /**
     * Sets the error code for the last operation. @n
     * On C++, throws an exception, on C, sets the error code. @n
     * @param err The error to set.
     */
    auto SetError(IPCStatus err) -> void {
#ifdef C_FFI
        ipc_errno = err;
#else
        throw err;
#endif
    }

  public:
#if defined(C_FFI) || defined(DOXYGEN)
    /**
     * Gets the last error code set.
     */
    auto GetError() -> IPCStatus {
        IPCStatus copy = ipc_errno;
        ipc_errno = Success;
        return copy;
    }
#endif

    /**
     * Returns the reply of an IPC command. @n
     * Throws an IPCStatus if there is no reply to read.
     * @param cmd A char array containing the IPC return buffer OR a
     * BatchCommand.
     * @param place An integer specifying where the argument is
     * in the buffer OR which function to read the reply of in
     * the case of a BatchCommand.
     * @return The reply, variable type.
     * @see IPCResult
     * @see IPCBuffer
     */
    template <IPCCommand T, typename Y>
    auto GetReply(const Y &cmd, int place) {
        char *buf;
        int loc;
        if constexpr (std::is_same<Y, BatchCommand>::value) {
            buf = cmd.ipc_return.buffer;
            loc = cmd.return_locations[place];
        } else {
            buf = cmd;
            loc = place;
        }
        if constexpr (T == MsgRead8)
            return FromArray<uint8_t>(buf, loc);
        else if constexpr (T == MsgRead16)
            return FromArray<uint16_t>(buf, loc);
        else if constexpr (T == MsgRead32)
            return FromArray<uint32_t>(buf, loc);
        else if constexpr (T == MsgRead64)
            return FromArray<uint64_t>(buf, loc);
        else {
            SetError(Unimplemented);
            return;
        }
    }

    /**
     * Sends an IPC command to PCSX2. @n
     * Fails if the IPC cannot be sent or if PCSX2 returns IPC_FAIL.
     * Throws an IPCStatus on failure.
     * @param cmd An IPCBuffer containing the IPC command size and buffer OR a
     * BatchCommand.
     * @param rt An IPCBuffer containing the IPC return size and buffer.
     * @see IPCResult
     * @see IPCBuffer
     */
    template <typename T>
    auto SendCommand(const T &cmd, const T &rt = T()) -> void {
        IPCBuffer command;
        IPCBuffer ret;
        if constexpr (std::is_same<T, BatchCommand>::value) {
            command = cmd.ipc_message;
            ret = cmd.ipc_return;
        } else {
            command = cmd;
            ret = rt;
        }
#ifdef _WIN32
        SOCKET sock;
        struct sockaddr_in server;

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            SetError(Fail);
            return;
        }

        // Prepare the sockaddr_in structure
        server.sin_family = AF_INET;
        // localhost only
        server.sin_addr.s_addr = inet_addr("127.0.0.1");
        server.sin_port = htons(PORT);

        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
            close(sock);
            SetError(Fail);
            return;
        }

#else
        int sock;
        struct sockaddr_un server;

        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock < 0) {
            SetError(Fail);
            return;
        }
        server.sun_family = AF_UNIX;
        strcpy(server.sun_path, SOCKET_NAME);

        if (connect(sock, (struct sockaddr *)&server,
                    sizeof(struct sockaddr_un)) < 0) {
            close(sock);
            SetError(Fail);
            return;
        }

#endif

        if (write_portable(sock, command.buffer, command.size) < 0) {
            SetError(Fail);
            return;
        }

        if (read_portable(sock, ret.buffer, ret.size) < 0) {
            SetError(Fail);
            return;
        }
        close(sock);

        if ((unsigned char)ret.buffer[0] == IPC_FAIL) {
            SetError(Fail);
            return;
        }
    }

    /**
     * Initializes a MsgMultiCommand IPC message.  @n
     * Batch IPC messages are preferred when dealing with a lot of IPC messages
     * in a quick fashion. They are _very_ fast, 1000 Write<uint8_t> are as fast
     * as one Read<uint32_t> in non-batch mode, give or take, which is about
     * 100µs. @n
     * You'll have to build the IPC messages in advance, with this function and
     * FinalizeBatch, and will have to send the command yourself, along with
     * dealing with the extraction of return values, if need there is.
     * It is a little bit less convenient than the standard IPC but has, at the
     * very least, a 1000x speedup on big commands.
     * @see batch_blocking
     * @see batch_len
     * @see reply_len
     * @see arg_cnt
     * @see FinalizeBatch
     */
    auto InitializeBatch() -> void {
        batch_blocking.lock();
        ipc_blocking.lock();
        ipc_buffer[0] = (unsigned char)MsgMultiCommand;
        batch_len = 3;
        reply_len = 1;
        arg_cnt = 0;
    }

    /**
     * Finalizes a MsgMultiCommand IPC message.
     * @return A BatchCommand with:
     *         * The IPCBuffer of the message.
     *         * The IPCBuffer of the return.
     *         * The argument location in the reply buffer.
     * @see batch_blocking
     * @see batch_len
     * @see reply_len
     * @see arg_cnt
     * @see InitializeBatch
     * @see IPCBuffer
     * @see BatchCommand
     */
    auto FinalizeBatch() -> BatchCommand {
        // save size in IPC message header.
        ToArray<uint16_t>(ipc_buffer, arg_cnt, 1);

        // we copy our arrays to unblock the IPC class.
        uint16_t bl = batch_len;
        int rl = reply_len;
        char *c_cmd = new char[batch_len];
        memcpy(c_cmd, ipc_buffer, batch_len * sizeof(char));
        char *c_ret = new char[reply_len];
        memcpy(c_ret, ret_buffer, reply_len * sizeof(char));
        unsigned int *arg_place = new unsigned int[arg_cnt];
        memcpy(arg_place, batch_arg_place, arg_cnt * sizeof(unsigned int));

        // we unblock the mutex
        batch_blocking.unlock();
        ipc_blocking.unlock();

        // MultiCommand is done!
        return BatchCommand{ IPCBuffer{ bl, c_cmd }, IPCBuffer{ rl, c_ret },
                             arg_place };
    }

    /**
     * Reads a value from PCSX2 game memory. @n
     * On error throws an IPCStatus. @n
     * Format: XX YY YY YY YY @n
     * Legend: XX = IPC Tag, YY = Address. @n
     * Return: (ZZ*??) @n
     * Legend: ZZ = Value read.
     * @see IPCCommand
     * @see IPCStatus
     * @see GetReply
     * @param address The address to read.
     * @param T Flag to enable batch processing or not.
     * @return The value read in memory. If in batch mode the IPC message.
     */
    template <typename Y, bool T = false>
    auto Read(uint32_t address) {

        // easiest way to get tag into a constexpr is a lambda, necessary for
        // GetReply
        constexpr IPCCommand tag = []() -> IPCCommand {
            switch (sizeof(Y)) {
                case 1:
                    return MsgRead8;
                case 2:
                    return MsgRead16;
                case 4:
                    return MsgRead32;
                case 8:
                    return MsgRead64;
                default:
                    return MsgUnimplemented;
            }
        }();
        if constexpr (tag == MsgUnimplemented) {
            SetError(Unimplemented);
            return;
        }

        // batch mode
        if constexpr (T) {
            if (BatchSafetyChecks(5, sizeof(Y))) {
                SetError(OutOfMemory);
                return (char *)0;
            }
            char *cmd = FormatBeginning(&ipc_buffer[batch_len], address, tag);
            batch_len += 5;
            batch_arg_place[arg_cnt] = reply_len;
            reply_len += sizeof(Y);
            arg_cnt += 1;
            return cmd;
        } else {
            // we are already locked in batch mode
            std::lock_guard<std::mutex> lock(ipc_blocking);
            SendCommand(
                IPCBuffer{ 5, FormatBeginning(ipc_buffer, address, tag) },
                IPCBuffer{ 1 + sizeof(Y), ret_buffer });
            return GetReply<tag>(ret_buffer, 1);
        }
    }

    /**
     * Writes a value to PCSX2 game memory. @n
     * On error throws an IPCStatus. @n
     * Format: XX YY YY YY YY (ZZ*??) @n
     * Legend: XX = IPC Tag, YY = Address, ZZ = Value.
     * @see IPCCommand
     * @see IPCStatus
     * @param address The address to write to.
     * @param value The value to write.
     * @param T Flag to enable batch processing or not.
     * @return If in batch mode the IPC message otherwise void.
     */
    template <typename Y, bool T = false>
    auto Write(uint32_t address, Y value) {

        // easiest way to get tag into a constexpr is a lambda, necessary for
        // GetReply
        constexpr IPCCommand tag = []() -> IPCCommand {
            switch (sizeof(Y)) {
                case 1:
                    return MsgWrite8;
                case 2:
                    return MsgWrite16;
                case 4:
                    return MsgWrite32;
                case 8:
                    return MsgWrite64;
                default:
                    return MsgUnimplemented;
            }
        }();
        if constexpr (tag == MsgUnimplemented) {
            SetError(Unimplemented);
            return;
        }

        // batch mode
        if constexpr (T) {
            if (BatchSafetyChecks(5 + sizeof(Y))) {
                SetError(OutOfMemory);
                return (char *)0;
            }
            char *cmd = ToArray<Y>(
                FormatBeginning(&ipc_buffer[batch_len], address, tag), value,
                5);
            batch_len += 5 + sizeof(Y);
            arg_cnt += 1;
            return cmd;
        } else {
            // we are already locked in batch mode
            std::lock_guard<std::mutex> lock(ipc_blocking);
            char *cmd =
                ToArray(FormatBeginning(ipc_buffer, address, tag), value, 5);
            SendCommand(IPCBuffer{ 5 + sizeof(Y), cmd },
                        IPCBuffer{ 1, ret_buffer });
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
        // we allocate once buffers to not have to do mallocs for each IPC
        // request, as malloc is expansive when we optimize for µs.
        ret_buffer = new char[MAX_IPC_RETURN_SIZE];
        ipc_buffer = new char[MAX_IPC_SIZE];
        batch_arg_place = new unsigned int[MAX_BATCH_REPLY_COUNT];
    }

    /**
     * PCSX2Ipc Destructor.
     */
    ~PCSX2Ipc() {
        // We clean up winsock.
#ifdef _WIN32
        WSACleanup();
#endif
        delete[] ret_buffer;
        delete[] ipc_buffer;
        delete[] batch_arg_place;
    }
};