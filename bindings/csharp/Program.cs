using System;
using System.Runtime.InteropServices;

namespace csharp
{
    class Program
    {
#if _WINDOWS
        const string libipc = "libpcsx2_ipc_c.dll";
#elif _OSX
        const string libipc = "libpcsx2_ipc_c.dylib";
#elif _UNIX
        const string libipc = "libpcsx2_ipc_c.so";
#endif

        // if the function you want to use isn't there you'll have to define the binding to the function
        // rule of thumb: pointers = IntPtr, enum = their underlying type(by default it's int in C but 
        // I should have properly defined their type explicitely).
        [DllImport(libipc)]
        static extern IntPtr newPCSX2Ipc();

        [DllImport(libipc)]
        static extern UInt64 Read(IntPtr v, UInt32 address, Byte msg,
                bool batch);

        [DllImport(libipc)]
        static extern void deletePCSX2Ipc(IntPtr v);

        [DllImport(libipc)]
        static extern UInt32 GetError(IntPtr v);

        static void Main(string[] args)
        {
            // we get our ipc object
            IntPtr ipc = newPCSX2Ipc();

            // we read an uint8_t from memory location 0x00347D34
            Console.WriteLine(Read(ipc, 0x00347D34, 0, false));

            // we check for errors
            Console.WriteLine("Error (if any): {0}", GetError(ipc));

            // we delete the object and free the resources
            deletePCSX2Ipc(ipc);

            // for more infos check out the C bindings documentation :D !
        }
    }
}
