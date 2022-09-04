pub mod color;
pub mod lcut;
pub mod merge;
pub mod pcut;
pub mod swap;

use std::fmt::Display;

use crate::types::State;

use self::{color::Color, lcut::LineCut, merge::Merge, pcut::PointCut, swap::Swap};

pub enum Move {
    LineCut(LineCut),
    PointCut(PointCut),
    Color(Color),
    Swap(Swap),
    Merge(Merge),
}

impl Display for Move {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Move::LineCut(m) => {
                write!(f, "{}", m)
            }
            Move::PointCut(m) => {
                write!(f, "{}", m)
            }
            Move::Color(m) => {
                write!(f, "{}", m)
            }
            Move::Swap(m) => {
                write!(f, "{}", m)
            }
            Move::Merge(m) => {
                write!(f, "{}", m)
            }
        }
    }
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
