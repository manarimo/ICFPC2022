use std::fs::File;

use kenkoooo::{
    io::read_input,
    ops::Move,
    parser::read_solution,
    strategies::color_free_lunch::optimize_color_free_lunch,
    types::{Picture, State},
};
use serde_json::Value;

fn main() {
    let args = std::env::args().collect::<Vec<_>>();

    let file = File::open(&args[1]).unwrap();
    let problem = &args[2];
    let output = &args[3];
    let ranking: Value = serde_json::from_reader(file).unwrap();

    for (problem_id, solutions) in ranking.as_object().unwrap() {
        let solutions = solutions.as_array().unwrap();
        if solutions.is_empty() {
            continue;
        }

        let solver_name = &solutions[0]["batchName"].as_str().unwrap();
        eprintln!("solver={solver_name} problem_id={problem_id}");
        let (target, initial_state) = read_input(format!("{problem}/plaintext/{problem_id}.txt"));
        let moves = read_solution(format!("{output}/{solver_name}/{problem_id}.isl"));
        let new_moves = optimize_color_free_lunch(&target, &initial_state, &moves);
        let before = evaluate(&moves, &initial_state, &target);
        let after = evaluate(&new_moves, &initial_state, &target);
        if before > after {
            eprintln!("{before} -> {after}");
        }
    }
}

fn evaluate(moves: &[Move], initial_state: &State, target: &Picture) -> i64 {
    let mut state = initial_state.clone();
    for mv in moves {
        state = state.apply(mv);
    }

    state.calc_score(&target)
}
