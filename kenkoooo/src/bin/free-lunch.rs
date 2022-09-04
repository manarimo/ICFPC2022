use kenkoooo::{
    io::read_input,
    ops::Move,
    parser::read_solution,
    types::{Picture, State},
};
use rand::{rngs::StdRng, SeedableRng};

fn main() {
    let args = std::env::args().collect::<Vec<_>>();
    let (picture, mut state) = read_input(&args[1]);
    let moves = read_solution(&args[2]);

    let mut rng = StdRng::seed_from_u64(1);
}

fn calc_score(solution: Vec<Move>, pic: &Picture, initial: State) -> i64 {
    let mut state = initial;
    for mv in solution {
        state = state.apply(mv);
    }
    state.calc_score(pic)
}
