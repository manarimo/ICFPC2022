use std::{
    fs::File,
    io::Write,
    sync::{
        atomic::{AtomicBool, Ordering},
        Arc,
    },
};

use kenkoooo::{fast::FastState, io::read_input, ops::Move, parser::read_solution, types::Picture};
use rand::{rngs::StdRng, Rng, SeedableRng};

fn main() -> anyhow::Result<()> {
    let running = Arc::new(AtomicBool::new(true));
    let r = running.clone();

    ctrlc::set_handler(move || {
        r.store(false, Ordering::SeqCst);
    })?;

    let args = std::env::args().collect::<Vec<_>>();

    let problem_id = &args[1];
    let solution = &args[2];
    let output = &args[3];

    let (target, state) = read_input(problem_id)?;
    let mut moves = read_solution(solution)?;

    let initial_state = FastState {
        global_counter: state.global_counter + 1,
        cost: 0,
        blocks: state
            .blocks
            .into_iter()
            .map(|(label, block)| (label.0, block))
            .collect(),
    };

    let mut cur_similarity = calc_similarity(&initial_state, &moves, &target).unwrap();
    let mut rng = StdRng::seed_from_u64(1);
    while running.load(Ordering::SeqCst) {
        let move_id = rng.gen_range(0..moves.len());
        match moves[move_id].clone() {
            Move::LineCut(mut m) => {
                let old_pos = m.pos;
                if rng.gen_bool(0.5) && m.pos > 0 {
                    m.pos -= 1;
                } else {
                    m.pos += 1;
                }

                moves[move_id] = Move::LineCut(m.clone());
                match calc_similarity(&initial_state, &moves, &target) {
                    Some(next_similarity) if next_similarity < cur_similarity => {
                        println!("{} -> {}", cur_similarity, next_similarity);
                        cur_similarity = next_similarity;
                    }
                    _ => {
                        m.pos = old_pos;
                        moves[move_id] = Move::LineCut(m);
                    }
                }
            }
            Move::PointCut(mut m) => {
                let (old_x, old_y) = (m.x, m.y);
                if rng.gen_bool(0.5) {
                    if rng.gen_bool(0.5) && m.x > 0 {
                        m.x -= 1;
                    } else {
                        m.x += 1;
                    }
                } else {
                    if rng.gen_bool(0.5) && m.y > 0 {
                        m.y -= 1;
                    } else {
                        m.y += 1;
                    }
                }
                moves[move_id] = Move::PointCut(m.clone());
                match calc_similarity(&initial_state, &moves, &target) {
                    Some(next_similarity) if next_similarity < cur_similarity => {
                        println!("{} -> {}", cur_similarity, next_similarity);

                        cur_similarity = next_similarity;
                    }
                    _ => {
                        m.x = old_x;
                        m.y = old_y;
                        moves[move_id] = Move::PointCut(m);
                    }
                }
            }
            _ => continue,
        }
    }

    let output = format!("{output}/{problem_id}.isl");
    let mut file = File::create(&output)?;
    for mv in moves {
        writeln!(&mut file, "{}", mv)?;
    }
    eprintln!("saved {}", output);
    Ok(())
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
