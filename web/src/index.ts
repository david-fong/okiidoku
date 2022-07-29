// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 *
 */
class Gui {
	private gen: Generator;
	public readonly in: Readonly<{
		gridOrder: HTMLSelectElement;
		genPath:   HTMLSelectElement;
	}>;
	private readonly host: Readonly<{
		grid: HTMLElement;
	}>;
	readonly #startOverBtn: HTMLButtonElement;

	public constructor() {
		this._initSolverParams();
		this._initPlaybackControls();
		(Object.keys(this.in) as Array<keyof Gui["in"]>).forEach((key) => {
			this.in[key].addEventListener("change", (ev) => {
				this.resetSolver();
			});
		})
		this.host = Object.freeze(<Gui["host"]>{
			grid: document.getElementById("host-grid"),
		});
		this.resetSolver();
	}

	private _initSolverParams(): void {
		const gridOrder = document.getElementById("sel-grid-order") as HTMLSelectElement;
		gridOrder.selectedIndex = Array.from(gridOrder.options).findIndex((opt) => {
			return opt.value === (localStorage.getItem("grid-order") ?? "3");
		});
		gridOrder.dispatchEvent(new Event("change"));
		gridOrder.addEventListener("change", (ev) => {
			localStorage.setItem("grid-order", gridOrder.value);
		});
		const genPath = document.getElementById("sel-gen-path") as HTMLSelectElement;
		genPath.selectedIndex = Array.from(genPath.options).findIndex((opt) => {
			return opt.value === Generator.GenPath.ROW_MAJOR;
		});
		(this.in as Gui["in"]) = Object.freeze(<Gui["in"]>{
			gridOrder,
			genPath,
		});
	}

	private _initPlaybackControls(): void {
		const startOver = document.getElementById("btn-playback-value-start-over") as HTMLButtonElement;
		const valueTest = document.getElementById("btn-playback-value-test") as HTMLButtonElement;
		const backtrack = document.getElementById("btn-playback-backtrack")  as HTMLButtonElement;
		const playPause = document.getElementById("btn-playback-play-pause") as HTMLButtonElement;
		// @ts-expect-error
		this.#startOverBtn = startOver;

		const uiNotifyDoneGenerating = (
			workerFunc: (() => boolean) = (() => true),
		) => {
			return () => {
				if (workerFunc()) {
					valueTest.disabled = true;
					backtrack.disabled = true;
					playPause.disabled = true;
				}
			};
		};
		startOver.addEventListener("click", (ev) => {
			if (setIntervalId !== undefined) {
				stopPlaying();
			}
			this.solver.clear();
			valueTest.disabled = false;
			backtrack.disabled = false;
			playPause.disabled = false;
		});
		valueTest.addEventListener("click", uiNotifyDoneGenerating(() => {
			if (this.solver.singleStep() === undefined) {
				return true;
			}
			return false;
		}));
		backtrack.addEventListener("click", uiNotifyDoneGenerating(() => {
			let dir: Generator.Direction | undefined = undefined;
			do { dir = this.solver.singleStep(); } while (dir !== undefined && !dir.isBack);
			return dir === undefined;
		}));

		const speedCoarse = document.getElementById("slider-playback-speed-coarse") as HTMLInputElement;
		const speedFine   = document.getElementById("slider-playback-speed-fine")   as HTMLInputElement;

		let setIntervalId: number | undefined = undefined;
		const stopPlaying = () => {
			playPause.textContent = "play";
			valueTest.disabled = false;
			backtrack.disabled = false;
			if (setIntervalId) {
				clearInterval(setIntervalId);
				setIntervalId = undefined;
			}
		};
		const startPlaying = () => {
			playPause.textContent = "pause";
			valueTest.disabled = true;
			backtrack.disabled = true;
			setIntervalId = setInterval(() => {
				if (!this.solver.singleStep()) {
					stopPlaying();
					uiNotifyDoneGenerating()();
				}
			}, 1000 / (Number(speedCoarse.value) + Number(speedFine.value)));
		};
		playPause.addEventListener("click", (ev) => {
			if (setIntervalId === undefined) {
				startPlaying();
			} else {
				stopPlaying();
			}
		});
		[speedCoarse,speedFine].forEach((slider) => {
			slider.addEventListener("change", (ev) => {
				if (setIntervalId !== undefined) {
					stopPlaying();
					startPlaying();
				}
			});
		});
	}

	public resetSolver(): void {
		const order = Number(this.in.gridOrder.selectedOptions.item(0)!.value);
		const genPath = this.in.genPath.value as Generator.GenPath;

		if (!this.solver || order !== this.solver.O1) {
			this.solver?.gridElem.remove();
			this.gen = new Generator(order, genPath);
			this.host.grid.appendChild(this.solver.gridElem);
			// TODO.mid select the solver's default genpath.
			this.in.genPath;

		} else {
			if (genPath !== this.solver.genPath) {
				this.solver.genPath = genPath;
			}
		}
		this.#startOverBtn.click();
	}

	public get solver(): Generator {
		return this.gen;
	}
}
Object.freeze(Gui);
Object.freeze(Gui.prototype);

const gui = new Gui();
