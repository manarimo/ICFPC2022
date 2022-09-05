use std::{collections::HashMap, fs::File, io::Write};

use anyhow::Result;
use kenkoooo::{
    io::read_input,
    ops::Move,
    parser::read_solution,
    strategies::color_free_lunch::optimize_color_free_lunch,
    types::{Picture, State},
};
use rayon::prelude::{IntoParallelRefIterator, ParallelIterator};
use serde::Deserialize;
use kenkoooo::ops::comment::Comment;

fn main() -> Result<()> {
    let args = std::env::args().collect::<Vec<_>>();
    let output = &args[1];

    let file = File::open("../output/ranking.json")?;
    let ranking: HashMap<String, Vec<Solutions>> = serde_json::from_reader(file)?;
    'L: for (problem_id, solutions) in ranking {
        let solver = &solutions[0];
        let moves = read_solution(format!("../output/{}/{}.isl", solver.batch_name, problem_id))?;
        eprintln!("problem:{} solved by {}", problem_id, solver.batch_name);
        for mv in &moves {
            if let Move::Comment(com) = mv {
                if com.comment.starts_with("color-optimized-true") {
                    continue 'L;
                }
            }
        }
        let (target, initial_state) = read_input(&problem_id)?;
        let mut new_moves = optimize_color_free_lunch(&target, &initial_state, &moves, true)?;
        let before = evaluate(&moves, &initial_state, &target)?;
        let after = evaluate(&new_moves, &initial_state, &target)?;
        eprintln!("{} -> {}", before, after);
        new_moves.insert(0, Move::Comment(Comment {
            comment: String::from(format!("color-optimized-{} {} -> {}. Base: {}", true, before, after, solver.batch_name))}));
        let output = format!("{output}/{problem_id}.isl");
        let mut file = File::create(&output)?;
        for mv in new_moves {
            writeln!(&mut file, "{}", mv)?;
        }
    }

    Ok(())
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct Solutions {
    batch_name: String,
    problem_id: String,
    score: i64,
}

fn evaluate(moves: &[Move], initial_state: &State, target: &Picture) -> Result<i64> {
    let mut state = initial_state.clone();
    for mv in moves {
        state = state.apply(mv)?;
    }

    Ok(state.calc_score(&target))
}
