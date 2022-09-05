use anyhow::{anyhow, Result};
use rayon::prelude::{IntoParallelIterator, ParallelIterator};

use crate::{
    ops::Move,
    types::{Picture, Point, State, RGBA},
};

pub fn optimize_color_free_lunch(
    target: &Picture,
    initial_state: &State,
    moves: &[Move],
) -> Result<Vec<Move>> {
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
        state = state.apply(mv)?;
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
            if state.picture.0[place.y][place.x] != color {
                return Err(anyhow!("invalid {} {}", place.y, place.x));
            }
        }
    }

    let mut result = vec![None; moves.len()];
    let colors = move_to_places
        .into_par_iter()
        .map(|(move_id, points)| {
            let color = state.picture.0[points[0].y][points[0].x];
            let new_color = optimize(&target, color, &points);
            (move_id, new_color)
        })
        .collect::<Vec<_>>();
    for (move_id, new_color) in colors {
        result[move_id] = Some(new_color);
    }

    Ok(moves
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
        .collect())
}

fn _old_optimize(target: &Picture, initial_color: RGBA, points: &[Point]) -> RGBA {
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

fn distance(v: &[f64; 4], w: &[f64; 4]) -> f64 {
    let mut sum = 0.;
    for d in 0..4 {
        sum += (v[d] - w[d]).powi(2);
    }
    sum.powf(0.5)
}

fn f64v(u: &[u8; 4]) -> [f64; 4] {
    [u[0] as f64, u[1] as f64, u[2] as f64, u[3] as f64]
}

fn round(f: f64) -> u8 {
    if f <= 0. {
        0
    } else if f >= 255. {
        255
    } else {
        f.round() as u8
    }
}

fn optimize(target: &Picture, initial_color: RGBA, points: &[Point]) -> RGBA {
    let mut current: [f64; 4] = f64v(&initial_color.0);
    for _ in 0..20 {
        let mut next: [f64; 4] = [0., 0., 0., 0.];
        let mut coef = 0.;
        for point in points {
            let color_vec = f64v(&target.0[point.y][point.x].0);
            let dist = distance(&color_vec, &current);
            if dist == 0. {
                break;
            }
            coef += 1. / dist;
        }
        for point in points {
            let color_vec = f64v(&target.0[point.y][point.x].0);
            let dist = distance(&color_vec, &current);
            for d in 0..4 {
                next[d] += color_vec[d] / dist / coef;
            }
        }
        current = next;
    }

    let mut min_max = [(255, 0), (255, 0), (255, 0), (255, 0)];
    for i in 0..4 {
        for point in points {
            min_max[i].0 = min_max[i].0.min(target.0[point.y][point.x].0[i]);
            min_max[i].1 = min_max[i].1.max(target.0[point.y][point.x].0[i]);
        }
    }

    let color = RGBA([
        round(current[0]),
        round(current[1]),
        round(current[2]),
        round(current[3]),
    ]);

    let init_similarity = point_raw_similarity(target, &initial_color, points);
    let similarity = point_raw_similarity(target, &color, points);
    if init_similarity > similarity {
        eprintln!("{} -> {}", init_similarity, similarity);
        color
    } else {
        initial_color
    }
}

fn point_raw_similarity(target: &Picture, color: &RGBA, points: &[Point]) -> f64 {
    let mut diff = 0.0;
    for point in points {
        let target = target.0[point.y][point.x];
        diff += target.diff(color);
    }
    diff
}
