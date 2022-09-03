use kenkoooo::{io::read_input, types::State};
use rand::{rngs::StdRng, SeedableRng};

fn main() {
    let args = std::env::args().collect::<Vec<_>>();

    let picture = read_input(&args[1]);
    let state = State::new(picture.width(), picture.height());

    let score = state.calc_score(&picture);
    println!("{}", score);
}
