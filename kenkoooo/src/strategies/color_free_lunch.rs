use rayon::prelude::{IntoParallelIterator, ParallelIterator};

use crate::{
    ops::Move,
    types::{Picture, Point, State, RGBA},
};

pub fn optimize_color_free_lunch(
    target: &Picture,
    initial_state: &State,
    moves: &[Move],
) -> Vec<Move> {
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

        for place in places {
            assert_eq!(
                state.picture.0[place.y][place.x], color,
                "{} {}",
                place.y, place.x
            );
        }
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

fn old_optimize(target: &Picture, initial_color: RGBA, points: &[Point]) -> RGBA {
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
fn optimize(target: &Picture, initial_color: RGBA, points: &[Point]) -> RGBA {
    let mut min_max = [(255, 0), (255, 0), (255, 0), (255, 0)];
    for i in 0..4 {
        for point in points {
            min_max[i].0 = min_max[i].0.min(target.0[point.y][point.x].0[i]);
            min_max[i].1 = min_max[i].1.max(target.0[point.y][point.x].0[i]);
        }
    }

    let init_similarity = point_raw_similarity(target, &initial_color, points);
    let cur_color = initial_color;
    let cur_similarity = init_similarity;
    let (similarity, color) = (min_max[0].0..=min_max[0].1)
        .into_par_iter()
        .map(|r| {
            let mut cur_color = cur_color;
            let mut cur_similarity = cur_similarity;
            for g in min_max[1].0..=min_max[1].1 {
                for b in min_max[2].0..=min_max[2].1 {
                    for a in min_max[3].0..=min_max[3].1 {
                        let next_color = RGBA([r, g, b, a]);
                        let next_similarity = point_raw_similarity(target, &next_color, points);
                        if cur_similarity > next_similarity {
                            cur_color = next_color;
                            cur_similarity = next_similarity;
                        }
                    }
                }
            }
            (cur_similarity, cur_color)
        })
        .reduce(
            || (cur_similarity, cur_color),
            |cur, next| {
                if cur.0 > next.0 {
                    next
                } else {
                    cur
                }
            },
        );
    if init_similarity > similarity {
        eprintln!("{} -> {}", init_similarity, similarity);
    }
    color
}

fn point_raw_similarity(target: &Picture, color: &RGBA, points: &[Point]) -> f64 {
    let mut diff = 0.0;
    for point in points {
        let target = target.0[point.y][point.x];
        diff += target.diff(color);
    }
    diff
}
