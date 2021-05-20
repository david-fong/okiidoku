use std::env;

fn main() {
	let args: Vec<String> = env::args().collect();
	println!("{:?}", args);
	let a = std::f32::NAN;
}
