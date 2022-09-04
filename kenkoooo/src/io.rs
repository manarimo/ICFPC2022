use std::fs::File;
use std::{fs::read_to_string, path::Path};

use crate::ops::color::Color;
use crate::ops::Move;
use crate::types::{Block, Label, State, RGBA};
use crate::{scanner::IO, types::Picture};

pub fn read_input<P: AsRef<Path>>(path: P) -> (Picture, State) {
    let input = read_to_string(path).unwrap();
    let mut sc = IO::new(input.as_bytes(), Vec::<u8>::new());

    let w: usize = sc.read();
    let h: usize = sc.read();

    let mut rgba = vec![vec![crate::types::RGBA([255; 4]); w]; h];
    for color in 0..4 {
        // 使いやすいように上下反転させておく
        for i in (0..h).rev() {
            for j in 0..w {
                rgba[i][j].0[color] = sc.read();
            }
        }
    }
    let target = Picture(rgba);

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

        let label = Label(vec![id]);

        state.push_block(label.clone(), Block { x1, y1, x2, y2 });
        let color_move = Move::Color(Color {
            color: RGBA([r, g, b, a]),
            label,
        });
        state.apply(&color_move);
        state.global_counter = state.global_counter.max(id);
    }
    state.cost = 0;
    (target, state)
}

pub fn read_input2(problem_id: &str) -> anyhow::Result<(Picture, State)> {
    let png = File::open(format!("../problem/original/${problem_id}.png"))?;
    let decoder = png::Decoder::new(png);
    let mut reader = decoder.read_info()?;
    let mut reader = decoder.read_info()?;
    let mut img_data = vec![0; reader.output_buffer_size()];
    let info = reader.next_frame(&mut img_data)?;

    let w = info.width as usize;
    let h = info.height as usize;

    let mut rgba = vec![vec![crate::types::RGBA([255; 4]); w]; h];
    for i in 0..h {
        for j in 0..w {
            for c in 0..4 {
                rgba[i][j][c]=img_data[]
            }
        }
    }
}
