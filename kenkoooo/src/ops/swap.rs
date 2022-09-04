use std::fmt::Display;

use crate::types::{Label, State};

#[derive(Clone)]
pub struct Swap {
    pub label1: Label,
    pub label2: Label,
}

impl Display for Swap {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "swap[{}][{}]", self.label1, self.label2)
    }
}

impl State {
    pub(super) fn apply_swap(&self, m: &Swap) -> Self {
        let mut new_state = self.clone();
        let block1 = self.get_block(&m.label1);
        let block2 = self.get_block(&m.label2);

        assert_eq!(block1.rect(), block2.rect());

        let dx = block1.x2 - block1.x1;
        let dy = block1.y2 - block1.y1;

        for x in 0..dx {
            for y in 0..dy {
                new_state.picture.0[y + block1.y1][x + block1.x1] =
                    self.picture.0[y + block2.y1][x + block2.x1];
                new_state.picture.0[y + block2.y1][x + block2.x1] =
                    self.picture.0[y + block1.y1][x + block1.x1];
            }
        }

        new_state.add_cost(3, block1);
        new_state
    }
}
