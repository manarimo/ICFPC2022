use kenkoooo::{io::read_input, parser::read_solution};

fn main() {
    let args = std::env::args().collect::<Vec<_>>();

    let (picture, mut state) = read_input(&args[1]);
    let moves = read_solution(&args[2]);

    for mv in moves {
        state = state.apply(mv);
    }
    let score = state.calc_score(&picture);
    println!("{}", score);
}
