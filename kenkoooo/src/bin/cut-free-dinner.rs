use std::{fs::File, io::Write, time::Instant};

use kenkoooo::{fast::FastState, io::read_input, ops::Move, parser::read_solution, types::Picture};
use rand::{rngs::StdRng, Rng, SeedableRng};

fn main() -> anyhow::Result<()> {
    let args = std::env::args().collect::<Vec<_>>();

    let problem_id = &args[1];
    let solution = &args[2];
    let output = &args[3];

    let (target, state) = read_input(problem_id)?;
    let moves = read_solution(solution)?;
    let initial_state = FastState {
        global_counter: state.global_counter + 1,
        cost: 0,
        blocks: state
            .blocks
            .into_iter()
            .map(|(label, block)| (label.0, block))
            .collect(),
    };

    let best_moves = annealing(&initial_state, moves, &target);

    let output = format!("{output}/{problem_id}.isl");
    let mut file = File::create(&output)?;
    for mv in best_moves {
        writeln!(&mut file, "{}", mv)?;
    }
    eprintln!("saved {}", output);
    Ok(())
}

fn annealing(initial_state: &FastState, moves: Vec<Move>, target: &Picture) -> Vec<Move> {
    const START_TEMP: f64 = 10000.;
    const END_TEMP: f64 = 1.;
    const TIME_SEC: f64 = 30.;

    let candidate_move_ids = (0..moves.len())
        .filter(|&i| matches!(&moves[i], Move::LineCut(_) | Move::PointCut(_)))
        .collect::<Vec<_>>();

    let start = Instant::now();
    let mut best_similarity = calc_similarity(&initial_state, &moves, &target).unwrap();
    let mut best_moves = moves;
    let mut cur_similarity = best_similarity;
    let mut cur_moves = best_moves.clone();
    let mut rng = StdRng::seed_from_u64(1);
    while start.elapsed().as_secs_f64() < TIME_SEC {
        let temperature = (START_TEMP - END_TEMP) / TIME_SEC * start.elapsed().as_secs_f64();

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

        let next_similarity = calc_similarity(initial_state, &cur_moves, target).unwrap_or(1e20);
        let probability = ((cur_similarity - next_similarity) / temperature).exp();
        if next_similarity < best_similarity {
            eprintln!("{best_similarity} -> {next_similarity}");
            best_similarity = next_similarity;
            best_moves = cur_moves.clone();
        }
        if probability >= 1.0 || rng.gen_bool(probability) {
            cur_similarity = next_similarity;
        } else {
            cur_moves[move_id] = old_move;
        }
    }
    best_moves
}

fn calc_similarity(initial_state: &FastState, moves: &[Move], target: &Picture) -> Option<f64> {
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
    Some(similarity)
}
