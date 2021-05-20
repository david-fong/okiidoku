use super::GenericGenerator;
use super::super::size::Prim;

#[derive(PartialEq, Eq)]
pub enum Path {
	/// Goes through rows top to bottom, each row left to right.
	RowMajor,
	CardDeal,
	BlockCol,
}

impl<
	OrdCount: Prim,
	LenCount: Prim,
	AreCount: Prim,
	LenField: Prim,
	const ORD: usize,
	const LEN: usize,
	const ARE: usize,
> GenericGenerator<OrdCount, LenCount, AreCount, LenField, ORD, LEN, ARE, {Path::RowMajor as usize}> {
	fn progress_to_coord(progress: AreCount) -> AreCount {
		return progress;
	}
}

static MAP<const O: num::PrimInt>: [u8; O] = [0u8; O];

impl<
	OrdCount: Prim,
	LenCount: Prim,
	AreCount: Prim,
	LenField: Prim,
	const ORD: usize,
	const LEN: usize,
	const ARE: usize,
> GenericGenerator<OrdCount, LenCount, AreCount, LenField, ORD, LEN, ARE, {Path::CardDeal as usize}> {
	const MAP: [AreCount; ARE] = Self::INIT_MAP();
	const fn INIT_MAP() -> [AreCount; ARE] {
		let map: [AreCount; ARE] = [num::zero::<AreCount>(); ARE];
		let i: usize = 0;
		/* for b_row in 0..ORD {
			for b_col in 0..ORD {
				for blk in 0..LEN {
					let blk_addr = ((blk % ORD) * ORD) + (blk / ORD * ORD * LEN);
					map[i] = blk_addr + (b_row * LEN) + b_col;
				}
			}
		} */
		let mut b_row = 0usize; while b_row < ORD {
			let mut b_col = 0usize; while b_col < ORD {
				let mut blk = 0usize; while blk < LEN {
					let blk_addr = ((blk % ORD) * ORD) + (blk / ORD * ORD * LEN);
					map[i] = num::cast(blk_addr + (b_row * LEN) + b_col).unwrap();
					blk += 1;
				}
				b_col += 1;
			}
			b_row += 1;
		}
		return map;
	}
	fn progress_to_coord(progress: AreCount) -> AreCount {
		return Self::MAP[num::cast::<AreCount, usize>(progress).unwrap()];
	}
}

impl<
	OrdCount: Prim,
	LenCount: Prim,
	AreCount: Prim,
	LenField: Prim,
	const ORD: usize,
	const LEN: usize,
	const ARE: usize,
> GenericGenerator<OrdCount, LenCount, AreCount, LenField, ORD, LEN, ARE, {Path::BlockCol as usize}> {
	const MAP: [AreCount; ARE] = Self::INIT_MAP();
	const fn INIT_MAP() -> [AreCount; ARE] {
		let map: [AreCount; ARE];
		let i: usize = 0;
		/* for blk_col in 0..ORD {
			for row in 0..LEN {
				for b_col in 0..ORD {
					map[i] = (blk_col * ORD) + (row * LEN) + (b_col);
				}
			}
		} */
		let mut blk_col = 0usize; while blk_col < ORD {
			let mut row = 0usize; while row < LEN {
				let mut b_col = 0usize; while b_col < ORD {
					map[i] = num::cast((blk_col * ORD) + (row * LEN) + (b_col)).unwrap();
					b_col += 1;
				}
				row += 1;
			}
			blk_col += 1;
		}
		return map;
	}
	fn progress_to_coord(progress: AreCount) -> AreCount {
		return Self::MAP[num::cast::<AreCount, usize>(progress).unwrap()];
	}
}