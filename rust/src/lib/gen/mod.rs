mod path;
use super::size::Prim;

pub trait Generator {
	fn generate() -> ();
}

///
struct GenericGenerator<
	Ord1: Prim,
	Ord2: Prim,
	Ord4: Prim,
	OccMask: Prim,
	const O1: usize,
	const O2: usize,
	const O4: usize,
	const PATH: usize,
> {
	_phantom_data: std::marker::PhantomData<Ord1>,
	try_order: [[Ord2; O2]; O2],

	/// Indexed by progress- not by coordinate (for better cache usage)
	path_val: [Ord2; O4],
	rows_has: [Ord2; O2],
	cols_has: [Ord2; O2],
	blks_has: [OccMask; O2],

	progress: Ord4,
	/// Indexed by progress- not by coordinate (for better cache usage).
	path_try_index: [Ord2; O4],

	/// The generator is "stuck" at a tile T when since the last time it was not
	/// stuck, symbols generated up to T left T with no valid choice of symbol.
	///
	/// This field is used to backtrack within the AOE of a placed symbol. It is
	/// effectively nil when progress advances past it, or when any advancement
	/// behind it touches a tile ouside its AOE.
	stuck_at_progress: Ord4,
}


impl<
	Ord1: Prim,
	Ord2: Prim,
	Ord4: Prim,
	OccMask: Prim,
	const O1: usize,
	const O2: usize,
	const O4: usize,
	const PATH: usize,
> GenericGenerator<Ord1, Ord2, Ord4, OccMask, O1, O2, O4, PATH>
{
	pub fn new() -> Self {
		Self {
			_phantom_data: std::marker::PhantomData::<Ord1>,
			try_order: [[LenCount::zero(); O2]; O2],

			val: [Ord2::zero(); O4],
			row_has: [Ord2::zero(); O2],
			col_has: [Ord2::zero(); O2],
			blk_has: [Ord2::zero(); O2],

			progress: Ord4::zero(),
			try_progress: [Ord2::zero(); O4],
			stuck_progress: Ord2::zero(),
		}
	}
	pub fn generate(&mut self) {
		self.val[0] = num::cast(1).unwrap();
		0;
	}
}

#[test]
fn test() {
	let gen = GenericGenerator::<u8,u8,u8,u8,3,9,81,1>::new();
}