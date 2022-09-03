pub mod color;
pub mod lcut;
pub mod pcut;

use crate::types::{Label, State};

use self::{color::Color, lcut::LineCut, pcut::PointCut};

pub enum Move {
    LineCut(LineCut),
    PointCut(PointCut),
    Color(Color),
    Swap(Swap),
    Merge(Merge),
}

pub struct Swap {
    pub label1: Label,
    pub label2: Label,
}

pub struct Merge {
    pub label1: Label,
    pub label2: Label,
}

impl State {
    pub fn apply(&self, m: Move) -> Self {
        match m {
            Move::LineCut(x) => self.apply_lcut(x),
            Move::PointCut(x) => self.apply_pcut(x),
            Move::Color(x) => self.apply_color(x),
            _ => unimplemented!(),
        }
    }
}
