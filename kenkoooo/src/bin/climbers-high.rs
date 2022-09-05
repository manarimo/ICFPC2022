use std::{collections::HashMap, fs::File, io::Write, time::Instant};

use anyhow::Result;
use kenkoooo::{
    fast::FastState,
    io::read_input,
    ops::Move,
    parser::read_solution,
    strategies::color_free_lunch::optimize_color_free_lunch,
    types::{Picture, State},
};
use rand::{rngs::StdRng, Rng, SeedableRng};
use rayon::prelude::{IntoParallelIterator, ParallelIterator};
use serde::Deserialize;

fn main() -> Result<()> {
    let args = std::env::args().collect::<Vec<_>>();
    let output = &args[1];

    let file = File::open("../output/ranking.json")?;
    let ranking: HashMap<String, Vec<Solutions>> = serde_json::from_reader(file)?;

    let mut starting_points = vec![];
    for (problem_id, solutions) in ranking {
        let (target, initial_state) = read_input(&problem_id)?;
        if let Some(s) = solutions.into_iter().min_by_key(|s| s.score) {
            let moves = read_solution(format!("../output/{}/{}.isl", s.batch_name, problem_id))?;
            starting_points.push((s.score, target, initial_state, moves, problem_id));
        }
    }

    starting_points.sort_by_key(|s| -s.0);
    starting_points
        .into_par_iter()
        .for_each(|(_, target, state, moves, problem_id)| {
            if let Err(e) = optimize_single_solution(&state, moves, &target, output, &problem_id) {
                eprintln!("{:?}", e);
            }
        });

    Ok(())
}

fn optimize_single_solution(
    state: &State,
    mut moves: Vec<Move>,
    target: &Picture,
    output: &str,
    problem_id: &str,
) -> Result<Vec<Move>> {
    eprintln!("optimizing {problem_id}");
    let mut cur_point = evaluate(&moves, &state, &target)?;
    let initial_state = FastState {
        global_counter: state.global_counter + 1,
        cost: 0,
        blocks: state
            .blocks
            .clone()
            .into_iter()
            .map(|(label, block)| (label.0, block))
            .collect(),
    };
    let mut same_count = 0;
    const LIMIT: usize = 10;
    while same_count < LIMIT {
        let new_moves = optimize_color_free_lunch(&target, &state, &moves)?;
        let new_moves = climbing(&initial_state, new_moves, &target);
        let next_point = evaluate(&new_moves, &state, &target)?;
        if cur_point > next_point {
            eprintln!("{problem_id}: {cur_point} -> {next_point}");
            same_count = 0;
            cur_point = next_point;
            moves = new_moves;

            let output = format!("{output}/{problem_id}.isl");
            let mut file = File::create(&output)?;
            for mv in &moves {
                writeln!(&mut file, "{}", mv)?;
            }
        } else {
            same_count += 1;
            eprintln!("{problem_id}: same={same_count}");
        }
    }
    eprintln!("finished: {problem_id}");
    Ok(moves)
}

fn climbing(initial_state: &FastState, moves: Vec<Move>, target: &Picture) -> Vec<Move> {
    const TIME_SEC: f64 = 20.;
    let candidate_move_ids = (0..moves.len())
        .filter(|&i| matches!(&moves[i], Move::LineCut(_) | Move::PointCut(_)))
        .collect::<Vec<_>>();

    let start = Instant::now();
    let mut cur_score = calc_score(&initial_state, &moves, &target).unwrap();
    let mut cur_moves = moves;
    let mut rng = StdRng::seed_from_u64(1);
    while start.elapsed().as_secs_f64() < TIME_SEC {
        let r = rng.gen_range(0..candidate_move_ids.len());
        let move_id = candidate_move_ids[r];
        let old_move = cur_moves[move_id].clone();

        match cur_moves[move_id].clone() {
            Move::LineCut(mut new_move) => {
                const D: [usize; 2] = [0, !0];
                let i = rng.gen_range(0..2);
                new_move.pos = new_move.pos.wrapping_add(D[i]);
                if new_move.pos >= 400 || new_move.pos == 0 {
                    continue;
                }

                cur_moves[move_id] = Move::LineCut(new_move);
            }
            Move::PointCut(mut new_move) => {
                const D: [usize; 2] = [0, !0];
                let r = rng.gen_range(0..4);
                let i = r & 1;
                let j = (r >> 1) & 1;

                if i == 0 {
                    new_move.x = new_move.x.wrapping_add(D[j]);
                } else {
                    new_move.y = new_move.y.wrapping_add(D[j]);
                }
                if new_move.x == 0 || new_move.y == 0 || new_move.x >= 400 || new_move.y >= 400 {
                    continue;
                }

                cur_moves[move_id] = Move::PointCut(new_move);
            }
            _ => unreachable!(),
        }

        //todo
        let next_score = calc_score(initial_state, &cur_moves, target).unwrap_or(1e20);
        if cur_score > next_score {
            cur_score = next_score;
        } else {
            cur_moves[move_id] = old_move;
        }
    }
    cur_moves
}

fn calc_score(initial_state: &FastState, moves: &[Move], target: &Picture) -> Option<f64> {
    let mut state = initial_state.clone();
    let map = match state.apply_all(&moves) {
        Ok(map) => map,
        Err(_) => return None,
    };

    let mut similarity = 0.0;
    for (y, row) in map.into_iter().enumerate() {
        for (x, color) in row.into_iter().enumerate() {
            let mut sum = 0.0;
            for i in 0..4 {
                let a = target.0[y][x].0[i] as f64;
                let b = color[i] as f64;
                let d = a - b;
                sum += d * d;
            }
            similarity += sum.sqrt();
        }
    }
    Some(similarity * 0.005 + state.cost as f64)
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct Solutions {
    batch_name: String,
    score: i64,
}

fn evaluate(moves: &[Move], initial_state: &State, target: &Picture) -> Result<i64> {
    let mut state = initial_state.clone();
    for mv in moves {
        state = state.apply(mv)?;
    }

    Ok(state.calc_score(&target))
}
