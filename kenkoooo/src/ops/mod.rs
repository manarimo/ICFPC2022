pub mod color;
pub mod lcut;
pub mod merge;
pub mod pcut;
pub mod swap;
pub mod comment;

use std::fmt::Display;

use anyhow::Result;

use crate::types::State;

use self::{color::Color, lcut::LineCut, merge::Merge, pcut::PointCut, swap::Swap, comment::Comment};

#[derive(Clone)]
pub enum Move {
    LineCut(LineCut),
    PointCut(PointCut),
    Color(Color),
    Swap(Swap),
    Merge(Merge),
    Comment(Comment),
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
            Move::Comment(m) => {
                write!(f, "{}", m)
            }
        }
    }
}

impl State {
    pub fn apply(&self, m: &Move) -> Result<Self> {
        match m {
            Move::LineCut(x) => self.apply_lcut(x),
            Move::PointCut(x) => self.apply_pcut(x),
            Move::Color(x) => Ok(self.apply_color(x)),
            Move::Swap(x) => Ok(self.apply_swap(x)),
            Move::Merge(x) => self.apply_merge(x),
            Move::Comment(_) => Ok(self.clone()),
        }
    }
}
