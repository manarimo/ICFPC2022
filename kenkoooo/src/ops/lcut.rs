use crate::types::{Label, State};

pub struct LineCut {
    pub label: Label,
    pub orientation: Orientation,
    pub pos: usize,
}

pub enum Orientation {
    X,
    Y,
}

impl State {
    pub fn apply_lcut(&self, m: LineCut) -> Self {
        let mut new_state = self.clone();
        let block = new_state.pop_block(&m.label);
        new_state.add_cost(7, &block);
        match m.orientation {
            Orientation::X => {
                assert!(block.x1 <= m.pos && m.pos < block.x2);

                let mut right = block.clone();
                let mut left = block;

                left.x2 = m.pos;
                right.x1 = m.pos;

                let mut left_label = m.label.clone();
                let mut right_label = m.label;
                left_label.push(0);
                right_label.push(1);

                new_state.push_block(left_label, left);
                new_state.push_block(right_label, right);

                new_state
            }
            Orientation::Y => {
                assert!(block.y1 <= m.pos && m.pos < block.y2);

                let mut top = block.clone();
                let mut bottom = block;

                top.y1 = m.pos;
                bottom.y2 = m.pos;

                let mut top_label = m.label.clone();
                let mut bottom_label = m.label;
                top_label.push(1);
                bottom_label.push(0);

                new_state.push_block(top_label, top);
                new_state.push_block(bottom_label, bottom);

                new_state
            }
        }
    }
}
