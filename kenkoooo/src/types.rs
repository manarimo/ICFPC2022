use std::{collections::BTreeMap, fmt::Display};

#[derive(Clone)]
pub struct Picture(pub Vec<Vec<RGBA>>);
impl Picture {
    pub fn new(width: usize, height: usize) -> Self {
        Self(vec![vec![RGBA([255; 4]); width]; height])
    }

    pub fn height(&self) -> usize {
        self.0.len()
    }

    pub fn width(&self) -> usize {
        self.0[0].len()
    }
}

#[derive(Clone)]
pub struct State {
    pub picture: Picture,
    blocks: BTreeMap<Label, Block>,
    pub cost: i64,
    canvas: Block,
    pub(crate) global_counter: u32,
}

impl State {
    pub fn new(width: usize, height: usize) -> Self {
        Self {
            picture: Picture::new(width, height),
            blocks: BTreeMap::from([(
                Label(vec![0]),
                Block {
                    x1: 0,
                    y1: 0,
                    x2: width,
                    y2: height,
                },
            )]),
            cost: 0,
            canvas: Block {
                x1: 0,
                x2: width,
                y1: 0,
                y2: height,
            },
            global_counter: 0,
        }
    }

    pub fn new_label(&mut self) -> Label {
        self.global_counter += 1;
        Label(vec![self.global_counter])
    }

    pub fn push_block(&mut self, label: Label, block: Block) {
        self.blocks.insert(label, block);
    }
    pub fn pop_block(&mut self, label: &Label) -> Block {
        self.blocks.remove(label).unwrap()
    }
    pub fn get_block(&self, label: &Label) -> &Block {
        self.blocks.get(label).unwrap()
    }

    pub fn add_cost(&mut self, base_cost: i64, block: &Block) {
        let base = base_cost * self.canvas.size();
        let block_size = block.size();

        self.cost += base / block_size;
        if (base % block_size) * 2 >= block_size {
            self.cost += 1;
        }
    }
}

#[derive(Clone, Copy)]
pub struct Block {
    pub x1: usize,
    pub x2: usize,
    pub y1: usize,
    pub y2: usize,
}

impl Block {
    pub fn size(&self) -> i64 {
        let (dx, dy) = self.rect();
        dx * dy
    }

    pub fn rect(&self) -> (i64, i64) {
        let dx = self.x2 - self.x1;
        let dy = self.y2 - self.y1;
        (dx as i64, dy as i64)
    }
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub struct RGBA(pub [u8; 4]);

impl Display for RGBA {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{},{},{},{}", self.0[0], self.0[1], self.0[2], self.0[3])
    }
}

impl RGBA {
    pub fn diff(&self, rhs: &Self) -> f64 {
        let mut sum = 0;
        for i in 0..4 {
            let d = i64::from(self.0[i]) - i64::from(rhs.0[i]);
            sum += d * d;
        }

        (sum as f64).sqrt()
    }
}

#[derive(PartialEq, Eq, PartialOrd, Ord, Clone)]
pub struct Label(pub Vec<u32>);

impl Display for Label {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        for (i, p) in self.0.iter().enumerate() {
            if i > 0 {
                write!(f, ".")?;
            }
            write!(f, "{}", p)?;
        }
        Ok(())
    }
}

impl Label {
    pub fn push(&mut self, suffix: u32) {
        self.0.push(suffix);
    }
}

#[derive(Clone, Copy, Debug)]
pub struct Point {
    pub x: usize,
    pub y: usize,
}
