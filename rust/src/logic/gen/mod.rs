mod path;
use super::size::Prim;

pub trait Generator {
	fn generate() -> ();
}

///
struct GenericGenerator<
	OrdCount: Prim,
	LenCount: Prim,
	AreCount: Prim,
	LenField: Prim,
	const ORD: usize,
	const LEN: usize,
	const ARE: usize,
	const PATH: usize,
> /* where [AreCount; ARE]: Default */ {
	_phantom_data: std::marker::PhantomData<OrdCount>,
	try_order: [[LenCount; LEN]; LEN],

	/// Indexed by progress- not by coordinate (for better cache usage)
	buf: [LenCount; ARE],
	row_has: [LenField; LEN],
	col_has: [LenField; LEN],
	blk_has: [LenField; LEN],

	progress: AreCount,
	/// Indexed by progress- not by coordinate (for better cache usage).
	try_progress: [LenCount; ARE],

	/// The generator is "stuck" at a tile T when since the last time it was not
	/// stuck, symbols generated up to T left T with no valid choice of symbol.
	///
	/// This field is used to backtrack within the AOE of a placed symbol. It is
	/// effectively nil when progress advances past it, or when any advancement
	/// behind it touches a tile ouside its AOE.
	stuck_progress: AreCount,
}


impl<
	OrdCount: Prim,
	LenCount: Prim,
	AreCount: Prim,
	LenField: Prim,
	const ORD: usize,
	const LEN: usize,
	const ARE: usize,
	const PATH: usize,
> GenericGenerator<OrdCount, LenCount, AreCount, LenField, ORD, LEN, ARE, PATH>
	// where [AreCount; ARE]: Default
{
	pub fn new() -> Self {
		Self {
			_phantom_data: std::marker::PhantomData::<OrdCount>,
			try_order: [[LenCount::zero(); LEN]; LEN],

			/// Indexed by progress- not by coordinate (for better cache usage)
			buf: [LenCount::zero(); ARE],
			row_has: [LenField::zero(); LEN],
			col_has: [LenField::zero(); LEN],
			blk_has: [LenField::zero(); LEN],

			progress: AreCount::zero(),
			/// Indexed by progress- not by coordinate (for better cache usage).
			try_progress: [LenCount::zero(); ARE],

			/// The generator is "stuck" at a tile T when since the last time it was not
			/// stuck, symbols generated up to T left T with no valid choice of symbol.
			///
			/// This field is used to backtrack within the AOE of a placed symbol. It is
			/// effectively nil when progress advances past it, or when any advancement
			/// behind it touches a tile ouside its AOE.
			stuck_progress: AreCount::zero(),
		}
	}
	pub fn generate(&mut self) {
		self.buf[0] = num::cast(1).unwrap();
		0;
	}
}

#[test]
fn test() {
	let gen = GenericGenerator::<u8,u8,u8,u8,3,9,81,1>::new();
}