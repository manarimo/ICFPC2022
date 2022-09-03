use std::{fs::read_to_string, path::Path};

use crate::{scanner::IO, types::Picture};
use crate::ops::color::Color;
use crate::ops::Move;
use crate::types::{Block, Label, RGBA, State};

pub fn read_input<P: AsRef<Path>>(path: P) -> (Picture, State) {
    let input = read_to_string(path).unwrap();
    let mut sc = IO::new(input.as_bytes(), Vec::<u8>::new());

    let w: usize = sc.read();
    let h: usize = sc.read();

    let mut rgba = vec![vec![crate::types::RGBA([255; 4]); w]; h];
    for color in 0..4 {
        for i in 0..h {
            for j in 0..w {
                rgba[i][j].0[color] = sc.read();
            }
        }
    }

    let mut state = State::new(w, h);
    state.pop_block(&Label(vec![0]));

    let blocks: usize = sc.read();
    for _i in 0..blocks {
        let id: u32 = sc.read();
        let x1: usize = sc.read();
        let y1: usize = sc.read();
        let x2: usize = sc.read();
        let y2: usize = sc.read();
        let r: u8 = sc.read();
        let g: u8 = sc.read();
        let b: u8 = sc.read();
        let a: u8 = sc.read();
        state.push_block(Label(vec![id]), Block{x1, y1, x2, y2});
        let color_move = Move::Color(Color{ color: RGBA([r, g, b, a]), label: Label(vec![id])});
        state.apply(color_move);
    }

    (Picture(rgba), state)
}
