use crate::types::{Picture, State};

impl State {
    pub fn calc_score(&self, picture: &Picture) -> i64 {
        calc_similarity(&self.picture, picture)
    }
}

fn calc_similarity(p1: &Picture, p2: &Picture) -> i64 {
    let mut diff = 0.;
    for i in 0..p1.0.len() {
        for j in 0..p1.0[i].len() {
            diff += p1.0[i][j].diff(&p2.0[i][j]);
        }
    }
    ((diff * 0.005).round() + 0.1) as i64
}
