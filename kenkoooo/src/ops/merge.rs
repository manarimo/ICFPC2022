use crate::types::{Block, Label, State};

pub struct Merge {
    pub label1: Label,
    pub label2: Label,
}

impl State {
    pub fn apply_merge(&self, m: Merge) -> Self {
        let mut new_state = self.clone();
        let block1 = new_state.pop_block(&m.label1);
        let block2 = new_state.pop_block(&m.label2);

        let x1 = block1.x1.min(block2.x1);
        let y1 = block1.y1.min(block2.y1);
        let x2 = block1.x2.max(block2.x2);
        let y2 = block1.y2.max(block2.y2);
        let label = new_state.new_label();
        new_state.push_block(label, Block { x1, x2, y1, y2 });

        if block1.size() > block2.size() {
            new_state.add_cost(1, &block1);
        } else {
            new_state.add_cost(1, &block2);
        }
        new_state
    }
}
