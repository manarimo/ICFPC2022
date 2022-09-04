use crate::types::{Picture, State};

impl State {
    pub fn calc_score(&self, picture: &Picture) -> i64 {
        let similarity = calc_similarity(&self.picture, picture);
        similarity + self.cost
    }
}

fn calc_similarity(p1: &Picture, p2: &Picture) -> i64 {
    let mut diff = 0.0;

    for (p1, p2) in p1.0.iter().zip(p2.0.iter().rev()) {
        for (p1, p2) in p1.iter().zip(p2.iter()) {
            diff += p1.diff(p2);
        }
    }
    ((diff * 0.005).round() + 0.1) as i64
}
