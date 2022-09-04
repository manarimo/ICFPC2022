use std::fmt::Display;

use crate::types::{Label, State};

pub struct PointCut {
    pub label: Label,
    pub x: usize,
    pub y: usize,
}
impl Display for PointCut {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "cut[{}][{},{}]", self.label, self.x, self.y)
    }
}

impl State {
    pub(super) fn apply_pcut(&self, m: PointCut) -> Self {
        let mut new_state = self.clone();
        let block = new_state.pop_block(&m.label);
        let (x, y) = (m.x, m.y);
        assert!(block.x1 <= x && x < block.x2);
        assert!(block.y1 <= y && y < block.y2);

        let mut bottom_left = block.clone();
        bottom_left.x2 = x;
        bottom_left.y2 = y;

        let mut bottom_left_label = m.label.clone();
        bottom_left_label.push(0);
        new_state.push_block(bottom_left_label, bottom_left);

        let mut bottom_right = block.clone();
        bottom_right.x1 = x;
        bottom_right.y2 = y;

        let mut bottom_right_label = m.label.clone();
        bottom_right_label.push(1);
        new_state.push_block(bottom_right_label, bottom_right);

        let mut top_right = block.clone();
        top_right.x1 = x;
        top_right.y1 = y;

        let mut top_right_label = m.label.clone();
        top_right_label.push(2);
        new_state.push_block(top_right_label, top_right);

        let mut top_left = block.clone();
        top_left.x2 = x;
        top_left.y1 = y;

        let mut top_left_label = m.label;
        top_left_label.push(2);
        new_state.push_block(top_left_label, top_left);

        new_state.add_cost(10, &block);
        new_state
    }
}
