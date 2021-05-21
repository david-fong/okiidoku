
///
struct Grid<
	// OrdCount: num::PrimInt,
	LenCount: num::PrimInt,
	// AreCount: num::PrimInt,
	// LenField: num::PrimInt,
	const ORD: usize,
	const LEN: usize,
	const ARE: usize,
> {
	pub buf: [LenCount; ARE],
}

impl<
	// OrdCount: num::PrimInt,
	LenCount: num::PrimInt,
	// AreCount: num::PrimInt,
	// LenField: num::PrimInt,
	const ORD: usize,
	const LEN: usize,
	const ARE: usize,
> Grid<LenCount, ORD, LEN, ARE>
{
	pub fn generate(&mut self) {
		self.buf[0] = num::cast(1).unwrap();
		0;
	}
}

#[test]
fn test() {
}