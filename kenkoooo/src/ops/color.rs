use std::fmt::Display;

use crate::types::{Label, State, RGBA};

pub struct Color {
    pub label: Label,
    pub color: RGBA,
}

impl Display for Color {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "color[{}][{}]", self.label, self.color)
    }
}

impl State {
    pub fn apply_color(&self, color: Color) -> Self {
        let mut new_state = self.clone();
        let block = new_state.get_block(&color.label).clone();
        for x in block.x1..block.x2 {
            for y in block.y1..block.y2 {
                new_state.picture.0[y][x] = color.color;
            }
        }

        new_state.add_cost(5, &block);
        new_state
    }
}
