use std::{collections::HashMap, fs::File, io::Write};

use anyhow::Result;
use kenkoooo::{
    io::read_input,
    ops::Move,
    parser::read_solution,
    strategies::color_free_lunch::optimize_color_free_lunch,
    types::{Picture, State},
};
use serde::Deserialize;

fn main() -> Result<()> {
    let args = std::env::args().collect::<Vec<_>>();
    let output = &args[1];

    let file = File::open("../output/ranking.json")?;
    let ranking: HashMap<String, Vec<Solutions>> = serde_json::from_reader(file)?;

    for (problem_id, solutions) in ranking {
        let mut min = solutions[0].score;
        let (target, initial_state) = read_input(&problem_id)?;

        for solution in solutions {
            if solution.score - min > min / 10 {
                break;
            }

            eprintln!("challenging {}/{} ...", solution.batch_name, problem_id);
            let moves = read_solution(format!(
                "../output/{}/{}.isl",
                solution.batch_name, problem_id
            ))?;

            let new_moves = match optimize_color_free_lunch(&target, &initial_state, &moves) {
                Ok(new_moves) => new_moves,
                Err(e) => {
                    eprintln!("{:?}", e);
                    continue;
                }
            };
            let after = evaluate(&new_moves, &initial_state, &target)?;
            if min > after {
                eprintln!("{} -> {}", min, after);
                let output = format!("{output}/{problem_id}.isl");
                let mut file = File::create(&output)?;
                for mv in new_moves {
                    writeln!(&mut file, "{}", mv)?;
                }
                min = after;
                eprintln!("saved {}", output);
            } else {
                eprintln!("sad!");
            }
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
