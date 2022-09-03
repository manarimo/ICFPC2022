pub mod color;
pub mod lcut;
pub mod merge;
pub mod pcut;
pub mod swap;

use crate::types::State;

use self::{color::Color, lcut::LineCut, merge::Merge, pcut::PointCut, swap::Swap};

pub enum Move {
    LineCut(LineCut),
    PointCut(PointCut),
    Color(Color),
    Swap(Swap),
    Merge(Merge),
}

impl State {
    pub fn apply(&self, m: Move) -> Self {
        match m {
            Move::LineCut(x) => self.apply_lcut(x),
            Move::PointCut(x) => self.apply_pcut(x),
            Move::Color(x) => self.apply_color(x),
            Move::Swap(x) => self.apply_swap(x),
            Move::Merge(x) => self.apply_merge(x),
        }
    }
}
