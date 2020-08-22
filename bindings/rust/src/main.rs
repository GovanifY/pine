extern crate libc;
use libc::{c_char, c_uint, c_ulong};

#[repr(C)] pub struct PCSX2Ipc { _private: [u8; 0] }

#[link(name = "pcsx2_ipc_c")]
extern "C" {
    /* I defined the functions I wanted to use for the example
     * below but if you need other ones you'll have to
     * define them here, it's not too hard, just be sure 
     * to look up Rust FFI documentation. */
    fn newPCSX2Ipc() -> *mut PCSX2Ipc;
    fn Read(v: *mut PCSX2Ipc, address: c_uint, msg: c_char, batch: bool) -> c_ulong;
    fn deletePCSX2Ipc(v: *mut PCSX2Ipc);
    fn GetError(v: *mut PCSX2Ipc) -> c_uint;

}
fn main() {
    unsafe {
        // we get our ipc object
        let _ipc = newPCSX2Ipc();

        // we read an uint8_t from memory location 0x00347D34
        println!("{}", Read(_ipc, 0x00347D34, 0x00, false));

        // we check for errors
        println!("Error (if any): {}", GetError(_ipc));

        // we delete the object and free the resources
        deletePCSX2Ipc(_ipc);
        
        // for more infos check out the C bindings documentation :D !
    }
}
