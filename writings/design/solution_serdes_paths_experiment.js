#!/usr/bin/env node
// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: GPL-3.0-or-later
const process = require("process");


for (let O = 3; O <= 20; ++O) {
	const O1 = O;
	const O2 = O1*O1;
	const O3 = O1*O2;
	const O4 = O2*O2;

	const wave_layer_order = [];
	for (let i = 0; i < O2; ++i) {
		// wave_layer_order[i] = ((i*O1) % O2) + Math.floor(i/O1);
		wave_layer_order[i] = i;
	}
	const wave_path_helper = (prog) => {
		let ii = prog;
		for (const layer of wave_layer_order) {
			if (ii < layer * 2 + 1) {
				return Object.freeze((ii > layer)
				? {layer, arm_a:0, arm_b:ii-layer}
				: {layer, arm_a:ii, arm_b:0});
			}
			ii -= layer * 2 + 1;
		}
		throw new Error();
	};
	const paths = {
		dealer: (i) => {
			const row = ((Math.floor(i / O1) * O1) % O2) + Math.floor(i / O3);
			const col = ((i * O1) % O2) + (Math.floor(i / O2) % O1);
			return (O2 * row) + col;
		},
		block_col2: (i) => {
			const row = ((Math.floor(i / O2) * O1) % O2) + Math.floor(i / O3);
			const col = i % O2;
			return (O2 * row) + col;
		},

		wave_out_arms_in: (i) => {
			const {layer, arm_a, arm_b} = wave_path_helper(i);
			return arm_a === 0
			? O2*layer + (arm_b)
			: O2*(arm_a-1) + layer;
		},
		wave_out_arms_out: (i) => {
			const {layer, arm_a, arm_b} = wave_path_helper(i);
			const row = layer - arm_b;
			const col = layer - arm_a;
			return (O2 * row) + col;
		},

		row_major: (i) => i,
		block_col1: (i) => {
			const row = Math.floor(i / O1) % O2;
			const col = (Math.floor(i / O3) * O1) + (i % O1);
			return (O2 * row) + col;
		},

		wave_in_arms_out: (i) => {
			const {layer, arm_a, arm_b} = wave_path_helper(O4-1-i);
			const row = layer - arm_a;
			const col = layer - arm_b;
			return (O2 * row) + col;
		},
		wave_in_arms_in: (i) => {
			const {layer, arm_a, arm_b} = wave_path_helper(O4-1-i);
			return arm_a === 0
			? O2*layer + (arm_b)
			: O2*(arm_a-1) + layer;
		},
	};
	Object.freeze(Object.entries(paths)).forEach(([path_name, prog_to_coord]) => {
		paths[path_name+"_hflip"] = (i) => {
			const coord = prog_to_coord(i);
			const row = Math.floor(coord / O2);
			const col = coord % O2;
			return (O2*row) + (O2-1-col);
		};
	});
	Object.freeze(paths);

	const data = Object.freeze(Object.entries(paths)).reduce((data, [path_name, prog_to_coord]) => {
		if (O === 3) {
			const coord_to_prog = Array(O4);
			for (let i = 0; i < O4; ++i) {
				coord_to_prog[prog_to_coord(i)] = i;
			}
			Object.freeze(coord_to_prog);
			for (let r = 0; r < O2; ++r) {
			for (let c = 0; c < O2; ++c) {
				process.stdout.write(coord_to_prog[O2*r+c].toString().padStart(2) + " ");
			}process.stdout.write("\n");}process.stdout.write("\n");
		}

		const rows_has  = Array(O2).fill(0);
		const cols_has  = Array(O2).fill(0);
		const boxes_has = Array(O2).fill(0);
		Object.seal(rows_has, cols_has, boxes_has);

		const max_candidates = Array(O4).fill(1);
		Object.seal(max_candidates);

		for (let prog = 0; prog < O4; ++prog) {
			const rmi = prog_to_coord(prog);
			const row = Math.floor(rmi / O2);
			const col = rmi % O2;
			const box = (Math.floor(row / O1) * O1) + Math.floor(col / O1);
			if (Math.floor(row / O1) === Math.floor(col / O1)) {
				continue;
			}
			max_candidates[rmi] = O2 - Math.max(rows_has[row], cols_has[col], boxes_has[box]);
			rows_has[row] += 1;
			cols_has[col] += 1;
			boxes_has[box] += 1;
			// process.stdout.write(coord_to_prog[(O2*r)+c] + " ");
		}
		Object.freeze(max_candidates);
		// for (let r = 0; r < O2; ++r) {
		// for (let c = 0; c < O2; ++c) {
		// 	process.stdout.write(max_candidates[O2*r+c].toString().padStart(2) + " ");
		// }process.stdout.write("\n");}process.stdout.write("\n");
		const total_grid_bits = max_candidates.reduce((sum_bits, num_cand) => sum_bits + Math.log2(num_cand), 0);
		const total_grid_bytes = Math.ceil(total_grid_bits / 8);
		const avg_entry_bits = total_grid_bits / O4;
		const avg_entry_range_normalized_for_order = (2 ** avg_entry_bits) / O2;
		data[path_name] = Object.freeze({
			total_grid_bytes,
			avg_entry_bits,
			avg_entry_range_normalized_for_order,
		});
		return data;
	}, {});
	console.log("\nfor order: " + O);
	console.table(Object.entries(data)
		.sort(([_a, a], [_b, b]) => a.avg_entry_bits - b.avg_entry_bits)
		.reduce((data, entry) => {
			data[entry[0]] = entry[1];
			return data;
		}, {})
	);
}
