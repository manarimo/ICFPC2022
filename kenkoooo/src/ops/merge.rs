use std::fmt::Display;

use anyhow::{anyhow, Result};

use crate::types::{Block, Label, State};

#[derive(Clone)]
pub struct Merge {
    pub label1: Label,
    pub label2: Label,
}

impl Display for Merge {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "merge[{}][{}]", self.label1, self.label2)
    }
}

impl State {
    pub(super) fn apply_merge(&self, m: &Merge) -> Result<Self> {
        let mut new_state = self.clone();
        let block1 = new_state.pop_block(&m.label1)?;
        let block2 = new_state.pop_block(&m.label2)?;

        let x1 = block1.x1.min(block2.x1);
        let y1 = block1.y1.min(block2.y1);
        let x2 = block1.x2.max(block2.x2);
        let y2 = block1.y2.max(block2.y2);
        let label = new_state.new_label();
        let block = Block { x1, x2, y1, y2 };
        if block.size() != block1.size() + block2.size() {
            return Err(anyhow!("can not merge"));
        }
        new_state.push_block(label, block);

        if block1.size() > block2.size() {
            new_state.add_cost(1, &block1);
        } else {
            new_state.add_cost(1, &block2);
        }
        Ok(new_state)
    }
}
