use kenkoooo::{
    io::read_input,
    ops::Move,
    parser::read_solution,
    strategies::color_free_lunch::optimize_color_free_lunch,
    types::{Picture, State},
};

fn main() {
    let args = std::env::args().collect::<Vec<_>>();
    let (target, initial_state) = read_input(&args[1]);
    let moves = read_solution(&args[2]);

    let new_moves = optimize_color_free_lunch(&target, &initial_state, &moves);

    let before = evaluate(&moves, &initial_state, &target);
    let after = evaluate(&new_moves, &initial_state, &target);
    if before > after {
        eprintln!("{} -> {}", before, after);
        for mv in moves {
            println!("{}", mv);
        }
    } else {
        eprintln!("sad!");
    }
}

fn evaluate(moves: &[Move], initial_state: &State, target: &Picture) -> i64 {
    let mut state = initial_state.clone();
    for mv in moves {
        state = state.apply(mv);
    }

    state.calc_score(&target)
}