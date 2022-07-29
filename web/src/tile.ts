// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 *
 */
class Tile {
	private readonly gen: Generator;
	private tryIndex: number;
	private _value: number;

	public  readonly baseElem: HTMLElement;
	private readonly valueElem: HTMLElement;

	public constructor(solver: Generator) {
		this.gen = solver;

		this.baseElem = document.createElement("div");
		this.baseElem.classList.add(
			"center-contents",
			"tile",
		);

		this.valueElem = document.createElement("div");
		this.valueElem.classList.add("tile--value");
		this.baseElem.appendChild(this.valueElem);
	}
	public clear(): void {
		this.nextTryIndex = 0;
		this.value = this.gen.O2;
	}
	public get isClear(): boolean {
		return this.value === this.gen.O2;
	}

	public get nextTryIndex(): number {
		return this.tryIndex;
	}
	public set nextTryIndex(newTryIndex: number) {
		this.tryIndex = newTryIndex;
	}

	public get value(): number {
		return this._value;
	}
	public set value(newValue: number) {
		this._value = newValue;
		this.valueElem.textContent = (newValue === this.gen.O2)
			? "" // <-- Set to empty string for empty tile.
			: "1234567890ABCDEFGHIJKLMNOPQRSTUVYXYZ"[newValue]!;
	}
}
Object.freeze(Tile);
Object.freeze(Tile.prototype);
