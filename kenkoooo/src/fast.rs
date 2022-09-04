use std::collections::{BTreeMap, BTreeSet};

use anyhow::{Context, Result};

use crate::ops::{lcut::Orientation, Move};

const H: usize = 400;
const W: usize = 400;
struct FastState {
    blocks: BTreeMap<Vec<u32>, Block>,
    cost: usize,
    global_counter: u32,
}

#[derive(Clone)]
struct Block {
    x1: usize,
    x2: usize,
    y1: usize,
    y2: usize,
}

impl Block {
    fn size(&self) -> usize {
        let dx = self.x2 - self.x1;
        let dy = self.y2 - self.y1;
        dx * dy
    }
}

impl FastState {
    pub fn apply_all(&mut self, moves: &[Move]) -> Result<()> {
        let mut colors = vec![];
        for (i, mv) in moves.iter().enumerate() {
            match mv {
                Move::LineCut(mv) => {
                    let block = self.blocks.remove(&mv.label.0).context("")?;
                    self.cost += round(7 * H * W, block.size());
                    let Block { x1, x2, y1, y2 } = block;
                    match mv.orientation {
                        Orientation::X => {
                            let x = mv.pos;

                            let mut left_label = mv.label.0.clone();
                            let mut right_label = mv.label.0.clone();
                            left_label.push(0);
                            right_label.push(1);

                            let left = Block { x1, x2: x, y1, y2 };
                            let right = Block { x1: x, x2, y1, y2 };
                            self.blocks.insert(left_label, left);
                            self.blocks.insert(right_label, right);
                        }
                        Orientation::Y => {
                            let y = mv.pos;

                            let mut lower_label = mv.label.0.clone();
                            let mut upper_label = mv.label.0.clone();
                            upper_label.push(0);
                            lower_label.push(1);

                            let upper = Block { x1, x2, y1, y2: y };
                            let lower = Block { x1, x2, y1: y, y2 };
                            self.blocks.insert(upper_label, upper);
                            self.blocks.insert(lower_label, lower);
                        }
                    }
                }
                Move::PointCut(mv) => {
                    let block = self.blocks.remove(&mv.label.0).context("")?;
                    self.cost += round(7 * H * W, block.size());
                    let Block { x1, x2, y1, y2 } = block;
                    let (x, y) = (mv.x, mv.y);

                    let mut upper_left = mv.label.0.clone();
                    upper_left.push(0);
                    let mut upper_right = mv.label.0.clone();
                    upper_right.push(1);
                    let mut lower_right = mv.label.0.clone();
                    lower_right.push(2);
                    let mut lower_left = mv.label.0.clone();
                    lower_left.push(3);

                    self.blocks.insert(
                        upper_left,
                        Block {
                            x1,
                            x2: x,
                            y1,
                            y2: y,
                        },
                    );
                    self.blocks.insert(
                        upper_right,
                        Block {
                            x1: x,
                            x2,
                            y1,
                            y2: y,
                        },
                    );
                    self.blocks.insert(
                        lower_right,
                        Block {
                            x1: x,
                            x2,
                            y1: y,
                            y2,
                        },
                    );
                    self.blocks.insert(
                        lower_left,
                        Block {
                            x1,
                            x2: x,
                            y1: y,
                            y2,
                        },
                    );
                }
                Move::Color(mv) => {
                    let block = self.blocks.get(&mv.label.0).context("")?;
                    self.cost += round(5 * H * W, block.size());
                    colors.push((i, block.clone()));
                }
                Move::Swap(_) => todo!(),
                Move::Merge(mv) => {
                    let block1 = self.blocks.remove(&mv.label1.0).context("")?;
                    let block2 = self.blocks.remove(&mv.label2.0).context("")?;
                    self.cost += round(1 * H * W, block1.size().max(block2.size()));

                    let x1 = block1.x1.min(block2.x1);
                    let y1 = block1.y1.min(block2.y1);
                    let x2 = block1.x2.max(block2.x2);
                    let y2 = block1.y2.max(block2.y2);

                    let label = vec![self.global_counter];
                    self.blocks.insert(label, Block { x1, x2, y1, y2 });
                }
            }
        }

        let mut add = vec![vec![]; H];
        let mut remove = vec![vec![]; H + 1];
        for (i, color) in colors.iter().enumerate() {
            let from = color.1.y1;
            let to = color.1.y2;
            add[from].push(i);
            remove[to].push(i);
        }

        let mut map = vec![vec![[255; 4]; W]; H];
        let mut hello = vec![BTreeSet::new(); W];
        let mut bye = vec![BTreeSet::new(); W + 1];
        for y in 0..H {
            for &add in add[y].iter() {
                let from = colors[add].1.x1;
                let to = colors[add].1.x2;
                hello[from].insert(colors[add].0);
                bye[to].insert(colors[add].0);
            }
            for &remove in remove[y].iter() {
                let from = colors[remove].1.x1;
                let to = colors[remove].1.x2;
                hello[from].remove(&colors[remove].0);
                bye[to].remove(&colors[remove].0);
            }

            let mut set = BTreeSet::new();
            for x in 0..W {
                for &hello in hello[x].iter() {
                    set.insert(hello);
                }
                for &bye in bye[x].iter() {
                    set.remove(&bye);
                }
                if let Some(move_id) = set.iter().next_back().cloned() {
                    if let Move::Color(c) = &moves[move_id] {
                        map[y][x] = c.color.0.clone();
                    }
                }
            }
        }

        Ok(())
    }
}

const fn round(x: usize, divisor: usize) -> usize {
    let t = x / divisor;
    let r = x - divisor * t;
    if r * 2 >= divisor {
        t + 1
    } else {
        t
    }
}
