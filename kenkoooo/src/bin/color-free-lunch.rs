use kenkoooo::{
    io::read_input,
    ops::Move,
    parser::read_solution,
    types::{Picture, Point, State, RGBA},
};

fn main() {
    let args = std::env::args().collect::<Vec<_>>();
    let (target, initial_state) = read_input(&args[1]);
    let moves = read_solution(&args[2]);

    let new_moves = optimize_moves(&target, &initial_state, &moves);

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

fn optimize_moves(target: &Picture, initial_state: &State, moves: &[Move]) -> Vec<Move> {
    let mut index_map = vec![vec![None; target.width()]; target.height()];
    let mut state = initial_state.clone();
    for (i, mv) in moves.iter().enumerate() {
        if let Move::Color(mv) = mv {
            let block = state.get_block(&mv.label);
            for y in block.y1..block.y2 {
                for x in block.x1..block.x2 {
                    index_map[y][x] = Some(i);
                }
            }
        }
        state = state.apply(mv);
    }

    let mut move_to_map = vec![vec![]; moves.len()];
    for (y, row) in index_map.into_iter().enumerate() {
        for (x, cell) in row.into_iter().enumerate() {
            if let Some(i) = cell {
                move_to_map[i].push(Point { y, x });
            }
        }
    }

    let move_to_places = move_to_map
        .into_iter()
        .enumerate()
        .filter(|a| !a.1.is_empty())
        .collect::<Vec<_>>();
    // validation
    for (_, places) in move_to_places.iter() {
        let color = state.picture.0[places[0].y][places[0].x];
        assert!(places
            .iter()
            .all(|place| state.picture.0[place.y][place.x] == color))
    }

    let mut result = vec![None; moves.len()];
    for (i, points) in move_to_places {
        let color = state.picture.0[points[0].y][points[0].x];
        let new_color = optimize(&target, color, &points);
        result[i] = Some(new_color);
    }

    moves
        .iter()
        .enumerate()
        .map(|(i, mv)| match (result[i], mv) {
            (Some(new_color), Move::Color(mv)) => {
                let mut new_move = mv.clone();
                new_move.color = new_color;
                Move::Color(new_move)
            }
            _ => mv.clone(),
        })
        .collect()
}

fn optimize(target: &Picture, initial_color: RGBA, points: &[Point]) -> RGBA {
    let mut cur_color = initial_color;
    let mut cur_similarity = point_raw_similarity(target, &cur_color, points);
    loop {
        let mut finished = true;
        for channel in 0..4 {
            for d in [0u8, !0u8] {
                while (cur_color.0[channel] as u16).wrapping_add(d as u16) < 256 {
                    cur_color.0[channel] = cur_color.0[channel].wrapping_add(d);
                    let next_similarity = point_raw_similarity(target, &cur_color, points);
                    if next_similarity < cur_similarity {
                        eprintln!("{}", next_similarity - cur_similarity);
                        cur_similarity = next_similarity;
                        finished = false;
                    } else {
                        cur_color.0[channel] = cur_color.0[channel].wrapping_sub(d);
                        break;
                    }
                }
            }
        }

        if finished {
            break;
        }
    }
    cur_color
}

fn point_raw_similarity(target: &Picture, color: &RGBA, points: &[Point]) -> f64 {
    let mut diff = 0.0;
    for point in points {
        let target = target.0[point.y][point.x];
        diff += target.diff(color);
    }
    diff
}
