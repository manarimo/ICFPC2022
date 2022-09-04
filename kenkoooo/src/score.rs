use crate::types::{Block, Picture, State};

impl State {
    pub fn calc_score(&self, target: &Picture) -> i64 {
        let similarity = calc_similarity(&self.picture, target);
        similarity + self.cost
    }
}

fn calc_similarity(result: &Picture, target: &Picture) -> i64 {
    let diff = range_raw_similarity(
        result,
        target,
        &Block {
            x1: 0,
            x2: target.width(),
            y1: 0,
            y2: target.height(),
        },
    );
    diff.to_normalized_similarity()
}

pub fn range_raw_similarity(p1: &Picture, p2: &Picture, block: &Block) -> f64 {
    let mut diff = 0.0;
    for y in block.y1..block.y2 {
        for x in block.x1..block.x2 {
            let p1 = p1.0[y][x];
            let p2 = p2.0[y][x];
            diff += p1.diff(&p2);
        }
    }
    diff
}

pub trait RawSimilarity {
    fn to_normalized_similarity(self) -> i64;
}

impl RawSimilarity for f64 {
    fn to_normalized_similarity(self) -> i64 {
        ((self * 0.005).round() + 0.1) as i64
    }
}
