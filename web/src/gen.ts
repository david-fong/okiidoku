
// TODO support order 6 (requires BigUint64Array)

/**
 *
 */
class Generator {
	public readonly O1: number;
	public readonly O2: number;
	public readonly O4: number;

	private readonly grid: ReadonlyArray<Tile>;
	private readonly rowsHas: Uint32Array;
	private readonly colsHas: Uint32Array;
	private readonly blksHas: Uint32Array;

	private readonly valTryOrder: Array<Uint8Array>;
	#genPath: Generator.GenPath;
	private readonly prog2coord: Uint16Array;

	#workOpCount:   number;
	private progress: number;

	public gridElem: HTMLElement;

	public constructor(
		order: number,
		initialGenPath: Generator.GenPath,
	) {
		this.O1 = order;
		this.O2 = this.O1 * this.O1;
		this.O4 = this.O2 * this.O2;

		const grid: Array<Tile> = [];
		const gridElem = document.createElement("div");
		gridElem.classList.add("grid");
		document.body.style.setProperty("--grid-order", this.O1.toString());
		this.gridElem = gridElem;

		{const blockElems = [];
		for (let i = 0; i < this.O2; ++i) {
			const blockElem = document.createElement("div");
			blockElem.classList.add("grid--block");
			blockElems.push(blockElem);
			gridElem.appendChild(blockElem);
		}
		for (let i = 0; i < this.O4; ++i) {
			const tile = new Tile(this);
			grid.push(tile);
			blockElems[this.getBlk(i)]!.appendChild(tile.baseElem);
		}}
		this.grid = Object.freeze(grid);

		this.valTryOrder = [];
		for (let i = 0; i < this.O2; ++i) {
			const iota = new Uint8Array(this.O2 + 1);
			for (let i = 0; i < this.O2 + 1; ++i) { iota[i] = i; }
			this.valTryOrder.push(iota);
		}
		this.prog2coord = new Uint16Array(this.O4);
		this.genPath = initialGenPath;

		// Note: Put the order-dependant types last as a JS engine shapes optimization.
		// The types will be dependent once support for order 6 is added.
		["row","col","blk"].forEach((type) => {
			((this as any)[type + "Has"] as Generator["rowsHas"]) = new Uint32Array(this.O2);
		});
	}

	public clear(): void {
		this.grid.forEach((tile) => tile.clear());
		[this.rowsHas, this.colsHas, this.blksHas].forEach((masksArr) => {
			for (let i = 0; i < this.O2; ++i) { masksArr[i] = 0; }
		});
		this.valTryOrder.forEach((tryOrder) => {
			const shuffle = tryOrder.slice(1,this.O2);
			shuffle.sort((a,b) => Math.random() - 0.5);
			tryOrder.set(shuffle);
		});
		this.#workOpCount   = 0;
		this.progress  = 0;
	}

	/**
	 * Tests a new value at the current tile in the traversal path.
	 */
	public singleStep(): Generator.Direction | undefined {
		if (!this.isDone) {
			const gridIndex = this.prog2coord[this.progress]!;
			return this.setNextValid(gridIndex);
		}
		return undefined;
	}

	public get isDone(): boolean {
		return this.progress === this.O4;
	}

	private setNextValid(index: number): Generator.Direction {
		const tile = this.grid[index]!;
		const row = this.getRow(index);
		const col = this.getCol(index);
		const blk = this.getBlk(index);
		if (!(tile.isClear)) {
			const eraseMask = ~(1 << tile.value);
			this.rowsHas[row] &= eraseMask;
			this.colsHas[col] &= eraseMask;
			this.blksHas[blk] &= eraseMask;
		}
		const invalidBin = (
			this.rowsHas[row]! |
			this.colsHas[col]! |
			this.blksHas[blk]!
		);
		for (let tryIndex = tile.nextTryIndex; tryIndex < this.O2; ++tryIndex) {
			const value = this.valTryOrder[this.getRow(index)]![tryIndex]!;
			const valueBit = 1 << value;
			if (!(invalidBin & valueBit)) {
				// If a valid value is found for this tile:
				this.rowsHas[row] |= valueBit;
				this.colsHas[col] |= valueBit;
				this.blksHas[blk] |= valueBit;
				tile.value = value;
				tile.nextTryIndex = (tryIndex + 1);
				++this.progress;
				this.#workOpCount++;
				return { isBack: false, isSkip: false };
			}
		}
		tile.clear();
		--this.progress;
		this.#workOpCount++;
		return { isBack: true, isSkip: false };
	}

	public get genPath(): Generator.GenPath {
		return this.#genPath;
	}
	public set genPath(newGenPath: Generator.GenPath) {
		this.#genPath = newGenPath;
		switch (newGenPath) {
		case Generator.GenPath.ROW_MAJOR: {
			for (let i = 0; i < this.O4; ++i) {
				this.prog2coord[i] = i;
			}
			break; }
		case Generator.GenPath.BLOCK_COL: {
			const order = this.O1;
			let i = 0;
			for (let blkCol = 0; blkCol < order; ++blkCol) {
				for (let row = 0; row < this.O2; ++row) {
					for (let bCol = 0; bCol < order; ++bCol) {
						this.prog2coord[i++] = (blkCol * order) + (row * this.O2) + (bCol);
					}
				}
			}
			break; }
		}
	}

	public getRow(index: number): number { return Math.floor(index / this.O2); }
	public getCol(index: number): number { return index % this.O2; }
	public getBlk(index: number): number { return this.getBlk2(this.getRow(index), this.getCol(index)); }
	public getBlk2(row: number, col: number): number {
		const order = this.O1;
		return (Math.floor(row / order) * order) + Math.floor(col / order);
	}
}
namespace Generator {
	export const enum GenPath {
		ROW_MAJOR = "row-major",
		BLOCK_COL = "block-col",
	};
	export interface Direction {
		isBack: boolean;
		isSkip: boolean; // only meaningful when is_back is true.
	};
}
Object.freeze(Generator);
Object.freeze(Generator.prototype);
