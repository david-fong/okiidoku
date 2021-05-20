use std::default::Default;
pub trait Prim: num::PrimInt + Default + From<usize> {}

// TODO when associated const functions become stable, replace these with implementations
// on the primitive types.
//impl Default for [ u8;   16] { pub fn default() -> Self { [0;   16]; }}
// impl Default for [u16;   81] { fn default() -> Self { return [0;   81]; }}
// impl Default for [u16;  256] { fn default() -> Self { return [0;  256]; }}
// impl Default for [u32;  625] { fn default() -> Self { return [0;  625]; }}
// impl Default for [u64; 1296] { fn default() -> Self { return [0; 1296]; }}

//  O    O    L    A      M
//  2:  u8,  u8,  u8,    u8
//  3:  u8,  u8,  u8,   u16
//  4:  u8,  u8, u16,   u16
//  5:  u8,  u8, u16,   u32
//  6:  u8,  u8, u16,   u64
//  7:  u8,  u8, u16,   u64
//  8:  u8,  u8, u16,   u64
//  9:  u8,  u8, u16,  u128
// 10:  u8,  u8, u16,  u128
// 11:  u8,  u8, u16,  u128
// 12:  u8,  u8, u16,  u128
// 13:  u8,  u8, u16,  u128
// 14:  u8,  u8, u16,  u128
// 15:  u8,  u8, u16,  u128
// 16:  u8, u16, u16,  u128
