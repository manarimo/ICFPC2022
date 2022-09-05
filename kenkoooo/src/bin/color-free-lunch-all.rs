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

fn main() -> Result<()> {
    let args = std::env::args().collect::<Vec<_>>();
    let output = &args[1];

    let file = File::open("../output/ranking.json")?;
    let ranking: HashMap<String, Vec<Solutions>> = serde_json::from_reader(file)?;

    for (problem_id, solutions) in ranking {
        let min = solutions[0].score;
        let (target, initial_state) = read_input(&problem_id)?;
        let solutions = solutions
            .into_iter()
            .filter(|s| s.score - min < min / 10)
            .map(|s| read_solution(format!("../output/{}/{}.isl", s.batch_name, problem_id)))
            .collect::<Result<Vec<_>>>()?;

        let (_, new_moves) = solutions
            .par_iter()
            .map(|moves| {
                let new_moves = optimize_color_free_lunch(&target, &initial_state, &moves, true)?;
                let point = evaluate(&new_moves, &initial_state, &target)?;
                Ok((point, new_moves))
            })
            .collect::<Vec<Result<_>>>()
            .into_iter()
            .filter_map(|a| a.ok())
            .min_by_key(|(point, _)| *point)
            .unwrap();

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
