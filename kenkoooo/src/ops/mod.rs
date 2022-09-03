pub mod color;
pub mod lcut;
pub mod pcut;

use crate::types::Label;

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
