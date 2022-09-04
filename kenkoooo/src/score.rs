use std::ops::Range;

use crate::types::{Picture, State};

impl State {
    pub fn calc_score(&self, target: &Picture) -> i64 {
        let similarity = calc_similarity(&self.picture, target);
        similarity + self.cost
    }
}

fn calc_similarity(result: &Picture, target: &Picture) -> i64 {
    let diff = range_raw_similarity(result, target, 0..result.width(), 0..target.height());
    ((diff * 0.005).round() + 0.1) as i64
}

pub fn range_raw_similarity(
    result: &Picture,
    target: &Picture,
    x: Range<usize>,
    y: Range<usize>,
) -> f64 {
    let mut diff = 0.0;
    for y in y {
        for x in x.clone() {
            let p1 = result.0[y][x];
            let p2 = target.0[target.height() - 1 - y][x];
            diff += p1.diff(&p2);
        }
    }
    diff
}
