extern crate libc;
use libc::{c_char, c_uint, c_ulong};

/* In a perfect world we would setup this object to be specific 
 * to the emulator object you're using (eg PCSX2) to avoid calling
 * functions undefined in your object scope */
#[repr(C)] pub struct PINE { _private: [u8; 0] }

#[link(name = "pine_c")]
extern "C" {
    /* I defined the functions I wanted to use for the example
     * below but if you need other ones you'll have to
     * define them here, it's not too hard, just be sure 
     * to look up Rust FFI documentation. */
    fn pine_pcsx2_new() -> *mut PINE;
    fn pine_read(v: *mut PINE, address: c_uint, msg: c_char, batch: bool) -> c_ulong;
    fn pine_pcsx2_delete(v: *mut PINE);
    fn pine_get_error(v: *mut PINE) -> c_uint;

}
fn main() {
    unsafe {
        // we get our ipc object
        let _ipc = pine_pcsx2_new();

        // we read an uint8_t from memory location 0x00347D34
        println!("{}", pine_read(_ipc, 0x00347D34, 0x00, false));

        // we check for errors
        println!("Error (if any): {}", pine_get_error(_ipc));

        // we delete the object and free the resources
        pine_pcsx2_delete(_ipc);
        
        // for more infos check out the C bindings documentation :D !
    }
}
